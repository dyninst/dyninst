#include <stdio.h>
#include <iostream.h>
#include <stdlib.h>

#ifdef sparc_sun_solaris2_4

#include <string.h>
#include <fstream.h>
#include <limits.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <tcl.h>
#include <tk.h>
#include <pthread.h>

#endif

#ifdef sparc_sun_solaris2_4

#include <CCcommon.h>
#include <FunctionCoverage.h>
#include <CCPreInstrument.h>
#include <CCOnDemandInstrument.h>

#endif

#ifdef sparc_sun_solaris2_4

typedef struct {
	Tcl_Interp* interp;
	CodeCoverage* codeCoverage;
}ARGS;

Tk_Window mainWindow = NULL;
Tk_Window textMessage = NULL;
Tk_Window fileDisplay = NULL;
Tk_Window fileList = NULL;
Tk_Window funcList = NULL;
Tk_Window startButton = NULL;
Tk_Window refreshButton = NULL;
Tk_Window statusMessage = NULL;

bool executionStarted = false;
bool isRunning = false;

void* codeCoverageThread(void* arg){

	CodeCoverage* codeCoverage = ((ARGS*)arg)->codeCoverage;
	Tcl_Interp* interp = ((ARGS*)arg)->interp;

	int errorCode = Error_OK;

        /** insert the initial instrumentation code for the functions
          * that are selected to be instrumented for code coverage
          */
        errorCode = codeCoverage->instrumentInitial();
        if(errorCode < Error_OK){
                codeCoverage->terminate();
		return NULL;
        }

	if(interp)
		Tcl_Eval(interp,"set alreadyStarted 1");

	isRunning = true;

        /** runs the mutatee after the instrumentation */
        errorCode = codeCoverage->run();
        if(errorCode < Error_OK){
                codeCoverage->terminate();
		return NULL;
        }

	if(interp)
		Tcl_Eval(interp,"set isTerminated 1");

	return arg;
}

void exitCodeCoverage(ClientData clientData){
	CodeCoverage* codeCoverage = ((ARGS*)clientData)->codeCoverage;
	codeCoverage->terminate();
	delete codeCoverage;
	cerr << "information: terminating code coverage execution ..." << endl;
}

void startButtonHandler(ClientData clientData,XEvent* eventPtr){
	sigset_t sigs_to_block;
	if(!eventPtr)
		return;

	CodeCoverage* codeCoverage = ((ARGS*)clientData)->codeCoverage;
	if(!executionStarted){
		pthread_attr_t attr;
		pthread_t newThreadId;
		pthread_attr_init(&attr);
		if(pthread_create(&newThreadId,&attr,
				  codeCoverageThread,(void*)clientData)){
			codeCoverage->terminate();
			cerr << "ERROR : Can not create graphical"
			     << "  interface pthread..." << endl;
			exit(-1);
		}
		executionStarted = true;
		sigemptyset(&sigs_to_block);
		sigaddset(&sigs_to_block,SIGALRM);
		pthread_sigmask(SIG_BLOCK,&sigs_to_block,NULL);
	}
}

void refreshButtonHandler(ClientData clientData,XEvent* eventPtr){
	if(!eventPtr)
		return;

	CodeCoverage* codeCoverage = ((ARGS*)clientData)->codeCoverage;
	Tcl_Interp* interp = ((ARGS*)clientData)->interp;
	
	if(!isRunning){
		Tcl_Eval(interp,".fileFrame.status configure -text \
			 \"To refresh, please wait untill mutatee starts...\"");
		return;
	}

	char tclFileName[124];
	sprintf(tclFileName,"./.updateExecuted.tcl.%d",(int)getpid());
	ofstream tclFile;
	tclFile.open(tclFileName,ios::out);
	codeCoverage->getTclTkExecutedLines(tclFile);

	if(Tcl_EvalFile(interp,tclFileName) != TCL_OK){
		codeCoverage->terminate();
		cerr << "ERROR: Update execution string is not valid..."
		     << endl;
		exit(-1);
	}

	tclFile.close();
	unlink(tclFileName);
}

