/* $Log: main.C,v $
/* Revision 1.27  1995/08/24 15:02:55  hollings
/* AIX/SP-2 port (including option for split instruction/data heaps)
/* Tracing of rexec (correctly spawns a paradynd if needed)
/* Added rtinst function to read getrusage stats (can now be used in metrics)
/* Critical Path
/* Improved Error reporting in MDL sematic checks
/* Fixed MDL Function call statement
/* Fixed bugs in TK usage (strings passed where UID expected)
/*
 * Revision 1.26  1995/08/23  21:03:21  mjrg
 * moved call to readStartUpFile() to after commands in configuration
 * file are executed.
 *
 * Revision 1.25  1995/08/20  03:42:02  newhall
 * changed arguments to DM_sequential_init
 *
 * Revision 1.24  1995/08/18  22:00:16  mjrg
 * Added calls to metDoProcess, metDoDaemon, metDoTunable.
 *
 * Revision 1.23  1995/08/16  15:17:40  krisna
 * double-bug fix.
 *   * do not pass addresses of stack variables into thread functions
 *   * do not use the first item of a struct as a scalar
 *
 * Revision 1.22  1995/08/14 22:49:49  tamches
 * Removed the TC thread.
 * The main tunable constant dictionaries are global variables
 * (in TCthread/TCmain.C); their constructors automatically
 * initialize the TC registry before main() even starts.  Hence,
 * no problems declaring any tunable constants after main starts.
 * But, don't declare any tunable constants as global variables.
 *
 * Revision 1.21  1995/08/13  23:22:26  tamches
 * Moved tcl/tk initialization code here from UImain.
 * tcl/tk initialization is now the very first thing done
 * in main()
 *
 * Revision 1.20  1995/08/12  22:27:51  newhall
 * added calls to DM and VM sequential initialization routines
 *
 * Revision 1.19  1995/08/11  21:51:16  newhall
 * Parsing of PDL files is done before thread creation
 * Removed call to dataManager kludge method function
 *
 * Revision 1.18  1995/06/02  20:55:58  newhall
 * made code compatable with new DM interface
 *
 * Revision 1.17  1995/05/18  11:00:31  markc
 * added mdl hooks
 *
 * Revision 1.16  1995/02/27  19:13:49  tamches
 * Many changes to reflect changes in tunable constants.
 * First change: TCthread is launched
 * other changes: Many tunable constants are declared here (within
 * main()) since they may no longer be declared globally in any
 * module.
 *
 * Revision 1.15  1995/02/16  08:25:09  markc
 * Removed system includes
 * Added includes of posix interfaces
 *
 * Revision 1.14  1994/11/02  04:39:01  karavan
 * added -s commandline option for a tcl script
 *
 * Revision 1.13  1994/11/01  22:27:22  karavan
 * Changed debugging printfs to PARADYN_DEBUG calls.
 *
 * Revision 1.12  1994/09/22  01:22:48  markc
 * Gave correct signature for signal
 *
 * Revision 1.11  1994/08/22  15:54:49  markc
 * Added command line argument to specify application config file.
 *
 * Revision 1.10  1994/07/28  22:31:42  krisna
 * proper prototypes and starting code for thread main functions
 *
 * Revision 1.9  1994/07/19  23:52:58  markc
 * Moved "include "metricExt.h"" to main.C from paradyn.h to remove false
 * dependencies.
 *
 * Revision 1.8  1994/07/07  03:26:24  markc
 * Added calls to parser routines.
 *
 * Revision 1.7  1994/05/18  00:51:03  hollings
 * We don't want SIGPIPEs to kill us.
 *
 * Revision 1.6  1994/05/10  03:57:54  hollings
 * Changed data upcall to return array of buckets.
 *
 * Revision 1.5  1994/04/28  22:07:39  newhall
 * added PARADYN_DEBUG macro: prints debug message if PARADYNDEBUG
 * environment variable has value >= 1
 *
 * Revision 1.4  1994/04/21  23:25:19  hollings
 * changed to no initial paradynd being defined.
 *
 * Revision 1.3  1994/04/10  19:08:48  newhall
 * added visualization manager thread create
 *
 * Revision 1.2  1994/04/05  04:36:48  karavan
 * Changed order of thread initialization to avoid deadlock.  Added global
 * user variables for data manager, uim, and performance consultant.
 *
 * Revision 1.1  1994/03/29  20:20:54  karavan
 * initial version for testing
 * */

