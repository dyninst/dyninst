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

#include "common/h/Vector.h"
#include "common/src/Dictionary.C"
#include "BPatch_Set.h"

#include "common/h/Types.h"
#include "common/h/String.h"

#include <CCcommon.h>
#include <CodeCoverage.h>
#include <FunctionCoverage.h>

/** constructor */
FunctionCoverage::FunctionCoverage() 
		: bpFunction(NULL),appThread(NULL),appImage(NULL),cfg(NULL),
		  instrumentationCount(0),instrumentedBlock(NULL),
		  blockVariable(NULL),instrumentationCode(NULL),
		  executionCounts(NULL),functionName(NULL),
		  fileName(NULL),id(-1),isPrecise(true),
		  lineCount(0),executionPercentage(0.0) 
{
	pthread_mutex_init(&updateLock,NULL);
}

/** constructor */
FunctionCoverage::FunctionCoverage(BPatch_function* f,BPatch_thread* t,
				   BPatch_image* i,
				   const char* funcN,const char* fileN) 
		: bpFunction(f),appThread(t),appImage(i),cfg(NULL),
		  instrumentationCount(0),instrumentedBlock(NULL),
		  blockVariable(NULL),instrumentationCode(NULL),
		  executionCounts(NULL),id(-1),isPrecise(true),
		  lineCount(0),executionPercentage(0.0)
{
	functionName = funcN;
	fileName = fileN;
	pthread_mutex_init(&updateLock,NULL);
}

/** method to set identifier of the object It is unique among
  * objects of this class
  */
void FunctionCoverage::setId(int i){
	id = i;
}

/** method to create control flow graph of the function
  * represented by this object
  */
int FunctionCoverage::createCFG(){
	cfg = bpFunction->getCFG();
	if(!cfg)
		return errorPrint(Error_CFGCreate);

	cfg->getAllBasicBlocks(allBlocks);

	BPatch_Vector<BPatch_basicBlock*> enB; 
	cfg->getEntryBasicBlock(enB);
	for(unsigned int i=0;i<enB.size();i++)
		entryBlock += enB[i];

	BPatch_Vector<BPatch_basicBlock*> exB; 
        cfg->getExitBasicBlock(exB);
	for(unsigned int i=0;i<exB.size();i++)
		exitBlock += exB[i];


	/** source line infomation is created */
	cfg->createSourceBlocks();

	return Error_OK;
}

/** this method inserts break point to the beginning of the
  * function and also an assigment which assigns the identifier of the
  * function to the variable given as an argument
  */
BPatchSnippetHandle* 
FunctionCoverage::insertBreakPointatEntry(BPatch_variableExpr* v)
{
	BPatch_Vector<BPatch_snippet*> codeList;
	BPatch_breakPointExpr *bp = new BPatch_breakPointExpr();
	BPatch_arithExpr *whichbp = new BPatch_arithExpr(BPatch_assign,
                                 		       *v,BPatch_constExpr(id));
        codeList.push_back(whichbp);
        codeList.push_back(bp);

        BPatch_Vector<BPatch_point*>* breakPoints = NULL;
        breakPoints = bpFunction->findPoint(BPatch_entry);

	BPatchSnippetHandle* ret = NULL;
        if(breakPoints){
                ret = appThread->insertSnippet(BPatch_sequence(codeList),
                                         *breakPoints,
                                         BPatch_callBefore,BPatch_lastSnippet);
                delete breakPoints;
        }
        delete bp;
        delete whichbp;
	return ret;
}

/** Method to instrument selected basic blocks. For each basic block
  * that is instrumented a variable is assigned and the instrumentation code
  * increments this variable at the beginning of the basic block
  * If the basic block is one of the entry point or exit points 
  * already existing point are used for instrumentation.
  */
