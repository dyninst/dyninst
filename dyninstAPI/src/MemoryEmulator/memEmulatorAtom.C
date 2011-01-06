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

#include "boost/tuple/tuple.hpp"
#include "memEmulator.h"
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

TrackerElement *MemEmulator::tracker(int_block *b) const {
   EmulatorTracker *e = new EmulatorTracker(addr_, b);
  return e;
}

bool MemEmulator::generate(const codeGen &templ,
                           const Trace *t,
                           CodeBuffer &buffer) {
   if (generateViaOverride(templ, t, buffer))
      return true;
   if (generateViaModRM(templ, t, buffer))
      return true;

   cerr << "Error: failed to emulate memory operation @ " << hex << addr() << dec << ", " << insn()->format() << endl;
   unsigned char *tmp = (unsigned char *)insn()->ptr();
   cerr << hex << "\t raw: ";
   for (unsigned i = 0; i < insn()->size(); ++i) {
	   cerr << tmp[i];
   }
   cerr << dec << endl;
   assert(0);
   return false;
}

bool MemEmulator::generateViaOverride(const codeGen &templ,
                                      const Trace *t,
                                      CodeBuffer &buffer) 
{
    // Watch for a1/a2/a3 moves 
    unsigned char *buf = (unsigned char *)insn_->ptr();
    if ((unsigned char) 0xa0 <= buf[0] &&
        buf[0] <= (unsigned char) 0xa3) {
			if (!generateEAXMove(buf[0], templ, t, buffer)) 
				assert(0);
			return true;
    }
                                          
   const InstructionAPI::Operation &op = insn_->getOperation();
   switch(op.getID()) {
      case e_scasb:
      case e_scasd:
      case e_scasw:
      case e_lodsb:
      case e_lodsd:
      case e_lodsw:
      case e_stosb:
      case e_stosd:
      case e_stosw:
      case e_movsb:
      case e_movsd:
      case e_movsw:
      case e_cmpsb:
      case e_cmpsd:
      case e_cmpsw:
      case e_insb:
      case e_insd:
      case e_insw:
      case e_outsb:
      case e_outsd:
      case e_outsw:
         if (!generateImplicit(templ, t, buffer)) {
			 assert(0);
		 }
		 return true;
         break;
      default:
         break;
   }
   return false;
}

bool MemEmulator::generateEAXMove(int opcode, 
                                  const codeGen &templ,
                                  const Trace *t,
                                  CodeBuffer &buffer) 
{
    // mov [offset], eax
    // We hates them.
    codeGen patch(128);
    patch.applyTemplate(templ);

    Address origTarget;
    switch(opcode) {
    case 0xa0:
    case 0xa1: {
        // read from memory
        std::set<Expression::Ptr> reads;
        insn_->getMemoryReadOperands(reads);
        assert(reads.size() == 1);
        Result res = (*(reads.begin()))->eval();
        assert(res.defined);
        origTarget = res.convert<Address>();
        break;
    }
    case 0xa2:
    case 0xa3: {
        // write
        std::set<Expression::Ptr> writes;
        insn_->getMemoryWriteOperands(writes);
        assert(writes.size() == 1);
        Result res = (*(writes.begin()))->eval();
        assert(res.defined);
        origTarget = res.convert<Address>();
        break;
    }
    default:
        assert(0);
        break;
    }
    // Map it to the new location
    bool valid; Address target;
    boost::tie(valid, target) = patch.addrSpace()->getMemEm()->translate(origTarget);
    if (!valid) target = origTarget;
    //cerr << "Handling mov EAX, [offset]: opcode " << hex << opcode << ", orig dest " << origTarget << " and translated " << target << dec << endl;
    // And emit the insn
    assert(insn_->size() == 5);
    GET_PTR(buf, patch);
    *buf = (char) opcode; buf++;
    int *tmp = (int *)buf;
    *tmp = target; tmp++;
    buf = (codeBuf_t *)tmp;
    SET_PTR(buf, patch);
    buffer.addPIC(patch, tracker(t->bbl()));
    return true;
}

