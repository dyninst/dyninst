if (UNIX)

include(ExternalProject)

find_package (LibElf)
if(NOT LIBELF_FOUND)
message(STATUS "No libelf found, attempting to build as external project")
ExternalProject_Add(LibElf
	PREFIX ${CMAKE_SOURCE_DIR}/libelf
	URL http://www.mr511.de/software/libelf-0.8.13.tar.gz
	CONFIGURE_COMMAND <SOURCE_DIR>/configure --enable-shared --prefix=${CMAKE_SOURCE_DIR}/libelf
	BUILD_COMMAND make
	INSTALL_COMMAND make install
)
set(LIBELF_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/libelf/include)
set(LIBELF_LIBRARIES ${CMAKE_SOURCE_DIR}/libelf/lib/libelf.so)
endif()

add_library(libelf_imp SHARED IMPORTED)
set_property(TARGET libelf_imp
		    PROPERTY IMPORTED_LOCATION ${LIBELF_LIBRARIES}) 

find_package (LibDwarf)

if(NOT LIBDWARF_FOUND)
message(STATUS "No libdwarf found, attempting to build as external project")
ExternalProject_Add(LibDwarf
	PREFIX ${CMAKE_SOURCE_DIR}/libdwarf
	DEPENDS libelf_imp
	URL http://reality.sgiweb.org/davea/libdwarf-20130126.tar.gz
	CONFIGURE_COMMAND CFLAGS=-I${LIBELF_INCLUDE_DIR} LDFLAGS=-L${CMAKE_SOURCE_DIR}/libelf/lib <SOURCE_DIR>/libdwarf/configure --enable-shared
	BUILD_COMMAND make
	INSTALL_DIR ${CMAKE_SOURCE_DIR}/libdwarf
	INSTALL_COMMAND mkdir -p <INSTALL_DIR>/include && mkdir -p <INSTALL_DIR>/lib && install <SOURCE_DIR>/libdwarf/libdwarf.h <INSTALL_DIR>/include && install <SOURCE_DIR>/libdwarf/dwarf.h <INSTALL_DIR>/include && install <BINARY_DIR>/libdwarf.so <INSTALL_DIR>/lib
)
add_dependencies(LibDwarf libelf_imp)
target_link_libraries(LibDwarf libelf_imp)
#ExternalProject_Get_Property(LibDwarf 
set(LIBDWARF_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/libdwarf/include)
set(LIBDWARF_LIBRARIES ${CMAKE_SOURCE_DIR}/libdwarf/lib/libdwarf.so)
endif()

add_library(libdwarf_imp SHARED IMPORTED)
set_property(TARGET libdwarf_imp 
		    PROPERTY IMPORTED_LOCATION ${LIBDWARF_LIBRARIES})
find_package (LibIberty)

if(NOT IBERTY_FOUND)
ExternalProject_Add(LibIberty
	PREFIX ${CMAKE_SOURCE_DIR}/binutils
	URL http://ftp.gnu.org/gnu/binutils/binutils-2.23.tar.gz
	CONFIGURE_COMMAND CFLAGS=-fPIC CPPFLAGS=-fPIC PICFLAG=-fPIC <SOURCE_DIR>/libiberty/configure --prefix=${CMAKE_SOURCE_DIR}/libiberty --enable-shared
	BUILD_COMMAND make all
	INSTALL_DIR ${CMAKE_SOURCE_DIR}/libiberty
	INSTALL_COMMAND install <BINARY_DIR>/libiberty.a <INSTALL_DIR>
)
set(IBERTY_LIBRARY ${CMAKE_SOURCE_DIR}/libiberty/libiberty.a)
endif()

message(STATUS "Using libiberty ${IBERTY_LIBRARY}")
add_library(libiberty_imp STATIC IMPORTED)
set_property(TARGET libiberty_imp
		    PROPERTY IMPORTED_LOCATION ${IBERTY_LIBRARY})

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

set (PATH_BOOST "/usr" CACHE STRING "Path to boost")
if (NOT (PATH_BOOST STREQUAL ""))
  set (CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} ${PATH_BOOST}/lib ${PATH_BOOST}/lib64)
  set (CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} ${PATH_BOOST}/include)
endif()

find_package (Boost REQUIRED)

include_directories (
                    ${Boost_INCLUDE_DIRS}
)
