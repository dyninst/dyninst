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


#include "dyninstAPI/src/Relocation/Atoms/Atom.h"
#include "dyninstAPI/src/Relocation/Atoms/Target.h"
#include "dyninstAPI/src/Relocation/Atoms/CFAtom.h" // CFPatch

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

#include "dyninstAPI/src/Relocation/CodeBuffer.h"

#include "memEmulatorAtom.h"

#include "dyninstAPI/src/RegisterConversion-x86.h"

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

TrackerElement *MemEmulator::tracker(int_function *f) const {
   EmulatorTracker *e = new EmulatorTracker(addr_, f);
  return e;
}

bool MemEmulator::generate(const codeGen &templ,
                           const Trace *t,
                           CodeBuffer &buffer) {
   if (generateViaOverride(templ, t, buffer))
      return true;
   if (generateViaModRM(templ, t, buffer))
      return true;
   return false;
}

bool MemEmulator::generateViaOverride(const codeGen &templ,
                                      const Trace *t,
                                      CodeBuffer &buffer) {
   const InstructionAPI::Operation &op = insn_->getOperation();

   switch(op.getID()) {
      case e_scasb:
      case e_scasd:
      case e_scasw:
         return generateSCAS(templ, t, buffer);
         break;
      case e_lodsb:
      case e_lodsd:
      case e_lodsw:
         return generateLODS(templ, t, buffer);
         break;
      default:
         // WTF?
         break;
   }
   return false;
}

bool MemEmulator::generateViaModRM(const codeGen &templ,
                                   const Trace *t, 
                                   CodeBuffer &buffer) {
   // We need a BPatch_something to do the memory handling. If that's 
   // not present, assume we don't need to emulate this piece of
   // memory.
   if (!templ.addrSpace()->up_ptr()) {
      buffer.addPIC(insn_->ptr(), insn_->size(), tracker(t->bbl()->func()));
      return true;
   }

   codeGen prepatch(128);
   prepatch.applyTemplate(templ);

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

  if (!initialize(prepatch)) {
     relocation_cerr << "\tInitialize failed, ret false" << endl;
     return false;
  }

  if (!checkLiveness(prepatch)) {
     relocation_cerr << "\tFlag check failed, ret false" << endl;
     return false;
  }

  if (!setupFrame(prepatch)) {
     cerr << " FAILED TO ALLOC REGISTERS for insn @ " << hex << addr() << dec << endl;
     buffer.addPIC(insn_->ptr(), insn_->size(), tracker(t->bbl()->func()));
     return false;
  }
    
  if (!computeEffectiveAddress(prepatch)) {
     relocation_cerr << "\tFailed to compute eff. addr. of memory operation!" << endl;
    return false;
  }

  // push/pop time!
  if (!preCallSave(prepatch))
     return false;
  buffer.addPIC(prepatch, tracker(t->bbl()->func()));
  
  buffer.addPatch(new MemEmulatorPatch(effAddr_, getTranslatorAddr(prepatch),point_), tracker(t->bbl()->func()));
  
  prepatch.setIndex(0);
  if (!postCallRestore(prepatch))
     return false;

  if (!teardownFrame(prepatch))
     return false;

  generateOrigAccess(prepatch);

  if (!trailingTeardown(prepatch)) {
     return false;
  }

  buffer.addPIC(prepatch, tracker(t->bbl()->func()));

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
  restoreEffAddr_ = false;

  // This is copied from ast.C
  gen.setPoint(point_);
  registerSpace *rs = registerSpace::actualRegSpace(point_, callPreInsn);
  gen.setRegisterSpace(rs);

  return true;
}

bool MemEmulator::checkLiveness(codeGen &gen) {
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
    }
  }
  else {
     // ARGH
     if ((*(gen.rs()))[IA32_FLAG_VIRTUAL_REGISTER]->liveState == registerSlot::live) {
        saveOthers_ = true;
        saveOF_ = true;
        saveFlags_ = true;
        //cerr << "Flags are live; RAX is " << (saveRAX_ ? "live" : "dead") << endl;
     }
  }
  
  // We need to use RAX to save flaggage
  if ((*(gen.rs()))[REGNUM_RAX]->liveState == registerSlot::live) {
     saveRAX_ = true;
  }

  return true;
}


bool MemEmulator::setupFrame(codeGen &gen) {
   // Goals:
   // To free a register for the modified effective address
   // To save flags (if live)
   gen.rs()->allocateSpecificRegister(gen, REGNUM_EAX, true);

   effAddr_ = gen.rs()->allocateRegister(gen, false, true);
   if (effAddr_ == Null_Register) {
      if (!stealEffectiveAddr(gen)) {
         return false;
      }
   }      

   if (saveRAX_) {
      ::emitPush(RealRegister(REGNUM_EAX), gen);
   }

   if (saveFlags_) {
      if (!saveFlags(gen)) return false;
   }
  
   return true;
}

