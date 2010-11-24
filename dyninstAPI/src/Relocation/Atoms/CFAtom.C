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

#include "CFAtom.h"
#include "Atom.h"
#include "Target.h"

#include "instructionAPI/h/Instruction.h"

#include "dyninstAPI/src/debug.h"

#include "../CodeTracker.h"
#include "../CodeBuffer.h"


#include "dyninstAPI/src/BPatch_memoryAccessAdapter.h"
#include "dyninstAPI/h/BPatch_memoryAccess_NP.h"

#include "dyninstAPI/src/MemoryEmulator/memEmulatorAtom.h"
#include "dyninstAPI/src/inst-x86.h"
#include "dyninstAPI/src/addressSpace.h"
using namespace Dyninst;
using namespace Relocation;
using namespace InstructionAPI;

///////////////////////

// Pick values that don't correspond to actual targets. I'm skipping
// 0 because it's used all over the place as a null.
const Address CFAtom::Fallthrough(1);
const Address CFAtom::Taken(2);

bool CFAtom::generate(const codeGen &templ,
                      const Trace *,
                      CodeBuffer &buffer)
{
  // We need to create jumps to wherever our successors are
  // We can assume the addresses returned by our Targets
  // are valid, since we'll fixpoint until those stabilize. 
  //
  // There are the following cases:
  //
  // No explicit control flow/unconditional direct branch:
  //   1) One target
  //   2) Generate a branch unless it's unnecessary
  // Conditional branch:
  //   1) Two targets
  //   2) Use stored instruction to generate correct condition
  //   3) Generate a fallthrough "branch" if necessary
  // Call:
  //   1) Two targets (call and natural successor)
  //   2) As above, except make sure call bit is flipped on
  // Indirect branch:
  //   1) Just go for it... we have no control, really
  
  // First check: if we're not an indirect branch
  // and we have no known successors return immediately.

						  
  if (!isIndirect_ && destMap_.empty()) {
	  // DEFENSIVE MODE: drop in an illegal instruction
	  // so that we're sure to notice this. We may have stopped
	  // early due to a believed garbage parse.
	  codeGen gen(10);
	  gen.fill(10, codeGen::cgIllegal);
	  buffer.addPIC(gen, tracker());
      return true;
	  }


  // TODO: address translation on an indirect branch...

  typedef enum {
    Illegal,
    Single,
    Taken_FT,
    Indirect } Options;
  
  Options opt = Illegal;

  if (isIndirect_) {
    opt = Indirect;
    relocation_cerr << "  generating CFAtom as indirect branch" << endl;
  }
  else if (isConditional_ || isCall_) {
    opt = Taken_FT;
    relocation_cerr << "  generating CFAtom as call or conditional branch" << endl;
  }
  else {
    opt = Single;
    relocation_cerr << "  generating CFAtom as direct branch" << endl;
  }

  switch (opt) {
  case Single: {

    assert(!isIndirect_);
    assert(!isConditional_);
    assert(!isCall_);

    // Check for a taken destination first.
    bool fallthrough = false;
    DestinationMap::iterator iter = destMap_.find(Taken);
    if (iter == destMap_.end()) {
        iter = destMap_.find(Fallthrough);
        fallthrough = true;
    }
    assert(iter != destMap_.end());

    TargetInt *target = iter->second;
    assert(target);

    if (target->necessary()) {
      if (!generateBranch(buffer,
			  target,
			  insn_,
			  fallthrough)) {
	return false;
      }
    }
    else {
      relocation_cerr << "    target reported unnecessary" << endl;
    }
    break;
  }
  case Taken_FT: {
    // This can be either a call (with an implicit fallthrough as shown by
    // the FUNLINK) or a conditional branch.
    if (isCall_) {
      // Well, that kinda explains things
      assert(!isConditional_);
      relocation_cerr << "  ... generating call" << endl;
      if (!generateCall(buffer,
			destMap_[Taken],
			insn_))
	return false;
    }
    else {
      assert(!isCall_);
      relocation_cerr << "  ... generating conditional branch" << endl;
      if (!generateConditionalBranch(buffer,
				     destMap_[Taken],
				     insn_))
	return false;
    }

    // Not necessary by design - fallthroughs are always to the next generated
    // We can have calls that don't return and thus don't have funlink edges
    

    if (destMap_.find(Fallthrough) != destMap_.end()) {
      TargetInt *ft = destMap_[Fallthrough];
      if (ft->necessary()) {
	if (!generateBranch(buffer, 
			    ft,
			    insn_,
			    true)) {
	  return false;
	}
      }
    }
    break;
  }
  case Indirect: {
/*
  for (DestinationMap::iterator iter = destMap_.begin();
  iter != destMap_.end(); ++iter) {
  if (iter->second->type() == TargetInt::TraceTarget) {
  requireTranslation = true;
  break;
  }
  }
*/
    Register reg = Null_Register; /* = originalRegister... */
	// Originally for use in helping with jump tables, I'm taking
	// this for the memory emulation effort. Huzzah!
	if (!generateAddressTranslator(buffer, templ, reg))
          return false;
    if (isCall_) {
       if (!generateIndirectCall(buffer, 
                                 reg, 
                                 insn_, 
                                 addr_)) 
			return false;
        // We may be putting another block in between this
        // one and its fallthrough due to edge instrumentation
        // So if there's the possibility for a return put in
        // a fallthrough branch
        if (destMap_.find(Fallthrough) != destMap_.end()) {
			if (!generateBranch(buffer,
					    destMap_[Fallthrough],
					    Instruction::Ptr(),
					    true)) 
			  return false;
			}
		}
    else {
		if (!generateIndirect(buffer, reg, insn_))
			return false;
    }
    break;
  }
  default:
    assert(0);
  }
  if (postCallPadding_ != 0) {
     if (postCallPadding_ == (unsigned) -1) {
        // We don't know what the callee does to the return addr,
        // so we'll catch it at runtime. 
        // The "10" is arbitrary.
        buffer.addPatch(new PaddingPatch(10, true, block_), addrTracker(addr_ + size()));
     }
     else {
        // Make up for stack tampering
        buffer.addPatch(new PaddingPatch(postCallPadding_, false, block_), addrTracker(addr_ + size()));
     }
  }
  
  return true;
}

