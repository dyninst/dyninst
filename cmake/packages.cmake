if (UNIX)
  find_package (LibElf)
  if(NOT LIBELF_FOUND)
    message(STATUS "No libelf found, attempting to build as external project")
    cmake_minimum_required (VERSION 2.8.11)
    include(ExternalProject)
    ExternalProject_Add(LibElf
      PREFIX ${CMAKE_BINARY_DIR}/libelf
      URL http://www.mr511.de/software/libelf-0.8.13.tar.gz
      CONFIGURE_COMMAND <SOURCE_DIR>/configure --enable-shared --prefix=${CMAKE_BINARY_DIR}/libelf
      BUILD_COMMAND make
      INSTALL_COMMAND make install
      )
    set(LIBELF_INCLUDE_DIR ${CMAKE_BINARY_DIR}/libelf/include)
    set(LIBELF_LIBRARIES ${CMAKE_BINARY_DIR}/libelf/lib/libelf.so)
    set(SHOULD_INSTALL_LIBELF 1)
  else()
    set(SHOULD_INSTALL_LIBELF 0)
  endif()

  add_library(libelf_imp SHARED IMPORTED)
  set_property(TARGET libelf_imp
    PROPERTY IMPORTED_LOCATION ${LIBELF_LIBRARIES}) 

  find_package (LibDwarf)

  if(NOT LIBDWARF_FOUND)
    message(STATUS "No libdwarf found, attempting to build as external project")
    cmake_minimum_required (VERSION 2.8.11)
    include(ExternalProject)
    ExternalProject_Add(LibDwarf
      PREFIX ${CMAKE_BINARY_DIR}/libdwarf
      DEPENDS libelf_imp
      #	URL http://reality.sgiweb.org/davea/libdwarf-20130126.tar.gz
      #	URL http://sourceforge.net/p/libdwarf/code/ci/20130126/tarball
      URL http://www.paradyn.org/libdwarf/libdwarf-20130126.tar.gz
      #	GIT_REPOSITORY git://git.code.sf.net/p/libdwarf/code libdwarf-code
      #	GIT_TAG 20130126
      CONFIGURE_COMMAND CFLAGS=-I${LIBELF_INCLUDE_DIR} LDFLAGS=-L${CMAKE_BINARY_DIR}/libelf/lib <SOURCE_DIR>/libdwarf/configure --enable-shared
      BUILD_COMMAND make
      INSTALL_DIR ${CMAKE_BINARY_DIR}/libdwarf
      INSTALL_COMMAND mkdir -p <INSTALL_DIR>/include && mkdir -p <INSTALL_DIR>/lib && install <SOURCE_DIR>/libdwarf/libdwarf.h <INSTALL_DIR>/include && install <SOURCE_DIR>/libdwarf/dwarf.h <INSTALL_DIR>/include && install <BINARY_DIR>/libdwarf.so <INSTALL_DIR>/lib
      )
    add_dependencies(LibDwarf libelf_imp)
    target_link_private_libraries(LibDwarf libelf_imp)
    #ExternalProject_Get_Property(LibDwarf 
    set(LIBDWARF_INCLUDE_DIR ${CMAKE_BINARY_DIR}/libdwarf/include)
    set(LIBDWARF_LIBRARIES ${CMAKE_BINARY_DIR}/libdwarf/lib/libdwarf.so)
  else()
    # Unfortunately, libdwarf doesn't always link to libelf itself.
    # (e.g. https://bugzilla.redhat.com/show_bug.cgi?id=1061432)
    set(LIBDWARF_LIBRARIES ${LIBDWARF_LIBRARIES} ${LIBELF_LIBRARIES})
  endif()

  add_library(libdwarf_imp SHARED IMPORTED)
  set_property(TARGET libdwarf_imp 
    PROPERTY IMPORTED_LOCATION ${LIBDWARF_LIBRARIES})

  if (NOT USE_GNU_DEMANGLER)
    find_package (LibIberty)

    if(NOT IBERTY_FOUND)
      cmake_minimum_required (VERSION 2.8.11)
      include(ExternalProject)
      ExternalProject_Add(LibIberty
	PREFIX ${CMAKE_BINARY_DIR}/binutils
	URL http://ftp.gnu.org/gnu/binutils/binutils-2.23.tar.gz
	CONFIGURE_COMMAND CFLAGS=-fPIC CPPFLAGS=-fPIC PICFLAG=-fPIC <SOURCE_DIR>/libiberty/configure --prefix=${CMAKE_BINARY_DIR}/libiberty --enable-shared
	BUILD_COMMAND make all
	INSTALL_DIR ${CMAKE_BINARY_DIR}/libiberty
	INSTALL_COMMAND install <BINARY_DIR>/libiberty.a <INSTALL_DIR>
	)
      set(IBERTY_LIBRARIES ${CMAKE_BINARY_DIR}/libiberty/libiberty.a)
    endif()

    message(STATUS "Using libiberty ${IBERTY_LIBRARIES}")
    add_library(libiberty_imp STATIC IMPORTED)
    set_property(TARGET libiberty_imp
      PROPERTY IMPORTED_LOCATION ${IBERTY_LIBRARIES})
  endif()

  find_package (ThreadDB)
  include_directories (
    ${LIBELF_INCLUDE_DIR}
    ${LIBDWARF_INCLUDE_DIR}
    )
