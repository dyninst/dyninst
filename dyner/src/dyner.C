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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tcl.h>

#if defined(i386_unknown_nt4_0)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <winbase.h>
#else
#include <unistd.h>
#include <signal.h>
#endif

#include "dynerList.h"
#include "BPatch.h"
#include "BPatch_type.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"
#include "breakpoint.h"

extern "C" {
#if !defined(i386_unknown_nt4_0)
#if !defined(i386_unknown_linux2_0)
#if !defined(ia64_unknown_linux2_4)
	int usleep(useconds_t);
#endif
#endif
#endif

	void set_lex_input(char *s);
	int dynerparse();
}

bool stopFlag = false;
extern int dynerdebug;

int debugPrint = 0;
BPatch_point *targetPoint;
bool errorLoggingOff = false;
bool verbose = false;

// control debug printf statements
#define dprintf	if (debugPrint) printf

class ListElem {
public:
    int			     number;
    char		     *function;
    BPatch_procedureLocation where;
    BPatch_callWhen	     when;
    BPatchSnippetHandle	     *handle;
};

class BPListElem: public ListElem {
public:
    char	         *condition;
    int			     lineNum;

    BPListElem(int _number, const char *_function,
           BPatch_procedureLocation _where,
	       BPatch_callWhen _when, const char *_condition,
	       BPatchSnippetHandle *_handle, int _lineNum);
    ~BPListElem();
    void Print();
};

class runtimeVar {
public:
    runtimeVar(BPatch_variableExpr *v, const char *n) { 
        var = v, name = strdup(n);
    }
    BPatch_variableExpr *var;
    char *name;
    bool readValue(void *buf) { return var->readValue(buf); }
};

typedef enum { NORMAL, TRACE, COUNT } InstPointType;

class IPListElem: public ListElem {
public:
    char		     *statement;
    InstPointType 	     instType;

    IPListElem(int _number, const char *_function,
           BPatch_procedureLocation _where,
	       BPatch_callWhen _when, const char *_condition,
	       BPatchSnippetHandle *_handle, InstPointType _instType);
    ~IPListElem();
    void Print();
};

BPatch *bpatch;
BPatch_thread *appThread = NULL;
BPatch_image *appImage = NULL;
static BPatch_variableExpr *bpNumber = NULL;
static int bpCtr = 1;
static int ipCtr = 1;
static DynerList<BPListElem *> bplist;
static DynerList<IPListElem *> iplist;
static DynerList<runtimeVar *> varList;
int whereAmINow = -1;/*ccw 10 mar 2004 : this holds the index of the current stackframe for where, up, down*/

#if !defined(i386_unknown_nt4_0)

//Ctrl-C signal handler
void  INThandler(int sig)
{
     signal(sig, SIG_IGN);
     signal(SIGINT, INThandler);
     stopFlag = true;
}
#endif

bool name2loc(const char *s, BPatch_procedureLocation &where,
	      BPatch_callWhen &when)
{
    if (!strcmp(s, "entry")) {
	where = BPatch_entry;
	when = BPatch_callBefore;
    } else if (!strcmp(s, "exit")) {
	where = BPatch_exit;
	/* This is not supported anymore!
	when = BPatch_callBefore;
	*/
	when = BPatch_callAfter;
    } else if (!strcmp(s, "preCall")) {
	where = BPatch_subroutine;
	when = BPatch_callBefore;
    } else if (!strcmp(s, "postCall")) {
	where = BPatch_subroutine;
	when = BPatch_callAfter;
    } else {
	return false;
    }
    return true;
}

char *loc2name(BPatch_procedureLocation where, BPatch_callWhen when)
{
    switch (where) {
      case BPatch_entry:
	return "entry";
      case BPatch_exit:
	return "exit";
      case BPatch_subroutine:
	if (when == BPatch_callBefore) return "preCall";
	else return "postCall";
      default:
	return "<error>";
    };
}

BPListElem::BPListElem(int _number, const char *_function,
    BPatch_procedureLocation _where, BPatch_callWhen _when,
    const char *_condition,
    BPatchSnippetHandle *_handle, int _lineNum)
{
    number = _number;
    function = strdup(_function);
    where = _where;
    when = _when;
    if (_condition)
    	condition = strdup(_condition);
    else
	condition = NULL;
    handle = _handle;
    lineNum = _lineNum;
}

BPListElem::~BPListElem()
{
    free(function);
    if (condition) free(condition);
}

void BPListElem::Print()
{
    printf("%2d: ", number);
    if (lineNum) 
	printf("%s:%d", function, lineNum);
    else
	printf("%s (%s)", function,
		loc2name(where, when));

    if (condition)
	printf(", condition %s\n", condition);
    else
	printf("\n");
}

IPListElem::IPListElem(int _number, const char *_function,
    BPatch_procedureLocation _where, BPatch_callWhen _when, 
    const char *_statement,
    BPatchSnippetHandle *_handle, InstPointType _instType)
{
    if (_instType == NORMAL)
    	number = _number;
    else
	number = -1;
    function = strdup(_function);
    where = _where;
    when = _when;
    statement = strdup(_statement);
    handle = _handle;
    instType = _instType;
}

IPListElem::~IPListElem()
{
    free(function);
    free(statement);
    if (handle) {
	if (appThread)
		appThread->deleteSnippet(handle);

	delete handle;
    }
}

void IPListElem::Print()
{
    printf("%2d: %s (%s)-->%s\n", number, function, loc2name(where, when), statement);
}

BPListElem *removeBPlist(int n)
{
  DynerList<BPListElem *>::iterator i;
  BPListElem *ret = NULL;
  for (i = bplist.begin(); i != bplist.end(); i++) {
    if ((*i)->number == n) {
      ret = *i;
      bplist.erase(i);
      break;
    }
  }
  return NULL;
}

BPListElem *findBP(int n)
{
    DynerList<BPListElem *>::iterator i;

    for (i = bplist.begin(); i != bplist.end(); i++) {
	if ((*i)->number == n) return (*i);
    }

    return NULL;
}

IPListElem *removeIP(int n)
{
    DynerList<IPListElem *>::iterator i;
    IPListElem *ret = NULL;

    for (i = iplist.begin(); i != iplist.end(); i++) {
	if ((*i)->number == n) {
	  ret = *i;
	  iplist.erase(i);
	  break;
	}
    }

    return ret;
}

IPListElem *findIP(int n)
{
    DynerList<IPListElem *>::iterator i;

    for (i = iplist.begin(); i != iplist.end(); i++) {
	if ((*i)->number == n) return (*i);
    }

    return NULL;
}

#define LIMIT_TO(name) if (argc < 2 || !strncmp(argv[1], name, strlen(name)))

int help(ClientData, Tcl_Interp *, int argc, TCLCONST char **argv)
{

    LIMIT_TO("at") {
	printf("at <function> [entry|exit|preCall|postCall] <statement> - insert statement\n");
	printf("at termination <statement> - Execute <statement> at program exit callback\n");
    }

    LIMIT_TO("attach") {
	printf("attach <pid> <program> - attach dyner to a running program\n");
    }

    LIMIT_TO("declare") {
	printf("declare <type> <variable> - create a new variable of type <type>\n");
    }

    LIMIT_TO("delete") {
	printf("deletebreak <breakpoint number ...> - delete breakpoint(s)\n");
	printf("deleteinst <instpoint number ...> - delete intrumentation point(s)\n");
    }

    LIMIT_TO("break") {
	printf("break <function> [entry|exit|preCall|postCall] [<condition>] - set a (conditional)\n");
	printf("     break point at specified points of <function>\n");
	printf("break <file name:line number> [<condition>] - set an arbitrary (conditional) break\n");
	printf("     point at <line number> of <file name>\n");
    }

    LIMIT_TO("debugparse") {
	printf("debugparse [enable | disable] - Turn on/off debug parsing of the mutatee programs\n");
    }

    LIMIT_TO("execute") {
	printf("execute <statement> - Execute <statement> at the current point\n");
    }

    LIMIT_TO("list") {
	printf("listbreak - list break points\n");
	printf("listinst - list intrumentation points\n");
    }
	
    LIMIT_TO("load") {
	printf("load <program> [arguments] [< filename] [> filename] - load a program\n");
	printf("load library <lib name> - load a dynamically linked library\n");
	printf("load source <C++ file name> - Create a dynamically linked library from a \n");
	printf("     C++ source file and load it to address space. All the functions and variables \n");
	printf("     in the source file will be available for instrumentation\n");
    }

    LIMIT_TO("run") {
	printf("run - run or continue the loaded program\n");
    }

    LIMIT_TO("show") {
	printf("show [modules|functions|variables] - display module names, global functions\n");
	printf("      and variables\n");
	printf("show functions in <module> - display function names declared in <module>\n");
	printf("show [parameters|variables] in <function> - display local variables or parameters of\n");
	printf("     <function>\n");
    }

    LIMIT_TO("count") {
	printf("count <function> - At the end of execution display how many times <function> is called\n");
    }

    LIMIT_TO("replace") {
	printf("replace function <function1> with <function2> - Replace all calls to <function1> with\n");
	printf("     calls to <function2>\n");
	printf("replace call <function1>[:n] with <function2> - All the calls or the nth function call \n");
	printf("     in <function1> are/is changed to the function <function2>\n");
    }

    LIMIT_TO("trace") {
	printf("trace function <function> - Print a message at the entry and exit of <function>\n");
	printf("trace functions in <module> - Print a message at the entry and exit of all functions\n");
	printf("     declared in <module>\n");
    }

    LIMIT_TO("untrace") {
	printf("untrace function <function> - Undo the effects of trace command for <function>\n");
	printf("untrace functions in <module> - Undo the effects of trace command for all functions\n");
	printf("     declared in <module>\n");
    }

    LIMIT_TO("mutations") {
	printf("mutations [enable|disable] - Enable or disable the execution of snippets\n");
    }

    LIMIT_TO("removecall") {
	printf("removecall <function>[:n] - Remove all the calls or n'th call in <function>\n");
    }

    LIMIT_TO("detach") {
	printf("detach - remove all the code inserted into target program\n");
    }

    LIMIT_TO("source") {
	printf("source <file name> - Execute dyner commands stored in the file <file name>\n");
    }

    LIMIT_TO("kill") {
	printf("kill - Terminate the execution of target program\n");
    }

    LIMIT_TO("print") {
	printf("print <Variable> - Display the data type and value of dyner variable\n");
    }

    LIMIT_TO("find") {
	printf("find function <pattern> [in <module>] - Display a list of functions in the image matching <pattern>.  Regex search will be performed if <pattern> contains regex, with some limitations\n");
    }

    LIMIT_TO("verbose") {
        printf("verbose - toggles verbose mode error reporting\n");
    }
    LIMIT_TO("whatis") {
	printf("whatis <variable> [in <function>] - Display detailed information about\n");
	printf("     variables in the target program. Local variables and parameters are\n");
	printf("     searched in the <function>.\n");
    }
#if defined(rs6000_ibm_aix4_1) || defined(sparc_sun_solaris2_4) || defined(i386_unknown_linux2_0) 
    LIMIT_TO("where") {
	printf("where - Print stack trace.\n");
    }
    LIMIT_TO("up"){
	printf("up - Move up the stack trace\n");
    }
    LIMIT_TO("down"){
	printf("down - Move down the stack trace\n");
    }

	LIMIT_TO("saveStart"){
		printf("saveStart - Call saveStart before 'save' to begin marking instrumentation\n");
		printf("	to be saved to the mutated binary. This must be called before save is\n");
		printf("	called.  Only instrumentation inserted after saveStart is called will be\n");
		printf("	saved in the mutated binary.\n");
	}
	LIMIT_TO("save"){
		printf("save <file name> - Save the currently loaded mutatee and its mutations to the\n");
		printf("	file <file name>.  Call 'saveStart' before 'save' to begin marking \n");
		printf("	instrumentation to be saved to the mutated binary\n");
	} 
#endif

    return TCL_OK;
}

