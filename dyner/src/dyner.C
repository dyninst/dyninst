#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>
#include <tcl.h>
#include <list>

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"
#include "BPatch_type.h"
#include "breakpoint.h"

extern "C" int sleep(unsigned);
extern "C" int usleep(unsigned);

extern "C" {
int sleep(unsigned);
void set_lex_input(char *s);
int yyparse();
extern int yydebug;
}

int debugPrint = 0;
BPatch_point *targetPoint;
BPatch_function *targetFunc;
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
static list<ListElem *> bplist;
static list<ListElem *> iplist;
static list<runtimeVar *> varList;


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

list<ListElem *>::iterator findBP(int n)
{
    list<ListElem *>::iterator i;

    for (i = bplist.begin(); i != bplist.end(); i++) {
	if ((*i)->number == n) break;
    }

    return i;
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
	when = BPatch_callBefore;
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

int loadApp(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
    char *pathname = argv[1];

    if (argc < 2) {
	printf("Usage load <program> [<arguments>]\n");
	return TCL_ERROR;
    }
    
    printf("Loading \"%s\"\n", pathname);

    if (appThread != NULL) delete appThread;
    bplist.erase(bplist.begin(), bplist.end());
    appThread = bpatch.createProcess(pathname, &argv[1]);
    bpNumber = NULL;

    if (!appThread || appThread->isTerminated()) {
	fprintf(stderr, "Unable to run test program.\n");
	appThread = NULL;
	return TCL_ERROR;
    }

    // Read the program's image and get an associated image object
    appImage = appThread->getImage();

    if (appImage) return TCL_OK;

    return TCL_ERROR;

}


int listBreak(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
    if (!haveApp()) return TCL_ERROR;

    BPListElem *curr;
    list<ListElem *>::iterator i;

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

       	list<ListElem *>::iterator i = findBP(n);
	if (i == bplist.end()) {
	    printf("No such breakpoint: %d\n", n);
	    ret = TCL_ERROR;
	} else {
	    appThread->deleteSnippet((*i)->handle);
	    delete *i;
	    bplist.erase(i);
	    printf("Breakpoint %d deleted.\n", n);
	}
    }

    return ret;
}

