#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <iostream.h>
#include <fstream.h>
#include <limits.h>

#include "common/h/Vector.h"
#include "common/src/Dictionary.C"
#include "BPatch_Set.h"

#include "common/h/Types.h"
#include "common/h/String.h"
#include "dyninstAPI/src/LineInformation.h"

#include <CCcommon.h>
#include <FCAllBlocks.h>
#include <FCUseDominator.h>
#include <CodeCoverage.h>

/** a linked list definition of functions to be used in hash structure */
class BPFunctionList {
public:
        BPatch_function* f;
        BPatch_module* m;
        BPFunctionList* n;

        BPFunctionList(BPatch_function* argf,BPatch_module* argm) :
                f(argf),m(argm),n(NULL){}
        BPFunctionList(BPatch_function* argf,BPatch_module* argm,
                       BPFunctionList* argn) : f(argf),m(argm),n(argn){}
};

/** mapping from function name to linked list of functions with the same name */
dictionary_hash<string,BPFunctionList*>* allFunctionsHash = NULL;

/** static initialization of the global code coverage object used
  * by interval call backs
  */
CodeCoverage* CodeCoverage::globalObject = NULL;

/** constructor */
CodeCoverage::CodeCoverage()
	: appThread(NULL),appImage(NULL),coverageFileName(NULL),
	  deletionInterval(0),appModules(NULL),
	  instrumentedFunctions(NULL),instrumentedFunctionCount(0),
	  useDominator(false)
{}

/** constructor */
CodeCoverage::~CodeCoverage()
{
	delete[] coverageFileName;
	delete appModules;
	for(int i=0;i<instrumentedFunctionCount;i++)
		delete instrumentedFunctions[i];
	delete[] instrumentedFunctions;
	delete FILE_EXTENSION;
}

/** error printing function that overrides dyninst default 
  * function
  */
void codeCoverageError(BPatchErrorLevel level,
                       int num,const char **params)
{ }

/** this method initializes the necessary data structures and
  * creates a map from string name to the linked list of function
  * records to be used later to access function faster
  */
int CodeCoverage::initialize(char* mutatee[],unsigned short interval,
			     bool dominatorInfo,const char* suffix)
{
	deletionInterval = interval;
	useDominator = dominatorInfo;
	FILE_EXTENSION = new char[strlen(suffix)+1];
	strcpy(FILE_EXTENSION,suffix);

	coverageFileName = new char[strlen(mutatee[0])+strlen(FILE_EXTENSION)+1];
	strcpy(coverageFileName,mutatee[0]);
	strcat(coverageFileName,FILE_EXTENSION);

	registerErrorCallback(codeCoverageError);

	/** create the process */
	appThread = bPatch.createProcess(mutatee[0],mutatee);
	if(!appThread)
		return errorPrint(Error_ThreadCreate);

	/** get the image */
	appImage = appThread->getImage();
	if(!appImage)
		return errorPrint(Error_ImageCreate);

	allFunctionsHash = 
		new dictionary_hash<string,BPFunctionList*>(string::hash);

	if(!allFunctionsHash)
		return errorPrint(Error_HashCreate);

	/** get the modules in the image */
	appModules = appImage->getModules();

	if(!appModules)
		return errorPrint(Error_ModuleCreate);
	
	/** for each module in the image get the functions and
	  * insert them into the map
	  */
	for(unsigned int i=0;i<appModules->size();i++){
		BPatch_module* m = (*appModules)[i];
		char mName[1024];
		m->getName(mName,1023);
		BPatch_Vector<BPatch_function*>* fs = m->getProcedures();
		for(unsigned int j=0;j<fs->size();j++){
			BPatch_function* f = (*fs)[j];
			char fName[1023];
			f->getMangledName(fName,1023); fName[1023] = '\0';
			if(allFunctionsHash->defines(string(fName))){
				/** if already there add to the front */
				BPFunctionList* fl = (*allFunctionsHash)[fName];
				(*allFunctionsHash)[string(fName)] = 
					new BPFunctionList(f,m,fl);
			}
			else
				/** create a new linked list */
				(*allFunctionsHash)[string(fName)] = 
					new BPFunctionList(f,m);
		}
		delete fs;
	}
				
	/** set the base trampoline deletion to true to delete
	  * base trampolines when there is no more instrumentation code
	  * at the point
	  */
	bPatch.setBaseTrampDeletion(true);

	return Error_OK;
}

