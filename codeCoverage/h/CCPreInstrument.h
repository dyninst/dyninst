#ifndef _CCPreInstrument_h_
#define _CCPreInstrument_h_

#include <CodeCoverage.h>

/** class that inherits code coverage class for the code coverage
  * that preinstruments every function in the executable that has
  * line information available. Due to the difference between 
  * initial instrumentation and the method to run the program this
  * class is added to the software
  */

class CCPreInstrument: public CodeCoverage {
private:

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

	/** due to need that signal handler has to be non-member
	  * function it is defined to be friend function
          */
	friend void intervalCallback(int signalNo);
public:

	/** constructor of the class */
	CCPreInstrument();

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
	~CCPreInstrument();
};

#endif /*_CCPreInstrument_h_*/
