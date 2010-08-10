/*
 * Copyright (c) 1996-2009 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "MemoryEmulator.h"

#include "Atom.h"
#include "Target.h"
#include "CFAtom.h" // CFPatch

// For our horribly horked memory effective address system
// Which I'm not fixing here. 
#include "dyninstAPI/h/BPatch_memoryAccess_NP.h"
#include "dyninstAPI/h/BPatch_addressSpace.h" // bpatch_address... you get the picture

// Memory hackitude
#include "dyninstAPI/src/emit-x86.h"
#include "dyninstAPI/src/inst-x86.h"

#include "instructionAPI/h/Instruction.h"
#include "dyninstAPI/src/addressSpace.h"

#include "dyninstAPI/src/debug.h"
#include "dyninstAPI/src/registerSpace.h"

using namespace Dyninst;
using namespace Relocation;
using namespace InstructionAPI;

MemEmulator::TranslatorMap MemEmulator::translators_;

MemEmulator::Ptr MemEmulator::create(Instruction::Ptr insn,
				     Address addr,
				     instPoint *point) {
  MemEmulator::Ptr ptr = MemEmulator::Ptr(new MemEmulator(insn, addr, point));
  return ptr;
}

void MemEmulator::initTranslators(TranslatorMap &t) {
  // Keep a copy so we don't have smart pointer issues.
  translators_ = t;
}

bool MemEmulator::generate(Trace &,  GenStack &gens) {
  codeGen &prepatch = gens();

  // We want to ensure that a memory operation produces its
  // original result in the face of overwriting the text
  // segment.
  //
  // We also believe that most operations do _not_ require any
  // modification, and so will be looking to fast-track any
  // that can be done without modification
  //
  // So, we want to take something like follows:
  //    result = *(EffectiveAddrCalc())
  // and turn it in to:
  //    addr = EffectiveAddrCalc()
  //    if (addr > lowWaterMark)
  //       result = *addr
  //    else
  //       result = *(addr + shiftVal)
  // where shiftVal and lowWaterMark are pre-defined constants.

  // x86 pseudocode version:
  // saveFlags
  // scratchReg = getASload(...)
  // cmp scratchReg <constant>
  // ja 1
  // translateMemOp <scratchReg>
  // restoreFlags
  // 1: origMemOp <scratchReg> 

  // First, get the effective address. We do this with existing
  // code... badly, but hey.
  
  // This is a bit rigmarole-ish. We first need an instPoint because
  // the entire memory system is wired that way. Next, we need
  // liveness info so we can determine which registers we can use.

  relocation_cerr << "    MemEmulator generating " << insn_->format() << "@"
		  << std::hex << addr_
		  << std::dec << endl;

  if (!initialize(prepatch)) return false;

  if (!checkLiveFlags(prepatch)) return false;

  if (!allocRegisters(prepatch)) {
    /*
    cerr << "Warning: failed to emulate memory insn @ " 
	 << std::hex << addr_ << std::dec
	 << ", no dead registers" << endl;
    */
    prepatch.copy(insn_->ptr(), insn_->size());
    return true;
  }
    
  if (!computeEffectiveAddress(prepatch)) {
    cerr << "Error: failed to compute eff. addr. of memory operation!" << endl;
    return false;
  }

  if (!saveFlags(prepatch)) { 
    cerr << "Error failed to save live flags!" << endl;
  }

#if 0
  assert(translators_[effAddr_]);
  // Call the appropriate translator for our particular effective address.
  TargetInt *targ = new Target<Trace::Ptr>(translators_[effAddr_]);
  CFPatch *patch = new CFPatch(CFPatch::Call,
			       Instruction::Ptr(),
			       targ);
  gens.addPatch(patch);
#endif
  
  DecisionTree dt(effAddr_);
  dt.generate(prepatch);

  codeGen &postpatch = gens();
  restoreFlags(postpatch);

  generateOrigAccess(postpatch);

  return true;
}

