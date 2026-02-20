#include "AstAtomicOperationStmtNode.h"
#include "AstOperandNode.h"
#include "codegen.h"

#include <iomanip>
#include <sstream>

#if defined(DYNINST_CODEGEN_ARCH_AMDGPU_GFX908)
#include "emit-amdgpu.h"
#endif

namespace Dyninst { namespace DyninstAPI {

std::string AstAtomicOperationStmtNode::format(std::string indent) {
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

bool AstAtomicOperationStmtNode::generateCode_phase2(codeGen &gen, bool noCost, Address &retAddr,
                                                     Dyninst::Register & /* retReg */) {
  // This has 2 operands - variable and constant.
  // Codegen for atomic add has the following steps:
  // 1. Evaluate constant -- load a register with the constant
  // 2. Evaluate variable -- load a register pair with the address of the variable.
  // 3. Emit s_atomic_add instruction - atomically add the constant to the variable.

  bool ret = true;

  // TODO : Introduce register blocks and specifically allocate src0 as a SGPR block of size 1.
  Register src0 = Dyninst::Null_Register;
  if (!constant->generateCode_phase2(gen, noCost, retAddr, src0)) {
    fprintf(stderr, "WARNING: failed in generateCode internals!\n");
    ret = false;
  }
  assert(src0 != Dyninst::Null_Register);

  // Now generate code for the variable -- load a register pair with the address of the variable.
  AstOperandNode *variableOperand = dynamic_cast<AstOperandNode *>((AstNode *)variable.get());
  assert(variableOperand);
  assert(variableOperand->getoType() == operandType::AddressAsPlaceholderRegAndOffset);

  AstOperandNode *offset =
      dynamic_cast<AstOperandNode *>((AstNode *)variableOperand->operand().get());
  assert(offset);

  EmitterAmdgpuGfx908 *emitter = dynamic_cast<EmitterAmdgpuGfx908 *>(gen.emitter());
  assert(emitter);

  // TODO : allocate addrRegPair as a SGPR block of size 2.
  // TODO : baseRegPair should be the cached value for a particular kernel.
  Register addrRegPair(OperandRegId(88), RegKind::SCALAR, BlockSize(2));
  Register baseRegPair(OperandRegId(94), RegKind::SCALAR, BlockSize(2));

  std::vector<Register> const &addrRegs = addrRegPair.getIndividualRegisters();
  std::vector<Register> const &baseRegs = baseRegPair.getIndividualRegisters();

  emitter->emitMoveRegToReg(baseRegs[0], addrRegs[0], gen);
  emitter->emitMoveRegToReg(baseRegs[1], addrRegs[1], gen);

  emitter->emitAddConstantToRegPair(addrRegPair, (Address)offset->getOValue(), gen);

  // Now we have s[88:89] with address of the variable. Emit appropriate atomic instruction.
  switch (opcode) {
  case plusOp:
    emitter->emitAtomicAdd(addrRegPair, src0, gen);
    break;
  case minusOp:
    emitter->emitAtomicSub(addrRegPair, src0, gen);
    break;
  default:
    assert(!"atomic operation for this opcode is not implemented");
  }

  return ret;
}
#else
bool AstAtomicOperationStmtNode::generateCode_phase2(codeGen &, bool, Dyninst::Address &,
                                                     Dyninst::Register &) {
  return false;
}
#endif

}}
