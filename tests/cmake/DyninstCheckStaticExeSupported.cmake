include_guard(GLOBAL)

include(DyninstCheckCompilerFlag)

# Check if the CMAKE_<LANG>_COMPILER supports static linking executables
#
# NOTE: This is only needed when creating executables because
#
#         add_executable(foo STATIC foo.cpp)
#
#       doesn't (currently) work. Libraries should use the STATIC
#       keyword as usual.

function(dyninst_check_static_exe_supported)

  cmake_parse_arguments(CHECK_STATIC_EXE "" "" "LANGUAGES" "${ARGN}")
  if(CHECK_STATIC_EXE_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "Unparsed arguments: ${CHECK_STATIC_EXE_UNPARSED_ARGUMENTS}")
  endif()

  if(CHECK_STATIC_EXE_LANGUAGES)
    set(_languages "${CHECK_STATIC_EXE_LANGUAGES}")
  else()
    # User did not set any languages, use defaults
    get_property(_languages GLOBAL PROPERTY ENABLED_LANGUAGES)
  endif()

  # gcc and clang both use '-static', but allow for other compilers
  set(_possible_static_flags "-static")

  set(_saved_link_opts ${CMAKE_REQUIRED_LINK_OPTIONS})

  foreach(_flag ${_possible_static_flags})
    set(CMAKE_REQUIRED_LINK_OPTIONS "${_flag}")

    foreach(_lang ${_languages})
      set(_is_supported_name DYNINST_${_lang}_STATIC_EXE_SUPPORTED)
      set(_flag_name DYNINST_${_lang}_STATIC_EXE_LINK_FLAG)

      # If we already found a flag in `_possible_static_flags` for this language, don't re-check.
      if(${_flag_name})
        continue()
      endif()

      dyninst_check_compiler_flag(${_lang} ${_flag} ${_is_supported_name})

      if(${_is_supported_name})
        set(${_flag_name}
            "${_flag}"
            CACHE STRING "${_lang} static executable link flag" FORCE)
      endif()
    endforeach()
  endforeach()

  set(CMAKE_REQUIRED_LINK_OPTIONS "${_saved_link_opts}")

endfunction()
