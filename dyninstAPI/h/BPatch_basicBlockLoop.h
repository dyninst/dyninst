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

/** class to represent the loops composed of machine code basic 
  * blocks in the executable (Natural loops)
  *
  * @see BPatch_basicBlock
  * @see BPatch_flowGraph
  */

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

        // the flow graph this loop is part of
        BPatch_flowGraph *flowGraph;

	/** set of loops that are contained (nested) in this loop. */
        std::set<BPatch_basicBlockLoop*> containedLoops;

	/** the basic blocks in the loop */
        std::set<BPatch_basicBlock*> basicBlocks;

        /** this func is only invoked by BPatch_flowGraph::createLoops */
        void addBackEdges(std::vector<BPatch_edge*> &edges);

public:
	/** If loop which directly encloses this loop. NULL if no such loop */
	BPatch_basicBlockLoop* parent;

	/** BPatch_basicBlockLoop::containsAddress    */
	/** Return true if the given address is within the range of
	    this loop's basicBlocks */

        bool containsAddress(unsigned long addr);
	  
	/** Return true if the given address is within the range of
	    this loop's basicBlocks or its children */
		   
	bool containsAddressInclusive(unsigned long addr);


	/** BPatch_basicBlockLoop::getBackEdges */
        /** Sets edges to the set of back edges that define this loop,
            returns the number of back edges that define this loop */
        int getBackEdges(BPatch_Vector<BPatch_edge*> &edges);

	/** BPatch_basicBlockLoop::getLoopEntries */
	/** Sets e to the set of entry blocks of the loop.
	 * A natural loop has a single entry block
	 * and an irreducible loop has mulbile entry blocks
	 * */
	int getLoopEntries(BPatch_Vector<BPatch_basicBlock*> &e);

	/** BPatch_basicBlockLoop::getContainedLoops    */
	/** returns vector of contained loops */

        bool getContainedLoops(BPatch_Vector<BPatch_basicBlockLoop*> &loops);

	/** BPatch_basicBlockLoop::getOuterLoops    */
	/** returns vector of outer contained loops */

	bool getOuterLoops(BPatch_Vector<BPatch_basicBlockLoop*> &loops);

	/** BPatch_basicBlockLoop::getLoopBasicBlocks    */
	/** returns all basic blocks in the loop */

        bool getLoopBasicBlocks(BPatch_Vector<BPatch_basicBlock*> &blocks);

	/** BPatch_basicBlockLoop::getLoopBasicBlocksExclusive    */
	/** returns all basic blocks in this loop, exluding the blocks
	    of its sub loops. */

        bool getLoopBasicBlocksExclusive(BPatch_Vector<BPatch_basicBlock*> &blocks);

        /** does this loop or its subloops contain the given block? */

        bool hasBlock(BPatch_basicBlock *b);

        /** does this loop contain the given block? */

        bool hasBlockExclusive(BPatch_basicBlock *b);

	/** BPatch_basicBlockLoop::hasAncestor    */
	/** returns true if this loop is a descendant of the given loop */

        bool hasAncestor(BPatch_basicBlockLoop *loop);

	/** returns the flow graph this loop is in */

        BPatch_flowGraph * getFlowGraph();

	/** BPatch_basicBlockLoop::getLoopIterators    */
	/** method that returns the variables used as iterator */
	/** not implemented yet */

        std::set<BPatch_variableExpr*> * getLoopIterators();

	/** BPatch_basicBlockLoop::~BPatch_basicBlockLoop    */
	/** destructor for the class */

        ~BPatch_basicBlockLoop() { }

        std::string format() const;

private:
	BPatch_basicBlockLoop(BPatch_flowGraph *, Dyninst::PatchAPI::PatchLoop*);

	/** get either contained or outer loops, determined by outerMostOnly */
	bool getLoops(BPatch_Vector<BPatch_basicBlockLoop*>&, 
		      bool outerMostOnly) const;
};

#endif /*_BPatch_basicBlockLoop_h_*/
