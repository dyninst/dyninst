include_guard(GLOBAL)

# The checkers are built in every configuration; only the tests need test_binaries.
set(_bin_dir "${DYNINST_TEST_BINARIES_DIR}/symtabAPI/DWARF")

set(_targ dwarf5_inline_func)
dyninst_add_test_executable(${_targ} SOURCE_FILES dwarf5_inline_func.cpp DEPS symtabAPI)
dyninst_add_test(
  symtabAPI_${_targ}_test
  TYPE regression
  COMMAND ${_targ} "${_bin_dir}/dwarf5_inline_func"
  REQUIRES test_binaries
)

set(_targ subrange_bounds)
dyninst_add_test_executable(${_targ} SOURCE_FILES subrange_bounds.cpp DEPS symtabAPI)
set(_bin_dir "${_bin_dir}/subrange_bounds")
# C99 variable-length arrays: the lower bound is the C default (0), the
# upper bound or count is a runtime value.
set(_vla_expected "vla_1d:a=0:?" "vla_2d:b=0:?,0:?" "fixed:c=0:9" "fixed:d=0:2,0:3")
# Fortran: the lower bound defaults to 1; assumed-shape, allocatable, and
# explicit-shape arrays with non-constant bounds have runtime bounds. What a
# runtime lower bound (z, y) should be reported as is unsettled, so it is not
# checked.
set(
  _fortran_expected
  "MAIN__:z=*:?"
  "__shapes_MOD_assumed_shape:x=1:?"
  "__shapes_MOD_explicit_dyn:y=*:?"
  "__shapes_MOD_assumed_size:v=1:?"
  "__shapes_MOD_fixed:w=3:7"
  "__shapes_MOD_fixed:u=1:10"
)
foreach(_bin vla_gcc_O0 vla_gcc_O2 vla_clang_O0)
  dyninst_add_test(
    symtabAPI_${_targ}_${_bin}_test
    TYPE regression
    COMMAND ${_targ} "${_bin_dir}/${_bin}" ${_vla_expected}
    REQUIRES test_binaries
  )
endforeach()
# VLAs whose size is a known constant: clang -O1 gives DW_AT_count as a
# reference to a DIE with DW_AT_const_value, gcc -O1 gives DW_AT_upper_bound
# as a single constant operation (DW_OP_lit6, DW_OP_const2u).
foreach(_bin const_ref_clang_O1 const_ref_gcc_O1)
  dyninst_add_test(
    symtabAPI_${_targ}_${_bin}_test
    TYPE regression
    COMMAND ${_targ} "${_bin_dir}/${_bin}" "const_vla:a=0:6" "const_vla_300:b=0:299"
    REQUIRES test_binaries
  )
endforeach()
foreach(_bin fortran_gfortran_O0 fortran_gfortran_dwarf3 fortran_gfortran_relname)
  dyninst_add_test(
    symtabAPI_${_targ}_${_bin}_test
    TYPE regression
    COMMAND ${_targ} "${_bin_dir}/${_bin}" ${_fortran_expected}
    REQUIRES test_binaries
  )
endforeach()
# The same source built with flang, which names symbols differently and
# describes 'u(10)' with DW_AT_count and no lower bound.
dyninst_add_test(
  symtabAPI_${_targ}_fortran_flang_O0_test
  TYPE regression
  COMMAND
    ${_targ} "${_bin_dir}/fortran_flang_O0" "_QQmain:z=*:?"
    "_QMshapesPassumed_shape:x=1:?" "_QMshapesPexplicit_dyn:y=*:?"
    "_QMshapesPassumed_size:v=1:?" "_QMshapesPfixed:w=3:7" "_QMshapesPfixed:u=1:10"
  REQUIRES test_binaries
)

# 32 threads catches a shared libdw handle even on a two-core machine. Each
# run is a fresh process; the serial parse and eight parallel ones take about
# a second, even on two cores. A walk that corrupts libdw's trees can loop
# forever instead of crashing, hence the timeout.
#
# shared_types_dwz*.so keep their shared types in partial units of their own
# (single-file dwz). Those with a 5 are DWARF 5, the others DWARF 4.
set(_targ parallel_parse)
dyninst_add_test_executable(${_targ} SOURCE_FILES parallel_parse.cpp DEPS symtabAPI)
set(_bin_dir "${DYNINST_TEST_BINARIES_DIR}/symtabAPI/DWARF/parallel_parse")
foreach(_bin shared_types_dwz shared_types_dwz5)
  dyninst_add_test(
    symtabAPI_${_targ}_${_bin}_test
    TYPE regression
    COMMAND ${_targ} "${_bin_dir}/${_bin}.so" 32 8
    REQUIRES test_binaries
    PROPERTIES TIMEOUT 60
  )
endforeach()