void prepareTclTkGlobalDataStucture(ARGS* passedArguments) {

	CodeCoverage* codeCoverage = passedArguments->codeCoverage;
	Tcl_Interp* interp = passedArguments->interp;

	char tclFileName[124];
	sprintf(tclFileName,"./.contructDS.tcl.%d",(int)getpid());
	ofstream tclFile;
	tclFile.open(tclFileName,ios::out);
	codeCoverage->getTclTkMenuListCreation(tclFile);

	if(Tcl_EvalFile(interp,tclFileName) != TCL_OK){
		codeCoverage->terminate();
		cerr << "ERROR: Menu list preparation string is not valid..."
		     << endl;
		exit(-1);
	}
	tclFile.close();
	unlink(tclFileName);
}

int Tcl_AppInit(Tcl_Interp* interp){
	return (interp ? TCL_OK : TCL_OK);
}

Tcl_Interp* initTclTk(CodeCoverage* codeCoverage,int interval){

	Tcl_Interp* interp = Tcl_CreateInterp();
	if(!interp)
		return NULL;

	if(Tcl_Init(interp) == TCL_ERROR)
		return NULL;
	if(Tk_Init(interp) == TCL_ERROR)
		return NULL;

	char buffer[124];
	sprintf(buffer,"set deletionInterval %d",interval);
	Tcl_Eval(interp,buffer);

	char tcktkFilePath[1024];
	char* p = getenv("DYNINST_ROOT");
	if(!p){
		codeCoverage->terminate();
		cerr << "ERROR: DYNINST_ROOT environment variable is not set..."
		     << endl;
		exit(-1);
	}
	sprintf(tcktkFilePath,"%s/core/codeCoverage/src/interface.tcl",p);

	if(Tcl_EvalFile(interp,tcktkFilePath) != TCL_OK){
		codeCoverage->terminate();
		cerr << "ERROR: Interface preparation script can not "
		     << "be run correctly (" << tcktkFilePath << ")..."
		     << endl;
		exit(-1);
	}

	mainWindow = Tk_MainWindow(interp);
	textMessage = Tk_NameToWindow(interp,
			        ".fileFrame.message",mainWindow);
	fileDisplay = Tk_NameToWindow(interp,
				".fileFrame.displayPanel.text",mainWindow);
	fileList = Tk_NameToWindow(interp,
				".menuFrame.listFrame.fileListFrame.list",mainWindow);
	funcList = Tk_NameToWindow(interp,
				".menuFrame.listFrame.funcListFrame",mainWindow);
	startButton = Tk_NameToWindow(interp,
				".menuFrame.buttonFrame.start",mainWindow);
	refreshButton = Tk_NameToWindow(interp,
				".menuFrame.buttonFrame.refresh",mainWindow);
	statusMessage = Tk_NameToWindow(interp,
				".menuFrame.buttonFrame.quit",mainWindow);

	if(!mainWindow || !textMessage || !fileDisplay || !fileList ||
	   !funcList || !startButton || !refreshButton || !statusMessage){
		codeCoverage->terminate();
		cerr << "ERROR: Some of the interface widgets can not be accessed..."
		     << endl;
		exit(-1);
	}

	codeCoverage->setTclTkSupport(interp,".fileFrame.status");

	return interp;
}

Tcl_Interp* initTclTkForView(){

	Tcl_Interp* interp = Tcl_CreateInterp();
	if(!interp)
		return NULL;

	if(Tcl_Init(interp) == TCL_ERROR)
		return NULL;
	if(Tk_Init(interp) == TCL_ERROR)
		return NULL;

	Tcl_Eval(interp,"set deletionInterval 0");

	char tcktkFilePath[1024];
	char* p = getenv("DYNINST_ROOT");
	if(!p){
		cerr << "ERROR: DYNINST_ROOT environment variable is not set..."
		     << endl;
		exit(-1);
	}
	sprintf(tcktkFilePath,"%s/core/codeCoverage/src/interface.tcl",p);

	if(Tcl_EvalFile(interp,tcktkFilePath) != TCL_OK){
		cerr << "ERROR: Interface preparation script can not "
		     << "be run correctly (" << tcktkFilePath << ")..."
		     << endl;
		exit(-1);
	}
	if((Tcl_Eval(interp,"destroy .menuFrame.buttonFrame.start") != TCL_OK) ||
	   (Tcl_Eval(interp,"destroy .menuFrame.buttonFrame.refresh") != TCL_OK) ||
	   (Tcl_Eval(interp,"destroy .fileFrame.status") != TCL_OK))
	{
		cerr << "ERROR: Destroying some widgets do not work poperly..." << endl;
		exit(-1);
	}

	return interp;
}

