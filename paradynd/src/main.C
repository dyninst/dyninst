/*
 * Main loop for the default paradynd.
 *
 * $Log: main.C,v $
 * Revision 1.15  1994/06/22 03:46:31  markc
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
#include <assert.h>
#include <fcntl.h>

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

extern "C" {
int gethostname(char*, int);
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

// default to once a second.
float samplingRate = 1.0;

main(int argc, char *argv[])
{
    int i, family, type, well_known_socket, flag;
    char *machine;
    metricList stuff;

    initLibraryFunctions();

    // process command line args passed in
    // flag == 1 --> started by paradyn
    assert (RPC_undo_arg_list (argc, argv, &machine, family, type,
		       well_known_socket, flag) == 0);

#ifdef PARADYND_PVM
    tp = init_pvm_code(argv, machine, family, type, well_known_socket, flag);
#else
    if (!flag) {
	int pid;


	pid = fork();
	if (pid == 0) {
	    int nullfd;

	    /* now make stdin, out and error things that we can't hurt us */
	    if ((nullfd = open("/dev/null", O_RDWR, 0)) < 0) {
		abort();
	    }
	    (void) dup2(nullfd, 0);
	    (void) dup2(nullfd, 1);
	    (void) dup2(nullfd, 2);

	    if (nullfd > 2) close(nullfd);

	    // setup socket
	    tp = new pdRPC(family, well_known_socket, type, machine, 
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
	// already setup on this FD.
	tp = new pdRPC(0, NULL, NULL);
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

void dynRPC::addResource(String parent, String name)
{
    resource pr;
    extern resource findResource(char*);

    pr = findResource(parent);
    if (pr) (void) newResource(pr, NULL, name, 0.0, FALSE);
}

void dynRPC::coreProcess(int pid)
{
    process *proc;

    proc = processList.find((void *) pid);
    dumpCore(proc);
}

String dynRPC::getStatus(int pid)
{
    process *proc;
    extern char *getProcessStatus(process *proc);
    char ret[50];

    proc = processList.find((void *) pid);

    if (!proc) {
	sprintf (ret, "PID:%d not found for getStatus\n", pid);
	return (ret);
    }
    else
	return(getProcessStatus(proc));
}

//
// NOTE: This version of getAvailableMetrics assumes that new metrics are
//   NOT added during execution.
//
metricInfo_Array dynRPC::getAvailableMetrics(void)
{
    int i;
    static int inited;
    static metricInfo_Array metInfo;

    if (!inited) {
	metricList stuff;

	stuff = getMetricList();
	metInfo.count = stuff->count;
	metInfo.data = (metricInfo*) calloc(sizeof(metricInfo), metInfo.count);
	for (i=0; i < metInfo.count; i++) {
	    metInfo.data[i] = stuff->elements[i].info;
	}
	inited = 1;
    }
    return(metInfo);
}

double dynRPC::getPredictedDataCost(String_Array focusString, String metric)
{
    metric m;
    double val;
    resourceList l;

    m = findMetric(metric);
    l = findFocus(focusString.count, focusString.data);
    if (!l) return(0.0);
    val = guessCost(l, m);

    return(val);
}

void dynRPC::disableDataCollection(int mid)
{
    metricInstance mi;
    extern void printResourceList(resourceList);

    printf("disable of %s for RL =", getMetricName(mi->met));
    printResourceList(mi->resList);
    printf("\n");

    mi = allMIs.find((void *) mid);
    mi->disable();
    allMIs.remove(mi);
    delete(mi);
}

int dynRPC::enableDataCollection(String_Array foucsString,String metric)
{
    int id;
    metric m;
    resourceList l;

    m = findMetric(metric);
    l = findFocus(foucsString.count, foucsString.data);
    if (!l) return(-1);
    id = startCollecting(l, m);
    return(id);
}

//
// not implemented yet.
//
void dynRPC::setSampleRate(double sampleInterval)
{
    samplingRate = sampleInterval;
    return;
}

Boolean dynRPC::detachProgram(int program,Boolean pause)
{
    return(detachProcess(program, pause));
}

//
// Continue all processes
//
void dynRPC::continueApplication()
{
    continueAllProcesses();
}

//
// Continue a process
//
void dynRPC::continueProgram(int program)
{
    struct List<process *> curr;

    for (curr = processList; *curr; curr++) {
	if ((*curr)->pid == pid) break;
    }
    if (*curr)
        continueProcess(*curr);
    else
	printf("Can't continue PID %d\n", program);
}

//
//  Stop all processes 
//
Boolean dynRPC::pauseApplication()
{
    pauseAllProcesses();
    return TRUE;
}

//
//  Stop a single process
//
Boolean dynRPC::pauseProgram(int program)
{
    struct List<process *> curr;

    for (curr = processList; *curr; curr++) {
        if ((*curr)->pid == program) break;
    }
    if (!(*curr)) {
	printf("Can't pause PID %d\n", program);
	return FALSE;
    }
    pauseProcess(*curr);
    flushPtrace();
    return TRUE;
}

Boolean dynRPC::startProgram(int program)
{
    continueAllProcesses();
    return(False);
}

//
// This is not implemented yet.
//
Boolean dynRPC::attachProgram(int pid)
{
    return(FALSE);
}

//
// start a new program for the tool.
//
int dynRPC::addExecutable(int argc,String_Array argv)
{
    return(addProcess(argc, argv.data));
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
