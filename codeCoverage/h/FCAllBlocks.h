#ifndef _FCAllBlocks_h_
#define _FCAllBlocks_h_

#include <FunctionCoverage.h>

/** class to represent a function in the executable
  * whose source code line information is available.
  * This class is instroduced for all basic block
  * instrumentation of the function for code coverage.
  */
class FCAllBlocks : public FunctionCoverage {
private:
	/** private constructor of the class */
	FCAllBlocks();

	/** method that fill the dominator information
	  * of the control flow graph it contains
	  */
	void fillDominatorInfo();

	/** method to decide which basic block will
	  * be instrumented when the function is instrumented
	  * for code coverage. For this class every basic block
	  * for the control flow graph is validated.
	  * @param bb basic block pointer
	  */
	bool validateBasicBlock(BPatch_basicBlock* bb);

	/** if interval deletion is given to be >= 0
	  * then in fixed intervals the mutatee is stopped
	  * and the execution counts of the basic blocks are 
	  * updated for all basic blocks. This method
	  * updates the execution count of the basic block
	  * with the given count.
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
	FCAllBlocks(BPatch_function* f,BPatch_thread* t,BPatch_image* i,
		    const char* funcN,const char* fileN); 

	/** destructor of the class */
	~FCAllBlocks();
};

#endif /* _FCAllBlocks_h_ */
