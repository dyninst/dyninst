# include guard
include_guard(GLOBAL)

include(CMakePackageConfigHelpers)

dyninst_get_property(DYNINST_BUILD_TARGETS)
dyninst_get_property(DYNINST_INTERFACE_TARGETS)

# ------------------------------------------------------------------------------#
# install tree
#
set(PROJECT_INSTALL_DIR ${CMAKE_INSTALL_PREFIX})
set(INCLUDE_INSTALL_DIR ${INSTALL_INCLUDE_DIR})
set(LIB_INSTALL_DIR ${INSTALL_LIB_DIR})

configure_package_config_file(
    ${PROJECT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in
    ${PROJECT_BINARY_DIR}/install-tree/${PROJECT_NAME}Config.cmake
    INSTALL_DESTINATION ${INSTALL_CMAKE_DIR}
    INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}
    PATH_VARS PROJECT_INSTALL_DIR INCLUDE_INSTALL_DIR LIB_INSTALL_DIR)

write_basic_package_version_file(
    ${PROJECT_BINARY_DIR}/install-tree/${PROJECT_NAME}Version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion)

install(
    FILES ${PROJECT_BINARY_DIR}/install-tree/${PROJECT_NAME}Config.cmake
          ${PROJECT_BINARY_DIR}/install-tree/${PROJECT_NAME}Version.cmake
    DESTINATION ${INSTALL_CMAKE_DIR}
    OPTIONAL)

export(PACKAGE ${PROJECT_NAME})

# ------------------------------------------------------------------------------#
# build tree
#
file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/build-tree/include)

configure_package_config_file(
    ${PROJECT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in
    ${PROJECT_BINARY_DIR}/build-tree/${PROJECT_NAME}Config.cmake
    INSTALL_DESTINATION ${PROJECT_BINARY_DIR}/build-tree
    INSTALL_PREFIX ${PROJECT_BINARY_DIR}/build-tree)

write_basic_package_version_file(
    ${PROJECT_BINARY_DIR}/build-tree/${PROJECT_NAME}Version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion)

# ------------------------------------------------------------------------------#
# packaging (when top-level project)
#
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set(Dyninst_DIR
        ${CMAKE_INSTALL_PREFIX}/${INSTALL_CMAKE_DIR}
        CACHE PATH "Dyninst cmake directory" FORCE)

    set(PROJECT_VENDOR "Paradyn")
    set(PROJECT_CONTACT "bart@cs.wisc.edu")
    set(PROJECT_LICENSE_FILE "${PROJECT_SOURCE_DIR}/COPYRIGHT")
    set(PROJECT_PACKAGE_PREFIX
        "${CMAKE_INSTALL_PREFIX}"
        CACHE STRING "Packaging install prefix")

    # Add packaging directives
    set(CPACK_PACKAGE_NAME
        "${PROJECT_NAME}"
        CACHE STRING "")
    set(CPACK_PACKAGE_VENDOR
        "${PACKAGE_VENDOR}"
        CACHE STRING "")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
        "${PROJECT_DESCRIPTION}"
        CACHE STRING "")
    set(CPACK_PACKAGE_VERSION_MAJOR
        "${PROJECT_VERSION_MAJOR}"
        CACHE STRING "")
    set(CPACK_PACKAGE_VERSION_MINOR
        "${PROJECT_VERSION_MINOR}"
        CACHE STRING "")
    set(CPACK_PACKAGE_VERSION_PATCH
        "${PROJECT_VERSION_PATCH}"
        CACHE STRING "")
    set(CPACK_PACKAGE_HOMEPAGE_URL
        "${PROJECT_HOMEPAGE_URL}"
        CACHE STRING "")
    set(CPACK_PACKAGE_CONTACT
        "${PROJECT_CONTACT}"
        CACHE STRING "")
    set(CPACK_RESOURCE_FILE_LICENSE
        "${PROJECT_LICENSE_FILE}"
        CACHE STRING "")

    foreach(
        _VAR
        NAME
        VENDOR
        DESCRIPTION_SUMMARY
        VERSION_MAJOR
        VERSION_MINOR
        VERSION_PATCH
        HOMEPAGE_URL
        CONTACT)
        mark_as_advanced(CPACK_PACKAGE_${_VAR})
    endforeach()
    mark_as_advanced(CPACK_RESOURCE_FILE_LICENSE)

    # Debian package specific variables
    set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "${PROJECT_HOMEPAGE_URL}")

    set(_DEBIAN_PACKAGE_DEPENDS "")
    if(USE_OpenMP)
        list(APPEND _DEBIAN_PACKAGE_DEPENDS libgomp1)
    endif()
    if(NOT BUILD_BOOST)
        foreach(_BOOST_COMPONENT atomic system thread date-time filesystem)
            list(APPEND _DEBIAN_PACKAGE_DEPENDS
                 "libboost-${_BOOST_COMPONENT}-dev (>= ${Boost_MIN_VERSION})")
        endforeach()
    endif()
    if(NOT BUILD_TBB)
        list(APPEND _DEBIAN_PACKAGE_DEPENDS "libtbb-dev (>= ${TBB_MIN_VERSION})")
    endif()
    if(NOT BUILD_LIBIBERTY)
        list(APPEND _DEBIAN_PACKAGE_DEPENDS "libiberty-dev (>= 20170913)")
    endif()
    if(NOT BUILD_ELFUTILS)
        list(APPEND _DEBIAN_PACKAGE_DEPENDS "libelf-dev (>= ${ElfUtils_MIN_VERSION})")
    endif()
    string(REPLACE ";" ", " _DEBIAN_PACKAGE_DEPENDS "${_DEBIAN_PACKAGE_DEPENDS}")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS
        "${_DEBIAN_PACKAGE_DEPENDS}"
        CACHE STRING "Debian package dependencies" FORCE)

    macro(DYNINST_SET_CPACK_VARIABLE _VAR)
        if(DEFINED DYNINST_${_VAR} AND NOT DYNINST_${_VAR} STREQUAL "")
            set(CPACK_${_VAR} ${DYNINST_${_VAR}})
        elseif(DEFINED ENV{DYNINST_${_VAR}})
            set(CPACK_${_VAR} $ENV{DYNINST_${_VAR}})
        else()
            set(CPACK_${_VAR} "local")
        endif()
    endmacro()

    dyninst_set_cpack_variable(DEBIAN_PACKAGE_RELEASE)
    dyninst_set_cpack_variable(RPM_PACKAGE_RELEASE)

    # RPM package specific variables
    if(CPACK_PACKAGING_INSTALL_PREFIX)
        set(CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION
            "${CPACK_PACKAGING_INSTALL_PREFIX}")
    endif()

    # Get rpm distro
    if(CPACK_RPM_PACKAGE_RELEASE)
        set(CPACK_RPM_PACKAGE_RELEASE_DIST ON)
    endif()

    # Prepare final version for the CPACK use
    set(CPACK_PACKAGE_VERSION
        "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}"
        )

    # Set the names now using CPACK utility
    set(CPACK_DEBIAN_FILE_NAME "DEB-DEFAULT")
    set(CPACK_RPM_FILE_NAME "RPM-DEFAULT")

    include(CPack)
else()
    set(Dyninst_DIR
        ${PROJECT_BINARY_DIR}/build-tree
        CACHE PATH "Dyninst build-tree cmake directory" FORCE)
endif()
