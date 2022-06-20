# Sets the installation variables
include(DyninstUtilities)

dyninst_add_cache_option(INSTALL_BIN_DIR bin
                         CACHE PATH "Installation directory for executables" ADVANCED)
dyninst_add_cache_option(INSTALL_LIB_DIR lib
                         CACHE PATH "Installation directory for libraries" ADVANCED)
dyninst_add_cache_option(INSTALL_INCLUDE_DIR include
                         CACHE PATH "Installation directory for header files" ADVANCED)
dyninst_add_cache_option(INSTALL_CMAKE_DIR lib/cmake/${PROJECT_NAME}
                         CACHE PATH "Installation directory for CMake files" ADVANCED)
dyninst_add_cache_option(INSTALL_DOC_DIR share/doc
                         CACHE PATH "Installation directory for manuals" ADVANCED)

dyninst_add_cache_option(
    TPL_STAGING_PREFIX "${PROJECT_BINARY_DIR}/tpls"
    CACHE PATH "Third-party library build-tree install prefix" ADVANCED)
dyninst_add_cache_option(
    TPL_INSTALL_PREFIX "dyninst"
    CACHE STRING "Third-party library install-tree install prefix" ADVANCED)

set(_DYNINST_TPL_INSTALL_PREFIX_LAST_VALUE
    "${TPL_INSTALL_PREFIX}"
    CACHE STRING "")
mark_as_advanced(_DYNINST_TPL_INSTALL_PREFIX_LAST_VALUE)

foreach(_TYPE BIN LIB INCLUDE CMAKE DOC)
    dyninst_sync_option(TPL_INSTALL_${_TYPE}_DIR)

    # if DYNINST_TPL_INSTALL_PREFIX changed and set to default, unset the cache value
    if(NOT TPL_INSTALL_PREFIX STREQUAL _DYNINST_TPL_INSTALL_PREFIX_LAST_VALUE
       AND "${DYNINST_TPL_INSTALL_${_TYPE}_DIR}" STREQUAL
           "${_DYNINST_TPL_INSTALL_PREFIX_LAST_VALUE}/${INSTALL_${_TYPE}_DIR}")
        dyninst_unset_cache_option(TPL_INSTALL_${_TYPE}_DIR)
    endif()

    dyninst_add_cache_option(
        TPL_INSTALL_${_TYPE}_DIR "${TPL_INSTALL_PREFIX}/${INSTALL_${_TYPE}_DIR}"
        CACHE PATH "" ADVANCED)
endforeach()

set(_DYNINST_TPL_INSTALL_PREFIX_LAST_VALUE
    "${TPL_INSTALL_PREFIX}"
    CACHE INTERNAL "" FORCE)

if(BUILD_BOOST
   OR BUILD_TBB
   OR BUILD_ELFUTILS
   OR BUILD_LIBIBERTY)
    set(_DYNINST_BUILDING_TPLS
        ON
        CACHE BOOL INTERNAL "Dyninst is building third-party library")
else()
    set(_DYNINST_BUILDING_TPLS
        OFF
        CACHE BOOL INTERNAL "Dyninst is not building third-party library")
endif()

if(UNIX AND NOT APPLE)
    list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib"
         isSystemDir)

    if("${isSystemDir}" STREQUAL "-1")
        if(_DYNINST_BUILDING_TPLS)
            set(CMAKE_INSTALL_RPATH "\$ORIGIN:\$ORIGIN/${DYNINST_TPL_INSTALL_LIB_DIR}")
        else()
            set(CMAKE_INSTALL_RPATH "\$ORIGIN")
        endif()
    elseif(_DYNINST_BUILDING_TPLS)
        set(CMAKE_INSTALL_RPATH "\$ORIGIN/${DYNINST_TPL_INSTALL_LIB_DIR}")
    endif()
endif()
