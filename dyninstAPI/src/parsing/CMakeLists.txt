include_guard(GLOBAL)

set(_headers DynCFGFactory.h DynParseCallback.h parse_block.h parse_func.h Parsing.h)

set(_sources DynCFGFactory.C DynParseCallback.C parse_block.C parse_func.C Parsing-arch.C
             Parsing.C)

# cmake-format: off
dyninst_library(
  dyninstAPI_parsing
  INTERNAL_LIBRARY
  SOURCE_FILES ${_sources}
  PRIVATE_HEADER_FILES ${_headers}
  DYNINST_INTERNAL_DEPS common patchAPI pcontrol
)

set(_inc_dirs
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/..
  ${CMAKE_CURRENT_SOURCE_DIR}/../Relocation
  ${CMAKE_CURRENT_SOURCE_DIR}/../../h
)
# cmake-format: on

target_include_directories(dyninstAPI_parsing PRIVATE $<BUILD_INTERFACE:${_inc_dirs}>)

unset(_headers)
unset(_sources)
