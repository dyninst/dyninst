#ifndef _FunctionCoverage_h_
#define _FunctionCoverage_h_

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_image.h"
#include "BPatch_module.h"
#include "BPatch_point.h"
#include "BPatch_snippet.h"
#include "BPatch_thread.h"
#include "BPatch_flowGraph.h"

/** class to represent the instrumentation of a function 
  * during code coverage. It also keeps data structures 
  * to store the control flow graph, execution counts of the
  * basic blocks of the function. It is the base class for 
  * two classes that either does all basic block instrumentation
  * or use dominator information during instrumentation
  */
class FunctionCoverage {
protected:
	/** function to be instrumented */
	BPatch_function* bpFunction;
	
	/** the thread function belongs to */
	BPatch_thread* appThread;
	
	/** image the function belongs to */
	BPatch_image* appImage;

	/** control flow graph of the function */
	BPatch_flowGraph* cfg;

	/** number of instrumentation code that is inserted */
	int instrumentationCount;

	/** the basic blocks that are instrumented */
	BPatch_basicBlock** instrumentedBlock;

	/** variables assigned to basic blocks */
	BPatch_variableExpr** blockVariable;

	/** instrumentation code inserted for each basic block*/
	BPatchSnippetHandle** instrumentationCode;

	/** instrumentation counts for all basic blocks */
	int* executionCounts;

	/** name of the function */
	const char* functionName;

	/** source file name the function is in */
	const char* fileName;

	/* the id of the function */
	int id;

	/** all blocks in the control flow graph */
	BPatch_Set<BPatch_basicBlock*> allBlocks;

	/** entry blocks in the control flow graph */
	BPatch_Set<BPatch_basicBlock*> entryBlock;

	/** exit blocks in the control flow graph */
	BPatch_Set<BPatch_basicBlock*> exitBlock;

	bool isPrecise;

	unsigned short lineCount;
	BPatch_Set<unsigned short> executedLines;
	BPatch_Set<unsigned short> unExecutedLines;

	float executionPercentage;

public:
	/** friend class */
	friend class CCOnDemandInstrument;
	friend class CodeCoverage;
	
	/** friend function that is used to sort Function Coverage 
	  * objects according to the function names
	  */
	friend int FCSortByFileName(const void* arg1,const void* arg2);

protected:
	/** method to print error message for this class */
	static int errorPrint(int code,char* text=NULL);

	/** method that initializes the dominator information 
	  * of the control flow graph
	  */
	virtual void fillDominatorInfo();

	/** method to validate whether a basic blocks is 
	  * needed to be instrumented or not.
	  * @param bb basic block pointer
	  */
	virtual bool validateBasicBlock(BPatch_basicBlock* bb);

	/** method to update the execution counts of a basic block
	  * and the others which can be deduced from this basic block
	  * @param bb basic block to start with
	  * @param ec execution count to be added 
	  */
	virtual int updateExecutionCounts(BPatch_basicBlock* bb,int ec);
public:
	pthread_mutex_t updateLock;

	/** constructor of the class */
	FunctionCoverage(); 

	/** constructor of the class
	 * @param f function assigned to the object
	 * @param t thread of the mutatee which function belongs
	 * @param i image of the mutatee which function belongs
	 * @param funcN name of the function
	 * @param fileN name of the source file function is in
	 */
	FunctionCoverage(BPatch_function* f,BPatch_thread* t,BPatch_image* i,
			 const char* funcN,const char* fileN);

	/** method to create control flow graph of the function */
	int createCFG();

	/** method to set the id of the object
	  * this id is used during on demand instrumentation
	  * to identify which function is just called
	  * @param i identifier for the object
	  */
	void setId(int i);

	/** method that inserts breakpoint to the beginning of the function
	  * It also inserts an instrumentation code that assigns id of the
	  * function to the variable given as an argument 
	  * @param v variable to assign the function id to
	  */
	BPatchSnippetHandle* insertBreakPointatEntry(BPatch_variableExpr* v);

	/** method that selects basic blocks to instrument */
	int selectInstrumentationPoints();

	/** method that inserts instrumentation code to the beginning of the
	  * basic blocks
	  */
	int instrumentPoints();

	/** method to update the execution counts of the basic blocks
	  * in deletion intervals.
	  */
	int updateExecutionCounts();

	/** method to update line stuctures */
	int updateLinesCovered(BPatch_sourceBlock* sb);

	/** method to print coverage results to the given file
	  * @param cf file to print the results to
	  * @param isInst flag whether the function is executed at all or not
	  */
	int printCoverageInformation(ofstream& cf);

	/** method to initialize lines that are not executed */
	void initializeLines(BPatch_Set<unsigned short>& lines);

	/** destructor of the class */
	virtual ~FunctionCoverage();
};

#endif /* _FunctionCoverage_h_ */
