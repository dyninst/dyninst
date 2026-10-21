include_guard(GLOBAL)

# The fixtures are aarch64 assembly (see start.S), built by the host toolchain and checked
# after stripping. findMain's aarch64 code is likewise built only on an aarch64 host, so a
# cross build (DYNINST_CODEGEN_ARCH=aarch64 on another host) has nothing to test here.
if(NOT DYNINST_HOST_ARCH_AARCH64 OR NOT DYNINST_CODEGEN_ARCH_AARCH64 OR NOT CMAKE_STRIP)
  return()
endif()

add_executable(findMainCheck findMainCheck.cpp)
target_compile_options(findMainCheck PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(findMainCheck PRIVATE dyninstAPI)

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
