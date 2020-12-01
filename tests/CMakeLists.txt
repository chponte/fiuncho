# This file is part of Fiuncho.
# Copyright (C) 2020 by Christian Ponte
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

find_package(GTest)
if(GTEST_FOUND)
    link_libraries(GTest::Main)
    macro(create_gtest test_name source_file)
        add_executable(${test_name} ${source_file})
        if(${ARGC} GREATER 2)
            gtest_add_tests(TARGET ${test_name}
                SOURCES ${source_file}
                EXTRA_ARGS ${ARGN})
        else()
            gtest_add_tests(TARGET ${test_name}
                SOURCES ${source_file})
        endif()
    endmacro()
else()
    include(FetchContent)

    FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        release-1.10.0
    )

    FetchContent_GetProperties(googletest)
    if(NOT googletest_POPULATED)
        FetchContent_Populate(googletest)
        # Disable GMock
        option(BUILD_GMOCK "Builds the googlemock subproject" OFF)
        add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})
        link_libraries(gtest_main)
    endif()

    macro(create_gtest test_name source_file)
        add_executable("${test_name}_bin" ${source_file})
        if (${ARGC} GREATER 2)
            add_test(NAME ${test_name} COMMAND "${test_name}_bin"
                ${ARGN})
        else()
            add_test(NAME ${test_name} COMMAND "${test_name}_bin")
        endif()
    endmacro()
endif()

################################### Targets  ###################################

link_libraries(libfiuncho)

create_gtest(test_dataset dataset.cpp
    "${CMAKE_CURRENT_LIST_DIR}/data/test.tped"
    "${CMAKE_CURRENT_LIST_DIR}/data/test.tfam")
create_gtest(test_bittable bittable.cpp)
create_gtest(test_mi mi.cpp)