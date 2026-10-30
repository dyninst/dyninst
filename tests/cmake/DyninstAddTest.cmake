#[=======================================================================[
DyninstAddTest
--------------

Functions for adding Dyninst's tests to ctest. tests/README.md describes
them in full, with examples.

  dyninst_add_test

    dyninst_add_test(<TestName>
      TYPE unit|integration|regression
      SOURCE_FILES <file>... | COMMAND <command> [<arg>...]
      [ARGS <arg>...]
      [DEPS <target>...]
      [DEFINES <val>...]
      [COMPILE_OPTIONS <option>...]
      [REQUIRES <requirement>...]
      [PROPERTIES <property> <value>...]
    )

  Creates a test named <TestName> with CTest's add_test, labeled TYPE,
  one of the test types DYNINST_ENABLE_TESTS selects.

  If a test can only be built or run when a configure-time value is set,
  such as DYNINST_ENABLE_EXTERNAL_BINARY_TESTS, name it in REQUIRES (see
  dyninst_test_requirement below). When a requirement is not met, nothing
  is built for the test, and a placeholder with the same name reports
  that it was skipped and why. A test that can only tell when it runs
  leaves REQUIRES out and skips itself with skip_message() from
  tests/dyninst_test_skip.h. A test is skipped when it exits with 77.

  For example,

    set(_flags ${SUPPORTED_CXX_WARNING_FLAGS})
    list(APPEND _flags "-Wframe-larger-than=50000")
    add_executable(aarch64_bits_test bits.cpp)
    target_compile_options(aarch64_bits_test PRIVATE ${_flags})
    target_link_libraries(
      aarch64_bits_test
      PRIVATE instructionAPI register_tests opcode_tests
    )
    add_test(NAME instructionAPI_aarch64_bits_test COMMAND aarch64_bits_test)
    set_tests_properties(instructionAPI_aarch64_bits_test PROPERTIES LABELS "integration")

  becomes

    dyninst_add_test(
      instructionAPI_aarch64_bits_test
      TYPE integration
      SOURCE_FILES bits.cpp
      DEPS instructionAPI register_tests opcode_tests
      COMPILE_OPTIONS -Wframe-larger-than=50000
    )

  dyninst_add_test always adds SUPPORTED_CXX_WARNING_FLAGS.

  TYPE
    unit, integration or regression; becomes the test's LABELS. A test
    that REQUIRES host:<arch> is also labeled host.

  SOURCE_FILES
    The sources of an executable named <TestName>, which the test runs.

  COMMAND
    A command to run instead, with its arguments. It is usually an
    executable from dyninst_add_test_executable (see below), named
    directly or as $<TARGET_FILE:...>; if that executable was not built
    because a requirement is not met, the test is skipped too.

  ARGS
    Command line arguments for the executable built from SOURCE_FILES.

  DEPS
    Link dependencies, as CMake target names: Dyninst libraries, the test
    helper libraries, and others such as Threads::Threads.

  DEFINES, COMPILE_OPTIONS
    Compiler flags used to build SOURCE_FILES (see
    dyninst_add_test_executable below).

  REQUIRES
    Requirements the test needs (see dyninst_test_requirement below).

  PROPERTIES
    Test properties, passed to set_tests_properties. A test that sets up
    a fixture (FIXTURES_SETUP) must not skip itself when it runs: ctest
    counts a skipped setup test as passed.

  dyninst_add_test_executable

    dyninst_add_test_executable(<TargetName>
      SOURCE_FILES <file>...
      [DEPS <target>...]
      [DEFINES <val>...]
      [COMPILE_OPTIONS <option>...]
      [REQUIRES <requirement>...]
    )

  Builds an executable that one or more tests run, or that a test uses
  as its input. DEPS are linked PRIVATE, and DEFINES are PRIVATE compile
  definitions. C++ sources are compiled with SUPPORTED_CXX_WARNING_FLAGS,
  then COMPILE_OPTIONS. If a requirement is not met, the executable is
  not created. It links dyninst_test_support, which puts tests/ on the
  include path for dyninst_test_skip.h.

  dyninst_test_requirement

    dyninst_test_requirement(<name> WHEN <condition>... REASON <text>
                             [VALUES <value>...])

  Defines a requirement for REQUIRES. <condition> is an if() condition,
  evaluated where the test is added. When it is false, the test is
  skipped with <text> as the reason.

  A <name> of the form <prefix>:<arg> defines a family: REQUIRES
  codegen:x86_64 puts x86_64 in place of <arg>, and X86_64 in place of
  <ARG>, in <condition> and <text>. VALUES lists the arguments a family
  accepts; any other is an error.

  The argument may be a comma-separated list, met when any one of its
  values is: REQUIRES host:aarch64,ppc64le is met on either host, and is
  reported as "needs the host to be aarch64 or ppc64le". A requirement
  on all of several values is written as separate requirements. A name
  that is defined on its own, such as codegen:i386, keeps its own
  condition inside a list.

  dyninst_test_skip_reason

    dyninst_test_skip_reason(<out-var> REQUIRES <requirement>...)

  Sets <out-var> to the reasons of the unmet requirements, joined by
  ", ", or to "" when all are met. Use it to build something a test needs
  only when the test can run.

