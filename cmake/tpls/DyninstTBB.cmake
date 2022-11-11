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

find_package(
    TBB ${_min_version} HINTS ${TBB_ROOT_DIR}
    COMPONENTS tbb tbbmalloc tbbmalloc_proxy
    REQUIRED)

# Make an interface dummy target to force includes to be treated as SYSTEM
add_library(Dyninst::TBB INTERFACE IMPORTED)
target_link_libraries(Dyninst::TBB INTERFACE TBB::tbb TBB::tbbmalloc TBB::tbbmalloc_proxy)
target_include_directories(
    Dyninst::TBB SYSTEM
    INTERFACE $<TARGET_PROPERTY:TBB::tbb,INTERFACE_INCLUDE_DIRECTORIES>
              $<TARGET_PROPERTY:TBB::tbbmalloc,INTERFACE_INCLUDE_DIRECTORIES>
              $<TARGET_PROPERTY:TBB::tbbmalloc_proxy,INTERFACE_INCLUDE_DIRECTORIES>)

message(STATUS "Found TBB ${TBB_VERSION}")
get_target_property(_tmp TBB::tbb INTERFACE_INCLUDE_DIRECTORIES)
message(STATUS "TBB include directories: ${_tmp}")
