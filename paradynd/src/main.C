/*
 * Main loop for the default paradynd.
 *
 * $Log: main.C,v $
 * Revision 1.7  1994/03/31 01:57:27  markc
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
#include "dyninstRPC.SRVR.h"

dynRPC *tp;
extern void controllerMainLoop();
extern void initLibraryFunctions();

#ifdef PARADYND_PVM
static dynRPC *init_pvm_code(char *argv[], char *machine, int family,
			     int type, int well_known_socket, int flag);
static char machine_name[80];
#endif     

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
    tp = new dynRPC(0, NULL, NULL);
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
    if (pr) (void) newResource(pr, NULL, name, 0.0);
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
    struct List<process *> curr;

    for (curr = processList; *curr; curr++) {
        continueProcess(*curr);
    }
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
    struct List<process *> curr;

    for (curr = processList; *curr; curr++) {
        pauseProcess(*curr);
    }
    flushPtrace();
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
    continueApplication();
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
dynRPC *
init_pvm_code(char *argv[], char *machine, int family,
	      int type, int well_known_socket, int flag)
{
  dynRPC *temp;

  extern int PDYN_initForPVM (char **, char *, int, int, int, int);

  assert(PDYN_initForPVM (argv, machine, family, type, well_known_socket,
			 flag) == 0);

  // connect to paradyn
  if (flag == 1)
    temp = new dynRPC(0, NULL, NULL);
  else
    {
      temp = new dynRPC(family, well_known_socket, type, machine, NULL, NULL);
      temp->reportSelf (machine, argv[0], getpid());
    }

    return temp;
}

int
PDYND_report_to_paradyn (int pid, int argc, char **argv)
{
    String_Array sa;

    sa.count = argc;
    sa.data = argv;
    assert(!gethostname(machine_name, 99));

    assert(tp);
    tp->newProgramCallbackFunc(pid, argc, sa, machine_name);
    return 0;
}
#endif

