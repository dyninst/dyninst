include_guard(GLOBAL)

set(_targ symeval_expand_slice)
add_executable(${_targ} expand_slice.cpp)
target_compile_options(${_targ} PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(${_targ} PRIVATE parseAPI)

# One test per architecture over all of its findMain binaries. They are only
# read, so they run on every host. Without the test binaries no path is passed,
# and the test reports a skip.
set(_dir "${DYNINST_TEST_BINARIES_DIR}/dyninstAPI/findMain")
foreach(
  _arch
  IN
  ITEMS aarch64 i686 ppc64le riscv64 x86_64
)
  set(_args)
  if(DYNINST_CODEGEN_ARCH_AARCH64 AND NOT _arch STREQUAL "aarch64")
    # StackAnalysis's entry state is chosen by the build's code-generation
    # architecture and has none for aarch64 builds.
    set(_args --skip "an aarch64 build cannot run StackAnalysis on ${_arch} code")
  elseif(_arch STREQUAL "riscv64" AND NOT DYNINST_ENABLE_CAPSTONE)
    set(_args --skip "riscv64 code needs Capstone")
  elseif(DYNINST_ENABLE_EXTERNAL_BINARY_TESTS)
    file(GLOB _args "${_dir}/${_arch}/*")
    list(SORT _args)
  endif()
  set(_tname dataflowAPI_${_targ}_${_arch}_test)
  add_test(NAME ${_tname} COMMAND ${_targ} ${_args})
  set_tests_properties(
    ${_tname}
    PROPERTIES LABELS "regression" SKIP_RETURN_CODE 77 SKIP_REGULAR_EXPRESSION "skipping:"
  )
endforeach()
