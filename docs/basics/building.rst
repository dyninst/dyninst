.. _`sec-building`:

Building Dyninst
################

Docker Containers
*****************

Containers are provided that can be used for Dyninst development (e.g., make changes to Dyninst and quickly rebuild it)
or for development of your own tools (e.g., have a container ready to go with Dyninst). All containers are available
through the Github package repository: https://github.com/orgs/dyninst/packages.


Install with Spack
******************

`Spack <https://spack.readthedocs.io>`_ is a package management tool designed to support
multiple versions and configurations of software on a wide variety of platforms and environments.
By default, Spack will build from source all the dependencies needed by Dyninst.

``spack install dyninst``

Build from source
*****************

Dyninst uses CMake to build itself. The minimum supported CMake version is 3.14.0.

1. Fetch the source from `GitHub <https://github.com/dyninst/dyninst>`_.

2. Configure Dyninst with CMake

   ``cmake /path/to/dyninst/source -DCMAKE_INSTALL_PREFIX=/path/to/installation``

3. Build and install Dyninst in parallel

   ``cmake --install . --parallel``

......

CMake Build Options
===================

USE_OpenMP
  Use OpenMP for parallel parsing

  *default*: **ON**


SW_ANALYSIS_STEPPER
  Use ParseAPI-based analysis stepper in Stackwalker

  *default*: **ON**

BUILD_RTLIB_32
  Build 32-bit runtime library on mixed 32/64 systems

  *default*: **OFF**

ENABLE_DEBUGINFOD
  Enable debuginfod support

  *default*: **OFF**

ADD_VALGRIND_ANNOTATIONS
  Enable annotations for Valgrind analysis

  *default*: **OFF**

ENABLE_STATIC_LIBS
  Build static libraries

  *default*: **OFF**

DYNINST_DISABLE_DIAGNOSTIC_SUPPRESSIONS
  Disable all warning suppressions and frame size overrides.

  *default*: **OFF**

DYNINST_EXTRA_WARNINGS
  Additional warning options to enable if available.

  Use a semicolon-separated list without a leading '-' (Wopt1[;Wopt2]...).
  For example ``Wswitch;Wsign-conversion``.

  *default*: empty string

DYNINST_WARNINGS_AS_ERRORS
  Treat compilation warnings as errors

  *default*: **OFF**

......

Options for advanced users
--------------------------

DYNINST_ENABLE_LTO
  Enable Link-Time Optimization

  *default*: **OFF**

DYNINST_LINKER
  The linker to use

  *default*: Use what the compiler defaults to

DYNINST_CXXSTDLIB
  The C++ standard library to use; only affects LLVM-based compilers

  *default*: libstdc++

DYNINST_FORCE_RUNPATH
  Require the use of RUNPATH instead of compiler's default

  *default*: **OFF**

