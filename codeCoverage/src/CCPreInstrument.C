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

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <iostream.h>
#include <fstream.h>
#include <limits.h>
#include <pthread.h>
#include <tcl.h>
#include <tk.h>

#include <CCcommon.h>
#include <FCAllBlocks.h>
#include <FCUseDominator.h>
#include <CCPreInstrument.h>

/** constructor */
CCPreInstrument::CCPreInstrument() : CodeCoverage() {}

/** destructor */
CCPreInstrument::~CCPreInstrument() {}

/** For pre instrumenting code coverage, control flow graph is
  * created for each function that is to be instrumented and then
  * instrumented prior to the execution.
  */
int CCPreInstrument::instrumentInitial(){

	cout << "information: Beginning initial instrumentation..." << endl;

	if(globalInterp && statusBarName){
		pthread_mutex_lock(&statusUpdateLock);
		tclStatusChanged = true;
		sprintf(tclStatusBuffer,"%s configure -text \
			\"Initial instrumentation is being done...\"",
			statusBarName);
		pthread_mutex_unlock(&statusUpdateLock);
	}

	for(int i=0;i<instrumentedFunctionCount;i++){
		FunctionCoverage* fc = instrumentedFunctions[i];

		/** an id is given to the function */
		fc->setId(i);

		int errorCode = Error_OK;

		/** instrumentation points are chosen */
		errorCode = fc->selectInstrumentationPoints();
		if(errorCode < Error_OK)
			return errorCode;

		/** necessary code is inserted */
		errorCode = fc->instrumentPoints();
		if(errorCode < Error_OK)
			return errorCode;
	}
	return Error_OK;
}

/** the mutatee is executed while mutator waits for fixed time intervals.
  * when the necessary time passes the mutator stops the mutatee and
  * calls the deletion interval callback to collect/update 
  * information and to delete the already executed instrumentation
  * code.
  */
int CCPreInstrument::run(){
	
	cout << endl
	     << "information: The execution of the mutatee starts..."
	     << endl << endl;

	if(globalInterp && statusBarName){
		pthread_mutex_lock(&statusUpdateLock);
		tclStatusChanged = true;
		sprintf(tclStatusBuffer,"%s configure -text \
			\"Execution of mutatee started...\"",
			statusBarName);
		pthread_mutex_unlock(&statusUpdateLock);
	}

	globalObject = this;

	int errorCode = Error_OK;

	appThread->continueExecution();

	/** if instrumentation code deletion is activated */
	if(deletionInterval > 0)
		while(true){
			if(appThread->isTerminated()){
				cerr << "Mutatee unexpectedly terminated....." << endl;
				exit(-1);
			}

			/** if already stooped, it reached the end of mutatee */
			if(appThread->isStopped())
				break;

			/** wait some time (in seconds) */
			sleep(deletionInterval);

			/** call the deletion interval callback */
			deletionIntervalCallback();

		}

	/** wait untill the breakpoint at exithandle is reached */
	bPatch.waitUntilStopped(appThread);

	whichInterval++;
	totalDeletions = 0;

	/** coverage results are printed into a binary file */
	errorCode = printCoverageInformation();
	if(errorCode < Error_OK)
		return errorPrint(Error_PrintResult);
	
	addTclTkFrequency();

	/** the mutatee is let to run to terminate */
	appThread->continueExecution();

	bPatch.waitForStatusChange();

	cout << endl 
	     << "information: the execution of mutatee terminates..."
	     << endl << endl;

	if(globalInterp && statusBarName){
		pthread_mutex_lock(&statusUpdateLock);
		tclStatusChanged = true;
		sprintf(tclStatusBuffer,"%s configure -text \
			\"Execution of mutatee ended...\"",
			statusBarName);
		pthread_mutex_unlock(&statusUpdateLock);
	}

	return Error_OK;
}

/** for pre instrumentating code coverage since every function
  * whose source code is available will be instrumented 
  * this method always returns true.
  */
bool CCPreInstrument::isInstrumented(int i){
	return true;
}

/** deletion interval callback is called at fixed
  * time intervals and the mutatee is stopped and
  * necessary execution counts are updated and 
  * already executed instrumentation code is deleted 
  */
int CCPreInstrument::deletionIntervalCallback(){

	/** stop the process if not stopped yet */
	bool already = false;
	if(!appThread->isStopped())
		appThread->stopExecution();
	else already = true;

	whichInterval++;
        totalDeletions = 0;

	if(globalInterp && statusBarName){
		pthread_mutex_lock(&statusUpdateLock);
		tclStatusChanged = true;
		sprintf(tclStatusBuffer,"%s configure -text \
		\"Interval %d has started...\"",statusBarName,whichInterval);
		pthread_mutex_unlock(&statusUpdateLock);
	}

	cout << endl 
	     << "information: mutatee stopped and deletion occurs..."
	     << endl << endl;

	/** update the execution counts of the basic blocks */
        updateFCObjectInfo();

	addTclTkFrequency();

	if(globalInterp && statusBarName){
		pthread_mutex_lock(&statusUpdateLock);
		tclStatusChanged = true;
		sprintf(tclStatusBuffer,"%s configure -text \
			\"Interval %d has ended...\"",statusBarName,whichInterval);
		pthread_mutex_unlock(&statusUpdateLock);
	}

	/** assign the alarm interval call back and continue exec */
	/** continue execution */
	if(!already)
		appThread->continueExecution();

	return Error_OK;
}
