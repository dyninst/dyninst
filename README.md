# Dyninst

## Branch states

| Branch                                  | Status        | Notes                                              |
| --------------------------------------- |:-------------:|:--------------------------------------------------:|
| master                                  | stable        | See below                                          |
| arm64                                   | experimental  |                                                    |

## Notes

* Known issues should have open issues associated with them.
* ARM64 support in Dataflow/ParseAPI is experimental and incomplete.
* PPC64/little endian support in read-level interfaces 
 (symtab, stackwalker, proccontrol) is experimental.
* PPC64/little endian support in write-level interfaces is not implemented.

All non-API-breaking bug fixes should land here. All non-ABI-breaking
bug fixes should also land on v9.2.x.

## Build DyninstAPI and its subcomponents

### Configuration

Dyninst is now built via CMake. We recommend performing an interactive
configuration with "ccmake ." first, in order to see which options are
relevant for your system. You may also perform a batch configuration
with "cmake .".  Options are passed to CMake with -DVAR=VALUE. Common
options include:

```
Boost_INCLUDE_DIR 
CMAKE_BUILD_TYPE 
CMAKE_INSTALL_PREFIX
LIBDWARF_INCLUDE_DIR 
LIBDWARF_LIBRARIES 
LIBELF_INCLUDE_DIR
LIBELF_LIBRARIES 
IBERTY_LIBRARIES
```

CMake's default generator on Linux is normally "Unix Makefiles", and
on Windows, it will normally produce project files for the most recent
version of Visual Studio on your system. Other generators should work
but are not tested. After the CMake step concludes, you will have
appropriate Makefiles or equivalent and can build Dyninst.

We require CMake 2.6 as a minimum on all systems, and CMake 2.8.11
allows us to automatically download and build libelf/libdwarf/binutils
on ELF systems if they are needed. If you do not have a sufficiently
recent CMake, you may need to manually specify the location of these
dependencies. If you are cross-compiling Dyninst, including builds for
various Cray and Intel MIC systems, you will either need a toolchain
file that specifies how to properly cross-compile, or you will need to
manually define the appropriate compiler, library locations, include
locations, and the CROSS_COMPILING flag so that the build system will
properly evaluate what can be built and linked in your environment.

### Building and installing

To build Dyninst and all its components, "make && make install" from
the top-level directory of the source tree. To build and install a
single component and its dependencies, do the same thing from that
component's subdirectory. Libraries will be installed into
`CMAKE_INSTALL_PREFIX/INSTALL_LIB_DIR`, and headers will be installed
into `CMAKE_INSTALL_PREFIX/INSTALL_INCLUDE_DIR`. If you wish to import
Dyninst into your own CMake projects, the export information is in
`CMAKE_INSTALL_PREFIX/INSTALL_CMAKE_DIR`. PDF documentation is included
and installed to `CMAKE_INSTALL_PREFIX/INSTALL_DOC_DIR`. If you update
the LaTeX source documents for any manuals, "make doc" will rebuild
them. Components may be built and installed individually: "make
$COMPONENT" and "make $COMPONENT-install" respectively; this will
appropriately respect inter-component dependencies.

### What's new

## New features

* ARM64 SIMD support in instructionAPI

* Support for all x86 instruction sets up to Knight's Landing (AVX, AVX2, AVX512)

* DataflowAPI now has an official manual

* Initial ppc64/little endian support in Symtab, InstructionAPI, ProcControl, and Stackwalker. Add
-Darch_ppc64_little_endian to your CMake command line when building on little-endian ppc64 systems.

## Bug fixes

* PIE binaries should now be rewritten correctly, even if they have a zero base address

* Symtab should now correctly file symbols into their associated modules based on the best available DWARF information

* Many more fixes in x86 instruction decoding

* Enhancements to jump table analysis

* PC-relative memory accesses in VEX instructions can now be relocated correctly

* Various proccontrol bug fixes

* RTlib's `DYNINSTos_malloc` and `DYNINSTos_free` should now be signal-safe

* RTlib's tramp guard lock/unlock functions should now avoid making implicit function calls
(which are unsafe from tramp guard code)

* ppc64 bit rot for create/attach modes is fixed

## Known Issues

* ppc64 rewriter mode does not handle any code that does not conform to the "caller sets up TOC" model for intermodule
calls

* Windows 64-bit mode is not yet supported

* Windows rewriter mode is not yet supported

* Exceptions in relocated code will not be caught

* Linux rewriter mode for 32-bit, statically linked binaries does not support binaries with .plt, .rel, or .rela
sections.

* Callbacks at thread or process exit that stop the process will deadlock when a SIGSEGV occurs on a thread other than
the main thread of a process

* InstructionAPI's format() method does not produce AT&T syntax output

* Stackwalker is fragile on Windows

* Parsing a binary with no functions (typically a single object file) will crash at CodeObject destruction time.
