#[=======================================================================[
DyninstLibrary
--------------

This module provides a uniform interface for creating a Dyninst
toolkit target.

  dyninst_library

  This command is a wrapper around `add_library` that creates a
  SHARED and, optionally, STATIC library for a Dyninst toolkit.

    dyninst_library(<TargetName>
      [PRIVATE_HEADER_FILES <file>...]
      [PUBLIC_HEADER_FILES <file>...]
      [SOURCE_FILES <file>...]
      [DEFINES <val>...]
      [DYNINST_DEPS <target>...]
      [PUBLIC_DEPS <target>...]
      [PRIVATE_DEPS <target>...]
    )

  The <TargetName>_TARGETS variable will contain the names of the
  created targets for this toolkit.

  The options are:

  PRIVATE_HEADER_FILES
    A list of header files that are needed to build <TargetName>, but
    are not part of the public interface. These files are not copied
    into the install tree.

  PUBLIC_HEADER_FILES
    A list of header files that are part of the toolkit's public API.
    These files are copied into the install tree.

  SOURCE_FILES
    A list of source files for building the library. They are always
    considered PRIVATE attributes of the target as in `target_sources`.

  DEFINES
    A list of compiler definitions to attach to the target. These are
    always PRIVATE attributes of the target.

  DYNINST_DEPS
    A list of dependent Dyninst targets. If a target for a static library
    is created, it will link against the corresponding static target for
    each library container here.

  PUBLIC_DEPS
    A list of targets that are PUBLIC dependencies of <TargetName>.

  PRIVATE_DEPS
    A list of targets that are PRIVATE dependencies of <TargetName>.

#]=======================================================================]

include_guard(DIRECTORY)

if(LIGHTWEIGHT_SYMTAB)
  set(SYMREADER symLite)
else()
  set(SYMREADER symtabAPI)
endif()

set(_dyninst_global_defs)
if(DYNINST_OS_Windows)
  list(APPEND _dyninst_global_defs WIN32_LEAN_AND_MEAN)
  if(CMAKE_C_COMPILER_VERSION VERSION_GREATER 19)
    list(APPEND _dyninst_global_defs _SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS=1)
  else()
    list(APPEND _dyninst_global_defs snprintf=_snprintf)
  endif()
endif()

if(LIGHTWEIGHT_SYMTAB)
  list(APPEND _dyninst_global_defs WITH_SYMLITE)
else()
  list(APPEND _dyninst_global_defs WITH_SYMTAB_API)
endif()

if(DYNINST_DISABLE_DIAGNOSTIC_SUPPRESSIONS)
  list(APPEND _dyninst_global_defs DYNINST_DIAGNOSTIC_NO_SUPPRESSIONS)
endif()

list(APPEND _dyninst_global_defs ${DYNINST_PLATFORM_CAPABILITIES})

