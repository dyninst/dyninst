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
#include "dyninstAPI/src/debug.h"
#include "../Atoms/Atom.h"
#include "../Atoms/Target.h"
#include "dyninstAPI/src/instPoint.h"
#include "dyninstAPI/src/baseTramp.h"
#include "../Atoms/Instrumentation.h"
#include "../Atoms/CFAtom.h"

using namespace std;
using namespace Dyninst;
using namespace Relocation;
using namespace InstructionAPI;


bool Instrumenter::processTrace(TraceList::iterator &iter) {
  //relocation_cerr << "Instrumenter, processing block " 
		  //<< std::hex << (*iter)->origAddr() << std::dec << endl;
  
  if ((*iter)->bbl() == NULL)
    return true;

  // Basic concept: iterate over all of our instructions and look
  // up the instPoint for that instruction. If it exists, prepend
  // or append an instrumentation element at the appropriate
  // point in the list.
  
  // TODO: edge instrumentation... that will require modifying
  // the block list as well as elements within a block. 

  baseTramp *pre = NULL;
  baseTramp *post = NULL;
  instPoint *point = NULL;

  AtomList &elements = (*iter)->elements();

  Address prevAddr = (Address) -1;

  for (AtomList::iterator e_iter = elements.begin();
       e_iter != elements.end(); ++e_iter) {

    // We're inserting an Inst element before us (if there is a baseTramp
    // with something interesting, that is). 

    // Assertion: we have no Inst elements already
    Address addr = (*e_iter)->addr();
    if (addr == 0) {
       //relocation_cerr << "Skipping Atom with address 0" << endl;
       continue;
    }
    if (addr == prevAddr) {
       // This is a hack - we can split a single instruction into a sequence
       // of Atoms that should be treated as one WRT instrumentation. 
       // Otherwise we get multiple copies of each instPoint for each new
       // Atom. We _really_ should have a "group" Atom, or a 1:1 restriction,
       // but I don't have time to fix that now.
       //relocation_cerr << "Skipping Atom with same addr as previous" << endl;
       continue;
    }
    prevAddr = addr;

    // CFAtoms can have an address even if they were invented.
    // We need a "virtual" boolean... but for now just check whether
    // there's an instruction there. 
    if (!(*e_iter)->insn()) {
       //relocation_cerr << "Skipping Atom with no insn" << endl;
       continue;
    }
    //relocation_cerr << "  Checking for point at " << std::hex << addr << std::dec << endl;

    point = (*iter)->bbl()->func()->findInstPByAddr(addr);

    if (point) {
        pre = point->preBaseTramp();
    }
    else {
        pre = NULL;
    }

    //relocation_cerr << "   Found instrumentation at addr " 
		   // << std::hex << addr << std::dec
           // << (post ? (post->empty() ? "<POST EMPTY>" : "<POST>") : "<NO POST>")
           // << (pre ? (pre->empty() ? "<PRE EMPTY>" : "<PRE>") : "<NO PRE>") << endl;
    
    Inst::Ptr inst = Inst::create();
    inst->addBaseTramp(post);

    inst->addBaseTramp(pre);

    if (!inst->empty())
      elements.insert(e_iter, inst);
    // Otherwise it silently disappears...

    if (point) {
        post = point->postBaseTramp();
    }
    else {
        post = NULL;
    }
  }

  // Edge instrumentation time
  // Only the final point can have edge instrumentation;
  // this includes the postBaseTramp (for fallthrough)
  // or a targetBaseTramp (for taken edges)

  if (point) {
    relocation_cerr << "   Trailing <point>, checking edge instrumentation" << endl;
    baseTramp *target = point->targetBaseTramp();
    // post is still assigned from above
    if (!target &&
	!post) {
      //relocation_cerr << "   ... neither target nor post, no edge" << endl;
      return true;
    }

    // Get the stuff we need: a CFAtom for the last instruction
    CFAtom::Ptr cf = dyn_detail::boost::dynamic_pointer_cast<CFAtom>(elements.back());
    assert(cf);

    if (post) {
      relocation_cerr << "   ... fallthrough inst @ " << hex << point->addr() << dec << ", adding" << endl;
      if (!addEdgeInstrumentation(post,
				  cf,
				  CFAtom::Fallthrough,
                  After,
				  *iter))
	return false;
    }
    if (target) {
       relocation_cerr << "   ... target inst, adding" << endl;
      if (!addEdgeInstrumentation(target,
				  cf,
				  CFAtom::Taken,
                  Before,
				  *iter))
	return false;
    }
  }
  return true;
}