bool GLOBAL_DISASSEMBLY = false;
bool MemEmulator::generateViaModRM(const codeGen &templ,
                                   const Trace *t, 
                                   CodeBuffer &buffer) {
   // We need a BPatch_something to do the memory handling. If that's 
   // not present, assume we don't need to emulate this piece of
   // memory.
   if (!templ.addrSpace()->up_ptr()) {
      buffer.addPIC(insn_->ptr(), insn_->size(), tracker(t->bbl()));
      return true;
   }

   codeGen prepatch(128);
   prepatch.applyTemplate(templ);

   bool debug = false;

   if (addr_ >= 0x9a18b0 &&
	   addr_ <= 0x9a1973) debug = true;
   if (addr_ >= 0x9a1f85 &&
	   addr_ <= 0x9a1f98) debug = true;

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

  if (debug) prepatch.fill(1, codeGen::cgTrap);

  if (!initialize(prepatch)) {
     relocation_cerr << "\tInitialize failed, ret false" << endl;
     return false;
  }

  if (!checkLiveness(prepatch)) {
     relocation_cerr << "\tFlag check failed, ret false" << endl;
     return false;
  }
  if (!setupFrame(false, prepatch)) {
     cerr << " FAILED TO ALLOC REGISTERS for insn @ " << hex << addr() << dec << endl;
     buffer.addPIC(insn_->ptr(), insn_->size(), tracker(t->bbl()));
     return false;
  }
    
  if (!computeEffectiveAddress(prepatch)) {
     relocation_cerr << "\tFailed to compute eff. addr. of memory operation!" << endl;
    return false;
  }

  // push/pop time!
  if (!preCallSave(prepatch))
     return false;
  buffer.addPIC(prepatch, tracker(t->bbl()));

  buffer.addPatch(new MemEmulatorPatch(effAddr_, addr_, getTranslatorAddr(prepatch, false)), tracker(t->bbl()));
  
  prepatch.setIndex(0);
  if (!postCallRestore(prepatch))
     return false;

  if (!teardownFrame(prepatch))
     return false;

  if (!generateOrigAccess(prepatch)) {
	  return false;
  }

  if (!trailingTeardown(prepatch)) {
     return false;
  }

  if (debug) prepatch.fill(1, codeGen::cgTrap);
  buffer.addPIC(prepatch, tracker(t->bbl()));

  return true;
}

bool MemEmulator::initialize(codeGen &gen) {
  effAddr_ = Null_Register;
  effAddr2_ = Null_Register;
  saveFlags_ = false;
  saveRAX_ = false;

  // This is copied from ast.C
  gen.setPoint(point_);
  registerSpace *rs = registerSpace::actualRegSpace(point_, callPreInsn);
  gen.setRegisterSpace(rs);

  stackShift_ = 0;
  externalSaved_.clear();

  return true;
}

bool MemEmulator::checkLiveness(codeGen &gen) {
   if ((*(gen.rs()))[IA32_FLAG_VIRTUAL_REGISTER]->liveState == registerSlot::live) {
      saveFlags_ = true;
      //cerr << "Flags are live; RAX is " << (saveRAX_ ? "live" : "dead") << endl;
   }
  
   // We need to use RAX to save flaggage
   if ((*(gen.rs()))[REGNUM_RAX]->liveState == registerSlot::live) {
      saveRAX_ = true;
   }

  return true;
}


