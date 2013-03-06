if (UNIX)
set (PATH_LIBDWARF "/usr" CACHE STRING "Path to libdwarf")
if (NOT (PATH_LIBDWARF STREQUAL ""))
  set (CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} ${PATH_LIBDWARF}/lib ${PATH_LIBDWARF}/lib64)
  set (CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} ${PATH_LIBDWARF}/include)
endif()

set (PATH_LIBELF "/usr" CACHE STRING "Path to libelf")
if (NOT (PATH_LIBELF STREQUAL ""))
  set (CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} ${PATH_LIBELF}/lib ${PATH_LIBELF}/lib64)
  set (CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} ${PATH_LIBELF}/include)
endif()

find_package (LibElf REQUIRED)
find_package (LibDwarf REQUIRED)
include_directories (
                    ${LIBELF_INCLUDE_DIRS}
                    ${LIBDWARF_INCLUDE_DIRS}
)
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
