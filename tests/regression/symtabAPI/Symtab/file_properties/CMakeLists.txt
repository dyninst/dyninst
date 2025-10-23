include_guard(GLOBAL)

include(DyninstCheckStaticExeSupported)
dyninst_check_static_exe_supported(LANGUAGES CXX)

include(DyninstCheckPIESupported)
dyninst_check_pie_supported(LANGUAGES CXX)

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
make_lib_test(STATIC "staticlib")

if(DYNINST_CXX_STATIC_EXE_SUPPORTED)
  set(_static_link "staticlink")
endif()

# Test executables
foreach(strip_t "stripped" "none")
  foreach(pic_t "pic" "none")
    foreach(link_t ${_static_link} "dynlink")

      set(_targ "exectest_${strip_t}_${pic_t}_${link_t}")
      make_exec_test(${_targ})

      if(link_t STREQUAL "${_static_link}")
        if(pic_t STREQUAL "pic" AND DYNINST_CXX_STATIC_PIE_SUPPORTED)
          # use static-pie
          target_link_options(${_targ} PRIVATE ${DYNINST_CXX_STATIC_PIE_FLAG})
        else()
          target_link_options(${_targ} PRIVATE ${DYNINST_CXX_STATIC_EXE_LINK_FLAG})
        endif()
      else()
        set(_use_pic FALSE)
        if(pic_t STREQUAL "pic")
          set(_use_pic TRUE)
        endif()
        set_property(TARGET ${_targ} PROPERTY POSITION_INDEPENDENT_CODE ${_use_pic})
      endif()

      if(strip_t STREQUAL "stripped")
        # '-s' works for gcc and clang
        target_link_libraries(${_targ} "-s")
      endif()

    endforeach()
  endforeach()
endforeach()

foreach(test_name ${_all_tests})

  set(test_file $<TARGET_FILE:${test_name}>)

  set(_targ "symtab_file_properties_isExec_${test_name}")
  # TODO: get __attribute__((section(".interp"))) working
  if(NOT ${test_name} MATCHES "sharedlib" AND NOT ${test_name} MATCHES "hasmain")
    add_test(NAME ${_targ} COMMAND $<TARGET_FILE:isExec> "${test_file}")
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
    add_test(NAME ${_targ} COMMAND $<TARGET_FILE:isExecutable> "${test_file}")
    if(${test_name} MATCHES "staticlib")
      set_tests_properties(${_targ} PROPERTIES WILL_FAIL TRUE)
    endif()
    set_tests_properties(${_targ} PROPERTIES LABELS "regression")
  endif()

  set(_targ "symtab_file_properties_isSharedLibrary_${test_name}")
  add_test(NAME ${_targ} COMMAND $<TARGET_FILE:isSharedLibrary> "${test_file}")
  if(NOT ${test_name} MATCHES "sharedlib")
    set_tests_properties(${_targ} PROPERTIES WILL_FAIL TRUE)
  endif()
  set_tests_properties(${_targ} PROPERTIES LABELS "regression")

  set(_targ "symtab_file_properties_isStripped_${test_name}")
  add_test(NAME ${_targ} COMMAND $<TARGET_FILE:isStripped> "${test_file}")
  if(NOT ${test_name} MATCHES "stripped")
    set_tests_properties(${_targ} PROPERTIES WILL_FAIL TRUE)
  endif()
  set_tests_properties(${_targ} PROPERTIES LABELS "regression")

  set(_targ "symtab_file_properties_isPositionIndependent_${test_name}")
  add_test(NAME ${_targ} COMMAND ${CMAKE_COMMAND} -E env
                                 $<TARGET_FILE:isPositionIndependent> "${test_file}")
  if(NOT ${test_name} MATCHES "pic")
    set_tests_properties(${_targ} PROPERTIES WILL_FAIL TRUE)
  elseif(_static_link AND ${test_name} MATCHES "${_static_link}")
    if(NOT DYNINST_CXX_STATIC_PIE_SUPPORTED)
      set_tests_properties(${_targ} PROPERTIES WILL_FAIL TRUE)
    endif()
  endif()
  set_tests_properties(${_targ} PROPERTIES LABELS "regression")

  set(_targ "symtab_file_properties_isStaticBinary_${test_name}")
  add_test(NAME ${_targ} COMMAND $<TARGET_FILE:isStaticBinary> "${test_file}")
  if(NOT ${test_name} MATCHES "staticlink")
    set_tests_properties(${_targ} PROPERTIES WILL_FAIL TRUE)
  endif()
  set_tests_properties(${_targ} PROPERTIES LABELS "regression")

endforeach()
