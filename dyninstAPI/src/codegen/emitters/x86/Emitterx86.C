#include "addressSpace.h"
#include "arch-regs-x86.h"
#include "arch-x86.h"
#include "Architecture.h"
#include "binaryEdit.h"
#include "BPatch_memoryAccess_NP.h"
#include "codegen/emitters/x86/Emitterx86.h"
#include "codegen/RegControl.h"
#include "common/src/bitmath.h"
#include "debug.h"
#include "function.h"
#include "image.h"
#include "inst-x86.h"
#include "parse_func.h"
#include "registerSpace/registerSpace.h"
#include "Symbol.h"
#include "unaligned_memory_access.h"

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <limits>

namespace Dyninst { namespace DyninstAPI {

  /*
   * emit code for op(src1,src2, dest)
   * ibuf is an instruction buffer where instructions are generated
   * base is the next free position on ibuf where code is to be generated
   */
  codeBufIndex_t Emitterx86::emitA(opCode op, Dyninst::Register src1, long dest, codeGen &gen,
                       Dyninst::DyninstAPI::RegControl rc) {
    // retval is the address of the jump (if one is created).
    // It's always the _start_ of the jump, which means that if we need
    // to offset (like x86 (to - (from + insnsize))) we do it later.
    codeBufIndex_t retval = 0;

    switch(op) {
      case ifOp: {
        // if src1 == 0 jump to dest
        // src1 is a temporary
        // dest is a target address
        retval = gen.codeEmitter()->emitIf(src1, dest, rc, gen);
        break;
      }
      case branchOp: {
        // dest is the displacement from the current value of insn
        // this will need to work for both 32-bits and 64-bits
        // (since there is no JMP rel64)
        retval = gen.getIndex();
        insnCodeGen::generateBranch(gen, dest);
        break;
      }
      default:
        abort(); // unexpected op for this emit!
    }

    return retval;
  }

  Dyninst::Register Emitterx86::emitR(opCode op, Dyninst::Register src1, Dyninst::Register src2,
                                      Dyninst::Register dest, codeGen &gen,
                                      const instPoint *location) {
    bool get_addr_of = (src2 != Null_Register);
    switch(op) {
      case getRetValOp:
        // dest is a register where we can store the value
        // the return value is in the saved EAX
        gen.codeEmitter()->emitGetRetVal(dest, get_addr_of, gen);
        if(!get_addr_of) {
          return dest;
        }
        break;
      case getRetAddrOp:
        // dest is a register where we can store the return address
        gen.codeEmitter()->emitGetRetAddr(dest, gen);
        return dest;
        break;
      case getParamOp:
      case getParamAtCallOp:
      case getParamAtEntryOp:
        // src1 is the number of the argument
        // dest is a register where we can store the value
        gen.codeEmitter()->emitGetParam(dest, src1, location->type(), op, get_addr_of, gen);
        if(!get_addr_of) {
          return dest;
        }
        break;
      case loadRegOp:
        assert(src1 == 0);
        assert(0);
        return dest;
      default:
        abort(); // unexpected op for this emit!
    }
    assert(get_addr_of);
    emitV(storeIndirOp, src2, 0, dest, gen, gen.addrSpace()->getAddressWidth(), gen.addrSpace());
    return (dest);
  }

  void Emitterx86::emitV(opCode op, Dyninst::Register src1, Dyninst::Register src2,
                         Dyninst::Register dest, codeGen &gen, int size, AddressSpace * /* proc */,
                         bool s) {
    assert((op != branchOp) && (op != ifOp));                                  // !emitA
    assert((op != getRetValOp) && (op != getRetAddrOp) && (op != getParamOp)); // !emitR
    assert((op != loadOp) && (op != loadConstOp));                             // !emitVload
    assert((op != storeOp));                                                   // !emitVstore

    if(op == loadIndirOp) {
      // same as loadOp, but the value to load is already in a register
      gen.codeEmitter()->emitLoadIndir(dest, src1, size, gen);
    } else if(op == storeIndirOp) {
      // same as storeOp, but the address where to store is already in a
      // register
      gen.codeEmitter()->emitStoreIndir(dest, src1, size, gen);
    } else if(op == noOp) {
      emitSimpleInsn(NOP, gen); // nop
    } else if(op == saveRegOp) {
      // Push....
      assert(src2 == 0);
      assert(dest == 0);
      gen.codeEmitter()->emitPush(gen, src1);
    } else if(op == loadRegOp) {
      assert(src1 == 0);
      assert(src2 == 0);
      gen.codeEmitter()->emitPop(gen, dest);
    } else {
      unsigned opcode = 0; // initialize to placate gcc warnings
      switch(op) {
          // integer ops
        case plusOp:
          // dest = src1 + src2
          // mv eax, src1
          // add eax, src2
          // mov dest, eax
          opcode = 0x03; // ADD
          break;

        case minusOp:
          opcode = 0x2B; // SUB
          break;

        case xorOp:
          opcode = 0x33; // XOR
          break;

        case timesOp:
          if(s) {
            opcode = 0x0FAF; // IMUL
          } else {
            opcode = 0x0F74; // Unsigned Multiply
          }
          break;
        case divOp: {
          gen.codeEmitter()->emitDiv(dest, src1, src2, gen, s);
          return;
        }
          // Bool ops
        case orOp:
          opcode = 0x0B; // OR
          break;

        case andOp:
          opcode = 0x23; // AND
          break;

          // rel ops
          // dest = src1 relop src2
        case eqOp:
        case neOp:
        case lessOp:
        case leOp:
        case greaterOp:
        case geOp: {
          gen.codeEmitter()->emitRelOp(op, dest, src1, src2, gen, s);
          return;
          break;
        }
        default:
          abort();
          break;
      }
      gen.codeEmitter()->emitOp(opcode, dest, src1, src2, gen);
    }
    return;
  }

