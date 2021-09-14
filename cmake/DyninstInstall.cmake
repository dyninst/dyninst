# include guard
include_guard(GLOBAL)

include(CMakePackageConfigHelpers)

dyninst_get_property(ALL_DYNINST_TARGETS)

#------------------------------------------------------------------------------#
# install tree
#
set(PROJECT_INSTALL_DIR      ${CMAKE_INSTALL_PREFIX})
set(INCLUDE_INSTALL_DIR      ${INSTALL_INCLUDE_DIR})
set(LIB_INSTALL_DIR          ${INSTALL_LIB_DIR})

configure_package_config_file(
    ${PROJECT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in
    ${PROJECT_BINARY_DIR}/install-tree/${PROJECT_NAME}Config.cmake
    INSTALL_DESTINATION ${INSTALL_CMAKE_DIR}
    INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}
    PATH_VARS
        PROJECT_INSTALL_DIR
        INCLUDE_INSTALL_DIR
        LIB_INSTALL_DIR)

write_basic_package_version_file(
    ${PROJECT_BINARY_DIR}/install-tree/${PROJECT_NAME}Version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion)

install(
    FILES
        ${PROJECT_BINARY_DIR}/install-tree/${PROJECT_NAME}Config.cmake
        ${PROJECT_BINARY_DIR}/install-tree/${PROJECT_NAME}Version.cmake
    DESTINATION
        ${INSTALL_CMAKE_DIR}
    OPTIONAL)

export(PACKAGE ${PROJECT_NAME})

#------------------------------------------------------------------------------#
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

set(Dyninst_DIR ${PROJECT_BINARY_DIR}/build-tree CACHE PATH "Dyninst build-tree cmake directory" FORCE)

#------------------------------------------------------------------------------#
# packaging (when top-level project)
#
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set(PROJECT_VENDOR "Paradyn")
    set(PROJECT_CONTACT "bart@cs.wisc.edu")
    set(PROJECT_LICENSE_FILE "${PROJECT_SOURCE_DIR}/COPYRIGHT")
    set(PROJECT_PACKAGE_PREFIX "${CMAKE_INSTALL_PREFIX}" CACHE STRING "Packaging install prefix")
    configure_file(${PROJECT_SOURCE_DIR}/cmake/DyninstCPack.cmake.in
        ${PROJECT_BINARY_DIR}/install-tree/DyninstCPack.cmake @ONLY)

    include(${PROJECT_BINARY_DIR}/install-tree/DyninstCPack.cmake)
endif()
