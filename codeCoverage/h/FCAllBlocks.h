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

#ifndef _FCAllBlocks_h_
#define _FCAllBlocks_h_

#include <FunctionCoverage.h>

/** class to represent a function in the executable
  * whose source code line information is available.
  * This class is instroduced for all basic block
  * instrumentation of the function for code coverage.
  */
class FCAllBlocks : public FunctionCoverage {
private:
	/** private constructor of the class */
	FCAllBlocks();

	/** method that fill the dominator information
	  * of the control flow graph it contains
	  */
	void fillDominatorInfo();

	/** method to decide which basic block will
	  * be instrumented when the function is instrumented
	  * for code coverage. For this class every basic block
	  * for the control flow graph is validated.
	  * @param bb basic block pointer
	  */
	bool validateBasicBlock(BPatch_basicBlock* bb);

	/** if interval deletion is given to be >= 0
	  * then in fixed intervals the mutatee is stopped
	  * and the execution counts of the basic blocks are 
	  * updated for all basic blocks. This method
	  * updates the execution count of the basic block
	  * with the given count.
	  * @param bb basic block whose execution count will be 
	  *        updated
	  * @param ec number of executions to add
	  */
	int updateExecutionCounts(BPatch_basicBlock* bb,int ec);

public:
	/** constructor of the class 
	  * @param f function to instrument later
	  * @param t thread object of the mutatee
	  * @param i image that the function belongs to
	  * @param funcN the name of the function
	  * @param fileN the name of the source file the function is in
	  */
	FCAllBlocks(BPatch_function* f,BPatch_thread* t,BPatch_image* i,
		    const char* funcN); 

	/** destructor of the class */
	~FCAllBlocks();
};

#endif /* _FCAllBlocks_h_ */
