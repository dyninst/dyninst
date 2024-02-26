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

if(DYNINST_EXPORT_ALL)
  set(CMAKE_C_VISIBILITY_PRESET default)
  set(CMAKE_CXX_VISIBILITY_PRESET default)
  set(CMAKE_VISIBILITY_INLINES_HIDDEN OFF)
else()
  set(CMAKE_C_VISIBILITY_PRESET hidden)
  set(CMAKE_CXX_VISIBILITY_PRESET hidden)
  set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)
endif()

set(DYNINST_INSTALL_BINDIR "bin")
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
