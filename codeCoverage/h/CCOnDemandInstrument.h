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
