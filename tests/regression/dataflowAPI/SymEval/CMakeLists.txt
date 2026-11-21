include_guard(GLOBAL)

dyninst_add_test_executable(
  symeval_expand_slice
  SOURCE_FILES expand_slice.cpp
  DEPS parseAPI
)

# One test per architecture over its findMain binaries, which are only read.
set(_dir "${DYNINST_TEST_BINARIES_DIR}/dyninstAPI/findMain")
foreach(
  _arch
  IN
  ITEMS aarch64 i686 ppc64le riscv64 x86_64
)
  set(_capstone)
  if(_arch STREQUAL "riscv64")
    set(_capstone capstone)
  endif()
  set(
    _binaries
    dyn.nopie.nostripped
    dyn.nopie.stripped
    dyn.pie.nostripped
    dyn.pie.stripped
    stat.nopie.nostripped
    stat.nopie.stripped
  )
  if(NOT _arch STREQUAL "ppc64le" AND NOT _arch STREQUAL "riscv64")
    list(APPEND _binaries stat.pie.nostripped stat.pie.stripped)
  endif()
  list(TRANSFORM _binaries PREPEND "${_dir}/${_arch}/")
  dyninst_add_test(
    dataflowAPI_symeval_expand_slice_${_arch}_test
    TYPE regression
    COMMAND symeval_expand_slice ${_binaries}
    REQUIRES test_binaries ${_capstone}
  )
endforeach()
unset(_capstone)
unset(_binaries)
unset(_dir)