#]=======================================================================]

include_guard(GLOBAL)

# The headers beside this directory that every test can include, such as
# dyninst_test_skip.h
add_library(dyninst_test_support INTERFACE)
target_include_directories(dyninst_test_support INTERFACE "${CMAKE_CURRENT_LIST_DIR}/..")

function(dyninst_test_requirement _name)
  # gersemi: hints { WHEN: command_line }
  cmake_parse_arguments(PARSE_ARGV 1 _req "" "REASON" "WHEN;VALUES")
  if(NOT DEFINED _req_WHEN OR NOT DEFINED _req_REASON OR DEFINED _req_UNPARSED_ARGUMENTS)
    message(
      FATAL_ERROR
      "dyninst_test_requirement(${_name}): needs WHEN and REASON, and nothing else"
    )
  endif()
  if(_name MATCHES ":<arg>$" AND NOT DEFINED _req_VALUES)
    message(FATAL_ERROR "dyninst_test_requirement(${_name}): a family needs VALUES")
  elseif(NOT _name MATCHES ":<arg>$" AND DEFINED _req_VALUES)
    message(FATAL_ERROR "dyninst_test_requirement(${_name}): only a family takes VALUES")
  endif()
  get_property(_known GLOBAL PROPERTY _DYNINST_TEST_REQUIREMENTS)
  if(_name IN_LIST _known)
    message(FATAL_ERROR "dyninst_test_requirement: '${_name}' is already defined")
  endif()
  set_property(GLOBAL APPEND PROPERTY _DYNINST_TEST_REQUIREMENTS ${_name})
  set_property(GLOBAL PROPERTY _DYNINST_TEST_REQUIREMENT_${_name}_CONDITION ${_req_WHEN})
  set_property(GLOBAL PROPERTY _DYNINST_TEST_REQUIREMENT_${_name}_REASON ${_req_REASON})
  set_property(GLOBAL PROPERTY _DYNINST_TEST_REQUIREMENT_${_name}_VALUES ${_req_VALUES})
endfunction()

