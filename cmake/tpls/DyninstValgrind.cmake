#================================================================
#
# Configure valgrind
#
#   ----------------------------------------
#
# Valgrind_ROOT_DIR - Directory hint for valgrind installation
#
# The individual find-modules use the <Package>_ROOT convention
# as the first location to search for the package. If the user
# specifies Valgrind_ROOT_DIR, we override the <Package>_ROOT
# values and require that each package ignores system directories.
# In effect, this forces the package search to find only
# candidates in <Package>_ROOT or CMAKE_PREFIX_PATH.
#
#================================================================

include_guard(GLOBAL)

# valgrind is only available on Unixes; provide a dummy target on other platforms
if(NOT UNIX OR NOT ADD_VALGRIND_ANNOTATIONS)
  if(NOT TARGET Dyninst::Valgrind)
    add_library(Dyninst::Valgrind INTERFACE IMPORTED)
  endif()
  return()
endif()

if(Valgrind_ROOT_DIR)
  set(Valgrind_NO_SYSTEM_PATHS ON)
  mark_as_advanced(Valgrind_NO_SYSTEM_PATHS)
  set(Valgrind_ROOT ${Valgrind_ROOT_DIR})
  mark_as_advanced(Valgrind_ROOT)
endif()

find_package(Valgrind REQUIRED)

if(NOT TARGET Dyninst::Valgrind)
  add_library(Dyninst::Valgrind INTERFACE IMPORTED)
  target_include_directories(Dyninst::Valgrind SYSTEM INTERFACE ${Valgrind_INCLUDE_DIRS})
  target_compile_definitions(Dyninst::Valgrind INTERFACE ENABLE_VG_ANNOTATIONS)
endif()
