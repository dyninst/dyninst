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



#include "Transformer.h"
#include "Instrumenter.h"
#include "dyninstAPI/src/debug.h"
#include "../Widgets/Widget.h"
#include "../CFG/RelocTarget.h"
#include "dyninstAPI/src/instPoint.h"
#include "dyninstAPI/src/baseTramp.h"
#include "../Widgets/InstWidget.h"
#include "../Widgets/CFWidget.h"
#include "../CFG/RelocGraph.h"

using namespace std;
using namespace Dyninst;
using namespace Relocation;
using namespace InstructionAPI;

bool Instrumenter::process(RelocBlock *trace,
                           RelocGraph *cfg) {
   assert(trace);
   if (trace->type() != RelocBlock::Relocated) {
      // One of ours!
      return true;
   }
   if (trace == skip) {
      skip = NULL;
      return true;
   }

   relocation_cerr << "Processing trace " << trace->id() << " @ " << hex << trace->origAddr() << dec << endl;

   // Hoo boy. Fun for the whole family...
   //
   // This transformer needs to create instrumentation blocks throughout the 
   // provided CFG (of RelocBlocks). We consider two classes of added
   // instrumentation:
   // Block insertion instrumentation (function entry, edge, post-call);
   // Block augmentation instrumentation (block entry, pre/post insn, pre-call).
   //
   // Block insertion instrumentation adds a new block (RelocBlock) to the CFG
   // that contains the instrumentation code. We will need to edit edges
   // to get this to work. 
   //
   // Block augmentation instrumentation just inserts a new Widget into the trace;
   // this is comparitively trivial. 

   // Let's run with non-block-inserting first...
   // This should be block inserting, but for now we're claiming just-before-return. 
   // Also, we tend to use a "highest container gets executed first" model,
   // which means we instrument the _lowest_ levels first. 
   if (!insnInstrumentation(trace)) return false;
   if (!preCallInstrumentation(trace)) return false;
   if (!blockEntryInstrumentation(trace)) return false;
   if (!blockExitInstrumentation(trace)) return false;


   // And on to the graph modification shtuff. 
   if (!funcExitInstrumentation(trace, cfg)) return false;
   if (!postCallInstrumentation(trace, cfg)) return false;
   if (!edgeInstrumentation(trace, cfg)) return false;
   if (!funcEntryInstrumentation(trace, cfg)) return false;
   return true;
}

bool Instrumenter::insnInstrumentation(RelocBlock *trace) {
   // We're going to unify pre- and post- instruction instrumentation
   // into a single pass for efficiency. 
   PatchAPI::InsnPoints::const_iterator pre;
   PatchAPI::InsnPoints::const_iterator preEnd;
   PatchAPI::InsnPoints::const_iterator post;
   PatchAPI::InsnPoints::const_iterator postEnd;

   bool instPre = false;
   bool instPost = false;


   if (trace->func()) {
      instPre = trace->func()->findInsnPoints(instPoint::PreInsn, trace->block(),
                                              pre, preEnd);
      instPost = trace->func()->findInsnPoints(instPoint::PostInsn, trace->block(),
                                               post, postEnd);
   }
   else {
      assert(0 && "Unimplemented!");
   }


   RelocBlock::WidgetList::iterator elem = trace->elements().begin();
   if (elem == trace->elements().end()) return true;
   while ((instPre && (pre != preEnd)) ||
          (instPost && (post != postEnd))) {
      Address preAddr = 0;
      if (pre != preEnd) {
         preAddr = pre->first;
      }
      Address postAddr = 0;
      if (post != postEnd) {
         postAddr = post->first;
      }
      assert(elem != trace->elements().end());

      Address next;
      if (preAddr == 0) next = postAddr;
      else if (postAddr == 0) next = preAddr;
      else next = (preAddr < postAddr) ? preAddr : postAddr;

      while ((*elem)->addr() == 0 ||
             (*elem)->addr() < next) {
         ++elem;
         if (elem == trace->elements().end()) return true;
      }

      if ((preAddr != 0) &&
          (preAddr < (*elem)->addr())) {
         // We missed this? Bad instPoint!
         assert(0);
      }
      if ((postAddr != 0) &&
          (postAddr < (*elem)->addr())) {
         assert(0);
      }

      if (preAddr == (*elem)->addr()) {
         if (!pre->second->empty()) {
            Widget::Ptr inst = makeInstrumentation(pre->second);
            if (!inst) {
	      relocation_cerr << "Failed to make pre-instrumentation at " << hex << preAddr << ", ret failed" << dec << endl;
	      return false;
	    }
            trace->elements().insert(elem, inst);
         }
         ++pre;
      }
      if (postAddr == (*elem)->addr()) {
         if (!post->second->empty()) {
            // We can split an instruction into multiple widgets so skip over all of them...
            RelocBlock::WidgetList::iterator tmp = elem;
            while ((tmp != trace->elements().end()) &&
                   ((*tmp)->addr() == postAddr) &&
                   ((*tmp)->insn().isValid())) ++tmp;
            Widget::Ptr inst = makeInstrumentation(post->second);
            if (!inst) {
	      relocation_cerr << "Failed to make post-instrumentation at " << hex << postAddr << ", ret failed" << dec << endl;
	      return false;
	    }
            trace->elements().insert(tmp, inst);
         }
         ++post;
      }
   }

   return true;
}

