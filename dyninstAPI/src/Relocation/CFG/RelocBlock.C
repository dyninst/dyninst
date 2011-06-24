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

#include "../patchapi_debug.h"

#include "../Widgets/Widget.h"
#include "../Widgets/InsnWidget.h" // Default Widget in each RelocBlock
#include "../Widgets/InstWidget.h"
#include "RelocBlock.h"
#include "RelocTarget.h"
#include "../Widgets/CFWidget.h"

#include "../CodeTracker.h"
#include "../CodeBuffer.h"
#include "../Transformers/Transformer.h" // transformer class
#include "RelocGraph.h"

#include "boost/tuple/tuple.hpp"


using namespace Dyninst;
using namespace Relocation;
using namespace InstructionAPI;

// a RelocBlock is a representation for instructions and instrumentation with
// a single entry point and (possibly) multiple exit points. For simplicity,
// we initially map a RelocBlock to a basic block. However, edge instrumentation
// or post-call padding (in defensive mode) may add additional RelocBlocks. Also,
// a RelocBlock may exit early if instrumentation explicitly branches out of a
// RelocBlock. 
//
// A RelocBlock is represented as a list of Widgets. An Widget represents a single
// code generation unit: an instruction, an instrumentation sequence, and
// the like. 
//
// Each RelocBlock ends in a distinguished "CFWidget" that tracks the successors 
// of the RelocBlock. Arguably this information should be stored in the RelocBlock itself,
// but then we'd still need the code generation techniques in a CFWidget anyway. 

int RelocBlock::RelocBlockID = 0;

RelocBlock *RelocBlock::createReloc(block_instance *block, func_instance *func) {
  if (!block) return NULL;

  relocation_cerr << "Creating new RelocBlock" << endl;

  RelocBlock *newRelocBlock = new RelocBlock(block, func);

  // Get the list of instructions in the block
  block_instance::Insns insns;
  block->getInsns(insns);

  for (block_instance::Insns::iterator iter = insns.begin();
       iter != insns.end(); ++iter) {
    relocation_cerr << "  Adding instruction " 
		    << std::hex << iter->first << std::dec
		    << " " << iter->second->format() << endl;
    Widget::Ptr ptr = InsnWidget::create(iter->second, iter->first);

    if (!ptr) {
       delete newRelocBlock;
       return NULL;
    }
    
    newRelocBlock->elements_.push_back(ptr);
  }

  // TODO: this should be done just before code generation, 
  // not up front; however, several transformers depend
  // on this behavior
  newRelocBlock->createCFWidget();
  
  return newRelocBlock;
}
  
RelocBlock *RelocBlock::createInst(instPoint *p, Address a, block_instance *block, func_instance *f) {
  if (!p) return NULL;
  if (p->empty()) return NULL;

  RelocBlock *newRelocBlock = new RelocBlock(a, block, f);
  newRelocBlock->elements_.push_back(InstWidget::create(p));
  newRelocBlock->createCFWidget();
  
  return newRelocBlock;
}

RelocBlock *RelocBlock::createStub(block_instance *block, func_instance *f) {
   RelocBlock *newRelocBlock = new RelocBlock(block->start(), block, f);
   newRelocBlock->createCFWidget();
   newRelocBlock->type_ = Stub; 
   return newRelocBlock;
}




bool RelocBlock::linkRelocBlocks(RelocGraph *cfg) {
   // We want to build each RelocBlock into a tangled web. Or at 
   // least build in links to successor RelocBlocks. This is pretty much
   // only for our internal code generation requirements;
   // if you want real CFG traversibility use the Block representation.
   
   getPredecessors(cfg);
   getSuccessors(cfg);

   return true;
}

void RelocBlock::getSuccessors(RelocGraph *cfg) {
   // We're constructing a copy of a subgraph of the CFG. Initially we just copy the nodes
   // (aka RelocBlocks), but we also need to copy edges. There are three types of edges we care
   // care about:
   // Edges to a block that corresponds to a RelocBlock (RelocBlockEdges)
   // Edges to a block that does not correspond to a RelocBlock (BlockEdges)
   // Edges to a raw address, caused by representing inter-CodeObject edges
   //   -- this last is a Defensive mode special.
  /*   const block_instance::edgelist &targets = block_->targets();
       for (block_instance::edgelist::const_iterator iter = targets.begin(); iter != targets.end(); ++iter) {*/
   const PatchBlock::edgelist &targets = block_->getTargets();
   for (PatchBlock::edgelist::const_iterator iter = targets.begin(); iter != targets.end(); ++iter) {
     processEdge(OutEdge, SCAST_EI(*iter), cfg);
   }
}

