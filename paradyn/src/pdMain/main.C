/*
 * Copyright (c) 1996-1999 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
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

// $Id: main.C,v 1.56 2000/07/28 17:22:10 pcroth Exp $

/*
 * main.C - main routine for paradyn.  
 *   This routine creates DM, UIM, VM, and PC threads.
 */

#include "../TCthread/tunableConst.h"
#include "common/h/headers.h"
#include "paradyn.h"
#include "thread/h/thread.h"
#include "dataManager.thread.SRVR.h"
#include "VM.thread.SRVR.h"
#include "../UIthread/tkTools.h" // tclpanic
#include "pdutil/h/makenan.h"
#include "paradyn/src/DMthread/BufferPool.h"
#include "paradyn/src/DMthread/DVbufferpool.h"

#include "tcl.h"
#include "tk.h"


// trace data streams
BufferPool<traceDataValueType>  tracedatavalues_bufferpool;

// maybe this should be a thread, but for now it's a global var.
BufferPool<dataValueType>  datavalues_bufferpool;


extern void *UImain(void *);
extern void *DMmain(void *);
extern void *PCmain(void *);
extern void *VMmain (void *);

extern bool mpichUnlinkWrappers();

#define MBUFSIZE 256
#define DEBUGBUFSIZE	4096

thread_t UIMtid;
thread_t MAINtid;
thread_t PCtid;
thread_t DMtid;
thread_t VMtid;
//thread_t TCtid; // tunable constants

// expanded stack by a factor of 10 to support AIX - jkh 8/14/95
// wrapped it in an ifdef so others don't pay the price --ari 10/95
#if defined(rs6000_ibm_aix3_2) || defined(rs6000_ibm_aix4_1)
char UIStack[327680];
char DMStack[327680];
#else
char UIStack[32768];
char DMStack[32768];
#endif


// applicationContext *context;
dataManagerUser *dataMgr;
performanceConsultantUser *perfConsult;
UIMUser *uiMgr;
VMUser  *vmMgr;
int paradyn_debug=0;
char debug_buf[DEBUGBUFSIZE];

// default_host defines the host where programs run when no host is
// specified in a PCL process definition, or in the process definition window.
string default_host="";
string local_domain="";

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
    printf("error #%d: %s\b", errno, message);
    abort();
}

//extern bool metMain(string&);
extern bool metDoTunable();
extern bool metDoProcess();
extern bool metDoDaemon();

Tcl_Interp *interp;
int         tty;

bool inDeveloperMode = false; // global variable used elsewhere
void develModeCallback(bool newValue) {
   inDeveloperMode = newValue;

   // plus any other necessary action...
   // The shg wants to hear of such changes, so it can resize its
   // status line (w/in the shg window) appropriately
   extern void shgDevelModeChange(Tcl_Interp *, bool);
   shgDevelModeChange(interp, inDeveloperMode);
}

