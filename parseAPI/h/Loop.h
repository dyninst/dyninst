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

#ifndef _Loop_h
#define _Loop_h

#include <stdlib.h>
#include <string>
#include "Annotatable.h"
#include "CFG.h"

/** class to represent the loops composed of machine code basic 
  * blocks in the executable (Natural loops)
  *
  */

namespace Dyninst{
namespace ParseAPI{

class LoopAnalyzer;
class LoopTreeNode;

class PARSER_EXPORT Loop  
{
	friend class LoopAnalyzer;
	friend std::ostream& operator<<(std::ostream&,Loop&);

private:
        std::set<Edge*> backEdges;

        // the function this loop is part of
        Function * func;

	/** set of loops that are contained (nested) in this loop. */
        std::set<Loop*> containedLoops;

	/** the basic blocks in the loop */
        std::set<Block*> basicBlocks;

        /** this func is only invoked by LoopAnalyzer::createLoops */
        void addBackEdges(std::vector<Edge*> &edges);

public:
	/** If loop which directly encloses this loop. NULL if no such loop */

	Loop* parent;

	/** Return true if the given address is within the range of
	    this loop's basicBlocks */

        bool containsAddress(Address addr);
	  
	/** Return true if the given address is within the range of
	    this loop's basicBlocks or its children */
		   
	bool containsAddressInclusive(Address addr);


	/** Loop::getBackEdge    */
        /** return a back edge that defines this loop */

        Edge * getBackEdge();

	/** Loop::getBackEdges */
        /** Sets edges to the set of back edges that define this loop,
            returns the number of back edges that define this loop */
        int getBackEdges(vector<Edge*> &edges);

	/** Loop::getContainedLoops    */
	/** returns vector of contained loops */

        bool getContainedLoops(vector<Loop*> &loops);

	/** Loop::getOuterLoops    */
	/** returns vector of outer contained loops */

	bool getOuterLoops(vector<Loop*> &loops);

	/** Loop::getLoopBasicBlocks    */
	/** returns all basic blocks in the loop */

        bool getLoopBasicBlocks(vector<Block*> &blocks);

	/** Loop::getLoopBasicBlocksExclusive    */
	/** returns all basic blocks in this loop, exluding the blocks
	    of its sub loops. */

        bool getLoopBasicBlocksExclusive(vector<Block*> &blocks);

        /** does this loop or its subloops contain the given block? */

        bool hasBlock(Block *b);

        /** does this loop contain the given block? */

        bool hasBlockExclusive(Block *b);

	/** Loop::hasAncestor    */
	/** returns true if this loop is a descendant of the given loop */

        bool hasAncestor(Loop *loop);

	/** returns the function this loop is in */

        Function * getFunction();

	/** Loop::getLoopHead    */
	/** returns the head basic block of the loop */

        Block * getLoopHead();

	/** Loop::~Loop    */
	/** destructor for the class */

        ~Loop() { }

        std::string format() const;

private:
// internal use only
	/** constructor of class */
	Loop(Function *);

	/** constructor of the class */
	Loop(Edge *, Function *);

	/** get either contained or outer loops, determined by outerMostOnly */
	bool getLoops(vector<Loop*>&, 
		      bool outerMostOnly) const;
}; // class Loop
}  // namespace ParseAPI
}  // namespace Dyninst
#endif /*_Loop_h_*/
