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
 * @file main.cpp
 * @author Christian Ponte
 * @date 1 March 2018
 *
 * @brief Main program. Initializes the MPI environment, parses the command-line
 * arguments, prints debug information if necessary, instantiates the Search and
 * terminates the execution.
 */

#include <fiuncho/Definitions.h>
#include <fiuncho/MPIEngine.h>
#include <fiuncho/ThreadedSearch.h>
#include <fiuncho/utils/Node_information.h>
#include <fstream>
#include <gflags/gflags.h>
#include <iostream>

DEFINE_uint32(nout, 10, "number of combinations to output");
DEFINE_bool(benchmark, false, "enable benchmarking mode");
DEFINE_uint32(threads, std::thread::hardware_concurrency(),
              "number of threads to use");
DECLARE_uint32(nout);

DEFINE_uint32(order, 3, "combination order to explore");

void read_file_names(const int &argc, char **const &argv, std::string &tped,
                     std::string &tfam, std::string &out)
{
    // Check if tped file was specified
    if (argc < 2) {
        std::cerr << "ERROR: TPED must be set on the commandline" << std::endl;
    }
    // Check if tfam file was specified
    if (argc < 3) {
        std::cerr << "ERROR: TFAM must be set on the commandline" << std::endl;
    }
    // Check if output file was specified
    if (argc < 4) {
        std::cerr << "ERROR: OUT must be set on the commandline" << std::endl;
        exit(1);
    }
    // Check if any extra parameter was given
    if (argc > 4) {
        for (auto i = 4; i < argc; i++) {
            std::cerr << "ERROR: unknown command line parameter '" << argv[i]
                      << "'" << std::endl;
        }
        exit(1);
    }
    // Return file names
    tped = argv[1];
    tfam = argv[2];
    out = argv[3];
}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

#ifdef DEBUG
    // Print the whole command
    for (int i = 0; i < argc; i++) {
        std::cout << argv[i] << " ";
    }
    std::cout << '\n';
#endif

    // Run argument parser
    gflags::SetVersionString(PROJECT_VERSION + '\n' + PROJECT_LICENSE);
    gflags::SetUsageMessage(argv[0] + std::string(" TPED TFAM OUT"));
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    std::string tped, tfam, out;
    read_file_names(argc, argv, tped, tfam, out);

#ifdef DEBUG
    // Print node information
    Node_information info;
    std::cout << info.to_string() << '\n';
#endif

    // Execute search
    try {
        MPIEngine engine;
        auto results = engine.run<ThreadedSearch>(
            tped, tfam, (unsigned int)FLAGS_order, (unsigned int)FLAGS_nout,
            (unsigned int)FLAGS_threads);
        if (rank == 0) {
            // Write results to the output file
            std::ofstream of(out.c_str(), std::ios::out);
            for (auto r : results) {
                of << r.str() << '\n';
            }
            of.close();
        }
    } catch (const std::runtime_error &e) {
        std::cerr << e.what() << '\n';
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    MPI_Finalize();
    gflags::ShutDownCommandLineFlags();
    return 0;
}
