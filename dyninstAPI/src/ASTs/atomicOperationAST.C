#include "atomicOperationAST.h"
#include "codegen.h"
#include "operandAST.h"

#include <iomanip>
#include <sstream>

#if defined(DYNINST_CODEGEN_ARCH_AMDGPU_GFX908)
#include "emit-amdgpu.h"
#endif

namespace Dyninst { namespace DyninstAPI {

std::string atomicOperationAST::format(std::string indent) {
  std::stringstream ret;

  ret << indent << "Op/" << std::hex << this << "("
      << "atomic " << format_opcode(opcode) << ")\n";

  if(variable) {
    ret << indent << variable->format(indent + "  ");
  }

  if(constant) {
    ret << indent << constant->format(indent + "  ");
  }

  return ret.str();
}

#if defined(DYNINST_CODEGEN_ARCH_AMDGPU_GFX908)

bool atomicOperationAST::generateCode_phase2(codeGen &gen, bool noCost, Address &retAddr,
                                                     Dyninst::Register &) {
  // This has 2 operands - variable and constant.
  // Codegen for atomic add has the following steps:
  // 1. Evaluate constant -- load a register with the constant
  // 2. Evaluate variable -- load a register pair with the address of the variable.
  // 3. Emit s_atomic_add instruction - atomically add the constant to the variable.

  bool ret = true;

  Register src0 = Dyninst::Null_Register;
  if(!constant->generateCode_phase2(gen, noCost, retAddr, src0)) {
    fprintf(stderr, "WARNING: failed in generateCode internals!\n");
    ret = false;
  }
  assert(src0 != Dyninst::Null_Register);

  // Now generate code for the variable -- load a register pair with the address of the variable.
  operandAST *variableOperand = dynamic_cast<operandAST *>((codeGenAST *)variable.get());
  assert(variableOperand);
  assert(variableOperand->getoType() == operandType::AddressAsPlaceholderRegAndOffset);

  operandAST *offset =
      dynamic_cast<operandAST *>((codeGenAST *)variableOperand->operand().get());
  assert(offset);

  EmitterAmdgpuGfx908 *emitter = dynamic_cast<EmitterAmdgpuGfx908 *>(gen.emitter());
  assert(emitter);

  // TODO : Remove all hardcoded registers.
  emitter->emitMoveRegToReg(94, 88, gen);
  emitter->emitMoveRegToReg(95, 89, gen);
  emitter->emitAddConstantToRegPair(88, (Address)offset->getOValue(), gen);

  // Now we have s[88:89] with address of the variable. Emit appropriate atomic instruction.
  switch(opcode) {
    case plusOp:
      emitter->emitAtomicAdd(88, src0, gen);
      break;
    case minusOp:
      emitter->emitAtomicSub(88, src0, gen);
      break;
    default:
      assert(!"atomic operation for this opcode is not implemented");
  }

  return ret;
}
#else
bool atomicOperationAST::generateCode_phase2(codeGen &, bool, Dyninst::Address &,
                                                     Dyninst::Register &) {
  return false;
}
#endif

}}