bool haveApp()
{
    if (appThread == NULL) {
	fprintf(stderr, "No application loaded.\n");
	return false;
    }

    if (appThread->isTerminated()) {
	fprintf(stderr, "The application has exited.\n");
	return false;
    }

    return true;
}

BPatch_module *FindModule(const char *name) {

    //Locate the module using module name 
    BPatch_Vector<BPatch_module *> *modules = appImage->getModules();

    if (!modules) {
	printf("Can not get module info !\n");
	return NULL;
    }

    char modName[1024];
    BPatch_module *module = NULL;

    for(unsigned int i=0; i<modules->size(); ++i) {
	(*modules)[i]->getName(modName, 1024);

	if (!strcmp(modName, name)) {
		module = (*modules)[i];
		break;
	}
    }

    if (!module) {
	printf("Module %s is not found !\n", name);
	return NULL;
    }

    return module;
}

int loadApp(const char *pathname, TCLCONST char **args)
{
    printf("Loading \"%s\"\n", pathname);

    if (appThread != NULL) delete appThread;
    bplist.clear();
    iplist.clear();
    varList.clear();
    appThread = bpatch->createProcess((char*)pathname, (const char**)args);
    bpNumber = NULL;

    if (!appThread || appThread->isTerminated()) {
	fprintf(stderr, "Unable to run test program.\n");
	appThread = NULL;
	return TCL_ERROR;
    }

    // Read the program's image and get an associated image object
    appImage = appThread->getImage();

    if (!appImage) return TCL_ERROR;

    //Create type info
    appImage->getModules();

#if !defined(i386_unknown_nt4_0)
    // make it a member of its own process group
    int ret = setpgid(0, getpid());
    if (ret != 0) {
	perror("setpgid");
    }
#endif

    return TCL_OK;

}


int loadLib(const char *libName) {
    if (!haveApp()) return TCL_ERROR;

    if (appThread->loadLibrary(libName))
	return TCL_OK;

    return TCL_ERROR;
}

int loadSource(const char *inp) {
    if (!haveApp()) return TCL_ERROR;

    //Create shared object file name
    char* dupstr = strdup( inp );
    char *ptr = strrchr(dupstr, '/');
    if (ptr)
	ptr++;
    else
	ptr = dupstr;

    char fname[1024];
    sprintf(fname, "./%s", ptr);

    ptr = strrchr(fname+2, '.'); //Ignore first 2 chars ('./')
    if (ptr)
	sprintf(ptr+1,"so");
    else
	strcat(fname,".so");

    //First ensure that there is no .so file for the input file
    unlink(fname);
    
    //Create a shared object from input file
    char cmdBuf[1024];
    sprintf(cmdBuf,"g++ -g -fPIC -shared -o %s %s", fname, dupstr);

    system(cmdBuf);

    //Test whether or not shared object is created
    FILE *fp = fopen(fname,"rb");
    if (!fp) {
	printf("Error in compilation of %s\n", dupstr);
    free(dupstr);
	return TCL_ERROR;
    }
    fclose(fp);

    //Now dynamically link the file
    free(dupstr);
    return loadLib(fname);
}

int loadCommand(ClientData, Tcl_Interp *, int argc, TCLCONST char *argv[])
{
    if (argc == 3) {
	if (!strcmp(argv[1], "library"))
		return loadLib(argv[2]);
	if (!strcmp(argv[1], "source"))
		return loadSource(argv[2]);
    }

    if (argc < 2) {
	printf("Usage load <program> [<arguments>]\n");
	printf("or    load library <lib name>\n");
	printf("or    load source <C++ file name>\n");
	return TCL_ERROR;
    }

    return loadApp(argv[1], &argv[1]);
}

int attachPid(ClientData, Tcl_Interp *, int argc, TCLCONST char *argv[])
{
    int pid;
    const char *pathName = "";

    if ((argc < 2) || (argc > 3)) {
	printf("Usage attach <pid> <program>\n");
	return TCL_ERROR;
    }

    if (appThread != NULL) delete appThread;
    bplist.clear();
    iplist.clear();
    varList.clear();

    pid = atoi(argv[1]);
    if (argc == 3) pathName = argv[2];

    appThread = bpatch->attachProcess(pathName, pid);
    bpNumber = NULL;

    if (!appThread || !appThread->getImage() || appThread->isTerminated()) {
	fprintf(stderr, "Unable to attach to pid %d\n", pid);
	appThread = NULL;
	return TCL_ERROR;
    }

    // Read the program's image and get an associated image object
    appImage = appThread->getImage();

    if (!appImage) return TCL_ERROR;

    //Create type info
    appImage->getModules();

    return TCL_OK;
}


int listBreak(ClientData, Tcl_Interp *, int, TCLCONST char **)
{
    if (!haveApp()) return TCL_ERROR;

    BPListElem *curr;
    DynerList<BPListElem *>::iterator i;

    for (i = bplist.begin(); i != bplist.end(); i++) {
	curr = (BPListElem *) *i;
	curr->Print();
    }

    return TCL_OK;
}

int listInstrument(ClientData, Tcl_Interp *, int, TCLCONST char **)
{
    if (!haveApp()) return TCL_ERROR;

    IPListElem *curr;
    DynerList<IPListElem *>::iterator i;

    for (i = iplist.begin(); i != iplist.end(); i++) {
	curr = (IPListElem *) *i;
	if (curr->instType == NORMAL)
		curr->Print();
    }

    return TCL_OK;
}

int deleteBreak(ClientData, Tcl_Interp *, int argc, TCLCONST char *argv[])
{
    if (!haveApp()) return TCL_ERROR;

    if (argc < 2) {
	printf("Specify breakpoint(s) to delete.\n");
	return TCL_ERROR;
    }

    int ret = TCL_OK;
    for (int j = 1; j < argc; j++) {
	int n = atoi(argv[j]);

       	BPListElem *i = removeBPlist(n);
	if (i == NULL) {
	    printf("No such breakpoint: %d\n", n);
	    ret = TCL_ERROR;
	} else {
	    appThread->deleteSnippet(i->handle);
	    delete i;
	    printf("Breakpoint %d deleted.\n", n);
	}
    }

    return ret;
}

int deleteInstrument(ClientData, Tcl_Interp *, int argc, TCLCONST char *argv[])
{
    if (!haveApp()) return TCL_ERROR;

    if (argc < 2) {
	printf("Specify intrument number(s) to delete.\n");
	return TCL_ERROR;
    }

    int ret = TCL_OK;
    for (int j = 1; j < argc; j++) {
	int n = atoi(argv[j]);

       	IPListElem *i = removeIP(n);
	if (i == NULL) {
	    printf("No such intrument point: %d\n", n);
	    ret = TCL_ERROR;
	} else {
	    delete i;
	    printf("Instrument number %d deleted.\n", n);
	}
    }

    return ret;
}

void setWhereAmINow(){
	BPatch_Vector<BPatch_frame> callStack;

	appThread->getCallStack(callStack);

	whereAmINow = callStack.size()-1;

}


int runApp(ClientData, Tcl_Interp *, int, TCLCONST char **)
{
    if (!haveApp()) return TCL_ERROR;

    if (bpNumber) {
	int inval = -1;
	bpNumber->writeValue(&inval);
    }

    // Start of code to continue the process.
    dprintf("starting program execution.\n");
    appThread->continueExecution();
	
	whereAmINow = -1 ; //ccw 10 mar 2004 : i dont know where i will be when i stop

    while (!appThread->isStopped() && !appThread->isTerminated() && !stopFlag)
#if defined(i386_unknown_nt4_0)
	Sleep(1);
#else
	usleep(250);
#endif

    if (stopFlag) {
	stopFlag = false;
	appThread->stopExecution();
	setWhereAmINow(); //ccw 10 mar 2004 : find myself
    }

    int bp = -1;
    if (bpNumber) bpNumber->readValue(&bp);

    if (appThread->isTerminated()) {
	printf("\nApplication exited.\n");
    } else if (appThread->isStopped() && bp > 0) {
		printf("\nStopped at break point %d.\n", bp);
		ListElem *i = findBP(bp);
		if (i != NULL) {
	    	BPListElem *curr = (BPListElem *) i;
		    curr->Print();
		}
    } else {
	printf("\nStopped.\n");
    }
	
    return TCL_OK;
}

int killApp(ClientData, Tcl_Interp *, int, TCLCONST char **)
{
    if (!haveApp()) return TCL_ERROR;

    appThread->terminateExecution();

    return TCL_OK;
}

#if defined(sparc_sun_solaris2_4) || defined(i386_unknown_linux2_0) || defined(rs6000_ibm_aix4_1)

bool saveWorldStart =false;

int saveStart(ClientData, Tcl_Interp *, int argc, TCLCONST char **argv){

	if (!haveApp()) return TCL_ERROR;


	saveWorldStart=true;
	appThread->enableDumpPatchedImage();
	return TCL_OK;

}

int saveWorld(ClientData, Tcl_Interp *, int argc, TCLCONST char **argv){
	if(argc != 2){
		printf(" Usage: save <filename>\n");
		return TCL_ERROR;
	}
	if(!saveWorldStart){
		printf("Call saveStart before issuing instrumentation to save\n");
		return TCL_ERROR;
	}
	char* directoryName = appThread->dumpPatchedImage(argv[1]);
	printf(" saved in %s \n", directoryName);
	delete [] directoryName;
	
	return TCL_OK;
}
#endif