CFAtom::Ptr CFAtom::create(int_block *b) {
  return Ptr(new CFAtom(b));
}

CFAtom::~CFAtom() {
  // Delete all Targets in our map
  for (DestinationMap::iterator i = destMap_.begin(); 
       i != destMap_.end(); ++i) {
    delete i->second;
  }
}

TrackerElement *CFAtom::tracker() const {
  assert(addr_ != 1);
  EmulatorTracker *e = new EmulatorTracker(addr_, block());
  return e;
}

TrackerElement *CFAtom::destTracker(TargetInt *dest) const {
   if (dest->origAddr() == 0) {
      cerr << "Error: no original address for target " << dest->format() << endl;
      assert(0);
   }

   int_function *destFunc = NULL;
   switch (dest->type()) {
      case TargetInt::TraceTarget: {
         Target<Trace::Ptr> *targ = static_cast<Target<Trace::Ptr> *>(dest);
         destFunc = targ->t()->func();
         assert(destFunc);
         break;
      }
      case TargetInt::BlockTarget:
         destFunc = (static_cast<Target<int_block *> *>(dest))->t()->func();
         assert(destFunc);
         break;
      default:
         assert(0);
         break;
   }
   EmulatorTracker *e = new EmulatorTracker(dest->origAddr(), destFunc->entryBlock());
   return e;
}

TrackerElement *CFAtom::addrTracker(Address addr) const {
   EmulatorTracker *e = new EmulatorTracker(addr, block());
   return e;
}

void CFAtom::addDestination(Address index, TargetInt *dest) {
  // Annoying required copy... 
  if (!dest) {
      printf("adding bad dest\n");
  }
  destMap_[index] = dest;
}

TargetInt *CFAtom::getDestination(Address dest) const {
    CFAtom::DestinationMap::const_iterator d_iter = destMap_.find(dest);
    if (d_iter != destMap_.end()) {
        return d_iter->second;
    }
    return NULL;
}

