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

#include <iostream>
#include <iomanip>

#include "patchapi_debug.h"

#include "Atom.h"
#include "InsnAtom.h" // Default Atom in each Trace
#include "Trace.h"
#include "Target.h"
#include "CFAtom.h"

#include "../CodeTracker.h"
#include "../CodeBuffer.h"
#include "../Transformers/Transformer.h" // transformer class

#include "boost/tuple/tuple.hpp"

using namespace Dyninst;
using namespace PatchAPI;
using namespace InstructionAPI;

// a Trace is a representation for instructions and instrumentation with
// a single entry point and (possibly) multiple exit points. For simplicity,
// we initially map a Trace to a basic block. However, edge instrumentation
// or post-call padding (in defensive mode) may add additional Traces. Also,
// a Trace may exit early if instrumentation explicitly branches out of a
// Trace. 
//
// A Trace is represented as a list of Atoms. An Atom represents a single
// code generation unit: an instruction, an instrumentation sequence, and
// the like. 
//
// Each Trace ends in a distinguished "CFAtom" that tracks the successors 
// of the Trace. Arguably this information should be stored in the Trace itself,
// but then we'd still need the code generation techniques in a CFAtom anyway. 

int Trace::TraceID = 0;

Trace::Ptr Trace::create(Block *block) {
  if (!block) return Ptr();

  relocation_cerr << "Creating new Trace" << endl;

  Ptr newTrace = Ptr(new Trace(block));  

  // Get the list of instructions in the block
  Block::InsnInstances insns;
  block->getInsns(insns);

  for (unsigned i = 0; i < insns.size(); ++i) {
    relocation_cerr << "  Adding instruction " 
		    << std::hex << insns[i].first << std::dec
		    << " " << insns[i].second->format() << endl;

    Atom::Ptr ptr = InsnAtom::create(insns[i].second, insns[i].first);

    if (!ptr) {
      // And this will clean up all of the created elements. Nice. 
       return Ptr();
    }
    
    newTrace->elements_.push_back(ptr);
  }
  
  newTrace->createCFAtom();

  return newTrace;
}
  
Trace::Ptr Trace::create(Atom::Ptr a, Address addr, Function *func) {
  if (!a) return Ptr();
  Ptr newTrace = Ptr(new Trace(addr, func));
  newTrace->elements_.push_back(a);
  return newTrace;
}

bool Trace::linkTraces(std::map<Block *, Trace::Ptr> &traces) {
   // We want to build each Trace into a tangled web. Or at 
   // least build in links to successor Traces. This is pretty much
   // only for our internal code generation requirements;
   // if you want real CFG traversibility use the Block representation.
   
   Successors successors;
   getSuccessors(successors, traces);

   for (Successors::iterator iter = successors.begin(); 
        iter != successors.end(); ++iter) {
      switch(iter->type) {
         case ParseAPI::INDIRECT:
            cfAtom()->addDestination(iter->target->origAddr(), iter->target);
            break;
         case ParseAPI::CALL:
         case ParseAPI::DIRECT:
         case ParseAPI::COND_TAKEN:
            cfAtom()->addDestination(CFAtom::Taken, iter->target);
            break;
         case ParseAPI::CALL_FT:
         case ParseAPI::FALLTHROUGH:
         case ParseAPI::COND_NOT_TAKEN:
            cfAtom()->addDestination(CFAtom::Fallthrough, iter->target);
         default:
            break;
      }
   }

   // Some defensive binaries have calls that play with their return address.
   // Make sure we mimic that gap between blocks. 
   // Also, if we have a call with a sink block fallthrough (that is, a call that
   // we don't know if it returns or not) put in the padding to allow later dropping
   // in a branch.

   preserveBlockGap();
   return true;
}

