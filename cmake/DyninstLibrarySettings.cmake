# -------------------------------------------------------------------
#
# Various build features of the Dyninst libraries
#
#  The values here have a wide range of effects, so this file should
#  be included before any other configurations are done.
#
# -------------------------------------------------------------------

include_guard(GLOBAL)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

set(BUILD_SHARED_LIBS ON)

set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

set(DYNINST_INSTALL_LIBDIR "lib")
set(DYNINST_INSTALL_INCLUDEDIR "include")
set(DYNINST_INSTALL_CMAKEDIR "${DYNINST_INSTALL_LIBDIR}/cmake/Dyninst")

# -- Set up the RPATH ---
#
# General guidelines:
#  https://gitlab.kitware.com/cmake/community/-/wikis/doc/cmake/RPATH-handling
#
# '$ORIGIN' is a special CMake variable that looks up the location of a package's
# private libraries via a relative expression so as to not lose the capability of
# providing a fully relocatable package
#
# CMP0060 is active and so libraries are linked by their full paths even in
# implicit directories (e.g., /usr/lib/foo.so instead of -lfoo)

# Populate RPATHs for binaries in the build tree
set(CMAKE_SKIP_BUILD_RPATH FALSE)

# Do not use the install path as the RPATH (this is what $ORIGIN is for)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

# Add paths to any directories outside the project that are in the linker
# search path or contain linked library files
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# Always include $ORIGIN
set(DYNINST_RPATH_DIRECTORIES "\$ORIGIN")

# Sometimes CMake uses RPATH instead of RUNPATH which prevents overriding the search
# paths when the user explicitly sets LD_LIBRARY_PATH. If Dyninst is installed into a
# system directory (e.g., /usr/lib) and CMake uses RPATH, the loader would find any
# needed libraries there first, even if the ones linked against actually live elsewhere.
set(_system_dirs CMAKE_C_IMPLICIT_LINK_DIRECTORIES CMAKE_CXX_IMPLICIT_LINK_DIRECTORIES
          CMAKE_Fortran_IMPLICIT_LINK_DIRECTORIES)
list(SORT _system_dirs)
list(REMOVE_DUPLICATES _system_dirs)

set(_loc "${CMAKE_INSTALL_PREFIX}/${DYNINST_INSTALL_LIBDIR}")
if(NOT ${_loc} IN_LIST _system_dirs)
  list(APPEND DYNINST_RPATH_DIRECTORIES ${_loc})
endif()
unset(_loc)
