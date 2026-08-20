include_guard(GLOBAL)

include(DyninstLibrary)

add_subdirectory(src/ASTs)
add_subdirectory(src/decoder)

set(_subprojects
    instructionAPI_decoder
    instructionAPI_ASTs)

set(_sources
    src/debug.C
    src/Instruction.C
    src/Operation.C
    src/Operand.C
    src/ArchSpecificFormatters.C
    src/syscalls.C
    src/interrupts.C)

set(_public_headers
    h/ArchSpecificFormatters.h
    h/BinaryFunction.h
    h/Dereference.h
    h/Expression.h
    h/Immediate.h
    h/InstructionAST.h
    h/InstructionCategories.h
    h/InstructionDecoder.h
    h/Instruction.h
    h/Operand.h
    h/Operation_impl.h
    h/Register.h
    h/MultiRegister.h
    h/Result.h
    h/Ternary.h
    h/Visitor.h)

set(_private_headers src/debug.h h/syscalls.h h/interrupts.h)

# cmake-format: off
dyninst_library(
  instructionAPI
  PUBLIC_HEADER_FILES ${_public_headers}
  PRIVATE_HEADER_FILES ${_private_headers}
  SOURCE_FILES ${_sources}
  DYNINST_DEPS common
  PUBLIC_DEPS Dyninst::Boost_headers
  PRIVATE_DEPS ${_subprojects}
)
# cmake-format: on

unset(_subprojects)