int runApp(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
    hrtime_t start_time, end_time;

    start_time = gethrtime();

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
	list<ListElem *>::iterator i = findBP(bp);
	if (i != bplist.end()) {
	    BPListElem *curr = (BPListElem *) *i;
	    printf("%s (%s); %s\n",
		    curr->function,
		    loc2name(curr->where, curr->when),
		    curr->condition);
	}
    } else {
	printf("\nStopped.\n");
    }

    end_time = gethrtime();

    printf("Running time (nanoseconds): %lld\n", end_time - start_time);

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
    if (yyparse() != 0) {
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
  if( type->type() == BPatch_pointer ){
    printPtr( type->getConstituentType() );
    printf("*");
  }
  else if((type->type() == BPatch_scalar)||(type->type() == BPatchSymTypeRange)){
    printf("        %s ", type->getName());
  }
  else{
    switch( type->type() ){
    case BPatch_enumerated:
      printf("        enum %s", type->getName());
      break;
    case BPatch_structure:
      printf("        struct %s", type->getName());
      break;
    case BPatch_union:
      printf("        union %s", type->getName());
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

    BPatch_Vector<BPatch_point *> *points =
	appImage->findProcedurePoint(argv[1], where);
    if (points == NULL) {
	fprintf(stderr, "Unable to locate function: %s\n", argv[1]);
	return TCL_ERROR;
    }
    if (points->size() != 1) {
	fprintf(stderr, "function %s is not unique (due to overloading?)\n", 
	    argv[1]);
	return TCL_ERROR;
    }

    targetPoint = (*points)[0];
    assert(targetPoint);

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

    printf("calling parse of %s\n", line_buf);
    set_lex_input(line_buf);
    if (yyparse() != 0) {
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

    printf("Inst point %d set.\n", ipCtr);
    ipCtr++;

    return TCL_OK;
}

printVarRecursive(BPatch_variableExpr *var, int level)
{
    int iVal;
    int pVal;
    float fVal;

    BPatch_dataClass dc;
    BPatch_type *type = var->getType();

    dc = type->type();
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
    if (!haveApp()) return TCL_ERROR;

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
 * declare <variable name> <type>
 */
int newVar(ClientData cd, Tcl_Interp *interp, int argc, char *argv[])
{
    if (!haveApp()) return TCL_ERROR;

    BPatch_type *type = appImage->findType(argv[2]);
    if (!type) {
	printf("type %s is not defined\n", argv[1]);
	return TCL_ERROR;
    }

    BPatch_variableExpr *newVar = appThread->malloc(*type);
    if (!newVar) {
	printf("unable to create variable.\n");
	return TCL_ERROR;
    }

    varList.push_back(new runtimeVar(newVar, argv[1]));
}

BPatch_variableExpr *findVariable(char *name)
{
    BPatch_variableExpr *var;
    list<runtimeVar *>::iterator i;

    // first check the function locals
    if (targetPoint) {
	var = appImage->findVariable(*targetPoint, name);
	if (var) return var;
    }

    errorLoggingOff = true;
    var = appImage->findVariable(name);
    errorLoggingOff = false;

    if (var) return var;

    // now look for runtime created variables
    for (i = varList.begin(); i != varList.end(); i++) {
	if (!strcmp(name, (*i)->name)) {
	    var = new BPatch_variableExpr(*((*i)->var));
	    return (var);
	}
    }

    printf("unable to locate variable %s\n", name);
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
      switch (type->type()) {
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
      BPatch_type *type = var->getType();
      switch (type->type()) {
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
      const BPatch_type *lType = lvar->getType();
      if (lType){
	switch (lType->type()) {
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
      switch(retType->type()){
      case BPatch_scalar:
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
      const BPatch_type *lType = localVar->getType();
      if (lType){
	if( (lType->type())  == BPatch_pointer){
	  printPtr(lType);
	  printf(" \t %s\n", localVar->getName());
	}
	else if( (lType->type()) == BPatch_array){
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

    BPatch_dataClass dc = type->type();
    switch (dc) {
    case BPatch_structure: {
      printf("    struct %s {\n", argv[1]);
      BPatch_Vector<BPatch_field *> *fields = type->getComponents();
      for (int i=0; i < fields->size(); i++) {
	BPatch_field *field = (*fields)[i];
	const BPatch_type *fType = field->getType();
	if (fType){
	  switch (fType->type()) {
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
	const BPatch_type *fType = field->getType();
	if (fType){
	  switch (fType->type()) {
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

    BPatch_type *type = var->getType();
    switch (type->type()) {
    case BPatch_structure: {
      printf("    struct %s {\n", argv[1]);
      BPatch_Vector<BPatch_field *> *fields = type->getComponents();
      for (int i=0; i < fields->size(); i++) {
	BPatch_field *field = (BPatch_field *) (*fields)[i];
	const BPatch_type *fType = field->getType();
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
	const BPatch_type *fType = field->getType();
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
    Tcl_CreateCommand(interp, "load", loadApp, NULL, NULL);
    Tcl_CreateCommand(interp, "run", runApp, NULL, NULL);
    Tcl_CreateCommand(interp, "print", printVar, NULL, NULL);
    Tcl_CreateCommand(interp, "whatis", whatisVar, NULL, NULL);
    
    bpatch.registerErrorCallback(errorFunc);
    bpatch.setTypeChecking(false);

    return TCL_OK;
}

int main(int argc, char *argv[])
{
    if (argc >= 2 && !strcmp(argv[1], "-debug")) {
	printf("parser debug enabled\n");
	yydebug = 1;
    }

    Tcl_Main(argc, argv, Tcl_AppInit);
    return 0;
}