/** method that validates whether the function has source line
  * information available. To do that, the records from the source
  * line information stab is compared with the functions with the same
  * name and its properties (if there are more than 1 possible functions
  * with the same name 
  */
BPatch_function* CodeCoverage::validateFunction(const char* funcN,
						unsigned long min)
{

	BPatch_function* currFunc = NULL;
	
	/** get the possible functions with the same name */
	BPFunctionList* possibleFunctions = NULL;
	if(allFunctionsHash->defines(string(funcN)))
		possibleFunctions = (*allFunctionsHash)[string(funcN)];

	if(!possibleFunctions)
		return NULL;

	/** for each possible function compare the source code line info
	  * record with the properties of the function. If matches
	  * then terminate otherwise continue
	  */
	for(;possibleFunctions;possibleFunctions=possibleFunctions->n){
		Address fb=(Address)(possibleFunctions->f->getBaseAddr());
		Address fe=fb + possibleFunctions->f->getSize();
		if((fb <= min) && (min <= fe)){
			currFunc = possibleFunctions->f;
			break;
		}
	}

	if(!currFunc)
		return NULL;

	return currFunc;
}

/** creates a function coverage object according to the 
  * the nature of instrumentation, tthat is whether all basic block
  * instrumentation will be used or dominator tree information
  * will be used 
  */
FunctionCoverage* CodeCoverage::newFunctionCoverage(BPatch_function* f,
				 const char* funcN,const char* fileN)
{
	FunctionCoverage* ret = NULL;
	if(useDominator)
		ret = new FCUseDominator(f,appThread,appImage,funcN,fileN);
	else
		ret = new FCAllBlocks(f,appThread,appImage,funcN,fileN);
	return ret;
}

/** method to select the functions whose source line information is available
  * and whose data is stored in dyninst. It goes over the line information 
  * strucute elements and for each source file and function it check whether
  * the source line info record matches the properties of the function in dyninst
  * If they match it is added to be instrumented. This method also creates and
  * initializes the data structures that will be used for function coverage
  */
int CodeCoverage::selectFunctions(){

	BPatch_Set<BPatch_function*> instFunctions;
	BPatch_Set<FunctionCoverage*> available;

	for(unsigned int i=0;i<appModules->size();i++){
		BPatch_module* appModule = (*appModules)[i];
		char mName[1024]; 
		appModule->getName(mName,1023);

		/** if the modules are the dynsint introduced ones skip */
		if(!strcmp(mName,"DEFAULT_MODULE") ||
		   !strcmp(mName,"DYN_MODULE") ||
		   !strcmp(mName,"LIBRARY_MODULE"))
			continue;

		LineInformation* linfo = appModule->getLineInformation();

		unsigned short sourceFileCount = linfo->getSourceFileCount();
		string** sourceFileList = linfo->getSourceFileList();
		FileLineInformation** lineInformationList = 
				linfo->getLineInformationList();

		/** for each source file in the mutatee */
		for(unsigned int j=0;j<sourceFileCount;j++){
			string fileN = *(sourceFileList[j]);
			FileLineInformation* fInfo = lineInformationList[j];

			unsigned short functionCount = fInfo->getFunctionCount();
			string** functionNameList = fInfo->getFunctionNameList();

			/** for each function record in the muattee stab records */
			for(unsigned int k=0;k<functionCount;k++){
				string funcN = *(functionNameList[k]);

				/** get the starting address of the function in stab records */
				Address min=0,max=0;
				if(!fInfo->getMinMaxAddress(k,min,max))
					continue;

				/** check whether stab record matches the dyninst records */
				BPatch_function* currFunc = 
					validateFunction(funcN.string_of(),min);

				if(!currFunc)
					continue;

				/** if already not created create the function 
				  * coverage object for the function 
				  */
				if(!instFunctions.contains(currFunc)){
					instFunctions += currFunc;
					available += 
						newFunctionCoverage(
							currFunc,
							funcN.string_of(),
							fileN.string_of());
				}
			}
		}
	}

	if(!available.size())
		return errorPrint(Error_NoFunctionsToCover);
		
	/** creates the necessary data structures and initializes them */
	instrumentedFunctionCount = available.size();
	instrumentedFunctions = new FunctionCoverage*[available.size()];
	available.elements(instrumentedFunctions);

	cout << "information: " << instrumentedFunctionCount 
	     << " functions are selected to be instrumented..." << endl;

	/** sort the function coverage objects according to the name of the functions */
	qsort((void*)instrumentedFunctions,
	      instrumentedFunctionCount,sizeof(FunctionCoverage*),FCSort);

	delete allFunctionsHash;

	return Error_OK;
}