int FunctionCoverage::instrumentPoints(){

	cout << "information: " << functionName
	     << " is being instrumented..." << endl;

	/** for each basic block */
	for(int i=0;i<instrumentationCount;i++){
		BPatch_basicBlock* bb = instrumentedBlock[i];

		/** create the code to increment its variable */
		BPatch_arithExpr increment(
                        BPatch_assign,*(blockVariable[i]),
                        BPatch_arithExpr(BPatch_plus,*(blockVariable[i]),
                                         BPatch_constExpr(1)));

		BPatch_Vector<BPatch_point*>* bpp = NULL;

		BPatch_callWhen callWhen = BPatch_callBefore;

		/** find the instrumentation point or create one arbitrary */
		if(entryBlock.contains(bb))
			bpp = bpFunction->findPoint(BPatch_entry);
		else if(exitBlock.contains(bb)){
			bpp = bpFunction->findPoint(BPatch_exit);
			callWhen = BPatch_callAfter;
		}
		else {
			bpp = new BPatch_Vector<BPatch_point*>;
			BPatch_point *p = NULL, *q = NULL; 
			void *startAddress=NULL,*endAddress=NULL;

			bb->getAddressRange(startAddress,endAddress);

			/** create the instrumentation point (arbitrary) */
			p = appImage->createInstPointAtAddr(startAddress,&q);

			if(p)
				bpp->push_back(p);
			else if(q)
				/** if can not be created but overlaps with other 
				  * use the overlapping one
				  */
				bpp->push_back(q);
			else{
				delete bpp;
				isPrecise = false;
				continue;
			}
		}

		/** insert the instrumentation code */
		if(bpp)
		instrumentationCode[i] = 
				appThread->insertSnippet(increment,*bpp,
							 callWhen,
							 BPatch_lastSnippet);
		if(!instrumentationCode[i])
			isPrecise = false;
		delete bpp;
	}
	if(!isPrecise)
		cout << "\tinformation: " << functionName << " results may"
		     << " not be precise..." << endl;
	return Error_OK;
}

/** method that selects the basic blocks to be instrumented
  * After selecting the basic blocks to be instrumented it initializes
  * some data strucutes to store the variables, executions counts, etc.
  * The basic blocks with single instructions are not instrumented
  */
int FunctionCoverage::selectInstrumentationPoints(){

	BPatch_Set<BPatch_basicBlock*> tobeInstrumented;

	int errorCode = Error_OK;

	/** create control flow graph */
	errorCode = createCFG();
	if(errorCode < Error_OK)
		return errorCode;

	/** if needed fill in the dominator information */
	fillDominatorInfo();

	BPatch_basicBlock** elements = 
			new BPatch_basicBlock*[allBlocks.size()];
	allBlocks.elements(elements);

	executionCounts = new int[allBlocks.size()];

	for(int i=0;i<allBlocks.size();i++){
		BPatch_basicBlock* bb = elements[i];
		executionCounts[i] = 0;

		void *startAddress=NULL,*endAddress=NULL;
		bb->getAddressRange(startAddress,endAddress);

		/** if the basic block does not need to be instrumented
		  * or even though it is needed it has only single instruction
		  * then do not select
		  */
		if(!validateBasicBlock(bb))
			continue;
		else if(!((char*)endAddress-(char*)startAddress)){
			isPrecise = false;
			continue;
		}

		/** otherwise selct to be instrumented */
		tobeInstrumented += bb;
	}

	instrumentationCount = tobeInstrumented.size();

	/** if there is no possible selection for this method return */
	if(!instrumentationCount)
		return Error_OK;

	/** create the necessary data structures to store info about 
	  * executions of basic blocks
	  */
	instrumentedBlock = new BPatch_basicBlock*[instrumentationCount];
	blockVariable = new BPatch_variableExpr*[instrumentationCount];
	instrumentationCode = new BPatchSnippetHandle*[instrumentationCount];

	/** initialize the data strucutes created */
	tobeInstrumented.elements(elements);
	for(int i=0;i<instrumentationCount;i++){
		instrumentedBlock[i] = elements[i];
		instrumentationCode[i] = NULL;
		int tmpV = 0;
		blockVariable[i] = 
			appThread->malloc(*appImage->findType("int"));
        	blockVariable[i]->writeValue((void*)&tmpV);
	}

	delete[] elements;

	return Error_OK;
}

/** method to fill dominator information if needed */
void FunctionCoverage::fillDominatorInfo(){
}

/** method to decide whether it is necessary a basic block will
  * be instrumented or not 
  */ 
bool FunctionCoverage::validateBasicBlock(BPatch_basicBlock* bb){
	if(bb)
		return true;
	return false;
}

/** method that prints the error codes of this class */
int FunctionCoverage::errorPrint(int code,char* text=NULL){
	cerr << "Error(" << code << ") : ";

	switch(code){
		case Error_CFGCreate:
			cerr << "Control flow graph for the function can not be created. ";
			break;
		default: cerr << "Unrecognized error!!!!";
	}

	if(text)
		cerr << endl << "\t[ " << text << " ]";

	cerr << endl;

	return code;
}