void RelocBlock::getPredecessors(RelocGraph *cfg) {
   const PatchBlock::edgelist &edges = block_->getSources();
   for (PatchBlock::edgelist::const_iterator iter = edges.begin(); iter != edges.end(); ++iter) {
     processEdge(InEdge, SCAST_EI(*iter), cfg);
   }

}

// There's some tricky logic going on here. We want to create the following 
// edges:
// 1) All out-edges, as _someone_ has to create them
// 2) In-edges that aren't from RelocBlocks; if it's from a RelocBlock we assume we'll
//    get it in out-edge construction. 

void RelocBlock::processEdge(EdgeDirection e, edge_instance *edge, RelocGraph *cfg) {
   ParseAPI::EdgeTypeEnum type = edge->type();
   // Maybe we want exception edges too?
   if (type == ParseAPI::RET || 
       type == ParseAPI::NOEDGE) return;
   
   if (edge->sinkEdge()) {
      assert(e == OutEdge);
      // We can get sink edges in the following situations:
      //   1) Existence of indirect control flow.
      //   2) An edge between CodeObjects
      //   3) An edge whose target was deleted as part of a code update
      //   4) An abrupt end if we think we're parsing garbage
      //   5) A call that may tamper with its return address or not return
      // In these cases we run with "mimic the original code behavior"
      switch (type) {
         case ParseAPI::CALL:
         case ParseAPI::COND_TAKEN:
         case ParseAPI::DIRECT: {
            bool valid;
            Address addr;
            boost::tie(valid, addr) = getJumpTarget();
            if (valid) {
               cfg->makeEdge(new Target<RelocBlock *>(this), 
                             new Target<Address>(addr), 
                             type);
            }
            break;
         }
         case ParseAPI::COND_NOT_TAKEN:
         case ParseAPI::FALLTHROUGH:
         case ParseAPI::CALL_FT: {
            cfg->makeEdge(new Target<RelocBlock *>(this), 
                          new Target<Address>(block_->end()), 
                          type);
            break;
         }
         default:
            break;
      }
   }
   else {
      block_instance *block = (e == OutEdge) ? edge->trg() : edge->src();

      RelocBlock *t = cfg->find(block);
      if (t) {
         if (e == OutEdge) {
            cfg->makeEdge(new Target<RelocBlock *>(this), 
                          new Target<RelocBlock *>(t),
                          type);
            return;
         }
         else {
            // RelocBlock -> trace edge, will get created later
            return;
         }
      }
      else {
         if (e == OutEdge) {
            cfg->makeEdge(new Target<RelocBlock *>(this), 
                          new Target<block_instance *>(block), 
                          type);
         }
         else {
            cfg->makeEdge(new Target<block_instance *>(block), 
                          new Target<RelocBlock *>(this),
                          type);
         }
      }
   }
}


bool RelocBlock::determineSpringboards(PriorityMap &p) {
   // We _require_ a springboard if:
   // 1) We are a function entry block;
   // 2) We are the target of an indirect branch;
   // 3) We are the target of an edge not from a trace. 

   if (func_ &&
       func_->entryBlock() == block_) {
      p[block_] = Required;
      return true;
   }
   if (inEdges_.contains(ParseAPI::INDIRECT)) {
      p[block_] = Required;
      return true;
   }
   // Slow crawl
   for (RelocEdges::const_iterator iter = inEdges_.begin();
        iter != inEdges_.end(); ++iter) {
      if ((*iter)->src->type() != TargetInt::RelocBlockTarget) {
         p[block_] = Required;
         return true;
      }
   }
   return true;
}


void RelocBlock::createCFWidget() {
   // If the last instruction in the trace is a CF instruction 
   // (jump, call, etc.) wrap it in a CFWidget pointer and replace.
   // Otherwise, create a default CFWidget and append it. In either case,
   // keep a handle to the atom in cfWidget_.

   if (elements_.empty()) {
      cfWidget_ = CFWidget::create(origAddr_);
      elements_.push_back(cfWidget_); 
      return;
   }

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
      cfWidget_ = CFWidget::create(elements_.back());
      elements_.pop_back();
   }
   else {
      cfWidget_ = CFWidget::create(origAddr_);
   }
   elements_.push_back(cfWidget_);
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
// it into the CFWidget.