bool MemEmulator::initialize(codeGen &gen) {
  effAddr_ = Null_Register;
  saveFlags_ = false;
  saveOF_ = false;
  saveOthers_ = false;
  saveRAX_ = false;
  RAXWritten_ = false;
  RAXSave_ = Null_Register;

  // This is copied from ast.C
  gen.setPoint(point_);
  registerSpace *rs = registerSpace::actualRegSpace(point_, callPreInsn);
  gen.setRegisterSpace(rs);

  return true;
}

bool MemEmulator::checkLiveFlags(codeGen &gen) {

  if (gen.addrSpace()->getAddressWidth() == 8) {
    if ((*(gen.rs()))[REGNUM_OF]->liveState == registerSlot::live) {
      saveOF_ = true;
    }
    
    for (unsigned i = REGNUM_SF; i <= REGNUM_RF; i++) {
      if ((*(gen.rs()))[i]->liveState == registerSlot::live) {
	saveOthers_ = true;
	break;
      }
    }
    if (saveOF_ || saveOthers_) {
      saveFlags_ = true;
      if ((*(gen.rs()))[REGNUM_RAX]->liveState == registerSlot::live) {
	saveRAX_ = true;
      }
    }
  }
  else {
    // ARGH
    if ((*(gen.rs()))[IA32_FLAG_VIRTUAL_REGISTER]->liveState == registerSlot::live) {
      saveOthers_ = true;
      saveOF_ = true;
      saveFlags_ = true;
      if (gen.rs()->physicalRegs(REGNUM_EAX)->liveState == registerSlot::live) {
	saveRAX_ = true;
      }
      //cerr << "Flags are live; RAX is " << (saveRAX_ ? "live" : "dead") << endl;
    }
  }

  return true;
}


bool MemEmulator::allocRegisters(codeGen &gen) {
  // We use RAX to save flags, which means we don't want to
  // use it for an effective address
  gen.rs()->allocateSpecificRegister(gen, 0, true);

  // We want to grab a reg for the effective address
  // calculation and one for flags (if flags are live)
  //
  // We prefer RAX for flags since that makes life easy.
  effAddr_ = gen.rs()->allocateRegister(gen, false, true);
  // We should be able to do this on the stack, but that will
  // require more thought...
  if (effAddr_ == Null_Register) return false;
  if (!saveFlags_) return true;

  // If RAX is live, we must save its value somewhere
  if (saveRAX_) {
    RAXSave_ = gen.rs()->getScratchRegister(gen, false, true);
  }
  
  return true;
}
#include "dyninstAPI/src/RegisterConversion-x86.h"

bool MemEmulator::calcWriteSet(pdvector<Register> &excluded, bool writesRAX) {
  // Mostly stolen from liveness.C

  std::set<RegisterAST::Ptr> written;
  insn_->getWriteSet(written);
 
  for (std::set<RegisterAST::Ptr>::const_iterator i = written.begin(); 
       i != written.end(); i++) {
    bool upcast = false;
    
    int reg = convertRegID(*i, upcast);
    if (reg == REGNUM_RAX) writesRAX = true;
    excluded.push_back(reg);
  }
  return true;
}


bool MemEmulator::computeEffectiveAddress(codeGen &gen) {
  BPatch_addressSpace *bproc = (BPatch_addressSpace *)gen.addrSpace()->up_ptr();
  BPatch_point *bpoint = bproc->findOrCreateBPPoint(NULL, gen.point(), BPatch_locInstruction);
  if (bpoint == NULL) {
    fprintf(stderr, "ERROR: Unable to find BPatch point for internal point %p/0x%lx\n",
	    gen.point(), gen.point()->addr());
    return false;
  }
  const BPatch_memoryAccess *ma = bpoint->getMemoryAccess();
  
  const BPatch_addrSpec_NP *start = ma->getStartAddr(0); // Guessing on 0, here...
  emitASload(start, effAddr_, gen, true);

  return true;
}

