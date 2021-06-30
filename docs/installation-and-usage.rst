==========================================
Installation and usage
==========================================

Fiuncho is distributed as a group of source files that the user is required to
compile into a binary before its use. This sections describes the process of
downloading the source code, building the application and using its command line
interface.

For this manual, we assume a standard Linux environment where all dependencies can be downloaded through the distribution's repositories.

------------------------------------------
Downloading the source files
------------------------------------------

All Fiuncho releases can be downloaded from the Github repository
https://github.com/chponte/fiuncho, listed under the Releases tab.

------------------------------------------
Configuring and building the CMake project
------------------------------------------

Fiuncho uses the CMake build system to configure and compile the final
application.

The requirements to build Fiuncho are:

*  Git.
*  CMake (>=3.11).
*  A C++ compiler. GCC (>=8.3.0) and Clang (>=10.0.0) with glibc (>=2.22) are
   supported. The Intel C/C++ Compiler (>=19.0) is also supported.
*  An MPI library. OpenMPI (>=4.0.0), MPICH (>=3.2) and Intel MPI (>=19.0) are
   supported.

.. TIP::
    Older versions of the compiler and system libraries can work fine, but the
    performance may not be optimal. Use at your own risk.

If the requirements are met, Fiuncho can be configured and built with the
following commands:

.. code-block:: shell-session

   mkdir build
   cd build
   CFLAGS="-O3 -march=native" CXXFLAGS=$CFLAGS cmake ..
   make fiuncho

Compilation flags can be passed to the compiler using the `CFLAGS` and
`CXXFLAGS` environmental variables, as indicated in the example. Specifying an
optimization level 3 (with `-O3`) and a target compilation architecture (with
`-march`) is highly recommended. If you are building Fiuncho to be run on a
different machine than the current one, replace the `native` target architecture
with the appropriate one.

More advanced configurations are also possible through CMake's project
variables. In addition to the default CMake variables, this project introduces
the following variables:

CMAKE_BUILD_TYPE
  The default CMake variable to select a build configuration. Accepted values
  are `Debug`, `DebWithRelInfo`, `Release` and `Benchmark`.

FORCE_AVX512F512
  Force CMake to build Fiuncho using the AVX Intrinsics implementation using 512
  bit operations from the AVX512F extension. Accepted values are `ON` and `OFF`.
  This option is incompatible with any other `FORCE_` option.

FORCE_AVX512F256
  Force CMake to build Fiuncho using the AVX Intrinsics implementation using 256
  bit operations from the AVX512F extension. Accepted values are `ON` and `OFF`.
  This option is incompatible with any other `FORCE_` option.

FORCE_AVX2
  Force CMake to build Fiuncho using the AVX Intrinsics implementation using 256
  bit operations from the AVX2 extension. Accepted values are `ON` and `OFF`.
  This option is incompatible with any other `FORCE_` option.

FORCE_NOAVX
  Force CMake to build Fiuncho not using any of the AVX Intrinsics
  implementations. Accepted values are `ON` and `OFF`. This option is
  incompatible with any other `FORCE_` option.

------------------------------------------
Command line usage
------------------------------------------

Fiuncho can be invoked as follows::

   fiuncho [-h] [--version] [-n <integer>]
           [-t <integer>] -o <integer>
           tped tfam output


Note that fiuncho is an MPI program, and as such it should be called through
`mpiexec` or any other parallel job launcher such as `srun` from SLURM. If you
need help with launching an MPI program, please refer to the MPI or job
scheduling system documentation instead.

^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Named arguments
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

-o, --order
    **Required.** Integer equal or greater than 2 specifying the order of the
    epistasis interactions to explore during the search.

-t, --threads
    Number of threads for the process to use during the search. Note that if you
    are running an MPI job with multiple processes, each process will create the
    same number of thread. If it's not specified, fiuncho will use as many
    threads as physical cores are available to the process.

-n, --noutputs
    Integer greater than 0 indicating the number of combinations to output. If
    it's not specified, it will output 10 combinations.

-h, --help
    Displays usage information and exits.

--version
    Displays version information and exits.

^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Positional arguments
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

tped
    **Required.** First positional argument indicating the path to the tped data
    file.
tfam
    **Required.** Second positional argument indicating the path to the tfam
    data file.
output
    **Required.** Third positional argument indicating the path to the output
    file.


