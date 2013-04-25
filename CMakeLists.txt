cmake_minimum_required (VERSION 2.6)
project (Dyninst)

set (DYNINST_MAJOR_VERSION 8)
set (DYNINST_MINOR_VERSION 1)
set (DYNINST_PATCH_VERSION 1)

set (SOVERSION "${DYNINST_MAJOR_VERSION}.${DYNINST_MINOR_VERSION}")
set (LIBVERSION "${SOVERSION}.${DYNINST_PATCH_VERSION}")
set (DYNINST_VERSION "${LIBVERSION}")

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/Modules")
include (cmake/platform.cmake)
include (cmake/packages.cmake)
include (cmake/cap_arch_def.cmake)
include (cmake/c++11.cmake)
include (cmake/warnings.cmake)
include (cmake/options.cmake)
include (cmake/optimization.cmake)

set (BUILD_SHARED_LIBS ON)

set (INSTALL_LIB_DIR lib CACHE PATH "Installation directory for libraries")
set (INSTALL_INCLUDE_DIR include CACHE PATH "Installation directory for header files")
set (INSTALL_CMAKE_DIR lib/cmake/Dyninst CACHE PATH "Installation directory for CMake files")

# Make the above absolute paths if necessary
foreach (p LIB INCLUDE CMAKE)
  set (var INSTALL_${p}_DIR)
  if (NOT IS_ABSOLUTE "${${var}}")
     set (${var} "${CMAKE_INSTALL_PREFIX}/${${var}}")
  endif()
endforeach()

include_directories (
                    ${PROJECT_SOURCE_DIR}
                    ${PROJECT_SOURCE_DIR}/external
  )

set (HEADER_DIRS common
                 dataflowAPI
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
   patchAPI pcontrol stackwalk dyninstAPI dyninstAPI_RT
   PROPERTIES
     SOVERSION ${SOVERSION}
     VERSION ${LIBVERSION}
)

# RTlib properties are set in its CMakeLists.txt; it seems
# like multiple set_target_properties calls override instead
# of accumulating





#
# DyninstConfig.cmake

# Not sure if needed...
#export (PACKAGE Dyninst)

file (RELATIVE_PATH REL_INCLUDE_DIR "${INSTALL_CMAKE_DIR}" "${INSTALL_INCLUDE_DIR}")

# For the build tree
# I don't think we want this; it doesn't work quite right
#foreach (dir ${HEADER_DIRS})
#    set (CONF_INCLUDE_DIRS ${CONF_INCLUDE_DIRS} ${PROJECT_SOURCE_DIR}/${dir}/h)
#endforeach()
#configure_file(cmake/DyninstConfig.cmake.in "${PROJECT_BINARY_DIR}/DyninstConfig.cmake" @ONLY)


# For the install tree
set (CONF_INCLUDE_DIRS "\${DYNINST_CMAKE_DIR}/${REL_INCLUDE_DIR}")
configure_file(cmake/DyninstConfig.cmake.in "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/DyninstConfig.cmake" @ONLY)

# And version file
configure_file(cmake/DyninstConfigVersion.cmake.in "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/DyninstConfigVersion.cmake" @ONLY)
configure_file(cmake/DyninstConfigVersion.cmake.in "${PROJECT_BINARY_DIR}/DyninstConfigVersion.cmake" @ONLY)

install (FILES
        "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/DyninstConfig.cmake"
        "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/DyninstConfigVersion.cmake"
        DESTINATION "${INSTALL_CMAKE_DIR}")

install (EXPORT DyninstTargets
        DESTINATION "${INSTALL_CMAKE_DIR}")
