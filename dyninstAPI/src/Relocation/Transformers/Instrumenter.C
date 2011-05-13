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



#include "Transformer.h"
#include "Instrumenter.h"
#include "../patchapi_debug.h"
#include "../Atoms/Atom.h"
#include "../Atoms/Target.h"
#include "dyninstAPI/src/instPoint.h"
#include "dyninstAPI/src/baseTramp.h"
#include "../Atoms/InstAtom.h"
#include "../Atoms/CFAtom.h"
#include "../RelocGraph.h"

using namespace std;
using namespace Dyninst;
using namespace Relocation;
using namespace InstructionAPI;

bool Instrumenter::process(Trace *trace,
                           RelocGraph *cfg) {
   assert(trace);
   if (trace->type() != Trace::Relocated) {
      // One of ours!
      return true;
   }

   // Hoo boy. Fun for the whole family...
   //
   // This transformer needs to create instrumentation blocks throughout the 
   // provided CFG (of Traces). We consider two classes of added
   // instrumentation:
   // Block insertion instrumentation (function entry, edge, post-call);
   // Block augmentation instrumentation (block entry, pre/post insn, pre-call).
   //
   // Block insertion instrumentation adds a new block (Trace) to the CFG
   // that contains the instrumentation code. We will need to edit edges
   // to get this to work. 
   //
   // Block augmentation instrumentation just inserts a new Atom into the trace;
   // this is comparitively trivial. 

   // Let's run with non-block-inserting first...
   // This should be block inserting, but for now we're claiming just-before-return. 
   // Also, we tend to use a "highest container gets executed first" model,
   // which means we instrument the _lowest_ levels first. 
   if (!insnInstrumentation(trace)) return false;
   if (!preCallInstrumentation(trace)) return false;
   if (!blockEntryInstrumentation(trace)) return false;
   if (!funcExitInstrumentation(trace)) return false;

   // And on to the graph modification shtuff. 
   if (!postCallInstrumentation(trace, cfg)) return false;
   if (!edgeInstrumentation(trace, cfg)) return false;
   if (!funcEntryInstrumentation(trace, cfg)) return false;
   return true;
}

bool Instrumenter::insnInstrumentation(Trace *trace) {
   // We're going to unify pre- and post- instruction instrumentation
   // into a single pass for efficiency. 
   InsnInstpoints::const_iterator pre;
   InsnInstpoints::const_iterator preEnd;
   InsnInstpoints::const_iterator post;
   InsnInstpoints::const_iterator postEnd;

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
   
   Trace::AtomList::iterator elem = trace->elements().begin();

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
         assert(elem != trace->elements().end());
      }

      if (preAddr == (*elem)->addr()) {
         if (!pre->second->empty()) {
            Atom::Ptr inst = makeInstrumentation(pre->second);
            if (!inst) return false;
            trace->elements().insert(elem, inst);
         }
         ++pre;
      }
      if (postAddr == (*elem)->addr()) {
         if (!post->second->empty()) {
            Trace::AtomList::iterator tmp = elem;
            ++tmp;
            Atom::Ptr inst = makeInstrumentation(post->second);
            if (!inst) return false;
            trace->elements().insert(tmp, inst);
         }
         ++post;
      }
   }

   return true;
}

bool Instrumenter::preCallInstrumentation(Trace *trace) {
   instPoint *call = NULL;
   if (trace->func()) {
      call = trace->func()->findPoint(instPoint::PreCall, trace->block(), false);
   }
   if (!call || call->empty()) return true;

   Trace::AtomList &elements = trace->elements();
   // For now, we're inserting this instrumentation immediately before the last instruction
   // in the list of elements. 
   Atom::Ptr inst = makeInstrumentation(call);
   if (!inst) return false;

   inst.swap(elements.back());
   elements.push_back(inst);

   return true;
}

bool Instrumenter::funcExitInstrumentation(Trace *trace) {
   // TODO: do this right :)
   instPoint *exit = trace->func()->findPoint(instPoint::FuncExit, trace->block(), false);
   if (!exit || exit->empty()) return true;

   Trace::AtomList &elements = trace->elements();
   // For now, we're inserting this instrumentation immediately before the last instruction
   // in the list of elements. 
   Trace::AtomList::reverse_iterator riter = elements.rbegin();
   Atom::Ptr inst = makeInstrumentation(exit);
   if (!inst) return false;

   inst.swap(elements.back());
   elements.push_back(inst);

   return true;
}

