/*
 * Main loop for the default paradynd.
 *
 * $Log: main.C,v $
 * Revision 1.21  1994/07/14 23:30:28  hollings
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
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <signal.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/termios.h>

#include "util/h/list.h"
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
#include "kludges.h"
#include "internalMetrics.h"

extern "C" {
int gethostname(char*, int);
int ioctl(int, int, caddr_t);
}

pdRPC *tp;
extern int controllerMainLoop();
extern void initLibraryFunctions();


#ifdef PARADYND_PVM
static pdRPC *init_pvm_code(char *argv[], char *machine, int family,
			     int type, int well_known_socket, int flag);
static char machine_name[80];
#endif     

int ready;

/*
 * These variables are global so that we can easily find out what
 * machine/socket/etc we're connected to paradyn on; we may need to
 * start up other paradynds (such as on the CM5), and need this later.
 */
char *pd_machine;
int pd_family;
int pd_type;
int pd_known_socket;
int pd_flag;

char *programName;


void configStdIO(Boolean closeStdIn)
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


main(int argc, char *argv[])
{
    int i;
    metricList stuff;

    programName = argv[0];

    initLibraryFunctions();

    // process command line args passed in
    // pd_flag == 1 --> started by paradyn

    assert (RPC_undo_arg_list (argc, argv, &pd_machine, pd_family, pd_type,
		       pd_known_socket, pd_flag) == 0);


#ifdef PARADYND_PVM
    tp = init_pvm_code(argv, pd_machine, pd_family, pd_type, 
		       pd_known_socket, pd_flag);
#else
    if (!pd_flag) {
	int pid;

	pid = fork();
	if (pid == 0) {
//	    configStdIO(TRUE);
	    // setup socket
	    tp = new pdRPC(pd_family, pd_known_socket, pd_type, pd_machine, 
			    NULL, NULL, 0);
	} else if (pid > 0) {
	    printf("PARADYND %d\n", pid);
	    fflush(stdout);
	    _exit(-1);
	} else {
	    fflush(stdout);
	    exit(-1);
	}
    } else {
	int ttyfd;
	// already setup on this FD.

	/* disconnect from controlling terminal */
	ttyfd = open ("/dev/tty", O_RDONLY);
	ioctl (ttyfd, TIOCNOTTY, NULL); 
	close (ttyfd);

	tp = new pdRPC(0, NULL, NULL);

//	configStdIO(FALSE);
    }
#endif

    //
    // tell client about our metrics.
    //
    stuff = getMetricList();
    for (i=0; i < stuff->count; i++) {
	tp->newMetricCallback(stuff->elements[i].info);
    }

    controllerMainLoop();
}

#ifdef PARADYND_PVM
pdRPC *
init_pvm_code(char *argv[], char *machine, int family,
	      int type, int well_known_socket, int flag)
{
  pdRPC *temp;
  extern int PDYN_initForPVM (char **, char *, int, int, int, int);

  assert(PDYN_initForPVM (argv, machine, family, type, well_known_socket,
			 flag) == 0);

  assert(!gethostname(machine_name, 99));

  // connect to paradyn
  if (flag == 1)
    temp = new pdRPC(0, NULL, NULL);
  else
    {
      temp = new pdRPC(family, well_known_socket, type, machine, NULL, NULL);
      temp->reportSelf (machine_name, argv[0], getpid());
    }

    return temp;
}

int
PDYND_report_to_paradyn (int pid, int argc, char **argv)
{
    String_Array sa;

    sa.count = argc;
    sa.data = argv;
    
    assert(tp);
    tp->newProgramCallbackFunc(pid, argc, sa, machine_name);
    return 0;
}
#endif