function(dyninst_test_skip_reason _out)
  cmake_parse_arguments(PARSE_ARGV 1 _req "" "" "REQUIRES")
  if(DEFINED _req_UNPARSED_ARGUMENTS)
    message(
      FATAL_ERROR
      "dyninst_test_skip_reason: unexpected arguments '${_req_UNPARSED_ARGUMENTS}'"
    )
  endif()
  get_property(_known GLOBAL PROPERTY _DYNINST_TEST_REQUIREMENTS)
  set(_reasons)
  foreach(_r IN LISTS _req_REQUIRES)
    # A plain name, including one such as codegen:i386 that a family also covers
    if(_r IN_LIST _known)
      get_property(_cond GLOBAL PROPERTY _DYNINST_TEST_REQUIREMENT_${_r}_CONDITION)
      get_property(_reason GLOBAL PROPERTY _DYNINST_TEST_REQUIREMENT_${_r}_REASON)
      if(NOT (${_cond}))
        list(APPEND _reasons "${_reason}")
      endif()
      continue()
    endif()
    if(NOT _r MATCHES "^([^:]+):(.+)$")
      message(FATAL_ERROR "Unknown test requirement '${_r}'")
    endif()
    # <prefix>:<a>,<b>,... is met when any one of <a>, <b>, ... is
    set(_prefix ${CMAKE_MATCH_1})
    set(_key "${_prefix}:<arg>")
    string(REPLACE "," ";" _args "${CMAKE_MATCH_2}")
    if(NOT _key IN_LIST _known)
      message(FATAL_ERROR "Unknown test requirement '${_r}'")
    endif()
    get_property(_values GLOBAL PROPERTY _DYNINST_TEST_REQUIREMENT_${_key}_VALUES)
    set(_met FALSE)
    set(_ARGS)
    foreach(_arg IN LISTS _args)
      if(NOT _arg IN_LIST _values)
        string(REPLACE ";" ", " _values "${_values}")
        message(
          FATAL_ERROR
          "Test requirement '${_r}': '${_arg}' is not one of ${_values}"
        )
      endif()
      string(TOUPPER ${_arg} _ARG)
      list(APPEND _ARGS ${_ARG})
      if("${_prefix}:${_arg}" IN_LIST _known)
        set(_name "${_prefix}:${_arg}")
      else()
        set(_name ${_key})
      endif()
      get_property(_cond GLOBAL PROPERTY _DYNINST_TEST_REQUIREMENT_${_name}_CONDITION)
      string(REPLACE "<arg>" ${_arg} _cond "${_cond}")
      string(REPLACE "<ARG>" ${_ARG} _cond "${_cond}")
      if(${_cond})
        set(_met TRUE)
      endif()
    endforeach()
    if(NOT _met)
      get_property(_reason GLOBAL PROPERTY _DYNINST_TEST_REQUIREMENT_${_key}_REASON)
      list(JOIN _args " or " _any)
      list(JOIN _ARGS " or " _ANY)
      string(REPLACE "<arg>" "${_any}" _reason "${_reason}")
      string(REPLACE "<ARG>" "${_ANY}" _reason "${_reason}")
      list(APPEND _reasons "${_reason}")
    endif()
  endforeach()
  # A ';' in the reason would split the placeholder's arguments
  string(REPLACE ";" ", " _reasons "${_reasons}")
  set(${_out} "${_reasons}" PARENT_SCOPE)
endfunction()

function(dyninst_add_test_executable _target)
  cmake_parse_arguments(
    PARSE_ARGV 1
    _exe
    ""
    ""
    "SOURCE_FILES;DEPS;DEFINES;COMPILE_OPTIONS;REQUIRES"
  )
  if(DEFINED _exe_UNPARSED_ARGUMENTS)
    message(
      FATAL_ERROR
      "dyninst_add_test_executable(${_target}): unexpected arguments '${_exe_UNPARSED_ARGUMENTS}'"
    )
  endif()
  if(NOT DEFINED _exe_SOURCE_FILES)
    message(FATAL_ERROR "dyninst_add_test_executable(${_target}): needs SOURCE_FILES")
  endif()

  dyninst_test_skip_reason(_reason REQUIRES ${_exe_REQUIRES})
  if(NOT "${_reason}" STREQUAL "")
    set_property(GLOBAL PROPERTY _DYNINST_TEST_EXE_SKIPPED_${_target} "${_reason}")
    return()
  endif()

  add_executable(${_target} ${_exe_SOURCE_FILES})
  target_compile_options(
    ${_target}
    PRIVATE
      "$<$<COMPILE_LANGUAGE:CXX>:${SUPPORTED_CXX_WARNING_FLAGS}>"
      ${_exe_COMPILE_OPTIONS}
  )
  target_compile_definitions(${_target} PRIVATE ${_exe_DEFINES})
  target_link_libraries(${_target} PRIVATE dyninst_test_support ${_exe_DEPS})