bool MemEmulator::saveFlags(codeGen &gen) {
  if (saveFlags_ == false) return true;
  
    // Save flags
    // We can use sahf to push most flags to RAX
  //cerr << "saveFlags: rax is " << (saveRAX_ ? "live" : "dead") << endl;
  if (saveRAX_) {    
    if (RAXSave_ == Null_Register) {
      if (!createStackFrame(gen)) return false;
    }
    
    if (RAXSave_ == Null_Register) {
      // RAX has to go on the stack before we can use it
      emitPush(RealRegister(REGNUM_EAX), gen);
    }
    else {
      if (!moveRegister(REGNUM_RAX, RAXSave_, gen)) return false;
    }
  }

  // Move flags to RAX

  // If others are live, emit an SAHF
  if (saveOthers_) {
    emitSimpleInsn(0x9f, gen);
  }
  // If OF is live, save it
  if (saveOF_) {
    emitSaveO(gen);
  }

  return true;
}

bool MemEmulator::restoreFlags(codeGen &gen) {
  if (saveFlags_ == false) return true;

  if (saveOF_)
    emitRestoreO(gen);
  if (saveOthers_)
    emitSimpleInsn(0x9E, gen);

  if (saveRAX_) {
    if (RAXSave_ == Null_Register) {
      emitPop(RealRegister(REGNUM_EAX), gen);
    }
    else {
      // We moved the original elsewhere
      if (!moveRegister(RAXSave_, REGNUM_RAX, gen)) return false;
    }
    
    if (RAXSave_ == Null_Register) {
      if (!destroyStackFrame(gen)) return false;
    }
  }

  return true;
}

bool MemEmulator::generateOrigAccess(codeGen &gen) {
  // Okay, theoretically effAddr_ holds the memory address.
  // Should do a compare here; for now, just drop out the instruction
  // and see what happens.
  instruction ugly_insn(insn_->ptr());

  if (!insnCodeGen::generateMem(gen,
				ugly_insn,
				0, // ignored
				0, // ignored
				effAddr_,
				Null_Register))
    return false;
  return true;
}

bool MemEmulator::createStackFrame(codeGen &gen) {
  if (gen.addrSpace()->getAddressWidth() == 8) {
#if defined(arch_x86_64)
    emitLEA64(REGNUM_RSP, Null_Register, 0, -1*STACK_PAD_CONSTANT, REGNUM_RSP, true, gen);
#endif
  }
  else {
    emitLEA(RealRegister(REGNUM_ESP), RealRegister(Null_Register), 0,
	    -1*STACK_PAD_CONSTANT, RealRegister(REGNUM_ESP), gen);
  }    
  return true;
}

bool MemEmulator::destroyStackFrame(codeGen &gen) {
  if (gen.addrSpace()->getAddressWidth() == 8) {
#if defined(arch_x86_64)
    emitLEA64(REGNUM_RSP, Null_Register, 0, STACK_PAD_CONSTANT, REGNUM_RSP, true, gen);
#endif
  }
  else {
    emitLEA(RealRegister(REGNUM_ESP), RealRegister(Null_Register), 0,
	    STACK_PAD_CONSTANT, RealRegister(REGNUM_ESP), gen);
  }    
  return true;
}

bool MemEmulator::moveRegister(Register from, Register to, codeGen &gen) {
  if (gen.addrSpace()->getAddressWidth() == 8) {
#if defined(arch_x86_64)
	emitMovRegToReg64(to, from, true, gen);
#endif
  }
  else {
    emitMovRegToReg(RealRegister(to), RealRegister(from), gen);
  }
  return true;
}

string MemEmulator::format() const {
  stringstream ret;
  ret << "MemE(" << insn_->format()
      << "," << std::hex << addr_ << std::dec
      << ")";

  return ret.str();
}



//////////////////////////////////////////////////////////

MemEmulatorTranslator::Ptr MemEmulatorTranslator::create(Register r) {
  return Ptr(new MemEmulatorTranslator(r));
}

bool MemEmulatorTranslator::generate(Trace &, GenStack &gens) {
  DecisionTree dt(reg_);
  codeGen &gen = gens();

  dt.generate(gen);

  generateReturn(gen);

  return true;
}

string MemEmulatorTranslator::format() const { 
  stringstream ret;
  ret << "MemE[T](" << reg_ << ")";
  return ret.str();
}

bool MemEmulatorTranslator::generateReturn(codeGen &gen) { 
  GET_PTR(insn, gen);
  *insn++ = 0xc3; // return
  SET_PTR(insn, gen);
  return true;
}

////////////////////////////////

