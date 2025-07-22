#[=======================================================================[.rst:
DyninstCheckPIESupported
------------------------

This module extends CMake's ``check_pie_supported()`` function to also
check for statically-linked PIE executables.

.. command:: dyninst_check_pie_supported

  .. code-block:: cmake

    dyninst_check_pie_supported([LANGUAGES <lang>...])

  Options are:

  ``LANGUAGES <lang>...``
    Check the linkers used for each of the specified languages.
    If this option is not provided, the command checks all enabled languages.

Variables
^^^^^^^^^

For each language checked, ``dyninst_check_pie_supported()`` function
defines the following cache variables in addition to the ones defined
by CMake's ``check_pie_supported()``:

 ``DYNINST_<lang>_STATIC_PIE_SUPPORTED``
   Set to true if the linker can create statically-linked position-independent
   executables and false otherwise.

 ``DYNINST_<lang>_STATIC_PIE_FLAG``
   The flag needed to create a static PIE executable or blank otherwise.

#]=======================================================================]
include_guard(GLOBAL)

include(DyninstCheckCompilerFlag)
include(DyninstCheckStaticExeSupported)

# To use check_pie_supported(), policy CMP0083 must be set to NEW.
cmake_policy(SET CMP0083 NEW)
include(CheckPIESupported)

function(dyninst_check_pie_supported)

  cmake_parse_arguments(DYNINST_CHECK_PIE "" "" "LANGUAGES" "${ARGN}")
  if(DYNINST_CHECK_PIE_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "Unparsed arguments: ${DYNINST_CHECK_PIE_UNPARSED_ARGUMENTS}")
  endif()

  if(DYNINST_CHECK_PIE_LANGUAGES)
    set(_languages "${DYNINST_CHECK_PIE_LANGUAGES}")
  else()
    # User did not set any languages, use defaults
    get_property(_languages GLOBAL PROPERTY ENABLED_LANGUAGES)
  endif()

  check_pie_supported(LANGUAGES ${_languages})
  dyninst_check_static_exe_supported(LANGUAGES ${_languages})

  # gcc and clang both use '-static-pie', but allow for other compilers
  set(_possible_staticpie_flags "-static-pie")

  set(_saved_link_opts ${CMAKE_REQUIRED_LINK_OPTIONS})

  foreach(_lang ${_languages})
    if(NOT DYNINST_${_lang}_STATIC_EXE_SUPPORTED)
      continue()
    endif()

    set(_spf DYNINST_${_lang}_STATIC_PIE_FLAG)

    # If a flag for this language has been found, don't re-check.
    if(${_spf})
      continue()
    endif()

    set(_sps DYNINST_${_lang}_STATIC_PIE_SUPPORTED)
    foreach(_flag ${_possible_staticpie_flags})
      set(CMAKE_REQUIRED_LINK_OPTIONS "${_flag}")

      dyninst_check_compiler_flag(${_lang} "${_flag}" "${_sps}")

      if(NOT ${_sps})
        # Not supported- ensure flag is stored as an empty string
        set(_flag "")
      endif()

      set(${_spf}
          "${_flag}"
          CACHE INTERNAL "${_lang} static PIE flag")

    endforeach()
  endforeach()

  set(CMAKE_REQUIRED_LINK_OPTIONS "${_saved_link_opts}")

endfunction()
