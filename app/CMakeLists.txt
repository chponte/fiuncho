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

find_package(GFlags)

if(GFLAGS_FOUND)
    include_directories(${GFLAGS_INCLUDE_DIR})
    link_libraries(${GFLAGS_LIBRARY})
else()
    # Unset variables created during the find_package call to avoid conflicts
    unset(GFlags_FOUND)
    unset(GFlags_FOUND CACHE)
    unset(GFLAGS_FOUND)
    unset(GFLAGS_FOUND CACHE)
    unset(GFLAGS_INCLUDE_DIR)
    unset(GFLAGS_INCLUDE_DIR CACHE)
    unset(GFLAGS_LIBRARY)
    unset(GFLAGS_LIBRARY CACHE)
    unset(GFLAGS_ROOT)
    unset(GFLAGS_ROOT CACHE)
    include(FetchContent)
    FetchContent_Declare(gflags
        GIT_REPOSITORY https://github.com/gflags/gflags.git
        GIT_TAG v2.2.2)
    FetchContent_GetProperties(gflags)
    if(NOT gflags_POPULATED)
        FetchContent_Populate(gflags)
        add_subdirectory(${gflags_SOURCE_DIR} ${gflags_BINARY_DIR}
            EXCLUDE_FROM_ALL)
        # Disable help messages from parameters defined inside the library
        get_target_property(gflags_target gflags::gflags ALIASED_TARGET)
        target_compile_definitions(${gflags_target} PRIVATE
            -D STRIP_FLAG_HELP=1)
    endif()
    link_libraries(gflags::gflags)
endif()

################################### Targets  ###################################

# Add all available targets
add_executable(fiuncho main.cpp)
target_link_libraries(fiuncho libfiuncho)