#if defined(arch_x86) || defined(arch_x86_64)
#define DEFENSIVE_GAP_SIZE 10
#else
#define DEFENSIVE_GAP_SIZE 12
#endif

void RelocBlock::preserveBlockGap() {
  /*   const block_instance::edgelist &targets = block_->targets();
       for (block_instance::edgelist::const_iterator iter = targets.begin(); iter != targets.end(); ++iter) {*/
   const PatchBlock::edgelist &targets = block_->getTargets();
   for (PatchBlock::edgelist::const_iterator iter = targets.begin(); iter != targets.end(); ++iter) {
      if ((*iter)->type() == ParseAPI::CALL_FT ||
          (*iter)->type() == ParseAPI::FALLTHROUGH ||
          (*iter)->type() == ParseAPI::COND_NOT_TAKEN) {
         // Okay, I admit - I want to see this code trigger in the
         // fallthrough or cond_not_taken cases...
         block_instance *target = SCAST_EI(*iter)->trg();
         if (target) {
            cfWidget()->setGap(target->start() - block_->end());
            return;
         }
         else {
            // No target... very odd
            cfWidget()->setGap(DEFENSIVE_GAP_SIZE);
         }
      }
   }
}

// Do the raw computation to determine the target (if it exists) of a
// jump instruction that we may not have encoded in ParseAPI.

std::pair<bool, Address> RelocBlock::getJumpTarget() {
   InstructionAPI::Instruction::Ptr insn = cfWidget()->insn();
   if (!insn) return std::make_pair(false, 0);

   Expression::Ptr cft = insn->getControlFlowTarget();
   if (!cft) return std::make_pair(false, 0);

   Expression::Ptr thePC(new RegisterAST(MachRegister::getPC(insn->getArch())));
   
   cft->bind(thePC.get(), Result(u64, cfWidget()->addr()));
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

void RelocBlock::determineNecessaryBranches(RelocBlock *successor) {
   for (CFWidget::DestinationMap::const_iterator d_iter = cfWidget_->destinations().begin();
        d_iter != cfWidget_->destinations().end(); ++d_iter) {
      if (d_iter->first != CFWidget::Taken &&
          d_iter->first != CFWidget::Fallthrough) continue;

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
             (cfWidget_->destinations().size() == 1)) {
            continue;
         }
         d_iter->second->setNecessary(false);
      }
   }
}   

// Each RelocBlock generates a mixture of PIC and non-PIC code. For
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


