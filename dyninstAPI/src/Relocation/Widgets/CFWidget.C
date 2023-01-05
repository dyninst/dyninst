/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
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

#include "CFWidget.h"
#include "Widget.h"
#include "../CFG/RelocTarget.h"

#include "instructionAPI/h/Instruction.h"
#include "dyninstAPI/src/BPatch_memoryAccessAdapter.h"
#include "dyninstAPI/src/emitter.h"
#include "dyninstAPI/src/inst-x86.h"
#include "dyninstAPI/src/debug.h"

#include "dyninstAPI/src/debug.h"

#include "../CodeTracker.h"
#include "../CodeBuffer.h"

using namespace Dyninst;
using namespace Relocation;
using namespace InstructionAPI;

///////////////////////

// Pick values that don't correspond to actual targets. I'm skipping
// 0 because it's used all over the place as a null.
const Address CFWidget::Fallthrough(1);
const Address CFWidget::Taken(2);

// Case 1: an empty trace ender for traces that do not
// end in a CF-category instruction
CFWidget::Ptr CFWidget::create(Address a) {
   CFWidget::Ptr ptr = Ptr(new CFWidget(a));
   return ptr;
}

// Case 2: wrap a CF-category instruction
CFWidget::Ptr CFWidget::create(Widget::Ptr atom) {
   CFWidget::Ptr ptr = Ptr(new CFWidget(atom->insn(), atom->addr()));
   return ptr;
}

CFWidget::CFWidget(InstructionAPI::Instruction insn, Address addr)  :
   isCall_(false), 
   isConditional_(false), 
   isIndirect_(false),
   gap_(0),
   insn_(insn),
   addr_(addr),
   origTarget_(0)
{
   
   // HACK to be sure things are parsed...
   insn.format();
   for (Instruction::cftConstIter iter = insn.cft_begin(); iter != insn.cft_end(); ++iter) {
      if (iter->isCall) isCall_ = true;
      if (iter->isIndirect) {
          isIndirect_ = true;
      }
      if (iter->isConditional) isConditional_ = true;
   }

#if 0
   // Old way
   if (insn->getCategory() == c_CallInsn) {
      // Calls have a fallthrough but are not conditional.
      // TODO: conditional calls work how?
      isCall_ = true;
   } else if (insn->allowsFallThrough()) {
      isConditional_ = true;
   }
#endif

   // This whole next section is obsolete, but IAPI's CFT interface doesn't say
   // what a "return" is (aka, they don't include "indirect"). So I'm using it
   // so that things work. 



   Expression::Ptr thePC(new RegisterAST(MachRegister::getPC(insn_.getArch())));

   Expression::Ptr exp = insn_.getControlFlowTarget();

   if(!exp) {
      isIndirect_ = true;
      return;
   }

   exp->bind(thePC.get(), Result(u64, addr_));
   //exp->bind(thePCFixme.get(), Result(u64, addr_));
   Result res = exp->eval();
   if (!res.defined) {
      if (!isIndirect_) {
         isIndirect_ = true;
      }
   }

}


