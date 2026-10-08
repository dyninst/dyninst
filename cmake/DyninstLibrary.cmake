#[=======================================================================[
DyninstLibrary
--------------

This module provides a uniform interface for creating a Dyninst
toolkit target.

  dyninst_library

  This command is a wrapper around `add_library`. For a Dyninst
  toolkit <TargetName> it creates up to three targets, all of them
  installed and exported in the Dyninst:: namespace:

  <TargetName>
    The shared library. It links its DYNINST_DEPS and PUBLIC_DEPS, so
    linking it brings everything a program needs to use the toolkit.

  <TargetName>_static
    The static library, created when ENABLE_STATIC_LIBS or FORCE_STATIC
    is on. DYNINST_DEPS below says which variant of each dependency it
    links.

  <TargetName>_headers
    An INTERFACE target with what is needed to compile against the
    toolkit, and no libraries to link: its public include directories,
    the <dep>_headers of each Dyninst dependency, and HEADER_DEPS. Both
    library variants link it. A program that uses the toolkit still
    links <TargetName> or <TargetName>_static; <TargetName>_headers
    alone is for code that only needs the declarations.

  An INTERNAL_LIBRARY or NO_HEADERS_TARGET library has no _headers
  target. An internal library is an OBJECT library absorbed into the
  real libraries that list it in DYNINST_INTERNAL_DEPS; its exported
  target is not meant for use outside Dyninst.

    dyninst_library(<TargetName>
      [PRIVATE_HEADER_FILES <file>...]
      [PUBLIC_HEADER_FILES <file>...]
      [SOURCE_FILES <file>...]
      [DEFINES <val>...]
      [DYNINST_DEPS <target>...]
      [DYNINST_INTERNAL_DEPS <target>...]
      [HEADER_DEPS <target>...]
      [PUBLIC_DEPS <target>...]
      [PRIVATE_DEPS <target>...]
      [FORCE_STATIC]
      [INTERNAL_LIBRARY]
      [NO_HEADERS_TARGET]
    )

  The <TargetName>_TARGETS variable will contain the names of the
  created library targets for this toolkit (not <TargetName>_headers).

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
    A list of dependent Dyninst targets. <TargetName> links each of them.
    <TargetName>_static links the static variant of each one that has a
    static variant, and it is an error for a SHARED library here to have
    none.

  DYNINST_INTERNAL_DEPS
    A list of targets that are PRIVATE dependencies of every variant of
    <TargetName>, such as the internal libraries it absorbs.

  HEADER_DEPS
    A list of third-party targets that the public headers of
    <TargetName> need. They are linked to <TargetName>_headers, so that
    their include directories keep SYSTEM and their compile options.
    An internal library has no _headers target; for it they are PUBLIC
    dependencies of the library itself. For any other library, every
    entry must be a compile-only target, such as Dyninst::Boost_headers
    or Dyninst::TBB_headers: its link libraries would come along into
    <TargetName>_headers.

  PUBLIC_DEPS
    A list of targets that are PUBLIC dependencies of <TargetName>. This
    is where a third-party library goes that code using <TargetName>
    must link, as common lists Dyninst::Boost and Dyninst::TBB, while
    their headers go in HEADER_DEPS.

  PRIVATE_DEPS
    A list of targets that are PRIVATE dependencies of <TargetName>.

  FORCE_STATIC
    Create <TargetName>_static even when ENABLE_STATIC_LIBS is off.

  INTERNAL_LIBRARY
    Create an OBJECT library, absorbed into the real libraries that list
    it in DYNINST_INTERNAL_DEPS, with no static variant and no _headers
    target.

  NO_HEADERS_TARGET
    Do not create <TargetName>_headers, for a library that nothing
    compiles against, such as the runtime library loaded into the
    mutatee. Its include directories and HEADER_DEPS go on each library
    variant instead, as for an internal library.

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

if(DYNINST_ENABLE_CAPSTONE)
  list(APPEND _dyninst_global_defs DYNINST_ENABLE_CAPSTONE)
endif()

list(APPEND _dyninst_global_defs ${DYNINST_PLATFORM_CAPABILITIES})