/** method to do initial instrumentation */
int CodeCoverage::instrumentInitial(){
	return Error_OK;
}

/** method to run the mutatee */
int CodeCoverage::run(){
	return Error_OK;
}

/** method to instrument exit handle to detect the
  * termination of the mutatee
  */
int CodeCoverage::instrumentExitHandle()
{
	BPatch_breakPointExpr breakExpr;
        BPatch_Vector<BPatch_point*>* breakPoints = NULL;
        BPatchSnippetHandle* ret = NULL;

	/** _exithandle is the exit function to be called for sparc*/
        breakPoints = appImage->findProcedurePoint("_exithandle",BPatch_entry);

	if(!breakPoints)
		return errorPrint(Error_ProcedurePoint,"Entry to _exithandle");

	ret = appThread->insertSnippet(breakExpr,*breakPoints,
				       BPatch_callBefore,BPatch_lastSnippet);

	if(!ret)
		return errorPrint(Error_InsertSnippet,"Breakpoint to _exithandle");

	delete breakPoints;

	return Error_OK;
}

/** method to print the error codes for this class */
int CodeCoverage::errorPrint(int code,char* text)
{
	cerr << "Error(" << code << ") : ";

	switch(code){
		case Error_FileOpen:
			cerr << "File can not be opened. ";
			break;
		case Error_ThreadCreate:
			cerr << "The bpatch thread can not be created. ";
			break;
		case Error_ImageCreate:
			cerr << "The bpatch image can not be created. ";
			break;
		case Error_HashCreate:
			cerr << "Buffer for possible intrumentable functions can not be created. ";
			break;
		case Error_ModuleCreate:
			cerr << "Modules in the image can not be created. ";
			break;
		case Error_NoFunctionsToCover:
			cerr << "There are no functions/line information to test for source coverage. ";
			break;
		case Error_DeletionInterval:
			cerr << "An error occurred in deletion interval. ";
			break;
		case Error_PrintResult:
			cerr << "Coverage results can not be printed. ";
			break;
		case Error_FileFormat:
			cerr << "Coverage file is not in valid format. ";
			break;
		default: cerr << "Unrecognized error!!!!";
	}

	if(text)
		cerr << endl << "\t[ " << text << " ]";

	cerr << endl;

	return code;
}

void CodeCoverage::terminate(){
	if(appThread)
		appThread->terminateExecution();
}

/** method to be called during the deletion intervals */
int CodeCoverage::deletionIntervalCallback(){
	return Error_OK;
}

/** function that is used to sort the function coverage objects according
  * to the names of the functions
  */
int FCSort(const void* arg1,const void* arg2){
	FunctionCoverage* e1 = *((FunctionCoverage* const *)arg1);
	FunctionCoverage* e2 = *((FunctionCoverage* const *)arg2);

	int check = strcmp(e1->functionName,e2->functionName);
	if(check > 0)
		return 1;
	if(check < 0)
		return -1;

	check = strcmp(e1->fileName,e2->fileName);
	if(check > 0)
		return 1;
	if(check < 0)
		return -1;
	return 0;
}

/** method to print the coverage results to a binary file.
  * it iterates over the function coverage objects and prints 
  * the results for each of them.
  */
