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

  add_library(${_target} SHARED ${_target_PUBLIC_HEADER_FILES} ${_target_PRIVATE_HEADER_FILES} ${_target_SOURCE_FILES})

  set(_all_targets ${_target})
  if(ENABLE_STATIC_LIBS)
    list(APPEND _all_targets ${_target}_static)
    add_library(${_target}_static STATIC ${_target_PUBLIC_HEADER_FILES} ${_target_PRIVATE_HEADER_FILES} ${_target_SOURCE_FILES})
  endif()

  foreach(t ${_all_targets})
    message(STATUS "Building ${t}...")
    target_link_libraries(${t} PRIVATE ${_target_PRIVATE_DEPS})
    target_link_libraries(${t} PUBLIC ${_target_PUBLIC_DEPS})

		target_include_directories(${t}
		   PUBLIC
		   "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>"
		   "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
		)

    set_target_properties(
      ${t}
      PROPERTIES
               INSTALL_RPATH "${DYNINST_RPATH_DIRECTORIES}"
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
    INCLUDES DESTINATION ${DYNINST_INSTALL_INCLUDEDIR})
endfunction()
