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
#include <time.h>

#include <CCcommon.h>
#include <FCAllBlocks.h>
#include <FCUseDominator.h>
#include <CCOnDemandInstrument.h>

/** flag used to identify whether alarm is given
  * when the mutatee already stopped or not. Accordingly
  * the execution counts will be updated just before the
  * the execution starts again
  */
bool alreadyStopped = false;

/** constructor */
CCOnDemandInstrument::CCOnDemandInstrument() 
	: CodeCoverage(),
	  isExecuted(NULL),firstInstrumentationHandles(NULL),
	  currentFunctionId(NULL) {}

/** destructor */
CCOnDemandInstrument::~CCOnDemandInstrument()
{
	delete[] isExecuted;
	delete[] firstInstrumentationHandles;
	delete currentFunctionId;
}

/** method instruments every function with a break point
  * and assigment statement at the entry point. The break point
  * is reached when the function is called first time and
  * the assigment statement assigns function id to a global
  * variable to identfy the function that is just called
  */
int CCOnDemandInstrument::instrumentInitial(){

	cout << "information: Beginning initial instrumentation..." << endl;

	if(globalInterp && statusBarName){
		pthread_mutex_lock(&statusUpdateLock);
		tclStatusChanged = true;
		sprintf(tclStatusBuffer,"%s configure -text \
			\"Initial instrumentation is being done...\"",
			statusBarName);
		pthread_mutex_unlock(&statusUpdateLock);
	}

	int tmpV = -1;

	/** creating the variable to store function id that is called */
        currentFunctionId = appThread->malloc(*appImage->findType("int"));
        currentFunctionId->writeValue((void*)&tmpV);

	/** initialization of the data structures */
	isExecuted = new bool[instrumentedFunctionCount];
	firstInstrumentationHandles = 
			new BPatchSnippetHandle*[instrumentedFunctionCount];

	/** for each function coverage object a new id is given and 
	  * instrumented with a break point and assignment
	  */
	for(int i=0;i<instrumentedFunctionCount;i++){
		FunctionCoverage* fc = instrumentedFunctions[i];

		fc->setId(i);

		isExecuted[i] = false;
		firstInstrumentationHandles[i] = 
				fc->insertBreakPointatEntry(currentFunctionId);
	}
	return Error_OK;
}

/** this method runs the mutatee program and stops the execution
  * at fixed time intervals if it is activated. Basically the intervals
  * are handled by alarm signal and its handler. The instrumentation of
  * functions for code coverage is done on-demand manner , where a function
  * is instrumented just after it is called for the first time. Initially
  * breakpoints are instrumented to the beginning of cuntions and when
  * one is reached the id of the function is read and then the records
  * are retrieved. It is instrumented. The execution continues. Meanwhile
  * at fixed time intervals the execution is stopped, if already not, and
  * necessary updates are done. If during the instrumentation alarm handler
  * runs then the updates are delayed to the end of the instrumentation.
  */
int CCOnDemandInstrument::run(){
	
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

	int errorCode = Error_OK;

	globalObject = this;

	appThread->continueExecution();

	bool isExpected = true;

	struct timespec timeBuffer;
	clock_gettime(CLOCK_REALTIME,&timeBuffer);
	unsigned beginTime = 
		(unsigned)(timeBuffer.tv_sec + 0.5 + (timeBuffer.tv_nsec / 1.0e9));

	while(true && isExpected){
		unsigned howManyPolls = 0;
		while (!appThread->isStopped() && !appThread->isTerminated()){
			if(!deletionInterval){
				bPatch.waitForStatusChange();
				continue;
			}

			if(howManyPolls && !(howManyPolls % 100))
				sleep(1);
				
			bPatch.pollForStatusChange(); 
			howManyPolls++;

			clock_gettime(CLOCK_REALTIME,&timeBuffer);
			unsigned endTime = 
				(unsigned)(timeBuffer.tv_sec + 0.5 + (timeBuffer.tv_nsec / 1.0e9));
			if((endTime - beginTime) >= deletionInterval){
				deletionIntervalCallback();
				beginTime = endTime;
				howManyPolls = 0;
			}
		}

		if (!appThread->isStopped() || 
		    (appThread->stopSignal() != SIGSTOP))
		{
			isExpected = false;
			continue;
		}

		/** read the identifier of the function that is called */
		int whichFunctionBreak;
		currentFunctionId->readValue((void*)&whichFunctionBreak);

		int functionId = whichFunctionBreak;

		/** if the end of the mutatee is reached */
		if(whichFunctionBreak == -1)
                        break;

		whichFunctionBreak = -1;
		currentFunctionId->writeValue((void*)&whichFunctionBreak);

		FunctionCoverage* instF = instrumentedFunctions[functionId];

		/** select instrumentation points for the function */
		errorCode = instF->selectInstrumentationPoints();
		if(errorCode < Error_OK)
			return errorCode;

		/** instrument the selected points */
		errorCode = instF->instrumentPoints();
		if(errorCode < Error_OK)
			 return errorCode;
			
		/** delete the breakpoint at entry point such that it never
 		  * is stopped for the same function again
		  */
		appThread->deleteSnippet(firstInstrumentationHandles[functionId]);

		/** set the function to be executed */
		isExecuted[functionId] = true;

		appThread->continueExecution();
		
		/** if the alarm signal is caught during instrumentation of a function
		  * or while the mutatee was stopped, we delay the interval callback to this point
		  */
		if((deletionInterval > 0) && alreadyStopped){
			appThread->stopExecution();

			whichInterval++;
			totalDeletions = 0;

			cout << endl
			     << "information: mutatee stopped and "
			     << "deletion occurs...(interval after instrumentation)" << endl << endl;

			/** update the execution counts of the basic blocks */
			updateFCObjectInfo();

			alreadyStopped = false;

			addTclTkFrequency();

			if(globalInterp && statusBarName){
				pthread_mutex_lock(&statusUpdateLock);
				tclStatusChanged = true;
				sprintf(tclStatusBuffer,"%s configure -text \
					\"Interval %d has ended...\"",
					statusBarName,whichInterval);
				pthread_mutex_unlock(&statusUpdateLock);
			}

			appThread->continueExecution();
		}

	}
	if(!isExpected){
		cout << "information: the mutatee terminated unexpectedly..."
		     << endl;
		return Error_OK;
	}

	whichInterval++;
	totalDeletions = 0;
	
	/** coverage results are printed into a binary file */
	errorCode = printCoverageInformation();
	if(errorCode < Error_OK)
		return errorPrint(Error_PrintResult);

	addTclTkFrequency();

	/** let the mutatee terminate */
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

/** if the function is executed during the execution of mutatee
  * it implies that the function is instrumented. Otherwise
  * no code coverage specific instrumentation is done
  */
bool CCOnDemandInstrument::isInstrumented(int i){
        return isExecuted[i];
}


/** deletion interval callback is called at fixed
  * time intervals and the mutatee is stopped and
  * necessary execution counts are updated and 
  * already executed instrumentation code is deleted
  */
int CCOnDemandInstrument::deletionIntervalCallback(){
	
	/** if stopped then delay the call back,
	  * otherwise stopp the execution of
	  * mutatee
	  */
	if(appThread->isStopped()){
		alreadyStopped = true;
		return Error_OK;
	}

	appThread->stopExecution();
	
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
	     << "information: mutatee stopped and "
	     << "deletion occurs...(interval after time interval)" << endl << endl;

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

	appThread->continueExecution();

	return Error_OK;
}