#endif

/** function to show the usage of code coverage tool 
  * one usage is to run and produce the coverage results
  * in a binary file and the other is to view the coverage results
  */
void printUsage(char* s,bool d=false){
	cerr << "Usage_1 : " << s << " [--interface] [--deletion <interval>] [--dominator] \\" << endl
	     << "                 [--ondemand] [--suffix <outfile suffix>] \\" << endl
	     << "                 --run <executable> <arguments>" << endl;
	cerr << "Usage_2 : " << s << " --view <fileName>" << endl;
	cerr << "Usage_3 : " << s << " --xview <fileName>" << endl;

	if(d)
 	  cerr 
	     << endl
	     << "Information : " << endl
	     << endl
	     << "--deletion     : Interval to delete instrumentation code in seconds." << endl
	     << "                 Default value is 0, that is no deletion of " << endl
	     << "                 instrumentation code." << endl
	     << "--dominator    : Flag to make this tool use dominator information." << endl
	     << "                 All basic blocks is used by default." << endl
	     << "--suffix       : The suffix of the output file, generated by appending" << endl
	     << "                 to the name of executable.Output file contains " << endl
	     << "                 coverage information" << endl
	     << "--ondemand     : Flag to instrument functions when called first time." << endl
	     << "                 By default, the functions with source line information" << endl
	     << "                 is pre-instrumented." << endl
	     << "--run          : The executable and its arguments to run is given after" << endl
	     << "                 this flag. This flag HAS to come after all other flags" << endl
	     << "--view         : To view the output file generated from coverage data" << endl
	     << "                 in text format. The output file is generated if it is" << endl
	     << "                 executed in Usage_1 format." << endl
	     << "--xview         : To view the output file generated from coverage data" << endl
	     << "                 using graphical interface." << endl
	     << "--interface    : To run the code coverage with its Tcl/Tk" << endl
	     << "                 based grahical user interface." << endl
	     << endl << endl;

	exit(0);
}

