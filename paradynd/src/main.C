/*
 * Copyright (c) 1996 Barton P. Miller
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

/*
 * Main loop for the default paradynd.
 *
 * $Log: main.C,v $
 * Revision 1.51  1996/12/06 09:37:55  tamches
 * cleanUpAndExit() moved here (from .h file); it now also
 * calls destructors for all processes, which should clean up
 * process state like shm segments.
 *
 * Revision 1.50  1996/11/29 19:41:08  newhall
 * Cleaned up some code.  Moved code that was duplicated in inst-sparc-solaris.C
 * and inst-sparc-sunos.C to inst-sparc.C.  Bug fix to process::findFunctionIn.
 *
 * Revision 1.49  1996/11/26 16:08:09  naim
 * Fixing asserts - naim
 *
 * Revision 1.48  1996/09/26 18:58:44  newhall
 * added support for instrumenting dynamic executables on sparc-solaris
 * platform
 *
 * Revision 1.47  1996/08/16 21:19:13  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.46  1996/08/12 16:27:08  mjrg
 * Code cleanup: removed cm5 kludges and some unused code
 *
 * Revision 1.45  1996/07/18 19:39:14  naim
 * Minor fix to give proper error message when the pvm daemon runs out of
 * virtual memory - naim
 *
 * Revision 1.44  1996/05/31  23:57:48  tamches
 * code to change send socket buffer size moved to comm.h
 * removed handshaking code w/paradyn (where we sent "PARADYND" plus
 * the pid) [wasn't being used and contributed to freeze in paradyn UI]
 *
 * Revision 1.43  1996/05/09 21:27:42  newhall
 * increased the socket send buffer size from 4K to 32K on sunos and hpux
 *
 * Revision 1.42  1996/05/08  23:54:50  mjrg
 * added support for handling fork and exec by an application
 * use /proc instead of ptrace on solaris
 * removed warnings
 *
 * Revision 1.41  1996/02/09 22:13:43  mjrg
 * metric inheritance now works in all cases
 * paradynd now always reports to paradyn when a process is ready to run
 * fixed aggregation to handle first samples and addition of new components
 *
 * Revision 1.40  1996/01/29 22:09:23  mjrg
 * Added metric propagation when new processes start
 * Adjust time to account for clock differences between machines
 * Daemons don't enable internal metrics when they are not running any processes
 * Changed CM5 start (paradynd doesn't stop application at first breakpoint;
 * the application stops only after it starts the CM5 daemon)
 *
 */

#include "util/h/headers.h"
#include "util/h/makenan.h"

#include "rtinst/h/rtinst.h"

#include "symtab.h"
#include "process.h"
#include "inst.h"
#include "instP.h"
#include "ast.h"
#include "util.h"
#include "dyninstP.h"
#include "metric.h"
#include "comm.h"
#include "internalMetrics.h"
#include "util/h/machineType.h"
#include "init.h"
#include "perfStream.h"
#include "clock.h"
#include "paradynd/src/mdld.h"
#include <sys/signal.h>

pdRPC *tp;

#ifdef PARADYND_PVM
#include "pvm_support.h"
extern "C" {
#include "pvm3.h"
}
#endif     


int traceSocket;
int traceSocket_fd;

bool pvm_running = false;

static char machine_name[80];

int ready;

/*
 * These variables are global so that we can easily find out what
 * machine/socket/etc we're connected to paradyn on; we may need to
 * start up other paradynds (such as on the CM5), and need this later.
 */
static string pd_machine;
static int pd_known_socket_portnum;
static int pd_flag;
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

void sigtermHandler(int ) {
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

  // Fry all processes
  for (unsigned lcv=0; lcv < processVec.size(); lcv++) {
     process *theProc = processVec[lcv];
     if (theProc == NULL)
        continue; // process has already been cleaned up

     delete theProc; // calls process::~process, which fries the shm seg

     processVec[lcv] = NULL; // probably not needed here.
  }

  P_exit(status);
}