bool MemEmulator::computeEffectiveAddress(codeGen &gen) {
  assert(gen.addrSpace());
  BPatch_addressSpace *bproc = (BPatch_addressSpace *)gen.addrSpace()->up_ptr();
  assert(bproc);
  assert(gen.point());
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

bool MemEmulator::teardownFrame(codeGen &gen) {
   // Reverse the order of operations performed in setupFrame
   // 1) Restore flags if live
   // 2) Restore RAX if saveRAX
   // 3) DO NOT restore the effAddr if we stole it; that happens
   //    after the memory access
   if (saveFlags_) {
      if (!restoreFlags(gen)) return false;
   }

   if (saveRAX_) {
      ::emitPop(RealRegister(REGNUM_EAX), gen);
   }
   return true;
}
   
bool MemEmulator::trailingTeardown(codeGen &gen) {
   if (restoreEffAddr_) {
      ::emitPop(RealRegister(effAddr_), gen);
   }
   return true;
}

bool MemEmulator::saveFlags(codeGen &gen) {
  if (saveFlags_ == false) return true;
  
  // If others are live, emit an SAHF
  if (saveOthers_) {
    emitSimpleInsn(0x9f, gen);
  }
  // If OF is live, save it
  if (saveOF_) {
    emitSaveO(gen);
  }

  // Get 'em out of here
  ::emitPush(RealRegister(REGNUM_EAX), gen);

  return true;
}

bool MemEmulator::restoreFlags(codeGen &gen) {
  if (saveFlags_ == false) return true;

  ::emitPop(RealRegister(REGNUM_EAX), gen);

  if (saveOF_)
    emitRestoreO(gen);
  if (saveOthers_)
    emitSimpleInsn(0x9E, gen);

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

bool MemEmulator::pushRegIfLive(registerSlot *reg, codeGen &gen) {
   if (reg->encoding() == effAddr_) {
      // Don't save it, as we're overwriting it anyway.
      return true;
   }

   if (reg->liveState == registerSlot::live || 1) {
      ::emitPush(RealRegister(reg->encoding()), gen);
      reg->liveState = registerSlot::spilled;
   }
   return true;
}

bool MemEmulator::popRegIfSaved(registerSlot *reg, codeGen &gen) {
   if (reg->liveState == registerSlot::spilled) {
      ::emitPop(RealRegister(reg->encoding()), gen);
      reg->liveState = registerSlot::live;
   }
   return true;
}

bool MemEmulator::preCallSave(codeGen &gen) {
   // Push registers eax, ecx, edx if live
   registerSlot *eax = (*(gen.rs()))[REGNUM_EAX];
   registerSlot *ecx = (*(gen.rs()))[REGNUM_ECX];
   registerSlot *edx = (*(gen.rs()))[REGNUM_EDX];
   pushRegIfLive(eax, gen);
   pushRegIfLive(ecx, gen);
   pushRegIfLive(edx, gen);
   
   return true;
}

bool MemEmulator::postCallRestore(codeGen &gen) {
   registerSlot *eax = (*(gen.rs()))[REGNUM_EAX];
   registerSlot *ecx = (*(gen.rs()))[REGNUM_ECX];
   registerSlot *edx = (*(gen.rs()))[REGNUM_EDX];

   popRegIfSaved(edx, gen);
   popRegIfSaved(ecx, gen);
   popRegIfSaved(eax, gen);

   return true;
}

bool MemEmulator::emitCallToTranslator(CodeBuffer &) {
   return true;
}
   
Address MemEmulator::getTranslatorAddr(codeGen &gen) {

   // Function lookup time
   int_function *func = gen.addrSpace()->findOnlyOneFunction("RTtranslateMemory");
   // FIXME for static rewriting; this is a dynamic-only hack for proof of concept.
   if (!func) return 0;
   // assert(func);
   return func->getAddress();
}

bool MemEmulator::generateSCAS(const codeGen &templ, const Trace *t, CodeBuffer &buffer) {
   cerr << "GENERATING SCAS FORM @ " << hex << addr_ << dec << endl;
   // This is an implicit use of... EDI. We're derefing EDI (hence the
   // memory access) and comparing it to EAX. OR EDI/AX, EDI/AL... you
   // get the idea. In any case, since it's an implicit operand we can't
   // play games with changing out MOD/RM bytes. 
   //
   // The approach: assume EDI is live. Push the old value, save 
   // flags, translate the addr, run the SCAS-equivalent, restore flags, 
   // pop the old value. 

   codeGen prepatch(128);
   prepatch.applyTemplate(templ);
   
   if (!initialize(prepatch)) return false;
   if (!checkLiveness(prepatch)) return false;
   if (!setupFrame(prepatch)) return false;
   if (!computeEffectiveAddress(prepatch)) return false;
   if (!preCallSave(prepatch)) return false;
   
   buffer.addPIC(prepatch, tracker(t->bbl()->func()));
   buffer.addPatch(new MemEmulatorPatch(effAddr_, getTranslatorAddr(prepatch), point_),
                   tracker(t->bbl()->func()));
   prepatch.setIndex(0);

   if (!postCallRestore(prepatch)) return false;
   if (!teardownFrame(prepatch)) return false;

   // Okay, effAddr_ now holds the translated EDI. 
   ::emitPush(RealRegister(REGNUM_EDI), prepatch);
   emitMovRegToReg(RealRegister(REGNUM_EDI), RealRegister(effAddr_), prepatch);
   // EDI now holds the translated addr
   prepatch.copy(insn_->ptr(), insn_->size());
   // And we performed the operation. Restore EDI.
   ::emitPop(RealRegister(REGNUM_EDI), prepatch);
   // And clean up
   if (!trailingTeardown(prepatch)) return false;

   return true;
}

bool MemEmulator::generateLODS(const codeGen &gen, const Trace *t, CodeBuffer &buffer) {
   return true;
}

bool MemEmulator::stealEffectiveAddr(codeGen &gen) {
   cerr << "STEALING EFFECTIVE ADDR REGISTER @ " << hex << addr_ << dec << endl;
   // This sucks. Find a register not used by this instruction
   // and push/pop it around the whole mess.
   std::set<RegisterAST::Ptr> regs;
   insn_->getReadSet(regs);
   insn_->getWriteSet(regs);
   std::set<Register> translated;
   for (std::set<RegisterAST::Ptr>::iterator i = regs.begin(); i != regs.end(); ++i) {
      bool whocares;
      translated.insert(convertRegID(*i, whocares));
   }
   for (unsigned candidate = REGNUM_EAX; candidate <= REGNUM_EDI; ++candidate) {
      if (candidate == REGNUM_ESP) continue;
      if (translated.find(candidate) == translated.end()) {
         effAddr_ = candidate;
         break;
      }
   }
   // Okay, so we stole a reg that wasn't used by the instruction
   ::emitPush(RealRegister(effAddr_), gen);
   restoreEffAddr_ = true;
   return true;
}

//////////////////////////////////////////////////////////

MemEmulatorTranslator::Ptr MemEmulatorTranslator::create(Register r) {
  return Ptr(new MemEmulatorTranslator(r));
}

TrackerElement *MemEmulatorTranslator::tracker() const {
  // This is a funny one... 
   EmulatorTracker *e = new EmulatorTracker(0, NULL);
   return e;
}

bool MemEmulatorTranslator::generate(const codeGen &templ,
                                     const Trace *,
                                     CodeBuffer &buffer) {
  DecisionTree dt(reg_);
  codeGen gen;
  gen.applyTemplate(templ);

  dt.generate(gen);

  generateReturn(gen);

  buffer.addPIC(gen, tracker());
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

/*
  origPatches.push_back(generateCompare(gen, gen.addrSpace()->heapBase()));
  instPatches.push_back(generateCompare(gen, gen.addrSpace()->instBase()));
  origPatches.push_back(generateCompare(gen, gen.addrSpace()->dataBase()));
  textPatches.push_back(generateCompare(gen, gen.addrSpace()->textBase()));
*/
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

bool MemEmulatorPatch::apply(codeGen &gen,
                             CodeBuffer *) {
   relocation_cerr << "MemEmulatorPatch::apply @ " << hex << gen.currAddr() << dec << endl;
   relocation_cerr << "\tPush reg " << reg_ << endl;
   registerSpace *aS = registerSpace::actualRegSpace(point, callPreInsn);
   gen.setRegisterSpace(aS);
   assert(!gen.bti());

   ::emitPush(RealRegister(reg_), gen);

   // Step 2: call the translator
   Address src = gen.currAddr() + 5;
   relocation_cerr << "\tCall " << hex << dest_ << ", offset " << dest_ - src << dec << endl;
   assert(dest_);
   emitCallRel32(dest_ - src, gen);

   ::emitMovRegToReg(RealRegister(reg_), RealRegister(REGNUM_EAX), gen);
   ::emitLEA(RealRegister(REGNUM_ESP), RealRegister(Null_Register), 0, 4, RealRegister(REGNUM_ESP), gen);

#if 0
   // Step 1: move the argument into ECX
   if (reg_ != REGNUM_ECX) {
      ::emitMovRegToReg(RealRegister(REGNUM_ECX), 
                        RealRegister(reg_),
                        gen);
   }

   // Step 2: call the translator
   Address src = gen.currAddr() + 5;
   relocation_cerr << "\tCall " << hex << dest_ << ", offset " << dest_ - src << dec << endl;
   assert(dest_);
   emitCallRel32(dest_ - src, gen);

   // Step 3: move the result from EAX into the target reg
   if (reg_ != REGNUM_EAX) {
      ::emitMovRegToReg(RealRegister(reg_), RealRegister(REGNUM_EAX), gen);
   }
#endif
   return true;
}
