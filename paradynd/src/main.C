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

// $Id: main.C,v 1.89 2000/07/28 17:22:11 pcroth Exp $

#include "common/h/headers.h"
#include "pdutil/h/makenan.h"
#include "common/h/Ident.h"

#if defined(MT_THREAD)
extern "C" const char V_paradyndMT[];
#else
extern "C" const char V_paradynd[];
#endif //MT_THREAD
extern "C" const char V_libpdutil[];

#if defined(MT_THREAD)
Ident V_id(V_paradyndMT,"Paradyn");
#else
Ident V_id(V_paradynd,"Paradyn");
#endif
Ident V_Uid(V_libpdutil,"Paradyn");

#include "rtinst/h/rtinst.h"

#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/util.h"
#include "dyninstAPI/src/dyninstP.h"
#include "paradynd/src/metric.h"
#include "paradynd/src/comm.h"
#include "paradynd/src/internalMetrics.h"
#include "common/h/machineType.h"
#include "paradynd/src/init.h"
#include "paradynd/src/perfStream.h"
#include "paradynd/src/mdld.h"

pdRPC *tp;

#ifdef PARADYND_PVM
#include "pvm_support.h"
extern "C" {
#include <pvm3.h>
}
#endif     


bool pvm_running = false;

static string machine_name;
string osName;
int pd_debug=0;

int ready;

#ifdef mips_sgi_irix6_4
extern bool execIrixMPIProcess(vector<string> &argv);
#endif

#ifdef DETACH_ON_THE_FLY
extern void initDetachOnTheFly();
#endif

/*
 * These variables are global so that we can easily find out what
 * machine/socket/etc we're connected to paradyn on; we may need to
 * start up other paradynds (such as on the CM5), and need this later.
 */
static string pd_machine;
static int pd_known_socket_portnum=0;
static int pd_flag=0;
static string pd_flavor;

void configStdIO(bool closeStdIn)
{
    int nullfd;

    /* now make stdin, out and error things that we can't hurt us */
    if ((nullfd = open("/dev/null", O_RDWR, 0)) < 0) {
	abort();
    }

    if (closeStdIn) (void) dup2(nullfd, 0);
    (void) dup2(nullfd, 1);
    (void) dup2(nullfd, 2);

    if (nullfd > 2) close(nullfd);
}

void sigtermHandler()
{
  showErrorCallback(98,"paradynd has been terminated");
}

// Cleanup for pvm and exit.
// This function must be called when we exit, to clean up and exit from pvm.
// Now also cleans up shm segs by deleting all processes  -ari
void cleanUpAndExit(int status) {
#ifdef PARADYND_PVM
  if (pvm_running)
    PDYN_exit_pvm();
#endif

#if !defined(i386_unknown_nt4_0)
  // delete the trace socket file
  unlink(traceSocketPath.string_of());
#endif

#ifdef SHM_SAMPLING_DEBUG
   cerr << "paradynd cleanUpAndExit: deleting all process structures now" << endl;
#endif

  // Fry all processes
  extern vector<process*> processVec;

  for (unsigned lcv=0; lcv < processVec.size(); lcv++) {
     process *theProc = processVec[lcv];
     if (theProc == NULL)
        continue; // process has already been cleaned up

#if defined(i386_unknown_linux2_0) || defined(alpha_dec_osf4_0)
     // Try to be a bit smarter when we clean up the processes - kill
     // all processes that were created, leave all processes that were
     // attached to running.  This should really go into the process class,
     // but I hesitate to do that this close to the release (3.0)
     // -nick (24-Mar-2000)
     int pid = theProc->getPid();
     bool wasAttachedTo = theProc->wasCreatedViaAttach();
#endif
     delete theProc; // calls process::~process, which fries the shm seg
#if defined(i386_unknown_linux2_0) || defined(alpha_dec_osf4_0)
     if (!wasAttachedTo) OS::osKill(pid);
#endif

     processVec[lcv] = NULL; // probably not needed here.
  }

  P_exit(status);
}

