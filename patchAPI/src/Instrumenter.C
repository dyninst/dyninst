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
/* Plugin */

#include "Instrumenter.h"
#include "PatchCFG.h"

using Dyninst::PatchAPI::Instrumenter;
using Dyninst::PatchAPI::InstanceSet;
using Dyninst::PatchAPI::PatchFunction;
using Dyninst::PatchAPI::PatchBlock;
using Dyninst::PatchAPI::AddrSpace;

/* Default implementation of Instrumenter */

Instrumenter*
Instrumenter::create(AddrSpace* as) {
  Instrumenter* ret = new Instrumenter(as);
  return ret;
}

bool
Instrumenter::replaceFunction(PatchFunction* oldfunc, PatchFunction *newfunc) {
  functionReplacements_[oldfunc] = newfunc;
  return true;
}

bool
Instrumenter::revertReplacedFunction(PatchFunction* oldfunc) {
  functionReplacements_.erase(oldfunc);
  return true;
}

bool
Instrumenter::wrapFunction(PatchFunction* oldfunc, PatchFunction *newfunc, std::string name) {
   functionWraps_[oldfunc] = std::make_pair(newfunc, name);
   return true;
}

bool
Instrumenter::revertWrappedFunction(PatchFunction* oldfunc) {
  functionWraps_.erase(oldfunc);
  return true;
}

bool
Instrumenter::modifyCall(PatchBlock *callBlock, PatchFunction *newCallee, PatchFunction *context) {
  callModifications_[callBlock][context] = newCallee;
  return true;
}

bool
Instrumenter::revertModifiedCall(PatchBlock *callBlock, PatchFunction *context) {
  if (callModifications_.find(callBlock) != callModifications_.end()) {
    callModifications_[callBlock].erase(context);
  }
  return true;
}

bool
Instrumenter::removeCall(PatchBlock *callBlock, PatchFunction *context) {
  modifyCall(callBlock, NULL, context);
  return true;
}
