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
#include "CF_Localization.h"
#include "dyninstAPI/src/debug.h"
#include "../Atoms/Atom.h"
#include "../Atoms/Target.h"
#include "../Atoms/CFAtom.h"


using namespace std;
using namespace Dyninst;
using namespace Relocation;
using namespace InstructionAPI;


bool LocalizeCF::processTrace(TraceList::iterator &iter) {
  // We don't care about elements that aren't CFAtoms
  // We may remove CFAtoms and replace them with a new 
  // CFAtom

  const Trace::AtomList &elements = (*iter)->elements();

  //relocation_cerr << "localCFTransformer going to work on block " 
//		  << std::hex << (*iter)->origAddr() << std::dec << endl;

  // We don't need to iterate over all the elements; by definition
  // the only one we care about is the last one. 
  // 
  // FIXME if this assumption no longer holds; that is, if we start
  // using traces instead of blocks. 

  CFAtom::Ptr cf = dyn_detail::boost::dynamic_pointer_cast<CFAtom>(elements.back() );
  
  if (!cf) return true;

  // I don't think we want this. I _really_ don't think we want this. But it's a change,
  // so getting commented out instead of replaced. 
  // if (cf->needsPostCallPadding()) return true;

  // If this CFAtom contains a Target that is a int_block
  // in our map, replace it with a Target to that block.

  for (CFAtom::DestinationMap::iterator d_iter = cf->destMap_.begin();
       d_iter != cf->destMap_.end(); ++d_iter) {

    assert(d_iter->second);

    TargetInt *target = d_iter->second;

    if (target->type() != TargetInt::BlockTarget) {
      continue;
    }

    Target<int_block *> *targ = static_cast<Target<int_block *> *> (target);
    int_block *bbl = targ->t();

    TraceMap::const_iterator found = bMap_.find(bbl);
    if (found != bMap_.end()) {
      //relocation_cerr << "      found matching block " << found->second.get() << endl;

      // First, record the int_block we removed an edge from. We will check
      // later to see if we removed all the incoming edges from this instance...
      recordIncomingEdges(d_iter->second);

      // And be sure not to leak
      if (d_iter->second)
	delete d_iter->second;

      Target<Trace::Ptr> *t = new Target<Trace::Ptr>(found->second);
      d_iter->second = t;

      //relocation_cerr << "        Incrementing removed edge count for " 
//		      << std::hex << found->first << std::dec << endl;
      replacedCount_[found->first]++;
    }
  }
  return true;
}

// Count up how many incoming edges we still have
// to see if we need to branch in to this block
bool LocalizeCF::postprocess(TraceList &) {
  //relocation_cerr << "Postprocessing LocalizeCF" << endl;
  for (std::map<int_block *, int>::iterator iter = replacedCount_.begin();
       iter != replacedCount_.end(); ++iter) {
    int_block *bbl = iter->first;

    int removedEdges = iter->second;

    // see if this block has any incoming edges that we didn't remove
    int incomingEdges = incomingCount_[bbl];

    //relocation_cerr << "   Considering block at " 
//		    << std::hex << bbl->firstInsnAddr() << std::dec
	//	    << ": incoming " << incomingEdges 
//<< " and removed " << removedEdges
	//	    << endl;

    
    if (removedEdges > incomingEdges) {
      cerr << "Odd case: " << removedEdges << " removed @ block " << hex << bbl->start() << dec
	   << ", but only " << incomingEdges << " known." << endl;
    }
    else if (removedEdges == incomingEdges) {
      //pMap_[addr] = Suggested;
    }
    else {
      pMap_[bbl] = Required;
    }
  }
  return true;
}

int LocalizeCF::getInEdgeCount(const int_block *bbl) {
  //relocation_cerr << "   ... getting number of in edges for block at "
//		  << std::hex << bbl->firstInsnAddr() << std::dec << endl;

  // Need to duck to internals to get interprocedural counts
  return bbl->llb()->sources().size();
}

void LocalizeCF::recordIncomingEdges(const TargetInt *in) {
  if (!in) return;

  const Target<int_block *> *targ = dynamic_cast<const Target<int_block *> *>(in);
  if (!targ) return;
 
  incomingCount_[targ->t()] = getInEdgeCount(targ->t());
}
