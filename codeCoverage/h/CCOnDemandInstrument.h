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