extern BPatch_snippet *parse_result;
int condBreak(ClientData, Tcl_Interp *, int argc, TCLCONST char *argv[])
{
    if (argc < 2) {
	printf("Usage: break <function> [entry|exit|preCall|postCall] [<condition>]\n");
	printf("or     break <file name:line number> [<condition>]\n");
	return TCL_ERROR;
    }

    if (!haveApp()) return TCL_ERROR;

    if (bpNumber == NULL) {
	bpNumber = appThread->malloc(*appImage->findType("int"));
	if (bpNumber == NULL) {
	    fprintf(stderr, "Unable to allocate memory in the inferior process.\n");
	    exit(1);
	}
    }

    int expr_start = 2;
    int lineNum = 0;
    BPatch_procedureLocation where;
    BPatch_callWhen when;

    BPatch_Vector<BPatch_point *> *points;
    char *ptr = strchr(argv[1],':');
    if (ptr) {
	//Arbitrary break point, first find module name
	expr_start = 2;
	where = BPatch_entry;
	when = BPatch_callBefore;
	*ptr = '\0';
	lineNum = atoi(ptr+1);

	std::vector< std::pair< unsigned long, unsigned long > > absAddrVec;
	if( ! appImage->getAddressRanges( argv[1], lineNum, absAddrVec ) ) {
		printf("Can not get arbitrary break point address!\n");
		return TCL_ERROR;
	}
	points = new BPatch_Vector<BPatch_point *>;
	
	/* Arbitrarily, the lower end of the first range. */
	points->push_back(appImage->createInstPointAtAddr((void *)absAddrVec[0].first));
    }
    else {
	if ( (argc > 2) && name2loc(argv[2], where, when)) {
		expr_start = 3;
	} else {
		where = BPatch_entry;
		when = BPatch_callBefore;
	}

	BPatch_Vector<BPatch_function *> found_funcs;
	if ((NULL == appImage->findFunction(argv[1], found_funcs)) 
	    || (0 == found_funcs.size())) {
	  printf("%s[%d]:  CANNOT CONTINUE  :  %s not found\n", __FILE__, __LINE__, argv[1]);
	  return TCL_ERROR;
	}

	if (1 < found_funcs.size()) {
	  printf("%s[%d]:  WARNING  :  %d functions called '%s'found.  Using the first\n", 
		 __FILE__, __LINE__, 
		 found_funcs.size(), argv[1]);
	}
	
	points = found_funcs[0]->findPoint(where);

	if (points == NULL) {
		printf("Unable to locate points for function %s\n", argv[1]);
		return TCL_ERROR;
	}
    }

    char *line_buf = NULL;

    if (argc > 3) {
	// Count up how large a buffer we need for the whole line
	int line_len = 0;
	int i = 0;
	for (i = expr_start; i < argc; i++)
		line_len += strlen(argv[i]) + 1;
	line_len++;
	// Make the buffer and copy the line into it
	line_buf = new char[line_len];
	*line_buf = '\0';
	for (i = expr_start; i < argc; i++) {
		strcat(line_buf, argv[i]);
		if (i != argc-1) strcat(line_buf, " ");
	}

	set_lex_input(line_buf);
	if (dynerparse() != 0) {
		fprintf(stderr, "Breakpoint not set due to error.\n");
		delete line_buf;
		return TCL_ERROR;
	}

	if (parse_type != parsed_bool) {
		fprintf(stderr, "Breakpoint not set due error, expression not boolean.\n");
		delete line_buf;
		delete parse_result;
		return TCL_ERROR;
	}
    }

    // Make a snippet to tell us which breakpoint it is
    BPatch_arithExpr storeBPNum(BPatch_assign,
			        *bpNumber, BPatch_constExpr(bpCtr));

    // Make a snippet to break at the breakpoint
    BPatch_Vector<BPatch_function *> bpfv;
    if (NULL == appImage->findFunction("DYNINSTbreakPoint", bpfv) || ! bpfv.size()) {
      fprintf(stderr, "Unable to find function DYNINSTbreakPoint "
	      "(required for setting break points)\n");
      if (argc > 3) {
	delete parse_result;
	delete line_buf;
      }
      return TCL_ERROR;
    }

    BPatch_function *breakFunc = bpfv[0];
    if (breakFunc == NULL) {
      fprintf(stderr, "Unable to find function DYNINSTbreakPoint "
	      "(required for setting break points)\n");
      if (argc > 3) {
	delete parse_result;
	delete line_buf;
      }
      return TCL_ERROR;
    }

    BPatch_Vector<BPatch_snippet *> nullArgs;
    BPatch_funcCallExpr callBreak(*breakFunc, nullArgs);

    // store the break point number, then  break
    BPatch_arithExpr doBreak(BPatch_seq, storeBPNum, callBreak);
    
    BPatch_snippet *brSnippet = (BPatch_snippet *) &doBreak;

    if (argc > 3) {
	// Call the break snippet conditionally
	BPatch_ifExpr condBreak(*(BPatch_boolExpr *)parse_result, doBreak); 
   	brSnippet = (BPatch_snippet *) &condBreak; 
	delete parse_result;
    }

    BPatchSnippetHandle *handle =
	appThread->insertSnippet(*brSnippet, *points, when,BPatch_lastSnippet);
    if (handle == 0) {
	fprintf(stderr, "Error inserting snippet.\n");
	if (line_buf) delete line_buf;
	return TCL_ERROR;
    }

    BPListElem *bpl =
	new BPListElem(bpCtr, argv[1], where, when, line_buf, handle, lineNum);
    if (line_buf) delete line_buf;

    bplist.push_back(bpl);

    printf("Breakpoint %d set.\n", bpCtr);
    bpCtr++;

    return TCL_OK;
}
/*
 * void printPtr( )
 *
 * This function recursively goes through the BPatch_type pointers until
 * it gets to the base type and prints that name.  The base type can be
 * a builtin type or an enum, struct, or union.  The default case handles
 * unknown pointer types.  -- jdd 5/27/99
 */
void printPtr( BPatch_type * type )
{
  if( type->getDataClass() == BPatch_pointer ){
    printPtr( type->getConstituentType() );
    printf("*");
  }
  else if((type->getDataClass() == BPatch_scalar)||
	(type->getDataClass() == BPatchSymTypeRange)){
    printf("%s ", type->getName());
  }
  else{
    switch( type->getDataClass() ){
    case BPatch_enumerated:
      printf("enum %s ", type->getName());
      break;
    case BPatch_structure:
      printf("struct %s ", type->getName());
      break;
    case BPatch_union:
      printf("union %s ", type->getName());
      break;
    default:
      printf("Undefined POINTER: %s", type->getName());
      break;
    }
  }
  
}
/*
 * printArray()
 *
 * This function prints arrays of various dimension, 1D to 3D. It first
 * checks how many dimensions the array is and then prints it out.
 * The default case handles the unknown. --jdd 5/27/99
 */
 
void printArray(BPatch_type * type )
{
  int j = 0;
  if(type->getConstituentType()){
    j++;
    if(type->getConstituentType()->getConstituentType()){
      j++;
      if(type->getConstituentType()->getConstituentType()->getConstituentType()){
	j++;
      }
    }
  }
  switch(j){
  case 3:
    
    //3D array
    printf("    %s is an array [%s..%s][%s..%s][%s..%s] of %s\n",
	   type->getName(), type->getLow(), type->getHigh(),
	   type->getConstituentType()->getLow(),
	   type->getConstituentType()->getHigh(),
	   type->getConstituentType()->getConstituentType()->getLow(),
	   type->getConstituentType()->getConstituentType()->getHigh(),
	   type->getConstituentType()->getConstituentType()->getConstituentType()->getName());
    break;
  case 2:
        
    //2D array
    printf("    %s is an array [%s..%s][%s..%s] of %s\n", type->getName(), 
	   type->getLow(), type->getHigh(),
	   type->getConstituentType()->getLow(),
	   type->getConstituentType()->getHigh(),
	   type->getConstituentType()->getConstituentType()->getName());
    break;
  case 1:
    printf("    %s is an array [%s..%s] of %s\n", type->getName(),
	   type->getLow(), type->getHigh(), 
	   type->getConstituentType()->getName());
    break;
  default:
    printf("Unable to process array %s\n", type->getName());
    break;
  }
}

BPatch_snippet *termStatement = NULL;

void exitCallback(BPatch_thread *thread, BPatch_exitType) {
    if (termStatement == NULL)
	return;

    BPatch_snippet *stmt = termStatement;
    termStatement = NULL;
    thread->oneTimeCode(*stmt);
    delete stmt;
}

int instTermStatement(int argc, TCLCONST char *argv[])
{
    int expr_start = 2;

    // Count up how large a buffer we need for the whole line
    int line_len = 0;
    int i = 0;
    for (i = expr_start; i < argc; i++)
	line_len += strlen(argv[i]) + 1;
    line_len += 2;
    // Make the buffer and copy the line into it
    char *line_buf = new char[line_len];
    *line_buf = '\0';
    for (i = expr_start; i < argc; i++) {
	strcat(line_buf, argv[i]);
	if (i != argc-1) strcat(line_buf, " ");
    }
    strcat(line_buf, "\n");

    // printf("calling parse of %s\n", line_buf);
    set_lex_input(line_buf);
    if (dynerparse() != 0) {
	fprintf(stderr, "Instrumentation not set due to error.\n");
	delete line_buf;
	return TCL_ERROR;
    }

    if (parse_type != parsed_statement) {
	fprintf(stderr, "code not inserted, expression is not a statement.\n");
	delete line_buf;
	delete parse_result;
	return TCL_ERROR;
    }

    termStatement = parse_result;
    return TCL_OK;
}

int instStatement(ClientData, Tcl_Interp *, int argc, TCLCONST char *argv[])
{
    if (!haveApp()) return TCL_ERROR;

    if (argc < 3) {
	printf("Usage: at <function> [entry|exit|preCall|postCall] <statement>\n");
	printf("or     at termination <statement>\n");
	return TCL_ERROR;
    }

    if (!strcmp(argv[1], "termination"))
	return instTermStatement(argc, argv);

    int expr_start = 2;
    BPatch_procedureLocation where;
    BPatch_callWhen when;

    if (name2loc(argv[2], where, when)) {
	expr_start = 3;
    } else {
	where = BPatch_entry;
	when = BPatch_callBefore;
    }
    BPatch_Vector<BPatch_function *> bpfv;
    if (NULL == appImage->findFunction(argv[1], bpfv) || !bpfv.size()) {
	fprintf(stderr, "Unable to locate function: %s\n", argv[1]);
	return TCL_ERROR;
    }

    BPatch_function *func = bpfv[0]; 
    if (func == NULL) {
	fprintf(stderr, "Unable to locate function: %s\n", argv[1]);
	return TCL_ERROR;
    }

    BPatch_Vector<BPatch_point *> *points = func->findPoint(where);

    if (points == NULL) {
	fprintf(stderr, "Unable to locate function: %s\n", argv[1]);
	return TCL_ERROR;
    }

    //Assign targetPoint using function entry point
    targetPoint = (*(func->findPoint(BPatch_entry)))[0];

    InstPointType instType = NORMAL;
    if (!strcmp(argv[argc-1], "trace")) {
	//This statement is used for tracing the functions
	instType = TRACE;
	argc--;
    }
    else if (!strcmp(argv[argc-1], "count")) {
	//This statement is used for counting the functions
	instType = COUNT;
	argc--;
    }

    // Count up how large a buffer we need for the whole line
    int line_len = 0;
    int i = 0;
    for (i = expr_start; i < argc; i++)
	line_len += strlen(argv[i]) + 1;
    line_len += 2;
    // Make the buffer and copy the line into it
    char *line_buf = new char[line_len];
    *line_buf = '\0';
    for (i = expr_start; i < argc; i++) {
	strcat(line_buf, argv[i]);
	if (i != argc-1) strcat(line_buf, " ");
    }
    // strcat(line_buf, "\n");

    // printf("calling parse of %s\n", line_buf);
    set_lex_input(line_buf);
    if (dynerparse() != 0) {
	fprintf(stderr, "Instrumentation not set due to error.\n");
	delete line_buf;
	targetPoint = NULL;
	return TCL_ERROR;
    }

    targetPoint = NULL;

    if (parse_type != parsed_statement) {
	fprintf(stderr, "code not inserted, expression is not a statement.\n");
	delete line_buf;
	delete parse_result;
	return TCL_ERROR;
    }

    BPatchSnippetHandle *handle =
	appThread->insertSnippet(*parse_result, *points, when,BPatch_lastSnippet);
    if (handle == 0) {
	fprintf(stderr, "Error inserting snippet.\n");
	delete line_buf;
	return TCL_ERROR;
    }

    IPListElem *snl =
	new IPListElem(ipCtr, argv[1], where, when, line_buf, handle, instType);
    delete line_buf;

    iplist.push_back(snl);

    if (instType == NORMAL) {
    	printf("Instrument point %d set.\n", ipCtr);
        ipCtr++;
    }

    return TCL_OK;
}

