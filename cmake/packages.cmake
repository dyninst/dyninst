if (UNIX)
  find_package (LibDwarf)
  find_package (LibElf 0.173)
  find_package(TBB)
  if(NOT LIBELF_FOUND OR NOT LIBDWARF_FOUND)
    message(STATUS "Attempting to build elfutils as external project")
    cmake_minimum_required (VERSION 2.8.11)
    include(ExternalProject)
    ExternalProject_Add(LibElf
      PREFIX ${CMAKE_BINARY_DIR}/elfutils
      URL https://sourceware.org/elfutils/ftp/elfutils-latest.tar.bz2
      CONFIGURE_COMMAND CFLAGS=-g <SOURCE_DIR>/configure --enable-install-elfh --enable-shared --prefix=${CMAKE_BINARY_DIR}/elfutils
      BUILD_COMMAND make
      INSTALL_COMMAND make install
      )
    set(LIBELF_INCLUDE_DIR ${CMAKE_BINARY_DIR}/elfutils/include)
    set(LIBELF_LIBRARIES ${CMAKE_BINARY_DIR}/elfutils/lib/libelf.so)
    set(LIBDWARF_INCLUDE_DIR ${CMAKE_BINARY_DIR}/elfutils/include)
    set(LIBDWARF_LIBRARIES ${CMAKE_BINARY_DIR}/elfutils/lib/libdw.so)
    set(SHOULD_INSTALL_LIBELF 1)
  else()
    set(SHOULD_INSTALL_LIBELF 0)
  endif()
  if(NOT TBB_FOUND)
    message(STATUS "Attempting to build TBB as external project")
    cmake_minimum_required (VERSION 2.8.11)
    include(ExternalProject)
    ExternalProject_Add(TBB
            PREFIX ${CMAKE_BINARY_DIR}/tbb
            STAMP_DIR ${CMAKE_BINARY_DIR}/tbb/src/TBB-stamp
            URL https://github.com/01org/tbb/archive/2018_U6.tar.gz
            URL_MD5 9a0f78db4f72356068b00f29f54ee6bc
            SOURCE_DIR ${CMAKE_BINARY_DIR}/tbb/src/TBB/src
            CONFIGURE_COMMAND ""
            BINARY_DIR ${CMAKE_BINARY_DIR}/tbb/src/TBB/src
            BUILD_COMMAND make -j${NCPU} tbb tbbmalloc tbb_build_dir=${CMAKE_BINARY_DIR}/tbb/src/TBB-build tbb_build_prefix=tbb
            INSTALL_COMMAND sh -c "mkdir -p ${CMAKE_BINARY_DIR}/tbb/include && mkdir -p ${CMAKE_BINARY_DIR}/tbb/lib \
                && cp ${CMAKE_BINARY_DIR}/tbb/src/TBB-build/tbb_release/*.so* ${CMAKE_BINARY_DIR}/tbb/lib \
                && cp -r ${CMAKE_BINARY_DIR}/tbb/src/TBB/src/include/* ${CMAKE_BINARY_DIR}/tbb/include"
            )
    set(TBB_INCLUDE_DIRS ${CMAKE_BINARY_DIR}/tbb/include)
    set(TBB_LIBRARIES ${CMAKE_BINARY_DIR}/tbb/lib/libtbb.so ${CMAKE_BINARY_DIR}/tbb/lib/libtbbmalloc_proxy.so)
    set(TBB_FOUND 1)
  endif()
  add_library(libelf_imp SHARED IMPORTED)
  set_property(TARGET libelf_imp
    PROPERTY IMPORTED_LOCATION ${LIBELF_LIBRARIES})
  if(NOT LIBELF_FOUND)
    add_dependencies(libelf_imp LibElf)
  endif()

  add_library(libdwarf_imp SHARED IMPORTED)
  set_property(TARGET libdwarf_imp 
    PROPERTY IMPORTED_LOCATION ${LIBDWARF_LIBRARIES})
  if(NOT LIBDWARF_FOUND)
    add_dependencies(libdwarf_imp LibDwarf)
  endif()

  if (NOT USE_GNU_DEMANGLER)
    find_package (LibIberty)

    if(NOT IBERTY_FOUND)
      cmake_minimum_required (VERSION 2.8.11)
      include(ExternalProject)
      ExternalProject_Add(LibIberty
	PREFIX ${CMAKE_BINARY_DIR}/binutils
	URL http://ftp.gnu.org/gnu/binutils/binutils-2.31.1.tar.gz
	CONFIGURE_COMMAND env CFLAGS=${CMAKE_C_FLAGS}\ -fPIC CPPFLAGS=-fPIC PICFLAG=-fPIC <SOURCE_DIR>/libiberty/configure --prefix=${CMAKE_BINARY_DIR}/libiberty --enable-shared
	BUILD_COMMAND make all
	INSTALL_DIR ${CMAKE_BINARY_DIR}/libiberty
	INSTALL_COMMAND install <BINARY_DIR>/libiberty.a <INSTALL_DIR>
	)
      set(IBERTY_LIBRARIES ${CMAKE_BINARY_DIR}/libiberty/libiberty.a)
      set(IBERTY_FOUND TRUE)
      set(IBERTY_BUILD TRUE)
    endif()

    message(STATUS "Using libiberty ${IBERTY_LIBRARIES}")
    add_library(libiberty_imp STATIC IMPORTED)
    set_property(TARGET libiberty_imp
      PROPERTY IMPORTED_LOCATION ${IBERTY_LIBRARIES})
    if(IBERTY_BUILD)
      add_dependencies(libiberty_imp LibIberty)
    endif()
  endif()

  find_package (ThreadDB)
  include_directories (
    ${LIBELF_INCLUDE_DIR}
    ${LIBDWARF_INCLUDE_DIR}
    )
endif()

if (PLATFORM MATCHES "bgq")
  # Not a find per se, just a magic include line
  set (PATH_BGQ "/bgsys/drivers/ppcfloor" CACHE STRING "Path to BG/Q include files")
  if (NOT (PATH_BGQ STREQUAL ""))
    include_directories (${PATH_BGQ})
  endif()
endif()


include(${DYNINST_ROOT}/cmake/CheckCXX11Features.cmake)
if(NOT HAS_CXX11_AUTO)
  message(FATAL_ERROR "No support for C++11 auto found. Dyninst requires this compiler feature.")
else()
  message(STATUS "C++11 support found, required flags are: ${CXX11_COMPILER_FLAGS}")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX11_COMPILER_FLAGS}")
