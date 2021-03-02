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
 * @file Search.h
 * @author Christian Ponte
 * @date 23 May 2018
 *
 * @brief Search class definition.
 */

#ifndef FIUNCHO_MPIENGINE_H
#define FIUNCHO_MPIENGINE_H

#include <fiuncho/engine/Result.h>
#include <fiuncho/engine/Search.h>
#include <mpi.h>
#include <sstream>
#include <vector>

#ifdef BENCHMARK
#include <iostream>
#endif

class MPIEngine
{
  public:
    MPIEngine() : mpi_size(get_mpi_size()), mpi_rank(get_mpi_rank()) {}

    template <typename T, typename... Args>
    std::vector<Result<uint32_t, float>>
    run(const std::string &tped, const std::string &tfam,
        const unsigned int order, const unsigned int outputs, Args &&...args)
    {
        std::vector<Result<uint32_t, float>> local_results, global_results;
#ifdef BENCHMARK
        double function_time, dataset_time;
        function_time = MPI_Wtime();
        dataset_time = MPI_Wtime();
#endif
#ifdef ALIGN
        const Dataset<uint64_t> dataset =
            Dataset<uint64_t>::read<ALIGN>(tped, tfam);
#else
        auto dataset = Dataset<uint64_t>::read(tped, tfam);
#endif
#ifdef BENCHMARK
        dataset_time = MPI_Wtime() - dataset_time;
        std::cout << "Read " << dataset.cases.size() << " SNPs from "
                  << dataset.inds_count << " individuals in " << dataset_time
                  << " seconds\n";
#endif
        const Distributor<uint32_t> distributor(dataset.snps, mpi_size,
                                                mpi_rank);
        Search *search = new T(std::forward<Args>(args)...);
        local_results = search->run(dataset, order, distributor, outputs);
        delete search;
        // Serialize the results
        std::stringstream oss;
        for (auto r : local_results) {
            Result<uint32_t, float>::serialize(oss, r);
        }
        std::string local_buff = oss.str(), global_buff;

        // Gather the results in 0
        if (mpi_rank == 0) {
            global_buff.resize(local_buff.size() * mpi_size);
        }
        MPI_Gather(local_buff.data(), local_buff.size(), MPI_BYTE,
                   (void *)global_buff.data(), local_buff.size(), MPI_BYTE, 0,
                   MPI_COMM_WORLD);

        // Deserialize the results
        if (mpi_rank == 0) {
            oss.str(global_buff);
            global_results.resize(local_results.size() * mpi_size);
            for (auto i = 0; i < local_results.size() * mpi_size; i++) {
                Result<uint32_t, float>::deserialize(oss, global_results[i]);
            }
            // Sort the result
            std::sort(global_results.rbegin(), global_results.rend());
            global_results.resize(outputs);
        }

#ifdef BENCHMARK
        function_time = MPI_Wtime() - function_time;
        std::cout << "Total elapsed time: " << function_time << '\n';
#endif

        return global_results;
    }

  private:
    unsigned int get_mpi_size()
    {
        int size;
        MPI_Comm_size(MPI_COMM_WORLD, &size);
        return size;
    }

    unsigned int get_mpi_rank()
    {
        int rank;
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        return rank;
    }

    const unsigned int mpi_rank;
    const unsigned int mpi_size;
};

#endif