bool Instrumenter::blockEntryInstrumentation(Trace *trace) {
   instPoint *entry = NULL;
   if (trace->func()) {
      entry = trace->func()->findPoint(instPoint::BlockEntry, trace->block(), false);
   }

   if (!entry || entry->empty()) return true;

   Trace::AtomList &elements = trace->elements();
   // Block entry instrumentation goes in before all instructions. 
   Atom::Ptr inst = makeInstrumentation(entry);
   if (!inst) return false;

   elements.push_front(inst);
   return true;
}

bool Instrumenter::postCallInstrumentation(Trace *trace, RelocGraph *cfg) {
   // We want to insert instrumentation that will execute after the call returns
   // but before any other function code does. This can effectively be 
   // modeled as edge instrumentation, as follows:
   //
   // C -> PC goes to C -> Inst -> PC, with any other edges into PC left
   // unmodified. 
   // The way we do this is by redirecting the call fallthrough edge of
   // C (DEFENSIVE TODO) to a new Trace. 
   instPoint *post = NULL;
   if (trace->func()) {
      post = trace->func()->findPoint(instPoint::PostCall, trace->block(), false);
   }

   if (!post || post->empty()) return true;
   relocation_cerr << "Adding post-call instrumentation to " << trace->id() << endl;

   Address postCallAddr = trace->block()->end();
   block_instance *FT = trace->block()->getFallthroughBlock();
   if (FT) postCallAddr = FT->start();
   else relocation_cerr << "Odd: post-call inst with no fallthrough block" << endl;

   Trace *instTrace = Trace::createInst(post, postCallAddr, FT ? FT : trace->block(), trace->func()); 

   // Edge redirection time. The call fallthrough edge from trace needs
   // to be updated to point to instTarget instead, and instTrace needs
   // a Fallthrough-typed edge to the previous target.
   // We always create a call fallthrough. Might
   // be sink typed, but we _always_ create a call fallthrough.

   cfg->addTraceBefore(trace, instTrace);

   Predicates::CallFallthrough pred;
   if (!cfg->interpose(pred, trace->outs(), instTrace)) return false;

   return true;
}


bool Instrumenter::funcEntryInstrumentation(Trace *trace, RelocGraph *cfg) {
   instPoint *entry = NULL;
   if (trace->func() &&
       trace->func()->entryBlock() == trace->block()) {
      entry = trace->func()->findPoint(instPoint::FuncEntry, false);
   }
   if (!entry || entry->empty()) return true;

   relocation_cerr << "Adding function entry at trace " << trace->id() << endl;

   // Create an instrumentation trace
   // Insert before the code trace
   // Redirect any interprocedural edges (and a springboard) from 
   // code trace to instrumentation trace
   // Leave intraprocedural edges untouched.

   Trace *instTrace = Trace::createInst(entry,
                                        trace->origAddr(),
                                        trace->block(),
                                        trace->func());
   cfg->addTraceBefore(trace, instTrace);

   if (!cfg->makeEdge(new Target<Trace *>(instTrace),
                      new Target<Trace *>(trace),
                      ParseAPI::FALLTHROUGH)) return false;

   if (!cfg->setSpringboard(trace->block(), instTrace)) return false;
   Predicates::Interprocedural pred;
   if (!cfg->changeTargets(pred, trace->ins(), instTrace)) return false;

   return true;
}

bool Instrumenter::edgeInstrumentation(Trace *trace, RelocGraph *cfg) {
   // Comparitively simple given the previous functions...
   assert(trace->type() == Trace::Relocated);
   block_instance *block = trace->block();
   assert(block);
   const block_instance::edgelist &targets = block->targets();
   for (block_instance::edgelist::const_iterator iter = targets.begin();
        iter != targets.end(); ++iter) {
      instPoint *point = NULL;
      if (trace->func()) {
         point = trace->func()->findPoint(instPoint::Edge, *iter, false);
      }
      if (!point || point->empty()) continue;

      Trace *instTrace = Trace::createInst(point, (*iter)->trg()->start(), (*iter)->trg(), trace->func());
      cfg->addTraceAfter(trace, instTrace);

      Predicates::Edge pred(*iter);
      if (!cfg->interpose(pred, trace->outs(), instTrace)) return false;
   }
   return true;
}

Atom::Ptr Instrumenter::makeInstrumentation(instPoint *point) {
   assert(!point->empty());

   InstAtom::Ptr inst = InstAtom::create(point);

   return inst;
}
