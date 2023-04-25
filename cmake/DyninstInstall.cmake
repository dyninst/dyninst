include(CMakePackageConfigHelpers)

# Export the Find modules for thirdy-party libraries provided by us
install(DIRECTORY ${PROJECT_SOURCE_DIR}/cmake/Modules
        DESTINATION ${DYNINST_INSTALL_CMAKEDIR})
install(DIRECTORY ${PROJECT_SOURCE_DIR}/cmake/tpls
        DESTINATION ${DYNINST_INSTALL_CMAKEDIR})

configure_package_config_file(
  ${PROJECT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in
  ${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
  INSTALL_DESTINATION ${DYNINST_INSTALL_CMAKEDIR}
  INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}
  PATH_VARS DYNINST_INSTALL_LIBDIR DYNINST_INSTALL_INCLUDEDIR)

write_basic_package_version_file(
  ${PROJECT_BINARY_DIR}/DyninstConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion)

install(FILES ${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
              ${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
        DESTINATION ${DYNINST_INSTALL_CMAKEDIR})

# Export all of the Dyninst libraries created by `dyninst_library`
install(
  EXPORT dyninst-targets
  NAMESPACE Dyninst::
  FILE DyninstTargets.cmake
  DESTINATION ${DYNINST_INSTALL_CMAKEDIR})
