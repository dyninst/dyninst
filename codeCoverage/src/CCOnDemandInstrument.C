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

	/** if activated assign deleteion interval call back to the alarm dignal */
	if(deletionInterval > 0)
		signal(SIGALRM,intervalCallback);

	appThread->continueExecution();

	/** if activated start the alarm */
	if(deletionInterval > 0)
		alarm(deletionInterval);

	bool isExpected = true;

	while(true){
		/** wait untill a function is called for the first time */
		if(!(isExpected = bPatch.waitUntilStopped(appThread)))
			break;

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
			     << "deletion occurs..." << endl << endl;

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

			/** assign the handler to the alarm signal and continue
			  * execution
			  */
			signal(SIGALRM,intervalCallback);
			appThread->continueExecution();

			/** start another interval */
			alarm(deletionInterval);
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
	     << "deletion occurs..." << endl << endl;

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
	signal(SIGALRM,intervalCallback);
	appThread->continueExecution();

	/** start a new interval */
	alarm(deletionInterval);

	return Error_OK;
}
