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

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>

#include <CCcommon.h>
#include <FCAllBlocks.h>

/** constructor */
FCAllBlocks::FCAllBlocks() : FunctionCoverage() {}

/** constructor */
FCAllBlocks::FCAllBlocks(BPatch_function* f,BPatch_thread* t,
			 BPatch_image* i,
			 const char* funcN) 
	: FunctionCoverage(f,t,i,funcN) {}

/* destructor */
FCAllBlocks::~FCAllBlocks() {}

/** since for all basic block instrumentation we do not need 
  * dominator information this method does nothing
  */
void FCAllBlocks::fillDominatorInfo(){
}

/** For all basic block instrumentation every basic blocks instrumented
  * so this method simply returns true for any given argument
  */ 
bool FCAllBlocks::validateBasicBlock(BPatch_basicBlock* bb){
	if(bb)
		return true;
	return false;
}

/** for all basic block instrumentation every basic block
  * is updated for its execution count. It does not infer any other
  * execution of any other basic block
  */ 
int FCAllBlocks::updateExecutionCounts(BPatch_basicBlock* bb,int ec){
	if(bb){
		BPatch_Vector<BPatch_sourceBlock*> sb;
		bb->getSourceBlocks(sb);
		for(unsigned int i=0;i<sb.size();i++)
			updateLinesCovered(sb[i]);
		executionCounts[bb->getBlockNumber()] += ec;
	}
	return Error_OK;
}