void Trace::createCFAtom() {
   // If the last instruction in the trace is a CF instruction 
   // (jump, call, etc.) wrap it in a CFAtom pointer and replace.
   // Otherwise, create a default CFAtom and append it. In either case,
   // keep a handle to the atom in cfAtom_.

   bool hasCF = false;

   InstructionAPI::Instruction::Ptr insn = elements_.back()->insn();
   if (insn) {
      if (insn->getCategory() == c_CallInsn ||
          insn->getCategory() == c_ReturnInsn ||
          insn->getCategory() == c_BranchInsn) {
         hasCF = true;
      }
   }

   if (hasCF) {
      cfAtom_ = CFAtom::create(elements_.back());
      elements_.pop_back();
   }
   else {
      cfAtom_ = CFAtom::create();
   }
   elements_.push_back(cfAtom_);
}

void Trace::getSuccessors(Successors &succ, const std::map<Block *, Trace::Ptr> &traces) {
   // We're constructing a copy of a subgraph of the CFG. Initially we just copy the nodes
   // (aka Traces), but we also need to copy edges. There are three types of edges we care
   // care about:
   // Edges to a block that corresponds to a Trace (TraceEdges)
   // Edges to a block that does not correspond to a Trace (BlockEdges)
   // Edges to a raw address, caused by representing inter-CodeObject edges
   //   -- this last is a Defensive mode special.

   const Block::edgelist &targets = block_->targets();
   for (Block::edgelist::iterator iter = targets.begin(); iter != targets.end(); ++iter) {
      Successor targ;
      targ.type = (*iter)->type();

      // Maybe we want exception edges too?
      if (targ.type == ParseAPI::RET || 
          targ.type == ParseAPI::NOEDGE || 
          targ.type == ParseAPI::INDIRECT) continue;

      if ((*iter)->sinkEdge()) {
         // We can get sink edges in the following situations:
         //   1) Existence of indirect control flow.
         //   2) An edge between CodeObjects
         //   3) An edge whose target was deleted as part of a code update
         //   4) An abrupt end if we think we're parsing garbage
         //   5) A call that may tamper with its return address or not return
         // In these cases we run with "mimic the original code behavior"
         switch (targ.type) {
            case ParseAPI::CALL:
            case ParseAPI::COND_TAKEN:
            case ParseAPI::DIRECT: {
               bool valid;
               Address target;
               boost::tie(valid, target) = getJumpTarget();
               if (valid) {
                  targ.target = new Target<Address>(target);
               }
               break;
            }
            case ParseAPI::COND_NOT_TAKEN:
            case ParseAPI::FALLTHROUGH:
            case ParseAPI::CALL_FT: {
               targ.target = new Target<Address>(block_->end());
               break;
            }
            default:
               break;
         }
      }
      else {
         Block *targBlock = (*iter)->trg();
         std::map<Block *, Trace::Ptr>::const_iterator iter = traces.find(targBlock);
         if (iter != traces.end()) {
            targ.target = new Target<Trace::Ptr>(iter->second);
         }
         else {
            targ.target = new Target<Block *>(targBlock);
         }
      }
      if (targ.target) {
         succ.push_back(targ);
      }
   }
}

// Some defensive binaries put gaps in after call instructions to try
// and confuse parsing; it's typically something like so:
//
// call foo
// jmp <offset>
//
// where foo contains code that increments the stack pointer by one,
// and <offset> is the encoding of a legal instruction. We really need
// to preserve that gap if it exists, and to make life easy we bundle
// it into the CFAtom.

#define DEFENSIVE_GAP_SIZE 10

void Trace::preserveBlockGap() {
   const Block::edgelist &targets = block_->targets();
   for (Block::edgelist::iterator iter = targets.begin(); iter != targets.end(); ++iter) {
      if ((*iter)->type() == ParseAPI::CALL_FT ||
          (*iter)->type() == ParseAPI::FALLTHROUGH ||
          (*iter)->type() == ParseAPI::COND_NOT_TAKEN) {
         // Okay, I admit - I want to see this code trigger in the
         // fallthrough or cond_not_taken cases...
         Block *target = (*iter)->trg();
         if (target) {
            cfAtom()->setGap(target->start() - block_->end());
            return;
         }
         else {
            // No target... very odd
            cfAtom()->setGap(DEFENSIVE_GAP_SIZE);
         }
      }
   }
}