int execStatement(ClientData, Tcl_Interp *, int argc, TCLCONST char *argv[])
{
    if (!haveApp()) return TCL_ERROR;

    if (argc < 2) {
	printf("Usage: execute <statement>\n");
	return TCL_ERROR;
    }

    int expr_start = 1;

    // Count up how large a buffer we need for the whole line
    int line_len = 0;
    int i = 0;
    for (i = expr_start; i < argc; i++)
	line_len += strlen(argv[i]) + 1;
    line_len += 2;
    // Make the buffer and copy the line into it
    char *line_buf = new char[line_len];
    *line_buf = '\0';
    for (i = expr_start; i < argc; i++) {
	strcat(line_buf, argv[i]);
	if (i != argc-1) strcat(line_buf, " ");
    }
    strcat(line_buf, "\n");

    //printf("calling parse of %s\n", line_buf);
    set_lex_input(line_buf);
    if (dynerparse() != 0) {
	fprintf(stderr, "Execution can not be done due to error.\n");
	delete line_buf;
	return TCL_ERROR;
    }

    if (parse_type != parsed_statement) {
	fprintf(stderr, "Syntax error, expression is not a statement.\n");
	delete line_buf;
	delete parse_result;
	return TCL_ERROR;
    }

    appThread->oneTimeCode(*parse_result);
    delete parse_result;
    delete line_buf;

    return TCL_OK;
}

void printVarRecursive(BPatch_variableExpr *var, int level)
{
    int iVal;
    int pVal;
    char cVal;
    float fVal;

    BPatch_dataClass dc;
    BPatch_type *type = (BPatch_type *) var->getType();

	if( !var ){
		fprintf(stderr," var is NULL\n");
		return;
	}
    dc = type->getDataClass();
    if (!strcmp(type->getName(), "int")) {
	/* print out the integer */
	var->readValue((void *) &iVal);
	printf("%d\n", iVal);
    } else if (!strcmp(type->getName(), "float")) {
	/* print out the float */
	var->readValue((void *) &fVal);
	printf("%f\n", fVal);
    }else if(!strcmp(type->getName(), "char")) {
	/* print out the char*/
	var->readValue((void *) &cVal);
	printf("%c\n", cVal);

    } else if (dc == BPatch_pointer) {
	/* print out the float */
	var->readValue((void *) &pVal);
	printf("0x%x\n", pVal);
    } else if (dc == BPatch_structure) {
	printf("struct {\n");
	level++;
	BPatch_Vector<BPatch_variableExpr *> *fields = var->getComponents();
	for (unsigned int i=0; i < fields->size(); i++) {
	     BPatch_variableExpr *fieldVar = (*fields)[i];
	     for (int i=0;i < level; i++) printf("    ");
	     printf("%s = ", fieldVar->getName());
	     printVarRecursive(fieldVar, level);
	}
	level--;
	printf("}\n");
    } else if (dc == BPatch_array) {
	printf("<arrays not yet implemented>\n");
    } else {
	printf("<unknown type>\n" );
    }
}

#if defined(rs6000_ibm_aix4_1) || defined(sparc_sun_solaris2_4) || defined(i386_unknown_linux2_0)  
void printStackFrame(int index, BPatch_Vector<BPatch_frame> &callStack, char *funcName){

	printf("#%d: (0x%x)\t%s (fp: 0x%x)\n", index,callStack[index].getPC(),funcName ,callStack[index].getFP());

}

int whereUp(ClientData, Tcl_Interp *, int, TCLCONST char *argv[])
{
	BPatch_Vector<BPatch_frame> callStack;
	char funcName [1024];
	int index=0;


	appThread->getCallStack(callStack);

	if(whereAmINow < (callStack.size()-1) ){
		whereAmINow++;
	}

	printf("     ");

	if(callStack[whereAmINow].findFunction()){

		BPatch_Vector<BPatch_point *> *points = callStack[index].findFunction()->findPoint(BPatch_subroutine);

		if ( points  && points->size() > 0){  //linux gets weird here
				
			targetPoint = (*points)[0];
		}
		callStack[whereAmINow].findFunction()->getName(funcName, 1024);

		printStackFrame(whereAmINow, callStack, funcName);


	}else{
		printStackFrame(whereAmINow, callStack, "<<FAILED TO FIND FUNCTION>>");
	}
	return TCL_OK;
}

int whereDown(ClientData, Tcl_Interp *, int, TCLCONST char *argv[])
{
	BPatch_Vector<BPatch_frame> callStack;
	char funcName [1024];
	int index=0;

	if( whereAmINow > 1) {
		whereAmINow-- ;
	}


	appThread->getCallStack(callStack);
	printf("     ");

	if(callStack[whereAmINow].findFunction()){

		BPatch_Vector<BPatch_point *> *points = callStack[index].findFunction()->findPoint(BPatch_subroutine);

		if ( points  && points->size() > 0){  //linux gets weird here
			
			targetPoint = (*points)[0];
		}

		callStack[whereAmINow].findFunction()->getName(funcName, 1024);

		printStackFrame(whereAmINow, callStack, funcName);


	}else{
		printStackFrame(whereAmINow, callStack, "<<FAILED TO FIND FUNCTION>>");
	}
	return TCL_OK;
}

int where(ClientData, Tcl_Interp *, int, TCLCONST char *argv[])
{
	BPatch_Vector<BPatch_frame> callStack;
	char funcName [1024];
	int index=0;

	appThread->getCallStack(callStack);
	index = 1; 
	while(index < callStack.size() -1){

		if( (whereAmINow == -1 && index == 1) || ( whereAmINow == index ) ){
			printf(" --> ");
			whereAmINow = index;
		}else{
			printf("     ");
		}

		if(callStack[index].findFunction()){
			BPatch_Vector<BPatch_point *> *points = callStack[index].findFunction()->findPoint(BPatch_subroutine);

			if ( points  && points->size() > 0){  //linux gets weird here
				
				targetPoint = (*points)[0];
			}


			callStack[index].findFunction()->getName(funcName, 1024);

			printStackFrame(index, callStack, funcName);


		}else{
			
			printStackFrame(index, callStack, "<<FAILED TO FIND FUNCTION>>");
		}
		index ++;
	}
	return TCL_OK;
}
#endif

#if defined(rs6000_ibm_aix4_1) || defined(sparc_sun_solaris2_4) || defined(i386_unknown_linux2_0)   

BPatch_variableExpr *findLocalVariable(const char *name, bool printError)
{
	BPatch_variableExpr *var=NULL;
#ifdef mips_sgi_irix6_4
	/* mips_sgi_irix6_4 does not support local vars but if it does we are ready! */
	long index;
#define CASTOFFSET long

#else
	int index;
#define CASTOFFSET int

#endif
	int offset;

	/* if we have a local context, use that. otherwise use the top of the call stack */
	if( whereAmINow == -1 ){

		index = 1;
	}else{
		index = whereAmINow;
	}

	BPatch_Vector<BPatch_frame> callStack;
	char funcName [1024];
	BPatch_variableExpr *tmpVar;

	appThread->getCallStack(callStack);

	if(	callStack[index].findFunction()){

		BPatch_Vector<BPatch_point *> *points = callStack[index].findFunction()->findPoint(BPatch_entry);//ccw 10 mar 2004 was subroutine

		if ( points  && points->size() > 0){ 
				
			targetPoint = (*points)[0];

			callStack[index].findFunction()->getName(funcName, 1024);

			tmpVar = appImage->findVariable(*targetPoint, name);
			targetPoint = NULL;


			if (tmpVar && 
#ifdef rs6000_ibm_aix4_1
				(((int)tmpVar->getBaseAddr()) < 0x1000) ) {

#else
				( ((CASTOFFSET) (tmpVar->getBaseAddr())) < 0) ) {

#endif
				offset = (CASTOFFSET) (tmpVar->getBaseAddr());



#ifdef sparc_sun_solaris2_4
				index ++; /* ccw 9 mar 2004 WHY DO I NEED TO DO THIS ?*/
#endif
			
				/* WARNING: the function BPatch_thread::lowlevel_process() is risky, it should go away
				   But i need to build a variable that points to a specific address, and I can not find
				   a better way to do it right now
				 */	
				var = new BPatch_variableExpr(tmpVar->getName(), appThread->getProcess(),
					(void*) ( ((CASTOFFSET) (callStack[index].getFP())) +offset), tmpVar->getType() );	

#ifdef sparc_sun_solaris2_4 
				index --;  
#endif
				whereAmINow = index; /* set local context just in case */

				return var;
			}
			
		}

		
	}

    	if (printError) printf("Unable to locate local variable %s\n", name);
	return NULL;
}
#endif

int printVar(ClientData, Tcl_Interp *, int, TCLCONST char *argv[])
{
    //if (!haveApp()) return TCL_ERROR;
	bool found = false;
    	BPatch_variableExpr *var; 
#if defined(rs6000_ibm_aix4_1) || defined(sparc_sun_solaris2_4) || defined(i386_unknown_linux2_0)  

   	var = findLocalVariable(argv[1],false);

	if( var) {
	
		printf("Local  Variable:\t%s = ", argv[1]);
		printVarRecursive(var, 1);
		found = true;
	}

#endif

	var = findVariable(argv[1],false);
	if( var ) {
		printf("Global Variable:\t%s = ", argv[1]);
		printVarRecursive(var, 1);
		found = true;
	}

	if( ! found  ){
		printf("%s is not defined\n", argv[1]);
		return TCL_ERROR;
	}
    
    	return TCL_OK;
}

/*
 * declare <type> <variable name>
 */
int newVar(ClientData, Tcl_Interp *, int argc, TCLCONST char *argv[])
{
    if (argc != 3) {
	printf("Usage: declare <type> <variable name>\n");
	return TCL_ERROR;
    }

    if (!haveApp()) return TCL_ERROR;

    BPatch_type *type = appImage->findType(argv[1]);
    if (!type) {
	printf("type %s is not defined\n", argv[1]);
	return TCL_ERROR;
    }

    BPatch_variableExpr *newVar = appThread->malloc(*type);
    if (!newVar) {
	printf("Unable to create variable.\n");
	return TCL_ERROR;
    }

    varList.push_back(new runtimeVar(newVar, argv[2]));

    return TCL_OK;
}