int main(int argc, char *argv[])
{
    int i;
    vector<string> cmdLine;
    vector<string> envp;
    struct sigaction act;

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

    // for debugging
    // { int i= 1; while (i); }

//    {volatile int i = 1; while (i);}

    process::programName = argv[0];

    // process command line args passed in
    // pd_flag == 1 --> started by paradyn
    int pvm_first;
    bool aflag;
    aflag = RPC_undo_arg_list (pd_flavor, argc, argv, pd_machine,
			       pd_known_socket_portnum, pd_flag, 
			       pvm_first);
    assert(aflag);
    aflag = RPC_make_arg_list(process::arg_list,
			      pd_known_socket_portnum, pd_flag, 0,
			      pd_machine, true);
    assert(aflag);
    string flav_arg(string("-z")+ pd_flavor);
    process::arg_list += flav_arg;
    struct utsname un;
    P_uname(&un);
    P_strcpy(machine_name, un.nodename);

    // kill(getpid(),SIGSTOP);

    //
    // See if we should fork an app process now.
    //
    for (i=0; argv[i]; i++) {
	int j;
	if (!strcmp(argv[i], "-runme")) {
	     // next arg is the command to run.
	     for (j=0,i++; argv[i]; i++,j++) {
		 cmdLine += argv[i];
	     }
	}
    }

#ifdef PARADYND_PVM
    // There are 3 ways to get here
    //     started by pvm_spawn from first paradynd -- must report back
    //     started by rsh, rexec, ugly code --> connect via socket
    //     started by exec --> use pipe
    
    // int pvm_id = pvm_mytid();
    int pvmParent = PvmSysErr;

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

      tp = new pdRPC(AF_INET, pd_known_socket_portnum, SOCK_STREAM, pd_machine, NULL, NULL, 0);
      assert(tp);

      tp->reportSelf (machine_name, argv[0], getpid(), "pvm");
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
	tp = new pdRPC(AF_INET, pd_known_socket_portnum, SOCK_STREAM, pd_machine, NULL, NULL, 0);
	assert(tp);

	if (pvm_running && !PDYN_initForPVM (argv, pd_machine, pd_known_socket_portnum, 1)) {
	    cleanUpAndExit(-1);
	}

      } else if (pid > 0) {
//	// Handshaking with handleRemoteConnect() of paradyn [rpcUtil.C]
//	sprintf(errorLine, "PARADYND %d\n", pid);
//	//logLine(errorLine); <<--- WON'T WORK SINCE SOCKETS NOT YET SET UP!!
//	fprintf(stdout, errorLine); // this works just fine...no need for logLine()
//	fflush(stdout);
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
      tp = new pdRPC(0, NULL, NULL);
      assert(tp);
    }
    assert(tp);
#else

    if (!pd_flag) {
      int pid = fork();
      if (pid == 0) {
	// configStdIO(true);
	// setup socket

	tp = new pdRPC(AF_INET, pd_known_socket_portnum, SOCK_STREAM, pd_machine, 
		       NULL, NULL, false);
	assert(tp);

	if (cmdLine.size()) {
	    tp->reportSelf(machine_name, argv[0], getpid(), metPVM);
	}
      } else if (pid > 0) {
//	// Handshaking with handleRemoteConnect() of paradyn [rpcUtil.C]
//	sprintf(errorLine, "PARADYND %d\n", pid);
//	// logLine(errorLine); <<--- WON'T WORK SINCE SOCKETS NOT YET SET UP!!
//	fprintf(stdout, errorLine); // this works just fine...no need for logLine()
//	fflush(stdout);
	P__exit(-1);
      } else {
	cerr << "Fatal error on paradyn daemon: fork failed." << endl;
	cerr.flush();
	cleanUpAndExit(-1);
      }
    } else {
      OS::osDisconnect();
      tp = new pdRPC(0, NULL, NULL);
      assert(tp);

      // configStdIO(false);
    }
#endif

    cyclesPerSecond = timing_loop() * 1000000;

    // Note -- it is important that this daemon receives all mdl info
    // before starting a process
    aflag = mdl_get_initial(pd_flavor, tp);
    assert(aflag);

    initLibraryFunctions();
    if (!init())
      abort();

    if (cmdLine.size()) {
	 addProcess(cmdLine, envp, string(""));
    }


    /* set up a socket to be used to create a trace link
       by inferior processes that are not forked 
       directly by this daemon.
    */
    traceSocket = RPC_setup_socket(traceSocket_fd, PF_INET, SOCK_STREAM);
    if (traceSocket < 0) {
      perror("paradynd -- cannot create socket");
      cleanUpAndExit(-1);
    }

    controllerMainLoop(true);
}

#ifdef notdef
#ifdef PARADYND_PVM

bool
PDYND_report_to_paradyn (int pid, int argc, char **argv)
{
    assert(tp);
    vector <string> as;
    for (int i=0; i<argc; i++) 
      as += argv[i];

    tp->newProgramCallbackFunc(pid, as, machine_name);
    return true;
}
#endif
#endif