void CFAtom::updateInsn(Instruction::Ptr insn) {

  relocation_cerr << "Updating CFAtom off insn " << insn->format() << endl;

  insn_ = insn;

  isConditional_ = isCall_ = isIndirect_ = false;

  // And set type flags based on what the instruction was
  // If it allows fallthrough it must be conditional...
  if (insn->allowsFallThrough()) {
    relocation_cerr << "... allows fallthrough, setting isConditional" << endl;
    isConditional_ = true;
  }
  // Calls show up as fallthrough-capable, which is true
  // (kind of) for parsing but _really_ not what we want
  // to identify conditional branches...
  if (insn->getCategory() == c_CallInsn) {
    relocation_cerr << "... is call, setting isCall and unsetting isConditional" << endl;
    isCall_ = true;
    isConditional_ = false;
  }

  // And here's the annoying bit - we can't directly determine
  // whether something is indirect. Bill suggests getting the 
  // control flow target, binding *something* as the PC, and
  // evaluating it. I think that sucks. 

  // TODO FIXME
  static Expression::Ptr thePC(new RegisterAST(MachRegister::getPC(Arch_x86)));
  static Expression::Ptr thePC64(new RegisterAST(MachRegister::getPC(Arch_x86_64)));

  Expression::Ptr exp = insn->getControlFlowTarget();
  
  // Bind the IP, why not...
  exp->bind(thePC.get(), Result(u32, addr_));
  exp->bind(thePC64.get(), Result(u64, addr_));

  Result res = exp->eval();
  if (!res.defined) {
    relocation_cerr << "... cannot statically resolve, setting isIndirect" << endl;
    isIndirect_ = true;
  }
  // EMXIF ODOT

}

void CFAtom::updateAddr(Address addr) {
  assert(addr != 1);
  addr_ = addr;
}

void CFAtom::updateInfo(CFAtom::Ptr old) {
   // Pull all misc. info out
   // Don't pull insn, as we often want to override
   addr_ = old->addr_;
   // Don't pull destMap...
   block_ = old->block_;
   postCallPadding_ = old->postCallPadding_;

   // Don't copy isCall/isConditional/isIndirect
}

bool CFAtom::generateBranch(CodeBuffer &buffer,
			    TargetInt *to,
			    Instruction::Ptr insn,
			    bool fallthrough) {
  assert(to);
  if (!to->necessary()) return true;

  // We can put in an unconditional branch as an ender for 
  // a block that doesn't have a real branch. So if we don't have
  // an instruction generate a "generic" branch

  // We can see a problem where we want to branch to (effectively) 
  // the next instruction. So if we ever see that (a branch of offset
  // == size) back up the codeGen and shrink us down.

  CFPatch *newPatch = new CFPatch(CFPatch::Jump, insn, to, addr_);
  if (fallthrough) {
     buffer.addPatch(newPatch, destTracker(to));
  }
  else {
     buffer.addPatch(newPatch, tracker());
  }
  
  return true;
}

bool CFAtom::generateCall(CodeBuffer &buffer,
			  TargetInt *to,
			  Instruction::Ptr insn) {
  if (!to) {
    // This can mean an inter-module branch...
    relocation_cerr << "    ... skipping call with no target!" << endl;
    return true;
  }

  CFPatch *newPatch = new CFPatch(CFPatch::Call, insn, to, addr_);
  buffer.addPatch(newPatch, tracker());

  return true;
}

bool CFAtom::generateConditionalBranch(CodeBuffer &buffer,
				       TargetInt *to,
				       Instruction::Ptr insn) {
  assert(to);

  CFPatch *newPatch = new CFPatch(CFPatch::JCC, insn, to, addr_);
  buffer.addPatch(newPatch, tracker());

  return true;
}

bool CFAtom::generateIndirect(CodeBuffer &buffer,
							  Register reg,
							  Instruction::Ptr insn) {
  // Two possibilities here: either copying an indirect jump w/o
  // changes, or turning an indirect call into an indirect jump because
  // we've had the isCall_ flag overridden.

  if (reg != Null_Register) {
	  // Whatever was there doesn't matter.
	  // Only thing we can handle right now is a "we left the destination
	  // at the top of the stack, go get 'er Tiger!"
	  assert(reg == REGNUM_ESP);
	  codeGen gen(1);
	  GET_PTR(insn, gen);
      *insn++ = 0xC3; // RET
      SET_PTR(insn, gen);
	  buffer.addPIC(gen, tracker());
	  return true;
  }
  instruction ugly_insn(insn->ptr());
  ia32_locations loc;
  ia32_memacc memacc[3];
  ia32_condition cond;

  ia32_instruction orig_instr(memacc, &cond, &loc);
  ia32_decode(IA32_FULL_DECODER, (unsigned char *)insn->ptr(), orig_instr);
  const unsigned char *ptr = (const unsigned char *)insn->ptr();

  std::vector<unsigned char> raw (ptr,
                                  ptr + insn->size());

  // Opcode might get modified;
  // 0xe8 -> 0xe9 (call Jz -> jmp Jz)
  // 0xff xx010xxx -> 0xff xx100xxx (call Ev -> jmp Ev)
  // 0xff xx011xxx -> 0xff xx101xxx (call Mp -> jmp Mp)

  bool fiddle_mod_rm = false;
  for (unsigned i = loc.num_prefixes; 
       i < loc.num_prefixes + (unsigned) loc.opcode_size;
       ++i) {
     switch(raw[i]) {
        case 0xE8:
           raw[i] = 0xE9;
           break;
        case 0xFF:
           fiddle_mod_rm = true;
           break;
        default:
           break;
     }
  }

  for (int i = loc.num_prefixes + (int) loc.opcode_size; 
       i < (int) insn->size(); 
       ++i) {
     if ((i == loc.modrm_position) &&
         fiddle_mod_rm) {
        raw[i] |= 0x20;
        raw[i] &= ~0x10;
     }
  } 
  
  // TODO: don't ignore reg...
  // Indirect branches don't use the PC and so are
  // easy - we just copy 'em.
  buffer.addPIC(raw, tracker());

  return true;
}

