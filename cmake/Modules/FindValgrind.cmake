# ========================================================================================
# FindValgrind.cmake
#
# Find Valgrind include dirs
#
# ----------------------------------------
#
# Use this module by invoking find_package with the form::
#
# find_package(Valgrind [REQUIRED]             # Fail with error if Valgrind headers are
# not found )
#
# This module reads hints about search locations from variables::
#
# Valgrind_ROOT_DIR       - Base directory the of Valgrind installation
# Valgrind_INCLUDEDIR     - Hint directory that contains the Valgrind headers files
#
# and saves search results persistently in CMake cache entries::
#
# Valgrind_FOUND          - True if headers were found Valgrind_INCLUDE_DIRS   - Valgrind
# include directories
#
# ========================================================================================

include(DyninstSystemPaths)

find_path(
    Valgrind_INCLUDE_DIR
    NAMES valgrind.h
    HINTS ${Valgrind_ROOT_DIR}/include ${Valgrind_ROOT_DIR} ${Valgrind_INCLUDEDIR}
    PATHS ${DYNINST_SYSTEM_INCLUDE_PATHS}
    PATH_SUFFIXES valgrind
    DOC "Valgrind include directory")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    Valgrind
    FOUND_VAR Valgrind_FOUND
    REQUIRED_VARS Valgrind_INCLUDE_DIR)

# Export cache variables
if(Valgrind_FOUND)
    set(Valgrind_INCLUDE_DIRS ${Valgrind_INCLUDE_DIR})
    add_library(Valgrind::Valgrind INTERFACE IMPORTED)
    target_link_libraries(Valgrind::Valgrind INTERFACE ${Valgrind_INCLUDE_DIR})
endif()
