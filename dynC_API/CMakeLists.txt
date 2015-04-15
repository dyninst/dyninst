
file(GLOB SRC_LIST "src/*.C")

include_directories(h 
  src 
  ${PROJECT_SOURCE_DIR}/dyninstAPI/src
  ${PROJECT_SOURCE_DIR}/proccontrol/h
)

set_source_files_properties(${SRC_LIST} PROPERTIES LANGUAGE CXX)
add_definitions(-DDYNC_EXPORTS)
add_library(dynC_API ${SRC_LIST})
add_library(dynC_API_static STATIC ${SRC_LIST})
file (GLOB headers "h/*.h")

set_target_properties(dynC_API dynC_API_static PROPERTIES PUBLIC_HEADER "${headers}")

target_link_private_libraries (dynC_API dyninstAPI)

install(TARGETS dynC_API dynC_API_static
		EXPORT DynCTargets
		RUNTIME DESTINATION ${INSTALL_LIB_DIR}
		LIBRARY DESTINATION ${INSTALL_LIB_DIR}		
		ARCHIVE DESTINATION ${INSTALL_LIB_DIR}
		PUBLIC_HEADER DESTINATION ${INSTALL_INCLUDE_DIR})

