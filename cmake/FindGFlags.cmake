# - Try to find gflags
#   Use CMake variable GFLAGS_ROOT if gflags is in an unusual location
#
# The following are set after configuration is done:
#  GFLAGS_FOUND
#  GFLAGS_INCLUDE_DIR
#  GFLAGS_LIBRARY

include(FindPackageHandleStandardArgs)

if (NOT DEFINED GFLAGS_ROOT)
    set(GFLAGS_ROOT "" CACHE PATH "Directory containing the gflags library")
endif()

find_library(GFLAGS_LIBRARY
    NAMES gflags libgflags)
mark_as_advanced(GFLAGS_LIBRARY)

find_path(GFLAGS_INCLUDE_DIR
    NAMES gflags.h
    HINTS ${GFLAGS_ROOT}
    PATH_SUFFIXES "include" "gflags")
# Get parent folder
get_filename_component(GFLAGS_INCLUDE_DIR ${GFLAGS_INCLUDE_DIR} DIRECTORY)
mark_as_advanced(GFLAGS_INCLUDE_DIR)

find_package_handle_standard_args(GFlags DEFAULT_MSG
    GFLAGS_INCLUDE_DIR GFLAGS_LIBRARY)
