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

class FunctionCoverage;

/** class to represent the instrumentation of a function 
  * during code coverage. It also keeps data structures 
  * to store the control flow graph, execution counts of the
  * basic blocks of the function. It is the base class for 
  * two classes that either does all basic block instrumentation
  * or use dominator information during instrumentation
  */
class FileLineCoverage {
	friend class CodeCoverage;
	friend class FunctionCoverage;
	friend int FCSortByFileName(const void* arg1,const void* arg2);
	friend int FLSortByFileName(const void* arg1,const void* arg2);

protected:
	FunctionCoverage* owner;
	/** source file name the function is in */
	const char* fileName;

	unsigned short lineCount;
	BPatch_Set<unsigned short> executedLines;
	BPatch_Set<unsigned short> unExecutedLines;

	float executionPercentage;
public:
	FileLineCoverage(const char*);
	void initializeLines(BPatch_Set<unsigned short>&);
	void setOwner(FunctionCoverage*);
	static FileLineCoverage* findFile(FileLineCoverage**,int,const char*);
	~FileLineCoverage();
};
	
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

	/* the id of the function */
	int id;

	/** all blocks in the control flow graph */
	BPatch_Set<BPatch_basicBlock*> allBlocks;

	/** entry blocks in the control flow graph */
	BPatch_Set<BPatch_basicBlock*> entryBlock;

	/** exit blocks in the control flow graph */
	BPatch_Set<BPatch_basicBlock*> exitBlock;

	bool isPrecise;

	int sourceFileLinesCount;
	FileLineCoverage** sourceFileLines;

public:
	/** friend class */
	friend class CCOnDemandInstrument;
	friend class CodeCoverage;
	
	/** friend function that is used to sort Function Coverage 
	  * objects according to the function names
	  */
	friend int FCSortByFileName(const void* arg1,const void* arg2);
	friend int FLSortByFileName(const void* arg1,const void* arg2);

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
			 const char* funcN);

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

	void addSourceFile(FileLineCoverage*);

	/** destructor of the class */
	virtual ~FunctionCoverage();
};

#endif /* _FunctionCoverage_h_ */
