#================================================================
#
# Configure libiberty
#
#   ----------------------------------------
#
# LibIberty_ROOT_DIR - Location of libiberty installation
#
# The individual find-modules use the <Package>_ROOT convention
# as the first location to search for the package. If the user
# specifies LibIberty_ROOT_DIR, we override the <Package>_ROOT
# values and require that each package ignores system directories.
# In effect, this forces the package search to find only
# candidates in <Package>_ROOT or CMAKE_PREFIX_PATH.
#
#================================================================

include_guard(GLOBAL)

# libiberty is only available on Unixes; provide a dummy target on other platforms
if(NOT UNIX)
  if(NOT TARGET Dyninst::LibIberty)
    add_library(Dyninst::LibIberty INTERFACE)
  endif()
  return()
endif()

if(LibIberty_ROOT_DIR)
  set(LibIberty_NO_SYSTEM_PATHS ON)
  mark_as_advanced(LibIberty_NO_SYSTEM_PATHS)
  set(LibIberty_ROOT ${LibIberty_ROOT_DIR})
  mark_as_advanced(LibIberty_ROOT)
endif()

find_package(LibIberty REQUIRED)

if(NOT TARGET Dyninst::LibIberty)
  add_library(Dyninst::LibIberty INTERFACE IMPORTED)
  target_include_directories(Dyninst::LibIberty SYSTEM
                             INTERFACE ${LibIberty_INCLUDE_DIRS})
  target_link_libraries(Dyninst::LibIberty INTERFACE LibIberty::LibIberty)
endif()
