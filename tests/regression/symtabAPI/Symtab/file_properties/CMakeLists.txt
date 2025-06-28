include_guard(GLOBAL)

include(DyninstLinkProperties)

# Use PIE when POSITION_INDEPENDENT_CODE is used for executables
if(POLICY CMP0083)
  cmake_policy(SET CMP0083 NEW)
endif()

include(CheckPIESupported)
check_pie_supported()

set(_all_tests "")

macro(make_lib_test test_type test_name)
  add_library(${test_name} ${test_type} test.cpp)
  list(APPEND _all_tests ${test_name})
endmacro()

macro(make_exec_test test_name)
  add_executable(${test_name} test.cpp)
  target_compile_definitions(${test_name} PRIVATE "HAS_MAIN")
  list(APPEND _all_tests ${test_name})
endmacro()

foreach(t "isExec" "isExecutable" "isSharedLibrary" "isStripped" "isPositionIndependent"
          "isStaticBinary")
  add_executable(${t} "${t}.cpp")
  target_compile_options(${t} PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
  target_link_libraries(${t} PRIVATE symtabAPI)
endforeach()

# Test shared libraries
foreach(main_t "hasmain" "none")
  foreach(strip_t "stripped" "none")

    # The '_pic' makes creating the test targets easier
    set(_targ "sharedlib_${main_t}_${strip_t}_pic")

    make_lib_test(SHARED ${_targ})

    if(strip_t STREQUAL "stripped")
      target_link_libraries(${_targ} "-s")
    endif()

    if(main_t STREQUAL "hasmain")
      target_compile_definitions(${_targ} PRIVATE "HAS_MAIN")
      target_compile_definitions(${_targ} PRIVATE "ADD_INTERP")
      target_link_options(${_targ} PRIVATE "-Wl,-e,main")
    endif()

  endforeach()
endforeach()

# Test static libraries
if(DYNINST_CXX_HAVE_STATIC_LINK_FLAG)
  make_lib_test(STATIC "staticlib")
endif()

# Test executables
foreach(strip_t "stripped" "none")
  foreach(pic_t "pic" "none")
    foreach(link_t "staticlink" "dynlink")

      if(link_t STREQUAL "staticlink" AND NOT DYNINST_CXX_HAVE_STATIC_LINK_FLAG)
        continue()
      endif()

      # static+pic isn't PIE
      if(link_t STREQUAL "staticlink" AND pic_t STREQUAL "pic")
        continue()
      endif()

      set(_targ "exectest_${strip_t}_${pic_t}_${link_t}")
      make_exec_test(${_targ})

      if(pic_t STREQUAL "pic")
        set_property(TARGET ${_targ} PROPERTY POSITION_INDEPENDENT_CODE TRUE)
      else()
        # some compilers use PIE by default
        target_compile_options(${_targ} PRIVATE "-no-pie")
        target_link_options(${_targ} PRIVATE "-no-pie")
      endif()

      if(strip_t STREQUAL "stripped")
        # '-s' works for gcc and clang
        target_link_libraries(${_targ} "-s")
      endif()

      if(link_t STREQUAL "staticlink")
        target_link_options(${_targ} PRIVATE "-static")
      endif()

    endforeach()
  endforeach()
endforeach()

foreach(test_name ${_all_tests})

  if(${test_name} MATCHES "sharedlib")
    set(test_file "lib${test_name}.so")
  else()
    set(test_file "${test_name}")
  endif()

  set(_targ "symtab_file_properties_isExec_${test_name}")
  # TODO: get __attribute__((section(".interp"))) working
  if(NOT ${test_name} MATCHES "sharedlib" AND NOT ${test_name} MATCHES "hasmain")
    add_test(NAME ${_targ} COMMAND isExec "${test_file}")
    if(NOT ${test_name} MATCHES "exectest")
      set_tests_properties(${_targ} PROPERTIES WILL_FAIL TRUE)
    endif()
    if(${test_name} MATCHES "staticlib")
      set_tests_properties(${_targ} PROPERTIES WILL_FAIL TRUE)
    endif()
    set_tests_properties(${_targ} PROPERTIES LABELS "regression")
  endif()
  set(_targ "symtab_file_properties_isExecutable_${test_name}")
  # TODO: get __attribute__((section(".interp"))) working
  if(NOT ${test_name} MATCHES "sharedlib" AND NOT ${test_name} MATCHES "hasmain")
    add_test(NAME ${_targ} COMMAND isExecutable "${test_file}")
    if(${test_name} MATCHES "staticlib")
      set_tests_properties(${_targ} PROPERTIES WILL_FAIL TRUE)
    endif()
    set_tests_properties(${_targ} PROPERTIES LABELS "regression")
  endif()

  set(_targ "symtab_file_properties_isSharedLibrary_${test_name}")
  add_test(NAME ${_targ} COMMAND isSharedLibrary "${test_file}")
  if(NOT ${test_name} MATCHES "sharedlib")
    set_tests_properties(${_targ} PROPERTIES WILL_FAIL TRUE)
  endif()
  set_tests_properties(${_targ} PROPERTIES LABELS "regression")

  set(_targ "symtab_file_properties_isStripped_${test_name}")
  add_test(NAME ${_targ} COMMAND isStripped "${test_file}")
  if(NOT ${test_name} MATCHES "stripped")
    set_tests_properties(${_targ} PROPERTIES WILL_FAIL TRUE)
  endif()
  set_tests_properties(${_targ} PROPERTIES LABELS "regression")

  set(_targ "symtab_file_properties_isPositionIndependent_${test_name}")
  add_test(NAME ${_targ} COMMAND isPositionIndependent "${test_file}")
  if(NOT ${test_name} MATCHES "pic")
    set_tests_properties(${_targ} PROPERTIES WILL_FAIL TRUE)
  endif()
  set_tests_properties(${_targ} PROPERTIES LABELS "regression")

  set(_targ "symtab_file_properties_isStaticBinary_${test_name}")
  add_test(NAME ${_targ} COMMAND isStaticBinary "${test_file}")
  if(NOT ${test_name} MATCHES "staticlink")
    set_tests_properties(${_targ} PROPERTIES WILL_FAIL TRUE)
  endif()
  set_tests_properties(${_targ} PROPERTIES LABELS "regression")

endforeach()
