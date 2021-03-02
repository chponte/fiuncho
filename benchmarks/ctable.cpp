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

#include <cstdint>
#include <fiuncho/dataset/Dataset.h>
#include <fiuncho/engine/BitTable.h>
#include <fiuncho/engine/ContingencyTable.h>
#include <iostream>
#include <pthread.h>
#include <thread>
#include <time.h>
#include <vector>

/*
 *  This benchmark programs calculates contingency tabless of a specific order
 *  following this process:
 *      1. Spawn as many threads as indicated
 *      2. Set processor affinity as indicated
 *      3. Initialize previous tables and such, and synchronize threads at the
 *         end of the initialization
 *      4. Warm up the CPU core by running an additional 10% of the total
 *         iterations
 *      5. Measure contingency table computation time
 *      6. Print elapsed time on each thread
 *
 *  Program arguments:
 *      1: Comma-separated list of hardware threads to run on.
 *      2: Order of the tables to benchmark
 *      3: How many times the main loop is repeated
 *      4: Path to the TPED input file
 *      5: Path to the TFAM input file
 */

unsigned int repetitions;

unsigned short thread_count;
pthread_barrier_t barrier;

std::vector<int> split_into_ints(const std::string &s, const char sep)
{
    std::vector<int> ints;
    size_t pos = s.find(sep, 0), prev = 0;
    while (pos != std::string::npos) {
        ints.push_back(atoi(s.substr(prev, pos - prev).c_str()));
        prev = pos + 1;
        pos = s.find(sep, pos + 1);
    }
    ints.push_back(atoi(s.substr(prev).c_str()));
    return ints;
}

void bench(const int tid, const std::string tped, const std::string tfam,
           const unsigned short order, double &elapsed_time, const int affinity)
{
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(affinity, &cpuset);
    int rc = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
    if (rc != 0) {
        std::cerr << "Error calling pthread_setaffinity_np: " << rc << "\n";
    }

#ifdef ALIGN
    const Dataset<uint64_t> &dataset =
        Dataset<uint64_t>::read<ALIGN>(tped, tfam);
#else
    const Dataset<uint64_t> &dataset = Dataset<uint64_t>::read(tped, tfam);
#endif

    const size_t snp_count = dataset.cases.size();
    ContingencyTable<uint32_t> ctable(order, dataset.cases_words,
                                      dataset.ctrls_words);
    struct timespec start, end;

    if (order == 2) {
        // Main compute loop for order == 2
        pthread_barrier_wait(&barrier);
        // Warmup CPU adding an extra 10% of iterations before measuring time
        for (auto reps = 0; reps < repetitions / 10; reps++) {
            for (auto snp = 1; snp < snp_count; snp++) {
                BitTable<uint64_t>::popcnt(
                    dataset.cases[0][0], dataset.ctrls[0][0], 3,
                    dataset.cases[snp][0], dataset.ctrls[snp][0], ctable);
            }
        }
        // Measure time
        clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start);
        for (auto reps = 0; reps < repetitions; reps++) {
            for (auto snp = 1; snp < snp_count; snp++) {
                BitTable<uint64_t>::popcnt(
                    dataset.cases[0][0], dataset.ctrls[0][0], 3,
                    dataset.cases[snp][0], dataset.ctrls[snp][0], ctable);
            }
        }
        clock_gettime(CLOCK_THREAD_CPUTIME_ID, &end);
        elapsed_time = end.tv_sec + end.tv_nsec * 1E-9 - start.tv_sec -
                       start.tv_nsec * 1E-9;
    } else {
        // Initialize previous bit tables outside of the main loop
        std::vector<BitTable<uint64_t>> prev_tables;
        prev_tables.emplace_back(2, dataset.cases_words, dataset.ctrls_words);
        BitTable<uint64_t>::fill(prev_tables[0], dataset.cases[0][0],
                                 dataset.ctrls[0][0], 3, dataset.cases[1][0],
                                 dataset.ctrls[1][0]);
        for (auto o = 3; o < order; o++) {
            prev_tables.emplace_back(o, dataset.cases_words,
                                     dataset.ctrls_words);
            BitTable<uint64_t>::fill(
                prev_tables[o - 2], prev_tables[o - 3].cases,
                prev_tables[o - 3].ctrls, prev_tables[o - 3].size,
                dataset.cases[o - 1][0], dataset.ctrls[o - 1][0]);
        }
        const BitTable<uint64_t> &last = prev_tables[order - 3];
        // Main compute loop for order > 2
        pthread_barrier_wait(&barrier);
        // Warmup CPU adding an extra 10% of iterations before measuring time
        for (auto reps = 0; reps < repetitions / 10; reps++) {
            for (auto snp = order - 1; snp < snp_count; snp++) {
                BitTable<uint64_t>::popcnt(last.cases, last.ctrls, last.size,
                                           dataset.cases[snp][0],
                                           dataset.ctrls[snp][0], ctable);
            }
        }
        // Measure time
        clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start);
        for (auto reps = 0; reps < repetitions; reps++) {
            for (auto snp = order - 1; snp < snp_count; snp++) {
                BitTable<uint64_t>::popcnt(last.cases, last.ctrls, last.size,
                                           dataset.cases[snp][0],
                                           dataset.ctrls[snp][0], ctable);
            }
        }
        clock_gettime(CLOCK_THREAD_CPUTIME_ID, &end);
        elapsed_time = end.tv_sec + end.tv_nsec * 1E-9 - start.tv_sec -
                       start.tv_nsec * 1E-9;
    }
}

int main(int argc, char *argv[])
{
    if (argc != 6) {
        std::cout << argv[0] << " <THREADS> <ORDER> <REPETITIONS> <TPED> <TFAM>"
                  << std::endl;
        return 0;
    }

    // Initialization
    // Arguments
    std::vector<int> affinity = split_into_ints(std::string(argv[1]), ',');
    thread_count = affinity.size();
    const unsigned short order = atoi(argv[2]);
    repetitions = atoi(argv[3]);
    const std::string tped = argv[4], tfam = argv[5];
    // Variables
    pthread_barrier_init(&barrier, NULL, thread_count);
    std::vector<std::thread> threads;
    std::vector<double> times;
    times.resize(thread_count);

    // Spawn thread_count - 1 threads
    for (auto i = 1; i < affinity.size(); i++) {
        threads.emplace_back(bench, i, tped, tfam, order, std::ref(times[i]),
                             affinity[i]);
    }
    // Also use current thread
    bench(0, tped, tfam, order, times[0], affinity[0]);

    // Finalization
    // Wait for completion
    for (auto &thread : threads) {
        thread.join();
    }
    // Print times
    for (auto i = 0; i < thread_count - 1; i++) {
        std::cout << times[i] << ',';
    }
    std::cout << times[thread_count - 1] << '\n';

    return 0;
}
