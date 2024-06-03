#======================================================================================
# PeParse.cmake
#
# Configure PeParse for Dyninst
#
#   ----------------------------------------
#
# Directly exports the following CMake variables
#
# PeParse_ROOT_DIR       - Computed base directory the of PeParse installation
#
# NOTE:
# The exported PeParse_ROOT_DIR can be different from the value provided by the user
# in the case that it is determined to build PeParse from source. In such a case,
# PeParse_ROOT_DIR will contain the directory of the from-source installation.
#
# See Modules/FindPeParse.cmake for details
#
#======================================================================================

include_guard(GLOBAL)

cmake_policy(SET CMP0074 NEW) # Use <Package>_ROOT

if(NOT UNIX)
    return()
endif()

if(PeParse_ROOT_DIR)
    set(PeParse_NO_SYSTEM_PATHS ON)
    mark_as_advanced(PeParse_NO_SYSTEM_PATHS)
    set(PeParse_ROOT "${PeParse_ROOT_DIR}")
    mark_as_advanced(PeParse_ROOT)
endif()

find_package(PeParse REQUIRED)

if(NOT TARGET Dyninst::PeParse)
    add_library(Dyninst::PeParse INTERFACE IMPORTED)
    if(PeParse_FOUND)
        target_include_directories(Dyninst::PeParse
                                   INTERFACE "${PeParse_INCLUDE_DIRS}")
        target_link_libraries(Dyninst::PeParse INTERFACE PeParse::PeParse)
    endif()
endif()
