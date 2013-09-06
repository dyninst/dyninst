if(UNIX)
cmake_minimum_required (VERSION 2.8.11)
else()
cmake_minimum_required (VERSION 2.6.4)
endif()
project (Dyninst)

set (DYNINST_MAJOR_VERSION 8)
set (DYNINST_MINOR_VERSION 2)
set (DYNINST_PATCH_VERSION 0)

# Debugging
set(Boost_DEBUG 1)

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

if(${PLATFORM} MATCHES nt)
set (HEADER_DIRS common
                 dataflowAPI
                 dyninstAPI
                 dyninstAPI_RT
                 dynutil
                 instructionAPI
                 parseAPI
                 patchAPI
                 proccontrol
                 stackwalk
                 symtabAPI
    )
else()
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
endif()

foreach (dir ${HEADER_DIRS})
  include_directories ( ${PROJECT_SOURCE_DIR}/${dir}/h )
endforeach()

# Component time
add_subdirectory (common)
if(NOT ${PLATFORM} MATCHES nt)
    add_subdirectory (elf)
    add_subdirectory (dwarf)
    add_subdirectory (symlite)
endif()
add_subdirectory (instructionAPI)
add_subdirectory (symtabAPI)
add_subdirectory (parseAPI)
add_subdirectory (patchAPI)
add_subdirectory (proccontrol)
add_subdirectory (stackwalk)
add_subdirectory (dyninstAPI)
add_subdirectory (dyninstAPI_RT)

if(UNIX)
add_dependencies(dynDwarf libdwarf_imp)
add_dependencies(dynElf libelf_imp)
add_dependencies(common libiberty_imp)
add_dependencies(libdwarf_imp libelf_imp)
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
    set (CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden)
    set (CMAKE_C_FLAGS ${CMAKE_C_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden)
    message(STATUS "Found g++, enabling -fvisibility=hidden")
endif()

if(NOT ${PLATFORM} MATCHES nt)
    SET_TARGET_PROPERTIES (
        common dynElf dynDwarf instructionAPI symtabAPI symLite parseAPI
        patchAPI pcontrol stackwalk dyninstAPI
        PROPERTIES
            SOVERSION ${SOVERSION}
            VERSION ${LIBVERSION}
    )
else()
    SET_TARGET_PROPERTIES (
        common instructionAPI symtabAPI parseAPI
        patchAPI pcontrol stackwalk dyninstAPI
        PROPERTIES
            SOVERSION ${SOVERSION}
            VERSION ${LIBVERSION}
    )

endif()

if(PLATFORM MATCHES nt OR PLATFORM MATCHES windows)
    add_definitions(-Dsnprintf=_snprintf)
endif()

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

if(UNIX)
if(SHOULD_INSTALL_LIBELF)
get_filename_component(ELFDIR "${LIBELF_LIBRARIES}" PATH)
file(GLOB LIBELF_INSTALL_FILES ${ELFDIR}/libelf.so*)
message(STATUS "Libelf is ${LIBELF_LIBRARIES}")
message(STATUS "Installing ${LIBELF_INSTALL_FILES} from ${ELFDIR}")
install (FILES ${LIBELF_INSTALL_FILES}
	DESTINATION "${INSTALL_LIB_DIR}")
endif()
if(NOT LIBDWARF_FOUND)
message(STATUS "Installing ${LIBDWARF_LIBRARIES}")
install (FILES ${LIBDWARF_LIBRARIES}
	DESTINATION "${INSTALL_LIB_DIR}")
endif()
endif()
install (FILES
        "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/DyninstConfig.cmake"
        "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/DyninstConfigVersion.cmake"
        DESTINATION "${INSTALL_CMAKE_DIR}")
install (EXPORT DyninstTargets
        DESTINATION "${INSTALL_CMAKE_DIR}")
