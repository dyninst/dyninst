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

#ifndef _CCOnDemandInstrument_h_
#define _CCOnDemandInstrument_h_

#include <CodeCoverage.h>

/** class that inherits code coverage class for the code coverage
  * that instruments a function in the executable that has
  * line information available when the function is called first time.
  * Due to the difference between initial instrumentation and the method 
  * to run the program from the preisntrumentation this
  * class is added to the software
  */


class CCOnDemandInstrument : public CodeCoverage {
private:

	/** field to identify which functions are actually
	  * executed during the run (instrumented).
          */
	bool* isExecuted;

	/** instrumentation code to identify which function
	  * is called (executed) fro the first time. One for each.
	  */
	BPatchSnippetHandle** firstInstrumentationHandles;

	/** variable that stores the identifer of the function
  	  * that is just called (executed).
	  */
	BPatch_variableExpr* currentFunctionId;

	/** method that handles the actions during the deletion
	  * intervals which happens in fixed time intervals
	  */
	int deletionIntervalCallback();

        /** method that returns true if a function is instrumented
	  * for code coverage. The argument given is the id of the
	  * function in the array kept in parent class
	  * @param i identifier of the function from the array of functions
	  */
	bool isInstrumented(int i);	

public:

	/** constructor of the class */
	CCOnDemandInstrument();
	
	/** method to do the initial instrumentation to the functions
	  * whose source line information is available. Initial
	  * instrumentation is the instrumentation done before the
	  * the execution of the mutatee starts
	  */
	int instrumentInitial(); 

	/** method that run the mutatee program after the initial
	  * instrumentation
	  */
	int run();

	/** destructor of the class */
	~CCOnDemandInstrument();
};

#endif /* _CCOnDemandInstrument_h_ */
