
file(GLOB SRC_LIST "src/*.C")

include_directories(h 
  src 
  ${PROJECT_SOURCE_DIR}/dyninstAPI/src
  ${PROJECT_SOURCE_DIR}/proccontrol/h
)

set_source_files_properties(${SRC_LIST} PROPERTIES LANGUAGE CXX)
add_definitions(-DDYNC_EXPORTS)
if(WIN32)
add_definitions(-DYY_NO_UNISTD_H)
endif()

dyninst_library(dynC_API dyninstAPI)
if (USE_COTIRE)
    cotire(dynC_API)
endif()