elseif(WIN32)
  find_package (DIASDK REQUIRED)
  include_directories(${DIASDK_INCLUDE_DIR})
endif()

if (PLATFORM MATCHES "bgq")
  # Not a find per se, just a magic include line
  set (PATH_BGQ "/bgsys/drivers/ppcfloor" CACHE STRING "Path to BG/Q include files")
  if (NOT (PATH_BGQ STREQUAL ""))
    include_directories (${PATH_BGQ})
  endif()
endif()


# UPDATE this for newer versions of Boost if you're using
# an older CMake and it complains that it can't find Boost
set(Boost_ADDITIONAL_VERSIONS "1.47" "1.47.0" "1.48" "1.48.0" "1.49" "1.49.0"
  "1.50" "1.50.0" "1.51" "1.51.0" "1.52" "1.52.0"
  "1.53" "1.53.0" "1.54" "1.54.0" "1.55" "1.55.0" "1.56" "1.56.0")

set (Boost_DEBUG ON)
set (PATH_BOOST "/usr" CACHE STRING "Path to boost")

set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

if (NOT ("${Boost_NO_BOOST_CMAKE}" STREQUAL "OFF"))
  message(STATUS "Disabling Boost's own CMake--known buggy in many cases")
  set(Boost_NO_BOOST_CMAKE ON)
endif()
if (NOT ("${PATH_BOOST}" STREQUAL ""))
  set (CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} ${PATH_BOOST}/lib ${PATH_BOOST}/lib64)
  set (CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} ${PATH_BOOST}/include)
endif()

# Boost 1.40/1.41 are not compatible with each other
# so ensure that we don't mix incompatible headers with
# the thread library
#set (BOOST_MIN_VERSION 1.41.0)

if(DEFINED PATH_BOOST OR 
	   DEFINED Boost_INCLUDE_DIR OR 
	   DEFINED Boost_LIBRARY_DIR)
  set(Boost_NO_SYSTEM_PATHS ON)
endif()

find_package (Boost ${BOOST_MIN_VERSION} REQUIRED COMPONENTS thread system)

link_directories ( ${Boost_LIBRARY_DIRS} )

include_directories (
  ${Boost_INCLUDE_DIRS}
  )

message(STATUS "Boost includes: ${Boost_INCLUDE_DIRS}")
message(STATUS "Boost library dirs: ${Boost_LIBRARY_DIRS}")
message(STATUS "Boost thread library: ${Boost_THREAD_LIBRARY}")
message(STATUS "Boost libraries: ${Boost_LIBRARIES}")

include(${DYNINST_ROOT}/cmake/CheckCXX11Features.cmake)
if(NOT HAS_CXX11_AUTO)
  message(FATAL_ERROR "No support for C++11 auto found. Dyninst requires this compiler feature.")
else()
  message(STATUS "C++11 support found, required flags are: ${CXX11_COMPILER_FLAGS}")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX11_COMPILER_FLAGS}")
