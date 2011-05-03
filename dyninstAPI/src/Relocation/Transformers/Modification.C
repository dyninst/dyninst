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
#include "Modification.h"
#include "../patchapi_debug.h"
#include "../Atoms/Target.h"
#include "../Atoms/Atom.h"
#include "../Atoms/CFAtom.h"
#include "../Atoms/ASTAtom.h"
#include "../Atoms/InstAtom.h"
#include "dyninstAPI/src/instPoint.h"
#include "dyninstAPI/src/function.h"

using namespace std;
using namespace Dyninst;
using namespace Relocation;

Modification::Modification(const CallModMap &callMod,
			   const FuncModMap &funcRepl,
			   const FuncModMap &funcWraps) :
  callMods_(callMod), 
  funcReps_(funcRepl),
  funcWraps_(funcWraps) {};

bool Modification::processTrace(TraceList &blocks, const TraceMap &traceMap) {
  // We define three types of program modification:
  // 1) Function call replacement; change the target of the corresponding
  //    call element
  // 2) Function call removal; modify the CFelement to have only a
  //    fallthrough edge
  // 3) Function replacement; TODO

   for (TraceList::iterator iter = blocks.begin(); iter != blocks.end(); ++iter) {
      Trace::Ptr trace = *iter;
      
      replaceCall(trace, traceMap); 
      replaceFunction(trace, traceMap);
      Trace::Ptr ret = wrapFunction(trace, traceMap);
      if (ret) {
         TraceList::iterator tmp = iter;
         ++tmp;
         blocks.insert(tmp, ret);
         // Skip this one
         ++iter;
      }
   }
   return true;
}

void Modification::replaceCall(Trace::Ptr trace, const TraceMap &traceMap) {
   // See if we have a modification for this point
   CallModMap::const_iterator iter = callMods_.find(trace->block());
   if (iter == callMods_.end()) return;
   std::map<func_instance *, func_instance *>::const_iterator iter2 = iter->second.find(trace->func());
   if (iter2 == iter->second.end()) return;

   func_instance *repl = iter2->second;
   
   // Replace the call at the end of this trace /w/ repl (if non-NULL),
   // or elide completely (if NULL)
   // We do this via edge twiddling in the Trace
   
   if (!repl) {
      trace->removeTargets(ParseAPI::CALL);
      return;
   }
   Trace::Targets &targets = trace->getTargets(ParseAPI::CALL);
   for (Trace::Targets::iterator d_iter = targets.begin();
        d_iter != targets.end(); ++d_iter) {
      delete *d_iter;
   }
   targets.clear();
   
   block_instance *entry = repl->entryBlock();
   assert(entry);

   targets.push_back(getTarget(entry, traceMap));
}

void Modification::replaceFunction(Trace::Ptr trace, const TraceMap &traceMap) {
   // See if we're the entry block
   if (trace->block() != trace->func()->entryBlock()) return;

   FuncModMap::const_iterator iter = funcReps_.find(trace->func());
   if (iter == funcReps_.end()) return;

   relocation_cerr << "Performing function replacement in trace " << trace->id() 
                   << " going to function " << iter->second->name() 
                   << " /w/ entry block " 
                   << (iter->second->entryBlock() ? iter->second->entryBlock()->start() : -1) << endl;
   // Okay, time to do work. 
   // Just update the out-edges (removing everything except
   // a... fallthrough, why not... to the replacement function)
   trace->removeTargets();
   trace->getTargets(ParseAPI::FALLTHROUGH).push_back(getTarget(iter->second->entryBlock(), traceMap));

   // And erase anything in the trace to be sure we immediately jump.
   // Amusingly? Entry instrumentation of the function will still execute...
   // Need to determine semantics of this and FIXME TODO
   trace->elements().clear();

   CFAtom::Ptr newCF = CFAtom::create(trace->block()->start());
   trace->elements().push_back(newCF);
   trace->cfAtom() = newCF;
}

Trace::Ptr Modification::wrapFunction(Trace::Ptr trace, const TraceMap &traceMap) {
   // See if we're the entry block
   if (trace->block() != trace->func()->entryBlock()) return Trace::Ptr();

   FuncModMap::const_iterator iter = funcWraps_.find(trace->func());
   if (iter == funcWraps_.end()) return Trace::Ptr();

   relocation_cerr << "Performing function wrapping in trace " << trace->id() 
                   << " going to function " << iter->second->name() 
                   << " /w/ entry block " 
                   << (iter->second->entryBlock() ? iter->second->entryBlock()->start() : -1) << endl;
   cerr << "Step 1" << endl;
   // Create a placeholder at the start of the old function that redirects execution
   // to the wrapper instead
   Trace::Ptr newTrace = trace->split(trace->elements().begin());
   cerr << trace->format() << endl;
   cerr << newTrace->format() << endl;

   cerr << "Step 2" << endl;
   trace->removeTargets();

   cerr << trace->format() << endl;
   cerr << newTrace->format() << endl;
   cerr << "Step 3" << endl;

   trace->getTargets(ParseAPI::FALLTHROUGH).push_back(getTarget(iter->second->entryBlock(), traceMap));

   // Go through the wrapper and redirect all of its edges to the wrappee (that is, to trace)
   // with edges to newtrace

   const func_instance::BlockSet &wrapperBlocks = iter->second->blocks();
   for (func_instance::BlockSet::const_iterator iter = wrapperBlocks.begin();
        iter != wrapperBlocks.end(); ++iter) {
      TraceMap::const_iterator tmp = traceMap.find(*iter);
      assert(tmp != traceMap.end());
      cerr << "Replacing in" << endl;
      cerr << tmp->second->format() << endl;
      tmp->second->replaceTarget(trace, newTrace);
      cerr << tmp->second->format() << endl;
   }
   return newTrace;
}

TargetInt *Modification::getTarget(block_instance *block, const TraceMap &traceMap) {
   // See if there's a reloc copy
   TraceMap::const_iterator t_iter = traceMap.find(block);
   if (t_iter != traceMap.end()) {
      return new Target<Trace *>(t_iter->second.get());
   }
   else {
      return new Target<block_instance *>(block);
   }
}