/*
 * main.C - main routine for paradyn.  
 *   This routine creates DM, UIM, VM, and PC threads.
 */

#include "tclclean.h"
#include "tkclean.h"

#include "../TCthread/tunableConst.h"
#include "util/h/headers.h"
#include "paradyn.h"
#include "thread/h/thread.h"
#include "dataManager.thread.SRVR.h"
#include "VM.thread.SRVR.h"

extern void *UImain(void *);
extern void *DMmain(void *);
extern void *PCmain(void *);
extern void *VMmain (void *);
//extern void *TCmain (void *); // tunable consts

#define MBUFSIZE 256
#define DEBUGBUFSIZE	4096

thread_t UIMtid;
thread_t MAINtid;
thread_t PCtid;
thread_t DMtid;
thread_t VMtid;
//thread_t TCtid; // tunable constants

// expanded stack by a factor of 10 to support AIX - jkh 8/14/95
char UIStack[327680];

// applicationContext *context;
dataManagerUser *dataMgr;
performanceConsultantUser *perfConsult;
UIMUser *uiMgr;
VMUser  *vmMgr;
int paradyn_debug;
char debug_buf[DEBUGBUFSIZE];

#define PRINT_DEBUG_MACRO				\
do {							\
	va_list args;					\
	va_start(args,format);				\
	(void) fflush(stdout);				\
	(void) vsprintf(debug_buf, format, args);	\
	(void) fprintf(stdout,"THREAD %d: %s\n",	\
	       thr_self(), debug_buf);			\
	(void) fflush(stdout);                          \
	va_end(args);					\
} while (0)

void print_debug_macro(const char* format, ...){
  if(paradyn_debug > 0)
     PRINT_DEBUG_MACRO;
  return;
}

void eFunction(int errno, char *message)
{
    printf("error: %s\b", message);
    abort();
}

extern bool metDoTunable();
extern bool metDoProcess();
extern bool metMain(string&);
extern bool metDoProcess();
extern bool metDoDaemon();


extern void tclpanic(Tcl_Interp *, const char *msg);
Tcl_Interp *interp;
Tk_Window   mainWindow;
int         tty;

