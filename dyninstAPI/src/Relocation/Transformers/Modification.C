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
#include "../CFG/RelocTarget.h"
#include "../Widgets/Widget.h"
#include "../Widgets/CFWidget.h"
#include "../Widgets/ASTWidget.h"
#include "../Widgets/InstWidget.h"
#include "../CFG/RelocGraph.h"
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

bool Modification::process(RelocBlock *cur, RelocGraph *cfg) {
  // We define three types of program modification:
  // 1) Function call replacement; change the target of the corresponding
  //    call element
  // 2) Function call removal; modify the CFelement to have only a
  //    fallthrough edge
  // 3) Function replacement; TODO

   if (!replaceCall(cur, cfg)) return false;
   if (!replaceFunction(cur, cfg)) return false;
   if (!wrapFunction(cur, cfg)) return false;

   return true;
}

bool Modification::replaceCall(RelocBlock *trace, RelocGraph *cfg) {
   // See if we have a modification for this point
   CallModMap::const_iterator iter = callMods_.find(trace->block());
   if (iter == callMods_.end()) return true;
   std::map<func_instance *, func_instance *>::const_iterator iter2 = iter->second.find(trace->func());
   if (iter2 == iter->second.end()) return true;

   func_instance *repl = iter2->second;

   relocation_cerr << "Replacing call in trace " 
                   << trace->id() << " with call to "
                   << (repl ? repl->name() : "<NULL>")
                   << ", " << hex 
                   << (repl ? repl->addr() : 0)
                   << dec << endl;
      
   
   // Replace the call at the end of this trace /w/ repl (if non-NULL),
   // or elide completely (if NULL)
   // We do this via edge twiddling in the RelocBlock
   
   Predicates::Type pred(ParseAPI::CALL);

   if (!repl) {
      if (!cfg->removeEdge(pred, trace->outs())) return false;
      return true;
   }
   
   RelocBlock *target = cfg->find(repl->entryBlock());
   if (target) {
      if (!cfg->changeTargets(pred, trace->outs(), target)) return false;
   }
   else {
      if (!cfg->changeTargets(pred, trace->outs(), repl->entryBlock())) return false;
   }

   return true;
}

bool Modification::replaceFunction(RelocBlock *trace, RelocGraph *cfg) {
   // See if we're the entry block
   if (trace->block() != trace->func()->entryBlock()) return true;

   FuncModMap::const_iterator iter = funcReps_.find(trace->func());
   if (iter == funcReps_.end()) return true;

   relocation_cerr << "Performing function replacement in trace " << trace->id() 
                   << " going to function " << iter->second->name() 
                   << " /w/ entry block " 
                   << (iter->second->entryBlock() ? iter->second->entryBlock()->start() : -1) << endl;
   // Stub a jump to the replacement function
   RelocBlock *stub = makeRelocBlock(iter->second->entryBlock(), 
                           iter->second,
                           cfg);
   RelocBlock *target = cfg->find(iter->second->entryBlock());
   if (target) {
      cfg->makeEdge(new Target<RelocBlock *>(stub),
                    new Target<RelocBlock *>(target),
                    ParseAPI::DIRECT);
   }
   else {
      cfg->makeEdge(new Target<RelocBlock *>(stub),
                    new Target<block_instance *>(iter->second->entryBlock()),
                    ParseAPI::DIRECT);
   }

   // Redirect the springboard to the replacement function
   cfg->setSpringboard(trace->block(), stub);
   
   // Redirect all call in-edges to the replacement function
   Predicates::Interprocedural pred;
   if (target) {
      if (!cfg->changeTargets(pred, trace->ins(), target)) return false;
   }
   else {
      if (!cfg->changeTargets(pred, trace->ins(), iter->second->entryBlock())) return false;
   }
   return true;
}

bool Modification::wrapFunction(RelocBlock *trace, RelocGraph *cfg) {
   // See if we're the entry block
   if (trace->block() != trace->func()->entryBlock()) return true;

   FuncModMap::const_iterator iter = funcWraps_.find(trace->func());
   if (iter == funcWraps_.end()) return true;

   relocation_cerr << "Performing function wrapping in trace " << trace->id() 
                   << " going to function " << iter->second->name() 
                   << " /w/ entry block " 
                   << (iter->second->entryBlock() ? iter->second->entryBlock()->start() : -1) << endl;

   // This is a special case of replaceFunction; the predicate is "all calls except from the 
   // wrapper are redirected". 
   RelocBlock *stub = makeRelocBlock(iter->second->entryBlock(), 
                           iter->second,
                           cfg);
   RelocBlock *target = cfg->find(iter->second->entryBlock());
   if (target) {
      cfg->makeEdge(new Target<RelocBlock *>(stub),
                    new Target<RelocBlock *>(target),
                    ParseAPI::DIRECT);
   }
   else {
      cfg->makeEdge(new Target<RelocBlock *>(stub),
                    new Target<block_instance *>(iter->second->entryBlock()),
                    ParseAPI::DIRECT);
   }
                 

   // TODO: have a more expressive representation of the wrapped function
   cfg->setSpringboard(trace->block(), stub);
   
   WrapperPredicate pred(trace->func());
   if (target) {
      if (!cfg->changeTargets(pred, trace->ins(), target)) return false;
   }
   else {
      if (!cfg->changeTargets(pred, trace->ins(), iter->second->entryBlock())) return false;
   }
   return true;
}

RelocBlock *Modification::makeRelocBlock(block_instance *block, func_instance *func, RelocGraph *cfg) {
   RelocBlock *t = cfg->find(block);
   if (t) return t;

   // Otherwise we need to make a stub RelocBlock that jumps to this function; 
   // this is annoying, but necessary. 
   
   t = RelocBlock::createStub(block, func);
   // Put it at the end, why not.
   cfg->addRelocBlock(t);
   return t;
}

// TODO: make this mildly more efficient. On the other hand, is it a big deal?
Modification::WrapperPredicate::WrapperPredicate(func_instance *f) 
   : f_(f) {};


bool Modification::WrapperPredicate::operator()(RelocEdge *e) {
   if (e->src->type() != TargetInt::RelocBlockTarget) return false;
   RelocBlock *t = static_cast<Target<RelocBlock *> *>(e->src)->t();
   return t->func() == f_;
}
