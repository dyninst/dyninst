cmake_minimum_required (VERSION 2.6)
project (Dyninst)

set (SOVERSION "8.1")
set (LIBVERSION "8.1.1")

if (UNIX)
enable_language(ASM-ATT)
endif()

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/Modules")
include (cmake/platform.cmake)
include (cmake/packages.cmake)
include (cmake/cap_arch_def.cmake)
include (cmake/c++11.cmake)
include (cmake/warnings.cmake)
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
add_subdirectory (dyninstAPI_RT)

SET_TARGET_PROPERTIES (
   common dynElf dynDwarf instructionAPI symtabAPI symLite parseAPI
   patchAPI pcontrol stackwalker dyninstAPI dyninstAPI_RT
   PROPERTIES
     SOVERSION ${SOVERSION}
     VERSION ${LIBVERSION}
)

# RTlib properties are set in its CMakeLists.txt; it seems
# like multiple set_target_properties calls override instead
# of accumulating

add_subdirectory(testsuite/${PLATFORM})
