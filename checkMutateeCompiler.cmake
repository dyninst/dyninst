
MACRO (CHECK_MUTATEE_COMPILER _COMPILER _COMP_FLAG _LINK_FLAG _LANG _MSG _RESULT)
   if (NOT DEFINED ${_RESULT})
      set(COMPILER_RESULT 0)
	  set(COMPILER_OUTPUT "")
      file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/CMakeTmp/CompilerTest)
      if(${_LANG} MATCHES CXX)
            execute_process(WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/CMakeTmp/CompilerTest
                            RESULT_VARIABLE COMPILER_RESULT
                            OUTPUT_QUIET
                            ERROR_QUIET
#							OUTPUT_VARIABLE COMPILER_OUTPUT
#							ERROR_VARIABLE COMPILER_OUTPUT
                            COMMAND ${CMAKE_COMMAND}
                            -DCMAKE_CXX_COMPILER=${_COMPILER}
                            -DCMAKE_CXX_FLAGS=${_COMP_FLAG}
                            -DCMAKE_EXE_LINKER_FLAGS=${_LINK_FLAG}
			    -G${CMAKE_GENERATOR}
                            ${PROJECT_SOURCE_DIR}/compiler_test/cxx)
      elseif (${_LANG} MATCHES Fortran)
            execute_process(WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/CMakeTmp/CompilerTest
                            RESULT_VARIABLE COMPILER_RESULT
                            OUTPUT_QUIET
                            ERROR_QUIET
#							OUTPUT_VARIABLE COMPILER_OUTPUT
#							ERROR_VARIABLE COMPILER_OUTPUT
                            COMMAND ${CMAKE_COMMAND}
                            -DCMAKE_Fortran_COMPILER=${_COMPILER}
                            -DCMAKE_Fortran_FLAGS=${_COMP_FLAG}
                            -DCMAKE_EXE_LINKER_FLAGS=${_LINK_FLAG}
			    -G${CMAKE_GENERATOR}
                            ${PROJECT_SOURCE_DIR}/compiler_test/fortran)
      elseif (${_LANG} MATCHES C)
            execute_process(WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/CMakeTmp/CompilerTest
                            RESULT_VARIABLE COMPILER_RESULT
                            OUTPUT_QUIET
                            ERROR_QUIET
#							OUTPUT_VARIABLE COMPILER_OUTPUT
#							ERROR_VARIABLE COMPILER_OUTPUT
                            COMMAND ${CMAKE_COMMAND}
                            -DCMAKE_C_COMPILER=${_COMPILER}
                            -DCMAKE_C_FLAGS=${_COMP_FLAG}
                            -DCMAKE_EXE_LINKER_FLAGS=${_LINK_FLAG}
			    -G${CMAKE_GENERATOR}
                            ${PROJECT_SOURCE_DIR}/compiler_test/c)
      endif()
      if (${COMPILER_RESULT} MATCHES 0)
            execute_process(WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/CMakeTmp/CompilerTest
                            RESULT_VARIABLE COMPILER_RESULT
                            OUTPUT_QUIET
                            ERROR_QUIET
#							OUTPUT_VARIABLE COMPILER_OUTPUT
#							ERROR_VARIABLE COMPILER_OUTPUT
                            COMMAND ${CMAKE_COMMAND}
                            --build ${CMAKE_BINARY_DIR}/CMakeTmp/CompilerTest)
      endif()
      file(REMOVE_RECURSE ${CMAKE_BINARY_DIR}/CMakeTmp/CompilerTest)
      if ("${COMPILER_RESULT}" MATCHES "0")
         message(STATUS "Compiler test ${_MSG} - Success")
         set(${_RESULT} 1 CACHE INTERNAL "Test ${VAR}")
      else()
         message(STATUS "Compiler test ${_MSG} - Failed")
		 message(STATUS "${COMPILER_OUTPUT}")
         set(${_RESULT} 0 CACHE INTERNAL "Test ${VAR}")
      endif()      
   endif()
ENDMACRO (CHECK_MUTATEE_COMPILER)