bool Instrumenter::preCallInstrumentation(RelocBlock *trace) {
   instPoint *call = NULL;
   if (trace->func()) {
     call = trace->func()->preCallPoint(trace->block(), false);
   }
   if (!call || call->empty()) return true;

   RelocBlock::WidgetList &elements = trace->elements();
   // For now, we're inserting this instrumentation immediately before the last instruction
   // in the list of elements. 

   // Unfortunately, we may have split the last instruction into multiple Widgets to 
   // piecewise emulate. Therefore, we can't just drop it in second-to-last; instead, we
   // need to put it in before that instruction. 

   Widget::Ptr inst = makeInstrumentation(call);
   if (!inst) return false;

   RelocBlock::WidgetList::reverse_iterator riter = elements.rbegin();
   InstructionAPI::Instruction call_insn = (*riter)->insn();
   if (call_insn.isValid()) {
      while (riter != elements.rend() && (*riter)->insn() == call_insn) ++riter;
   }
   elements.insert(riter.base(), inst);

   return true;
}

bool Instrumenter::funcExitInstrumentation(RelocBlock *trace, RelocGraph *cfg) {
   // Function exit instrumentation is complicated by the existence of conditional
   // return instructions (e.g., PowerPC's bclr and its various mnemonics). 
   // If we have an unconditional return, we simply insert the basetramp immediately
   // before the return instruction. That's trivial. If we have a conditional return,
   // we construct the following sequence:
   //
   // let retcc = conditional return
   // let jcc = a conditional branch with the same condition as retcc
   // let ret = unconditional return (... not strictly necessary, but nice)
   //
   // New "instruction stream"
   // 
   // jcc - taken: inst, not taken: ft
   // inst:
   //   instWidget
   //   ret (or retcc, really...)
   // ft:
   //
   // Since CFWidgets end blocks, this requires:
   //   1) Replace retcc with jcc at the end of the current relocBlock
   //   2) Create a new relocBlock with instWidget and ret/retcc
   //   3) Hook up edges so that jcc "taken" to the new relocBlock and "not taken" to ft. 

   instPoint *exit = trace->func()->funcExitPoint(trace->block(), false);
   if (!exit || exit->empty()) return true;

   // Let's see if this is easy or complicated. Woo complicated!
   CFWidget::Ptr retcc = trace->cfWidget();
   assert(retcc);

   relocation_cerr << "Checking return statement " << (retcc->insn().isValid() ? retcc->insn().format() : "<no insn>") << ": "
		   << (retcc->isConditional() ? "<cond>" : "")
		   << (retcc->isIndirect() ? "<ind>" : "")
		   << endl;

   if (retcc->isConditional()) {
     // May either be indirect/fallthrough
     // or taken/not taken conditional
     if (trace->outs()->find(ParseAPI::INDIRECT))
       return handleCondIndExits(trace, cfg, exit);
     else
       return handleCondDirExits(trace, cfg, exit);
   }
   else {
     return handleUnconditionalExitInstrumentation(trace, cfg, exit);
   }
}

