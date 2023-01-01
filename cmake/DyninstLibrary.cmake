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

if(SW_ANALYSIS_STEPPER)
  list(APPEND _dyninst_global_defs USE_PARSE_API)
endif()

if(DYNINST_DISABLE_DIAGNOSTIC_SUPPRESSIONS)
    list(APPEND _dyninst_global_defs DYNINST_DIAGNOSTIC_NO_SUPPRESSIONS)
endif()

function(dyninst_library _target)
	set(_keywords
		PRIVATE_HEADER_FILES
		PUBLIC_HEADER_FILES
		SOURCE_FILES		# Both public and private
		DEFINES
		PUBLIC_DEPS
		PRIVATE_DEPS)

  cmake_parse_arguments(PARSE_ARGV 0 _target "" "" "${_keywords}")
  
  add_library(${_target} ${SRC_LIST})
  target_link_libraries(${_target} PRIVATE ${ARGN})
  file(GLOB headers "h/*.h" "${CMAKE_CURRENT_BINARY_DIR}/h/*.h")
  set(ACTUAL_TARGETS ${_target})
  set(ALL_TARGETS "${ARGN};${_target}")
  if(${ENABLE_STATIC_LIBS})
    set(ACTUAL_TARGETS ${ACTUAL_TARGETS} ${_target}_static)
    add_library(${_target}_static STATIC ${SRC_LIST})
  endif()
  message(STATUS "Building ${ACTUAL_TARGETS}...")
  set_target_properties(
    ${ACTUAL_TARGETS}
    PROPERTIES PUBLIC_HEADER "${headers}"
               LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
               INSTALL_RPATH "${DYNINST_RPATH_DIRECTORIES}"
               SOVERSION ${DYNINST_SOVERSION}
               VERSION ${DYNINST_LIBVERSION})

  foreach(t ${ACTUAL_TARGETS})
    target_compile_definitions(${t} PRIVATE ${_dyninst_global_defs})
  endforeach()

  install(
    TARGETS ${ACTUAL_TARGETS}
    EXPORT dyninst-targets
    RUNTIME DESTINATION ${DYNINST_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${DYNINST_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${DYNINST_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${DYNINST_INSTALL_INCLUDEDIR})
endfunction()
