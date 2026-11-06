include_guard(GLOBAL)

dyninst_add_test_executable(
  symeval_expand_slice
  SOURCE_FILES expand_slice.cpp
  DEPS parseAPI
)

# One test per architecture over all of its findMain binaries. They are only
# read, so they run on every host.
set(_dir "${DYNINST_TEST_BINARIES_DIR}/dyninstAPI/findMain")
foreach(
  _arch
  IN
  ITEMS aarch64 i686 ppc64le riscv64 x86_64
)
  # StackAnalysis's entry state is chosen by the build's code-generation
  # architecture: a stack height of 0 in POWER and RISC-V builds, minus the
  # return address in x86 builds, none in native aarch64 builds. aarch64 code
  # skips StackAnalysis.
  set(_requires test_binaries)
  if(_arch STREQUAL "i686" OR _arch STREQUAL "x86_64")
    list(APPEND _requires codegen:i386)
  elseif(NOT _arch STREQUAL "aarch64")
    list(APPEND _requires stackanalysis_any_arch)
  endif()
  if(_arch STREQUAL "riscv64")
    list(APPEND _requires capstone)
  endif()
  file(GLOB _binaries "${_dir}/${_arch}/*")
  list(SORT _binaries)
  dyninst_add_test(
    dataflowAPI_symeval_expand_slice_${_arch}_test
    TYPE regression
    COMMAND symeval_expand_slice ${_binaries}
    REQUIRES ${_requires}
  )
endforeach()
unset(_requires)
unset(_binaries)
unset(_dir)