/** destructor of the class */
FunctionCoverage::~FunctionCoverage() {
	delete cfg;
	delete[] instrumentedBlock;
	delete[] blockVariable;
	delete[] instrumentationCode;
	delete[] executionCounts;
	pthread_mutex_destroy(&updateLock);
}

/** method to print the coverage results of the function it represents 
  * into a binary file generated by concatenation of the given suffix
  * to the executable name
  */
int FunctionCoverage::printCoverageInformation(ofstream& cf)
{
	unsigned tmp_u;
	unsigned short tmp_s;

	tmp_u = strlen(functionName);
	cf.write((char*)&tmp_u,sizeof(unsigned));
	cf.write(functionName,tmp_u);

	tmp_u = strlen(fileName);
	cf.write((char*)&tmp_u,sizeof(unsigned));
	cf.write(fileName,tmp_u);

//	if(!isInst || !allBlocks.size()){
//		tmp_u = 0;
//		cf.write((char*)&tmp_u,sizeof(unsigned));
//		cf.write((char*)&tmp_u,sizeof(unsigned));
//		return Error_OK;
//	}

	unsigned short* lelements = NULL;

	tmp_u = executedLines.size();
	cf.write((char*)&tmp_u,sizeof(unsigned));

	if(tmp_u){
		lelements = new unsigned short[tmp_u];
		executedLines.elements(lelements);
		for(unsigned int i=0;i<tmp_u;i++){
			tmp_s = lelements[i];
			cf.write((char*)&tmp_s,sizeof(unsigned short));
		}
		delete[] lelements;
	}

	pthread_mutex_lock(&updateLock);
	tmp_u = unExecutedLines.size();
	cf.write((char*)&tmp_u,sizeof(unsigned));

	if(tmp_u){
		lelements = new unsigned short[tmp_u];
		unExecutedLines.elements(lelements);
		for(unsigned int i=0;i<tmp_u;i++){
			tmp_s = lelements[i];
			cf.write((char*)&tmp_s,sizeof(unsigned short));
		}
		delete[] lelements;
	}
	pthread_mutex_unlock(&updateLock);

	return Error_OK;
}

/** method that updates execution count for a basic block */
int FunctionCoverage::updateExecutionCounts(BPatch_basicBlock* bb,int ec){
	if(bb && ec)
        	return Error_OK;
	return Error_OK;
}

/** method to update execution counts of the basic blocks */
int FunctionCoverage::updateExecutionCounts(){

	/** for each instrumented basic block */
	for(int i=0;i<instrumentationCount;i++){
		if(!instrumentedBlock[i])
			continue;
		BPatch_basicBlock* bb = instrumentedBlock[i];
		BPatch_variableExpr* bv = blockVariable[i];

		/** read the execution count */
		int ec = 0;
		if(bv) bv->readValue((void*)&ec);
		if(!ec)
			continue;

		/** using the basic block and execution count
		  * update necessary basic block execution counts.
		  */
		updateExecutionCounts(bb,ec);

		/** delete the instrumentation code for the basic block
		  * since it does not produce any useful information
		  * after it has been executed once
		  */
		
		if(instrumentationCode[i]){
			CodeCoverage::globalObject->totalDeletions++;
			appThread->deleteSnippet(instrumentationCode[i]);
		}

		instrumentedBlock[i] = NULL;
	}

	if(lineCount){
		pthread_mutex_lock(&updateLock);

		executionPercentage =  executedLines.size();
		executionPercentage /= lineCount;
		executionPercentage *= 100;

		pthread_mutex_unlock(&updateLock);
	}

	return Error_OK;
}

void FunctionCoverage::initializeLines(BPatch_Set<unsigned short>& lines){
	unExecutedLines |= lines;
	lineCount = lines.size();
}

int FunctionCoverage::updateLinesCovered(BPatch_sourceBlock* sb)
{
	if(sb){
		pthread_mutex_lock(&updateLock);

		BPatch_Vector<unsigned short> lines;
		sb->getLines(lines);
		for(unsigned int j=0;j<lines.size();j++){
			if(!executedLines.contains(lines[j]))
				CodeCoverage::globalObject->totalCoveredLines++;
			executedLines += lines[j];
			unExecutedLines.remove(lines[j]);
		}

		pthread_mutex_unlock(&updateLock);
	}
	return Error_OK;
}
