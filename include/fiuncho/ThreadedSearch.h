/*
 * This file is part of Fiuncho.
 *
 * Fiuncho is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Fiuncho is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Fiuncho. If not, see <https://www.gnu.org/licenses/>.
 */

/**
 * @file Engine.h
 * @author Christian Ponte
 * @date 1 March 2018
 *
 * @brief Abstract class Engine definition.
 */

#ifndef FIUNCHO_THREADEDSEARCH_H
#define FIUNCHO_THREADEDSEARCH_H

#include <fiuncho/ContingencyTable.h>
#include <fiuncho/GenotypeTable.h>
#include <fiuncho/Search.h>
#include <fiuncho/algorithms/MutualInformation.h>
#include <fiuncho/dataset/Dataset.h>
#include <fiuncho/utils/MaxArray.h>
#include <fiuncho/utils/StaticStack.h>
#include <thread>
#include <vector>

#ifdef BENCHMARK
#include <iostream>
#endif

#define BLOCK_SIZE 10240

class ThreadedSearch : public Search
{
  public:
    ThreadedSearch(unsigned int threads) : nthreads(threads) {}

    std::vector<Result<uint32_t, float>> run(const Dataset<uint64_t> &dataset,
                                             const unsigned short order,
                                             Distributor<uint32_t> distributor,
                                             const unsigned int outputs)
    {
        int i;
        // Spawn threads
        std::vector<std::thread> threads;
        std::vector<MaxArray<Result<uint32_t, float>>> maxarrays;
#ifdef BENCHMARK
        std::vector<std::chrono::high_resolution_clock::time_point>
            thread_timers;
#endif
        // Pre-reserve space to avoid reallocating the underlying array, which
        // results in an error since previous addresses are rendered incorrect
        maxarrays.reserve(nthreads);
        for (i = 0; i < nthreads; i++) {
            maxarrays.emplace_back(outputs);
#ifdef BENCHMARK
            thread_timers.push_back(std::chrono::high_resolution_clock::now());
#endif
            threads.emplace_back(thread_main, std::ref(dataset), order,
                                 distributor.layer(nthreads, i).get_pairs(),
                                 std::ref(maxarrays.back()));
        }

        std::vector<Result<uint32_t, float>> results;
        results.reserve(nthreads *
                        outputs); // Wait for the completion of all threads
        int completed = 0;
        i = 0;
        while (completed < nthreads) {
            if (threads[i].joinable()) {
                threads[i].join();
#ifdef BENCHMARK
                // Measure elapsed time
                auto elapsed_time = std::chrono::high_resolution_clock::now() -
                                    thread_timers[i];
                double seconds =
                    std::chrono::duration<double>(elapsed_time).count();
                // Count operations
                const unsigned short k = order - 2;
                unsigned long ops = 0;
                // for (auto p : pairs_vector[i]) {
                //     const unsigned int n = dataset.cases.size() - p.second -
                //     1; unsigned long num = 1; unsigned long denom = 1; for
                //     (auto j = n; j > n - k; j--) {
                //         num *= j;
                //     }
                //     for (auto j = k; j > 1; j--) {
                //         denom *= j;
                //     }
                //     ops += num / denom;
                // }
                // Print information
                std::cout << "Thread " << i << ": " << seconds << "s, " << ops
                          << " combinations\n";
#endif
                results.insert(results.end(), &maxarrays[i][0],
                               &maxarrays[i][outputs]);
                completed++;
            }
            i = (i + 1) % nthreads;
        }
        // Sort the auxiliar array and resize the result before returning
        std::sort(results.rbegin(), results.rend());
        results.resize(outputs);
        return results;
    }

    static void thread_main(const Dataset<uint64_t> &dataset,
                            const unsigned short order,
                            PairList<uint32_t> pairs,
                            MaxArray<Result<uint32_t, float>> &maxarray)
    {
        if (order == 2) {
            flat_exploration(dataset, pairs, maxarray);
        } else {
            depth_exploration(dataset, order, pairs, maxarray);
        }
    }

