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

find_package(AVX)

################################# Definitions  #################################

file(GLOB_RECURSE HEADER_LIST "${PROJECT_SOURCE_DIR}/include/fiuncho/*.h")
set(SOURCE_LIST
    "${PROJECT_SOURCE_DIR}/src/utils/Node_information.cpp")
set(SOURCE_LIST_BASE
    "${PROJECT_SOURCE_DIR}/src/cpu/base/BitTable.cpp"
    "${PROJECT_SOURCE_DIR}/src/cpu/base/ContingencyTable.cpp"
    "${PROJECT_SOURCE_DIR}/src/cpu/base/MutualInformation.cpp")
set(SOURCE_LIST_256_AVX2
    "${PROJECT_SOURCE_DIR}/src/cpu/256_avx2/BitTable.cpp"
    "${PROJECT_SOURCE_DIR}/src/cpu/256_avx2/ContingencyTable.cpp"
    "${PROJECT_SOURCE_DIR}/src/cpu/256_avx2/MutualInformation.cpp")
set(SOURCE_LIST_256_AVX512BW
    "${PROJECT_SOURCE_DIR}/src/cpu/256_avx2/BitTable.cpp"
    "${PROJECT_SOURCE_DIR}/src/cpu/256_avx2/ContingencyTable.cpp"
    "${PROJECT_SOURCE_DIR}/src/cpu/256_avx512bw/MutualInformation.cpp")
set(SOURCE_LIST_512_AVX512BW
    "${PROJECT_SOURCE_DIR}/src/cpu/512_avx512bw/BitTable.cpp"
    "${PROJECT_SOURCE_DIR}/src/cpu/512_avx512bw/ContingencyTable.cpp"
    "${PROJECT_SOURCE_DIR}/src/cpu/512_avx512bw/MutualInformation.cpp")

################################ Configuration  ################################

# AVX operation width
if ((AVX512BW_ENABLED OR AVX2_ENABLED) AND NOT DEFINED AVX_WIDTH)
    set(AVX_WIDTH 256 CACHE STRING "Preferred AVX operation width" FORCE)
    set_property(CACHE AVX_WIDTH PROPERTY STRINGS "256" "512")
endif()

################################### Targets  ###################################

configure_file(${PROJECT_SOURCE_DIR}/include/fiuncho/Definitions.h.in
    ${PROJECT_BINARY_DIR}/include/fiuncho/Definitions.h)

if(AVX512BW_ENABLED)
    if(AVX_WIDTH EQUAL 256)
        add_library(libfiuncho
            ${SOURCE_LIST}
            ${SOURCE_LIST_256_AVX512BW})
        target_compile_options(libfiuncho PUBLIC "-DALIGN=32")
    elseif(AVX_WIDTH EQUAL 512)
        add_library(libfiuncho
            ${SOURCE_LIST}
            ${SOURCE_LIST_512_AVX512BW})
        target_compile_options(libfiuncho PUBLIC "-DALIGN=64")
    else()
        message(FATAL_ERROR "Invalid AVX operation width")
    endif()
elseif(AVX2_ENABLED)
    if(AVX_WIDTH EQUAL 256)
        add_library(libfiuncho
            ${SOURCE_LIST}
            ${SOURCE_LIST_256_AVX2})
        target_compile_options(libfiuncho PUBLIC "-DALIGN=32")
    else()
        message(FATAL_ERROR "Invalid AVX operation width, note that only AVX2 "
        "was enabled during compilation")
    endif()
else()
    add_library(libfiuncho
        ${SOURCE_LIST}
        ${SOURCE_LIST_BASE})
    target_compile_options(libfiuncho PUBLIC)
endif()

target_include_directories(libfiuncho PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_BINARY_DIR}/include)
target_link_libraries(libfiuncho PUBLIC
    Threads::Threads
    MPI::MPI_CXX)
