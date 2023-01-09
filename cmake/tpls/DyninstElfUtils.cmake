#================================================================
#
# Configure elfutils
#
#   ----------------------------------------
#
# ElfUtils_ROOT_DIR - Directory hint for elfutils installation
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

# Base directory the of elfutils installation
set(ElfUtils_ROOT_DIR
    "/usr"
    CACHE PATH "Base directory the of elfutils installation")
mark_as_advanced(ElfUtils_ROOT_DIR)

# Find modules expect <LIB>_ROOT instead of <LIB>_ROOT_DIR
set(LibELF_ROOT ${ElfUtils_ROOT_DIR})
set(LibDW_ROOT ${ElfUtils_ROOT_DIR})
set(LibDebuginfod_ROOT ${ElfUtils_ROOT_DIR})

if(ENABLE_DEBUGINFOD)
  set(_components debuginfod)
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