  private:
    static void flat_exploration(const Dataset<uint64_t> &d,
                                 PairList<uint32_t> &pairs,
                                 MaxArray<Result<uint32_t, float>> &maxarray)
    {
        std::vector<ContingencyTable<uint32_t>> ctables;
        ctables.reserve(BLOCK_SIZE);
        for (auto i = 0; i < BLOCK_SIZE; i++) {
            ctables.emplace_back(2, d[0].cases_words, d[0].ctrls_words);
        }
        uint32_t ids[BLOCK_SIZE * 2];
        MutualInformation<float> mi(d.cases, d.ctrls);
        Result<uint32_t, float> buffer;
        buffer.combination.resize(2);

        int i, j;
        auto it = pairs.begin();
        while (it != pairs.end()) {
            for (i = 0; i < BLOCK_SIZE && it != pairs.end(); i++, ++it) {
                ids[i * 2] = it->first;
                ids[i * 2 + 1] = it->second;
                GenotypeTable<uint64_t>::combine_and_popcnt(
                    d[it->first], d[it->second], ctables[i]);
            }
            for (j = 0; j < i; j++) {
                buffer.combination[0] = ids[j * 2];
                buffer.combination[1] = ids[j * 2 + 1];
                buffer.val = mi.compute(ctables[j]);
                maxarray.add(buffer);
            }
        }
    }

    static void depth_exploration(const Dataset<uint64_t> &d,
                                  const unsigned short order,
                                  PairList<uint32_t> &pairs,
                                  MaxArray<Result<uint32_t, float>> &maxarray)
    {
        typedef struct {
            unsigned short size;
            uint32_t pos[];
        } Combination;
        // Use a stack to explore combination space
        const size_t item_size = sizeof(Combination) + order * sizeof(uint32_t);
        StaticStack<Combination> stack(item_size, d.snps * order - 2);
        Combination *cbuffer = (Combination *)new char[item_size];

        // Auxiliary bit tables for combinations sized under the target order
        std::vector<GenotypeTable<uint64_t>> gtables;
        for (auto o = 2; o < order; o++) {
            gtables.emplace_back(o, d[0].cases_words, d[0].ctrls_words);
        }
        // Vector of contingency tables (and their SNPs) for block processing
        std::vector<ContingencyTable<uint32_t>> ctables;
        for (auto i = 0; i < BLOCK_SIZE; i++) {
            ctables.emplace_back(order, d[0].cases_words, d[0].ctrls_words);
        }
        uint32_t ids[BLOCK_SIZE * order];

        MutualInformation<float> mi(d.cases, d.ctrls);
        Result<uint32_t, float> rbuffer;
        rbuffer.combination.resize(order);

        const auto data_lim = d.snps - 1;
        int i, j;
        auto it = pairs.begin();
        while (!(stack.empty() && it == pairs.end())) {
            // Fill ctables block
            i = 0;
            while (i < BLOCK_SIZE) {
                // If the stack is empty read next pair and refill it
                if (stack.empty()) {
                    // If there are no pairs left, exit
                    if (it == pairs.end()) {
                        break;
                    } else {
                        GenotypeTable<uint64_t>::combine(
                            d[it->first], d[it->second], gtables[0]);
                        cbuffer->size = 3;
                        cbuffer->pos[0] = it->first;
                        cbuffer->pos[1] = it->second;
                        for (j = data_lim; j > it->second; j--) {
                            cbuffer->pos[2] = j;
                            stack.push(*cbuffer);
                        }
                        ++it;
                        continue; // A pair can result in no triplets,
                                  // therefore we must check if the stack is
                                  // empty again
                    }
                }
                // Process the combination from the top of the stack
                stack.pop(*cbuffer);
                const auto &prev = gtables[cbuffer->size - 3];
                const auto &s = cbuffer->pos[cbuffer->size - 1];
                if (cbuffer->size == order) {
                    memcpy(ids + i * order, cbuffer->pos, order * 4);
                    GenotypeTable<uint64_t>::combine_and_popcnt(prev, d[s],
                                                                ctables[i]);
                    i++;
                } else {
                    GenotypeTable<uint64_t>::combine(
                        prev, d[s], gtables[cbuffer->size - 2]);
                    cbuffer->size += 1;
                    for (j = data_lim; j > s; j--) {
                        cbuffer->pos[cbuffer->size - 1] = j;
                        stack.push(*cbuffer);
                    }
                }
            }
            // Compute MI for ctables in block
            for (j = 0; j < i; j++) {
                memcpy(rbuffer.combination.data(), ids + j * order, order * 4);
                rbuffer.val = mi.compute(ctables[j]);
                maxarray.add(rbuffer);
            }
        }

        free(cbuffer);
    }

    const unsigned int nthreads;
};

#endif