bool MemEmulator::setupFrame(bool needTwo, codeGen &gen) {
   // If we're emulating a push, we need to create a stack
   // slot where this value is going _before_ we do anything
   // else. 
   if (insn_->getOperation().getID() == e_push) {
      // Value doesn't matter
      ::emitPush(RealRegister(REGNUM_EAX), gen);
	  stackShift_ -= 4;
   }

   // Goals:
   // To free a register for the modified effective address
   // To save flags (if live)
   
   // Ensure that the RS doesn't give us EAX, 
   // since we _really_ need to use it for flag saves
   gen.rs()->allocateSpecificRegister(gen, REGNUM_EAX, true);
   
//   effAddr_ = gen.rs()->allocateRegister(gen, false, true);
   if (true || effAddr_ == Null_Register) {
      if (!stealEffectiveAddr(effAddr_, gen)) {
         return false;
      }
   }      
   if (needTwo) {
//      effAddr2_ = gen.rs()->allocateRegister(gen, false, true);
      if (true || effAddr2_ == Null_Register) {
         if (!stealEffectiveAddr(effAddr2_, gen)) {
            return false;
         }
      }
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
  
  // If we use RAX for the effective address calculation, we need to restore it, since
  // we just st0mped the flags. 

  emitASload(start, effAddr_, stackShift_, gen, true);

  return true;
}

bool MemEmulator::teardownFrame(codeGen &gen) {
   // Reverse the order of operations performed in setupFrame
   // 1) Restore flags if live
   // 2) Restore RAX if saveRAX
   // 3) DO NOT restore the effAddr if we stole it; that happens
   //    after the memory access

   return true;
}
   
bool MemEmulator::trailingTeardown(codeGen &gen) {
   while (!externalSaved_.empty()) {
      Register pop = externalSaved_.front(); externalSaved_.pop_front();
      ::emitPop(RealRegister(pop), gen);
  stackShift_ += 4;
   }
   return true;
}

bool MemEmulator::saveFlags(codeGen &gen) {
  if (saveFlags_ == false) return true;
  
  // If others are live, emit an SAHF
  emitSimpleInsn(0x9f, gen);
  
  emitSaveO(gen);

  // Get 'em out of here
  ::emitPush(RealRegister(REGNUM_EAX), gen);
  stackShift_ -= 4;

  return true;
}

bool MemEmulator::restoreFlags(codeGen &gen) {
  if (saveFlags_ == false) return true;

  ::emitPop(RealRegister(REGNUM_EAX), gen);
  stackShift_ += 4;

  emitRestoreO(gen);
  emitSimpleInsn(0x9E, gen);
  
  return true;
}


bool MemEmulator::generateOrigAccess(codeGen &gen) {
  // Okay, theoretically effAddr_ holds the memory address.
  // Should do a compare here; for now, just drop out the instruction
  // and see what happens.


   if (insn_->getOperation().getID() == e_push) {
      return emulatePush(gen);
   }
   if (insn_->getOperation().getID() == e_pop) {
      return emulatePop(gen);
   }
   if (usesESP()) {
	   return emulateESPUse(gen);
   }

   return emulateCommon(gen);
}

bool MemEmulator::usesESP() {
	std::set<InstructionAPI::RegisterAST::Ptr> regs;
	insn_->getReadSet(regs);
    std::set<Register> translated;
   for (std::set<RegisterAST::Ptr>::iterator i = regs.begin(); i != regs.end(); ++i) {
      bool whocares;
      translated.insert(convertRegID(*i, whocares));
   }
	bool usesESP = false;
	for (std::set<Register>::iterator iter = translated.begin(); 
		iter != translated.end(); ++iter) {
			if (*iter == REGNUM_ESP) {
				usesESP = true;
				break;
				}
		}

	return usesESP;
	}

bool MemEmulator::emulateESPUse(codeGen &gen) {
	// The only tricky bit here is that we've changed
	// ESP, and so we can't just directly stash it. 
	// Instead, we need to fake it (again).
	// So we LEA esp up to where it was, run the insn,
	// and LEA it back down.
	if (externalSaved_.empty()) {
		// ESP is where it should be
		return emulateCommon(gen);
	}
	::emitLEA(RealRegister(REGNUM_ESP),
			RealRegister(Null_Register),
			0,
			4*externalSaved_.size(),
			RealRegister(REGNUM_ESP), gen);
	if (!emulateCommon(gen)) return false;
	::emitLEA(RealRegister(REGNUM_ESP),
			RealRegister(Null_Register),
			0,
			-4*(externalSaved_.size()),
			RealRegister(REGNUM_ESP), gen);
	return true;
	}

bool MemEmulator::emulateCommon(codeGen &gen) {
  instruction ugly_insn(insn_->ptr());
  if (addr_ == 0x9a196d) {
	  int i = 3;
  }
  if (!insnCodeGen::generateMem(gen,
				ugly_insn,
				0, // ignored
				0, // ignored
				effAddr_,
				Null_Register))
    return false;
  return true;
}

bool MemEmulator::emulatePush(codeGen &gen) {
   // A lot like emulatePop, except instead of
   // slurping an extra word off the stack we've
   // got one sitting there waiting to be filled.
   Register toUse = REGNUM_EAX;
   if (effAddr_ == REGNUM_EAX) {
      toUse = REGNUM_EBX;
   }

   // We have to push, since we _know_ we have no free
   // registers
   ::emitPush(RealRegister(toUse), gen);
   stackShift_ -= 4;

   // Pull from memory into our spare register
   ::emitMovRMToReg(RealRegister(toUse),
                    RealRegister(effAddr_),
                    0,
                    gen);
   
   // Push (move) it onto the stack
   ::emitMovRegToRM(RealRegister(REGNUM_ESP),
                    4 + 4*externalSaved_.size(), 
                    RealRegister(toUse),
                    gen);
   // Restore toUse
   ::emitPop(RealRegister(toUse), gen);
   stackShift_ += 4;
   // Restore the externalSaved stack
   if (!trailingTeardown(gen)) return false;

   return true;

}

bool MemEmulator::emulatePop(codeGen &gen) {
   // What we're starting with: pop [R1]
   // And the stack looks like:
   // <external saved stack>
   // <val>
   //
   // So we need to get at val.
   if (externalSaved_.empty()) {
      // Easy!
      return emulateCommon(gen);
   }

   // So the stack looks like so:
   // 
   // <external saved n>
   // ...
   // <external saved 1> 
   // <val>
   // 
   // Save another register to use as a temporary. 
   Register toUse = REGNUM_EAX;
   if (effAddr_ == REGNUM_EAX) {
      toUse = REGNUM_EBX;
   }
   // We have to push, since we _know_ we have no free
   // registers
   ::emitPush(RealRegister(toUse), gen);
   stackShift_ -= 4;
   // Load the value off the stack. We want an offset of
   // 4 + 4*(sizeof externalSaved)
   ::emitMovRMToReg(RealRegister(toUse),
                    RealRegister(REGNUM_ESP),
                    4 + 4*externalSaved_.size(),
                    gen);
   // Save it to wherever it's going
   ::emitMovRegToRM(RealRegister(effAddr_),
                    0, 
                    RealRegister(toUse),
                    gen);
   // Restore toUse
   ::emitPop(RealRegister(toUse), gen);
  stackShift_ += 4;
   // Restore the externalSaved stack
   if (!trailingTeardown(gen)) return false;
   // And now LEA to clear the dead value
   ::emitLEA(RealRegister(REGNUM_ESP), RealRegister(Null_Register), 0, 4, RealRegister(REGNUM_ESP), gen);
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
   if ((reg->encoding() == effAddr_) ||
       (reg->encoding() == effAddr2_)) {
      // Don't save it, as we're overwriting it anyway.
      return true;
   }

   if (true || reg->liveState == registerSlot::live) {
      ::emitPush(RealRegister(reg->encoding()), gen);
	  stackShift_ -= 4;
	  reg->liveState = registerSlot::spilled;
   }
   return true;
}

bool MemEmulator::popRegIfSaved(registerSlot *reg, codeGen &gen) {
   if (reg->liveState == registerSlot::spilled) {
      ::emitPop(RealRegister(reg->encoding()), gen);
  stackShift_ += 4;
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
   
   if (saveFlags_) {
      if (!saveFlags(gen)) return false;
   }

   return true;
}

bool MemEmulator::postCallRestore(codeGen &gen) {
   registerSlot *eax = (*(gen.rs()))[REGNUM_EAX];
   registerSlot *ecx = (*(gen.rs()))[REGNUM_ECX];
   registerSlot *edx = (*(gen.rs()))[REGNUM_EDX];

   if (saveFlags_) {
      if (!restoreFlags(gen)) return false;
   }
   
   popRegIfSaved(edx, gen);
   popRegIfSaved(ecx, gen);
   popRegIfSaved(eax, gen);

   return true;
}

bool MemEmulator::emitCallToTranslator(CodeBuffer &) {
   return true;
}
   
Address MemEmulator::getTranslatorAddr(codeGen &gen, bool wantShift) {
   if (wantShift) {
      // Function lookup time
      int_function *func = gen.addrSpace()->findOnlyOneFunction("RTtranslateMemoryShift");
      // FIXME for static rewriting; this is a dynamic-only hack for proof of concept.
      if (!func) return 0;
      // assert(func);
      return func->getAddress();
   }
   else {
      // Function lookup time
      int_function *func = gen.addrSpace()->findOnlyOneFunction("RTtranslateMemory");
      // FIXME for static rewriting; this is a dynamic-only hack for proof of concept.
      if (!func) return 0;
      // assert(func);
      return func->getAddress();
   }
}

bool MemEmulator::generateImplicit(const codeGen &templ, const Trace *t, CodeBuffer &buffer) {


   codeGen prepatch(128);
   prepatch.applyTemplate(templ);

   // This is an implicit use of ESI, EDI, or both. The both? Sucks. 
bool debug = false;
if (debug) {
	prepatch.fill(1, codeGen::cgTrap);
}

   bool usesEDI = false;
   bool usesESI = false;
   bool usesTwo = false;
   
   boost::tie(usesEDI, usesESI) = getImplicitRegs(prepatch);
   if (usesEDI && usesESI) usesTwo = true;

   // We need to calculate one shift value for each implict reg that we use. 


   // This is an implicit use of... EDI. We're derefing EDI (hence the
   // memory access) and comparing it to EAX. OR EDI/AX, EDI/AL... you
   // get the idea. In any case, since it's an implicit operand we can't
   // play games with changing out MOD/RM bytes. 
   //
   // The approach: assume EDI is live. Push the old value, save 
   // flags, translate the addr, run the SCAS-equivalent, restore flags, 
   // pop the old value. 

   
   if (!initialize(prepatch)) return false;
   if (!checkLiveness(prepatch)) return false;
   if (!setupFrame(usesTwo, prepatch)) return false;

   // Move EDI/ESI into our effective address temporaries
   if (usesEDI) {
	   ::emitMovRegToReg(RealRegister(effAddr_), RealRegister(REGNUM_EDI), prepatch);
   }
   if (usesESI) {
	   ::emitMovRegToReg(RealRegister((usesEDI ? effAddr2_ : effAddr_)), RealRegister(REGNUM_ESI), prepatch);
   }

   if (!preCallSave(prepatch)) return false;

   if (usesTwo) {
       ::emitPush(RealRegister(effAddr2_), prepatch);
	   stackShift_ -= 4;
   }
   buffer.addPIC(prepatch, tracker(t->bbl()));

   buffer.addPatch(new MemEmulatorPatch(effAddr_, addr_, getTranslatorAddr(prepatch, true)),
                   tracker(t->bbl()));

       prepatch.setIndex(0);
   if (usesTwo) {
      ::emitPop(RealRegister(effAddr2_), prepatch);
  stackShift_ += 4;
      ::emitPush(RealRegister(effAddr_), prepatch);
	  stackShift_ -= 4;
      buffer.addPIC(prepatch, tracker(t->bbl()));
      buffer.addPatch(new MemEmulatorPatch(effAddr2_, addr_, getTranslatorAddr(prepatch, true)),
                   tracker(t->bbl()));
      prepatch.setIndex(0);
      ::emitPop(RealRegister(effAddr_), prepatch);
  stackShift_ += 4;
   }

   if (!postCallRestore(prepatch)) return false;
   if (!teardownFrame(prepatch)) return false;

   // Okay, effAddr_ now holds the _shift value_ for the first operand,
   // and effAddr2_ for the second (if necessary)

   if (usesEDI) {
      ::emitLEA(RealRegister(REGNUM_EDI),
                RealRegister(effAddr_),
                0, 0, 
                RealRegister(REGNUM_EDI), prepatch);
   }
   if (usesESI) {
      ::emitLEA(RealRegister(REGNUM_ESI),
                RealRegister((usesTwo ? effAddr2_ : effAddr_)),
                0, 0, 
                RealRegister(REGNUM_ESI), prepatch);
   }

   // We've translated, so execute the instruction
   prepatch.copy(insn_->ptr(), insn_->size());

   // And we performed the operation. Restore EDI.
   // But it might have been changed by the operation, 
   // so instead subtract the shift
   ::emitPush(RealRegister(REGNUM_EAX), prepatch);
   	   stackShift_ -= 4;

   if (saveFlags_) {
      if (!saveFlags(prepatch)) return false;
   }

   if (usesEDI) {
      ::emitSubRegReg(RealRegister(REGNUM_EDI),
                      RealRegister(effAddr_),
                      prepatch);
   }
   if (usesESI) {
      ::emitSubRegReg(RealRegister(REGNUM_ESI),
                      RealRegister((usesTwo ? effAddr2_ : effAddr_)),
                      prepatch);
   }

   if (saveFlags_) {
      if (!restoreFlags(prepatch)) return false;
   }

   ::emitPop(RealRegister(REGNUM_EAX), prepatch);
     stackShift_ += 4;

   // And clean up
   if (!trailingTeardown(prepatch)) return false;
#if 0
   if (debug) {
	prepatch.fill(1, codeGen::cgTrap);
}
#endif
   buffer.addPIC(prepatch, tracker(t->bbl()));
   
   return true;
}

bool MemEmulator::stealEffectiveAddr(Register &ret, codeGen &gen) {
   //cerr << "STEALING EFFECTIVE ADDR REGISTER @ " << hex << addr_ << dec << endl;
   // This sucks. Find a register not used by this instruction
   // and push/pop it around the whole mess.
   std::set<RegisterAST::Ptr> regs;
   insn_->getReadSet(regs);
   insn_->getWriteSet(regs);
   std::set<Register> translated;
   translated.insert(effAddr_);
   for (std::set<RegisterAST::Ptr>::iterator i = regs.begin(); i != regs.end(); ++i) {
      bool whocares;
      translated.insert(convertRegID(*i, whocares));
   }
   unsigned candidate;
   // Don't use EAX since we require it for flag saves
   for (candidate = REGNUM_ECX; candidate <= REGNUM_EDI; ++candidate) {
      if (candidate == REGNUM_ESP) continue;
      if (translated.find(candidate) == translated.end()) {
         ret = candidate;
         break;
      }
   }
   if (candidate == Null_Register) return false;

   // Okay, so we stole a reg that wasn't used by the instruction
   ::emitPush(RealRegister(ret), gen);
	   stackShift_ -= 4;
   externalSaved_.push_front(ret);
   return true;
}

std::pair<bool, bool> MemEmulator::getImplicitRegs(codeGen &) {
   // Could go through IAPI, but I'm laaaazy
   const InstructionAPI::Operation &op = insn_->getOperation();

   // EDI, ESI (EAX is also possible, but we save it regardless) 

   switch(op.getID()) {
      case e_scasb:
      case e_scasd:
      case e_scasw:
      case e_insb:
      case e_insd:
      case e_insw:
      case e_stosb:
      case e_stosd:
      case e_stosw:
         return std::make_pair(true, false);
         break;
      case e_lodsb:
      case e_lodsd:
      case e_lodsw:
      case e_outsb:
      case e_outsd:
      case e_outsw:
         return std::make_pair(false, true);
         break;
      case e_movsb:
      case e_movsd:
      case e_movsw:
      case e_cmpsb:
      case e_cmpsw:
      case e_cmpsd:
         return std::make_pair(true, true);
         break;
      default:
         assert(0);
         return std::make_pair(false, false);
   }
   assert(0);
   return std::make_pair(false, false);
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

std::set<Address> suicideAddrs;

bool MemEmulatorPatch::apply(codeGen &gen,
                             CodeBuffer *) {
   relocation_cerr << "MemEmulatorPatch::apply @ " << hex << gen.currAddr() << dec << endl;
   relocation_cerr << "\tPush reg " << reg_ << endl;
   assert(!gen.bti());

   // Two debugging assists
   ::emitPushImm(gen.currAddr(), gen);
   ::emitPushImm(orig_, gen);
	// And our argument
   ::emitPush(RealRegister(reg_), gen);

   // Step 2: call the translator
   Address src = gen.currAddr() + 5;
   relocation_cerr << "\tCall " << hex << dest_ << ", offset " << dest_ - src << dec << endl;
   assert(dest_);
   emitCallRel32(dest_ - src, gen);
   if (reg_ != REGNUM_EAX) {
	   ::emitMovRegToReg(RealRegister(reg_), RealRegister(REGNUM_EAX), gen);
	   }
   ::emitLEA(RealRegister(REGNUM_ESP), RealRegister(Null_Register), 0, 12, RealRegister(REGNUM_ESP), gen);

   return true;
}
