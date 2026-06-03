include_guard(GLOBAL)

set(_headers block.h function.h instMapping.h instPoint.h patch.h Patching.h)

set(_sources block.C edge.C function.C instPoint.C Patching.C)

# cmake-format: off
dyninst_library(
  dyninstAPI_patching
  INTERNAL_LIBRARY
  SOURCE_FILES ${_sources}
  PRIVATE_HEADER_FILES ${_headers}
  DYNINST_INTERNAL_DEPS common parseAPI patchAPI pcontrol
)

set(_inc_dirs
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/..
  ${CMAKE_CURRENT_SOURCE_DIR}/../../h
)
# cmake-format: on

target_include_directories(dyninstAPI_patching PRIVATE $<BUILD_INTERFACE:${_inc_dirs}>)

unset(_headers)
unset(_sources)
