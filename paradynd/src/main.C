/*
 * Main loop for the default paradynd.
 *
 * $Log: main.C,v $
 * Revision 1.33  1995/05/18 10:38:17  markc
 * deleted metric callbacks -- these are now requested from the daemon
 * initialize mdl data before parsing an image
 *
 * Revision 1.32  1995/02/26  22:46:19  markc
 * Fixed for pvm version.  The pvm ifdefs are still ugly, but they compile.
 *
 * Revision 1.31  1995/02/16  08:53:38  markc
 * Corrected error in comments -- I put a "star slash" in the comment.
 *
 * Revision 1.30  1995/02/16  08:33:38  markc
 * Changed igen interfaces to use strings/vectors rather than char igen-arrays
 * Changed igen interfaces to use bool, not Boolean.
 * Cleaned up symbol table parsing - favor properly labeled symbol table objects
 * Updated binary search for modules
 * Moved machine dependnent ptrace code to architecture specific files.
 * Moved machine dependent code out of class process.
 * Removed almost all compiler warnings.
 * Use "posix" like library to remove compiler warnings
 *
 * Revision 1.29  1994/11/11  07:04:25  markc
 * Fixed the code to allow paradyndPVM to be started via rsh/rexec.  This had been
 * ignored in the past and paradyndPVM would block on rsh starts.
 *
 * Revision 1.28  1994/11/10  22:22:58  markc
 * "Ported" remote execution to pvm.  It was only working for the non-pvm case.
 * Made all cases of remote execution call report_self.
 *
 * Revision 1.27  1994/11/06  09:58:20  newhall
 * fix to support remote paradynd start, replaced logLine with fprintf
 * to stdout (this is the handshaking signal sent to paradyn).  logLine
 * requires an initialized "tp" variable, but it was NULL.
 *
 * Revision 1.26  1994/11/02  11:10:22  markc
 * Removed compiler warnings.
 *
 * Revision 1.25  1994/10/13  07:24:49  krisna
 * solaris porting and updates
 *
 * Revision 1.24  1994/09/22  02:10:45  markc
 * access metricList using method
 *
 * Revision 1.23  1994/09/20  18:18:26  hollings
 * added code to use actual clock speed for cost model numbers.
 *
 * Revision 1.22  1994/08/17  18:14:03  markc
 * Added extra parameter to reportSelf call.
 *
 * Revision 1.21  1994/07/14  23:30:28  hollings
 * Hybrid cost model added.
 *
 * Revision 1.20  1994/07/14  14:29:20  jcargill
 * Renamed connection-related variables (family, port, ...), and moved all the
 * standard dynRPC functions to another file.
 *
 * Revision 1.19  1994/07/05  03:26:08  hollings
 * observed cost model
 *
 * Revision 1.18  1994/06/29  02:52:34  hollings
 * Added metricDefs-common.{C,h}
 * Added module level performance data
 * cleanedup types of inferrior addresses instrumentation defintions
 * added firewalls for large branch displacements due to text+data over 2meg.
 * assorted bug fixes.
 *
 * Revision 1.17  1994/06/27  21:28:11  rbi
 * Abstraction-specific resources and mapping info
 *
 * Revision 1.16  1994/06/27  18:56:53  hollings
 * removed printfs.  Now use logLine so it works in the remote case.
 * added internalMetric class.
 * added extra paramter to metric info for aggregation.
 *
 * Revision 1.15  1994/06/22  03:46:31  markc
 * Removed compiler warnings.
 *
 * Revision 1.14  1994/06/02  23:27:56  markc
 * Replaced references to igen generated class to a new class derived from
 * this class to implement error handling for igen code.
 *
 * Revision 1.13  1994/05/18  00:52:28  hollings
 * added ability to gather IO from application processes and forward it to
 * the paradyn proces.
 *
 * Revision 1.12  1994/05/16  22:31:50  hollings
 * added way to request unique resource name.
 *
 * Revision 1.11  1994/04/12  15:29:19  hollings
 * Added samplingRate as a global set by an RPC call to control sampling
 * rates.
 *
 * Revision 1.10  1994/04/09  18:34:54  hollings
 * Changed {pause,continue}Application to {pause,continue}AllProceses, and
 * made the RPC interfaces use these.  This makes the computation of pause
 * Time correct.
 *
 * Revision 1.9  1994/04/06  21:35:39  markc
 * Added correct machine name reporting.
 *
 * Revision 1.8  1994/04/01  20:06:41  hollings
 * Added ability to start remote paradynd's
 *
 * Revision 1.7  1994/03/31  01:57:27  markc
 * Added support for pauseProcess, continueProcess.  Added pvm interface code.
 *
 * Revision 1.6  1994/03/20  01:53:09  markc
 * Added a buffer to each process structure to allow for multiple writers on the
 * traceStream.  Replaced old inst-pvm.C.  Changed addProcess to return type
 * int.
 *
 * Revision 1.5  1994/02/28  05:09:42  markc
 * Added pvm hooks and ifdefs.
 *
 * Revision 1.4  1994/02/25  13:40:55  markc
 * Added hooks for pvm support.
 *
 * Revision 1.3  1994/02/24  04:32:33  markc
 * Changed header files to reflect igen changes.  main.C does not look at the number of command line arguments now.
 *
 * Revision 1.2  1994/02/01  18:46:52  hollings
 * Changes for adding perfConsult thread.
 *
 * Revision 1.1  1994/01/27  20:31:27  hollings
 * Iinital version of paradynd speaking dynRPC igend protocol.
 *
 *
 */

