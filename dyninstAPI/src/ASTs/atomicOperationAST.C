#include "atomicOperationAST.h"
#include "codegen.h"
#include "debug.h"
#include "operandAST.h"
#include "registerSpace.h"

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

bool atomicOperationAST::generateCode_phase2(codeGen &gen, Address &retAddr,
                                                     Dyninst::Register &) {
  // This has 2 operands - variable and constant.
  // Codegen for atomic add has the following steps:
  // 1. Evaluate constant -- load a register with the constant
  // 2. Evaluate variable -- load a register pair with the address of the variable.
  // 3. Emit s_atomic_add instruction - atomically add the constant to the variable.

  bool ret = true;

  Dyninst::Register src0 = Dyninst::Null_Register;
  if(!constant->generateCode_phase2(gen, retAddr, src0)) {
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

  registerSpace *regSpace = gen.rs();

  Dyninst::Register addrRegPair =
      regSpace->allocateGprBlock(RegKind::SCALAR, /* numRegs */2, NS_amdgpu::PAIR_ALIGNMENT);

  assert(addrRegPair != Null_Register &&
         "atomicOperationAST : Failed to allocate register pair");

  const uint32_t addrRegId = addrRegPair.getId();
  ast_printf("atomicOperationAST allocated scalar register block s[%u:%u]\n", addrRegId,
             addrRegId + 1);

  // TODO this needs to pick up the register from placeholderReg
  Dyninst::Register baseRegPair = Dyninst::Register(OperandRegId(94), RegKind::SCALAR, BlockSize(2));

  // TODO: make movs work with blocks.
  std::vector<Dyninst::Register> addrRegs = addrRegPair.getIndividualRegisters();
  std::vector<Dyninst::Register> baseRegs = baseRegPair.getIndividualRegisters();

  emitter->emitMoveRegToReg(baseRegs[0], addrRegs[0], gen);
  emitter->emitMoveRegToReg(baseRegs[1], addrRegs[1], gen);
  emitter->emitAddConstantToRegPair(addrRegPair, (Address)offset->getOValue(), gen);

  switch(opcode) {
    case plusOp:
      emitter->emitAtomicAdd(addrRegPair, src0, gen);
      break;
    case minusOp:
      emitter->emitAtomicSub(addrRegPair, src0, gen);
      break;
    default:
      assert(!"atomic operation for this opcode is not implemented");
  }

  regSpace->freeGprBlock(addrRegPair);

  return ret;
}
#else
bool atomicOperationAST::generateCode_phase2(codeGen &, Dyninst::Address &,
                                                     Dyninst::Register &) {
  return false;
}
#endif

}}