function(dyninst_library _target)
  set(
    _keywords
    PRIVATE_HEADER_FILES
    PUBLIC_HEADER_FILES
    SOURCE_FILES # Both public and private
    DEFINES
    DYNINST_DEPS
    DYNINST_INTERNAL_DEPS
    HEADER_DEPS
    PUBLIC_DEPS
    PRIVATE_DEPS
  )

  cmake_parse_arguments(
    PARSE_ARGV 0
    _target
    "FORCE_STATIC;INTERNAL_LIBRARY;NO_HEADERS_TARGET"
    ""
    "${_keywords}"
  )

  if(_target_INTERNAL_LIBRARY)
    # Internal libraries never create actual library files (.so, .a, etc.)
    set(_lib_type OBJECT)
  else()
    set(_lib_type SHARED)
  endif()

  add_library(
    ${_target}
    ${_lib_type}
    ${_target_PUBLIC_HEADER_FILES}
    ${_target_PRIVATE_HEADER_FILES}
    ${_target_SOURCE_FILES}
  )

  if(_target_INTERNAL_LIBRARY)
    set_target_properties(${_target} PROPERTIES POSITION_INDEPENDENT_CODE ON)

    # Let an object library start compiling without first waiting for the
    # libraries it depends on to be built: it never links, and nothing here
    # generates headers, so the Makefile generator's target ordering only
    # idles the build.  Not for static libraries (see below).  CMake < 3.19
    # ignores this, and -DCMAKE_OPTIMIZE_DEPENDENCIES=OFF turns it off.
    if(NOT DEFINED CMAKE_OPTIMIZE_DEPENDENCIES OR CMAKE_OPTIMIZE_DEPENDENCIES)
      set_target_properties(${_target} PROPERTIES OPTIMIZE_DEPENDENCIES ON)
    endif()
  endif()

  set(_all_targets ${_target})

  set(
    _include_dirs
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR};${CMAKE_CURRENT_SOURCE_DIR}/src;${CMAKE_CURRENT_SOURCE_DIR}/h>"
    "$<INSTALL_INTERFACE:${DYNINST_INSTALL_INCLUDEDIR}>"
  )

  # Internal libraries are absorbed into real ones, and nothing compiles
  # against a NO_HEADERS_TARGET library: neither has a headers target, and
  # their include directories and HEADER_DEPS go on the targets themselves
  # (below, once the static variant exists).
  set(_headers_target)
  if(NOT _target_INTERNAL_LIBRARY AND NOT _target_NO_HEADERS_TARGET)
    set(_headers_target ${_target}_headers)
    add_library(${_target}_headers INTERFACE)
    target_include_directories(${_target}_headers INTERFACE "${_include_dirs}")
    foreach(d ${_target_DYNINST_DEPS})
      if(TARGET ${d}_headers)
        target_link_libraries(${_target}_headers INTERFACE ${d}_headers)
      endif()
    endforeach()
    target_link_libraries(${_target}_headers INTERFACE ${_target_HEADER_DEPS})

    # Linked before anything else, so that the library's own include
    # directories come before those of its dependencies: several components
    # have headers with the same name (e.g. util.h in common and parseAPI).
    target_link_libraries(${_target} PUBLIC ${_target}_headers)
  endif()

  if(NOT _target_INTERNAL_LIBRARY AND (_target_FORCE_STATIC OR ENABLE_STATIC_LIBS))
    list(APPEND _all_targets ${_target}_static)
    add_library(
      ${_target}_static
      STATIC
      ${_target_PUBLIC_HEADER_FILES}
      ${_target_PRIVATE_HEADER_FILES}
      ${_target_SOURCE_FILES}
    )
    if(_headers_target)
      target_link_libraries(${_target}_static PUBLIC ${_headers_target})
    endif()

    # A static library archives the objects of the object libraries it
    # links, so it must wait for them to be built, even when
    # CMAKE_OPTIMIZE_DEPENDENCIES=ON would drop that ordering.
    set_target_properties(${_target}_static PROPERTIES OPTIMIZE_DEPENDENCIES OFF)

    # Depending on another Dyninst library is always public. Dependencies
    # are added before their dependents, so the static variant either
    # exists by now or never will.
    foreach(d ${_target_DYNINST_DEPS})
      if(TARGET ${d}_static)
        target_link_libraries(${_target}_static PUBLIC ${d}_static)
      else()
        get_target_property(_dep_type ${d} TYPE)
        if(_dep_type STREQUAL "SHARED_LIBRARY")
          message(
            FATAL_ERROR
            "${_target}_static depends on ${d}, which has no static variant"
          )
        endif()
        # An OBJECT or INTERFACE library has only one variant
        target_link_libraries(${_target}_static PUBLIC ${d})
      endif()
    endforeach()
  endif()

  if(NOT _headers_target)
    foreach(t ${_all_targets})
      target_include_directories(${t} PUBLIC "${_include_dirs}")
      target_link_libraries(${t} PUBLIC ${_target_HEADER_DEPS})
    endforeach()
  endif()

  # Depending on another Dyninst library is always public
  target_link_libraries(${_target} PUBLIC ${_target_DYNINST_DEPS})

  foreach(t ${_all_targets})
    message(STATUS "Adding library '${t}'")

    # Internal library dependencies are NOT public
    target_link_libraries(${t} PRIVATE ${_target_DYNINST_INTERNAL_DEPS})

    target_link_options(
      ${t}
      PRIVATE
        $<$<COMPILE_LANGUAGE:C>:${DYNINST_LINK_FLAGS}>
        $<$<COMPILE_LANGUAGE:CXX>:${DYNINST_CXX_LINK_FLAGS}>
    )

    target_compile_options(
      ${t}
      PRIVATE
        $<$<COMPILE_LANGUAGE:C>:${SUPPORTED_C_WARNING_FLAGS}>
        $<$<COMPILE_LANGUAGE:CXX>:${SUPPORTED_CXX_WARNING_FLAGS}>
    )

    target_compile_options(
      ${t}
      PRIVATE
        $<$<COMPILE_LANGUAGE:C>:
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
        >
    )

    foreach(_v "PUBLIC" "PRIVATE")
      set(_d ${_target_${_v}_DEPS})
      if("${t}" MATCHES "static")
        # OpenMP doesn't work with static libraries, so explicitly
        # remove it from link dependencies
        list(FILTER _d EXCLUDE REGEX "OpenMP")
      endif()
      target_link_libraries(${t} ${_v} ${_d})
      unset(_d)
    endforeach()

    set_target_properties(
      ${t}
      PROPERTIES
        INSTALL_RPATH "${DYNINST_RPATH_DIRECTORIES}"
        SOVERSION ${DYNINST_SOVERSION}
        VERSION ${DYNINST_VERSION}
    )

    target_compile_definitions(${t} PRIVATE ${_dyninst_global_defs} ${_target_DEFINES})
  endforeach()

  set(_installed_targets ${_all_targets} ${_headers_target})

  install(
    TARGETS ${_installed_targets}
    EXPORT dyninst-targets
    RUNTIME DESTINATION ${DYNINST_INSTALL_BINDIR}
    LIBRARY DESTINATION ${DYNINST_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${DYNINST_INSTALL_LIBDIR}
  )

  # Install headers, preserving the directory structure under h/.
  # Note: By convention, headers are stored in "h/"
  foreach(h ${_target_PUBLIC_HEADER_FILES})
    string(REGEX MATCH "^h/(.*)" _file ${h})
    if(NOT CMAKE_MATCH_1)
      # An absolute path, which a generated header is.  The leading .* is
      # greedy on purpose: it makes the LAST "/h/" the one that splits the
      # path, so that an "h" directory above it in the build or home
      # directory -- /home/h/proj/build/common/h/foo.h -- does not carry the
      # header with it into include/proj/build/common/h.
      string(REGEX MATCH ".*/h/(.*)" _file ${h})
    endif()
    if(NOT CMAKE_MATCH_1)
      message(FATAL_ERROR "Unable to process file '${h}'")
    endif()
    get_filename_component(_dir ${CMAKE_MATCH_1} DIRECTORY)
    install(FILES ${h} DESTINATION "${DYNINST_INSTALL_INCLUDEDIR}/${_dir}")
  endforeach()

  set(${_target}_TARGETS ${_all_targets} PARENT_SCOPE)
endfunction()