bool CFWidget::generate(const codeGen &,
                      const RelocBlock *trace,
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
   relocation_cerr << "CFWidget generation for " << trace->id() << endl;
   if (destMap_.empty() && !isIndirect_) {
      // No successors at all? Well, it happens if
      // we hit a halt...
      relocation_cerr << "CFWidget /w/ no successors, ret true" << endl;
      return true;
   }

   typedef enum {
      Illegal,
      Single,
      Taken_FT,
      Indirect } Options;
  
   Options opt = Illegal;

   if (isIndirect_) {
      opt = Indirect;
      relocation_cerr << "  generating CFWidget as indirect branch" << endl;
   }
   else if (isConditional_ || isCall_) {
      opt = Taken_FT;
      relocation_cerr << "  generating CFWidget as call or conditional branch" << endl;
   }
   else {
      opt = Single;
      relocation_cerr << "  generating CFWidget as direct branch" << endl;
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
         if (iter == destMap_.end()) {
            cerr << "Error in CFWidget from trace " << trace->id()
                 << ", could not find target for single control transfer" << endl;
            cerr << "\t DestMap dump:" << endl;
            for (DestinationMap::iterator d = destMap_.begin(); 
                 d != destMap_.end(); ++d) {
               cerr << "\t\t " << d->first << " : " << d->second->format() << endl;
            }
         }
            
         assert(iter != destMap_.end());

         TargetInt *target = iter->second;
         assert(target);

         if (target->necessary()) {
            if (!generateBranch(buffer,
                                target,
                                insn_,
                                trace,
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
                              trace,
                              insn_))
               return false;
         }
         else {
            assert(!isCall_);
            relocation_cerr << "  ... generating conditional branch" << endl;
            if (!generateConditionalBranch(buffer,
                                           destMap_[Taken],
                                           trace,
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
                                   trace,
                                   true)) {
                  return false;
               }
            }
         }
         break;
      }
      case Indirect: {
         Register reg = Null_Register; /* = originalRegister... */

	 // If this is an indirect tail call, we still treat it
	 // as an indirect call
         if (isCall_ || trace->block()->llb()->isIndirectTailCallBlock()) {
            if (!generateIndirectCall(buffer, 
                                      reg, 
                                      insn_, 
                                      trace,
                                      addr_)) 
               return false;
            // We may be putting another block in between this
            // one and its fallthrough due to edge instrumentation
            // So if there's the possibility for a return put in
            // a fallthrough branch
            if (destMap_.find(Fallthrough) != destMap_.end()) {
               if (!generateBranch(buffer,
                                   destMap_[Fallthrough],
                                   Instruction(),
                                   trace,
                                   true)) 
                  return false;
            }
         }
         else {
            if (!generateIndirect(buffer, reg, trace, insn_))
               return false;
         }
         break;
      }
      default:
         assert(0);
   }
   if (gap_) {
      // We don't know what the callee does to the return addr,
      // so we'll catch it at runtime. 
      buffer.addPatch(new PaddingPatch(gap_, true, false, trace->block()), 
                      padTracker(addr_, gap_,
                                 trace));
   }
  
   return true;
}

CFWidget::~CFWidget() {
   // Don't delete the Targets; they're taken care of when we nuke the overall CFG. 
}

TrackerElement *CFWidget::tracker(const RelocBlock *trace) const {
   assert(addr_ != 1);
   EmulatorTracker *e = new EmulatorTracker(addr_, trace->block(), trace->func());
   return e;
}

TrackerElement *CFWidget::destTracker(TargetInt *dest, const RelocBlock *trace) const {
   block_instance *destBlock = NULL;
   func_instance *destFunc = NULL;
   switch (dest->type()) {
      case TargetInt::RelocBlockTarget: {
         Target<RelocBlock *> *targ = static_cast<Target<RelocBlock *> *>(dest);
         assert(targ);
         assert(targ->t());
         destBlock = targ->t()->block();
         destFunc = targ->t()->func();
         assert(destBlock);
         break;
      }
      case TargetInt::BlockTarget:
         destBlock = (static_cast<Target<block_instance *> *>(dest))->t();
         assert(destBlock);
         break;
      default:
         return new EmulatorTracker(trace->block()->last(), trace->block(), trace->func());
         break;
   }
   EmulatorTracker *e = new EmulatorTracker(dest->origAddr(), destBlock, destFunc);
   return e;
}

TrackerElement *CFWidget::addrTracker(Address addr, const RelocBlock *trace) const {
   EmulatorTracker *e = new EmulatorTracker(addr, trace->block(), trace->func());
   return e;
}

TrackerElement *CFWidget::padTracker(Address addr, unsigned size, const RelocBlock *trace) const {
   PaddingTracker *p = new PaddingTracker(addr, size, trace->block(), trace->func());
   return p;
}

void CFWidget::addDestination(Address index, TargetInt *dest) {
   assert(dest);
   relocation_cerr << "CFWidget @ " << std::hex << addr() << ", adding destination " << dest->format()
                   << " / " << index << std::dec << endl;

   destMap_[index] = dest;
}

TargetInt *CFWidget::getDestination(Address dest) const {
   CFWidget::DestinationMap::const_iterator d_iter = destMap_.find(dest);
   if (d_iter != destMap_.end()) {
      return d_iter->second;
   }
   return NULL;
}


