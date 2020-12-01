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

#include <fiuncho/dataset/Dataset.h>
#include <fiuncho/engine/BitTable.h>
#include <fiuncho/engine/ContingencyTable.h>
#include <fiuncho/engine/Result.h>
#include <fiuncho/engine/Search.h>
#include <fiuncho/engine/algorithms/MutualInformation.h>
#include <fiuncho/utils/MaxArray.h>
#include <fiuncho/utils/StaticStack.h>
#include <thread>
#include <vector>

#ifdef BENCHMARK
#include <iostream>
#endif

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
                for (auto p : pairs_vector[i]) {
                    const unsigned int n = dataset.cases.size() - p.second - 1;
                    unsigned long num = 1;
                    unsigned long denom = 1;
                    for (auto j = n; j > n - k; j--) {
                        num *= j;
                    }
                    for (auto j = k; j > 1; j--) {
                        denom *= j;
                    }
                    ops += num / denom;
                }
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
    static void flat_exploration(const Dataset<uint64_t> &dataset,
                                 PairList<uint32_t> &pairs,
                                 MaxArray<Result<uint32_t, float>> &maxarray)
    {
        ContingencyTable<uint32_t> ctable(2, dataset.cases_words,
                                          dataset.ctrls_words);
        MutualInformation<float> mi(dataset.cases_count, dataset.ctrls_count);
        Result<uint32_t, float> buffer;
        buffer.combination.resize(2);

        for (auto it = pairs.begin(); it != pairs.end(); ++it) {
            BitTable<uint64_t>::popcnt(dataset.cases[it->first][0],
                                       dataset.ctrls[it->first][0], 3,
                                       dataset.cases[it->second][0],
                                       dataset.ctrls[it->second][0], ctable);
            buffer.combination[0] = it->first;
            buffer.combination[1] = it->second;
            buffer.val = mi.compute(ctable);
            maxarray.add(buffer);
        }
    }

    static void depth_exploration(const Dataset<uint64_t> &dataset,
                                  const unsigned short order,
                                  PairList<uint32_t> &pairs,
                                  MaxArray<Result<uint32_t, float>> &maxarray)
    {
        typedef struct {
            unsigned short size;
            uint32_t pos[];
        } Combination;
        const size_t item_size = sizeof(Combination) + order * sizeof(uint32_t);
        StaticStack<Combination> stack(item_size,
                                       dataset.cases.size() * order - 2);
        Combination *cbuffer = (Combination *)new char[item_size];
        // Auxiliary bit tables for combinations sized under the target order
        std::vector<BitTable<uint64_t>> btables;
        for (auto o = 2; o < order; o++) {
            btables.emplace_back(o, dataset.cases_words, dataset.ctrls_words);
        }
        // Contingency tables (reused for all combinations)
        ContingencyTable<uint32_t> ctable(order, dataset.cases_words,
                                          dataset.ctrls_words);
        MutualInformation<float> mi(dataset.cases_count, dataset.ctrls_count);
        Result<uint32_t, float> rbuffer;
        rbuffer.combination.resize(order);

        const size_t data_lim = dataset.cases.size() - 1;
        size_t i;
        for (auto it = pairs.begin(); it != pairs.end(); ++it) {
            // Init stack with combinations derived from current pair
            BitTable<uint64_t>::fill(btables[0],
                                     dataset.cases[it->first][0],
                                     dataset.ctrls[it->first][0], 3,
                                     dataset.cases[it->second][0],
                                     dataset.ctrls[it->second][0]);
            cbuffer->size = 3;
            cbuffer->pos[0] = it->first;
            cbuffer->pos[1] = it->second;
            for (i = it->second + 1; i < data_lim; i++) {
                cbuffer->pos[2] = i;
                stack.push(*cbuffer);
            }
            // Iterate until the stack is emptied
            while (!stack.empty()) {
                stack.pop(*cbuffer);
                const auto &prev = btables[cbuffer->size - 3];
                const auto &s = cbuffer->pos[cbuffer->size - 1];
                if (cbuffer->size == order) {
                    BitTable<uint64_t>::popcnt(prev.cases, prev.ctrls,
                                               prev.size, dataset.cases[s][0],
                                               dataset.ctrls[s][0], ctable);
                    memcpy(rbuffer.combination.data(), cbuffer->pos, order * 4);
                    rbuffer.val = mi.compute(ctable);
                    maxarray.add(rbuffer);
                } else {
                    BitTable<uint64_t>::fill(
                        btables[cbuffer->size - 2], prev.cases, prev.ctrls,
                        prev.size, dataset.cases[s][0], dataset.ctrls[s][0]);
                    cbuffer->size += 1;
                    for (i = data_lim; i > s; i--) {
                        cbuffer->pos[cbuffer->size - 1] = i;
                        stack.push(*cbuffer);
                    }
                }
            }
        }

        free(cbuffer);
    }

    const unsigned int nthreads;
};

#endif
