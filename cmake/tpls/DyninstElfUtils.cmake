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

if(ENABLE_DEBUGINFOD)
  set(_components debuginfod)
endif()

# As of CMake 3.12, the first three locations searched are <PackageName>_ROOT,
# CMAKE_PREFIX_PATH, then <PackageName>_DIR. We need to propagate the
# ElfUtils_ROOT_DIR to the first and third variables for LibELF and LibDW
set(_modules "ELF" "DW")
if(ENABLE_DEBUGINFOD)
  list(APPEND _modules "Debuginfod")
endif()
foreach(_n ${_modules})
  if(NOT Lib${_n}_ROOT AND NOT Lib${_n}_DIR)
    set(Lib${_n}_DIR
        ${ElfUtils_ROOT_DIR}
        CACHE PATH "" FORCE)
    mark_as_advanced(Lib${_n}_DIR)
  endif()
endforeach()

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