bool Instrumenter::handleUnconditionalExitInstrumentation(RelocBlock *trace, RelocGraph *, instPoint *exit) {
  // Easy
  RelocBlock::WidgetList &elements = trace->elements();
  // For now, we're inserting this instrumentation immediately before the last instruction
  // in the list of elements. 
  
  Widget::Ptr inst = makeInstrumentation(exit);
  if (!inst) return false;
  
  inst.swap(elements.back());
  elements.push_back(inst);
  return true;
}

bool Instrumenter::handleCondIndExits(RelocBlock *trace, RelocGraph *cfg, instPoint *exit) {
   // Let's see if this is easy or complicated. Woo complicated!
   CFWidget::Ptr retcc = trace->cfWidget();
   assert(retcc);

   relocation_cerr << "Function exit instrumentation /w/ conditional return!" << endl;
   // Complex!
   // Make the jcc
   CFWidget::Ptr jcc = CFWidget::create(retcc);
   jcc->clearIsIndirect(); // Will turn this into a straight conditional branch
   jcc->clearIsCall();
   assert(jcc->isConditional());
   trace->setCF(jcc);
   
   // And the new relocBlock
   RelocBlock *instRelocBlock = RelocBlock::createInst(exit, retcc->addr(), trace->block(), trace->func());
   instRelocBlock->setCF(retcc);
   cfg->addRelocBlockAfter(trace, instRelocBlock);
   
   // Skip this block when it comes up; we don't want to re-instrument it :)
   skip = trace;
   
   // And wire edges correctly. 
   // Lessee...
   // trace should have a batch of <INDIRECT> edges and a <COND_NOT_TAKEN> edge. The COND_NOT_TAKEN is fine,
   // but the INDIRECTs need to move to instRelocBlock.
   // So... move 
   Predicates::Type ind(ParseAPI::INDIRECT);
   if (!cfg->changeSources(ind, trace->outs(), instRelocBlock)) {
     relocation_cerr << "Failed to change sources of indirect edges from old block to new block!" << endl;
     return false;
   }
   
   // Also, instRelocBlock needs a <COND_NOT_TAKEN> to wherever trace's COND_NOT_TAKEN went. 
   RelocEdge *ft = trace->outs()->find(ParseAPI::FALLTHROUGH);  
   if (!ft) {
     relocation_cerr << "Error: did not find a FALLTHROUGH out-edge from current block!" << endl;
     return false;
   }  
   ft->type = ParseAPI::COND_NOT_TAKEN;
   
   // As a rule, we can't share Targets so that we can get deletion right later. So 
   // copy this puppy...
   
   cfg->makeEdge(new Target<RelocBlock *>(instRelocBlock),
		 ft->trg->copy(),
         NULL,
		 ft->type);
   // Finally, we need a COND_TAKEN from trace to instRelocBlock.
   cfg->makeEdge(new Target<RelocBlock *>(trace),
		 new Target<RelocBlock *>(instRelocBlock),
         NULL,
		 ParseAPI::COND_TAKEN);

   return true;
}