#include "util/h/headers.h"

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

pdRPC *tp;

#ifdef PARADYND_PVM
#include "paradyndPVM/h/pvm_support.h"
extern "C" {
#include "pvm3.h"
}
#endif     

static char machine_name[80];

int ready;

/*
 * These variables are global so that we can easily find out what
 * machine/socket/etc we're connected to paradyn on; we may need to
 * start up other paradynds (such as on the CM5), and need this later.
 */
static string pd_machine;
static int pd_family;
static int pd_type;
static int pd_known_socket;
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


int main(int argc, char *argv[])
{
    process::programName = argv[0];

    // process command line args passed in
    // pd_flag == 1 --> started by paradyn
    int pvm_first;
    assert (RPC_undo_arg_list (pd_flavor, argc, argv, pd_machine, pd_family, pd_type,
			       pd_known_socket, pd_flag, pvm_first));
    assert (RPC_make_arg_list(process::arg_list, pd_family, pd_type,
			      pd_known_socket, pd_flag, 0,
			      pd_machine, true));
    string flav_arg(string("-z")+ pd_flavor);
    process::arg_list += flav_arg;
    struct utsname un;
    P_uname(&un);
    P_strcpy(machine_name, un.nodename);
 
#ifdef PARADYND_PVM
    // There are 3 ways to get here
    //     started by pvm_spawn from first paradyndPVM, must report back
    //     started by rsh, rexec, ugly code --> connect via socket
    //     started by exec --> use pipe
    
    // int pvm_id = pvm_mytid();

    if (pvm_parent() != PvmNoParent) {
      // started by pvm_spawn
      // TODO -- report error here
      if (!PDYN_initForPVM (argv, pd_machine, pd_family, pd_type, pd_known_socket, 0))
	exit(0);
      tp = new pdRPC(pd_family, pd_known_socket, pd_type, pd_machine, NULL, NULL, 0);
      tp->reportSelf (machine_name, argv[0], getpid(), "pvm");
    } else if (!pd_flag) {
      // started via rsh/rexec --> use socket
      int pid;
      pid = fork();
      if (pid == 0) {
	// configStdIO(true);
	// setup socket
	// TODO -- report error here
	if (!PDYN_initForPVM (argv, pd_machine, pd_family, pd_type, pd_known_socket, 1))
	  exit(0);
	tp = new pdRPC(pd_family, pd_known_socket, pd_type, pd_machine, NULL, NULL, 0);
      } else if (pid > 0) {
	// Handshaking with handleRemoteConnect() of paradyn [rpcUtil.C]
	sprintf(errorLine, "PARADYND %d\n", pid);
	//logLine(errorLine); <<--- WON'T WORK SINCE SOCKETS NOT YET SET UP!!
	fprintf(stdout, errorLine); // this works just fine...no need for logLine()
	fflush(stdout);
	_exit(-1);
      } else {
	fflush(stdout);
	exit(-1);
      }
     } else {
       // started via exec   --> use pipe
       // TODO -- report error here
      if (!PDYN_initForPVM (argv, pd_machine, pd_family, pd_type, pd_known_socket, 1))
	exit(0);
      // already setup on this FD.
      // disconnect from controlling terminal 
      OS::osDisconnect();
      tp = new pdRPC(0, NULL, NULL);
    }
    assert(tp);
#else

    if (!pd_flag) {
      int pid;

      pid = fork();
      if (pid == 0) {
	// configStdIO(true);
	// setup socket

	tp = new pdRPC(pd_family, pd_known_socket, pd_type, pd_machine, 
		       NULL, NULL, false);

      } else if (pid > 0) {
	// Handshaking with handleRemoteConnect() of paradyn [rpcUtil.C]
	sprintf(errorLine, "PARADYND %d\n", pid);
	// logLine(errorLine); <<--- WON'T WORK SINCE SOCKETS NOT YET SET UP!!
	fprintf(stdout, errorLine); // this works just fine...no need for logLine()
	fflush(stdout);
	_exit(-1);
      } else {
	fflush(stdout);
	exit(-1);
      }
    } else {
      OS::osDisconnect();
      tp = new pdRPC(0, NULL, NULL);
      // configStdIO(false);
    }
#endif

    cyclesPerSecond = timing_loop() * 1000000;

    // Note -- it is important that this daemon receives all mdl info
    // before starting a process
    assert(mdl_get_initial(pd_flavor, tp));

    initLibraryFunctions();
    if (!init())
      abort();

    controllerMainLoop(true);
}

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