/** main function */
int main(int argc,char* argv[]){
#ifdef sparc_sun_solaris2_4

	bool useInterface = false;
	bool useDominator = false;
	bool useOnDemand = false;
	char* suffix = ".dyncov";
	int interval = 0;
	int execIndex = 0;
	char* p = NULL;
	int errorCode = Error_OK;
	Tcl_Interp* interp = NULL;


	if(argc < 3)
		printUsage(argv[0],true);

	if((argc == 3) && !strncmp("--view",argv[1],6)){
		return CodeCoverage::viewCodeCoverageInfo(argv[2]);
	}
	else if((argc == 3) && !strncmp("--xview",argv[1],7)){
		interp = initTclTkForView(); 
		if(!interp){
			cerr << "ERROR : The Tcl/Tk interpreter"
			     << " can not be created..." << endl; 
			exit(-1);
		}

        	char tclFileName[124];
        	sprintf(tclFileName,"./.view.tcl.%d",(int)getpid());
	        ofstream tclFile;
		tclFile.open(tclFileName,ios::out);

		errorCode = CodeCoverage::getTclTkMenuListForView(argv[2],tclFile);

		if(errorCode != Error_OK){
			cerr << "ERROR: Coverage file can not be parsed properly..."
			     << endl;
			exit(-1);
		}

		if(Tcl_EvalFile(interp,tclFileName) != TCL_OK){
			cerr << "ERROR: Menu list preparation string is not valid..."
		     	     << endl;
			exit(-1);
		}

		tclFile.close();
		unlink(tclFileName);

		Tk_Main(argc,argv,Tcl_AppInit);
		Tcl_DeleteInterp(interp);

		return errorCode;
	}

	for(int i=1;i<argc;i++){
		if(!strncmp("--del",argv[i],5)){
			i++;
			if(!strncmp("--",argv[i],2))
				printUsage(argv[0]);
			interval = strtol(argv[i],&p,10);
			if(argv[i] == p)
				printUsage(argv[0]);
		}
		else if(!strncmp("--dom",argv[i],5))
			useDominator = true;
		else if(!strncmp("--ond",argv[i],5))
			useOnDemand = true;
		else if(!strncmp("--run",argv[i],5)){
			execIndex = i+1;
			break;
		}
		else if(!strncmp("--suf",argv[i],5)){
			i++;
			if(!strncmp("--",argv[i],2))
                                printUsage(argv[0]);
			suffix = argv[i];
		}
		else if(!strncmp("--int",argv[i],5)){
			useInterface = true;
		}
		else
			printUsage(argv[0]);
	}

	if(!execIndex || (execIndex == argc))
		 printUsage(argv[0]);

        struct stat statBuffer;
        if(stat(argv[execIndex],&statBuffer) < 0){
		cerr << "ERROR : Executable " << argv[execIndex] 
		     << " does not exist" << endl;
		exit(-100);
	}

	CodeCoverage* codeCoverage = NULL;

	/** create the corresponding code coverage object */
	if(useOnDemand)
		codeCoverage = new CCOnDemandInstrument;
	else
		codeCoverage = new CCPreInstrument;

	errorCode = Error_OK;

	/** initialize the necessary BPatch obejcts */
	errorCode = codeCoverage->initialize(argv+execIndex,interval,
					     useDominator,suffix);
	if(errorCode < Error_OK){
		codeCoverage->terminate();
		exit(errorCode);
	}

	if(useInterface){
		interp = initTclTk(codeCoverage,interval); 
		if(!interp){
			codeCoverage->terminate();
			cerr << "ERROR : The Tcl/Tk interpreter"
			     << " can not be created..." << endl; 
			exit(-1);
		}
	}

	ARGS* passedArguments = new ARGS;
	passedArguments->interp = interp;
	passedArguments->codeCoverage = codeCoverage;

	if(useInterface){
		Tcl_CreateExitHandler(exitCodeCoverage,
				     (ClientData)passedArguments);

		Tk_CreateEventHandler(startButton,ButtonReleaseMask,
				      startButtonHandler,
				      (ClientData)passedArguments);

		Tk_CreateEventHandler(refreshButton,ButtonPressMask,
				      refreshButtonHandler,
				      (ClientData)passedArguments);
	}

	/** instrument a breakpoint to the beginning of the exit handle
	  * to catch the termination of the mutatee 
	  */
	errorCode = codeCoverage->instrumentExitHandle();
	if(errorCode < Error_OK){
		codeCoverage->terminate();
		Tcl_DeleteInterp(interp);
		exit(errorCode);
	}

	/** select functions whose source code line information
	  * is available in the executable 
	  */
	errorCode = codeCoverage->selectFunctions();
	if(errorCode < Error_OK){
		codeCoverage->terminate();
		Tcl_DeleteInterp(interp);
		exit(errorCode);
	}

	if(useInterface) {
		prepareTclTkGlobalDataStucture(passedArguments);
		Tk_Main(argc,argv,Tcl_AppInit);
		Tcl_DeleteInterp(interp);
	}
	else 
		codeCoverageThread((void*)passedArguments);

	delete passedArguments;

	return Error_OK;
#else
	cerr << endl
	     << "IMPORTANT Information : " << endl
	     << "\tCodeCoverage Tool is not implemented for" << endl
	     << "\tthis platform...." << endl << endl << endl;

	printUsage(argv[0],true);

	return 0;
#endif
}