bool Instrumenter::handleCondDirExits(RelocBlock *trace, RelocGraph *cfg, instPoint *exit) {
   // Let's see if this is easy or complicated. Woo complicated!
   CFWidget::Ptr retcc = trace->cfWidget();
   assert(retcc);

   relocation_cerr << "Checking return statement " << (retcc->insn().isValid() ? retcc->insn().format() : "<no insn>") << ": "
		   << (retcc->isConditional() ? "<cond>" : "")
		   << (retcc->isIndirect() ? "<ind>" : "")
		   << endl;

   CFWidget::Ptr jcc = CFWidget::create(retcc);
   jcc->clearIsIndirect(); // Will turn this into a straight conditional branch
   jcc->clearIsCall();
   assert(jcc->isConditional());
   trace->setCF(jcc);
   
   // And the new relocBlock
   RelocBlock *instRelocBlock = RelocBlock::createInst(exit, retcc->addr(), trace->block(), trace->func());
   retcc->clearIsConditional();
   instRelocBlock->setCF(retcc);
   cfg->addRelocBlockAfter(trace, instRelocBlock);
   
   // Skip this block when it comes up; we don't want to re-instrument it :)
   skip = trace;
   
   // And wire edges correctly. 
   // Lessee...
   // The trace has a COND_TAKEN, which we want to redirect to the instrumentation block, and a COND_NOT_TAKEN,
   // which we leave alone. 

   // First, redirect the conditional taken edge _from_ trace _to_ instRelocBlock

   relocation_cerr << "Step 1:" << endl;
   relocation_cerr << trace->format();
   relocation_cerr << instRelocBlock->format() << endl;
   relocation_cerr << "----" << endl;

   RelocEdge *cond = trace->outs()->find(ParseAPI::COND_TAKEN);

   cfg->makeEdge(new Target<RelocBlock *>(instRelocBlock),
		 cond->trg->copy(),
         NULL, 
		 ParseAPI::DIRECT);

   cfg->makeEdge(new Target<RelocBlock *>(trace),
		 new Target<RelocBlock *>(instRelocBlock),
         NULL,
		 ParseAPI::COND_TAKEN);

   cfg->removeEdge(cond);

   relocation_cerr << "Step 3:" << endl;
   relocation_cerr << trace->format();
   relocation_cerr << instRelocBlock->format() << endl;
   relocation_cerr << "----" << endl;

   return true;
}

bool Instrumenter::blockEntryInstrumentation(RelocBlock *trace) {
   instPoint *entry = NULL;
   if (trace->func()) {
     entry = trace->func()->blockEntryPoint(trace->block(), false);
   }

   if (!entry || entry->empty()) return true;

   RelocBlock::WidgetList &elements = trace->elements();
   // Block entry instrumentation goes in before all instructions. 
   Widget::Ptr inst = makeInstrumentation(entry);
   if (!inst) return false;

   elements.push_front(inst);
   return true;
}

bool Instrumenter::blockExitInstrumentation(RelocBlock *trace) {
   instPoint *exit = NULL;
   if (trace->func()) {
     exit = trace->func()->blockExitPoint(trace->block(), false);
   }

   if (!exit || exit->empty()) return true;

   RelocBlock::WidgetList &elements = trace->elements();
   // Block exit instrumentation goes in after anything except a CFAtom
   // .. which means before the last instruction

   Widget::Ptr last;
   if (!elements.empty()) {
      last = elements.back();
      elements.pop_back();
   }
   Widget::Ptr inst = makeInstrumentation(exit);
   if (!inst) return false;

   elements.push_back(inst);
   if (last) elements.push_back(last);

   return true;
}

