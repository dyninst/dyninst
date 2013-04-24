if (UNIX)

find_package (LibElf REQUIRED)
find_package (LibDwarf REQUIRED)
find_package (LibIberty REQUIRED)
find_package (ThreadDB)
include_directories (
                    ${LIBELF_INCLUDE_DIR}
                    ${LIBDWARF_INCLUDE_DIR}
)

if (${PLATFORM} MATCHES "bgq")
find_package (LaunchMon REQUIRED)
include_directories (${LAUNCHMON_INCLUDE_DIR})
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