bool RelocBlock::generate(const codeGen &templ,
                     CodeBuffer &buffer) {
   relocation_cerr << "Generating block " << id() << " orig @ " << hex << origAddr() << dec << endl;
   relocation_cerr << "\t" << elements_.size() << " elements" << endl;
   
   // Register ourselves with the CodeBuffer and get a label
   label_ = buffer.getLabel();

   // Simple form: iterate over every Widget, in order, and generate it.
   for (WidgetList::iterator iter = elements_.begin(); iter != elements_.end(); ++iter) {
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

std::string RelocBlock::format() const {
    stringstream ret;
  ret << "RelocBlock(" 
      << std::hex << origAddr() << std::dec
      << "/" << id() << "/" << label_ 
      << ") {" << endl;
  for (WidgetList::const_iterator iter = elements_.begin();
       iter != elements_.end(); ++iter) {
    ret << "  " << (*iter)->format() << endl;
  }
  ret << "In edges: ";
  for (RelocEdges::const_iterator iter = inEdges_.begin();
       iter != inEdges_.end(); ++iter) {
     ret << (*iter)->src->format() << ", ";
  }
  ret << endl;
  ret << "Out edges:";
  for (RelocEdges::const_iterator iter = outEdges_.begin();
       iter != outEdges_.end(); ++iter) {
     ret << (*iter)->trg->format() << ", ";
  }
  ret << endl;

  ret << "}" << endl;
  return ret.str();
}

RelocBlock::Label RelocBlock::getLabel() const {
   if (label_ == -1) {
      cerr << "Error: trace with zero label!" << endl;
      cerr << format() << endl;
      assert(0);
   }
   return label_;
}

#if 0
// Logic: interpose the new provided target along all edges
// of the provided type. 
bool RelocBlock::interposeTarget(ParseAPI::EdgeTypeEnum type,
                            RelocBlock *newRelocBlock) {
   assert(newRelocBlock);

   TargetInt *newTarget = new Target<RelocBlock *>(newRelocBlock.get());

   std::map<ParseAPI::EdgeTypeEnum, Targets>::iterator oe_iter = outEdges_.find(type);
   if (oe_iter == outEdges_.end()) return true;

   newRelocBlock->outEdges_[ParseAPI::FALLTHROUGH] = oe_iter->second;

   // Fix up in-edges as necessary
   for (Targets::iterator iter = oe_iter->second.begin();
        iter != oe_iter->second.end(); ++iter) {
      // If we're pointing to a RelocBlock, fix its in-edge
      if ((*iter)->type() == TargetInt::RelocBlockTarget) {
         Target<RelocBlock *> *t = static_cast<Target<RelocBlock *> *>(*iter);
         t->t()->replaceInEdge(type, this, newTarget);
      }
   }

   // And replace our out-edge
   oe_iter->second.clear();
   oe_iter->second.push_back(newTarget);
   return true;
}

bool RelocBlock::moveSources(ParseAPI::EdgeTypeEnum type,
                        RelocBlock *newRelocBlock) {
   // Move all the in-edges of a particular type to
   // newRelocBlock. Like interposition, but does not
   // create an edge from newSource to us. 

   assert(newRelocBlock);

   TargetInt *newTarget = new Target<RelocBlock *>(newRelocBlock.get());
   
   std::map<ParseAPI::EdgeTypeEnum, Targets>::iterator ie_iter = inEdges_.find(type);
   if (ie_iter == inEdges_.end()) return true;

   // Fix up in-edges as necessary
   for (Targets::iterator iter = ie_iter->second.begin();
        iter != ie_iter->second.end(); ++iter) {
      // If we're pointing to a RelocBlock, fix its out-edge
      // For that matter, we can probably assert trace-ness...
      assert((*iter)->type() == TargetInt::RelocBlockTarget); 
      Target<RelocBlock *> *t = static_cast<Target<RelocBlock *> *>(*iter);
      t->t()->replaceOutEdge(type, this, newTarget);
   }
   return true;
}

bool RelocBlock::interposeTarget(edge_instance *edge, 
                     RelocBlock *newRelocBlock) {
   assert(edge->src() == block());
   // Like interposeTarget /w/ a type, but for a
   // specific element...
   assert(newRelocBlock);

   TargetInt *newTarget = new Target<RelocBlock *>(newRelocBlock.get());
   
   std::map<ParseAPI::EdgeTypeEnum, Targets>::iterator oe_iter = outEdges_.find(edge->type());
   if (oe_iter == outEdges_.end()) return true;

   // Fix up in-edges as necessary
   for (Targets::iterator iter = oe_iter->second.begin();
        iter != oe_iter->second.end(); ++iter) {
      // If we're pointing to a RelocBlock, fix its in-edge
      if ((*iter)->type() == TargetInt::RelocBlockTarget) {
         Target<RelocBlock *> *t = static_cast<Target<RelocBlock *> *>(*iter);
         if (t->t()->block() != edge->trg()) continue;

         t->t()->replaceInEdge(edge->type(), this, newTarget);
         newRelocBlock->outEdges_[ParseAPI::DIRECT].push_back(*iter);
         oe_iter->second.erase(iter);
         oe_iter->second.push_back(newTarget);
         return true;
      }
      else if ((*iter)->type() == TargetInt::BlockTarget) {
         Target<block_instance *> *t = static_cast<Target<block_instance *> *>(*iter);
         if (t->t() != edge->trg()) continue;

         newRelocBlock->outEdges_[ParseAPI::DIRECT].push_back(*iter);
         oe_iter->second.erase(iter);
         oe_iter->second.push_back(newTarget);
         return true;
      }
   }

   return true;
}

RelocBlock::Targets &RelocBlock::getTargets(ParseAPI::EdgeTypeEnum type) {
   return outEdges_[type];
}

bool RelocBlock::removeTargets() {
   cerr << "removeTargets in " << id() << endl;
#if 0
   for (std::map<ParseAPI::EdgeTypeEnum, Targets>::iterator iter = outEdges_.begin();
        iter != outEdges_.end(); ++iter) {
      for (Targets::iterator iter2 = iter->second.begin(); 
           iter2 != iter->second.end(); ++iter2) {
         cerr << "Deleting " << (*iter2)->format() << endl;
         delete *iter2;
      }
   }
#endif
   outEdges_.clear();
   cerr << "Cleared all edges" << endl;
   return true;
}

bool RelocBlock::removeTargets(ParseAPI::EdgeTypeEnum type) {
   Targets &t = outEdges_[type];
   for (Targets::iterator iter = t.begin(); iter != t.end(); ++iter) {
      delete *iter;
   }
   
   outEdges_.erase(type);
   return true;
}


bool RelocBlock::replaceTarget(RelocBlock *oldRelocBlock, RelocBlock *newRelocBlock) {
   cerr << "Replacing target in trace" << endl;
   cerr << format();
   for (Edges::iterator iter = outEdges_.begin(); iter != outEdges_.end(); ++iter) {
      Targets newTargs;
      for (Targets::iterator t_iter = iter->second.begin(); t_iter != iter->second.end(); ++t_iter) {
         if ((*t_iter)->type() != TargetInt::RelocBlockTarget) {
            newTargs.push_back(*t_iter);
            continue;
         }
         Target<RelocBlock *> *t = static_cast<Target<RelocBlock *> *>(*t_iter);
         if (t->t() != oldRelocBlock.get()) {
            newTargs.push_back(*t_iter);
            continue;
         }
         delete t;
         newTargs.push_back(new Target<RelocBlock *>(newRelocBlock.get()));
      }
      iter->second = newTargs;
   }
   cerr << "Replaced target" << endl;
   cerr << format(); 
   return true;
}


void RelocBlock::replaceInEdge(ParseAPI::EdgeTypeEnum type,
                   RelocBlock *oldSource,
                   TargetInt *newSource) {
   std::map<ParseAPI::EdgeTypeEnum, Targets>::iterator ie_iter = inEdges_.find(type);
   if (ie_iter == inEdges_.end()) return;

   // Fix up in-edges as necessary
   for (Targets::iterator iter = ie_iter->second.begin();
        iter != ie_iter->second.end(); ++iter) {
      // If we're pointing to a RelocBlock, fix its in-edge
      if ((*iter)->type() == TargetInt::RelocBlockTarget) {
         Target<RelocBlock *> *t = static_cast<Target<RelocBlock *> *>(*iter);
         if (t->t() == oldSource) {
            // Got it!
            ie_iter->second.erase(iter);
            ie_iter->second.push_back(newSource);
            return;
         }
      }
   }
   return;
}

void RelocBlock::replaceOutEdge(ParseAPI::EdgeTypeEnum type,
                           RelocBlock *oldTarget,
                           TargetInt *newTarget) {
   std::map<ParseAPI::EdgeTypeEnum, Targets>::iterator oe_iter = outEdges_.find(type);
   if (oe_iter == outEdges_.end()) return;

   // Fix up in-edges as necessary
   for (Targets::iterator iter = oe_iter->second.begin();
        iter != oe_iter->second.end(); ++iter) {
      // If we're pointing to a RelocBlock, fix its in-edge
      if ((*iter)->type() == TargetInt::RelocBlockTarget) {
         Target<RelocBlock *> *t = static_cast<Target<RelocBlock *> *>(*iter);
         if (t->t() == oldTarget) {
            // Got it!
            oe_iter->second.erase(iter);
            oe_iter->second.push_back(newTarget);
            return;
         }
      }
   }
   return;
}


void RelocBlock::setAsInstrumentationRelocBlock() {
   origRelocBlock_ = false;
}

RelocBlock *RelocBlock::split(WidgetList::iterator where) {
   // Create a new trace and move where and everything
   // after it to the new trace. 
   // Don't use block creation because it pulls in
   // all the Widgets in the block...
   RelocBlock *newRelocBlock = new RelocBlock(block(), func());

   TargetInt *newTarget = new Target<RelocBlock *>(newRelocBlock.get());

   // This isn't necessarily bad, it just raises questions
   // I don't want to answer right now. Forex, do we move
   // the CFWidget or leave a trace with no CFWidget?
   assert(where != elements_.end());

   // Tell all our targets they have a replacement source
   for (std::map<ParseAPI::EdgeTypeEnum, Targets>::iterator iter = outEdges_.begin();
        iter != outEdges_.end(); ++iter) {
      for (Targets::iterator iter2 = iter->second.begin(); 
           iter2 != iter->second.end(); ++iter2) {
         if ((*iter2)->type() == TargetInt::RelocBlockTarget) {
            Target<RelocBlock *> *t = static_cast<Target<RelocBlock *> *>(*iter2);
            t->t()->replaceInEdge(iter->first, this, newTarget);
         }
      }
   }

   // Move all our targets to the new block
   newRelocBlock->outEdges_ = outEdges_;
   outEdges_.clear();

   // And drop in a fallthrough
   outEdges_[ParseAPI::FALLTHROUGH].push_back(newTarget);
   newRelocBlock->inEdges_[ParseAPI::FALLTHROUGH].push_back(newTarget);

   unsigned count = elements_.size();
   {
      relocation_cerr << "Starting with " << count << " elements..." << endl;
      relocation_cerr << "\t And the new trace has " << newRelocBlock->elements_.size() << endl;
      for (WidgetList::iterator d = elements_.begin(); d != elements_.end(); ++d) {
         relocation_cerr << "\t" << (*d)->format() << endl;
         if (*d == *where) relocation_cerr << "\t\t Split point" << endl;
      }
   }
   // Now, move over the Widgets that we want to. 
   newRelocBlock->elements_.splice(newRelocBlock->elements_.begin(),
                              elements_, where, elements_.end());
   {
      relocation_cerr << "Post splice, we have " << elements_.size() << endl;
      for (WidgetList::iterator d = elements_.begin(); d != elements_.end(); ++d) {
         relocation_cerr << "\t" << (*d)->format() << endl;
      }
      relocation_cerr << "And the new trace has " << newRelocBlock->elements_.size() << endl;
      for (WidgetList::iterator d = newRelocBlock->elements_.begin(); d != newRelocBlock->elements_.end(); ++d) {
         relocation_cerr << "\t" << (*d)->format() << endl;
      }
   }

   assert((elements_.size() + newRelocBlock->elements_.size()) == count);


   // Fix up CFWidgets; move the one we have to newRelocBlock and create a
   // new one
   // We need to do this post-assert (above) as it might create a new atom
   newRelocBlock->cfWidget_ = cfWidget_;
   
   createCFWidget();

   return newRelocBlock;
}

#endif

bool RelocBlock::finalizeCF() {
   if (!cfWidget_) {
      cerr << "Warning: trace has no CFWidget!" << endl;
      cerr << format() << endl;
      assert(0);
   }

   // We've had people munging our out-edges; now
   // push them to the CFWidget so that it can do its work. 
   for (RelocEdges::iterator iter = outEdges_.begin(); iter != outEdges_.end(); ++iter) {
      if ((*iter)->type == ParseAPI::CATCH ||
          (*iter)->type == ParseAPI::RET ||
          (*iter)->type == ParseAPI::NOEDGE) continue;
      Address index;
      if ((*iter)->type == ParseAPI::FALLTHROUGH ||
          (*iter)->type == ParseAPI::COND_NOT_TAKEN ||
          (*iter)->type == ParseAPI::CALL_FT) {
         index = CFWidget::Fallthrough;
      }
      else if ((*iter)->type == ParseAPI::DIRECT ||
               (*iter)->type == ParseAPI::COND_TAKEN ||
               (*iter)->type == ParseAPI::CALL) {
         index = CFWidget::Taken;
      }
      else {
         assert((*iter)->type == ParseAPI::INDIRECT);
         index = (*iter)->trg->origAddr();
      }
      cfWidget_->addDestination(index, (*iter)->trg);
      (*iter)->trg->setNecessary(isNecessary((*iter)->trg, (*iter)->type));
   }
   
   return true;
}

bool RelocBlock::isNecessary(TargetInt *target,
                        ParseAPI::EdgeTypeEnum edgeType) {
   if (!next_) return true;

   // Code copied from the old Fallthrough transformer

   // Case 1: if we're a single direct branch, be sure we don't
   // elide, or the CFG gets messed up. 
   // ... or does it...
   if (elements_.size() == 1 &&
       (edgeType == ParseAPI::DIRECT ||
        edgeType == ParseAPI::COND_TAKEN))
      return true;

   // Case 2: keep calls, d00d
   if (edgeType == ParseAPI::CALL) return true;
   
   // Case 3: if the CFWidget wants a gap, a gap it gets
   if (cfWidget_->gap() != 0) return true;

   // And finally, case 4: if the next trace isn't our target, keep it
   if (!target->matches(next_)) return true;

   return false;
}


