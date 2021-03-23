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
#include <fiuncho/ThreadedSearch.h>
#include <fiuncho/utils/MaxArray.h>
#include <iostream>
#include <thread>
#include <time.h>
#include <vector>

/*
 *  This benchmark programs calculates bit tables of a specific order following
 *  this process:
 *      1. Spawn as many threads as indicated
 *      2. Set processor affinity as indicated
 *      3. Initialize previous tables and such, and synchronize threads at the
 *         end of the initialization
 *      4. Warm up the CPU core by running an additional 10% of the total
 *         iterations
 *      5. Measure bit table computation time
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

    struct timespec start, end;

    // Init input data
#ifdef ALIGN
    const Dataset<uint64_t> &dataset =
        Dataset<uint64_t>::read<ALIGN>(tped, tfam);
#else
    const Dataset<uint64_t> &dataset = Dataset<uint64_t>::read(tped, tfam);
#endif
    PairList<uint32_t> pairs(dataset.snps, dataset.snps, 0, 1);
    MaxArray<Result<uint32_t, float>> maxarray(10);

    // Wait for all threads in order to start at the same time
    pthread_barrier_wait(&barrier);
    // Meassure search time
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start);
    ThreadedSearch::thread_main(dataset, order, pairs, maxarray);
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &end);
    elapsed_time =
        end.tv_sec + end.tv_nsec * 1E-9 - start.tv_sec - start.tv_nsec * 1E-9;
}

int main(int argc, char *argv[])
{
    if (argc != 5) {
        std::cout << argv[0] << " <THREADS> <ORDER> <TPED> <TFAM>" << std::endl;
        return 0;
    }

    // Initialization
    // Arguments
    std::vector<int> affinity = split_into_ints(std::string(argv[1]), ',');
    thread_count = affinity.size();
    const unsigned short order = atoi(argv[2]);
    const std::string tped = argv[3], tfam = argv[4];
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
