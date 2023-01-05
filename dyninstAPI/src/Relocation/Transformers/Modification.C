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
#include "Modification.h"
#include "dyninstAPI/src/debug.h"
#include "../CFG/RelocTarget.h"
#include "../Widgets/Widget.h"
#include "../Widgets/CFWidget.h"
#include "../Widgets/ASTWidget.h"
#include "../Widgets/InstWidget.h"
#include "../CFG/RelocGraph.h"
#include "dyninstAPI/src/instPoint.h"
#include "dyninstAPI/src/function.h"
#include "../Widgets/CallbackWidget.h"


using namespace std;
using namespace Dyninst;
using namespace Relocation;

Modification::Modification(const CallModMap &callMod,
                           const FuncModMap &funcRepl,
                           const FuncWrapMap &funcWraps) :
  callMods_(callMod),
  funcReps_(funcRepl),
  funcWraps_(funcWraps) {}

bool Modification::process(RelocBlock *cur, RelocGraph *cfg) {
  //relocation_cerr << "Modification transformer, processing block" << endl;
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
   std::map<PatchFunction*, PatchFunction*>::const_iterator iter2 = iter->second.find(trace->func());
   if (iter2 == iter->second.end()) return true;

   func_instance *repl = SCAST_FI(iter2->second);

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

   RelocBlock *target = cfg->find(repl->entryBlock(), repl);
   if (target) {
      if (!cfg->changeTargets(pred, trace->outs(), target)) return false;
   }
   else {
      if (!cfg->changeTargets(pred, trace->outs(), repl->entryBlock())) return false;
   }

   return true;
}

bool Modification::replaceFunction(RelocBlock *trace, RelocGraph *cfg) {
  // See if we were something created later
  if (trace->elements().empty()) return true;
  
   // See if we're the entry block
   if (trace->block() != trace->func()->entryBlock()) return true;

   FuncModMap::const_iterator iter = funcReps_.find(trace->func());
   if (iter == funcReps_.end()) return true;
   func_instance* oldfun = SCAST_FI(iter->first);
   PatchFunction* pnewfun = iter->second;
   //func_instance* newfun = SCAST_FI(iter->second);
   func_instance* newfun = SCAST_FI(pnewfun);

   relocation_cerr << "Performing function replacement in trace " << trace->id()
                   << " going to function " << newfun->name()
                   << " /w/ entry block "
                   << (newfun->entryBlock() ? newfun->entryBlock()->start() : -1) << endl;
   // Stub a jump to the replacement function
   RelocBlock *stub = makeRelocBlock(oldfun->entryBlock(),
                                     oldfun,
				     trace,
                                     cfg);
   RelocBlock *target = cfg->find(newfun->entryBlock(), newfun);
   if (target) {
      cfg->makeEdge(new Target<RelocBlock *>(stub),
                    new Target<RelocBlock *>(target),
                    NULL,
                    ParseAPI::DIRECT);
   }
   else {
      cfg->makeEdge(new Target<RelocBlock *>(stub),
                    new Target<block_instance *>(newfun->entryBlock()),
                    NULL,
                    ParseAPI::DIRECT);
   }

   // Redirect the springboard to the replacement function
   cfg->setSpringboard(trace->block(), trace->func(), stub);

   // Redirect all call in-edges to the replacement function
   Predicates::Interprocedural pred;
   if (target) {
      if (!cfg->changeTargets(pred, trace->ins(), target)) return false;
   }
   else {
      if (!cfg->changeTargets(pred, trace->ins(), newfun->entryBlock())) return false;
   }
   return true;
}

bool Modification::wrapFunction(RelocBlock *trace, RelocGraph *cfg) {
   // See if we're the entry block
   if (trace->block() != trace->func()->entryBlock()) return true;

   FuncWrapMap::const_iterator iter = funcWraps_.find(trace->func());
   if (iter == funcWraps_.end()) return true;
   
   // func_instance* oldfun = SCAST_FI(iter->first);
   func_instance* newfun = SCAST_FI(iter->second.first);
   std::string newname = iter->second.second;
   

   relocation_cerr << "Performing function wrapping in trace " << trace->id()
                   << " going to function " << newfun->name()
                   << " /w/ entry block "
                   << (newfun->entryBlock() ? newfun->entryBlock()->start() : -1) << endl;

   // This is a special case of replaceFunction; the predicate is "all calls except from the
   // wrapper are redirected".
   RelocBlock *stub = makeRelocBlock(newfun->entryBlock(),
                                     newfun,
				     trace,
                                     cfg);
   RelocBlock *target = cfg->find(newfun->entryBlock(), newfun);
   if (target) {
      relocation_cerr << "\t Also relocated new function, using target " << target->id() << endl;
      cfg->makeEdge(new Target<RelocBlock *>(stub),
                    new Target<RelocBlock *>(target),
                    NULL,
                    ParseAPI::DIRECT);
   }
   else {
      relocation_cerr << "\t New function " << newfun->name() << " not relocated targeting entry block " 
                      << hex << newfun->entryBlock()->start() << dec << " directly" << endl;
      cfg->makeEdge(new Target<RelocBlock *>(stub),
                    new Target<block_instance *>(newfun->entryBlock()),
                    NULL,
                    ParseAPI::DIRECT);
   }
   relocation_cerr << "Stub block is " << stub->format() << endl;
   
   cfg->setSpringboard(trace->block(), trace->func(), stub);

   WrapperPredicate pred(trace->func());
   if (target) {
      if (!cfg->changeTargets(pred, trace->ins(), target)) return false;
   }
   else {
      if (!cfg->changeTargets(pred, trace->ins(), newfun->entryBlock())) return false;
   }
   // We also need to track the "new" entry block so we can build a new symbol for it.
   CallbackWidget::Ptr c = CallbackWidget::create(new WrapperPatch(trace->func(), newname));
   trace->elements().push_front(c);

   return true;
}

RelocBlock *Modification::makeRelocBlock(block_instance *block, func_instance *func, RelocBlock *trace, RelocGraph *cfg) {
   RelocBlock *t = RelocBlock::createStub(block, func);

   // Current, new. 
   cfg->addRelocBlockBefore(trace, t);
   return t;
}

// TODO: make this mildly more efficient. On the other hand, is it a big deal?
Modification::WrapperPredicate::WrapperPredicate(func_instance *f)
   : f_(f) {}


bool Modification::WrapperPredicate::operator()(RelocEdge *e) {
   if (e->src->type() != TargetInt::RelocBlockTarget) return false;
   RelocBlock *t = static_cast<Target<RelocBlock *> *>(e->src)->t();
   return t->func() == f_;
}

bool Modification::WrapperPatch::apply(codeGen &gen, CodeBuffer *) {
   // Tell our function to create a wrapper symbol at this address
   func_->createWrapperSymbol(gen.currAddr(), name_);
   return true;
}

