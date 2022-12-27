# CMake configuration for symlite directory

if(NOT DYNINST_OS_UNIX)
    return()
endif()

include_directories(src ${PROJECT_SOURCE_DIR}/elf/h)

set(SRC_LIST src/SymLite-elf.C)
add_definitions(-DSYMLITE_LIB)

# add_library (symLite ${SRC_LIST}) add_library (symLite_static STATIC ${SRC_LIST}) FILE
# (GLOB headers "h/*.h") set_target_properties (symLite symLite_static PROPERTIES
# PUBLIC_HEADER "${headers}")

# target_link_private_libraries (symLite common) target_link_private_libraries (symLite
# dynElf)

# INSTALL (TARGETS symLite symLite_static EXPORT DyninstTargets RUNTIME DESTINATION
# ${DYNINST_INSTALL_LIBDIR} LIBRARY DESTINATION ${DYNINST_INSTALL_LIBDIR} ARCHIVE DESTINATION
# ${DYNINST_INSTALL_LIBDIR} PUBLIC_HEADER DESTINATION ${DYNINST_INSTALL_INCLUDEDIR})
dyninst_library(symLite common dynElf)
