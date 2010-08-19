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

using namespace Dyninst;
using namespace Relocation;
using namespace InstructionAPI;

///////////////////////

// Pick values that don't correspond to actual targets. I'm skipping
// 0 because it's used all over the place as a null.
const Address CFAtom::Fallthrough(1);
const Address CFAtom::Taken(2);

bool CFAtom::generate(GenStack &gens) {
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
  if (!isIndirect_ && destMap_.empty()) return true;


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

    bool fallthrough = (destMap_.begin()->first == Fallthrough);
    TargetInt *target = destMap_.begin()->second;
    assert(target);

    if (target->necessary()) {
      if (!generateBranch(gens,
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
      if (!generateCall(gens,
			destMap_[Taken],
			insn_))
	return false;
    }
    else {
      assert(!isCall_);
      relocation_cerr << "  ... generating conditional branch" << endl;
      if (!generateConditionalBranch(gens,
				     destMap_[Taken],
				     insn_))
	return false;
    }

    // Not necessary by design - fallthroughs are always to the next generated
    // We can have calls that don't return and thus don't have funlink edges
    

    if (destMap_.find(Fallthrough) != destMap_.end()) {
      TargetInt *ft = destMap_[Fallthrough];
      if (ft->necessary()) {
	if (!generateBranch(gens, 
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
    bool requireTranslation = false;
    for (DestinationMap::iterator iter = destMap_.begin();
	 iter != destMap_.end(); ++iter) {
      if (iter->first != iter->second->addr()) {
	requireTranslation = true;
	break;
      }
    }
    Register reg; /* = originalRegister... */
    if (requireTranslation) {
      if (!generateAddressTranslator(gens(), reg))
	return false;
    }
    if (isCall_) {
      if (!generateIndirectCall(gens, 
				reg, 
				insn_, 
				addr_)) 
	return false;
      // We may be putting another block in between this
      // one and its fallthrough due to edge instrumentation
      // So if there's the possibility for a return put in
      // a fallthrough branch
      if (destMap_.find(Fallthrough) != destMap_.end()) {
	if (!generateBranch(gens,
			    destMap_[Fallthrough],
			    Instruction::Ptr(),
			    true)) 
	  return false;
      }
    }
    else {
      if (!generateIndirect(gens, reg, insn_))
	return false;
    }
    break;
  }
  default:
    assert(0);
  }
  return true;
}

CFAtom::Ptr CFAtom::create() {
  return Ptr(new CFAtom());
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
  EmulatorTracker *e = new EmulatorTracker(addr_);
  return e;
}

void CFAtom::addDestination(Address index, TargetInt *dest) {
  // Annoying required copy... 
  destMap_[index] = dest;
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

bool CFAtom::generateBranch(GenStack &gens,
			    TargetInt *to,
			    Instruction::Ptr insn,
			    bool) {
  assert(to);

  // We can put in an unconditional branch as an ender for 
  // a block that doesn't have a real branch. So if we don't have
  // an instruction generate a "generic" branch

  // We can see a problem where we want to branch to (effectively) 
  // the next instruction. So if we ever see that (a branch of offset
  // == size) back up the codeGen and shrink us down.

  CFPatch *newPatch = new CFPatch(CFPatch::Jump, insn, to, padded_, addr_);
  gens.addPatch(newPatch);

  return true;
}

bool CFAtom::generateCall(GenStack &gens,
			  TargetInt *to,
			  Instruction::Ptr insn) {
  if (!to) {
    // This can mean an inter-module branch...
    relocation_cerr << "    ... skipping call with no target!" << endl;
    return true;
  }

  CFPatch *newPatch = new CFPatch(CFPatch::Call, insn, to, padded_, addr_);
  gens.addPatch(newPatch);

  return true;
}

bool CFAtom::generateConditionalBranch(GenStack &gens,
				       TargetInt *to,
				       Instruction::Ptr insn) {
  assert(to);

  CFPatch *newPatch = new CFPatch(CFPatch::JCC, insn, to, padded_, addr_);
  gens.addPatch(newPatch);

  return true;
}

bool CFAtom::generateIndirect(GenStack &gens,
				 Register,
			      Instruction::Ptr insn) {
  // Two possibilities here: either copying an indirect jump w/o
  // changes, or turning an indirect call into an indirect jump because
  // we've had the isCall_ flag overridden.

  instruction ugly_insn(insn->ptr());
  ia32_locations loc;
  ia32_memacc memacc[3];
  ia32_condition cond;

  ia32_instruction orig_instr(memacc, &cond, &loc);
  ia32_decode(IA32_FULL_DECODER, (unsigned char *)insn->ptr(), orig_instr);

  char *buffer = (char *)malloc(insn->size());

  // Copy prefixes untouched.
  for (int i = 0; i < loc.num_prefixes; ++i) {
    buffer[i] = ugly_insn.ptr()[i];
  }
  
  // Opcode might get modified;
  // 0xe8 -> 0xe9 (call Jz -> jmp Jz)
  // 0xff xx010xxx -> 0xff xx100xxx (call Ev -> jmp Ev)
  // 0xff xx011xxx -> 0xff xx101xxx (call Mp -> jmp Mp)

  bool fiddle_mod_rm = false;
  for (int i = loc.num_prefixes; i < loc.num_prefixes + (int) loc.opcode_size; ++i) {
    if (ugly_insn.ptr()[i] == 0xe8) {
      buffer[i] = 0xe9;
    }
    else if (ugly_insn.ptr()[i] == 0xff) {
      buffer[i] = 0xff;
      fiddle_mod_rm = true;
    }
    else {
      buffer[i] = ugly_insn.ptr()[i];
    }
  }
  
  for (int i = loc.num_prefixes + (int) loc.opcode_size; 
       i < (int) insn->size(); ++i) {
    buffer[i] = ugly_insn.ptr()[i];
    if ((i == loc.modrm_position) &&
	fiddle_mod_rm) {
      buffer[i] |= 0x20;
      buffer[i] &= ~0x10;
    }
  } 

  // TODO: don't ignore reg...
  // Indirect branches don't use the PC and so are
  // easy - we just copy 'em.
  gens().copy(buffer, insn->size());
  free(buffer);

  if (padded_) {
    gens().registerPostCallPad(addr_);
    //gen.fill(10, codeGen::cgIllegal);
    gens().fill(10, codeGen::cgNOP);
  }

  return true;
}

bool CFAtom::generateIndirectCall(GenStack &gens,
				     Register,
				     Instruction::Ptr insn,
				  Address /*origAddr*/) {
  // Check this to see if it's RIP-relative
  instruction ugly_insn(insn->ptr());
  if (ugly_insn.type() & REL_D_DATA) {
    // We don't know our final address, so use the patching system
    assert(0 && "Unimplemented!");
    // This target better not be NULL...
    CFPatch *newPatch = new CFPatch(CFPatch::Data, insn, NULL, padded_, addr_);
    gens.addPatch(newPatch);
  }
  else {
    gens().copy(insn->ptr(), insn->size());
  }

  if (padded_) {
    gens().registerPostCallPad(addr_);
    //gen.fill(10, codeGen::cgIllegal);
    gens().fill(10, codeGen::cgNOP);
  }

  return true;
}

bool CFAtom::generateAddressTranslator(codeGen &,
					  Register &) {
  // Do nothing...
  return true;
}

std::string CFAtom::format() const {
  stringstream ret;
  ret << "CFAtom(" << std::hex;
  ret << addr_;
  if (isIndirect_) ret << "<ind>";
  if (isConditional_) ret << "<cond>";
  if (isCall_) ret << "<call>";
		     
  for (DestinationMap::const_iterator iter = destMap_.begin();
       iter != destMap_.end();
       ++iter) {
    ret << iter->first << "->" << iter->second->format() << ",";
  }
  ret << std::dec << ")";
  return ret.str();
}


/////////////////////////
// Patching!
/////////////////////////

bool CFPatch::apply(codeGen &gen, int iteration, int shift) {
  relocation_cerr << "\t\t CFPatch::apply, type " << type << " origAddr " << hex << origAddr_ << endl;
  if (orig_insn) {
    instruction ugly_insn(orig_insn->ptr());
    switch(type) {
    case CFPatch::Jump: {
      pcRelJump pcr(target->adjAddr(iteration, shift), ugly_insn);
      pcr.gen = &gen;
      pcr.apply(gen.currAddr());
      relocation_cerr << "\t\t\t Generating CFPatch::Jump from " 
		      << hex << gen.currAddr() << " to " << target->adjAddr(iteration, shift) << dec << endl;
      break;
    }
    case CFPatch::JCC: {
      pcRelJCC pcr(target->adjAddr(iteration, shift), ugly_insn);
      pcr.gen = &gen;
      pcr.apply(gen.currAddr());

      relocation_cerr << "\t\t\t Generating CFPatch::JCC from " 
		      << hex << gen.currAddr() << " to " << target->adjAddr(iteration, shift) << dec
		      << "(" << shift << "), with unmod addr" << hex << target->addr() << dec << endl;
      
      break;
    }
    case CFPatch::Call: {
      pcRelCall pcr(target->adjAddr(iteration, shift), ugly_insn);
      pcr.gen = &gen;
      pcr.apply(gen.currAddr());
      break;
    }
    case CFPatch::Data: {
      pcRelData pcr(target->adjAddr(iteration, shift), ugly_insn);
      pcr.gen = &gen;
      pcr.apply(gen.currAddr());
      break;
    }
    }
  }
  else {
    switch(type) {
    case CFPatch::Jump:
      insnCodeGen::generateBranch(gen, gen.currAddr(), target->adjAddr(iteration, shift));
      break;
    case CFPatch::Call:
      insnCodeGen::generateCall(gen, gen.currAddr(), target->adjAddr(iteration, shift));
      break;
    default:
      assert(0);
    }
  }

  if (postCFPadding_) {
    gen.registerPostCallPad(origAddr_);
    gen.fill(10, codeGen::cgIllegal);
    //gen.fill(10, codeGen::cgNOP);
  }

  return true;
}

bool CFPatch::preapply(codeGen &gen) {
  if (orig_insn) {
    gen.copy(orig_insn->ptr(), orig_insn->size());
  }

  // Hopefully a fallthrough which will be skipped.

  return true;
}
