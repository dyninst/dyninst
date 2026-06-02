include_guard(GLOBAL)

set(_headers parse_block.h parse_func.h Parsing.h)

set(_sources parse_block.C parse_func.C Parsing-arch.C Parsing.C)

add_library(dyninstAPI_parsing INTERFACE)
target_sources(dyninstAPI_parsing INTERFACE ${_headers} ${_sources})
target_include_directories(dyninstAPI_parsing INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
