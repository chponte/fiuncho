# This file is part of Fiuncho.
#
# Fiuncho is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Fiuncho is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Fiuncho. If not, see <http://www.gnu.org/licenses/>.

################################# Dependencies #################################

find_package(Threads)
if (NOT TARGET Threads::Threads)
    message(FATAL_ERROR "Missing threading library")
endif()

find_package(MPI)
if (NOT TARGET MPI::MPI_CXX)
    message(FATAL_ERROR "Missing MPI library")
endif()

################################# Definitions  #################################

file(GLOB_RECURSE HEADER_LIST "${PROJECT_SOURCE_DIR}/include/fiuncho/*.h")
set(SOURCE_LIST
    "${PROJECT_SOURCE_DIR}/src/utils/Node_information.cpp")
set(SOURCE_LIST_BASE
    "${PROJECT_SOURCE_DIR}/src/cpu/base/BitTable.cpp"
    "${PROJECT_SOURCE_DIR}/src/cpu/base/ContingencyTable.cpp"
    "${PROJECT_SOURCE_DIR}/src/cpu/base/MutualInformation.cpp")

################################ Configuration  ################################

################################### Targets  ###################################

configure_file(${PROJECT_SOURCE_DIR}/include/fiuncho/Definitions.h.in
    ${PROJECT_BINARY_DIR}/include/fiuncho/Definitions.h)

add_library(libfiuncho
    ${SOURCE_LIST}
    ${SOURCE_LIST_BASE})
target_compile_options(libfiuncho PUBLIC)

target_include_directories(libfiuncho PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_BINARY_DIR}/include)
target_link_libraries(libfiuncho PUBLIC
    Threads::Threads
    MPI::MPI_CXX)
