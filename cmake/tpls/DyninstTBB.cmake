#=====================================================
#
# Configure Intel's Threading Building Blocks
#
#   ----------------------------------------
#
# TBB_ROOT_DIR - Directory hint for TBB installation
#
#=====================================================

include_guard(GLOBAL)

# Minimum supported version
set(_min_version 2018.6)
if(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
    set(_min_version 2019.7)
endif()

set(TBB_ROOT_DIR
    "/usr"
    CACHE PATH "TBB root directory for Dyninst")
mark_as_advanced(TBB_ROOT_DIR)

find_package(TBB ${_min_version}
	HINTS ${TBB_ROOT_DIR}
	COMPONENTS tbb tbbmalloc tbbmalloc_proxy
	REQUIRED)

message(STATUS "Found TBB ${TBB_VERSION}")
get_target_property(_tmp TBB::tbb INTERFACE_INCLUDE_DIRECTORIES)
message(STATUS "TBB include directories: ${_tmp}")
