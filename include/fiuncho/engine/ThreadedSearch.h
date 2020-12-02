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
#include <fiuncho/engine/Search.h>
#include <fiuncho/engine/algorithms/MutualInformation.h>
#include <fiuncho/utils/MaxArray.h>
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
                                             Distributor<uint32_t> distributor,
                                             const unsigned int outputs)
    {
        int i;
        // Spawn threads
        std::vector<std::thread> threads;
        std::vector<MaxArray<Result<uint32_t, float>>> maxarrays;
        std::vector<std::vector<std::pair<uint32_t, uint32_t>>> pairs_vector;
#ifdef BENCHMARK
        std::vector<std::chrono::high_resolution_clock::time_point>
            thread_timers;
#endif
        // Pre-reserve space to avoid reallocating the underlying array, which
        // results in an error since previous addresses are rendered incorrect
        maxarrays.reserve(nthreads);
        pairs_vector.reserve(nthreads);
        for (i = 0; i < nthreads; i++) {
            maxarrays.emplace_back(outputs);
            pairs_vector.emplace_back();
            distributor.layer(nthreads, i).get_pairs(pairs_vector.back());
#ifdef BENCHMARK
            thread_timers.push_back(std::chrono::high_resolution_clock::now());
#endif
            threads.emplace_back(thread_main, std::ref(dataset),
                                 std::ref(pairs_vector.back()),
                                 std::ref(maxarrays.back()));
        }

        std::vector<Result<uint32_t, float>> results;
        results.reserve(nthreads * outputs);\
        // Wait for the completion of all threads
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
                unsigned long ops = 0;
                for (auto p : pairs_vector[i]) {
                    ops += dataset.cases.size() - p.second - 1;
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

  private:
    static void
    thread_main(const Dataset<uint64_t> &dataset,
                const std::vector<std::pair<uint32_t, uint32_t>> &pairs,
                MaxArray<Result<uint32_t, float>> &maxarray)
    {
        BitTable<uint64_t> btable(2, dataset.cases_words, dataset.ctrls_words);
        ContingencyTable<uint32_t> ctable(3, dataset.cases_words,
                                          dataset.ctrls_words);
        MutualInformation<float> mi(dataset.cases_count, dataset.ctrls_count);

        const auto data_lim = dataset.cases.size();
        for (auto p = pairs.rbegin(); p != pairs.rend(); p++) {
            BitTable<uint64_t>::fill(
                btable, dataset.cases[p->first][0], dataset.ctrls[p->first][0],
                3, dataset.cases[p->second][0], dataset.ctrls[p->second][0]);
            for (auto i = p->second + 1; i < data_lim; i++) {
                BitTable<uint64_t>::popcnt(btable.cases, btable.ctrls, 9,
                                           dataset.cases[i][0],
                                           dataset.ctrls[i][0], ctable);
                maxarray.add(Result<uint32_t, float>(
                    std::vector<uint32_t>{p->first, p->second, i},
                    mi.compute(ctable)));
            }
        }
    }

    const unsigned int nthreads;
};

#endif
