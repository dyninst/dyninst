set (DYNINST_MAJOR_VERSION 8)
set (DYNINST_MINOR_VERSION 2)
set (DYNINST_PATCH_VERSION 0)

# Debugging
set(Boost_DEBUG 1)

set (SOVERSION "${DYNINST_MAJOR_VERSION}.${DYNINST_MINOR_VERSION}")
set (LIBVERSION "${SOVERSION}.${DYNINST_PATCH_VERSION}")
set (DYNINST_VERSION "${LIBVERSION}")

#Change to switch between libiberty/libstdc++ demangler
#set(USE_GNU_DEMANGLER 1)

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${DYNINST_ROOT}/cmake/Modules")
include (${DYNINST_ROOT}/cmake/platform.cmake)
if (NOT ${PROJECT_NAME} MATCHES DyninstRT)
include (${DYNINST_ROOT}/cmake/packages.cmake)
include (${DYNINST_ROOT}/cmake/c++11.cmake)
endif()
include (${DYNINST_ROOT}/cmake/cap_arch_def.cmake)
include (${DYNINST_ROOT}/cmake/visibility.cmake)
include (${DYNINST_ROOT}/cmake/warnings.cmake)
include (${DYNINST_ROOT}/cmake/options.cmake)
include (${DYNINST_ROOT}/cmake/optimization.cmake)

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

if(PLATFORM MATCHES nt OR PLATFORM MATCHES windows)
    add_definitions(-Dsnprintf=_snprintf)
endif()

#
# DyninstConfig.cmake

file (RELATIVE_PATH REL_INCLUDE_DIR "${INSTALL_CMAKE_DIR}" "${INSTALL_INCLUDE_DIR}")

# For the install tree
set (CONF_INCLUDE_DIRS "\${DYNINST_CMAKE_DIR}/${REL_INCLUDE_DIR}")



