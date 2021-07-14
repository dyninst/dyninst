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

#ifndef _BPatch_sourceBlock_h_
#define _BPatch_sourceBlock_h_

#include "BPatch_dll.h"
#include "BPatch_Vector.h"
#include <set>

/** this class represents the basic blocks in the source
  * code. The source code basic blocks are calculated according to the
  * machine code basic blocks. The constructors can be called by only
  * BPatch_flowGraph class since we do not want to make the user 
  * create source blocks that does not exist and we do not want the user
  * to change the start and end line numbers of the source block
  *
  * @see BPatch_flowGraph
  * @see BPatch_basicBlock
  */

class BPATCH_DLL_EXPORT BPatch_sourceBlock {
	friend class BPatch_flowGraph;

private:
	const char* sourceFile;
        std::set<unsigned short> sourceLines;

public:

	/** method to return source file name 
	  * @param i the number of source file requested */
        const char* getSourceFile();

	/** method to return source lines in the
	  * corresponding source file 
	  * @param i the number of source file requested */
        void getSourceLines(BPatch_Vector<unsigned short> &lines);

	/** destructor for the sourceBlock class */

	virtual ~BPatch_sourceBlock() {}

private:
	/** constructor of the class */
	BPatch_sourceBlock();
	BPatch_sourceBlock(const char*,std::set<unsigned short>&);
       
};

#endif /* _BPatch_sourceBlock_h_ */