// TODO
// mdc - I need to clean this up
bool
RPC_undo_arg_list (string &flavor, unsigned argc, char **arg_list, 
		   string &machine, int &well_known_socket, int &flag)
{
  char *ptr;
  bool b_well_known=false; // found well-known socket port num
  bool b_machine = false, b_flag = false, b_flavor=false;

  for (unsigned loop=0; loop < argc; ++loop) {
      // stop at the -runme argument since the rest are for the application
      //   process we are about to spawn
      if (!P_strcmp(arg_list[loop], "-runme")) break;
      if (!P_strncmp(arg_list[loop], "-p", 2)) {
	  well_known_socket = P_strtol (arg_list[loop] + 2, &ptr, 10);
	  if (ptr == (arg_list[loop] + 2))
	    return(false);
	  b_well_known = true;
      }
      else if (!P_strncmp(arg_list[loop], "-V", 2)) { // optional
          cout << V_id << endl;
      }
      else if (!P_strncmp(arg_list[loop], "-v", 2)) {
          pd_debug++;
          //cerr << "paradynd: -v flag is obsolete (and ignored)" << endl;
      }
      else if (!P_strncmp(arg_list[loop], "-L", 2)) {
          // this is an optional specification of the runtime library,
          // overriding PARADYN_LIB (primarily for debugging/testing purposes)
	  process::dyninstName = (arg_list[loop] + 2);
	  if (!process::dyninstName.length()) return false;
      }
      else if (!P_strncmp(arg_list[loop], "-m", 2)) {
	  machine = (arg_list[loop] + 2);
	  if (!machine.length()) return false;
	  b_machine = true;
      }
      else if (!P_strncmp(arg_list[loop], "-l", 2)) {
	  flag = P_strtol (arg_list[loop] + 2, &ptr, 10);
	  if (ptr == (arg_list[loop] + 2))
	    return(false);
	  b_flag = true;
      }
      else if (!P_strncmp(arg_list[loop], "-z", 2)) {
	  flavor = (arg_list[loop]+2);
	  if (!flavor.length()) return false;
	  b_flavor = true;
      }
  }

  // verify required parameters
  return (b_flag && b_machine && b_well_known && b_flavor);
}

// PARADYND_DEBUG_XXX
static void initialize_debug_flag(void) {
  char *p;

  if ( (p=getenv("PARADYND_DEBUG_INFRPC")) ) {
    pd_debug_infrpc = 1;
  }

  if ( (p=getenv("PARADYND_DEBUG_CATCHUP")) ) {
    pd_debug_catchup = 1;
  }
}

