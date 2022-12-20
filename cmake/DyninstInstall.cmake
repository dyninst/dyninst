include(CMakePackageConfigHelpers)

# Export the Find modules for thirdy-party libraries provided by us
install(DIRECTORY ${DYNINST_ROOT}/cmake/Modules DESTINATION ${INSTALL_CMAKE_DIR})
install(DIRECTORY ${DYNINST_ROOT}/cmake/tpls DESTINATION ${INSTALL_CMAKE_DIR})

configure_package_config_file(
    ${PROJECT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in
    ${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
    INSTALL_DESTINATION ${INSTALL_CMAKE_DIR}
    INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}
    PATH_VARS INSTALL_LIB_DIR INSTALL_INCLUDE_DIR)

write_basic_package_version_file(
    ${PROJECT_BINARY_DIR}/DyninstConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion)

install(
    FILES ${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
          ${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
    DESTINATION ${INSTALL_CMAKE_DIR})