function(dyninst_library _target)
  # cmake-format: off
  set(_keywords
      PRIVATE_HEADER_FILES
      PUBLIC_HEADER_FILES
      SOURCE_FILES # Both public and private
      DEFINES
      DYNINST_DEPS
      PUBLIC_DEPS
      PRIVATE_DEPS)
  # cmake-format: on
  cmake_parse_arguments(PARSE_ARGV 0 _target "FORCE_STATIC" "" "${_keywords}")

  add_library(${_target} SHARED ${_target_PUBLIC_HEADER_FILES}
                                ${_target_PRIVATE_HEADER_FILES} ${_target_SOURCE_FILES})

  # Depending on another Dyninst library is always public
  target_link_libraries(${_target} PUBLIC ${_target_DYNINST_DEPS})

  set(_all_targets ${_target})

  if(_target_FORCE_STATIC OR ENABLE_STATIC_LIBS)
    list(APPEND _all_targets ${_target}_static)
    add_library(
      ${_target}_static STATIC ${_target_PUBLIC_HEADER_FILES}
                               ${_target_PRIVATE_HEADER_FILES} ${_target_SOURCE_FILES})

    # When building all libraries as static, they have a '_static' suffix
    # but not when FORCE_STATIC is active
    if(NOT _target_FORCE_STATIC)
      set(_suffix "_static")
    endif()

    # Link against the corresponding static Dyninst target
    foreach(d ${_target_DYNINST_DEPS})
      # Depending on another Dyninst library is always public
      target_link_libraries(${_target}_static PUBLIC "${d}${_suffix}")
    endforeach()
    unset(_suffix)
  endif()

  foreach(t ${_all_targets})
    message(STATUS "Adding library '${t}'")

    target_link_options(${t} PRIVATE $<$<COMPILE_LANGUAGE:C>:${DYNINST_LINK_FLAGS}>
                        $<$<COMPILE_LANGUAGE:CXX>:${DYNINST_CXX_LINK_FLAGS}>)

    target_compile_options(
      ${t} PRIVATE $<$<COMPILE_LANGUAGE:C>:${SUPPORTED_C_WARNING_FLAGS}>
                   $<$<COMPILE_LANGUAGE:CXX>:${SUPPORTED_CXX_WARNING_FLAGS}>)

    target_compile_options(
      ${t}
      PRIVATE $<$<COMPILE_LANGUAGE:C>:
              $<$<CONFIG:DEBUG>:${DYNINST_C_FLAGS_DEBUG}>
              $<$<CONFIG:RELWITHDEBINFO>:${DYNINST_C_FLAGS_RELWITHDEBINFO}>
              $<$<CONFIG:RELEASE>:${DYNINST_C_FLAGS_RELEASE}>
              $<$<CONFIG:MINSIZEREL>:${DYNINST_C_FLAGS_MINSIZEREL}>
              >
              $<$<COMPILE_LANGUAGE:CXX>:
              $<$<CONFIG:DEBUG>:${DYNINST_CXX_FLAGS_DEBUG}>
              $<$<CONFIG:RELWITHDEBINFO>:${DYNINST_CXX_FLAGS_RELWITHDEBINFO}>
              $<$<CONFIG:RELEASE>:${DYNINST_CXX_FLAGS_RELEASE}>
              $<$<CONFIG:MINSIZEREL>:${DYNINST_CXX_FLAGS_MINSIZEREL}>
              >)

    foreach(_v "PUBLIC" "PRIVATE")
      set(_d ${_target_${_v}_DEPS})
      if(${t} MATCHES "static")
        # OpenMP doesn't work with static libraries, so explicitly
        # remove it from link dependencies
        list(FILTER _d EXCLUDE REGEX "OpenMP")
      endif()
      target_link_libraries(${t} ${_v} ${_d})
      unset(_d)
    endforeach()

    target_include_directories(
      ${t}
      PUBLIC
        "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR};${CMAKE_CURRENT_SOURCE_DIR}/src;${CMAKE_CURRENT_SOURCE_DIR}/h>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")

    set_target_properties(
      ${t}
      PROPERTIES INSTALL_RPATH "${DYNINST_RPATH_DIRECTORIES}"
                 SOVERSION ${DYNINST_SOVERSION}
                 VERSION ${DYNINST_VERSION})

    target_compile_definitions(${t} PRIVATE ${_dyninst_global_defs} ${_target_DEFINES})
  endforeach()

  install(
    TARGETS ${_all_targets}
    EXPORT dyninst-targets
    RUNTIME DESTINATION ${DYNINST_INSTALL_BINDIR}
    LIBRARY DESTINATION ${DYNINST_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${DYNINST_INSTALL_LIBDIR}
    INCLUDES
    DESTINATION ${DYNINST_INSTALL_INCLUDEDIR})

  # Install headers, preserving the directory structure under h/.
  # Note: By convention, headers are stored in "h/"
  foreach(h ${_target_PUBLIC_HEADER_FILES})
    string(REGEX MATCH "^h/(.*)" _file ${h})
    get_filename_component(_dir ${CMAKE_MATCH_1} DIRECTORY)
    install(FILES ${h} DESTINATION "${DYNINST_INSTALL_INCLUDEDIR}/${_dir}")
  endforeach()

  set(${_target}_TARGETS
      ${_all_targets}
      PARENT_SCOPE)
endfunction()
