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

class BPATCH_DLL_EXPORT BPatch_sourceBlock {
	friend class BPatch_flowGraph;

private:
	const char* sourceFile;
        std::set<unsigned short> sourceLines;

public:

        const char* getSourceFile();

        void getSourceLines(BPatch_Vector<unsigned short> &lines);

	virtual ~BPatch_sourceBlock() {}

private:
	BPatch_sourceBlock();
	BPatch_sourceBlock(const char*,std::set<unsigned short>&);
       
};

#endif /* _BPatch_sourceBlock_h_ */