bool CFWidget::generateBranch(CodeBuffer &buffer,
                              TargetInt *to,
                              Instruction insn,
                              const RelocBlock *trace,
                              bool fallthrough) {
   assert(to);
   if (!to->necessary()) return true;

   // We can put in an unconditional branch as an ender for 
   // a block that doesn't have a real branch. So if we don't have
   // an instruction generate a "generic" branch

   // We can see a problem where we want to branch to (effectively) 
   // the next instruction. So if we ever see that (a branch of offset
   // == size) back up the codeGen and shrink us down.

   CFPatch *newPatch = new CFPatch(CFPatch::Jump, insn, to, trace->func(), addr_);

   if (fallthrough || trace->block() == NULL) {
      buffer.addPatch(newPatch, destTracker(to, trace));
   }
   else {
      buffer.addPatch(newPatch, tracker(trace));
   }
  
   return true;
}

bool CFWidget::generateCall(CodeBuffer &buffer,
                            TargetInt *to,
                            const RelocBlock *trace,
                            Instruction insn) {
   if (!to) {
      // This can mean an inter-module branch...
      return true;
   }

   CFPatch *newPatch = new CFPatch(CFPatch::Call, insn, to, trace->func(), addr_);

   buffer.addPatch(newPatch, tracker(trace));

   return true;
}

bool CFWidget::generateConditionalBranch(CodeBuffer &buffer,
                                         TargetInt *to,
                                         const RelocBlock *trace,
                                         Instruction insn) {
   assert(to);
   CFPatch *newPatch = new CFPatch(CFPatch::JCC, insn, to, trace->func(), addr_);

   buffer.addPatch(newPatch, tracker(trace));

   return true;
}


std::string CFWidget::format() const {
   stringstream ret;
   ret << "CFWidget(" << std::hex;
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
      ret << "->" << (iter->second ? iter->second->format() : "<NULL>") << ",";
   }
   ret << std::dec << ")";
   return ret.str();
}

#if 0
unsigned CFWidget::size() const
{ 
   if (insn_ != NULL) 
      return insn_->size(); 
   return 0;
}
#endif

/////////////////////////
// Patching!
/////////////////////////

CFPatch::CFPatch(Type a,
                 Instruction b,
                 TargetInt *c,
                 const func_instance *d,
                 Address e) :
  type(a), orig_insn(b), target(c), func(d), origAddr_(e) {
  if (b.isValid()) {
    insn_ptr = new unsigned char[b.size()];
    memcpy(insn_ptr, b.ptr(), b.size());
    ugly_insn = new instruction(insn_ptr, (b.getArch() == Dyninst::Arch_x86_64));
  }
  else
    ugly_insn = NULL;
}

CFPatch::~CFPatch() { 
  if (ugly_insn) {
    delete ugly_insn;
    delete[] insn_ptr;
  }
}

unsigned CFPatch::estimate(codeGen &) {
   if (orig_insn.isValid()) {
      return orig_insn.size();
   }
   return 0;
}

PaddingPatch::PaddingPatch(unsigned size, bool registerDefensive, bool noop, block_instance *b)
  : size_(size), registerDefensive_(registerDefensive), noop_(noop), block_(b) 
{
   //malware_cerr << hex << "PaddingPatch(" << size << "," << registerDefensive << "," << noop << ", [" << b->start() << " " << b->end() << ") )" << dec <<  endl;
}


bool PaddingPatch::apply(codeGen &gen, CodeBuffer *) {
   //TODO: find smarter way of telling that we're doing CFG modification, 
   // in which case we don't want to add padding in between blocks
   if (BPatch_defensiveMode != block_->obj()->hybridMode()) {
      bpwarn("WARNING: Disabling post-call block padding %s[%d]\n",FILE__,__LINE__);
      return true;
   }
   //malware_cerr << "PaddingPatch::apply, addr [" << hex << block_->end() << "]["<< gen.currAddr() << "], size " << size_ << ", registerDefensive " << (registerDefensive_ ? "<true>" : "<false>") << dec << endl;
   if (noop_) {
      gen.fill(size_, codeGen::cgNOP);
   }
   else if ( 0 == (size_ % 2) ) {
      gen.fill(size_, codeGen::cgIllegal);
   } else {
      gen.fill(size_, codeGen::cgTrap);
   }
   //cerr << "\t After filling, current addr " << hex << gen.currAddr() << dec << endl;
   //gen.fill(10, codeGen::cgNOP);
   return true;
}

unsigned PaddingPatch::estimate(codeGen &) {
   return size_;;
}

