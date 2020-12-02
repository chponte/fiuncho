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
 * @file Node_information.h
 * @author Christian Ponte
 * @date 05 Feb 2020
 *
 * @brief Abstract class definition. Node_information provides debug
 * information about the nodes where MPI is running, and the process allocation.
 */

#ifndef FIUNCHO_NODE_INFORMATION_H
#define FIUNCHO_NODE_INFORMATION_H

#include <string>
#include <vector>

class Node_information {
  public:
    Node_information();

    std::string to_string() const;

    const std::string hostname, cpu, mpi_library;
    const long memory;
    const std::vector<int> process_list;

  private:
    static constexpr const char *hardware_id_file = "/proc/net/netlink";
};

#endif
