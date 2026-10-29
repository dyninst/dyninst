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

foreach(
  t
  "isExec"
  "isExecutable"
  "isSharedLibrary"
  "isStripped"
  "isPositionIndependent"
  "isStaticBinary"
)
  dyninst_add_test_executable(${t} SOURCE_FILES "${t}.cpp" DEPS symtabAPI)
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

# Test executables. The staticlink ones are built only if the toolchain can link a
# static executable; their tests are registered either way, and skipped without them.
dyninst_test_skip_reason(_no_static_exe REQUIRES static_exe)
foreach(strip_t "stripped" "none")
  foreach(pic_t "pic" "none")
    foreach(link_t "staticlink" "dynlink")
      set(_targ "exectest_${strip_t}_${pic_t}_${link_t}")
      if(link_t STREQUAL "staticlink" AND _no_static_exe)
        list(APPEND _all_tests ${_targ})
        continue()
      endif()
      make_exec_test(${_targ})
      if(link_t STREQUAL "staticlink")
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

# Registers the test of <checker> on <test_name>, expected to fail if <will_fail>
function(_file_properties_test checker test_name will_fail)
  set(_requires)
  if("${test_name}" MATCHES "staticlink")
    set(_requires static_exe)
  endif()
  set(_props)
  if(will_fail)
    set(_props WILL_FAIL TRUE)
  endif()
  dyninst_add_test(
    symtab_file_properties_${checker}_${test_name}
    TYPE regression
    COMMAND ${ARGN} "$<TARGET_FILE:${test_name}>"
    REQUIRES ${_requires}
    PROPERTIES ${_props}
  )
endfunction()

foreach(test_name ${_all_tests})
  # TODO: get __attribute__((section(".interp"))) working
  if(NOT "${test_name}" MATCHES "sharedlib" AND NOT "${test_name}" MATCHES "hasmain")
    set(_fail FALSE)
    if(NOT "${test_name}" MATCHES "exectest" OR "${test_name}" MATCHES "staticlib")
      set(_fail TRUE)
    endif()
    _file_properties_test(isExec ${test_name} ${_fail} $<TARGET_FILE:isExec>)
  endif()

  # TODO: get __attribute__((section(".interp"))) working
  if(NOT "${test_name}" MATCHES "sharedlib" AND NOT "${test_name}" MATCHES "hasmain")
    set(_fail FALSE)
    if("${test_name}" MATCHES "staticlib")
      set(_fail TRUE)
    endif()
    _file_properties_test(isExecutable ${test_name} ${_fail} $<TARGET_FILE:isExecutable>)
  endif()

  set(_fail FALSE)
  if(NOT "${test_name}" MATCHES "sharedlib")
    set(_fail TRUE)
  endif()
  _file_properties_test(
    isSharedLibrary
    ${test_name}
    ${_fail}
    $<TARGET_FILE:isSharedLibrary>
  )

  set(_fail FALSE)
  if(NOT "${test_name}" MATCHES "stripped")
    set(_fail TRUE)
  endif()
  _file_properties_test(isStripped ${test_name} ${_fail} $<TARGET_FILE:isStripped>)

  set(_fail FALSE)
  if(NOT "${test_name}" MATCHES "pic")
    set(_fail TRUE)
  elseif("${test_name}" MATCHES "staticlink" AND NOT DYNINST_CXX_STATIC_PIE_SUPPORTED)
    set(_fail TRUE)
  endif()
  _file_properties_test(
    isPositionIndependent
    ${test_name}
    ${_fail}
    ${CMAKE_COMMAND}
    -E
    env
    $<TARGET_FILE:isPositionIndependent>
  )

  set(_fail FALSE)
  if(NOT "${test_name}" MATCHES "staticlink")
    set(_fail TRUE)
  endif()
  _file_properties_test(
    isStaticBinary
    ${test_name}
    ${_fail}
    $<TARGET_FILE:isStaticBinary>
  )
endforeach()