endfunction()

function(dyninst_add_test _name)
  # gersemi: hints { COMMAND: command_line, ARGS: command_line, PROPERTIES: pairs }
  cmake_parse_arguments(
    PARSE_ARGV 1
    _test
    ""
    "TYPE"
    "SOURCE_FILES;COMMAND;ARGS;DEPS;DEFINES;COMPILE_OPTIONS;REQUIRES;PROPERTIES"
  )
  set(_where "dyninst_add_test(${_name})")
  if(DEFINED _test_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "${_where}: unexpected arguments '${_test_UNPARSED_ARGUMENTS}'")
  endif()
  set(_types unit integration regression)
  if(NOT _test_TYPE IN_LIST _types)
    message(FATAL_ERROR "${_where}: TYPE must be one of ${_types}")
  endif()
  if(
    (DEFINED _test_SOURCE_FILES AND DEFINED _test_COMMAND)
    OR (NOT DEFINED _test_SOURCE_FILES AND NOT DEFINED _test_COMMAND)
  )
    message(FATAL_ERROR "${_where}: needs exactly one of SOURCE_FILES and COMMAND")
  endif()
  if(DEFINED _test_COMMAND)
    foreach(_kw ARGS DEPS DEFINES COMPILE_OPTIONS)
      if(DEFINED _test_${_kw})
        message(FATAL_ERROR "${_where}: ${_kw} goes with SOURCE_FILES, not COMMAND")
      endif()
    endforeach()
  endif()

  dyninst_test_skip_reason(_reason REQUIRES ${_test_REQUIRES})

  if(DEFINED _test_SOURCE_FILES)
    if("${_reason}" STREQUAL "")
      dyninst_add_test_executable(
        ${_name}
        SOURCE_FILES ${_test_SOURCE_FILES}
        DEPS ${_test_DEPS}
        DEFINES ${_test_DEFINES}
        COMPILE_OPTIONS ${_test_COMPILE_OPTIONS}
      )
    endif()
    set(_command ${_name})
    set(_args _test_ARGS)
  else()
    set(_command)
    set(_args _test_COMMAND)
    # Skip with the executable the command runs
    list(GET _test_COMMAND 0 _exe)
    if(_exe MATCHES "^\\$<TARGET_FILE:([^>]+)>$")
      set(_exe ${CMAKE_MATCH_1})
    endif()
    get_property(_exe_reason GLOBAL PROPERTY _DYNINST_TEST_EXE_SKIPPED_${_exe})
    if(NOT "${_exe_reason}" STREQUAL "")
      if(NOT "${_reason}" STREQUAL "")
        string(APPEND _reason ", ")
      endif()
      string(APPEND _reason "${_exe_reason}")
    endif()
  endif()

  # Keep each argument whole: expanding the list would drop an empty one, and split one
  # containing ';'. $<0:> is an empty argument that survives.
  foreach(_a IN LISTS ${_args})
    if("${_a}" STREQUAL "")
      set(_a "$<0:>")
    endif()
    string(REPLACE ";" "\\;" _a "${_a}")
    list(APPEND _command "${_a}")
  endforeach()

  if(NOT "${_reason}" STREQUAL "")
    add_test(
      NAME ${_name}
      COMMAND sh -c [[echo "skipped: $1"; exit 77]] sh "${_reason}"
    )
  else()
    add_test(NAME ${_name} COMMAND ${_command})
  endif()
  # A test tied to the host's architecture is also labeled host, so that
  # `ctest -N -L host` lists them
  set(_labels ${_test_TYPE})
  foreach(_r IN LISTS _test_REQUIRES)
    if(_r MATCHES "^host:")
      list(APPEND _labels host)
      break()
    endif()
  endforeach()
  set_tests_properties(
    ${_name}
    PROPERTIES LABELS "${_labels}" SKIP_RETURN_CODE 77 ${_test_PROPERTIES}
  )
endfunction()
