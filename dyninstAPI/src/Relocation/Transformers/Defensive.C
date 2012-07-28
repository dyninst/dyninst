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
#include "Defensive.h"
#include "dyninstAPI/src/debug.h"
#include "../Atoms/Atom.h"
#include "dyninstAPI/src/function.h"
#include "../Atoms/DefensivePadding.h"
#include "../Atoms/CFAtom.h"
#include "../Atoms/Target.h"

using namespace std;
using namespace Dyninst;
using namespace Relocation;
using namespace InstructionAPI;

// This transformer supports Kevin's run-time parsing of binaries. Specifically,
// it handles the case of calls that we do not have a return status for. When we
// see such a call we append a block of null operations (a pad). If the call returns,
// we use this block to insert a jump to the newly parsed code.
//
// Logically, this code exists in the CFG between the call return and the newly parsed
// code. Since we are inserting a branch to that new code, it is more tightly tied to
// its successor than the call. In addition, it has to place nicely with things like
// post-call instrumentation. Therefore, we are implementing this as a CFG transformation;
// the padding consists of a new Trace that is added between the call trace C and the new trace
// N. 

bool DefensiveTransformer::processTrace(TraceList::iterator &iter) {
  relocation_cerr << "DefensiveMode, processing block " 
		  << std::hex << (*iter)->origAddr() << std::dec << endl;
  int_block *bbl = (*iter)->bbl();
  if (!bbl) return true;

  if (!requiresDefensivePad(bbl)) {
      return true;
  }

  // Create a new Trace for this, and add
  // a TODO to insert it during postprocess
  DefensivePadding::Ptr pad = DefensivePadding::create(bbl);
  TracePtr trace = Trace::create(pad, bbl->end(), bbl->func());

  // Add it as a fallthrough from the prior block, and if the block had a fallthrough
  // reassign it to us.

  // 1) Fallthrough for old block.
  // 1a) Get CF atom for the block
  CFAtom::Ptr cf = boost::dynamic_pointer_cast<CFAtom>((*iter)->elements().back());
  assert(cf);
  // 1b) Cache old fallthrough if it exists
  TargetInt *oldFallthrough = cf->getDestination(CFAtom::Fallthrough);
  // 1c) Set us to be fallthrough
  Target<Trace::Ptr> *t = new Target<Trace::Ptr>(trace);
  cf->addDestination(CFAtom::Fallthrough, t);

  // 2) Fallthrough for us
  CFAtom::Ptr newCFAtom = CFAtom::create(bbl);
  newCFAtom->updateAddr(bbl->end());
  trace->elements().push_back(newCFAtom);
  if (oldFallthrough) {
      newCFAtom->addDestination(CFAtom::Fallthrough, oldFallthrough);
  }

  defensivePads_[*iter] = trace;

  return true;
}

bool DefensiveTransformer::postprocess(TraceList &l) {
    for (TraceList::iterator iter = l.begin();
        iter != l.end(); ++iter)
    {
        InsertionMap::iterator foo = defensivePads_.find(*iter);
        if (foo != defensivePads_.end()) {
            // We want the new one _after_ the current location.
            if (iter == l.end()) {
                // Oddd....
                l.push_back(foo->second);
            }
            else {
                ++iter;
                l.insert(iter, foo->second);
                --iter;
            }
        }
    }
    return true;
}
// First parameter: do we need a defensive 
bool DefensiveTransformer::requiresDefensivePad(const int_block *block) {
   // Find if the program does anything funky with a call fallthrough
   // 1) A call edge with no fallthrough
   // 2) A gap between the call block and the fallthrough block.
   
   ParseAPI::Edge *callEdge = NULL;
   ParseAPI::Edge *ftEdge = NULL;
   
   const ParseAPI::Block::edgelist &targets = block->llb()->targets();
   ParseAPI::Block::edgelist::iterator iter = targets.begin();
   for (; iter != targets.end(); ++iter) {
      if ((*iter)->type() == ParseAPI::CALL) {
         callEdge = *iter;
      }
      if ((*iter)->type() == ParseAPI::CALL_FT) {
         ftEdge = *iter;
      }
   }

   if (callEdge && !ftEdge) {
       malware_cerr << "Found call edge w/o fallthrough, block @ " 
           << hex << block->start() << " gets defensive pad " << dec << endl;
       return true;
   }
   else if (callEdge && ftEdge) {
       return false;
   }


   // See big comment in ControlFlow.C ; we're omitting edges from the ParseAPI
   // and that makes things go badly.

   using namespace InstructionAPI;

   int_block::InsnInstances insns;
   block->getInsnInstances(insns);

   // Hack: this also triggers on call-next thunks; only go if we have _no_
   // targets.
   if ((insns.back().first->getCategory() == c_CallInsn) &&
       (targets.empty())) 
   {
       cerr << "Hacky defensive pad for block @ " << hex << block->start() << dec << endl;
       return true;
   }
   return false;
}
