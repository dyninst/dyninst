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

using namespace std;
using namespace Dyninst;
using namespace Relocation;
using namespace InstructionAPI;


bool Instrumenter::processTrace(TraceList::iterator &iter) {
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

   // Let's rack up which instPoints exist for us. 
   Trace::Ptr trace = *iter;
   relocation_cerr << "Instrumenter called on trace " << trace->id() << endl;
   if (!trace->block()) {
      relocation_cerr << "\tNo block, assuming inst, skipping" << endl;
      return true; // An inst block or something
   }
   


   // Let's run with non-block-inserting first...
   // This should be block inserting, but for now we're claiming just-before-return. 
   // Also, we tend to use a "highest container gets executed first" model,
   // which means we instrument the _lowest_ levels first. 
   if (!insnInstrumentation(trace)) return false;
   if (!preCallInstrumentation(trace)) return false;
   if (!blockEntryInstrumentation(trace)) return false;
   if (!funcExitInstrumentation(trace)) return false;

   // And on to the graph modification shtuff. 
   if (!postCallInstrumentation(trace)) return false;
   if (!edgeInstrumentation(trace)) return false;
   if (!funcEntryInstrumentation(trace)) return false;

   return true;
}


bool Instrumenter::insnInstrumentation(Trace::Ptr trace) {
   // We're going to unify pre- and post- instruction instrumentation
   // into a single pass for efficiency. 
   const std::map<Address, instPoint *> &prePoints = trace->block()->findPoints(instPoint::PreInsn);
   const std::map<Address, instPoint *> &postPoints = trace->block()->findPoints(instPoint::PostInsn);
   
   std::map<Address, instPoint *>::const_iterator pre = prePoints.begin();
   std::map<Address, instPoint *>::const_iterator post = postPoints.begin();
   
   Trace::AtomList::iterator elem = trace->elements().begin();

   while ((pre != prePoints.end()) ||
          (post != postPoints.end())) {
      Address preAddr = 0;
      if (pre != prePoints.end()) {
         preAddr = pre->first;
      }
      Address postAddr = 0;
      if (post != postPoints.end()) {
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

bool Instrumenter::preCallInstrumentation(Trace::Ptr trace) {
   instPoint *call = trace->func()->findPoint(instPoint::PreCall, trace->block());
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

bool Instrumenter::funcExitInstrumentation(Trace::Ptr trace) {
   // TODO: do this right :)
   instPoint *exit = trace->func()->findPoint(instPoint::FunctionExit, trace->block());
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

bool Instrumenter::blockEntryInstrumentation(Trace::Ptr trace) {
   instPoint *entry = trace->block()->findPoint(instPoint::BlockEntry);
   if (!entry || entry->empty()) return true;

   Trace::AtomList &elements = trace->elements();
   // Block entry instrumentation goes in before all instructions. 
   Atom::Ptr inst = makeInstrumentation(entry);
   if (!inst) return false;

   elements.push_front(inst);
   return true;
}

bool Instrumenter::postCallInstrumentation(Trace::Ptr trace) {
   // We want to insert instrumentation that will execute after the call returns
   // but before any other function code does. This can effectively be 
   // modeled as edge instrumentation, as follows:
   //
   // C -> PC goes to C -> Inst -> PC, with any other edges into PC left
   // unmodified. 
   // The way we do this is by redirecting the call fallthrough edge of
   // C (DEFENSIVE TODO) to a new Trace. 
   instPoint *post = trace->block()->findPoint(instPoint::PostCall);
   if (!post || post->empty()) return true;
   
   Atom::Ptr inst = makeInstrumentation(post);

   Address postCallAddr = trace->block()->end();
   int_block *FT = trace->block()->getFallthrough();
   if (FT) postCallAddr = FT->start();

   Trace::Ptr instTrace = Trace::create(inst, postCallAddr, FT ? FT : trace->block()); 

   // Edge redirection time. The call fallthrough edge from trace needs
   // to be updated to point to instTarget instead, and instTrace needs
   // a Fallthrough-typed edge to the previous target.
   assert(trace->getTargets(ParseAPI::CALL_FT).size() == 1);
   // We always create a call fallthrough. Might
   // be sink typed, but we _always_ create a call fallthrough.
   
   // Magic function that does the insertion
   if (!trace->interposeTarget(ParseAPI::CALL_FT, instTrace)) return false;

   edgeTraces_[std::make_pair(trace, After)].push_back(instTrace);

   return true;
}


bool Instrumenter::funcEntryInstrumentation(Trace::Ptr trace) {
   relocation_cerr << "\tFuncEntryInst for trace " << trace->id() << endl;
   instPoint *entry = trace->func()->findPoint(instPoint::FunctionEntry, 
                                               trace->block());
   if (!entry || entry->empty()) return true;

   // Transformation time. We have an entry block E with two types
   // of incoming edges; interprocedural (call) and intraprocedural.
   // We want to create a new trace, I, and redirect all interprocedural
   // edges from E to I. 
   // Problem is, we have to consider non-relocated code as well. We
   // reach the entry points of functions via springboards from a
   // block's original location to its relocated location. However, we
   // want to redirect that springboard to the instrumentation block.
   // Rather than having to track everywhere that a trace may possibly
   // have been added, we're going to replace the element list in the
   // entry trace (E) with instrumentation, and create a new E' that 
   // contains original code. This is kind of roundabout, but...
   // yeah. 
   relocation_cerr << "\t\tDoing work" << endl;
   Trace::Ptr newTrace = trace->split(trace->elements().begin());

   // Can't assert this as there is probably a CFAtom in the old
   // trace, but leaving for reference
   //assert(trace->elements().empty());
   trace->setAsInstrumentationTrace();

   Atom::Ptr inst = makeInstrumentation(entry);
   trace->elements().push_front(inst);
   
   // Okay, we just split the trace in half. We still need to redirect 
   // edges.
   trace->moveSources(ParseAPI::COND_TAKEN, newTrace);
   trace->moveSources(ParseAPI::COND_NOT_TAKEN, newTrace);
   trace->moveSources(ParseAPI::INDIRECT, newTrace);
   trace->moveSources(ParseAPI::DIRECT, newTrace);
   trace->moveSources(ParseAPI::FALLTHROUGH, newTrace);
   trace->moveSources(ParseAPI::CALL_FT, newTrace);
   trace->moveSources(ParseAPI::CATCH, newTrace);
   trace->moveSources(ParseAPI::RET, newTrace);
   
   edgeTraces_[std::make_pair(trace, After)].push_back(newTrace);
   return true;
}

bool Instrumenter::edgeInstrumentation(Trace::Ptr trace) {
   // Comparitively simple given the previous functions...
   int_block *block = trace->block();
   if (!block) {
      cerr << "WTF: " << trace->id() << endl;
   }
   assert(block);
   const int_block::edgelist &targets = block->targets();
   for (int_block::edgelist::iterator iter = targets.begin();
        iter != targets.end(); ++iter) {
      instPoint *point = (*iter)->findPoint(instPoint::Edge);
      if (!point || point->empty()) continue;

      Atom::Ptr inst = makeInstrumentation(point);
      Trace::Ptr instTrace = Trace::create(inst, (*iter)->trg()->start(), (*iter)->trg());

      if (!trace->interposeTarget(*iter, instTrace)) return false;

      // If the edge's target is also a trace, put us in before them. 
      // If not, add us after the current block.
      TraceMap::const_iterator oldTarget = traceMap_.find((*iter)->trg());
      if (oldTarget != traceMap_.end()) {
         edgeTraces_[std::make_pair(oldTarget->second, Before)].push_back(instTrace);
      }
      else {
         edgeTraces_[std::make_pair(trace, After)].push_back(instTrace);
      }
   }
   return true;
}

Atom::Ptr Instrumenter::makeInstrumentation(instPoint *point) {
   assert(!point->empty());

   InstAtom::Ptr inst = InstAtom::create(point);

   return inst;
}

bool Instrumenter::postprocess(TraceList &l) {
   relocation_cerr << "Instrumenter::postprocess, " << edgeTraces_.size() << " new Traces to add" << endl;
   if (edgeTraces_.empty()) return true;

   for (TraceList::iterator iter = l.begin();
        iter != l.end(); ++iter) {
      EdgeTraces::iterator pre = edgeTraces_.find(std::make_pair(*iter, Before));
      if (pre != edgeTraces_.end()) {
         relocation_cerr << "\tAdding " << pre->second.size() << " traces before " << (*iter)->id() << endl;
         l.insert(iter, pre->second.begin(), pre->second.end());
      }
      
      EdgeTraces::iterator post = edgeTraces_.find(std::make_pair(*iter, After));
      if (post != edgeTraces_.end()) {
         TraceList::iterator iter2 = iter;
         ++iter2;
         relocation_cerr << "\tAdding " << post->second.size() << " traces after " << (*iter)->id() << endl;
         l.insert(iter2, post->second.begin(), post->second.end());
      }
   }
   return true;
}
