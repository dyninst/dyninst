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
file(
    COPY ${PREFIX}/src/tbb_release/
    DESTINATION ${LIBDIR}
    FILES_MATCHING
    PATTERN "*.so.*")
file(COPY ${PREFIX}/src/TBB-External/include/tbb DESTINATION ${INCDIR})
file(GLOB _tbb_libs ${LIBDIR}/libtbb*.so.*)

foreach(_lib ${_tbb_libs})
    string(REGEX REPLACE "\\.2$" "" _lib_short ${_lib})
    get_filename_component(_lib "${_lib}" NAME)
    execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${_lib} ${_lib_short})
endforeach()
