#ifndef _FCUseDominator_h_
#define _FCUseDominator_h_

#include <FunctionCoverage.h>

/** class to represent a function in the executable
  * whose source code line information is available.
  * This class is introduced for instrumentation of the function 
  * using the dominator information of its control flow graph
  * for code coverage.
  */
class FCUseDominator : public FunctionCoverage {
private:
	/** private constructor of the class */
	FCUseDominator();

	/** method that fill the dominator information
	  * of the control flow graph it contains
	  */
	void fillDominatorInfo();

	/** method to decide which basic block will
	  * be instrumented when the function is instrumented
	  * for code coverage. A basic block is validated
	  * if it is a leaf in dominator tree or has an 
	  * outgoing edge to a basic block it does not dominate.
	  * @param bb basic block pointer
	  */
	bool validateBasicBlock(BPatch_basicBlock* bb);

	/** if interval deletion is given to be >= 0
	  * then in fixed intervals the mutatee is stopped
	  * and the execution counts of the basic blocks are
	  * updated for all blocks. This method
	  * updates the execution count of the basic block
	  * with the given count using the dominator information.
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
	FCUseDominator(BPatch_function* f,BPatch_thread* t,BPatch_image* i,
		       const char* funcN,const char* fileN);

	/** destructor of the class */
	~FCUseDominator();
};

#endif /* _FCUseDominator_h_ */
