#include "ast_helpers.h"
#include "BPatch.h"
#include "BPatch_collections.h"
#include "BPatch_function.h"
#include "codegen.h"
#include "debug.h"
#include "headers.h"
#include "mapped_module.h"
#include "mapped_object.h"
#include "operandAST.h"
#include "registerSpace.h"

#include <iomanip>
#include <sstream>

namespace Dyninst { namespace DyninstAPI {

#if defined(DYNINST_CODEGEN_ARCH_AMDGPU_GFX908)
int operandAST::lastOffset = 0;
std::map<std::string, int> operandAST::allocTable = {{"--init--", -1}};
#endif

// Direct operand
operandAST::operandAST(operandType ot, void *arg) : oType(ot), oVar(NULL), operand_() {

  if(ot == operandType::ConstantString) {
    oValue = (void *)P_strdup((char *)arg);
  } else {
    oValue = (void *)arg;
  }

  if(operand_) {
    children.push_back(operand_);
  }
}

// And an indirect (say, a load)
operandAST::operandAST(operandType ot, codeGenASTPtr l)
    : oType(ot), oValue(NULL), oVar(NULL), operand_(l) {
  if(operand_) {
    children.push_back(operand_);
  }
}

operandAST::operandAST(operandType ot, const image_variable *iv)
    : oType(ot), oValue(NULL), oVar(iv), operand_() {
  assert(oVar);
  if(operand_) {
    children.push_back(operand_);
  }
}

bool operandAST::generateCode_phase2(codeGen &gen, bool noCost, Address &,
                                         Dyninst::Register &retReg) {
  RETURN_KEPT_REG(retReg);

  Address addr = ADDR_NULL;
  Dyninst::Register src = Dyninst::Null_Register;

  // Allocate a register to return
  if(retReg == Dyninst::Null_Register) {
    retReg = allocateAndKeep(gen, noCost);
  }
  Dyninst::Register temp;
  int tSize;
  int len;
  BPatch_type *Type;
  switch(oType) {
#if defined(DYNINST_CODEGEN_ARCH_AMDGPU_GFX908)
    case operandType::Constant: {
      assert(oVar == NULL);
      // Move constant into retReg
      Emitter *emitter = gen.emitter();
      const uint32_t immediateValue = (uint32_t)((uint64_t)this->getOValue());
      emitter->emitMovLiteral(retReg, immediateValue, gen);
      break;
    }
#else
    case operandType::Constant:
      assert(oVar == NULL);
      emitVload(loadConstOp, (Address)oValue, retReg, retReg, gen, noCost, gen.rs(), size,
                gen.point(), gen.addrSpace());
      break;
#endif
    case operandType::DataIndir:
      if(!operand_->generateCode_phase2(gen, noCost, addr, src)) {
        ERROR_RETURN;
      }
      REGISTER_CHECK(src);
      Type = const_cast<BPatch_type *>(getType());
      // Internally generated calls will not have type information set
      if(Type) {
        tSize = Type->getSize();
      } else {
        tSize = sizeof(long);
      }
      emitV(loadIndirOp, src, 0, retReg, gen, noCost, gen.rs(), tSize, gen.point(),
            gen.addrSpace());
      gen.rs()->freeRegister(src);
      break;
    case operandType::origRegister:
      gen.rs()->readProgramRegister(gen, (Dyninst::Register)(long)oValue, retReg, size);
      // emitLoadPreviousStackFrameRegister((Address) oValue, retReg, gen,
      // size, noCost);
      break;
    case operandType::variableAddr:
      assert(oVar);
      emitVariableLoad(loadConstOp, retReg, retReg, gen, noCost, gen.rs(), size, gen.point(),
                       gen.addrSpace());
      break;
    case operandType::variableValue:
      assert(oVar);
      emitVariableLoad(loadOp, retReg, retReg, gen, noCost, gen.rs(), size, gen.point(),
                       gen.addrSpace());
      break;
    case operandType::ReturnVal:
      src = emitR(getRetValOp, 0, Dyninst::Null_Register, retReg, gen, noCost, gen.point(),
                  gen.addrSpace()->multithread_capable());
      REGISTER_CHECK(src);
      if(src != retReg) {
        // Move src to retReg. Can't simply return src, since it was not
        // allocated properly
        emitImm(orOp, src, 0, retReg, gen, noCost, gen.rs());
      }
      break;
    case operandType::ReturnAddr:
      src = emitR(getRetAddrOp, 0, Dyninst::Null_Register, retReg, gen, noCost, gen.point(),
                  gen.addrSpace()->multithread_capable());
      REGISTER_CHECK(src);
      if(src != retReg) {
        // Move src to retReg. Can't simply return src, since it was not
        // allocated properly
        emitImm(orOp, src, 0, retReg, gen, noCost, gen.rs());
      }
      break;
    case operandType::Param:
    case operandType::ParamAtCall:
    case operandType::ParamAtEntry: {
      opCode paramOp = undefOp;
      switch(oType) {
        case operandType::Param:
          paramOp = getParamOp;
          break;
        case operandType::ParamAtCall:
          paramOp = getParamAtCallOp;
          break;
        case operandType::ParamAtEntry:
          paramOp = getParamAtEntryOp;
          break;
        default:
          assert(0);
          break;
      }
      src = emitR(paramOp, (Address)oValue, Dyninst::Null_Register, retReg, gen, noCost,
                  gen.point(), gen.addrSpace()->multithread_capable());
      REGISTER_CHECK(src);
      if(src != retReg) {
        // Move src to retReg. Can't simply return src, since it was not
        // allocated properly
        emitImm(orOp, src, 0, retReg, gen, noCost, gen.rs());
      }
    } break;
    case operandType::DataAddr:
      assert(oVar == NULL);
      addr = reinterpret_cast<Address>(oValue);
      emitVload(loadOp, addr, retReg, retReg, gen, noCost, NULL, size, gen.point(),
                gen.addrSpace());
      break;
    case operandType::FrameAddr:
      addr = (Address)oValue;
      temp = gen.rs()->allocateRegister(gen, noCost);
      emitVload(loadFrameRelativeOp, addr, temp, retReg, gen, noCost, gen.rs(), size, gen.point(),
                gen.addrSpace());
      gen.rs()->freeRegister(temp);
      break;
    case operandType::RegOffset:
      // Prepare offset from value in any general register (not just fp).
      // This codeGenAST holds the register number, and loperand holds offset.
      assert(operand_);
      addr = (Address)operand_->getOValue();
      emitVload(loadRegRelativeOp, addr, (long)oValue, retReg, gen, noCost, gen.rs(), size,
                gen.point(), gen.addrSpace());
      break;
    case operandType::ConstantString:
      // XXX This is for the std::string type.  If/when we fix the std::string type
      // to make it less of a hack, we'll need to change this.
      len = strlen((char *)oValue) + 1;

      addr = (Address)gen.addrSpace()->inferiorMalloc(len, dataHeap); // dataheap

      if(!gen.addrSpace()->writeDataSpace((char *)addr, len, (char *)oValue)) {
        ast_printf("Failed to write string constant into mutatee\n");
        return false;
      }

      if(!gen.addrSpace()->needsPIC()) {
        emitVload(loadConstOp, addr, retReg, retReg, gen, noCost, gen.rs(), size, gen.point(),
                  gen.addrSpace());
      } else {
        gen.codeEmitter()->emitLoadShared(loadConstOp, retReg, NULL, true, size, gen, addr);
      }
      break;
    default:
      fprintf(stderr, "[%s:%d] ERROR: Unknown operand type %d in AstOperandNode generation\n",
              __FILE__, __LINE__, static_cast<int>(oType));
      return false;
      break;
  }
  decUseCount(gen);
  return true;
}

bool operandAST::usesAppRegister() const {
  if(oType == operandType::FrameAddr || oType == operandType::RegOffset ||
     oType == operandType::origRegister || oType == operandType::Param ||
     oType == operandType::ParamAtEntry || oType == operandType::ParamAtCall ||
     oType == operandType::ReturnVal) {
    return true;
  }

  if(operand_ && operand_->usesAppRegister()) {
    return true;
  }
  return false;
}

int_variable *operandAST::lookUpVar(AddressSpace *as) {
  mapped_module *mod = as->findModule(oVar->pdmod()->fileName());
  if(mod && mod->obj()) // && (oVar->pdmod() == mod->pmod()))
  {
    int_variable *tmp = mod->obj()->findVariable(const_cast<image_variable *>(oVar));
    return tmp;
  }
  return NULL;
}

void operandAST::emitVariableLoad(opCode op, Dyninst::Register src2, Dyninst::Register dest,
                                      codeGen &gen, bool noCost, registerSpace *rs, int size_,
                                      const instPoint *point, AddressSpace *as) {
  int_variable *var = lookUpVar(as);
  if(var && !as->needsPIC(var)) {
    emitVload(op, var->getAddress(), src2, dest, gen, noCost, rs, size_, point, as);
  } else {
    gen.codeEmitter()->emitLoadShared(op, dest, oVar, (var != NULL), size_, gen, 0);
  }
}

void operandAST::emitVariableStore(opCode op, Dyninst::Register src1, Dyninst::Register src2,
                                       codeGen &gen, bool noCost, registerSpace *rs, int size_,
                                       const instPoint *point, AddressSpace *as) {
  int_variable *var = lookUpVar(as);
  if(var && !as->needsPIC(var)) {
    emitVstore(op, src1, src2, var->getAddress(), gen, noCost, rs, size_, point, as);
  } else {
    gen.codeEmitter()->emitStoreShared(src1, oVar, (var != NULL), size_, gen);
  }
}

bool operandAST::initRegisters(codeGen &g) {
  bool ret = true;
  for(unsigned i = 0; i < children.size(); i++) {
    if(!children[i]->initRegisters(g)) {
      ret = false;
    }
  }

  // If we're an origRegister, override its state as live.
  if(oType == operandType::origRegister) {
    Address origReg = (Address)oValue;
    // Mark that register as live so we are sure to save it.
    registerSlot *r = (*(g.rs()))[origReg];
    r->liveState = registerSlot::live;
  }

  return ret;
}

std::string operandAST::format(std::string indent) {
  std::stringstream ret;
  ret << indent << "Oper/" << hex << this << dec << "(" << format_operand(oType) << "/" << oValue
      << ")" << endl;
  if(operand_) {
    ret << indent << operand_->format(indent + "  ");
  }

  return ret.str();
}

BPatch_type *operandAST::checkType(BPatch_function *func) {
  BPatch_type *ret = NULL;
  BPatch_type *type = NULL;
  bool errorFlag = false;

  assert(BPatch::bpatch != NULL); /* We'll use this later. */

  if(operand_ && getType()) {
    // something has already set the type for us.
    // this is likely an expression for array access
    ret = const_cast<BPatch_type *>(getType());
    return ret;
  }

  if(operand_) {
    type = operand_->checkType(func);
  }

  if(type == BPatch::bpatch->type_Error) {
    errorFlag = true;
  }

  if(oType == operandType::DataIndir) {
    // XXX Should really be pointer to lType -- jkh 7/23/99
    ret = BPatch::bpatch->type_Untyped;
  } else if((oType == operandType::Param) || (oType == operandType::ParamAtCall) ||
            (oType == operandType::ParamAtEntry) || (oType == operandType::ReturnVal) ||
            (oType == operandType::ReturnAddr)) {
    if(func) {
      switch(oType) {
        case operandType::ReturnVal: {
          ret = func->getReturnType();
          if(!ret || (ret->isCompatible(BPatch::bpatch->builtInTypes->findBuiltInType("void")))) {
            if(ret) {
              errorFlag = true;
            }
            ret = BPatch::bpatch->type_Untyped;
          }
          break;
        }
        default:
          ret = BPatch::bpatch->type_Untyped;
      }

    } else {
      // If we don't have a function context, then ignore types
      ret = BPatch::bpatch->type_Untyped;
    }
  } else if(oType == operandType::origRegister) {
    ret = BPatch::bpatch->type_Untyped;
  } else {
    ret = const_cast<BPatch_type *>(getType());
  }
  assert(ret != NULL);

  if(errorFlag && doTypeCheck) {
    ret = BPatch::bpatch->type_Error;
  } else if(errorFlag) {
    ret = BPatch::bpatch->type_Untyped;
  }

  // remember what type we are
  setType(ret);

  return ret;
}

bool operandAST::canBeKept() const {

  switch(oType) {
    case operandType::DataIndir:
    case operandType::RegOffset:
    case operandType::origRegister:
    case operandType::DataAddr:
    case operandType::variableValue:
      return false;
    default:
      break;
  }
  if(operand_ && !operand_->canBeKept()) {
    return false;
  }
  return true;
}

}}
