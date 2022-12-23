file(GLOB SRC_LIST "src/*.C")

include_directories(h src ${PROJECT_SOURCE_DIR}/dyninstAPI/src
                    ${PROJECT_SOURCE_DIR}/proccontrol/h)

set_source_files_properties(${SRC_LIST} PROPERTIES LANGUAGE CXX)
add_definitions(-DDYNC_EXPORTS)
if(DYNINST_OS_Windows)
    add_definitions(-DYY_NO_UNISTD_H)
endif()

dyninst_library(dynC_API dyninstAPI symtabAPI)
target_link_libraries(dynC_API PRIVATE common)
