#=====================================================
#
# Configure Intel's Threading Building Blocks
#
#   ----------------------------------------
#
# TBB_ROOT_DIR - Directory hint for TBB installation
#
# The individual find-modules use the <Package>_ROOT convention
# as the first location to search for the package. If the user
# specifies TBB_ROOT_DIR, we override the <Package>_ROOT
# values and require that each package ignores system directories.
# In effect, this forces the package search to find only
# candidates in <Package>_ROOT or CMAKE_PREFIX_PATH.
#
#=====================================================

include_guard(GLOBAL)

# Minimum supported version
set(_min_version 2019.9)

if(TBB_ROOT_DIR)
  set(TBB_ROOT ${TBB_ROOT_DIR})
  mark_as_advanced(TBB_ROOT)
  set(_find_path_args NO_CMAKE_SYSTEM_PATH NO_SYSTEM_ENVIRONMENT_PATH)
endif()

find_package(
  TBB ${_min_version}
  COMPONENTS tbb tbbmalloc tbbmalloc_proxy
  REQUIRED ${_find_path_args})

# Don't let TBB variables seep through
mark_as_advanced(TBB_DIR)

if(NOT TARGET Dyninst::TBB)
  add_library(Dyninst::TBB INTERFACE IMPORTED)
  target_link_libraries(Dyninst::TBB INTERFACE TBB::tbb TBB::tbbmalloc
                                               TBB::tbbmalloc_proxy)
  target_include_directories(
    Dyninst::TBB SYSTEM
    INTERFACE $<TARGET_PROPERTY:TBB::tbb,INTERFACE_INCLUDE_DIRECTORIES>
              $<TARGET_PROPERTY:TBB::tbbmalloc,INTERFACE_INCLUDE_DIRECTORIES>
              $<TARGET_PROPERTY:TBB::tbbmalloc_proxy,INTERFACE_INCLUDE_DIRECTORIES>)
endif()

message(STATUS "Found TBB ${TBB_VERSION}")
get_target_property(_tmp TBB::tbb INTERFACE_INCLUDE_DIRECTORIES)
message(STATUS "TBB include directories: ${_tmp}")

unset(_find_path_args)
