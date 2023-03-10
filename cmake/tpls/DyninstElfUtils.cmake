#================================================================
#
# Configure elfutils
#
#   ----------------------------------------
#
# ElfUtils_ROOT_DIR - Location of elfutils installation
#
# The individual find-modules use the <Package>_ROOT convention
# as the first location to search for the package. If the user
# specifies ElfUtils_ROOT_DIR, we override the <Package>_ROOT
# values and require that each package ignores system directories.
# In effect, this forces the package search to find only
# candidates in <Package>_ROOT or CMAKE_PREFIX_PATH.
#
#================================================================

include_guard(GLOBAL)

# elfutils is only available on Unixes; provide a dummy target on other platforms
if(NOT UNIX)
  if(NOT TARGET Dyninst::ElfUtils)
    add_library(Dyninst::ElfUtils INTERFACE)
  endif()
  return()
endif()

# We need >=0.186 because of NVIDIA line map extensions
set(_min_version 0.186)

if(ENABLE_DEBUGINFOD)
  set(_components debuginfod)
endif()

if(ElfUtils_ROOT_DIR)
  set(ElfUtils_NO_SYSTEM_PATHS ON)
endif()

find_package(Elfutils ${_min_version} REQUIRED COMPONENTS ${_components})
unset(_components)

if(NOT TARGET Dyninst::ElfUtils)
  add_library(Dyninst::ElfUtils INTERFACE IMPORTED)
  target_include_directories(Dyninst::ElfUtils SYSTEM INTERFACE ${Elfutils_INCLUDE_DIRS})
  target_link_libraries(Dyninst::ElfUtils INTERFACE Elfutils::Elfutils)
  if(ENABLE_DEBUGINFOD)
    set_property(
      TARGET Dyninst::ElfUtils
      APPEND
      PROPERTY INTERFACE_COMPILE_DEFINITIONS DEBUGINFOD_LIB)
  endif()
endif()
