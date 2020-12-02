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
 * @file Cpu_node_information.cpp
 * @author Christian Ponte
 * @date 05 Feb 2020
 *
 * @brief Cpu_node_information class members implementation.
 */

#include <climits>
#include <cstring>
#include <fiuncho/utils/Node_information.h>
#include <fstream>
#include <mpi.h>
#include <sstream>
#include <unistd.h>

std::string get_hostname() {
    char hostname_buff[HOST_NAME_MAX];
    gethostname(hostname_buff, HOST_NAME_MAX);
    return std::string(hostname_buff);
}

std::string get_cpu_info() {
    constexpr const char *cpu_file = "/proc/cpuinfo";

    // Read complete file
    std::ifstream ifs(cpu_file);
    std::string content((std::istreambuf_iterator<char>(ifs)),
                        (std::istreambuf_iterator<char>()));

    // Find keyword "model name"
    unsigned long beginning, ending, pos = content.find("model name");
    // Parse field
    while (content[pos] != ':') {
        pos++;
    }
    pos += 2;
    beginning = pos;
    while (content[pos] != '\n') {
        pos++;
    }
    ending = pos;
    return content.substr(beginning, ending - beginning);
}

long get_physical_memory() {
    constexpr const char *memory_file = "/proc/meminfo";

    // Read complete file
    std::ifstream ifs(memory_file);
    std::string content((std::istreambuf_iterator<char>(ifs)),
                        (std::istreambuf_iterator<char>()));

    // Find keyword "MemTotal:"
    unsigned long beginning, ending, pos = content.find("MemTotal:");
    pos += 9;
    // Parse field
    while (content[pos] == ' ') {
        pos++;
    }
    beginning = pos;
    while (content[pos] != '\n') {
        pos++;
    }
    ending = pos;
    return atol(content.substr(beginning, ending - beginning).c_str()) * 1024;
}

std::string get_mpi_library_version() {
    int len;
    char mpi_v[MPI_MAX_LIBRARY_VERSION_STRING];
    MPI_Get_library_version(mpi_v, &len);
    return std::string(mpi_v, 0, len);
}

std::vector<int> get_processes(const char *hardware_id_file) {
    // Calculate hardware id for the current host
    std::ifstream ifs(hardware_id_file);
    std::string content((std::istreambuf_iterator<char>(ifs)),
                        (std::istreambuf_iterator<char>()));
    std::hash<std::string> hash_function;
    const size_t hardware_id = hash_function(content);
    // Find out what processes share host
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    std::vector<int> process_list;
    for (auto id = 0; id < size; id++) {
        size_t hid;
        if (id == rank) {
            hid = hardware_id;
        }
        MPI_Bcast(&hid, sizeof(size_t), MPI_BYTE, id, MPI_COMM_WORLD);
        if (hid == hardware_id) {
            process_list.push_back(id);
        }
    }
    return process_list;
}

Node_information::Node_information()
    : hostname(get_hostname()), cpu(get_cpu_info()),
      mpi_library(get_mpi_library_version()), memory(get_physical_memory()),
      process_list(get_processes(hardware_id_file)) {}

std::string Node_information::to_string() const {
    std::string output;
    output += "Hostname: " + hostname + "\n";
    output += "CPU: " + cpu + "\n";
    output += "Memory: " + std::to_string(memory / (1024 * 1024)) + " MB\n";
    output += "MPI Library: " + mpi_library + "\n";
    output += "Processes: ";
    for (auto p = process_list.begin(); p < process_list.end() - 1; p++) {
        output += std::to_string(*p) + ", ";
    }
    output += std::to_string(process_list.back()) + "\n";
    return output;
}