  void Emitterx86::emitVload(opCode op, Address src1, Dyninst::Register src2, Dyninst::Register dest,
                             codeGen &gen, int size, AddressSpace * /* proc */) {
    if(op == loadConstOp) {
      // dest is a temporary
      // src1 is an immediate value
      // dest = src1:imm32
      gen.codeEmitter()->emitLoadConst(dest, src1, gen);
      return;
    } else if(op == loadOp) {
      // dest is a temporary
      // src1 is the address of the operand
      // dest = [src1]
      gen.codeEmitter()->emitLoad(dest, src1, size, gen);
      return;
    } else if(op == loadFrameRelativeOp) {
      // dest is a temporary
      // src1 is the offset of the from the frame of the variable
      gen.codeEmitter()->emitLoadOrigFrameRelative(dest, src1, gen);
      return;
    } else if(op == loadRegRelativeOp) {
      // dest is a temporary
      // src2 is the register
      // src1 is the offset from the address in src2
      gen.codeEmitter()->emitLoadOrigRegRelative(dest, src1, src2, gen, true);
      return;
    } else if(op == loadRegRelativeAddr) {
      // dest is a temporary
      // src2 is the register
      // src1 is the offset from the address in src2
      gen.codeEmitter()->emitLoadOrigRegRelative(dest, src1, src2, gen, false);
      return;
    } else if(op == loadFrameAddr) {
      gen.codeEmitter()->emitLoadFrameAddr(dest, src1, gen);
      return;
    } else {
      abort(); // unexpected op for this emit!
    }
  }

  void Emitterx86::emitVstore(opCode op, Dyninst::Register src1, Dyninst::Register src2, Address dest,
                              codeGen &gen, int size, AddressSpace * /* proc */) {
    if(op == storeOp) {
      // [dest] = src1
      // dest has the address where src1 is to be stored
      // src1 is a temporary
      // src2 is a "scratch" register, we don't need it in this architecture
      gen.codeEmitter()->emitStore(dest, src1, size, gen);
      return;
    } else if(op == storeFrameRelativeOp) {
      // src1 is a temporary
      // src2 is a "scratch" register, we don't need it in this architecture
      // dest is the frame offset
      gen.codeEmitter()->emitStoreFrameRelative(dest, src1, src2, size, gen);
      return;
    } else {
      abort(); // unexpected op for this emit!
    }
  }

  // VG(11/07/01): Load in destination the effective address given
  // by the address descriptor. Used for memory access stuff.
  void Emitterx86::emitAddrSpecLoad(const BPatch_addrSpec_NP *as, Dyninst::Register dest, int stackShift,
                                    codeGen &gen) {
    // TODO 16-bit registers, rep hacks
    long imm = as->getImm();
    int ra = as->getReg(0);
    int rb = as->getReg(1);
    int sc = as->getScale();

    gen.codeEmitter()->emitASload(ra, rb, sc, imm, dest, stackShift, gen);
  }

  void Emitterx86::emitCountSpecLoad(const BPatch_countSpec_NP *as, Dyninst::Register dest, codeGen &gen) {
    // VG(7/30/02): different from ASload on this platform, no LEA business

    long imm = as->getImm();
    int ra = as->getReg(0);
    int rb = as->getReg(1);
    int sc = as->getScale();

    gen.codeEmitter()->emitCSload(ra, rb, sc, imm, dest, gen);
  }