bool CFAtom::generateIndirectCall(CodeBuffer &buffer,
                                  Register reg,
                                  Instruction::Ptr insn,
				  Address /*origAddr*/) 
{
	// I'm pretty sure that anything that can get translated will be
	// turned into a push/jump combo already. 
	assert(reg == Null_Register);
  // Check this to see if it's RIP-relative
  instruction ugly_insn(insn->ptr());
  if (ugly_insn.type() & REL_D_DATA) {
    // We don't know our final address, so use the patching system
    assert(0 && "Unimplemented!");
    // This target better not be NULL...
    CFPatch *newPatch = new CFPatch(CFPatch::Data, insn, NULL, addr_);
    buffer.addPatch(newPatch, tracker());
  }
  else {
     buffer.addPIC(insn->ptr(), insn->size(), tracker());
  }

  return true;
}

bool CFAtom::generateAddressTranslator(CodeBuffer &buffer,
									   const codeGen &templ,
									   Register &reg) 
{
	if (insn_->getOperation().getID() == e_ret_near ||
		insn_->getOperation().getID() == e_ret_far) {
		// Oops!
		return true;
	}
	if (!insn_->readsMemory()) {
		return true;
		}
	if (addr_ == 0x40e4ca)	{
		cerr << "Got it!" << endl;
	}

BPatch_memoryAccessAdapter converter;
	BPatch_memoryAccess *acc = converter.convert(insn_, addr_, false);
	if (!acc) {
		reg = Null_Register;
		return true;
		}

codeGen patch(128);
	patch.applyTemplate(templ);

	// step 1: create space on the stack. 
	::emitPush(RealRegister(REGNUM_EAX), patch);

    // step 2: save registers that will be affected by the call
    ::emitPush(RealRegister(REGNUM_ECX), patch);
    ::emitPush(RealRegister(REGNUM_EDX), patch);
    ::emitPush(RealRegister(REGNUM_EAX), patch);

    // Step 3: LEA this sucker into ECX.
	const BPatch_addrSpec_NP *start = acc->getStartAddr(0);
	emitASload(start, REGNUM_ECX, patch, true);
    
    // Step 4: save flags post-LEA
	emitSimpleInsn(0x9f, patch);
	emitSaveO(patch);
	::emitPush(RealRegister(REGNUM_EAX), patch);

	// This might look a lot like a memEmulatorAtom. That's, well, because it
	// is. 
	buffer.addPIC(patch, tracker());

	// Where are we going?
    int_function *func = templ.addrSpace()->findOnlyOneFunction("RTtranslateMemory");
    // FIXME for static rewriting; this is a dynamic-only hack for proof of concept.
	assert(func);

	// Now we start stealing from memEmulatorAtom. We need to call our translation function,
	// which means a non-PIC patch to the CodeBuffer. I don't feel like rewriting everything,
	// so there we go.
	buffer.addPatch(new MemEmulatorPatch(REGNUM_ECX, addr_, func->getAddress()),
					tracker());
	patch.setIndex(0);

	// Restore flags
	::emitPop(RealRegister(REGNUM_EAX), patch);
    emitRestoreO(patch);
    emitSimpleInsn(0x9E, patch);
    ::emitPop(RealRegister(REGNUM_EAX), patch);
    ::emitPop(RealRegister(REGNUM_EDX), patch);

	// ECX now holds the pointer to the destination...
    // Dereference
	::emitMovRMToReg(RealRegister(REGNUM_ECX),
                     RealRegister(REGNUM_ECX),
                     0,
                     patch);

	// ECX now holds the _actual_ destination, so move it on to the stack. 
    // We've got ECX saved
	::emitMovRegToRM(RealRegister(REGNUM_ESP),
                     1*4, 
                     RealRegister(REGNUM_ECX),
                     patch);
	::emitPop(RealRegister(REGNUM_ECX), patch);
	// And tell our people to use the top of the stack
	// for their work.
	// TODO: trust liveness and leave this in a register. 

	buffer.addPIC(patch, tracker());
	reg = REGNUM_ESP;
	return true;
}

