#include <stdio.h>
#include <stdlib.h>
#if defined(sparc_sun_solaris2_4)
	#include <sys/time.h>
#else
	#include <sys/timeb.h>

	extern "C" int ftime(struct timeb *);
#endif

#include "tcl.h"
#include "dynerList.h"
#include "BPatch.h"
#include "BPatch_type.h"
#include "BPatch_collections.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"
#include "breakpoint.h"

extern "C" {
	int usleep(useconds_t);

	void set_lex_input(char *s);
	int dynerparse();
}

extern "C" int dynerdebug;

int debugPrint = 0;
BPatch_point *targetPoint;
bool errorLoggingOff = false;

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
    char		     *condition;

    BPListElem(int _number, char *_function, BPatch_procedureLocation _where,
	       BPatch_callWhen _when, char *_condition,
	       BPatchSnippetHandle *_handle);
    ~BPListElem();
};

class runtimeVar {
public:
    runtimeVar(BPatch_variableExpr *v, char *n) { var = v, name = strdup(n); }
    BPatch_variableExpr *var;
    char *name;
    bool readValue(void *buf) { return var->readValue(buf); }
};

class IPListElem: public ListElem {
public:
    char		     *statement;

    IPListElem(int _number, char *_function, BPatch_procedureLocation _where,
	       BPatch_callWhen _when, char *_condition,
	       BPatchSnippetHandle *_handle);
    ~IPListElem();
};

BPatch bpatch;
BPatch_thread *appThread = NULL;
BPatch_image *appImage = NULL;
static BPatch_variableExpr *bpNumber = NULL;
static int bpCtr = 1;
static int ipCtr = 1;
static DynerList<ListElem *> bplist;
static DynerList<ListElem *> iplist;
static DynerList<runtimeVar *> varList;


BPListElem::BPListElem(int _number, char *_function,
    BPatch_procedureLocation _where, BPatch_callWhen _when, char *_condition,
    BPatchSnippetHandle *_handle)
{
    number = _number;
    function = strdup(_function);
    where = _where;
    when = _when;
    condition = strdup(_condition);
    handle = _handle;
}

BPListElem::~BPListElem()
{
    free(function);
    free(condition);
}

IPListElem::IPListElem(int _number, char *_function,
    BPatch_procedureLocation _where, BPatch_callWhen _when, char *_condition,
    BPatchSnippetHandle *_handle)
{
    number = _number;
    function = strdup(_function);
    where = _where;
    when = _when;
    statement = strdup(_condition);
    handle = _handle;
}

IPListElem::~IPListElem()
{
    free(function);
    free(statement);
    if (handle) delete handle;
}

ListElem *findBP(int n)
{
    DynerList<ListElem *>::iterator i;

    for (i = bplist.begin(); i != bplist.end(); i++) {
	if ((*i)->number == n) return (*i);
    }

    return NULL;
}