int main(unsigned argc, char *argv[]) {

  initialize_debug_flag();

  string *dir = new string("");
#if !defined(i386_unknown_nt4_0)
    {
        char *pdkill;
        pdkill = getenv( "PARADYND_DEBUG" );
        if( pdkill && ( *pdkill == 'y' || *pdkill == 'Y' ) ) {
            int pid = getpid();
            cerr << "breaking for debug in controllerMainLoop...pid=" << pid << endl;
#if defined(i386_unknown_nt4_0)
            DebugBreak();
#else
            kill(pid, SIGSTOP);
#endif
        }
    }
#endif // !defined(i386_unknown_nt4_0)

#if !defined(i386_unknown_nt4_0)
    struct sigaction act;

    // int i = 1;
    // while (i) {};


#if defined(sparc_sun_sunos4_1_3) || defined(sparc_sun_solaris2_4)
    act.sa_handler = (void (*)(...)) sigtermHandler;
#else
    act.sa_handler = (void (*)(int)) sigtermHandler;
#endif
    act.sa_flags   = 0;

    /* for AIX - default (non BSD) library does not restart - jkh 7/26/95 */
#if defined(SA_RESTART)
    act.sa_flags  |= SA_RESTART;
#endif

    sigfillset(&act.sa_mask);

    if (sigaction(SIGTERM, &act, 0) == -1) {
        perror("sigaction(SIGTERM)");
        abort();
    }
#endif

#ifdef DETACH_ON_THE_FLY
    initDetachOnTheFly();
#endif

#if defined(i386_unknown_nt4_0)
    // Windows NT needs to initialize winsock library
    WORD wsversion = MAKEWORD(2,0);
    WSADATA wsadata;
    WSAStartup(wsversion, &wsadata);
#endif

    process::programName = argv[0];
    // process command line args passed in
    // pd_flag == 1 --> started by paradyn
    bool aflag;
    aflag = RPC_undo_arg_list (pd_flavor, argc, argv, pd_machine,
			       pd_known_socket_portnum, pd_flag);
    if (!aflag || pd_debug) {
        if (!aflag) cerr << "Invalid/incomplete command-line args:" << endl;
        cerr << "   -z<flavor";
        if (pd_flavor.length()) cerr << "=" << pd_flavor;
        cerr << "> -l<flag";
        if (pd_flag) cerr << "=" << pd_flag;
        cerr << "> -m<hostmachine";
        if (pd_machine.length()) cerr << "=" << pd_machine;
        cerr << "> -p<hostport";
        if (pd_known_socket_portnum) cerr << "=" << pd_known_socket_portnum;
        cerr << ">" << endl;
        if (process::dyninstName.length())
            cerr << "   -L<library=" << process::dyninstName << ">" << endl;
        if (!aflag) cleanUpAndExit(-1);
    }

    aflag = RPC_make_arg_list(process::arg_list,
			      pd_known_socket_portnum, pd_flag, 0,
			      pd_machine, true);
    assert(aflag);
    string flav_arg(string("-z")+ pd_flavor);
    process::arg_list += flav_arg;
    machine_name = getNetworkName();

    // kill(getpid(),SIGSTOP);

    //
    // See if we should fork an app process now.
    //

    // We want to find two things
    // First, get the current working dir (PWD)
    dir = new string(getenv("PWD"));

    // Second, put the inferior application and its command line
    // arguments into cmdLine. Basically, loop through argv until
    // we find -runme, and put everything after it into cmdLine.
    vector<string> cmdLine;
    unsigned int argNum = 0;
    while ((argNum < argc) && (strcmp(argv[argNum], "-runme")))
      argNum++;
    // Okay, argNum is the command line argument which is "-runme"
    argNum++;
    // Copy everything from argNum to < argc
    for (unsigned int i = argNum; i < argc; i++)
      cmdLine += argv[i];

#ifdef PARADYND_PVM
    // There are 3 ways to get here
    //     started by pvm_spawn from first paradynd -- must report back
    //     started by rsh, rexec, ugly code --> connect via socket
    //     started by exec --> use pipe
    
    // int pvm_id = pvm_mytid();
    int pvmParent = PvmSysErr;

#ifdef PDYN_DEBUG
    cerr << "pd_flavor: " << pd_flavor.string_of() << endl;
#endif
    process::pdFlavor = pd_flavor;
    if (pd_flavor == string("pvm")) {
       pvmParent = pvm_parent();

       if (pvmParent == PvmSysErr) {
	  fprintf(stdout, "Unable to connect to PVM daemon, is PVM running?\n");
 	  fflush(stdout);
  	  cleanUpAndExit(-1);
	}
       else
	  pvm_running = true;
    }

    if (pvm_running && pvmParent != PvmNoParent) {
      // started by pvm_spawn
      // TODO -- report error here
      if (!PDYN_initForPVM (argv, pd_machine, pd_known_socket_portnum, 0)) {
	cleanUpAndExit(-1);
      }
      tp = new pdRPC(AF_INET, pd_known_socket_portnum, SOCK_STREAM, pd_machine,
		     NULL, NULL, 2);
      assert(tp);

      tp->reportSelf (machine_name, argv[0], getpid(), "pvm");
    } else if ( pd_flavor == "mpi" ) {
      // Both IRIX and AIX MPI job-launchers will start paradynd,
      // which must report to paradyn
      // the pdRPC is allocated and reportSelf is called
      tp = new pdRPC(AF_INET, pd_known_socket_portnum, SOCK_STREAM, 
		     pd_machine, NULL, NULL, 2);
      assert(tp);

      tp->reportSelf(machine_name, argv[0], getpid(), "mpi");
    } else if (pd_flag == 2) {
       // manual startup
	tp = new pdRPC(AF_INET, pd_known_socket_portnum, SOCK_STREAM, pd_machine, 
		       NULL, NULL, 2);
	assert(tp);
        //assert(tp->net_obj()); // shouldn't this be part of pdRPC::pdRPC?
        if (!tp->net_obj()) {
            cerr << "Failed to establish connection to Paradyn on "
                 << pd_machine << " port " << pd_known_socket_portnum << endl;
	    cleanUpAndExit(-1);
        }
	tp->reportSelf(machine_name, argv[0], getpid(), pd_flavor);

	if (pvm_running
	    && !PDYN_initForPVM (argv, pd_machine, pd_known_socket_portnum, 1))
	    cleanUpAndExit(-1);

    } else if (!pd_flag) {
      // not started by pvm_spawn; rather, started via rsh/rexec --> use socket
      int pid = fork();
      if (pid == 0) {
	// configStdIO(true);
	// setup socket
	// TODO -- report error here
	
	// We must get a connection with paradyn before starting any other daemons,
	// or else one of the daemons we start (in PDYN_initForPVM), may get our
	// connection.
	tp = new pdRPC(AF_INET, pd_known_socket_portnum, SOCK_STREAM, pd_machine,
		       NULL, NULL, 2);
	assert(tp);

	if (pvm_running && !PDYN_initForPVM (argv, pd_machine, pd_known_socket_portnum, 1)) {
	    cleanUpAndExit(-1);
	}

      } else if (pid > 0) {
	P__exit(-1);
      } else {
	cerr << "Fatal error on paradyn daemon: fork failed." << endl;
	cerr.flush();
	cleanUpAndExit(-1);
      }
    } else {
       // started via exec   --> use pipe
       // TODO -- report error here
      if (pvm_running && !PDYN_initForPVM (argv, pd_machine, pd_known_socket_portnum, 1)) {
	  cleanUpAndExit(-1);
      }
      // already setup on this FD.
      // disconnect from controlling terminal 
      OS::osDisconnect();
      tp = new pdRPC(0, NULL, NULL, 2);
      assert(tp);
    }
    assert(tp);
#else

    if ( pd_flavor == "mpi" ) {
      // Both IRIX and AIX MPI job-launchers will start paradynd,
      // which must report to paradyn
      // the pdRPC is allocated and reportSelf is called
      tp = new pdRPC(AF_INET, pd_known_socket_portnum, SOCK_STREAM, 
		     pd_machine, NULL, NULL, 2);
      assert(tp);

      tp->reportSelf(machine_name, argv[0], getpid(), "mpi");
    } else if (pd_flag == 2) {
       // manual startup
	tp = new pdRPC(AF_INET, pd_known_socket_portnum, SOCK_STREAM, pd_machine, 
		       NULL, NULL, 2);
	assert(tp);
	tp->reportSelf(machine_name, argv[0], getpid(), pd_flavor);

    } else if (!pd_flag) {
#if !defined(i386_unknown_nt4_0)
      int pid = fork();
#else
      int pid = 0;
#endif
      if (pid == 0) {
	// configStdIO(true);
	// setup socket

	tp = new pdRPC(AF_INET, pd_known_socket_portnum, SOCK_STREAM, pd_machine, 
		       NULL, NULL, 2);
	assert(tp);

	if (cmdLine.size()) {
	    tp->reportSelf(machine_name, argv[0], getpid(), pd_flavor);
	}
    } else if (!pd_flag) {
      } else if (pid > 0) {
	P__exit(-1);
      } else {
	cerr << "Fatal error on paradyn daemon: fork failed." << endl;
	cerr.flush();
	cleanUpAndExit(-1);
      }
    } else {
      OS::osDisconnect();

#if !defined(i386_unknown_nt4_0)
		PDSOCKET sock = 0;
#else
		PDSOCKET sock = _get_osfhandle(0);
#endif // defined(i386_unknown_nt4_0)
      tp = new pdRPC(sock, NULL, NULL, 2);
      assert(tp);

      // configStdIO(false);
    }
#endif

#if defined(MT_THREAD)
    statusLine(V_paradyndMT);
#else
    statusLine(V_paradynd);
#endif

    extern unsigned getCyclesPerSecond();
    cyclesPerSecond = (double)getCyclesPerSecond();

    // Note -- it is important that this daemon receives all mdl info
    // before starting a process
    aflag = mdl_get_initial(pd_flavor, tp);
    assert(aflag);

    initLibraryFunctions();
    if (!init()) 
      abort();

#ifdef mips_sgi_irix6_4
    struct utsname unameInfo;
    if ( P_uname(&unameInfo) == -1 )
    {
        perror("uname");
        return false;
    }

    // osName is used in irix.C and process.C
    osName = unameInfo.sysname;

    if ( pd_flavor == "mpi" && osName.prefixed_by("IRIX") )
    {
      if ( !execIrixMPIProcess(cmdLine) )
        return(0);
    }
    else
#endif
        if (cmdLine.size()) {
            vector<string> envp;
            addProcess(cmdLine, envp, *dir); // ignore return val (is this right?)
        }

    controllerMainLoop(true);
    return(0);
}

