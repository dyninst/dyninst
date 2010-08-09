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

#include "r_t_Base.h"
#include "r_t_Fallthroughs.h"
#include "debug.h"
#include "r_e_Target.h"
#include "r_e_Base.h"
#include "r_e_ControlFlow.h"

using namespace std;
using namespace Dyninst;
using namespace Relocation;

bool Fallthroughs::processBlock(BlockList::iterator &iter) {
  // The logic is simple: 
  // For each target of the block
  //   If the target is indirect, skip
  //   If the target is the immediate successor, set necessary to false
  //   If the target is not the immediate successor, set necessary to true

  const Block::ElementList &elements = (*iter)->elements();
  
  relocation_cerr << "Fallthrough transformer going to work on block " << (*iter)->id() << endl;

  // We don't need to iterate over all the elements; by definition
  // the only one we care about is the last one. 
  //
  // FIXME if this assumption no longer holds; that is, if we start
  // using traces instead of blocks. 

  CFElement::Ptr cf = dyn_detail::boost::dynamic_pointer_cast<CFElement>(elements.back() );
  
  if (!cf) return true;
  
  for (CFElement::DestinationMap::iterator d_iter = cf->destMap_.begin();
       d_iter != cf->destMap_.end(); ++d_iter) {

    if ((d_iter->first != CFElement::Fallthrough) &&
	(d_iter->first != CFElement::Taken)) {
      relocation_cerr << "\t Skipping addr-based edge " << d_iter->first << endl;
      continue;
    }

    TargetInt *target = d_iter->second;

    BlockList::iterator next = iter; ++next;

    if (target->matches(*next)) {
      relocation_cerr << "\t " << d_iter->first << ": target " << target->format()
		      << " and next block " << (*next)->id() << ", setting false" << endl;
      target->setNecessary(false);
    }
    else {
      relocation_cerr << "\t " << d_iter->first << ": target " << target->format()
		      << " and next block " << (*next)->id() << ", setting true" << endl;
      target->setNecessary(true);
    }
  }
  return true;
}
