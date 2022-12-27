function(dyninst_library target)
  add_library(${target} ${SRC_LIST})
  target_link_libraries(${target} PRIVATE ${ARGN})
  file(GLOB headers "h/*.h" "${CMAKE_CURRENT_BINARY_DIR}/h/*.h")
  set(ACTUAL_TARGETS ${target})
  set(ALL_TARGETS "${ARGN};${target}")
  if(${ENABLE_STATIC_LIBS})
    set(ACTUAL_TARGETS ${ACTUAL_TARGETS} ${target}_static)
    add_library(${target}_static STATIC ${SRC_LIST})
  endif()
  message(STATUS "Building ${ACTUAL_TARGETS}...")
  set_target_properties(
    ${ACTUAL_TARGETS}
    PROPERTIES PUBLIC_HEADER "${headers}"
               LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
               INSTALL_RPATH "${DYNINST_RPATH_DIRECTORIES}"
               SOVERSION ${DYNINST_SOVERSION}
               VERSION ${DYNINST_LIBVERSION}
               CLEAN_DIRECT_OUTPUT 1)

  set(_defs)
  if(DYNINST_OS_Windows)
    list(APPEND _defs WIN32_LEAN_AND_MEAN)
    if(CMAKE_C_COMPILER_VERSION VERSION_GREATER 19)
      list(APPEND _defs _SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS=1)
    else()
      list(APPEND _defs snprintf=_snprintf)
    endif()
  endif()

  if(LIGHTWEIGHT_SYMTAB)
    list(APPEND _defs WITHOUT_SYMTAB_API WITH_SYMLITE)
  else()
    list(APPEND _defs WITH_SYMTAB_API WITHOUT_SYMLITE)
  endif()

  if(SW_ANALYSIS_STEPPER)
    list(APPEND _defs USE_PARSE_API)
  endif()

	if(DYNINST_DISABLE_DIAGNOSTIC_SUPPRESSIONS)
	    list(APPEND _defs DYNINST_DIAGNOSTIC_NO_SUPPRESSIONS)
	endif()

  foreach(t ${ACTUAL_TARGETS})
    target_compile_definitions(${t} PRIVATE ${_defs})
  endforeach()

  install(
    TARGETS ${ACTUAL_TARGETS}
    EXPORT dyninst-targets
    COMPONENT ${target}
    RUNTIME DESTINATION ${DYNINST_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${DYNINST_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${DYNINST_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${DYNINST_INSTALL_INCLUDEDIR})
endfunction()
