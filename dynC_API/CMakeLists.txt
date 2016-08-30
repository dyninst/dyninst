
file(GLOB SRC_LIST "src/*.C")

include_directories(h 
  src 
  ${PROJECT_SOURCE_DIR}/dyninstAPI/src
  ${PROJECT_SOURCE_DIR}/proccontrol/h
)

set_source_files_properties(${SRC_LIST} PROPERTIES LANGUAGE CXX)
add_definitions(-DDYNC_EXPORTS)

dyninst_library(dynC_API dyninstAPI)
if (USE_COTIRE EQUAL true)
    cotire(dynC_API)
endif()