int
main (int argc, char **argv)
{

//  CLargStruct* clargs;
  char mbuf[MBUFSIZE];
  unsigned int msgsize;
  tag_t mtag;
  char *temp;

  // Initialize tcl/tk
  interp = Tcl_CreateInterp();
  assert(interp);

  mainWindow = Tk_CreateMainWindow(interp, NULL, "paradyn", "Paradyn");
  if (mainWindow == NULL)
     tclpanic(interp, "Could not Tk_CreateMainWindow");
  Tk_GeometryRequest(mainWindow, 725, 475);

  if (false)
     // This is cool for X debugging...but we don't want it normally.
     // It forces a flush after every X event -- no buffering.
     XSynchronize(Tk_Display(mainWindow), True);

  tty = isatty(0);
  Tcl_SetVar(interp, "tcl_interactive", "0", TCL_GLOBAL_ONLY);

  if (Tcl_Init(interp) == TCL_ERROR)
     tclpanic(interp, "tcl_init() failed (perhaps TCL_LIBRARY not set?)");
  if (Tk_Init(interp) == TCL_ERROR)
     tclpanic(interp, "tk_init() failed (perhaps TK_LIBRARY not set?");

  // copy command-line arguments into tcl vrbles argc / argv
  char *args = Tcl_Merge(argc - 1, argv + 1);
  Tcl_SetVar(interp, "argv", args, TCL_GLOBAL_ONLY);
  ckfree(args);

  string argcStr = string(argc - 1);
  Tcl_SetVar(interp, "argc", argcStr.string_of(), TCL_GLOBAL_ONLY);
  Tcl_SetVar(interp, "argv0", argv[0], TCL_GLOBAL_ONLY);

  // Here is one tunable constant that is definitely intended to be hard-coded in:
   tunableBooleanConstantDeclarator tcInDeveloperMode("developerMode",
	 "Allow access to all tunable constants, including those limited to developer mode.  (Use with caution)",
	 false, // initial value
	 NULL, // no callback routine (yet)
         userConstant);

  //
  // We check our own read/write events.
  //
  P_signal(SIGPIPE, (P_sig_handler) SIG_IGN);

  if (argc != 1 && argc != 3 && argc != 5) {
    printf("usage: %s [-f <filename>] [-s <tclscriptname>]\n", argv[0]);
    exit(-1);
  }

  // get paradyn_debug environment var PARADYNDEBUG, if its value
  // is > 1, then PARADYN_DEBUG msgs will be printed to stdout
  if((temp = (char *) getenv("PARADYNDEBUG"))){
     paradyn_debug = atoi(temp);
  }
  else {
    paradyn_debug = 0;
  }

// parse the command line arguments
  int a_ct=0;
  char *fname=0, *sname=0;
  while (argv[a_ct + 1]) {
    if (!strcmp(argv[a_ct], "-f")) {
      fname = argv[a_ct+1];
      break;
    } else {
      if (!strcmp(argv[a_ct], "-s")) {
        sname = argv[a_ct+1];
        break;
      }
    }
    a_ct++;
  }

// get tid of parent
  MAINtid = thr_self();

// Structure used to pass initial arguments to data manager
//  init_struct init; init.tid = MAINtid; init.met_file = fname;

// call sequential initialization routines
  dataManager::DM_sequential_init(fname);
  VM::VM_sequential_init(); 


     /* initialize the 4 main threads of paradyn: data manager, visi manager,
        user interface manager, performance consultant */
  
// initialize Tunable Constants thread
//  if (THR_ERR == thr_create(NULL, // stack
//			    0, // stack size
//			    TCmain, // entry-point function
//			    NULL, // args
//			    0, // flags
//			    &TCtid))
//     exit(1);
//  PARADYN_DEBUG (("TC thread created\n"));
//  // wait until TC has properly initialized (it'll send us a blank msg)
//  mtag = MSG_TAG_TC_READY;
//  msgsize = MBUFSIZE;
//  (void)msg_recv(&mtag, mbuf, &msgsize);
////  cout << "pdMain: TC thread has given us the okay to continue creating threads!" << endl;
  
// Declare some tunable constants (declaring them here makes them last as long
// as this routine does, which is "forever")
  extern bool predictedCostLimitValidChecker(float); // in PCauto.C
  tunableFloatConstantDeclarator pcl ("predictedCostLimit",
				 "Max. allowable perturbation of the application.",
				 100.0, // initial value
				 predictedCostLimitValidChecker, // validation function (in PCauto.C)
				 NULL, // callback routine
				 userConstant);

  tunableFloatConstantDeclarator mnh ("maxEval",
				 "Max. number of hypotheses to consider at once.",
				 25.0, // initial
				 0.0,  // min
				 250.0, // max
				 NULL, // callback
				 userConstant);
  tunableFloatConstantDeclarator hysRange("hysteresisRange",
					  "Fraction above and below threshold that a test should use.",
					  0.15, // initial
					  0.0, // min
					  1.0, // max
					  NULL, // callback
					  userConstant);

  //
  // Fix this soon... This should be based on some real information.
  //
  tunableFloatConstantDeclarator minObsTime("minObservationTime",
					    "min. time (in seconds) to wait after changing inst to start try hypotheses.",
					    1.0, // initial
					    0.0, // min
					    60.0, // max
					    NULL, // callback
					    userConstant);

  tunableFloatConstantDeclarator sufficientTime("sufficientTime",
						"How long to wait (in seconds) before we can conclude a hypothesis is false.",
						6.0, // initial
						0.0, // min
						1000.0, // max
						NULL,
						userConstant);
  tunableBooleanConstantDeclarator printNodes("printNodes",
					      "Print out changes to the state of SHG nodes",
					      false, // initial value
					      NULL, // callback
					      developerConstant);

  tunableBooleanConstantDeclarator printTestResults("printTestResults",
						    "Print out the result of each test as it is computed",
						    false,
						    NULL,
						    developerConstant);

  tunableBooleanConstantDeclarator pcEvalPrint("pcEvalPrint",
					       "Print out the values of tests each time they are evaluated",
					       false,
					       NULL,
					       developerConstant);
					       
  tunableBooleanConstantDeclarator suppressSHG("suppressSHG",
					       "Don't print the SHG",
					       false,
					       NULL,
					       userConstant);

// initialize DM

  if (thr_create(0, 0, DMmain, (void *) &MAINtid, 0, 
		 (unsigned int *) &DMtid) == THR_ERR)
    exit(1);
  PARADYN_DEBUG (("DM thread created\n"));

  msgsize = MBUFSIZE;
  mtag = MSG_TAG_DM_READY;
  msg_recv(&mtag, mbuf, &msgsize);
  msg_send (DMtid, MSG_TAG_ALL_CHILDREN_READY, (char *) NULL, 0);
  dataMgr = new dataManagerUser (DMtid);
  // context = dataMgr->createApplicationContext(eFunction);

// initialize UIM 
 
//  if ((clargs = (CLargStruct *) malloc(sizeof(CLargStruct))) == 0) {
//    perror("malloc(ClargsStruct)");
//    return -1;
//  }
//  clargs->clargc = argc;	
//  clargs->clargv = argv; 

//  if (thr_create (UIStack, sizeof(UIStack), &UImain, (void *) clargs, 
//		  0, &UIMtid) == THR_ERR) 
  if (thr_create (UIStack, sizeof(UIStack), &UImain, NULL,
		  0, &UIMtid) == THR_ERR) 
    exit(1);
  PARADYN_DEBUG (("UI thread created\n"));

  msgsize = MBUFSIZE;
  mtag = MSG_TAG_UIM_READY;
  msg_recv(&mtag, mbuf, &msgsize);
  msg_send (UIMtid, MSG_TAG_ALL_CHILDREN_READY, (char *) NULL, 0);
  uiMgr = new UIMUser (UIMtid);

// initialize PC

  if (thr_create(0, 0, PCmain, (void*) &MAINtid, 0, 
		 (unsigned int *) &PCtid) == THR_ERR)
    exit(1);
  PARADYN_DEBUG (("PC thread created\n"));

  msgsize = MBUFSIZE;
  mtag = MSG_TAG_PC_READY;
  msg_recv(&mtag, mbuf, &msgsize);
  msg_send (PCtid, MSG_TAG_ALL_CHILDREN_READY, (char *) NULL, 0);
  perfConsult = new performanceConsultantUser (PCtid);

// initialize VM
  if (thr_create(0, 0, VMmain, (void *) &MAINtid, 0, 
		 (unsigned int *) &VMtid) == THR_ERR)
    exit(1);

  PARADYN_DEBUG (("VM thread created\n"));
  msgsize = MBUFSIZE;
  mtag = MSG_TAG_VM_READY;
  msg_recv(&mtag, mbuf, &msgsize);
  msg_send (VMtid, MSG_TAG_ALL_CHILDREN_READY, (char *) NULL, 0);
  vmMgr = new VMUser (VMtid);

  // execute the commands in the configuration files
  metDoTunable();
  metDoDaemon();
  metDoProcess();

  // keep this here to prevent UI from starting up till everything's 
  // been initialized properly!!
  //  -OR-
  // move this elsewhere to create a race condition
  if (sname)
    uiMgr->readStartupFile (sname);

// wait for UIM thread to exit 

  thr_join (UIMtid, NULL, NULL);

//  Tcl_DeleteInterp(interp);
//    Unfortunately, status lines core dump on exit if I
//    un-comment this out. --ari
}
