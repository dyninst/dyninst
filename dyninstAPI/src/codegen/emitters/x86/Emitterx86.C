#include "addressSpace.h"
#include "arch-regs-x86.h"
#include "arch-x86.h"
#include "Architecture.h"
#include "binaryEdit.h"
#include "BPatch_memoryAccess_NP.h"
#include "codegen/emitters/x86/Emitterx86.h"
#include "codegen/RegControl.h"
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

}}
