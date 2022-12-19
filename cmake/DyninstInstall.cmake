include(CMakePackageConfigHelpers)

# Export the Find modules for thirdy-party libraries provided by us
install(DIRECTORY ${DYNINST_ROOT}/cmake/Modules DESTINATION ${INSTALL_CMAKE_DIR})
install(DIRECTORY ${DYNINST_ROOT}/cmake/tpls DESTINATION ${INSTALL_CMAKE_DIR})

configure_file(cmake/version.h.in common/h/dyninstversion.h)

write_basic_package_version_file(
    ${PROJECT_BINARY_DIR}/DyninstConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion)

install(
    FILES ${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
    DESTINATION ${INSTALL_CMAKE_DIR})
