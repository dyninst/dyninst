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
set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

set(INSTALL_LIB_DIR "lib")
set(INSTALL_INCLUDE_DIR "include")
set(INSTALL_CMAKE_DIR "${INSTALL_LIB_DIR}/cmake/${PROJECT_NAME}")