BPatch_variableExpr *findVariable(const char *name, bool printError)
{
    BPatch_variableExpr *var;
    DynerList<runtimeVar *>::iterator i;

    // First look for runtime created variables
    for (i = varList.begin(); i != varList.end(); i++) {
	if (!strcmp(name, (*i)->name)) {
	    var = new BPatch_variableExpr(*((*i)->var));
	    return (var);
	}
    }

    if(targetPoint){
	var = appImage->findVariable(*targetPoint, name);
	if(var) {
		return var;
	}
    }

    // Check global vars in the mutatee
    errorLoggingOff = true;
    var = appImage->findVariable(name);
    errorLoggingOff = false;

    if (var) return var;

    if (printError) printf("Unable to locate variable %s\n", name);
    return NULL;
}

/* 
 * int whatisParam()
 *
 * This function is an extension to the "whatis" command.  This function
 * processes the command: whatis -scope <function> <variable>. If the
 * <function> is undefined then TCL_ERROR is returned.  Otherwise, the
 * local variable collection is searched first; followed by the parameter
 * collection; and finally the Global collection is searched last. If the
 * <variable> is not found, then "<variable> is not defined" is reported
 * to the user. --jdd 5/27/98
 */
int whatisParam(ClientData, Tcl_Interp *, int, TCLCONST char *argv[])
{
  if (!haveApp()) return TCL_ERROR;
  BPatch_Vector<BPatch_function *> pdfv;
  if (NULL == appImage->findFunction(argv[3], pdfv) || !pdfv.size()) {
    printf("%s is not defined\n", argv[3]);
    return TCL_ERROR;
  }

  if (pdfv.size() > 1) {
    printf("%s[%d]:  WARNING, found %d functions called %s, using the first\n",
	   __FILE__, __LINE__, pdfv.size(), argv[3]);
  }
  
  BPatch_function * func = pdfv[0];

  //printf("func is %x\n", func);
  if(!func){
    printf("%s is not defined\n", argv[3]);
    return TCL_ERROR;
  }
  
  
  BPatch_localVar * lvar = func->findLocalVar(argv[1]);
  if(lvar){
    BPatch_type *type = lvar->getType();
    if( type ){
      switch (type->getDataClass()) {
      case BPatch_array:
	printArray( type );
	break;
	
      case BPatch_pointer:
	printPtr( type );
	printf("\t %s\n", argv[1]);
	break;
	
      default:
	printf("    %s is of type %s \n", argv[1], type->getName());
	break;
      }
    }
    //print local variable info
    printf("        %s is a local variable in function %s\n", lvar->getName(),
	   argv[3]);
    printf("        Declared on line %d and has a Frame offset of %d\n",
	   lvar->getLineNum(), lvar->getFrameOffset());
  }
  else{
    lvar = func->findLocalParam(argv[1]);
    
    if (!lvar) {
      BPatch_variableExpr *var = findVariable(argv[1]);
      if(!var){
	// do something else ??
	//int ret = whatisType(cd, interp, argc, argv);
	return TCL_OK;
      }
      BPatch_type *type = (BPatch_type *) var->getType();
      switch (type->getDataClass()) {
      case BPatch_array:
	printArray( type );
	break;
	
      case BPatch_pointer:
	printPtr( type );
	printf("\t %s\n", argv[3]);
	break;
	
      default:
	printf("    %s is of type %s \n", argv[1], type->getName());
	break;
      }
     printf("      %s is a Global variable\n", argv[1]);
    }
    else{
      BPatch_type *lType = (BPatch_type *) lvar->getType();
      if (lType){
	switch (lType->getDataClass()) {
	case BPatch_array:
	  printArray( lType );
	  break;
	  
	case BPatch_pointer:
	  printPtr( lType );
	  printf("\t %s\n", argv[1]);
	  break;
	  
	default:
	  printf("    %s is of type %s \n", argv[1], lType->getName());
	  break;
	}
	printf("       %s is a parameter of the function %s\n",lvar->getName(),
	   argv[3]);
	printf("       Declared on line %d and has a Frame offset of %d\n",
	   lvar->getLineNum(), lvar->getFrameOffset());
      }
      else{
	printf("   unknown type %s\n", lvar->getName());
      }
    }   
  }
  return TCL_OK;
}

/*
 * int whatisFunc()
 *
 * This function is an extension to the "whatis" command.  This function
 * searches all of the modules to find the BPatch_function with the name
 * given be argv[1]. If no such function is found, then TCL_ERROR is returned.
 * This is the last function in the chain of "whatis" command functions to
 * be executed in the search for argv[1]. --jdd 5/27/99
 */
 
int whatisFunc(ClientData, Tcl_Interp *, int, TCLCONST char *argv[])
{
  if (!haveApp()) return TCL_ERROR;
  
  BPatch_Vector<BPatch_function *> pdfv;
  if (NULL == appImage->findFunction(argv[3], pdfv) || ! pdfv.size()) {
    printf("%s is not defined\n", argv[1]);
    return TCL_ERROR;
  }

  if (pdfv.size() > 1) {
    printf("%s[%d]:  WARNING, found %d functions called %s, using the first\n",
	   __FILE__, __LINE__, pdfv.size(), argv[3]);
  }
  
  BPatch_function * func = pdfv[0];
    
    if(!func){
      printf("%s is not defined\n", argv[1]);
      return TCL_ERROR;
    }
    BPatch_type * retType = func->getReturnType();
    if(retType)
      switch(retType->getDataClass()){
      case BPatch_scalar:
      case BPatchSymTypeRange:
	printf("%s",retType->getName() );
	break;
      case BPatch_array:
	printArray( retType );
	break;
	  
      case BPatch_pointer:
	printPtr( retType );
	break;
	
      default:
	printf("*unknown return type %s %d*\n",
	       retType->getName(), retType->getID());
	break;
      }
    else
      printf("*unknown*");
    printf(" %s(", argv[1]);
    BPatch_Vector<BPatch_localVar *> *params = func->getParams();
    for (unsigned int i=0; i < params->size(); i++) {
      BPatch_localVar *localVar = (*params)[i];
      BPatch_type *lType = (BPatch_type *) localVar->getType();
      if (lType){
	if( (lType->getDataClass())  == BPatch_pointer){
	  printPtr(lType);
	  printf("%s", localVar->getName());
	} else if( (lType->getDataClass()) == BPatch_array){
	  printArray( lType );
	} else {
	  printf("%s %s",lType->getName(), localVar->getName());
	}
      } else {
	printf("unknown type %s", localVar->getName());
      }
      if (i < params->size()-1) printf(", ");
    }    
    printf(") \n");
    if (func->getBaseAddr()) {
	unsigned int firstLine, lastLine;
	char file[1024];
	printf("   starts at 0x%lx", (long)func->getBaseAddr());
	unsigned int size = sizeof(file);
	
	std::vector< std::pair< const char *, unsigned int > > lines;
	if( func->getProc()->getSourceLines( (unsigned long)func->getBaseAddr(), lines ) ) {
		printf(" defined at %s:%d\n", lines[0].first, lines[0].second );
	} else {
	    printf("\n");
	}
    }
    return TCL_OK;
}

/*
 * int whatisType()
 *
 * This function is an extension to the "whatis" command.  This function
 * searches for the type collection of the all the modules looking for
 * the type specified by argv[1]. If it is not found, then whatisFunc()
 * is called. This is function is called by whatisVar if a variable is
 * not found. -- jdd 5/27/99
 */
 
int whatisType(ClientData cd, Tcl_Interp *interp, int argc, TCLCONST char *argv[])
{
    if (!haveApp()) return TCL_ERROR;
    
    BPatch_type *type = appImage->findType(argv[1]);
    if (!type) {
      int ret = whatisFunc(cd, interp, argc, argv);
      return ret;
    }

    BPatch_dataClass dc = type->getDataClass();
    switch (dc) {
    case BPatch_structure: {
      printf("    struct %s {\n", argv[1]);
      BPatch_Vector<BPatch_field *> *fields = type->getComponents();
      for (unsigned int i=0; i < fields->size(); i++) {
	BPatch_field *field = (*fields)[i];
	BPatch_type *fType = (BPatch_type *) field->getType();
	if (fType){
	  switch (fType->getDataClass()) {
	  case BPatch_array:
	    printArray( fType );
	    printf("  %s\n", field->getName());
	    break;
	
	  case BPatch_pointer:
	    printPtr( fType );
	    printf("  %s\n", field->getName());
	    break;
	
	  default:
	    printf("        %s %s\n", fType->getName(), field->getName());
	    break;
	  }
	}
	else{
	  printf("   unknown type %s\n", field->getName());
	}
      }
      printf("    }\n");
      break;
    }
    case BPatch_union: {
      printf("    union %s {\n", argv[1]);
      BPatch_Vector<BPatch_field *> *fields = type->getComponents();
      for (unsigned int i=0; i < fields->size(); i++) {
	BPatch_field *field = (*fields)[i];
	BPatch_type *fType = (BPatch_type *) field->getType();
	if (fType){
	  switch (fType->getDataClass()) {
	  case BPatch_array:
	    printArray( fType );
	    printf("  %s\n", field->getName());
	    break;
	
	  case BPatch_pointer:
	    printPtr( fType );
	    printf("  %s\n", field->getName());
	    break;
	    
	  default:
	    printf("        %s %s\n", fType->getName(), field->getName());
	    break;
	  }
	}
	else{
	  printf("        unknown type %s\n", field->getName());
	}
      }
      printf("    }\n");
      break;
    }
    case BPatch_enumerated: {
      printf("    enum %s {\n", argv[1]);
      BPatch_Vector<BPatch_field *> *fields = type->getComponents();
      for (unsigned int i=0; i < fields->size(); i++) {
	BPatch_field *field = (*fields)[i];
	printf("        %s    \t%d\n", field->getName(),
	       field->getValue());
      }
      printf("    }\n");
      break;
    }
    case BPatch_array:
      printArray( type );
      break;
      
    case BPatch_scalar:
      printf("    %s is a scalar of size %d\n",argv[1],type->getSize());
      break;
      
    case BPatch_pointer:
      printPtr( type );
      printf("\t %s\n", argv[1]);
      /*
      printf("    %s is pointer to type %s\n", argv[1], 
	     type->getConstituentType()->getName());*/
      break;
    case BPatchSymTypeRange:
      if(type->getConstituentType())
	printf("    %s is a %s of size %d\n",argv[1],
	       (type->getConstituentType())->getName(),type->getSize());
      else
	printf("    %s is a scalar of size %d\n",argv[1],type->getSize());
      if( type->getLow() ){
	 printf("        with range %s to %s\n", type->getLow(),
		type->getHigh());
      }
      break;
    default:
      printf("%s is of an unknown data class %d\n", argv[1],
	     dc);
      break;
    }
    
    return TCL_OK;
}

/*
 * int whatisVar()
 *
 * This function is an extension to the "whatis" command.  This function
 * searches the process for the variable named by argv[1] and then searches
 * all the Global variables for all the modules to find the variable
 * specified by argv[1].  If nothing is found, then whatisType() is called. If
 * whatisType() finds nothing, whatisFunc() is called.  If the command is
 * of the form "whatis -scope <function> <variable>", then whatisParam() is
 * called skipping all other attempts to find the variable. -- jdd 5/27/99
 */