int CodeCoverage::printCoverageInformation(){

	/** update the execution counts for the last time */
        updateFCObjectInfo();

	/** create the coverage results file */
        coverageFile.open(coverageFileName,ios::out);
        if(!coverageFile)
                return errorPrint(Error_FileOpen,coverageFileName);

	/** write the unique identifier for the file format */
	char* ccid = "Dyncov-1.0";
	coverageFile.write(ccid,10);

	/** for each function coverage print the results */
        for(int i=0;i<instrumentedFunctionCount;i++)
                instrumentedFunctions[i]->printCoverageInformation(coverageFile,isInstrumented(i));

	/** print the termination flag */
	unsigned tmp_u = 0;
	coverageFile.write((char*)&tmp_u,sizeof(unsigned));

	/** close the output file */
        coverageFile.close();

        return Error_OK;
}

/** method that updates execution counts of each basic block 
  * instrumented going over the function coverage objects
  */
int CodeCoverage::updateFCObjectInfo(){
        for(int i=0;i<instrumentedFunctionCount;i++)
             if(isInstrumented(i))
                instrumentedFunctions[i]->updateExecutionCounts();
        return Error_OK;
}

/** method that returns whether a function is instrumented or not */
bool CodeCoverage::isInstrumented(int i){
        return true;
}

/** method to register the error function for dyninst error call back */
BPatchErrorCallback
CodeCoverage::registerErrorCallback(BPatchErrorCallback f){
	return bPatch.registerErrorCallback(f);
}

/** function that is used as interval call back. It is needed
  * as signal handler can not be a member function
  */
void intervalCallback(int signalNo){
	if(signalNo != SIGALRM)
		return;
	CodeCoverage::globalObject->deletionIntervalCallback();
}

/** method to print the coverage results after reading
  * from the binary file produced by code coverage tool.
  */
int CodeCoverage::viewCodeCoverageInfo(char* fN){

	unsigned tmp_u;
	unsigned short tmp_s;
	char buffer[1024];

	ifstream inputFile;
	inputFile.open(fN,ios::in);

	if(!inputFile)
		return errorPrint(Error_FileOpen,fN);

	char ccid[10];
	inputFile.read(ccid,10);
	if(strncmp(ccid,"Dyncov-1.0",10))
		return errorPrint(Error_FileFormat,fN);

	while(true){

		inputFile.read((char*)&tmp_u,sizeof(unsigned));
		if(!tmp_u)
			break;

		cout << "# # # # # # # # # # # # # # # # # # # "
		     << "# # # # # # # # # # # # # # # # # # #" << endl; 
		inputFile.read(buffer,tmp_u);buffer[tmp_u] = '\0';
		cout << "** Function  :  " << buffer << endl;

		inputFile.read((char*)&tmp_u,sizeof(unsigned));
		inputFile.read(buffer,tmp_u);buffer[tmp_u] = '\0';
		cout << "** File      :  " << buffer << endl;

		cout << "** Executed  : ";
		inputFile.read((char*)&tmp_u,sizeof(unsigned));
		unsigned nofe = tmp_u;
		for(unsigned i=1;i<=tmp_u;i++){
			inputFile.read((char*)&tmp_s,sizeof(unsigned short));
			cout << " " << tmp_s;
			if(!(i % 10) && (i < tmp_u))
				cout << endl << "              ";
		}

		cout << endl << "** UnExecuted: ";
		inputFile.read((char*)&tmp_u,sizeof(unsigned));
		unsigned nofu = tmp_u;
		for(unsigned i=1;i<=tmp_u;i++){
			inputFile.read((char*)&tmp_s,sizeof(unsigned short));
			cout << " " << tmp_s;
			if(!(i % 10) && (i < tmp_u))
				cout << endl << "              ";
		}

		cout << endl << "[ Percentage : ";
		if(nofe+nofu) 
			cout << ((float)nofe/(nofe+nofu))*100;
		else
			cout << "0.0";
		cout << " % ]" << endl << endl;
	}

	inputFile.close();

	return Error_OK;
}

