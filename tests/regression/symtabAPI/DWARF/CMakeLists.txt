include_guard(GLOBAL)

set(_targ dwarf5_inline_func)
add_executable(${_targ} dwarf5_inline_func.cpp)
target_compile_options(${_targ} PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(${_targ} PRIVATE symtabAPI)

if(DYNINST_ENABLE_EXTERNAL_BINARY_TESTS)
  add_test(NAME symtabAPI_${_targ}_test
           COMMAND ${_targ}
                   "${DYNINST_TEST_BINARIES_DIR}/symtabAPI/DWARF/dwarf5_inline_func")
  set_tests_properties(symtabAPI_${_targ}_test PROPERTIES LABELS "regression")
endif()

set(_targ subrange_bounds)
add_executable(${_targ} subrange_bounds.cpp)
target_compile_options(${_targ} PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(${_targ} PRIVATE symtabAPI)

if(DYNINST_ENABLE_EXTERNAL_BINARY_TESTS)
  set(_bin_dir "${DYNINST_TEST_BINARIES_DIR}/symtabAPI/DWARF/subrange_bounds")

  # C99 variable-length arrays: the lower bound is the C default (0), the
  # upper bound or count is a runtime value.
  set(_vla_expected "vla_1d:a=0:?" "vla_2d:b=0:?,0:?" "fixed:c=0:9"
                    "fixed:d=0:2,0:3")

  # Fortran: the lower bound defaults to 1; assumed-shape, allocatable, and
  # explicit-shape arrays with non-constant bounds have runtime bounds. What a
  # runtime lower bound (z, y) should be reported as is unsettled, so it is not
  # checked.
  set(_fortran_expected
      "MAIN__:z=*:?" "__shapes_MOD_assumed_shape:x=1:?"
      "__shapes_MOD_explicit_dyn:y=*:?" "__shapes_MOD_assumed_size:v=1:?"
      "__shapes_MOD_fixed:w=3:7" "__shapes_MOD_fixed:u=1:10")

  foreach(_bin vla_gcc_O0 vla_gcc_O2 vla_clang_O0)
    add_test(NAME symtabAPI_${_targ}_${_bin}_test
             COMMAND ${_targ} "${_bin_dir}/${_bin}" ${_vla_expected})
    set_tests_properties(symtabAPI_${_targ}_${_bin}_test PROPERTIES LABELS "regression")
  endforeach()

  foreach(_bin fortran_gfortran_O0 fortran_gfortran_dwarf3 fortran_gfortran_relname)
    add_test(NAME symtabAPI_${_targ}_${_bin}_test
             COMMAND ${_targ} "${_bin_dir}/${_bin}" ${_fortran_expected})
    set_tests_properties(symtabAPI_${_targ}_${_bin}_test PROPERTIES LABELS "regression")
  endforeach()

  # The same source built with flang, which names symbols differently and
  # describes 'u(10)' with DW_AT_count and no lower bound.
  add_test(NAME symtabAPI_${_targ}_fortran_flang_O0_test
           COMMAND ${_targ} "${_bin_dir}/fortran_flang_O0"
                   "_QQmain:z=*:?" "_QMshapesPassumed_shape:x=1:?"
                   "_QMshapesPexplicit_dyn:y=*:?" "_QMshapesPassumed_size:v=1:?"
                   "_QMshapesPfixed:w=3:7" "_QMshapesPfixed:u=1:10")
  set_tests_properties(symtabAPI_${_targ}_fortran_flang_O0_test PROPERTIES LABELS "regression")
endif()