int whatisVar(ClientData cd, Tcl_Interp *interp, int argc, TCLCONST char *argv[])
{
    if (!haveApp()) return TCL_ERROR;
    
    if (argc == 4){ //looking for a local variable
      if(!(strcmp(argv[2], "in"))){
	int ret = whatisParam(cd, interp, argc, argv );
	return ret;
      } else {
	printf("inavlid argument %s\n", argv[1]);
	return TCL_ERROR;
      }
    }

    BPatch_variableExpr *var = findVariable(argv[1], false);
    if (!var) {
	int ret = whatisType(cd, interp, argc, argv);
	return ret;
    }

    BPatch_type *type = (BPatch_type *) var->getType();
    switch (type->getDataClass()) {
    case BPatch_structure: {
      printf("    struct %s {\n", argv[1]);
      BPatch_Vector<BPatch_field *> *fields = type->getComponents();
      for (unsigned int i=0; i < fields->size(); i++) {
	BPatch_field *field = (BPatch_field *) (*fields)[i];
	BPatch_type *fType = (BPatch_type *) field->getType();
	if (fType){
	  printf("        %s %s\n", fType->getName(), field->getName());
	}
	else{
	  printf("   unknown type %s\n", field->getName());
	}
      }
      printf("    }\n");
      break;
    }
    case BPatch_union: {
      printf("    union %s {\n", argv[1]);
      BPatch_Vector<BPatch_field *> *fields = type->getComponents();
      for (unsigned int i=0; i < fields->size(); i++) {
	BPatch_field *field = (*fields)[i];
	BPatch_type *fType = (BPatch_type *) field->getType();
	if (fType){
	  printf("        %s %s\n", fType->getName(), field->getName());
	}
	else{
	  printf("        unknown type %s\n", field->getName());
	}
      }
      printf("    }\n");
      break;
    }
    case BPatch_enumerated: {
      printf("    enum %s {\n", argv[1]);
      BPatch_Vector<BPatch_field *> *fields = type->getComponents();
      for (unsigned int i=0; i < fields->size(); i++) {
	BPatch_field *field = (*fields)[i];
	printf("        %s    \t%d\n", field->getName(),
	       field->getValue());
      }
      printf("    }\n");
      break;
    }
    case BPatch_array:
      printArray(type);
      break;
   
    case BPatch_pointer:
      printPtr( type );
      printf("\t %s\n", argv[1]);
      /*
      printf("    %s is pointer to type %s\n", argv[1], 
	     type->getConstituentType()->getName());*/
      break;
    case BPatchSymTypeRange:
      if(type->getConstituentType())
	printf("    %s is a %s of size %d\n",argv[1],
	       (type->getConstituentType())->getName(),type->getSize());
      else
	printf("    %s is a scalar of size %d\n",argv[1],type->getSize());
      if( type->getLow() ){
	printf("        with range %s to %s\n", type->getLow(),
	       type->getHigh());
      }
      break;
    case BPatch_scalar:
      printf("    %s is a scalar of size %d\n",argv[1],type->getSize());
      break;
      
    default:
      printf("    %s is of type %s \n", argv[1], type->getName());
      //int ret = whatisType(cd, interp, argc, argv);
      break;
    }
    
    return TCL_OK;
}

const int DYNINST_NO_ERROR = -1;

void errorFunc(BPatchErrorLevel level, int num, const char **params)
{
    char line[256];

    if (errorLoggingOff) return;

    if ((num == 0) && ((level == BPatchWarning) || (level == BPatchInfo))) {
	 // these are old warnings/status info from paradyn
	 if (!verbose) return;
    }

    const char *msg = bpatch->getEnglishErrorString(num);
    bpatch->formatErrorString(line, sizeof(line), msg, params);

    if (num != DYNINST_NO_ERROR) {

	/*error call back 100 seems to be a variable not found error
	  OR Dyninst assert(0) right after it is thrown, so in either case
	  we dont need to tell the user from here, they will get a message
	  elsewhere
	*/
	if(num != 100 ){ //ccw 9 mar 2004
	        printf("Error #%d (level %d): %s\n", num, level, line);
	}

        // We consider some errors fatal.
        if (num == 101) {
            exit(-1);
        }
    }
}

/* This function prints type information. */
void PrintTypeInfo(char *var, BPatch_type *type) {
      if (!type) {
	printf("NO RETURN TYPE INFO for %s\n", var);
	return;
      }

      switch(type->getDataClass()){
      case BPatch_array:
	printArray( type );
	printf(" %s\n", var);
	break;
	  
      case BPatch_pointer:
	printPtr( type );
	printf(" %s\n", var);
	break;
	
      default:
	printf("%s %s\n", type->getName(), var);
	break;
      }
}
int findAndShowCommand(ClientData, Tcl_Interp *, int argc, TCLCONST char *argv[])
{
  BPatch_Vector<BPatch_function *> functions;// = NULL;
  if ((argc != 3) && (argc!=5)) {
    printf("Syntax error !\n");
    return TCL_ERROR;
  }

  if (strcmp(argv[1], "function")){
    printf("Usage:  'find function <name>' or 'find function <name> in <module>'\n");
    return TCL_ERROR;
  }

  if (argc == 3) {
    if (NULL == appImage->findFunction(argv[2], functions) || !functions.size()) {
      printf("No matches for %s\n", argv[2]);
      return TCL_OK;
    }
  }
  else if (argc == 5) {
    BPatch_module *module = FindModule(argv[4]);
    if (!module) {
      printf("No matches for module %s\n", argv[4]);
      return TCL_ERROR;
    }
    
    if (NULL == module->findFunction(argv[2], functions)) {
      printf("Error finding %s in module %s\n", argv[2], argv[4]);
      return TCL_ERROR;
    }
  }
  
  //Now print all the functions in the set
  char funcName[1024];
  for(unsigned int i=0; i<functions.size(); ++i) {
    (functions)[i]->getName(funcName, 1024);
    PrintTypeInfo(funcName, (functions)[i]->getReturnType());
  }

  return TCL_OK;
}
/* This function displays all the functions in the executable or the functions in a module
 * if a module name is provided.
 */
int ShowFunctions(int argc, TCLCONST char *argv[]) {
    BPatch_Vector<BPatch_function *> *functions = NULL;

    if (argc == 2) 
	functions = appImage->getProcedures();
    else if ( (argc == 4) && (!strcmp(argv[2], "in")) ) {
	BPatch_module *module = FindModule(argv[3]);
	if (!module)
		return TCL_ERROR;

	//Get the module functions
	functions = module->getProcedures();
    }
    else {
	printf("Syntax error !\n");
	return TCL_ERROR;
    }

    if (!functions) {
	printf("Can not get function list!\n");
	return TCL_ERROR;
    }

    //Now print all the functions in the module
    char funcName[1024];
    for(unsigned int i=0; i<functions->size(); ++i) {
	(*functions)[i]->getName(funcName, 1024);
	PrintTypeInfo(funcName, (*functions)[i]->getReturnType());
    }

    return TCL_OK;
}

int verboseCommand(ClientData, Tcl_Interp *, int argc, TCLCONST char *argv[])
{
  verbose = !verbose;
  printf("verbose mode is %s\n", verbose ? "on" : "off");
  return TCL_OK;
}
static int stringCompare(const void *a, const void *b)
{
    return strcmp(*(char **) a, *(char **) b);
}

/* Finds all the modules in the executable and displays the module names */
int ShowModules()
{
    char *prevName;
    char modName[1024];
    char **names;
    BPatch_Vector<BPatch_module *> *modules = appImage->getModules();

    if (!modules) {
	printf("Can not get module info !\n");
	return TCL_ERROR;
    }

    names = (char **) calloc(modules->size(), sizeof(char *));
    for(unsigned int m=0; m<modules->size(); ++m) {
	(*modules)[m]->getName(modName, 1024);
	names[m] = strdup(modName);
    }

    qsort(names, modules->size(), sizeof(char *), stringCompare);
    prevName = strdup("");
    for (unsigned int i=0; i<modules->size(); ++i) {
	 if (strcmp(prevName, names[i])) {
	     printf("%s\n", names[i]);
	 }
	 if (prevName) free(prevName);
	 prevName = names[i];
    }
    if (prevName) free(prevName);
    free (names);

    return TCL_OK;
}

/*
 * This function finds and prints all the parameters
 * of a function whose name is given as input.
 */
int ShowParameters(int argc, TCLCONST char *argv[]) {
    if ( (argc != 4) || (strcmp(argv[2], "in")) ) {
	printf("Usage: show parameters in <function>\n");
	return TCL_ERROR;
    }

    BPatch_Vector<BPatch_function *> bpfv;
    if (NULL == appImage->findFunction(argv[3], bpfv) || !bpfv.size()) {
	printf("Invalid function name: %s\n", argv[3]);
	return TCL_ERROR;
    }

    if (bpfv.size() > 1) {
      printf("warning:  found %d functions called %s, picking the first\n", 
	     bpfv.size(), argv[3]);
    }

    BPatch_function *fp = bpfv[0];

    if (!fp) {
	printf("Invalid function name: %s\n", argv[3]);
	return TCL_ERROR;
    }

    BPatch_Vector<BPatch_localVar *> *params = fp->getParams();
    for(unsigned int i=0; i<params->size(); ++i) {
	PrintTypeInfo((char *) (*params)[i]->getName(), (*params)[i]->getType());
    }

    return TCL_OK;
}

/*
 * This function finds and prints all the local variables
 * of a function whose name is given as input.
 */
int ShowLocalVars(const char *funcName) {
  BPatch_Vector<BPatch_function *> bpfv;

  if (NULL == appImage->findFunction(funcName, bpfv) || !bpfv.size()) {
    printf("Invalid function name: %s\n", funcName);
    return TCL_ERROR;
  }

  if (bpfv.size() > 1) {
    printf("warning:  found %d functions called %s, picking the first\n", 
	   bpfv.size(), funcName);
  }

  BPatch_function *fp = bpfv[0];
  
  if (!fp) {
    printf("Invalid function name: %s\n", funcName);
    return TCL_ERROR;
  }

  BPatch_Vector<BPatch_localVar *> *vars = fp->getVars();
  for(unsigned int i=0; i<vars->size(); ++i) {
    PrintTypeInfo((char *) (*vars)[i]->getName(), (*vars)[i]->getType());
  }
  
  delete vars;
  
  return TCL_OK;
}

/* Finds all the global vars in the executable and prints their names. */
int ShowGlobalVars() {
    BPatch_Vector<BPatch_variableExpr *> *vars = appImage->getGlobalVariables();

    if (!vars) {
	printf("Can not get global variable info !\n");
	return TCL_ERROR;
    }

    for(unsigned int i=0; i<vars->size(); ++i) {
	PrintTypeInfo((*vars)[i]->getName(), (BPatch_type *) (*vars)[i]->getType());
    }

    return TCL_OK;
}

/* Finds all global variables or local variables in the function */
int ShowVariables(int argc, TCLCONST char *argv[])
{
    if (argc == 2)
	return ShowGlobalVars();

    if ( (argc == 4) && (!strcmp(argv[2], "in")) )
	return ShowLocalVars(argv[3]);

    //Should not reach here!
    return TCL_ERROR;
}

