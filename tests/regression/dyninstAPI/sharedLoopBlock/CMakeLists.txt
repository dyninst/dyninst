include_guard(GLOBAL)

# The input is x86_64 assembly; the test only parses it, never runs it.
dyninst_add_test_executable(
  sharedLoopBlock_input
  SOURCE_FILES input.cpp
  REQUIRES codegen:x86_64
)

dyninst_add_test(
  dyninstAPI_sharedLoopBlock_test
  TYPE regression
  SOURCE_FILES sharedLoopBlock.cpp
  DEPS dyninstAPI
  ARGS $<TARGET_FILE:sharedLoopBlock_input>
  REQUIRES codegen:x86_64
  PROPERTIES ENVIRONMENT DYNINSTAPI_RT_LIB=$<TARGET_FILE:dyninstAPI_RT>
)