  void Emitterx86::emitImm(opCode op, Dyninst::Register src1, RegValue src2imm,
                           Dyninst::Register dest, codeGen &gen, bool s) {
    if(op == storeOp) {
      // this doesn't seem to ever be called from ast.C (or anywhere) - gq

      // [dest] = src1
      // dest has the address where src1 is to be stored
      // src1 is an immediate value
      // src2 is a "scratch" register, we don't need it in this architecture
      emitMovImmToReg(RealRegister(REGNUM_EAX), dest, gen);
      emitMovImmToRM(RealRegister(REGNUM_EAX), 0, src1, gen);
    } else {
      unsigned opcode1;
      unsigned opcode2;
      switch(op) {
        // integer ops
        case plusOp:
          opcode1 = 0x81;
          opcode2 = 0x0; // ADD
          break;
        case minusOp:
          opcode1 = 0x81;
          opcode2 = 0x5; // SUB
          break;
        case xorOp:
          opcode1 = 0x81;
          opcode2 = 0x6; // XOR
          break;
        case timesOp:
          gen.codeEmitter()->emitTimesImm(dest, src1, src2imm, gen);
          return;
        case divOp:
          gen.codeEmitter()->emitDivImm(dest, src1, src2imm, gen, s);
          return;
        // Bool ops
        case orOp:
          opcode1 = 0x81;
          opcode2 = 0x1; // OR
          break;
        case andOp:
          opcode1 = 0x81;
          opcode2 = 0x4; // AND
          break;
        // rel ops
        // dest = src1 relop src2
        case eqOp:
        case neOp:
        case lessOp:
        case leOp:
        case greaterOp:
        case geOp:
          gen.codeEmitter()->emitRelOpImm(op, dest, src1, src2imm, gen, s);
          return;
        default:
          abort();
          break;
      }
      gen.codeEmitter()->emitOpImm(opcode1, opcode2, dest, src1, src2imm, gen);
    }
    return;
  }

  Address Emitterx86::getInterModuleFuncAddr(func_instance *func, codeGen &gen) {
    AddressSpace *addrSpace = gen.addrSpace();
    BinaryEdit *binEdit = addrSpace->edit();
    Address relocation_address;
    const unsigned int jump_slot_size = getArchAddressWidth(gen.getArch());

    if(!binEdit || !func) {
      assert(!"Invalid function call (function info is missing)");
    }

    SymtabAPI::Symbol *referring = func->getRelocSymbol();

    // have we added this relocation already?
    relocation_address = binEdit->getDependentRelocationAddr(referring);

    if(!relocation_address) {
      // inferiorMalloc addr location and initialize to zero
      relocation_address = binEdit->inferiorMalloc(jump_slot_size);
      unsigned char *dat = (unsigned char *)malloc(jump_slot_size);
      memset(dat, 0, jump_slot_size);
      binEdit->writeDataSpace((void *)relocation_address, jump_slot_size, dat);
      free(dat);

      // add write new relocation symbol/entry
      binEdit->addDependentRelocation(relocation_address, referring);
    }

    return relocation_address;
  }

  Address Emitterx86::getInterModuleVarAddr(const image_variable *var, codeGen &gen) {
    AddressSpace *addrSpace = gen.addrSpace();
    BinaryEdit *binEdit = addrSpace->edit();
    Address relocation_address;
    const unsigned int jump_slot_size = getArchAddressWidth(gen.getArch());

    if(!binEdit || !var) {
      assert(!"Invalid variable load (variable info is missing)");
    }

    // find the Symbol corresponding to the int_variable
    std::vector<SymtabAPI::Symbol *> syms;
    var->svar()->getSymbols(syms);

    if(syms.size() == 0) {
      char msg[256];
      sprintf(msg, "%s[%d]:  internal error:  cannot find symbol %s", __FILE__, __LINE__,
              var->symTabName().c_str());
      showErrorCallback(80, msg);
      assert(0);
    }

    // try to find a dynamic symbol
    // (take first static symbol if none are found)
    SymtabAPI::Symbol *referring = syms[0];
    for(unsigned k = 0; k < syms.size(); k++) {
      if(syms[k]->isInDynSymtab()) {
        referring = syms[k];
        break;
      }
    }

    // have we added this relocation already?
    relocation_address = binEdit->getDependentRelocationAddr(referring);

    if(!relocation_address) {
      // inferiorMalloc addr location and initialize to zero
      relocation_address = binEdit->inferiorMalloc(jump_slot_size);
      unsigned int dat = 0;
      binEdit->writeDataSpace((void *)relocation_address, jump_slot_size, &dat);

      // add write new relocation symbol/entry
      binEdit->addDependentRelocation(relocation_address, referring);
    }

    return relocation_address;
  }

  bool Emitterx86::can_optimize_as_shift(RegValue val, uint8_t max_num_bits) const {
    if(!Dyninst::isPowerOf2(val)) {
      return false;
    }

    // number of bits, n, such that val = 2^n
    boost::optional<uint8_t> n = Dyninst::ilog2(val);

    if(!n) {
      return false;
    }

    // Can the result fit in a single register without overflowing?
    return *n < max_num_bits;
  }


}}
