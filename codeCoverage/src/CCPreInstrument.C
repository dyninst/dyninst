/*
 * Copyright (c) 1996-2011 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
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
#include <string.h>
#include <iostream>
#include <fstream>
#include <limits.h>
#include <pthread.h>
#include <tcl.h>
#include <tk.h>

#include <CCcommon.h>
#include <FCAllBlocks.h>
#include <FCUseDominator.h>
#include <CCPreInstrument.h>

using namespace std;
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
bool CCPreInstrument::isInstrumented(int){
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
