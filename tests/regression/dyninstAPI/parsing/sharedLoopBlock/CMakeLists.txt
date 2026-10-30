include_guard(GLOBAL)

dyninst_add_test(
  dyninstAPI_sharedLoopBlock_test
  TYPE regression
  SOURCE_FILES sharedLoopBlock.cpp
  DEPS dyninstAPI
  ARGS "${DYNINST_TEST_BINARIES_DIR}/dyninstAPI/parsing/sharedLoopBlock/x86_64"
  REQUIRES test_binaries codegen:x86_64
  PROPERTIES ENVIRONMENT DYNINSTAPI_RT_LIB=$<TARGET_FILE:dyninstAPI_RT>
)
