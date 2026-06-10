#===========================================================
#
# Configure Zydis
#
#   ----------------------------------------
#
# Zydis_ROOT_DIR - Location of the Zydis installation
#
# Zydis ships a CMake package config that exports the
# Zydis::Zydis target. If the user specifies Zydis_ROOT_DIR,
# we restrict the search to that location (or CMAKE_PREFIX_PATH).
#
#===========================================================

include_guard(GLOBAL)

if(Zydis_ROOT)
  set(zydis_ROOT ${Zydis_ROOT})
endif()

if(Zydis_ROOT_DIR)
  set(zydis_ROOT ${Zydis_ROOT_DIR})
  set(Zydis_ROOT ${Zydis_ROOT_DIR})
  mark_as_advanced(Zydis_ROOT_DIR)
  set(_find_path_args NO_CMAKE_SYSTEM_PATH NO_SYSTEM_ENVIRONMENT_PATH)
endif()

find_package(Zydis REQUIRED ${_find_path_args})

if(NOT TARGET Dyninst::Zydis)
  add_library(Dyninst::Zydis INTERFACE IMPORTED)
  target_link_libraries(Dyninst::Zydis INTERFACE Zydis::Zydis)
endif()

message(STATUS "Found Zydis ${Zydis_VERSION}")

unset(_find_path_args)
