#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <iostream.h>
#include <fstream.h>
#include <limits.h>
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
		char tclBuffer[256];
		sprintf(tclBuffer,"%s configure -text \
			\"Initial instrumentation is being done...\"",
			statusBarName);
		Tcl_Eval(globalInterp,tclBuffer);
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
		char tclBuffer[256];
		sprintf(tclBuffer,"%s configure -text \
			\"Execution of mutatee started...\"",
			statusBarName);
		Tcl_Eval(globalInterp,tclBuffer);
	}

	globalObject = this;

	int errorCode = Error_OK;

	appThread->continueExecution();

	/** if instrumentation code deletion is activated */
	if(deletionInterval > 0)
		while(true){
			/** if already stooped, it reached the end of mutatee */
			if(appThread->isStopped())
				break;

			/** wait some time (in seconds) */
			sleep(deletionInterval);

			/** call the deletion interval callback */
			intervalCallback(SIGALRM);

		}

	/** wait untill the breakpoint at exithandle is reached */
	bPatch.waitUntilStopped(appThread);

	/** coverage results are printed into a binary file */
	errorCode = printCoverageInformation();
	if(errorCode < Error_OK)
		return errorPrint(Error_PrintResult);
	
	/** the mutatee is let to run to terminate */
	appThread->continueExecution();

	cout << endl 
	     << "information: the execution of mutatee terminates..."
	     << endl << endl;

	if(globalInterp && statusBarName){
		char tclBuffer[256];
		sprintf(tclBuffer,"%s configure -text \
			\"Execution of mutatee started...\"",
			statusBarName);
		Tcl_Eval(globalInterp,tclBuffer);
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

	extern unsigned totalDeletions;
        totalDeletions = 0;
	extern unsigned totalCoveredLines;
	totalCoveredLines = 0;

	cout << endl 
	     << "information: mutatee stopped and deletion occurs..."
	     << endl << endl;

	/** update the execution counts of the basic blocks */
        updateFCObjectInfo();

	/** continue execution */
	if(!already)
		appThread->continueExecution();

	return Error_OK;
}