int
main (int argc, char **argv)
{
  char mbuf[MBUFSIZE];
  unsigned int msgsize;
  thread_t mtid;
  tag_t mtag;
  char *temp=NULL;

  // Initialize tcl/tk
  interp = Tcl_CreateInterp();
  assert(interp);

  tty = isatty(0);
  Tcl_SetVar(interp, "tcl_interactive", "0", TCL_GLOBAL_ONLY);

  if (Tcl_Init(interp) == TCL_ERROR)
     tclpanic(interp, "tcl_init() failed (perhaps TCL_LIBRARY not set?)");
  if (Tk_Init(interp) == TCL_ERROR)
     tclpanic(interp, "tk_init() failed (perhaps TK_LIBRARY not set?)");
//  if (Tix_Init(interp) == TCL_ERROR)
//     tclpanic(interp, "tix_init() failed (perhaps TIX_LIBRARY not set?");

  // copy command-line arguments into tcl vrbles argc / argv
  Tcl_Obj* argsObj = Tcl_NewListObj( 0, NULL );
  for( int i = 0; i < argc - 1; i++ )
  {
        int result = Tcl_ListObjAppendElement( interp,
                                    argsObj,
                                    Tcl_NewStringObj( argv[i+1], -1 ) );
        if( result != TCL_OK )
        {
            tclpanic( interp, "Failed to build argv list" );
        }
  }
  Tcl_ObjSetVar2(interp,
                    Tcl_NewStringObj( "argv", -1 ), NULL,
                    argsObj,
                    TCL_GLOBAL_ONLY);

  string argcStr = string(argc - 1);
  Tcl_ObjSetVar2(interp,
                    Tcl_NewStringObj( "argc", -1 ), NULL,
                    Tcl_NewStringObj( argcStr.string_of(), -1 ),
                    TCL_GLOBAL_ONLY);
  Tcl_ObjSetVar2(interp,
                    Tcl_NewStringObj( "argv0", -1 ), NULL,
                    Tcl_NewStringObj( argv[0], -1 ),
                    TCL_GLOBAL_ONLY);

  // Here is one tunable constant that is definitely intended to be hard-coded in:
   tunableBooleanConstantDeclarator tcInDeveloperMode("developerMode",
	 "Allow access to all tunable constants, including those limited to developer mode.  (Use with caution)",
	 false, // initial value
	 develModeCallback,
         userConstant);

    tunableConstantRegistry::createFloatTunableConstant
    ("EnableRequestPacketSize",
     "Enable request packet size",
     NULL,
     developerConstant,
     10.0, // initial value
     1.0,  // min
     100.0); // max

  //
  // We check our own read/write events.
  //
#if !defined(i386_unknown_nt4_0)
  P_signal(SIGPIPE, (P_sig_handler) SIG_IGN);
#endif // !defined(i386_unknown_nt4_0)

  // get paradyn_debug environment var PARADYNDEBUG, if its value
  // is > 1, then PARADYN_DEBUG msgs will be printed to stdout
  temp = (char *) getenv("PARADYNDEBUG");
  if (temp != NULL) {
    paradyn_debug = atoi(temp);
  }
  else {
    paradyn_debug = 0;
  }

// parse the command line arguments
  int a_ct=1;
  char *fname=0, *sname=0, *xname=0;
  while (argv[a_ct]) {
    if (fname == 0 && !strcmp(argv[a_ct], "-f") && argv[a_ct+1]) {
      fname = argv[++a_ct];
    } else if (sname == 0 && !strcmp(argv[a_ct], "-s") && argv[a_ct+1]) {
      sname = argv[++a_ct];
    } else if (xname == 0 && !strcmp(argv[a_ct], "-x") && argv[a_ct+1]) {
      xname = argv[++a_ct];
    } else if (!default_host.length() && (!strcmp(argv[a_ct], "-default_host") || !strcmp(argv[a_ct], "-d")) && argv[a_ct+1]) {
      default_host = argv[++a_ct];
    } else {
      printf("usage: %s [-f <pcl_filename>] [-s <tcl_scriptname>]"
                      " [-x <connect_filename>] [-default_host <hostname>]\n",
                 argv[0]);
      exit(-1);
    }
    a_ct++;
  }

#ifdef notdef // this isn't relevant here as default_host is defined later
              // when required (in paradynDaemon::getDaemonHelper)
  default_host = getNetworkName(default_host);
#endif

  const string localhost = getNetworkName();
  //cerr << "main: localhost=<" << localhost << ">" << endl;
  unsigned index=0;
  while (index<localhost.length() && localhost[index]!='.') index++;
  if (index == localhost.length())
      cerr << "Failed to determine local machine domain: localhost=<" 
           << localhost << ">" << endl;
  else
      local_domain = localhost.substr(index+1,localhost.length());
  //cerr << "main: local_domain=<" << local_domain << ">" << endl;

// get tid of parent
  MAINtid = thr_self();

#if defined(i386_unknown_nt4_0)
  // enable interaction between thread library and RPC package
  rpcSockCallback += (RPCSockCallbackFunc)thr_update_socket_data_state;
#endif // defined(i386_unknown_nt4_0)

// Structure used to pass initial arguments to data manager
//  init_struct init; init.tid = MAINtid; init.met_file = fname;

// call sequential initialization routines
  if(!dataManager::DM_sequential_init(fname)) {
    printf("Error found in Paradyn Configuration File, exiting\n");
    exit(-1);
  }
  VM::VM_sequential_init(); 


     /* initialize the 4 main threads of paradyn: data manager, visi manager,
        user interface manager, performance consultant */
  
// initialize DM

  if (thr_create(DMStack, sizeof(DMStack), DMmain, (void *) &MAINtid, 0, 
		 (unsigned int *) &DMtid) == THR_ERR)
    exit(1);
  PARADYN_DEBUG (("DM thread created\n"));

  msgsize = MBUFSIZE;
  mtid = THR_TID_UNSPEC;
  mtag = MSG_TAG_DM_READY;
  msg_recv(&mtid, &mtag, mbuf, &msgsize);
  assert( mtid == DMtid );
  msg_send (DMtid, MSG_TAG_ALL_CHILDREN_READY, (char *) NULL, 0);
  dataMgr = new dataManagerUser (DMtid);
  // context = dataMgr->createApplicationContext(eFunction);

// initialize UIM 
 
  if (thr_create (UIStack, sizeof(UIStack), &UImain, NULL,
		  0, &UIMtid) == THR_ERR) 
    exit(1);
  PARADYN_DEBUG (("UI thread created\n"));

  msgsize = MBUFSIZE;
  mtid = THR_TID_UNSPEC;
  mtag = MSG_TAG_UIM_READY;
  msg_recv(&mtid, &mtag, mbuf, &msgsize);
  assert( mtid == UIMtid );
  msg_send (UIMtid, MSG_TAG_ALL_CHILDREN_READY, (char *) NULL, 0);
  uiMgr = new UIMUser (UIMtid);

// initialize PC

  if (thr_create(0, 0, PCmain, (void*) &MAINtid, 0, 
		 (unsigned int *) &PCtid) == THR_ERR)
    exit(1);
  PARADYN_DEBUG (("PC thread created\n"));

  msgsize = MBUFSIZE;
  mtid = THR_TID_UNSPEC;
  mtag = MSG_TAG_PC_READY;
  msg_recv(&mtid, &mtag, mbuf, &msgsize);
  assert( mtid == PCtid );
  msg_send (PCtid, MSG_TAG_ALL_CHILDREN_READY, (char *) NULL, 0);
  perfConsult = new performanceConsultantUser (PCtid);

// initialize VM
  if (thr_create(0, 0, VMmain, (void *) &MAINtid, 0, 
		 (unsigned int *) &VMtid) == THR_ERR)
    exit(1);

  PARADYN_DEBUG (("VM thread created\n"));
  msgsize = MBUFSIZE;
  mtid = THR_TID_UNSPEC;
  mtag = MSG_TAG_VM_READY;
  msg_recv(&mtid, &mtag, mbuf, &msgsize);
  assert( mtid == VMtid );
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
 
  if (xname)
    dataMgr->printDaemonStartInfo (xname);

// wait for UIM thread to exit 

  thr_join (UIMtid, NULL, NULL);

  mpichUnlinkWrappers();

  Tcl_DeleteInterp(interp);

  return 0;
}
