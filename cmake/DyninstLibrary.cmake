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
  cmake_parse_arguments(PARSE_ARGV 0 _target "" "" "${_keywords}")

  add_library(${_target} SHARED ${_target_PUBLIC_HEADER_FILES}
                                ${_target_PRIVATE_HEADER_FILES} ${_target_SOURCE_FILES})

  # Depending on another Dyninst library is always public
  target_link_libraries(${_target} PUBLIC ${_target_DYNINST_DEPS})

  set(_all_targets ${_target})

  if(ENABLE_STATIC_LIBS)
    list(APPEND _all_targets ${_target}_static)
    add_library(
      ${_target}_static STATIC ${_target_PUBLIC_HEADER_FILES}
                               ${_target_PRIVATE_HEADER_FILES} ${_target_SOURCE_FILES})

    # Link against the corresponding static Dyninst target
    foreach(d ${_target_DYNINST_DEPS})
      # Depending on another Dyninst library is always public
      target_link_libraries(${_target}_static PUBLIC "${d}_static")
    endforeach()
  endif()

  foreach(t ${_all_targets})
    message(STATUS "Adding library '${t}'")

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
      ${t} PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>"
                  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")

    set_target_properties(
      ${t}
      PROPERTIES INSTALL_RPATH "${DYNINST_RPATH_DIRECTORIES}"
                 SOVERSION ${DYNINST_SOVERSION}
                 VERSION ${DYNINST_LIBVERSION})

    target_compile_definitions(${t} PRIVATE ${_dyninst_global_defs} ${_target_DEFINES})
  endforeach()

  install(
    TARGETS ${_all_targets}
    EXPORT dyninst-targets
    RUNTIME DESTINATION ${DYNINST_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${DYNINST_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${DYNINST_INSTALL_LIBDIR}
    INCLUDES
    DESTINATION ${DYNINST_INSTALL_INCLUDEDIR})

  # Install headers, preserving the directory structure
  # Note: By convention, headers are stored in "h/"
  foreach(h ${_public_headers})
    string(REGEX MATCH "^h\\/(.*)" _file ${h})
    get_filename_component(_dir ${CMAKE_MATCH_1} DIRECTORY)
    install(FILES ${h} DESTINATION "${DYNINST_INSTALL_INCLUDEDIR}/${_dir}")
  endforeach()

  set(${_target}_TARGETS
      ${_all_targets}
      PARENT_SCOPE)
endfunction()