bool Instrumenter::postCallInstrumentation(RelocBlock *trace, RelocGraph *cfg) {
   // We want to insert instrumentation that will execute after the call returns
   // but before any other function code does. This can effectively be 
   // modeled as edge instrumentation, as follows:
   //
   // C -> PC goes to C -> Inst -> PC, with any other edges into PC left
   // unmodified. 
   // The way we do this is by redirecting the call fallthrough edge of
   // C (DEFENSIVE TODO) to a new RelocBlock. 
   instPoint *post = NULL;
   if (trace->func()) {
     post = trace->func()->postCallPoint(trace->block(), false);
   }

   if (!post || post->empty()) return true;
   relocation_cerr << "Adding post-call instrumentation to " << trace->id() << endl;

   Address postCallAddr = trace->block()->end();
   block_instance *FT = trace->block()->getFallthroughBlock();
   if (FT) postCallAddr = FT->start();
   else relocation_cerr << "Odd: post-call inst with no fallthrough block" << endl;

   RelocBlock *instRelocBlock = RelocBlock::createInst(post, postCallAddr, FT ? FT : trace->block(), trace->func()); 

   // Edge redirection time. The call fallthrough edge from trace needs
   // to be updated to point to instTarget instead, and instRelocBlock needs
   // a Fallthrough-typed edge to the previous target.
   // We always create a call fallthrough. Might
   // be sink typed, but we _always_ create a call fallthrough.

   cfg->addRelocBlockBefore(trace, instRelocBlock);

   Predicates::CallFallthrough pred;
   if (!cfg->interpose(pred, trace->outs(), instRelocBlock)) return false;

   return true;
}


bool Instrumenter::funcEntryInstrumentation(RelocBlock *trace, RelocGraph *cfg) {
   instPoint *entry = NULL;
   if (trace->func() &&
       trace->func()->entryBlock() == trace->block()) {
     entry = trace->func()->funcEntryPoint(false);
   }
   if (!entry || entry->empty()) return true;

   relocation_cerr << "Adding function entry at trace " << trace->id() << endl;

   // Create an instrumentation trace
   // Insert before the code trace
   // Redirect any interprocedural edges (and a springboard) from 
   // code trace to instrumentation trace
   // Leave intraprocedural edges untouched.

   RelocBlock *instRelocBlock = RelocBlock::createInst(entry,
                                        trace->origAddr(),
                                        trace->block(),
                                        trace->func());
   cfg->addRelocBlockBefore(trace, instRelocBlock);

   if (!cfg->makeEdge(new Target<RelocBlock *>(instRelocBlock),
                      new Target<RelocBlock *>(trace),
                      NULL,
                      ParseAPI::FALLTHROUGH)) return false;

   if (!cfg->setSpringboard(trace->block(), trace->func(), instRelocBlock)) return false;
   Predicates::Interprocedural pred;
   if (!cfg->changeTargets(pred, trace->ins(), instRelocBlock)) return false;

   return true;
}

bool Instrumenter::edgeInstrumentation(RelocBlock *trace, RelocGraph *cfg) {
   // Comparitively simple given the previous functions...
   assert(trace->type() == RelocBlock::Relocated);
   block_instance *block = trace->block();
   assert(block);
   const PatchBlock::edgelist &targets = block->targets();
   for (PatchBlock::edgelist::const_iterator iter = targets.begin();
        iter != targets.end(); ++iter) {
      instPoint *point = NULL;
      edge_instance* iedge = SCAST_EI(*iter);
      if (trace->func()) {
        // point = trace->func()->findPoint(instPoint::EdgeDuring, *iter, false);
        point = trace->func()->edgePoint(iedge, false);
      }
      if (!point || point->empty()) continue;

      RelocBlock *instRelocBlock = RelocBlock::createInst(point, iedge->trg()->start(), iedge->trg(), trace->func());

      cfg->addRelocBlockAfter(trace, instRelocBlock);
      skip = trace;

      Predicates::Edge pred(iedge);
      if (!cfg->interpose(pred, trace->outs(), instRelocBlock)) return false;
   }
   return true;
}

Widget::Ptr Instrumenter::makeInstrumentation(PatchAPI::Point *p) {
   instPoint *point = IPCONV(p);
   assert(!point->empty());

   InstWidget::Ptr inst = InstWidget::create(point);

   return inst;
}
