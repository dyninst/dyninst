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

#ifndef _BPatch_basicBlockLoop_h
#define _BPatch_basicBlockLoop_h

#include <stdlib.h>
#include <string>
#include <set>
#include <vector>
#include "Annotatable.h"
#include "BPatch_dll.h"
#include "BPatch_Vector.h"
#include "BPatch_Set.h"
#include "BPatch_basicBlock.h"
#include "BPatch_flowGraph.h" 

namespace Dyninst {
namespace PatchAPI{
class PatchLoop;
}
}

class BPatch_variableExpr;
class BPatch_loopTreeNode;

class BPATCH_DLL_EXPORT BPatch_basicBlockLoop : 
   public Dyninst::AnnotatableSparse 
{
	friend class BPatch_flowGraph;

private:
        std::set<BPatch_edge*> backEdges;
	std::set<BPatch_basicBlock*> entries;

        BPatch_flowGraph *flowGraph;

        std::set<BPatch_basicBlockLoop*> containedLoops;

        std::set<BPatch_basicBlock*> basicBlocks;

        void addBackEdges(std::vector<BPatch_edge*> &edges);

public:
	BPatch_basicBlockLoop* parent;

        bool containsAddress(unsigned long addr);
	  
	bool containsAddressInclusive(unsigned long addr);


        int getBackEdges(BPatch_Vector<BPatch_edge*> &edges);

	int getLoopEntries(BPatch_Vector<BPatch_basicBlock*> &e);

        bool getContainedLoops(BPatch_Vector<BPatch_basicBlockLoop*> &loops);

	bool getOuterLoops(BPatch_Vector<BPatch_basicBlockLoop*> &loops);

        bool getLoopBasicBlocks(BPatch_Vector<BPatch_basicBlock*> &blocks);

        bool getLoopBasicBlocksExclusive(BPatch_Vector<BPatch_basicBlock*> &blocks);

        bool hasBlock(BPatch_basicBlock *b);

        bool hasBlockExclusive(BPatch_basicBlock *b);

        bool hasAncestor(BPatch_basicBlockLoop *loop);

        BPatch_flowGraph * getFlowGraph();

        std::set<BPatch_variableExpr*> * getLoopIterators();

        ~BPatch_basicBlockLoop() { }

        std::string format() const;

private:
	BPatch_basicBlockLoop(BPatch_flowGraph *, Dyninst::PatchAPI::PatchLoop*);

	bool getLoops(BPatch_Vector<BPatch_basicBlockLoop*>&, 
		      bool outerMostOnly) const;
};

#endif /*_BPatch_basicBlockLoop_h_*/
