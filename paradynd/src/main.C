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
 * Revision 1.67  1997/11/26 21:50:18  mcheyney
 * Extra debugging information in main.C
 * Changed exclude syntax in mdl:
 * Old:
 *   exclude "module";  or
 *   exclude "module/function";
 *
 * New:
 *   exclude "/Code/module"; or
 *   exclude "/Code/module/function";
 *
 * Also added some better error messages for identifying cases where
 *  underlying process falls over.
 *
 * Revision 1.66  1997/06/06 22:01:38  mjrg
 * added option for manual daemon start-up
 *
 * Revision 1.65  1997/05/23 23:02:17  mjrg
 * Windows NT port
 *
 * Revision 1.64  1997/05/17 19:58:51  lzheng
 * Changes made for nonblocking write
 *
 * Revision 1.63  1997/05/13 14:26:04  sec
 * Removed a simple assert
 *
 * Revision 1.62  1997/05/07 19:01:15  naim
 * Getting rid of old support for threads and turning it off until the new
 * version is finished. Additionally, new superTable, baseTable and superVector
 * classes for future support of multiple threads. The fastInferiorHeap class has
 * also changed - naim
 *
 * Revision 1.61  1997/03/29 02:08:53  sec
 * Changed the daemon poe to mpi
 *
 * Revision 1.60  1997/03/23 16:57:48  zhichen
 * added code to set process:pdFlavor
 *
 * Revision 1.59  1997/03/14 18:50:57  zhichen
 * Added reportSelf in the case when the daemons were started by
 * COW DJM. Search for 'Tempest' for the change
 *
 * Revision 1.58  1997/02/26 23:46:35  mjrg
 * First part of WindowsNT port: changes for compiling with Visual C++;
 * moved unix specific code to unix.C file
 *
 * Revision 1.57  1997/02/21 20:15:50  naim
 * Moving files from paradynd to dyninstAPI + eliminating references to
 * dataReqNode from the ast class. This is the first pre-dyninstAPI commit! - naim
 *
 * Revision 1.56  1997/02/18 21:25:15  sec
 * Added poe support
 *
 * Revision 1.55  1997/01/21 20:07:47  mjrg
 * Changed to unix domain socket for trace stream
 * Replaced calls to uname by calls to libutil function getHostName
 *
 * Revision 1.54  1997/01/16 22:07:15  tamches
 * moved RPC_undo_arg_list here from util lib
 *
 * Revision 1.53  1997/01/15 00:28:09  tamches
 * added some debug msgs
 *
 * Revision 1.52  1996/12/16 23:10:16  mjrg
 * bug fixes to fork/exec on all platforms, partial fix to fork on AIX
 *
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
#include "util/h/machineType.h"
#include "paradynd/src/init.h"
#include "paradynd/src/perfStream.h"
#include "dyninstAPI/src/clock.h"
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

  // delete the trace socket file
  unlink(traceSocketPath.string_of());

#ifdef SHM_SAMPLING_DEBUG
   cerr << "paradynd cleanUpAndExit: deleting all process structures now" << endl;
#endif

  // Fry all processes
  extern vector<process*> processVec;

  for (unsigned lcv=0; lcv < processVec.size(); lcv++) {
     process *theProc = processVec[lcv];
     if (theProc == NULL)
        continue; // process has already been cleaned up

     delete theProc; // calls process::~process, which fries the shm seg

     processVec[lcv] = NULL; // probably not needed here.
  }

  P_exit(status);
}

// TODO
// mdc - I need to clean this up
bool
RPC_undo_arg_list (string& flavor, int argc, char **arg_list, string &machine,
		   int &well_known_socket, int &flag,
		   int &firstPVM)
{
  char *ptr;
  bool b_well_known=false; // found well-known socket port num
  bool b_first=false;
  bool b_machine = false, b_flag = false, b_flavor=false;

  for (int loop=0; loop < argc; ++loop) {
      // stop at the -runme argument since the rest are for the application
      //   process we are about to spawn
      if (!strcmp(arg_list[loop], "-runme")) break;
      if (!P_strncmp(arg_list[loop], "-p", 2)) {
	  well_known_socket = P_strtol (arg_list[loop] + 2, &ptr, 10);
	  if (ptr == (arg_list[loop] + 2))
	    return(false);
	  b_well_known = true;
      }
      else if (!P_strncmp(arg_list[loop], "-v", 2)) {
	  firstPVM = P_strtol (arg_list[loop] + 2, &ptr, 10);
	  if (ptr == (arg_list[loop] + 2))
	    return(false);
	  b_first = true;
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

  return (b_flag && b_first && b_machine && b_well_known && b_flavor);
}

int main(int argc, char *argv[]) {
    cerr << "welcome to paradynd, args are:" << endl;
    for (unsigned lcv=0; lcv < argc; lcv++) {
       cerr << argv[lcv] << endl;
    }

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

#if defined(i386_unknown_nt4_0)
    // Windows NT needs to initialize winsock library
    WORD wsversion = MAKEWORD(2,0);
    WSADATA wsadata;
    WSAStartup(wsversion, &wsadata);
#endif

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
    machine_name = getHostName();

    // kill(getpid(),SIGSTOP);

    //
    // See if we should fork an app process now.
    //
    vector<string> cmdLine;
    for (int i=0; argv[i]; i++) {
	if (!strcmp(argv[i], "-runme")) {
	     // next arg is the command to run.
	     int j;
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

    cerr << "pd_flavor: " << pd_flavor.string_of() << endl ;
    process::pdFlavor = pd_flavor ;
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
    } else if(pd_flavor == string("mpi")) {
      // the executables which are started by poe (for mpi daemon) must report to paradyn
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
	//Tempest, in the case of blizzard_cow, all daemons should report themselves
    	if (pd_flavor == string("cow")) {
		cerr << "rsh, rexec, reportSelf " 
		     << machine_name.string_of() 
		     << argv[0] << endl ;
		tp->reportSelf (machine_name, argv[0], getpid(), "cow");
	}
      

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

    if(pd_flavor == string("mpi")) {
      // not put here, only up above since PARADYND_PVM is always set
      assert(0);
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
      tp = new pdRPC(0, NULL, NULL, 2);
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
         //logLine("paradynd: cmdLine is non-empty so we'll be calling addProcess now!\n") ; 
	 //cerr << "cmdLine is:" << endl;
	 //for (unsigned lcv=0; lcv < cmdLine.size(); lcv++)
	 //   cerr << cmdLine[lcv] << endl;

         vector<string> envp;
	 addProcess(cmdLine, envp, string("")); // ignore return val (is this right?)
    }

    controllerMainLoop(true);
}
