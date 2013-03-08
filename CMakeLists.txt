cmake_minimum_required (VERSION 2.6)
project (Dyninst)

set (PLATFORM $ENV{PLATFORM})

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/Modules")
include (cmake/cap_arch_def.cmake)
include (cmake/c++11.cmake)
include (cmake/warnings.cmake)
include (cmake/packages.cmake)
include (cmake/options.cmake)

set (BUILD_SHARED_LIBS ON)

include_directories (
                    ${PROJECT_SOURCE_DIR}
                    ${PROJECT_SOURCE_DIR}/external
  )

set (HEADER_DIRS dataflowAPI
                 dwarf
                 dyninstAPI
                 dyninstAPI_RT
                 dynutil
                 elf
                 instructionAPI
                 parseAPI
                 patchAPI
                 proccontrol
                 stackwalk
                 symlite
                 symtabAPI
    )

foreach (dir ${HEADER_DIRS})
  include_directories ( ${PROJECT_SOURCE_DIR}/${dir}/h )
endforeach()

# Component time
add_subdirectory (dynutil)
add_subdirectory (common)
add_subdirectory (elf)
add_subdirectory (dwarf)
add_subdirectory (instructionAPI)
add_subdirectory (symtabAPI)
add_subdirectory (symlite)
add_subdirectory (parseAPI)
add_subdirectory (patchAPI)
add_subdirectory (proccontrol)
add_subdirectory (stackwalk)
add_subdirectory (dyninstAPI)
