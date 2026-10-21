include_guard(GLOBAL)

# findMain only reads the binary and is chosen by the binary's architecture, so these tests
# open aarch64 executables on any host and for any code-generation target.  Each checks that
# findMain names, in a stripped executable, the main of its unstripped twin.
add_executable(findMainCheck findMainCheck.cpp)
target_compile_options(findMainCheck PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(findMainCheck PRIVATE dyninstAPI)

# Prebuilt aarch64 executables from the test_binaries repository, each a pair
# <name>.stripped / <name>.nostripped: one test per pair, named after it, so a pair added
# there is tested once CMake is run again.
if(DYNINST_ENABLE_EXTERNAL_BINARY_TESTS)
  set(_fpath "${DYNINST_TEST_BINARIES_DIR}/dyninstAPI/findMain/aarch64")
  file(GLOB _stripped RELATIVE "${_fpath}" "${_fpath}/*.stripped")
  foreach(_file ${_stripped})
    string(REGEX REPLACE "\\.stripped$" "" _binary "${_file}")
    set(_tname dyninstAPI_findMain_aarch64_${_binary})
    add_test(
      NAME ${_tname}
      COMMAND findMainCheck "${_fpath}/${_binary}.nostripped" "${_fpath}/${_file}"
    )
    set_tests_properties(
      ${_tname}
      PROPERTIES
        LABELS "regression"
        ENVIRONMENT "DYNINSTAPI_RT_LIB=$<TARGET_FILE:dyninstAPI_RT>"
    )
  endforeach()
endif()

# Fixtures for the _start forms the prebuilt set does not have (see start.S).  They are
# aarch64 assembly built by the host toolchain, so they need an aarch64 host.
if(NOT DYNINST_HOST_ARCH_AARCH64 OR NOT CMAKE_STRIP)
  return()
endif()

foreach(
  _form
  adrp
  got
  literal
  wrap_main
  wrap_main_hint
  not_glibc
)
  set(_fixture findMain_${_form})
  add_executable(${_fixture} start.S)
  target_compile_definitions(${_fixture} PRIVATE FORM_${_form})
  if(_form STREQUAL "got")
    set_target_properties(${_fixture} PROPERTIES POSITION_INDEPENDENT_CODE ON)
    target_link_options(${_fixture} PRIVATE -nostdlib -pie)
  else()
    set_target_properties(${_fixture} PROPERTIES POSITION_INDEPENDENT_CODE OFF)
    target_link_options(${_fixture} PRIVATE -nostdlib -static -no-pie)
  endif()
  add_custom_command(
    TARGET ${_fixture}
    POST_BUILD
    COMMAND
      ${CMAKE_STRIP} -o $<TARGET_FILE:${_fixture}>.stripped $<TARGET_FILE:${_fixture}>
  )

  set(_args $<TARGET_FILE:${_fixture}> $<TARGET_FILE:${_fixture}>.stripped)
  if(_form STREQUAL "not_glibc")
    list(APPEND _args --no-main)
  endif()
  add_test(NAME dyninstAPI_findMain_${_form} COMMAND findMainCheck ${_args})
  set_tests_properties(
    dyninstAPI_findMain_${_form}
    PROPERTIES
      LABELS "regression"
      ENVIRONMENT "DYNINSTAPI_RT_LIB=$<TARGET_FILE:dyninstAPI_RT>"
  )
endforeach()
