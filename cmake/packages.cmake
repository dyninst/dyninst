if (UNIX)

find_package (LibElf REQUIRED)
find_package (LibDwarf REQUIRED)
find_package (LibIberty)
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
