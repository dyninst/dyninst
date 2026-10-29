include_guard(GLOBAL)

dyninst_add_test(
  instructionapi_getUsedRegisters
  TYPE unit
  SOURCE_FILES getUsedRegisters.cpp
  DEPS instructionAPI
  ARGS "64"
)

dyninst_add_test(
  instructionapi_multiRegisterEquality
  TYPE unit
  SOURCE_FILES multiRegisterEquality.cpp
  DEPS instructionAPI
)
