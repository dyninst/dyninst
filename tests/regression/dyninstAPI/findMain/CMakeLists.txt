include_guard(GLOBAL)

include(DyninstLinkProperties)

# Use PIE when POSITION_INDEPENDENT_CODE is used for executables
if(POLICY CMP0083)
  cmake_policy(SET CMP0083 NEW)
endif()

add_executable(findMain findMain.cpp)
target_compile_options(findMain PRIVATE ${SUPPORTED_CXX_WARNING_FLAGS})
target_link_libraries(findMain PRIVATE symtabAPI dyninstAPI)

foreach(lang_t "c" "cxx")
  foreach(pic_t "nopie" "pie")
    foreach(link_t "stat" "dyn")
      foreach(strip_t "nostripped" "stripped")

        set(_targ "test.${lang_t}.${link_t}.${pic_t}.${strip_t}")
        add_executable(${_targ} test.c)

        if(lang_t STREQUAL "cxx")
          set_property(TARGET ${_targ} PROPERTY LANGUAGE "CXX")
        endif()

        if(pic_t STREQUAL "pie")
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

      set(_test_name "test.${lang_t}.${link_t}.${pic_t}")
      add_test(NAME "testMain_${_test_name}" COMMAND findMain "${_test_name}.nostripped"
                                                     "${_test_name}.stripped")
      set_tests_properties("testMain_${_test_name}" PROPERTIES LABELS "regression")
    endforeach()
  endforeach()
endforeach()