bool DecisionTree::generate(codeGen &gen) {
  vector<codeBufIndex_t> origPatches;
  vector<codeBufIndex_t> textPatches;
  vector<codeBufIndex_t> instPatches;

  origPatches.push_back(generateCompare(gen, gen.addrSpace()->heapBase()));
  instPatches.push_back(generateCompare(gen, gen.addrSpace()->instBase()));
  origPatches.push_back(generateCompare(gen, gen.addrSpace()->dataBase()));
  textPatches.push_back(generateCompare(gen, gen.addrSpace()->textBase()));
  origPatches.push_back(generateSkip(gen));

  codeBufIndex_t instShift = generateInst(gen);
  origPatches.push_back(generateSkip(gen));
  codeBufIndex_t textShift = generateText(gen);

  codeBufIndex_t origShift = gen.getIndex();

  generateJCC(gen, origShift, origPatches);
  generateJCC(gen, textShift, textPatches);
  generateJCC(gen, instShift, instPatches);
  gen.setIndex(origShift);

  return true;
}


codeBufIndex_t DecisionTree::generateSkip(codeGen &gen) {
  // A short branch
  GET_PTR(jumpBuf, gen);
  *jumpBuf++ = 0xEB;
  SET_PTR(jumpBuf, gen);
  REGET_PTR(jumpBuf, gen);
  codeBufIndex_t ret = gen.getIndex();
  *jumpBuf++ = 0x0;
  SET_PTR(jumpBuf, gen);
  return ret;
}

codeBufIndex_t DecisionTree::generateOrig(codeGen &gen) {
  codeBufIndex_t ret = gen.getIndex();
  return ret;
}  

codeBufIndex_t DecisionTree::generateText(codeGen &gen) {
  // We want to add a constant to the register holding the effective
  // address...
  codeBufIndex_t ret = gen.getIndex();
  /*  
  emitLEA(RealRegister(effAddr_), 
	  RealRegister(Null_Register), 
	  0, 
	  0x0000, 
	  RealRegister(effAddr_), 
	  gen);
  */
  return ret;
}

codeBufIndex_t DecisionTree::generateInst(codeGen &gen) {
  codeBufIndex_t ret = gen.getIndex();
  // We want to zero out whichever register we're using
  // for the effective address so that derefing it will
  // result in a SEGV. Do so with an xor. 
  // The 9* is actually (effAddr_ + 8*effAddr_)
   /*
     unsigned char modrm = 0xC0 + 9*effAddr_;
     
     GET_PTR(buf, gen);
     *buf++ = 0x31;
     *buf++ = modrm;
     SET_PTR(buf, gen);
   */
   return ret;
}

// Generate a <compare, jcc> sequence, returning the
// codeBufIndex_t of the jcc's destination. 

codeBufIndex_t DecisionTree::generateCompare(codeGen &gen, Address comp) {

if (gen.addrSpace()->getAddressWidth() == 8) {
#if defined(arch_x86_64)
	emitOpRegImm64(0x81, EXTENDED_0x81_CMP, 
		   effAddr_, comp, 
		   true, gen);
#endif
  }
  else {
    emitOpExtRegImm(0x81, EXTENDED_0x81_CMP,
		    RealRegister(effAddr_), comp, 
		    gen);
  }

  GET_PTR(jumpBuf, gen);
  *jumpBuf++ = 0x73;
  SET_PTR(jumpBuf, gen);
  REGET_PTR(jumpBuf, gen);
  codeBufIndex_t ret = gen.getIndex();
  *jumpBuf++ = 0x0;
  SET_PTR(jumpBuf, gen);
  return ret;
}

void DecisionTree::generateJumps(codeGen &gen,
				codeBufIndex_t target,
				vector<codeBufIndex_t> &sources) {
  for (unsigned i = 0; i < sources.size(); ++i) {
    gen.setIndex(sources[i]);
    GET_PTR(jumpBuf, gen);
    *jumpBuf = gen.getDisplacement(sources[i], target) - 1;
  }
}

void DecisionTree::generateJCC(codeGen &gen,
			      codeBufIndex_t target,
			      vector<codeBufIndex_t> &sources) {
  generateJumps(gen, target, sources);
}

