include_guard(GLOBAL)

include(DyninstLinkProperties)

# Use PIE when POSITION_INDEPENDENT_CODE is used for executables
if(POLICY CMP0083)
  cmake_policy(SET CMP0083 NEW)
endif()

add_executable(find_main find_main.cpp)
target_compile_options(find_main PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(find_main PRIVATE symtabAPI dyninstAPI)

foreach(lang_t "c" "cxx")
  foreach(pie_t "pie" "nopie")
    foreach(link_t "stat" "dyn")
      foreach(strip_t "stripped" "nostripped")

        set(_targ "${lang_t}_${link_t}_${pie_t}_${strip_t}")
        add_executable(${_targ} find_main_test.c)

        if(lang_t STREQUAL "cxx")
          set_property(TARGET ${_targ} PROPERTY LANGUAGE "CXX")
        endif()

        if(pie_t STREQUAL "pie")
          set_property(TARGET ${_targ} PROPERTY POSITION_INDEPENDENT_CODE TRUE)
        endif()

        if(link_t STREQUAL "stat")
          if(${lang_t} STREQUAL "c" AND DYNINST_C_STATIC_LINK)
            target_link_libraries(${_targ} "-static")
          endif()
          if(${lang_t} STREQUAL "cxx" AND DYNINST_CXX_STATIC_LINK)
            target_link_libraries(${_targ} "-static")
          endif()
        endif()

        if(strip_t STREQUAL "stripped")
          # '-s' works for gcc and clang
          target_link_libraries(${_targ} "-s")
        endif()

      endforeach()

      set(_input_name "${lang_t}_${link_t}_${pie_t}")
      set(_input_stripped "$<TARGET_FILE_NAME:${_input_name}_stripped>")
      set(_input_nostripped "$<TARGET_FILE_NAME:${_input_name}_nostripped>")
      set(_test_name "dyninstAPI_find_main_${_input_name}")
      add_test(NAME ${_test_name} COMMAND find_main ${_input_nostripped}
                                          ${_input_stripped})
      set_tests_properties(${_test_name} PROPERTIES LABELS "regression")

    endforeach()
  endforeach()
endforeach()
