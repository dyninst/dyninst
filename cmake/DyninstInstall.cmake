include(CMakePackageConfigHelpers)

# Export the Find modules for thirdy-party libraries provided by us
install(DIRECTORY ${DYNINST_ROOT}/cmake/Modules DESTINATION ${INSTALL_CMAKE_DIR})
install(DIRECTORY ${DYNINST_ROOT}/cmake/tpls DESTINATION ${INSTALL_CMAKE_DIR})

configure_file(cmake/version.h.in common/h/dyninstversion.h)
