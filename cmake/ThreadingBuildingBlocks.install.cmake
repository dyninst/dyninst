#################################################################
# ThreadingBuildingBlocks.cmake
#
# Install Intel's Threading Building Blocks for Dyninst
#
# The default TBB build does not have an 'install' target, so we
# have to do it manually. This file contains the necessary CMake
# commands to complete the installation assuming it has been built
# using ExternalProject_Add.
#
#################################################################

file(MAKE_DIRECTORY ${LIBDIR} ${INCDIR})
file(COPY ${PREFIX}/src/tbb_release/ DESTINATION ${LIBDIR} FILES_MATCHING PATTERN "*.so.*")
file(COPY ${PREFIX}/src/TBB/include/tbb DESTINATION ${INCDIR})

file(GLOB _tbb_libs ${LIBDIR}/libtbb*.so.*)

foreach(l ${_tbb_libs})
  string(REGEX REPLACE "\\.2$" "" _l_short ${l})
  execute_process(
    COMMAND ${CMAKE_COMMAND} -E create_symlink ${l} ${_l_short}
  )
endforeach()
