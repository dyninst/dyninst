/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

#ifndef _BPatch_sourceBlock_h_
#define _BPatch_sourceBlock_h_

#include <iostream>
#include "BPatch_dll.h"
#include "BPatch_Vector.h"
#include "BPatch_Set.h"
#include "BPatch_eventLock.h"

#ifdef IBM_BPATCH_COMPAT
class BPatch_point;
#endif

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
#ifdef DYNINST_CLASS_NAME
#undef DYNINST_CLASS_NAME
#endif
#define DYNINST_CLASS_NAME BPatch_sourceBlock

class BPATCH_DLL_EXPORT BPatch_sourceBlock : public BPatch_eventLock {
	friend class BPatch_flowGraph;
	friend std::ostream& operator<<(std::ostream&,BPatch_sourceBlock&);

private:
	const char* sourceFile;
	BPatch_Set<unsigned short>* sourceLines;

public:

	/** method to return source file name 
	  * @param i the number of source file requested */
        API_EXPORT(Int, (),
        const char *,getSourceFile,());

	/** method to return source lines in the
	  * corresponding source file 
	  * @param i the number of source file requested */
        API_EXPORT_V(Int, (lines),
        void,getSourceLines,(BPatch_Vector<unsigned short> &lines));

	/** destructor for the sourceBlock class */

	virtual ~BPatch_sourceBlock() {}

#ifdef IBM_BPATCH_COMPAT
        API_EXPORT(Int, (_startAddress, _endAddress),
        bool,getAddressRange,(void*& _startAddress, void*& _endAddress));

        API_EXPORT(Int, (_startLine, _endLine),
        bool,getLineNumbers,(unsigned int &_startLine, unsigned int  &_endLine));

        API_EXPORT_V(Int, (vect),
        void,getExcPoints,(BPatch_Vector<BPatch_point *> &vect));

        API_EXPORT_V(Int, (vect),
        void,getIncPoints,(BPatch_Vector<BPatch_point *> &vect));

        API_EXPORT(Int, (buf, buflen),
        char *,getName,(char *buf, int buflen));
#endif

private:
	/** constructor of the class */
	BPatch_sourceBlock();
	BPatch_sourceBlock(const char*,BPatch_Set<unsigned short>&);
       
};

#endif /* _BPatch_sourceBlock_h_ */

