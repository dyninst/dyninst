/*
 * Copyright (c) 1996-2011 Barton P. Miller
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
using namespace Dyninst;
using namespace DepGraphAPI;

// Assume these represent a function and block of interest
BPatch_function *func;
BPatch_basicBlock *block;

// Access the DDG
DDG::Ptr ddg = DDG::analyze(func);

// Get the list of instructions (and their addresses) from the block

typedef std::pair<InstructionAPI::Instruction, Address> InsnInstance;
std::vector<InsnInstance> insnInstances;
block->getInstructions(insnInstances);

// For each instruction, look up the DDG node and see if it has itself as a target
for (std::vector<InsnInstance>::iterator iter = insnInstances.begin();
     iter != insnInstances.end(); iter++) {
  Address addr = iter->second;
  
	  NodeIterator nodeBegin, nodeEnd;
  ddg->find(addr, nodeBegin, nodeEnd);
  for (; nodeBegin != nodeEnd; nodeBegin++) {
    NodeIterator targetBegin, targetEnd;
    (*nodeBegin)->getTargets(targetBegin, targetEnd);
    for (; targetBegin != targetEnd; targetBegin++) {
      if (*targetBegin == *nodeBegin) {
        // Found a node that has itself as a target
        actOnSelfDefiningNode(*nodeBegin);
      }
    }
  }
}
