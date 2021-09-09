
file(GLOB SRC_LIST "src/*.C")

set_source_files_properties(${SRC_LIST} PROPERTIES LANGUAGE CXX)

if(WIN32)
    set(_PRIVATE_COMPILE_DEFS YY_NO_UNISTD_H)
endif()

dyninst_library(dynC_API
    SOURCES
        ${SRC_LIST}
    COMPILE_DEFINITIONS
        PRIVATE
            DYNC_EXPORTS
            ${_PRIVATE_COMPILE_DEFS}
    INCLUDE_DIRECTORIES
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/h>
        $<INSTALL_INTERFACE:${INSTALL_INCLUDE_DIR}>
    LINK_LIBRARIES
        Dyninst::dyninstAPI
        Dyninst::pcontrol
        Dyninst::Boost)