int help(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{

    printf("at <function> [entry|exit|preCall|postCall] <statement> - instert statement\n");
    printf("declare <variable> <type> - create a new variable of type <type>\n");
    printf("deletebreak <breakpoint number ...> - delete breakpoint(s)\n");
    printf("cbreak <function> [entry|exit|preCall|postCall] <condition> - set a conditional\n");
    printf("  break point\n");
    printf("listbreak - list break points\n");
    printf("load <program> [arguments] [< filename] [> filename] - load a program\n");
    printf("run - run or continue the loaded program\n");
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

bool name2loc(char *s, BPatch_procedureLocation &where,
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

int loadApp(char *pathname, char **args)
{
    printf("Loading \"%s\"\n", pathname);

    if (appThread != NULL) delete appThread;
    bplist.clear();
    iplist.clear();
    varList.clear();
    appThread = bpatch.createProcess(pathname, args);
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

    return TCL_OK;

}


int loadLib(char *libName) {
    if (!haveApp()) return TCL_ERROR;

    if (appThread->loadLibrary(libName))
	return TCL_OK;

    return TCL_ERROR;
}

int loadCommand(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
    if ( (argc == 3) && (!strcmp(argv[1], "library")) )
	return loadLib(argv[2]);

    if (argc < 2) {
	printf("Usage load <program> [<arguments>]\n");
	return TCL_ERROR;
    }

    return loadApp(argv[1], &argv[1]);
}

int listBreak(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
    if (!haveApp()) return TCL_ERROR;

    BPListElem *curr;
    DynerList<ListElem *>::iterator i;

    for (i = bplist.begin(); i != bplist.end(); i++) {
	curr = (BPListElem *) *i;
	printf("%2d: in %s (%s), condition %s\n",
		curr->number, curr->function,
		loc2name(curr->where, curr->when),
		curr->condition);
    }
}

int deleteBreak(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
    if (!haveApp()) return TCL_ERROR;

    if (argc < 2) {
	fprintf(stderr, "Specify breakpoint(s) to delete.\n");
	return TCL_ERROR;
    }

    int ret = TCL_OK;
    for (int j = 1; j < argc; j++) {
	int n = atoi(argv[j]);

       	ListElem *i = findBP(n);
	if (i == NULL) {
	    printf("No such breakpoint: %d\n", n);
	    ret = TCL_ERROR;
	} else {
	    appThread->deleteSnippet(i->handle);
	    bplist.erase(i);
	    delete i;
	    printf("Breakpoint %d deleted.\n", n);
	}
    }

    return ret;
}

int runApp(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
#if defined(sparc_sun_solaris2_4) 
    hrtime_t start_time, end_time;

    start_time = gethrtime();
#else
    struct timeb start_time, end_time;

    ftime(&start_time);
#endif

    if (!haveApp()) return TCL_ERROR;

    if (bpNumber) {
	int inval = -1;
	bpNumber->writeValue(&inval);
    }

    // Start of code to continue the process.
    dprintf("starting program execution.\n");
    appThread->continueExecution();

    while (!appThread->isStopped() && !appThread->isTerminated())
	usleep(250);

    int bp = -1;
    if (bpNumber) bpNumber->readValue(&bp);

    if (appThread->isTerminated()) {
	printf("\nApplication exited.\n");
    } else if (appThread->isStopped() && bp > 0) {
	printf("\nStopped at break point %d.\n", bp);
	ListElem *i = findBP(bp);
	if (i != NULL) {
	    BPListElem *curr = (BPListElem *) i;
	    printf("%s (%s); %s\n",
		    curr->function,
		    loc2name(curr->where, curr->when),
		    curr->condition);
	}
    } else {
	printf("\nStopped.\n");
    }

#if defined(sparc_sun_solaris2_4)
    end_time = gethrtime();
    printf("Running time (nanoseconds): %lld\n", end_time - start_time);
#else
    ftime(&end_time);
    printf("Running time (miliseconds): %d\n", (end_time.time - start_time.time) * 1000 + 
							end_time.millitm - start_time.millitm);
#endif

    return TCL_OK;
}

int killApp(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
    if (!haveApp()) return TCL_ERROR;

    appThread->terminateExecution();

    return TCL_OK;
}

extern BPatch_snippet *parse_result;
int condBreak(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
    if (!haveApp()) return TCL_ERROR;

    if (bpNumber == NULL) {
	bpNumber = appThread->malloc(*appImage->findType("int"));
	if (bpNumber == NULL) {
	    fprintf(stderr, "Unable to allocate memory in the inferior process.\n");
	    exit(1);
	}
    }

    if (argc < 3) {
	printf("Wrong number of paramters.\n");
	return TCL_ERROR;
    }

    int expr_start = 2;
    BPatch_procedureLocation where;
    BPatch_callWhen when;
    if (name2loc(argv[2], where, when)) {
	expr_start = 3;
    } else {
	where = BPatch_entry;
	when = BPatch_callBefore;
    }

    BPatch_Vector<BPatch_point *> *points =
	appImage->findProcedurePoint(argv[1], where);
    if (points == NULL) {
	fprintf(stderr, "Unable to locate function: %s\n", argv[1]);
	return TCL_ERROR;
    }

    // Count up how large a buffer we need for the whole line
    int line_len = 0;
    for (int i = expr_start; i < argc; i++)
	line_len += strlen(argv[i]) + 1;
    line_len++;
    // Make the buffer and copy the line into it
    char *line_buf = new char[line_len];
    *line_buf = '\0';
    for (int i = expr_start; i < argc; i++) {
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

    // Make a snippet to tell us which breakpoint it is
    BPatch_arithExpr storeBPNum(BPatch_assign,
			        *bpNumber, BPatch_constExpr(bpCtr));

    // Make a snippet to break at the breakpoint
    BPatch_function *breakFunc = appImage->findFunction("DYNINSTbreakPoint");
    if (breakFunc == NULL) {
	fprintf(stderr, "Unable to find function DYNINSTbreakPoint (required for setting break points)\n");
	delete parse_result;
	delete line_buf;
	return TCL_ERROR;
    }

    BPatch_Vector<BPatch_snippet *> nullArgs;
    BPatch_funcCallExpr callBreak(*breakFunc, nullArgs);

    // store the break point number, then  break
    BPatch_arithExpr doBreak(BPatch_seq, storeBPNum, callBreak);
    

    // Call the break snippet conditionally
    BPatch_ifExpr condBreak(*(BPatch_boolExpr *)parse_result, doBreak); 

    delete parse_result;

    BPatchSnippetHandle *handle =
	appThread->insertSnippet(condBreak, *points, when,BPatch_lastSnippet);
    if (handle == 0) {
	fprintf(stderr, "Error inserting snippet.\n");
	delete line_buf;
	return TCL_ERROR;
    }

    BPListElem *bpl =
	new BPListElem(bpCtr, argv[1], where, when, line_buf, handle);
    delete line_buf;

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
  else if((type->getDataClass() == BPatch_scalar)||(type->getDataClass() == BPatch_built_inType)||
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

int instStatement(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
    if (!haveApp()) return TCL_ERROR;

    if (argc < 3) {
	printf("Wrong number of paramters.\n");
	return TCL_ERROR;
    }

    int expr_start = 2;
    BPatch_procedureLocation where;
    BPatch_callWhen when;
    if (name2loc(argv[2], where, when)) {
	expr_start = 3;
    } else {
	where = BPatch_entry;
	when = BPatch_callBefore;
    }

    BPatch_function *func = appImage->findFunction(argv[1]);
    if (func == NULL) {
	fprintf(stderr, "Unable to locate function: %s\n", argv[1]);
	return TCL_ERROR;
    }


    BPatch_Vector<BPatch_point *> *points = func->findPoint(where);
				// appImage->findProcedurePoint(argv[1], where);

    if (points == NULL) {
	fprintf(stderr, "Unable to locate function: %s\n", argv[1]);
	return TCL_ERROR;
    }

    //Assign targetPoint using function entry point
    targetPoint = (*(func->findPoint(BPatch_entry)))[0];

    // Count up how large a buffer we need for the whole line
    int line_len = 0;
    for (int i = expr_start; i < argc; i++)
	line_len += strlen(argv[i]) + 1;
    line_len += 2;
    // Make the buffer and copy the line into it
    char *line_buf = new char[line_len];
    *line_buf = '\0';
    for (int i = expr_start; i < argc; i++) {
	strcat(line_buf, argv[i]);
	if (i != argc-1) strcat(line_buf, " ");
    }
    strcat(line_buf, "\n");

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
	new IPListElem(ipCtr, argv[1], where, when, line_buf, handle);
    delete line_buf;

    iplist.push_back(snl);

    // printf("Inst point %d set.\n", ipCtr);
    ipCtr++;

    return TCL_OK;
}

void printVarRecursive(BPatch_variableExpr *var, int level)
{
    int iVal;
    int pVal;
    float fVal;

    BPatch_dataClass dc;
    BPatch_type *type = (BPatch_type *) var->getType();

    dc = type->getDataClass();
    if (!strcmp(type->getName(), "int")) {
	/* print out the integer */
	var->readValue((void *) &iVal);
	printf("%d\n", iVal);
    } else if (!strcmp(type->getName(), "float")) {
	/* print out the float */
	var->readValue((void *) &fVal);
	printf("%f\n", fVal);
    } else if (dc == BPatch_pointer) {
	/* print out the float */
	var->readValue((void *) &pVal);
	printf("0x%x\n", pVal);
    } else if (dc == BPatch_structure) {
	printf("struct {\n");
	level++;
	BPatch_Vector<BPatch_variableExpr *> *fields = var->getComponents();
	for (int i=0; i < fields->size(); i++) {
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
	printf("<unknown type>\n");
    }
}

int printVar(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
    //if (!haveApp()) return TCL_ERROR;

    BPatch_variableExpr *var = findVariable(argv[1]);
    if (!var) {
	printf("%s is not defined\n", argv[1]);
	return TCL_ERROR;
    }

    printf("    %s = ", argv[1]);
    printVarRecursive(var, 1);

    return TCL_OK;
}

/*
 * declare <type> <variable name>
 */
int newVar(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
    if (!haveApp()) return TCL_ERROR;

    BPatch_type *type = appImage->findType(argv[1]);
    if (!type) {
	printf("type %s is not defined\n", argv[1]);
	return TCL_ERROR;
    }

    BPatch_variableExpr *newVar = appThread->malloc(*type);
    if (!newVar) {
	printf("unable to create variable.\n");
	return TCL_ERROR;
    }

    varList.push_back(new runtimeVar(newVar, argv[2]));
}

BPatch_variableExpr *findVariable(char *name)
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

    // Now check the function locals
    if (targetPoint) {
	var = appImage->findVariable(*targetPoint, name);
	if (var) return var;
    }

    // Check global vars in the mutatee
    errorLoggingOff = true;
    var = appImage->findVariable(name);
    errorLoggingOff = false;

    if (var) return var;

    printf("Unable to locate variable %s\n", name);
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
int whatisParam(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
  if (!haveApp()) return TCL_ERROR;
  BPatch_function * func = appImage->findBPFunction(argv[2]);
  printf("func is %x\n", func);
  if(!func){
    printf("%s is not defined\n", argv[3]);
    return TCL_ERROR;
  }
  
  
  BPatch_localVar * lvar = func->findLocalVar(argv[3]);
  if(lvar){
    BPatch_type *type = lvar->getType();
    if( type ){
      switch (type->getDataClass()) {
      case BPatch_array:
	printArray( type );
	break;
	
      case BPatch_pointer:
	printPtr( type );
	printf("\t %s\n", argv[3]);
	break;
	
      default:
	printf("    %s is of type %s \n", argv[3], type->getName());
	break;
      }
    }
    //print local variable info
    printf("        %s is a local variable in function %s\n", lvar->getName(),
	   argv[2]);
    printf("        Declared on line %d and has a Frame offset of %d\n",
	   lvar->getLineNum(), lvar->getFrameOffset());
  }
  else{
    lvar = func->findLocalParam(argv[3]);
    
    if (!lvar) {
      BPatch_variableExpr *var = findVariable(argv[3]);
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
	printf("    %s is of type %s \n", argv[3], type->getName());
	break;
      }
     printf("      %s is a Global variable\n", argv[3]);
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
	  printf("\t %s\n", argv[3]);
	  break;
	  
	default:
	  printf("    %s is of type %s \n", argv[3], lType->getName());
	  break;
	}
	printf("       %s is a parameter of the function %s\n",lvar->getName(),
	   argv[2]);
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
 
int whatisFunc(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
    if (!haveApp()) return TCL_ERROR;
    // If the function is found it returns the BPatch_function.
    // findFunction() creates a new BPatch_function so it is not
    // used here. --jdd 5/27/99
    BPatch_function * func = appImage->findBPFunction(argv[1]);
    
    if(!func){
      printf("%s is not defined\n", argv[1]);
      return TCL_ERROR;
    }
    printf("    Function %s ", argv[1]);
    BPatch_type * retType = func->getReturnType();
    if(retType)
      switch(retType->getDataClass()){
      case BPatch_scalar:
      case BPatch_built_inType:
      case BPatchSymTypeRange:
	printf("  with return type %s\n",retType->getName() );
	break;
      case BPatch_array:
	printf("  with return type ");
	printArray( retType );
	printf("\n");
	break;
	  
      case BPatch_pointer:
	printf("  with return type ");
	printPtr( retType );
	printf("\n");
	break;
	
      default:
	printf("    %s has unknown return type %s %d\n",argv[1],
	       retType->getName(), retType->getID());
	break;
      }
    else
      printf("   unknown return type for %s\n", argv[1]);
    BPatch_Vector<BPatch_localVar *> *params = func->getParams();
    if( params->size())
      printf("        Parameters: \n");
    for (int i=0; i < params->size(); i++) {
      BPatch_localVar *localVar = (*params)[i];
      BPatch_type *lType = (BPatch_type *) localVar->getType();
      if (lType){
	if( (lType->getDataClass())  == BPatch_pointer){
	  printPtr(lType);
	  printf(" \t %s\n", localVar->getName());
	}
	else if( (lType->getDataClass()) == BPatch_array){
	  printArray( lType );
	}
	else
	  printf("        %s %s\n",lType->getName(), localVar->getName());
	
      }
      else{
	printf("   unknown type %s\n", localVar->getName());
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
 
int whatisType(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
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
      for (int i=0; i < fields->size(); i++) {
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
      for (int i=0; i < fields->size(); i++) {
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
      for (int i=0; i < fields->size(); i++) {
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
    case BPatch_built_inType:
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

int whatisVar(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
    if (!haveApp()) return TCL_ERROR;
    
    if (argc == 4){ //looking for a local variable
      if(!(strcmp(argv[1], "-scope"))){
	int ret = whatisParam(cd, interp, argc, argv );
	return ret;
      } else {
	printf("inavlid argument %s\n", argv[1]);
	return TCL_ERROR;
      }
    }

    BPatch_variableExpr *var = findVariable(argv[1]);
    if (!var) {
	int ret = whatisType(cd, interp, argc, argv);
	return ret;
    }

    BPatch_type *type = (BPatch_type *) var->getType();
    switch (type->getDataClass()) {
    case BPatch_structure: {
      printf("    struct %s {\n", argv[1]);
      BPatch_Vector<BPatch_field *> *fields = type->getComponents();
      for (int i=0; i < fields->size(); i++) {
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
      for (int i=0; i < fields->size(); i++) {
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
      for (int i=0; i < fields->size(); i++) {
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
    case BPatch_built_inType:
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
	 return;
    }

    const char *msg = bpatch.getEnglishErrorString(num);
    bpatch.formatErrorString(line, sizeof(line), msg, params);

    if (num != DYNINST_NO_ERROR) {
        printf("Error #%d (level %d): %s\n", num, level, line);

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

/* This function displays all the functions in the executable or the functions in a module
 * if a module name is provided.
 */
int ShowFunctions(int argc, char *argv[]) {
    BPatch_Vector<BPatch_function *> *functions = NULL;

    if (argc == 2) 
	functions = appImage->getProcedures();
    else if ( (argc == 4) && (!strcmp(argv[2], "in")) ) {
	//First locate the module using module name given in argv[3]
	BPatch_Vector<BPatch_module *> *modules = appImage->getModules();

	if (!modules) {
		printf("Can not get module info !\n");
		return TCL_ERROR;
	}

	char modName[1024];
	BPatch_module *module = NULL;

	for(int i=0; i<modules->size(); ++i) {
		(*modules)[i]->getName(modName, 1024);

		if (!strcmp(modName, argv[3])) {
			module = (*modules)[i];
			break;
		}
	}

	if (!module) {
		printf("Module %s is not found !\n", argv[3]);
		return TCL_ERROR;
	}

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
    for(int i=0; i<functions->size(); ++i) {
	(*functions)[i]->getName(funcName, 1024);
	PrintTypeInfo(funcName, (*functions)[i]->getReturnType());
    }

    return TCL_OK;
}

/* Finds all the modules in the executable and displays the module names */
int ShowModules()
{
    char modName[1024];
    BPatch_Vector<BPatch_module *> *modules = appImage->getModules();

    if (!modules) {
	printf("Can not get module info !\n");
	return TCL_ERROR;
    }

    for(int i=0; i<modules->size(); ++i) {
	(*modules)[i]->getName(modName, 1024);
	printf("%s\n", modName);
    }

    return TCL_OK;
}

/*
 * This function finds and prints all the parameters
 * of a function whose name is given as input.
 */
int ShowParameters(int argc, char *argv[]) {
    if ( (argc != 4) || (strcmp(argv[2], "in")) ) {
	printf("Syntax error!\n");
	return TCL_ERROR;
    }

    BPatch_function *fp = appImage->findFunction(argv[3]);

    if (!fp) {
	printf("Invalid function name: %s\n", argv[3]);
	return TCL_ERROR;
    }

    BPatch_Vector<BPatch_localVar *> *params = fp->getParams();
    for(int i=0; i<params->size(); ++i) {
	PrintTypeInfo((char *) (*params)[i]->getName(), (*params)[i]->getType());
    }

    return TCL_OK;
}

/*
 * This function finds and prints all the local variables
 * of a function whose name is given as input.
 */
int ShowLocalVars(char *funcName) {
    BPatch_function *fp = appImage->findFunction(funcName);

    if (!fp) {
	printf("Invalid function name: %s\n", funcName);
	return TCL_ERROR;
    }

    BPatch_Vector<BPatch_localVar *> *vars = fp->localVariables->getAllVars();
    for(int i=0; i<vars->size(); ++i) {
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

    for(int i=0; i<vars->size(); ++i) {
	PrintTypeInfo((*vars)[i]->getName(), (BPatch_type *) (*vars)[i]->getType());
    }

    return TCL_OK;
}

/* Finds all global variables or local variables in the function */
int ShowVariables(int argc, char *argv[])
{
    if (argc == 2)
	return ShowGlobalVars();

    if ( (argc == 4) && (!strcmp(argv[2], "in")) )
	return ShowLocalVars(argv[3]);

    //Should not reach here!
    return TCL_ERROR;
}

/* Displays either modules or functions in a specific module in the executable */

int showCommand(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
    if (!haveApp()) return TCL_ERROR;

    if (argc == 1)
	return TCL_ERROR;

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

/* Displays how many times input fcn is called */
int countCommand(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
    if (!haveApp()) return TCL_ERROR;

    if (argc != 2) {
	printf("Invalid number of arguments for count command!\n");
	return TCL_ERROR;
    }

    BPatch_function *fp = appImage->findFunction(argv[1]);

    if (!fp) {
	printf("Invalid function name: %s\n", argv[1]);
	return TCL_ERROR;
    }

    char *fcnName = argv[1];
    char cmdBuf[1024];

    sprintf(cmdBuf, "declare int _%s_cnt", fcnName);
    if (Tcl_Eval(interp, cmdBuf) == TCL_ERROR)
	return TCL_ERROR;

    sprintf(cmdBuf, "at main entry { _%s_cnt = 0; }", fcnName);
    if (Tcl_Eval(interp, cmdBuf) == TCL_ERROR)
	return TCL_ERROR;

    sprintf(cmdBuf, "at %s entry { _%s_cnt++; }", fcnName, fcnName);
    if (Tcl_Eval(interp, cmdBuf) == TCL_ERROR)
	return TCL_ERROR;

    sprintf(cmdBuf, "at main exit { printf(\"%s called %%d times\\n\", _%s_cnt); }", 
									fcnName, fcnName);
    if (Tcl_Eval(interp, cmdBuf) == TCL_ERROR)
	return TCL_ERROR;

    return TCL_OK;
}

/*
 * Replace all calls to fcn1 with calls fcn2
 */
int repFunc(char *name1, char *name2) {

    BPatch_function *func1 = appImage->findFunction(name1);
    if (!func1) {
	printf("Invalid function name: %s\n", name1);
	return TCL_ERROR;
    }

    BPatch_function *func2 = appImage->findFunction(name2);
    if (!func2) {
	printf("Invalid function name: %s\n", name2);
	return TCL_ERROR;
    }

    if (appThread->replaceFunction(*func1, *func2))
	return TCL_OK;

    return TCL_ERROR;
}

/*
 * Replaces functions or function calls with input function
 */
int replaceCommand(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
    if (!haveApp()) return TCL_ERROR;

    if ( (argc != 5) || (strcmp(argv[1], "function")) || 
			(strcmp(argv[3], "with")) ) 
    {
	printf("Invalid number of arguments to replace command!\n");
	return TCL_ERROR;
    }

    if (!strcmp(argv[1], "function"))
	return repFunc(argv[2], argv[4]);

    // Replace function calls
    // Not implemented yet!
 
    return TCL_OK;
}

//
// Tcl AppInit defines the new commands for dyninst
//
int Tcl_AppInit(Tcl_Interp *interp)
{
    if (Tcl_Init(interp) == TCL_ERROR) {
	return TCL_ERROR;
    }
    
    Tcl_CreateCommand(interp, "at", instStatement, NULL, NULL);
    Tcl_CreateCommand(interp, "cbreak", condBreak, NULL, NULL);
    Tcl_CreateCommand(interp, "declare", newVar, NULL, NULL);
    Tcl_CreateCommand(interp, "listbreak", listBreak, NULL, NULL);
    Tcl_CreateCommand(interp, "deletebreak", deleteBreak, NULL, NULL);
    Tcl_CreateCommand(interp, "help", help, NULL, NULL);
    Tcl_CreateCommand(interp, "kill", killApp, NULL, NULL);
    Tcl_CreateCommand(interp, "load", loadCommand, NULL, NULL);
    Tcl_CreateCommand(interp, "run", runApp, NULL, NULL);
    Tcl_CreateCommand(interp, "print", printVar, NULL, NULL);
    Tcl_CreateCommand(interp, "whatis", whatisVar, NULL, NULL);
    Tcl_CreateCommand(interp, "show", showCommand, NULL, NULL);
    Tcl_CreateCommand(interp, "count", countCommand, NULL, NULL);
    Tcl_CreateCommand(interp, "replace", replaceCommand, NULL, NULL);
    
    bpatch.registerErrorCallback(errorFunc);
    bpatch.setTypeChecking(false);

    return TCL_OK;
}

int main(int argc, char *argv[])
{
    if (argc >= 2 && !strcmp(argv[1], "-debug")) {
	printf("parser debug enabled\n");
	dynerdebug = 1;
    }

    Tcl_Main(argc, argv, Tcl_AppInit);
    return 0;
}