/* Displays either modules or functions in a specific module in the executable */

int showCommand(ClientData, Tcl_Interp *, int argc, TCLCONST char *argv[])
{
    if (!haveApp()) return TCL_ERROR;

    if (argc == 1) {
	printf("Usage: show [modules|functions|variables]\n");
	printf("or     show functions in <module>\n");
	printf("or     show [variables|parameters] in <function>\n");
	return TCL_ERROR;
    }

    if (!strcmp(argv[1], "modules"))
	return ShowModules();

    if (!strcmp(argv[1], "functions"))
	return ShowFunctions(argc, argv);

    if (!strcmp(argv[1], "variables"))
	return ShowVariables(argc, argv);

    if (!strcmp(argv[1], "parameters"))
	return ShowParameters(argc, argv);

    printf("Syntax error!\n");
    return TCL_ERROR;
}

int dsetCommand(ClientData, Tcl_Interp *, int argc, TCLCONST char *argv[])
{
    if (argc == 1) {
	printf("Usage: dset <variable> <value>");
	return TCL_ERROR;
    }

    if (!strcmp(argv[1], "recTramps")) {
        if (!strcmp(argv[2], "false")) {
	    bpatch->setTrampRecursive(false);
	    return TCL_OK;
        } else if (!strcmp(argv[2], "true")) {
	    bpatch->setTrampRecursive(true);
	    return TCL_OK;
	} else {
	    printf("Usage: dset recTramps [true|false]\n");
	    return TCL_ERROR;
	}
    }

    printf("Syntax error!\n");
    return TCL_ERROR;
}

/* Displays how many times input fcn is called */
int countCommand(ClientData, Tcl_Interp *interp, int argc, TCLCONST char *argv[])
{
    if (!haveApp()) return TCL_ERROR;

    if (argc != 2) {
	printf("Usage: count <function>\n");
	return TCL_ERROR;
    }

    BPatch_Vector<BPatch_function *> bpfv;
    if (NULL == appImage->findFunction(argv[1], bpfv) || !bpfv.size()) {
      printf("Invalid function name: %s\n", argv[1]);
      return TCL_ERROR;
    }

    if (bpfv.size() > 1) {
      printf("warning:  found %d functions called %s, picking the first\n", 
	     bpfv.size(), argv[1]);
    }

    BPatch_function *fp = bpfv[0];

    if (!fp) {
	printf("Invalid function name: %s\n", argv[1]);
	return TCL_ERROR;
    }

    const char *fcnName = argv[1];
    char cmdBuf[1024];

    sprintf(cmdBuf, "declare int _%s_cnt", fcnName);
    if (Tcl_Eval(interp, cmdBuf) == TCL_ERROR)
	return TCL_ERROR;

    sprintf(cmdBuf, "at main entry { _%s_cnt = 0; } count", fcnName);
    if (Tcl_Eval(interp, cmdBuf) == TCL_ERROR)
	return TCL_ERROR;

    sprintf(cmdBuf, "at %s entry { _%s_cnt++; } count", fcnName, fcnName);
    if (Tcl_Eval(interp, cmdBuf) == TCL_ERROR)
	return TCL_ERROR;

    sprintf(cmdBuf, "at main exit { printf(\"%s called %%d times\\n\", _%s_cnt); } count", 
									fcnName, fcnName);
    if (Tcl_Eval(interp, cmdBuf) == TCL_ERROR)
	return TCL_ERROR;

    return TCL_OK;
}

/*
 * Replace all calls to fcn1 with calls fcn2
 */
int repFunc(const char *name1, const char *name2) {

  BPatch_Vector<BPatch_function *> bpfv;
  if (NULL == appImage->findFunction(name1, bpfv) || !bpfv.size()) {
    printf("Invalid function name: %s\n", name1);
    return TCL_ERROR;
  }
  
  if (bpfv.size() > 1) {
    printf("warning:  found %d functions called %s, picking the first\n", 
	   bpfv.size(), name1);
  }
  
  BPatch_function *func1 = bpfv[0];

  if (!func1) {
    printf("Invalid function name: %s\n", name1);
    return TCL_ERROR;
  }

  BPatch_Vector<BPatch_function *> bpfv2;
  if (NULL == appImage->findFunction(name2, bpfv2) || !bpfv2.size()) {
    printf("Invalid function name: %s\n", name2);
    return TCL_ERROR;
  }
  
  if (bpfv2.size() > 1) {
    printf("warning:  found %d functions called %s, picking the first\n", 
	   bpfv2.size(), name2);
  }
  
  BPatch_function *func2 = bpfv2[0];

  if (!func2) {
    printf("Invalid function name: %s\n", name2);
    return TCL_ERROR;
  }

  if (appThread->replaceFunction(*func1, *func2))
    return TCL_OK;
  
  return TCL_ERROR;
}

/*
 * Replace all or n'th call in func1 with a call to func2
 */
int repCall(const char *func1, const char *func2) {

  // Replace function calls
  int n = 0;
  char *ptr = strchr(func1,':');
  if (ptr) {
    *ptr = '\0';
    n = atoi(ptr+1) - 1;
    if (n == -1) {
      printf("Invalid number is entered!\n");
      return TCL_ERROR;
    }
  }
  
  BPatch_Vector<BPatch_function *> bpfv2;
  if (NULL == appImage->findFunction(func2, bpfv2) || !bpfv2.size()) {
    printf("Invalid function name: %s\n", func2);
    return TCL_ERROR;
  }
  
  if (bpfv2.size() > 1) {
    printf("warning:  found %d functions called %s, picking the first\n", 
	   bpfv2.size(), func2);
  }

  BPatch_function *newFunc = bpfv2[0];
  if (newFunc == NULL) {
    printf("Invalid function name: %s\n", func2);
    return TCL_ERROR;
  }

  BPatch_Vector<BPatch_function *> found_funcs;
  if ((NULL == appImage->findFunction(func1, found_funcs, 1)) || !found_funcs.size()) {
    printf("%s[%d]:  CANNOT CONTINUE  :  %s not found\n", __FILE__, __LINE__, func1);
    return TCL_ERROR;
  }
    
  if (1 < found_funcs.size()) {
    printf("%s[%d]:  WARNING  :  %d functions called '%s'found.  Using the first\n", 
	   __FILE__, __LINE__, 
	   found_funcs.size(), func1);
  }
    
  BPatch_Vector<BPatch_point *> *points = found_funcs[0]->findPoint(BPatch_subroutine);

    if (points == NULL) {
	printf("Could not locate function %s\n", func1);
	return TCL_ERROR;
    }

    if (points->size() == 0) {
	printf("Function %s has no calls!\n", func1);
	return TCL_ERROR;
    }

    if (n > (int) points->size()) {
	printf("Function %s does not have %d calls!\n", func1, n);
	return TCL_ERROR;
    }

    if (n == -1) {
	//Remove all function calls
	for(unsigned int i=0; i<points->size(); ++i) {
		if (!appThread->replaceFunctionCall(*((*points)[i]), *newFunc) ) {
			printf("Unable to replace call %d !\n", i);
			return TCL_ERROR;
		}
	}
	return TCL_OK;
    }

    //Replace n'th function call
    if (!appThread->replaceFunctionCall(*((*points)[n]), *newFunc) ) {
	printf("Unable to replace call %d !\n", n);
	return TCL_ERROR;
    }

    return TCL_OK;
}

/*
 * Replaces functions or function calls with input function
 */
int replaceCommand(ClientData, Tcl_Interp *, int argc, TCLCONST char *argv[])
{
    if (!haveApp()) return TCL_ERROR;

    if ( (argc != 5) || ( (strcmp(argv[1], "function")) && (strcmp(argv[1], "call")) )
			 || (strcmp(argv[3], "with")) ) 
    {
	printf("Usage: replace function <function1> with <function2>\n");
	printf("or     replace call <function1>[:n] with <function2>\n");
	return TCL_ERROR;
    }

    if (!strcmp(argv[1], "function"))
	return repFunc(argv[2], argv[4]);

    if (!strcmp(argv[1], "call"))
	return repCall(argv[2], argv[4]);

    printf("Invalid option %s\n", argv[1]);
    return TCL_ERROR;
}

/*
 * Print a message while entering a function
 */
int traceFunc(Tcl_Interp *interp, const char *name) 
{
  BPatch_Vector<BPatch_function *> bpfv2;
  if (NULL == appImage->findFunction(name, bpfv2) || !bpfv2.size()) {
    printf("Invalid function name: %s\n", name);
    return TCL_ERROR;
  }
  
  if (bpfv2.size() > 1) {
    printf("warning:  found %d functions called %s, picking the first\n", 
	   bpfv2.size(), name);
  }

  BPatch_function *func = bpfv2[0];
  if (!func) {
    printf("Invalid function name: %s\n", name);
    return TCL_ERROR;
  }

    char cmdBuf[1024];
    sprintf(cmdBuf, 
	"at %s entry { printf(\"Entering function %s\\n\"); } trace", name, name);
    if (Tcl_Eval(interp, cmdBuf) == TCL_ERROR)
	return TCL_ERROR;

    sprintf(cmdBuf, 
	"at %s exit { printf(\"Exiting function %s\\n\"); } trace", name, name);
    return Tcl_Eval(interp, cmdBuf);
}

/*
 * Trace all the function in a module
 */
int traceMod(Tcl_Interp *interp, const char *name) {
    BPatch_Vector<BPatch_function *> *functions = NULL;

    BPatch_module *module = FindModule(name);
    if (!module)
	return TCL_ERROR;

    //Get the module functions
    functions = module->getProcedures();

    if (!functions) {
	printf("Can not get function list!\n");
	return TCL_ERROR;
    }

    //Now print all the functions in the module
    char funcName[1024];
    for(unsigned int i=0; i<functions->size(); ++i) {
	(*functions)[i]->getName(funcName, 1024);
	if (traceFunc(interp, funcName) == TCL_ERROR)
		return TCL_ERROR;
    }

    return TCL_OK;
}

/*
 * Trace functions alone or in a module
 */
int traceCommand(ClientData, Tcl_Interp *interp, int argc, TCLCONST char *argv[])
{
    if (!haveApp()) return TCL_ERROR;

    if (argc < 3) {
	printf("Usage: trace function <function>\n");
	printf("or     trace functions in <module>\n");
	return TCL_ERROR;
    }

    if (!strcmp(argv[1], "function"))
	return traceFunc(interp, argv[2]);

    if (!strcmp(argv[1], "functions") && !strcmp(argv[2], "in"))
	return traceMod(interp, argv[3]);

    return TCL_ERROR;
}


int untraceFunc(const char *name)
{
    DynerList<IPListElem *>::iterator i;
    int removed_a_point = 0;
    IPListElem *ip;

    i = iplist.begin();
    while(i != iplist.end()) {
      ip = *i;

      if ((ip->instType == TRACE) && !strcmp(name, ip->function)) {
        printf("removing tracing for function %s\n", ip->function);
        fflush(NULL);
        iplist.erase(i);
        delete ip;
        removed_a_point = 1;
      }
      else i++;

    }

    if (removed_a_point)
       return TCL_OK;

    printf("function %s is not currently traced\n", name);
    return TCL_ERROR;
}

