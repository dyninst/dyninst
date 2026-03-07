include_guard(GLOBAL)

include(DyninstCheckStaticExeSupported)
include(DyninstCheck32bitRuntimeSupported)
include(DyninstCheckPIESupported)

add_executable(find_main find_main.cpp)
target_compile_options(find_main PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(find_main PRIVATE symtabAPI dyninstAPI)

set(_languages C CXX)
dyninst_check_static_exe_supported(LANGUAGES ${_languages})
dyninst_check_32bit_runtime_supported(LANGUAGES ${_languages})
dyninst_check_pie_supported(LANGUAGES ${_languages})

foreach(lang_t ${_languages})

  set(_pie_flags "nopie")
  if(CMAKE_${lang_t}_LINK_PIE_SUPPORTED)
    list(APPEND _pie_flags "pie")
  endif()

  set(_bit_flags "64")
  if(DYNINST_${lang_t}_32BIT_RUNTIME_SUPPORTED)
    list(APPEND _bit_flags "32")
  endif()

  set(_link_flags "dyn")
  if(DYNINST_${lang_t}_STATIC_EXE_SUPPORTED)
    list(APPEND _link_flags "stat")
  endif()

  foreach(pie_t ${_pie_flags})
    foreach(bits_t ${_bit_flags})
      foreach(link_t ${_link_flags})

        unset(_compile_opts)
        unset(_link_opts)

        if(link_t STREQUAL "stat")
          if(bits_t STREQUAL "32" AND DYNINST_${lang_t}_32BIT_STATIC_RUNTIME_SUPPORTED)
            list(APPEND _compile_opts ${DYNINST_${lang_t}_32BIT_STATIC_RUNTIME_FLAG})
            list(APPEND _link_opts ${DYNINST_${lang_t}_32BIT_STATIC_RUNTIME_FLAG})
          endif()

          # The static-pie flag must be after the 32-bit and static flags
          if(pie_t STREQUAL "pie")
            list(APPEND _compile_opts ${DYNINST_${lang_t}_STATIC_PIE_FLAG})
            list(APPEND _link_opts ${DYNINST_${lang_t}_STATIC_PIE_FLAG})
          else()
            list(APPEND _compile_opts ${DYNINST_${lang_t}_STATIC_EXE_LINK_FLAG})
            list(APPEND _link_opts ${DYNINST_${lang_t}_STATIC_EXE_LINK_FLAG})
          endif()
        else()
          if(bits_t STREQUAL "32" AND DYNINST_${lang_t}_32BIT_RUNTIME_SUPPORTED)
            list(APPEND _compile_opts ${DYNINST_${lang_t}_32BIT_RUNTIME_FLAG})
            list(APPEND _link_opts ${DYNINST_${lang_t}_32BIT_RUNTIME_FLAG})
          endif()
        endif()

        foreach(strip_t "stripped" "nostripped")

          set(_targ "${lang_t}_${link_t}_${pie_t}_${bits_t}_${strip_t}")
          add_executable(${_targ} find_main_test.c)

          set_property(TARGET ${_targ} PROPERTY LANGUAGE ${lang_t})

          # Some compilers will use 'pie' by default, so explicitly set
          set(_pie FALSE)
          if(pie_t STREQUAL "pie")
            set(_pie TRUE)
          endif()
          set_property(TARGET ${_targ} PROPERTY POSITION_INDEPENDENT_CODE ${_pie})

          target_compile_options(${_targ} PRIVATE ${_compile_opts})
          target_link_libraries(${_targ} PRIVATE ${_link_opts})

          if(strip_t STREQUAL "stripped")
            # '-s' works for gcc and clang
            target_link_libraries(${_targ} PRIVATE "-s")
          endif()
        endforeach()

        set(_input_name "${lang_t}_${link_t}_${pie_t}_${bits_t}")
        set(_input_stripped "$<TARGET_FILE_NAME:${_input_name}_stripped>")
        set(_input_nostripped "$<TARGET_FILE_NAME:${_input_name}_nostripped>")
        set(_test_name "dyninstAPI_find_main_${_input_name}")
        add_test(NAME ${_test_name} COMMAND find_main ${_input_nostripped}
                                            ${_input_stripped})
        set_tests_properties(${_test_name} PROPERTIES LABELS "regression")

      endforeach()
    endforeach()
  endforeach()
endforeach()
