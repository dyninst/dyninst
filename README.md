# Dyninst

## Branch states

| Branch                                  | Status        | Notes                                              |
| --------------------------------------- |:-------------:|:--------------------------------------------------:|
| master                                  | stable        | See below                                          |
| aarch32                                 | experimental  | Contact Ray Chen (rchen at cs dot umd dot edu)     |

## Notes

* Known issues should have open issues associated with them.
* ARMv8 (64 bit) support for dynamic instrumentation is experimental and incomplete.
  For more details about current supported functionality refer to [Dyninst Support for the ARMv8 (64 bit)](https://github.com/dyninst/dyninst/wiki/DyninstAPI-ARMv8-status).

## Build DyninstAPI and its subcomponents

### Install with Spack

```spack install dyninst```

### Build from source (the short version)

1. Configure Dyninst with CMake

```ccmake /path/to/dyninst/source```

2. Build and install Dyninst in parallel

```make install -jN```

If this short version does not work for you, please refer to the long version and the FAQs below.

### Build from source (the long version)

#### Configuration

Dyninst is built via CMake. We require CMake 3.0.0 as a minimum on all systems. CMake will automatically
search for dependencies and download them when not found. The main dependencies of Dyninst include:

1. elfutils, 0.173 minimum, https://sourceware.org/elfutils/

2. Boost, 1.61.0 or later recommended, https://www.boost.org/

3. The Intel Thread Building Blocks (TBB), 2018 U6 or later recommended, https://www.threadingbuildingblocks.org/

4. OpenMP, optional for parallel code parsing. Note that Dyninst will not automatically download or install OpenMP

We recommend performing an interactive
configuration with "ccmake /path/to/dyninst/source" first, in order to see which options are
relevant for your system. You may also perform a batch configuration
with "cmake /path/to/dyninst/source".  Options are passed to CMake with -DVAR=VALUE. Common
options include:

```PATH_BOOST```: base directory of your boost installation

```LibElf_INCLUDE_DIR```: the location of elf.h and libelf.h

```LibElf_LIBRARIES```: full path of libelf.so

```LibDwarf_INCLUDE_DIR```: location of libdw.h

```LibDwarf_LIBRARIES```: full path of libdw.so

```TBB_INCLUDE_DIRS```: the direcory of the include files of TBB

```TBB_tbb_LIBRARY_DEBUG```: full path of libtbb_debug.so

```TBB_tbb_LIBRARY_RELEASE```: full path of libtbb.so

```CMAKE_BUILD_TYPE```: may be set to Debug, Release, or RelWithDebInfo for unoptimized, optimized, and optimized with debug information builds respectively. Note that RelWithDebInfo is the default.

```CMAKE_INSTALL_PREFIX```: like PREFIX for autotools-based systems. Where to install things.

```USE_OpenMP```: whether or not use OpenMP for parallel parsing. Default to be ```ON```

```ENABLE_STATIC_LIBS```: also build dyninst in static libraries. Default to be ```NO```. Set to ```YES``` to build static libraries. 

CMake's default generator on Linux is normally "Unix Makefiles", and
on Windows, it will normally produce project files for the most recent
version of Visual Studio on your system. Other generators should work
but are not tested. After the CMake step concludes, you will have
appropriate Makefiles or equivalent and can build Dyninst.

If you are cross-compiling Dyninst, including builds for
various Cray and Intel MIC systems, you will either need a toolchain
file that specifies how to properly cross-compile, or you will need to
manually define the appropriate compiler, library locations, include
locations, and the CROSS_COMPILING flag so that the build system will
properly evaluate what can be built and linked in your environment.

#### Building and installing
CMake allows Dyninst to be built out-of-source; simply invoke CMake in your desired build location. In-source builds are still fully supported as well.
Each component of Dyninst may be built independently: cd $component; make. Standard make options will work; we fully support parallel compilation for make -jN. Setting VERBOSE=1 will replace the beautified CMake output with raw commands and their output, which can be useful for troubleshooting.

On Windows, you will need the Debug Information Access (DIA) SDK, which should be available with an MSDN subscription, in order to build Dyninst; you will not need libelf, libdwarf, binutils, or the GCC demangler. Dyninst is still built via CMake, and the NMake and Visual Studio project file generators should both work. We have not tested building Dyninst on Windows with gcc, and we do not expect this to work presently.

If you wish to import
Dyninst into your own CMake projects, the export information is in
`CMAKE_INSTALL_PREFIX/INSTALL_CMAKE_DIR`. PDF documentation is included
and installed to `CMAKE_INSTALL_PREFIX/INSTALL_DOC_DIR`. If you update
the LaTeX source documents for any manuals, "make doc" will rebuild
them. Components may be built and installed individually: "make
$COMPONENT" and "make $COMPONENT-install" respectively; this will
appropriately respect inter-component dependencies.

### FAQs of building Dyninst

1. Q: What should I do if the build failed and showed

```"/lib64/libdw.so.1: version `ELFUTILS_0.173' not found```

or

```error: 'dwarf_next_lines' was not declared in this scope```

A: Dyninst now depends on elfutils-0.173 or later. If you are seeing above errors, it means the elfutils installed on your system is older than 0.173. We recommend that you set ```LibElf_INCLUDE_DIR```, ```LibElf_LIBRARIES```, ```LibDwarf_INCLUDE_DIR```, and ```LibDwarf_LIBRARIES``` to empty, which will trigger the CMake to automatically download the correct version of elfutils. Or you can upgrade your elfutils with your system package manager.

2. Q: Where are the dependency libraries downloaded by Dyninst?

A: After installation, Dyninst should copy all dependencies to the install location. In case where the dependencies are not copied, they can be found in the directory where you build Dyninst. For example, suppose you configure and build Dyninst at: ```/home/user/dyninst/build```

Then elfutils, TBB, boost can be found at ```/home/user/dyninst/build/elfutils/```, ```/home/user/dyninst/build/tbb/```, ```/home/user/dyninst/build/boost/```, respectively.

3. Q: My system has pre-installed Boost, but the build failed due to that cannot find boost libraries.

A: Boost's library naming convention is a little confusing. You probably have the non-multi-threading version installed. So your boost libraries will look like "libboost_system.so". Dyninst links against the multi-threading version of boost, so it will needs "libboost_system-mt.so". Note the "-mt" in the library name. There are two ways to work around:

(1) Set ```PATH_BOOST``` to empy which will trigger Dyninst to automaticall build Boost

(2) Create symbolic links from non-multi-threading ones to multi-threading ones (https://stackoverflow.com/questions/3031768/boost-thread-linking-boost-thread-vs-boost-thread-mt)

## Known Issues

* Windows 64-bit mode is not yet supported

* Windows rewriter mode is not yet supported

* Exceptions in relocated code will not be caught

* Linux rewriter mode for 32-bit, statically linked binaries does not support binaries with .plt, .rel, or .rela
sections.

* Callbacks at thread or process exit that stop the process will deadlock when a SIGSEGV occurs on a thread other than
the main thread of a process

* Stackwalker is fragile on Windows

* Parsing a binary with no functions (typically a single object file) will crash at CodeObject destruction time.