std::string CFAtom::format() const {
  stringstream ret;
  ret << "CFAtom(" << std::hex;
  ret << addr_ << ",";
  if (isIndirect_) ret << "<ind>";
  if (isConditional_) ret << "<cond>";
  if (isCall_) ret << "<call>";
		     
  for (DestinationMap::const_iterator iter = destMap_.begin();
       iter != destMap_.end();
       ++iter) {
    switch (iter->first) {
    case Fallthrough:
      ret << "FT";
      break;
    case Taken:
      ret << "T";
      break;
    default:
      ret << iter->first;
      break;
    }
    ret << "->" << iter->second->format() << ",";
  }
  ret << std::dec << ")";
  return ret.str();
}

unsigned CFAtom::size() const
{ 
    if (insn_ != NULL) 
        return insn_->size(); 
    return 0;
}

/////////////////////////
// Patching!
/////////////////////////

bool CFPatch::apply(codeGen &gen, CodeBuffer *buf) {
   int targetLabel = target->label(buf);

   relocation_cerr << "\t\t CFPatch::apply, type " << type << ", origAddr " << hex << origAddr_ 
                   << ", and label " << dec << targetLabel << endl;
   if (orig_insn) {
      instruction ugly_insn(orig_insn->ptr());
      switch(type) {
         case CFPatch::Jump: {
            pcRelJump pcr(buf->predictedAddr(targetLabel), ugly_insn);
            pcr.gen = &gen;
            pcr.apply(gen.currAddr());
            relocation_cerr << "\t\t\t Generating CFPatch::Jump from " 
                            << hex << gen.currAddr() << " to " << buf->predictedAddr(targetLabel) << dec << endl;
            break;
         }
         case CFPatch::JCC: {
            pcRelJCC pcr(buf->predictedAddr(targetLabel), ugly_insn);
            pcr.gen = &gen;
            pcr.apply(gen.currAddr());
            
            relocation_cerr << "\t\t\t Generating CFPatch::JCC from " 
                            << hex << gen.currAddr() << " to " << buf->predictedAddr(targetLabel) << dec << endl;            
            break;
         }
         case CFPatch::Call: {
            pcRelCall pcr(buf->predictedAddr(targetLabel), ugly_insn);
            pcr.gen = &gen;
            pcr.apply(gen.currAddr());
            break;
         }
         case CFPatch::Data: {
            pcRelData pcr(buf->predictedAddr(targetLabel), ugly_insn);
            pcr.gen = &gen;
            pcr.apply(gen.currAddr());
            break;
         }
      }
   }
   else {
      switch(type) {
         case CFPatch::Jump:
            insnCodeGen::generateBranch(gen, gen.currAddr(), buf->predictedAddr(targetLabel));
            break;
         case CFPatch::Call:
            insnCodeGen::generateCall(gen, gen.currAddr(), buf->predictedAddr(targetLabel));
            break;
         default:
            assert(0);
      }
   }
   
   return true;
}

unsigned CFPatch::estimate(codeGen &) {
  if (orig_insn) {
     return orig_insn->size();
  }
  return 0;
}

bool PaddingPatch::apply(codeGen &gen, CodeBuffer *) {
   if (registerDefensive_) {
      assert(block_);
      gen.registerDefensivePad(block_, gen.currAddr(), 10);
   }
   if ( 0 == (size_ % 2) ) {
       gen.fill(size_, codeGen::cgIllegal);
   } else {
       gen.fill(size_, codeGen::cgTrap);
   }
   //gen.fill(10, codeGen::cgNOP);
   return true;
}

unsigned PaddingPatch::estimate(codeGen &) {
   return size_;;
}

