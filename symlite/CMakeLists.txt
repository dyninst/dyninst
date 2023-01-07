include_guard(GLOBAL)

include(DyninstLibrary)

if(NOT DYNINST_OS_UNIX)
  if(NOT TARGET symLite)
    add_library(symLite INTERFACE)
  endif()
  return()
endif()

set(_public_headers h/SymLite-elf.h)

set(_sources src/SymLite-elf.C)

# cmake-format: off
dyninst_library(
  symLite
  PUBLIC_HEADER_FILES ${_public_headers}
  SOURCE_FILES ${_sources}
  DEFINES SYMLITE_LIB
  DYNINST_DEPS common dynElf
)
# cmake-format: on

foreach(t ${symLite_TARGETS})
  target_include_directories(
    ${t}
    PUBLIC
      "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src;${CMAKE_CURRENT_SOURCE_DIR}/h>")
endforeach()
