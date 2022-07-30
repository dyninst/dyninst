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

if(BUILD_BOOST
   OR BUILD_TBB
   OR BUILD_ELFUTILS
   OR BUILD_LIBIBERTY)
    set(DYNINST_BUILDING_TPLS
        ON
        CACHE BOOL INTERNAL "Dyninst is building third-party library")
else()
    set(DYNINST_BUILDING_TPLS
        OFF
        CACHE BOOL INTERNAL "Dyninst is not building third-party library")
endif()

# here we configure: - the location of where we install the third-party libraries in the
# build tree (TPL_STAGING_PREFIX) - the location of where we install the third-party
# libraries in the install tree (TPL_INSTALL_PREFIX)
#
if(DYNINST_BUILDING_TPLS)
    # i.e. ran cmake once and then reconfigured to build tpls
    if("${DYNINST_LIBRARY_INSTALL_RPATH}" STREQUAL "\$ORIGIN")
        unset(DYNINST_LIBRARY_INSTALL_RPATH CACHE)
    endif()
    # internal cache variables. don't let the user configure so we
    set(TPL_STAGING_PREFIX
        "${PROJECT_BINARY_DIR}/tpls"
        CACHE INTERNAL "Third-party library build-tree install prefix")

    set(TPL_INSTALL_PREFIX
        "dyninst"
        CACHE INTERNAL "Third-party library install-tree install prefix")

    foreach(_TYPE BIN LIB INCLUDE CMAKE DOC)
        set(TPL_INSTALL_${_TYPE}_DIR
            "${TPL_INSTALL_PREFIX}/${INSTALL_${_TYPE}_DIR}"
            CACHE INTERNAL "")
        dyninst_sync_option(TPL_INSTALL_${_TYPE}_DIR)
    endforeach()

    # for packaging
    install(
        DIRECTORY ${TPL_STAGING_PREFIX}/include/
        DESTINATION ${INSTALL_LIB_DIR}/${TPL_INSTALL_INCLUDE_DIR}
        COMPONENT "tpls"
        OPTIONAL)

    install(
        DIRECTORY ${TPL_STAGING_PREFIX}/bin/
        DESTINATION ${INSTALL_LIB_DIR}/${TPL_INSTALL_BIN_DIR}
        COMPONENT "tpls"
        OPTIONAL)

    install(
        DIRECTORY ${TPL_STAGING_PREFIX}/share/
        DESTINATION ${INSTALL_LIB_DIR}/${TPL_INSTALL_DOC_DIR}
        COMPONENT "tpls"
        OPTIONAL)

    install(
        DIRECTORY ${TPL_STAGING_PREFIX}/lib/
        DESTINATION ${INSTALL_LIB_DIR}/${TPL_INSTALL_LIB_DIR}
        COMPONENT "tpls"
        OPTIONAL FILES_MATCHING
        PATTERN "*${CMAKE_SHARED_LIBRARY_SUFFIX}*"
        PATTERN "*${CMAKE_STATIC_LIBRARY_SUFFIX}")

    install(
        DIRECTORY ${TPL_STAGING_PREFIX}/lib/pkgconfig/
        DESTINATION ${INSTALL_LIB_DIR}/${TPL_INSTALL_LIB_DIR}/pkgconfig
        COMPONENT "tpls"
        OPTIONAL FILES_MATCHING
        PATTERN "*.pc")
endif()

if(UNIX AND NOT APPLE)
    list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib"
         isSystemDir)

    if("${isSystemDir}" STREQUAL "-1")
        if(DYNINST_BUILDING_TPLS)
            dyninst_add_cache_option(
                DYNINST_LIBRARY_INSTALL_RPATH "\$ORIGIN:\$ORIGIN/${TPL_INSTALL_LIB_DIR}"
                CACHE STRING "Dyninst library rpath")
        else()
            dyninst_add_cache_option(DYNINST_LIBRARY_INSTALL_RPATH "\$ORIGIN"
                                     CACHE STRING "Dyninst library rpath")
        endif()
    elseif(DYNINST_BUILDING_TPLS)
        dyninst_add_cache_option(
            DYNINST_LIBRARY_INSTALL_RPATH "\$ORIGIN/${TPL_INSTALL_LIB_DIR}"
            CACHE STRING "Dyninst library rpath")
    endif()
endif()
