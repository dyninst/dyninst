#[=======================================================================[
DyninstAddTest
--------------

Functions for adding a project's tests to ctest. README.md, in the
directory above this one, describes them in full, with examples.

  dyninst_add_test

    dyninst_add_test(<TestName>
      [TYPE <type>]
      SOURCE_FILES <file>... | COMMAND <command> [<arg>...]
      [ARGS <arg>...]
      [DEPS <target>...]
      [DEFINES <val>...]
      [COMPILE_OPTIONS <option>...]
      [REQUIRES <requirement>...]
      [PROPERTIES <property> <value>...]
    )

  Creates a test named <TestName> with CTest's add_test.

  If a test can only be built or run when a configure-time value is set,
  name it in REQUIRES (see dyninst_test_requirement below). When a
  requirement is not met, nothing is built for the test, and a
  placeholder with the same name reports that it was skipped and why. A
  test that can only tell when it runs leaves REQUIRES out and skips
  itself with skip_message() from dyninst_test_skip.h. A test is skipped
  when it exits with 77.

  For example,

    add_executable(parser_test parser.cpp)
    target_compile_options(parser_test PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
    target_link_libraries(parser_test PRIVATE Dyninst::symtabAPI)
    add_test(NAME parser_test COMMAND parser_test)

  becomes

    dyninst_add_test(parser_test SOURCE_FILES parser.cpp DEPS Dyninst::symtabAPI)

  dyninst_add_test always adds SUPPORTED_CXX_WARNING_FLAGS.

  TYPE
    One of DYNINST_TEST_TYPES, which becomes the test's LABELS. Required
    when the project sets DYNINST_TEST_TYPES, and an error when it does
    not. A test that REQUIRES host:<arch> is also labeled host.

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
    Link dependencies, as CMake target names, such as Dyninst::symtabAPI
    and Threads::Threads.

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
  not created. It links dyninst_test_support, which puts this directory on
  the include path for dyninst_test_skip.h.

  dyninst_test_requirement

    dyninst_test_requirement(<name> WHEN <condition>... REASON <text>
                             [VALUES <value>...])

  Defines a requirement for REQUIRES. <condition> is an if() condition,
  evaluated where the test is added. When it is false, the test is
  skipped with <text> as the reason.

  A <name> of the form <prefix>:<arg> defines a family: REQUIRES
  <prefix>:foo puts foo in place of <arg>, and FOO in place of <ARG>, in
  <condition> and <text>. VALUES lists the arguments a family accepts;
  any other is an error.

  The argument may be a comma-separated list, met when any one of its
  values is: <prefix>:foo,bar is met when <prefix>:foo or <prefix>:bar
  is, and puts "foo or bar" in place of <arg> in <text>. A requirement
  on all of several values is written as separate requirements. A name
  defined on its own, such as <prefix>:foo, keeps its own condition
  inside a list.

  dyninst_test_skip_reason

    dyninst_test_skip_reason(<out-var> REQUIRES <requirement>...)

  Sets <out-var> to the reasons of the unmet requirements, joined by
  ", ", or to "" when all are met. Use it to build something a test needs
  only when the test can run.

#]=======================================================================]

include_guard(GLOBAL)

# The headers in this directory that every test can include, such as
# dyninst_test_skip.h
add_library(dyninst_test_support INTERFACE)
target_include_directories(dyninst_test_support INTERFACE "${CMAKE_CURRENT_LIST_DIR}")

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
  # A project with test types lists them in DYNINST_TEST_TYPES; each test then names one
  # in TYPE, which becomes its label
  set(_labels)
  if(DEFINED DYNINST_TEST_TYPES)
    if(NOT _test_TYPE IN_LIST DYNINST_TEST_TYPES)
      string(REPLACE ";" ", " _types "${DYNINST_TEST_TYPES}")
      message(FATAL_ERROR "${_where}: TYPE must be one of ${_types}")
    endif()
    set(_labels ${_test_TYPE})
  elseif(DEFINED _test_TYPE)
    message(FATAL_ERROR "${_where}: TYPE needs DYNINST_TEST_TYPES, which is not set")
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
  foreach(_r IN LISTS _test_REQUIRES)
    if(_r MATCHES "^host:")
      list(APPEND _labels host)
      break()
    endif()
  endforeach()
  if(_labels)
    set_tests_properties(${_name} PROPERTIES LABELS "${_labels}")
  endif()
  set_tests_properties(${_name} PROPERTIES SKIP_RETURN_CODE 77 ${_test_PROPERTIES})
endfunction()