// Do the raw computation to determine the target (if it exists) of a
// jump instruction that we may not have encoded in ParseAPI.

std::pair<bool, Address> Trace::getJumpTarget() {
   InstructionAPI::Instruction::Ptr insn = cfAtom()->insn();
   if (!insn) return std::make_pair(false, 0);

   Expression::Ptr cft = insn->getControlFlowTarget();
   if (!cft) return std::make_pair(false, 0);

   Expression::Ptr thePC(new RegisterAST(MachRegister::getPC(insn->getArch())));
   
   cft->bind(thePC.get(), Result(u64, cfAtom()->addr()));
   Result res = cft->eval();
   if (res.defined) {
      return std::make_pair(true, res.convert<Address>());
   }
   return std::make_pair(false, 0);
}

// We put in edges for everything - jumps, fallthroughs, you name
// it. Some of these can be obviated by code layout. Instead of trying
// to precompute which branches are needed, we do a final
// pre-generation pass to run through and see which branches we need
// based on where we plan to lay blocks out.
//
// Side note: necessary is set on a per-Target basis. 

void Trace::determineNecessaryBranches(Trace *successor) {
   for (CFAtom::DestinationMap::const_iterator d_iter = cfAtom_->destinations().begin();
        d_iter != cfAtom_->destinations().end(); ++d_iter) {
      if (d_iter->first != CFAtom::Taken &&
          d_iter->first != CFAtom::Fallthrough) continue;

      d_iter->second->setNecessary(true);

      if (d_iter->second->matches(successor)) {
         // A candidate for setting as non-necessary. However, there
         // are some overrides...
         //   1) An old one that was in there was if we had post-CF 
         //      instruction padding, be sure to set this as necessary.
         //      However, I'm not sure this can realistically happen...


         //   2) If the block consisted only of a jump, be sure to keep it
         //      so that we don't entirely remove original blocks.
         if ((elements().size() == 1) &&
             (cfAtom_->destinations().size() == 1)) {
            continue;
         }
         d_iter->second->setNecessary(false);
      }
   }
}   

// Each Trace generates a mixture of PIC and non-PIC code. For
// efficiency, we precompute the PIC code and generate function callbacks
// for the non-PIC code. Thus, what we hand into the code generator
// looks like so:
//
// PIC
// PIC
// PIC
// non-PIC callback
// PIC
// non-PIC callback
// PIC
// PIC
// ...
//
// This allows us to minimize unnecessary regeneration when we're trying
// to produce the final code sequence. This function generates the mixture
// of PIC (as a byte buffer) and non-PIC (in terms of Patch objects) sequences.


bool Trace::generate(const codeGen &templ,
                     CodeBuffer &buffer) {
   relocation_cerr << "Generating block " << id() << " orig @ " << hex << origAddr() << dec << endl;
   
   // Register ourselves with the CodeBuffer and get a label
   label_ = buffer.getLabel();

   // Simple form: iterate over every Atom, in order, and generate it.
   for (AtomList::iterator iter = elements_.begin(); iter != elements_.end(); ++iter) {
      if (!(*iter)->generate(templ, 
                             this,
                             buffer)) {
         return false;
         // This leaves the block in an inconsistent state and should only be used
         // for fatal failures.
      }
   }
   
   return true;
}

std::string Trace::format() const {
    stringstream ret;
  ret << "Trace(" 
      << std::hex << origAddr() << std::dec
      << "/" << id() 
      << ") {" << endl;
  for (AtomList::const_iterator iter = elements_.begin();
       iter != elements_.end(); ++iter) {
    ret << "  " << (*iter)->format() << endl;
  }
  ret << "}" << endl;
  return ret.str();
}