bool Instrumenter::postprocess(TraceList &bl) {
  // Yuck iteration... anyone have a better idea?

  relocation_cerr << "Instrumenter: postProcess "  << edgeTraces_.size() << endl;
  
  if (edgeTraces_.empty()) {
    //relocation_cerr << "  ... nothing to do, returning" << endl;
    return true;
  }

  for (TraceList::iterator iter = bl.begin();
       iter != bl.end(); ++iter) {

    // Try pre-insertion
    EdgeTraces::iterator pre = edgeTraces_.find(std::make_pair(*iter, Before));
    if (pre != edgeTraces_.end()) {
      relocation_cerr << "     Inserting " << pre->second.size() << " pre blocks" << endl;
      bl.insert(iter, pre->second.begin(), pre->second.end());
    }
    // And post-insertion?
    EdgeTraces::iterator post = edgeTraces_.find(std::make_pair(*iter, After));
    if (post != edgeTraces_.end()) {
      // Game the main iterator here...
      ++iter; // To get successor
      bl.insert(iter, post->second.begin(), post->second.end());
      // We're now one too far; back up so the for loop will
      // move us forward.
      --iter;
    }

  }
  return true;
}


bool Instrumenter::addEdgeInstrumentation(baseTramp *tramp,
					  CFAtom::Ptr cf,
					  Address dest,
                      When when,
					  Trace::Ptr cur) {
  if (tramp->empty()) return true;
  relocation_cerr << "Adding edge inst" << endl;
  // We handle edge instrumentation by creating a new Trace and
  // wiring it in in the appropriate place. The actual adding is
  // done later, since we can't modify the list from here. 
  // What we can do is leave a marker of _where_ it should be 
  // inserted. 

  Trace::Ptr inst = Trace::create(tramp);
  Target<Trace::Ptr> *t = new Target<Trace::Ptr>(inst);

  // 1) Find the appropriate successor S of the current block B
  // 2) Set the fallthrough successor of the instrumentation block I to S
  // 3) Set the appropriate successor of S to I
  // 4) Add the inst block to the "add this to the list" list.

  // 1)
  CFAtom::DestinationMap::iterator d_iter = cf->destMap_.find(dest);
  // What if someone requested edge instrumentation for an edge that
  // doesn't exist? Oopsie.
  if (d_iter == cf->destMap_.end()) {
    delete t;
    return true;
  }

  // 2)
  // Keep this info for later...
  TargetInt *target = d_iter->second;

  CFAtom::Ptr postCF = CFAtom::create(cf->block());
  // Give this a valid destination
  postCF->updateAddr(cf->addr());
  postCF->addDestination(CFAtom::Fallthrough, target);
  inst->elements().push_back(postCF);

  // 3)
  d_iter->second = t;    
  
  // 4) 
  // It's more efficient branch-wise to put a block in
  // before its target rather than after this; 
  // however, if we aren't moving the target then we 
  // fall back.
  Trace::Ptr insertPoint;
  Target<Trace::Ptr> *targ = dynamic_cast<Target<Trace::Ptr> *>(target);
  if ((when == Before) && targ) {
    edgeTraces_[std::make_pair(targ->t(), Before)].push_back(inst);
  }
  else {
      // Sorry, can't do it before a non-relocated trace
    edgeTraces_[std::make_pair(cur, After)].push_back(inst);
  }

  return true;
}