int untraceMod(const char *name)
{
    BPatch_Vector<BPatch_function *> *functions = NULL;

    BPatch_module *module = FindModule(name);
    if (!module)
	return TCL_ERROR;

    //Get the module functions
    functions = module->getProcedures();

    if (!functions) {
	printf("Can not get function list in the module!\n");
	return TCL_ERROR;
    }

    //Now print all the functions in the module
    char funcName[1024];
    for(unsigned int i=0; i<functions->size(); ++i) {
	(*functions)[i]->getName(funcName, 1024);
	if (untraceFunc(funcName) == TCL_ERROR)
		return TCL_ERROR;
    }

    return TCL_OK;
}

/*
 * Deletes trace effects
 */
int untraceCommand(ClientData, Tcl_Interp *, int argc, TCLCONST char *argv[])
{
    if (!haveApp()) return TCL_ERROR;

    if (argc < 3) {
	printf("Usage: untrace function <function>\n");
	printf("or     untrace functions in <module>\n");
	return TCL_ERROR;
    }

    if (!strcmp(argv[1], "function"))
	return untraceFunc(argv[2]);

    if (!strcmp(argv[1], "functions") && !strcmp(argv[2], "in"))
	return untraceMod(argv[3]);

    return TCL_ERROR;
}

/*
 * Enable or disable the execution of snippets
 */
int mutationsCommand(ClientData, Tcl_Interp *, int argc, TCLCONST char *argv[])
{
    if (argc != 2) {
	printf("Usage: mutations [enable|disable]\n");
	return TCL_ERROR;
    }

    if (!haveApp()) return TCL_ERROR;

    if (!strcmp(argv[1], "enable")) {
	appThread->setMutationsActive(true);
	return TCL_OK;
    }

    if (!strcmp(argv[1], "disable")) {
	appThread->setMutationsActive(false);
	return TCL_OK;
    }

    printf("Invalid option!\n"); 
    return TCL_ERROR;
}

/*
 * Remove all or n'th function call in the input function
 */
int removeCommand(ClientData, Tcl_Interp *, int argc, TCLCONST char *argv[])
{
    if (argc != 2) {
	printf("Usage: removecall <function>[:n]\n");
	return TCL_ERROR;
    }

    if (!haveApp()) return TCL_ERROR;

    int n = -1;
    char *ptr = strchr(argv[1],':');
    if (ptr) {
	*ptr = '\0';
	n = atoi(ptr+1) - 1;
	if (n == -1) {
		printf("Invalid number is entered!\n");
		return TCL_ERROR;
	}
    }

    BPatch_Vector<BPatch_function *> found_funcs;
    if ((NULL == appImage->findFunction(argv[1], found_funcs, 1)) || !found_funcs.size()) {
      printf("%s[%d]:  CANNOT CONTINUE  :  %s not found\n", __FILE__, __LINE__, argv[1]);
      return TCL_ERROR;
    }
    
    if (1 < found_funcs.size()) {
      printf("%s[%d]:  WARNING  :  %d functions called '%s'found.  Using the first\n", 
	     __FILE__, __LINE__, 
	     found_funcs.size(), argv[1]);
    }
    
    BPatch_Vector<BPatch_point *> *points = found_funcs[0]->findPoint(BPatch_subroutine);

    if (points == NULL) {
	printf("Could not locate function %s\n", argv[1]);
	return TCL_ERROR;
    }

    if (points->size() == 0) {
	printf("Function %s has no calls!\n", argv[1]);
	return TCL_ERROR;
    }

    if (n > (int) points->size()) {
	printf("Function %s does not have %d calls!\n", argv[1], n);
	return TCL_ERROR;
    }

    if (n == -1) {
	//Remove all function calls
	for(unsigned int i=0; i<points->size(); ++i) {
		if (!appThread->removeFunctionCall(*((*points)[i])) ) {
			printf("Unable to remove call %d !\n", i);
			return TCL_ERROR;
		}
	}
	return TCL_OK;
    }

    //Remove n'th function call
    if (!appThread->removeFunctionCall(*((*points)[n])) ) {
	printf("Unable to remove call %d !\n", n);
	return TCL_ERROR;
    }

    return TCL_OK;
}

/*
 * Write the in-memory version of the program to the specified file
 */
int dumpCommand(ClientData, Tcl_Interp *, int argc, TCLCONST char *argv[])
{
    if (argc != 2) {
	printf("Usage: dump <file name>\n");
	return TCL_ERROR;
    }

    if (!haveApp()) return TCL_ERROR;

    appThread->dumpImage(argv[1]);

    return TCL_OK;
}

/*
 * remove all the code inserted into target program
 */
int detachCommand(ClientData, Tcl_Interp *, int argc, TCLCONST char **)
{
    if (argc != 1) {
	printf("Usage: detach\n");
	return TCL_ERROR;
    }

    if (!haveApp()) return TCL_ERROR;

    appThread->detach(true);

    return TCL_OK;
}

/*
 * enable or disable debug parse of the mutatee
 */
int debugParse(ClientData, Tcl_Interp *, int argc, TCLCONST char **argv)
{
    if (argc > 2) {
	printf("Usage: debugparse [enable | disable]");
	return TCL_ERROR;
    }

    if (argc == 1) {
	printf("Debug parsing is %s\n", (bpatch->parseDebugInfo()?"on":"off") );
        printf("Usage: debugparse [enable | disable]\n");
	return TCL_OK;
    }

    bool flag;
    if ( !strcmp(argv[1], "enable") ) 
	flag = true;
    else if ( !strcmp(argv[1], "disable") )
	flag = false;
    else {
	printf("Invalid option for debugparse command.\n");
	printf("Usage: debugparse [enable | disable]");
	return TCL_ERROR;
    }

    bpatch->setDebugParsing(flag);

    return TCL_OK;
}
	
int exitDyner(ClientData, Tcl_Interp *, int, TCLCONST char **)
{

    if (haveApp()) {
	// this forces terminatation if the app has not been detached
	if (appThread) delete appThread;
    }
    exit(0);
}

//
//
int Tcl_AppInit(Tcl_Interp *interp)
{
    if (Tcl_Init(interp) == TCL_ERROR) {
	return TCL_ERROR;
    }
    
    //Create BPatch library
    bpatch = new BPatch;
    if (!bpatch)
	return TCL_ERROR;

    Tcl_CreateCommand(interp, "at", (Tcl_CmdProc*)instStatement, NULL, NULL);
    Tcl_CreateCommand(interp, "attach", (Tcl_CmdProc*)attachPid, NULL, NULL);
    Tcl_CreateCommand(interp, "break", (Tcl_CmdProc*)condBreak, NULL, NULL);
    Tcl_CreateCommand(interp, "declare", (Tcl_CmdProc*)newVar, NULL, NULL);
    Tcl_CreateCommand(interp, "listbreak", (Tcl_CmdProc*)listBreak, NULL, NULL);
    Tcl_CreateCommand(interp, "deletebreak", (Tcl_CmdProc*)deleteBreak, NULL, NULL);
    Tcl_CreateCommand(interp, "dset", (Tcl_CmdProc*)dsetCommand, NULL, NULL);
    Tcl_CreateCommand(interp, "help", (Tcl_CmdProc*)help, NULL, NULL);
    Tcl_CreateCommand(interp, "exit", (Tcl_CmdProc*)exitDyner, NULL, NULL);
    Tcl_CreateCommand(interp, "kill", (Tcl_CmdProc*)killApp, NULL, NULL);
    Tcl_CreateCommand(interp, "load", (Tcl_CmdProc*)loadCommand, NULL, NULL);
    Tcl_CreateCommand(interp, "run", (Tcl_CmdProc*)runApp, NULL, NULL);
    Tcl_CreateCommand(interp, "print", (Tcl_CmdProc*)printVar, NULL, NULL);
    Tcl_CreateCommand(interp, "whatis", (Tcl_CmdProc*)whatisVar, NULL, NULL);
    Tcl_CreateCommand(interp, "show", (Tcl_CmdProc*)showCommand, NULL, NULL);
    Tcl_CreateCommand(interp, "find", (Tcl_CmdProc*)findAndShowCommand, NULL, NULL);
    Tcl_CreateCommand(interp, "verbose", (Tcl_CmdProc*)verboseCommand, NULL, NULL);
    Tcl_CreateCommand(interp, "count", (Tcl_CmdProc*)countCommand, NULL, NULL);
    Tcl_CreateCommand(interp, "replace", (Tcl_CmdProc*)replaceCommand, NULL, NULL);
    Tcl_CreateCommand(interp, "trace", (Tcl_CmdProc*)traceCommand, NULL, NULL);
    Tcl_CreateCommand(interp, "untrace", (Tcl_CmdProc*)untraceCommand, NULL, NULL);
    Tcl_CreateCommand(interp, "mutations", (Tcl_CmdProc*)mutationsCommand, NULL, NULL);
    Tcl_CreateCommand(interp, "removecall", (Tcl_CmdProc*)removeCommand, NULL, NULL);
    //Tcl_CreateCommand(interp, "dump", (Tcl_CmdProc*)dumpCommand, NULL, NULL);
    Tcl_CreateCommand(interp, "detach", (Tcl_CmdProc*)detachCommand, NULL, NULL);
    Tcl_CreateCommand(interp, "execute", (Tcl_CmdProc*)execStatement, NULL, NULL);
    Tcl_CreateCommand(interp, "listinst", (Tcl_CmdProc*)listInstrument, NULL, NULL);
    Tcl_CreateCommand(interp, "deleteinst", (Tcl_CmdProc*)deleteInstrument, NULL, NULL);
    Tcl_CreateCommand(interp, "debugparse", (Tcl_CmdProc*)debugParse, NULL, NULL);
#if defined(rs6000_ibm_aix4_1) || defined(sparc_sun_solaris2_4) || defined(i386_unknown_linux2_0)  

    Tcl_CreateCommand(interp, "where", (Tcl_CmdProc*)where, NULL, NULL);
    Tcl_CreateCommand(interp, "up", (Tcl_CmdProc*)whereUp, NULL, NULL);
    Tcl_CreateCommand(interp, "down", (Tcl_CmdProc*)whereDown, NULL, NULL);

    Tcl_CreateCommand(interp, "save", (Tcl_CmdProc*)saveWorld, NULL, NULL);
    Tcl_CreateCommand(interp, "saveStart", (Tcl_CmdProc*)saveStart, NULL, NULL);

#endif

    Tcl_AllowExceptions(interp);

    bpatch->registerErrorCallback(errorFunc);
    bpatch->setTypeChecking(false);
    bpatch->registerExitCallback(&exitCallback);

    return TCL_OK;
}

int main(int argc, char *argv[])
{
    if (argc >= 2 && !strcmp(argv[1], "-debug")) {
	printf("parser debug enabled\n");
	dynerdebug = 1;
	verbose = true;
    }

#if !defined(i386_unknown_nt4_0)
    signal(SIGINT, INThandler);
#endif

    Tcl_Main(argc, argv, Tcl_AppInit);
    return 0;
}


template class DynerList<BPListElem*>;
template class DynerList<IPListElem*>;
template class DynerList<runtimeVar*>;

