/*
 *  Copyright 1993 Jeff Hollingsworth.  All rights reserved.
 *
 */

/*
 * perfStream.C - Manage performance streams.
 *
 * $Log: perfStream.C,v $
 * Revision 1.57  1996/05/08 23:54:59  mjrg
 * added support for handling fork and exec by an application
 * use /proc instead of ptrace on solaris
 * removed warnings
 *
 * Revision 1.56  1996/03/12 20:48:32  mjrg
 * Improved handling of process termination
 * New version of aggregateSample to support adding and removing components
 * dynamically
 * Added error messages
 *
 * Revision 1.55  1996/03/05 16:14:05  naim
 * Making enableDataCollection asynchronous in order to improve performance - naim
 *
 * Revision 1.54  1996/03/01  22:37:23  mjrg
 * Added a type to resources.
 * Added function handleProcessExit to handle exiting processes.
 *
 * Revision 1.53  1996/02/09 22:13:53  mjrg
 * metric inheritance now works in all cases
 * paradynd now always reports to paradyn when a process is ready to run
 * fixed aggregation to handle first samples and addition of new components
 *
 * Revision 1.52  1996/01/29 22:09:34  mjrg
 * Added metric propagation when new processes start
 * Adjust time to account for clock differences between machines
 * Daemons don't enable internal metrics when they are not running any processes
 * Changed CM5 start (paradynd doesn't stop application at first breakpoint;
 * the application stops only after it starts the CM5 daemon)
 *
 * Revision 1.51  1995/12/28 23:43:56  zhichen
 * processTraceStream() sets BURST_HAS_COMPLETED to true at the end of
 * a batch of data.
 *
 * Revision 1.50  1995/12/18  14:59:20  naim
 * Minor change to status line messages - naim
 *
 * Revision 1.49  1995/12/15  22:26:56  mjrg
 * Merged paradynd and paradyndPVM
 * Get module name for functions from symbol table in solaris
 * Fixed code generation for multiple instrumentation statements
 * Changed syntax of MDL resource lists
 *
 * Revision 1.48  1995/11/28 15:56:54  naim
 * Minor fix. Changing char[number] by string - naim
 *
 * Revision 1.47  1995/11/22  00:02:20  mjrg
 * Updates for paradyndPVM on solaris
 * Fixed problem with wrong daemon getting connection to paradyn
 * Removed -f and -t arguments to paradyn
 * Added cleanUpAndExit to clean up and exit from pvm before we exit paradynd
 * Fixed bug in my previous commit
 *
 * Revision 1.46  1995/11/03 00:06:10  newhall
 * changes to support changing the sampling rate: dynRPC::setSampleRate changes
 *     the value of DYNINSTsampleMultiple, implemented image::findInternalSymbol
 * fix so that SIGKILL is not being forwarded to CM5 applications.
 *
 * Revision 1.45  1995/10/30  23:27:52  naim
 * Fixing minor warning message - naim
 *
 * Revision 1.44  1995/10/30  23:09:01  naim
 * Updating error message - naim
 *
 * Revision 1.43  1995/10/19  22:36:43  mjrg
 * Added callback function for paradynd's to report change in status of application.
 * Added Exited status for applications.
 * Removed breakpoints from CM5 applications.
 * Added search for executables in a given directory.
 *
 * Revision 1.42  1995/09/26  22:01:21  naim
 * Minor comment added about function bzero.
 *
 * Revision 1.41  1995/09/26  20:28:49  naim
 * Minor warning fixes and some other minor error messages fixes
 *
 * Revision 1.40  1995/08/24  15:04:26  hollings
 * AIX/SP-2 port (including option for split instruction/data heaps)
 * Tracing of rexec (correctly spawns a paradynd if needed)
 * Added rtinst function to read getrusage stats (can now be used in metrics)
 * Critical Path
 * Improved Error reporting in MDL sematic checks
 * Fixed MDL Function call statement
 * Fixed bugs in TK usage (strings passed where UID expected)
 *
 * Revision 1.39  1995/05/18  10:40:37  markc
 * Added code to read mdl calls to prevent starting a process before this
 * data arrives.
 *
 * Revision 1.38  1995/02/16  08:53:54  markc
 * Corrected error in comments -- I put a "star slash" in the comment.
 *
 * Revision 1.37  1995/02/16  08:34:21  markc
 * Changed igen interfaces to use strings/vectors rather than char igen-arrays
 * Changed igen interfaces to use bool, not Boolean.
 * Cleaned up symbol table parsing - favor properly labeled symbol table objects
 * Updated binary search for modules
 * Moved machine dependnent ptrace code to architecture specific files.
 * Moved machine dependent code out of class process.
 * Removed almost all compiler warnings.
 * Use "posix" like library to remove compiler warnings
 *
 * Revision 1.36  1994/11/12  17:28:59  rbi
 * improved status reporting for applications pauses
 *
 * Revision 1.35  1994/11/11  10:44:06  markc
 * Remove non-emergency prints
 * Changed others to use statusLine
 *
 * Revision 1.34  1994/11/10  18:58:15  jcargill
 * The "Don't Blame Me Either" commit
 *
 * Revision 1.33  1994/11/09  18:40:31  rbi
 * the "Don't Blame Me" commit
 *
 * Revision 1.32  1994/11/06  18:29:56  rbi
 * hid some debugging output.
 *
 * Revision 1.31  1994/11/02  11:14:21  markc
 * Removed compiler warnings.
 * Removed unused pvm code.
 *
 * Revision 1.30  1994/10/13  07:24:52  krisna
 * solaris porting and updates
 *
 * Revision 1.29  1994/09/30  19:47:12  rbi
 * Basic instrumentation for CMFortran
 *
 * Revision 1.28  1994/09/22  02:20:56  markc
 * Added signatures for select, wait3
 *
 * Revision 1.27  1994/09/20  18:18:30  hollings
 * added code to use actual clock speed for cost model numbers.
 *
 * Revision 1.26  1994/08/17  18:17:02  markc
 * Call installDefaultInst when SIGTRAP is seen.
 * Cleaned up error messages.
 *
 * Revision 1.25  1994/08/02  18:24:16  hollings
 * added clock speed argument to printAppStats
 *
 * Revision 1.24  1994/07/26  20:01:14  hollings
 * added CMMDhostless variable.
 *
 * Revision 1.23  1994/07/15  04:19:12  hollings
 * moved dyninst stats to stats.C
 *
 * Revision 1.22  1994/07/14  23:30:30  hollings
 * Hybrid cost model added.
 *
 * Revision 1.21  1994/07/14  14:35:37  jcargill
 * Removed some dead code, added called to processArchDependentTraceStream
 *
 * Revision 1.20  1994/07/12  18:45:30  jcargill
 * Got rid of old/unused trace-record types
 *
 * Revision 1.19  1994/07/05  03:26:17  hollings
 * observed cost model
 *
 * Revision 1.18  1994/06/29  02:52:45  hollings
 * Added metricDefs-common.{C,h}
 * Added module level performance data
 * cleanedup types of inferrior addresses instrumentation defintions
 * added firewalls for large branch displacements due to text+data over 2meg.
 * assorted bug fixes.
 *
 * Revision 1.17  1994/06/27  21:28:14  rbi
 * Abstraction-specific resources and mapping info
 *
 * Revision 1.16  1994/06/27  18:57:04  hollings
 * removed printfs.  Now use logLine so it works in the remote case.
 * added internalMetric class.
 * added extra paramter to metric info for aggregation.
 *
 * Revision 1.15  1994/06/22  01:43:17  markc
 * Removed warnings.  Changed bcopy in inst-sparc.C to memcpy.  Changed process.C
 * reference to proc->status to use proc->heap->status.
 *
 * Revision 1.14  1994/06/02  23:27:59  markc
 * Replaced references to igen generated class to a new class derived from
 * this class to implement error handling for igen code.
 *
 * Revision 1.13  1994/05/31  19:53:52  markc
 * Fixed pause time bug which was causing negative values to be reported.  The
 * fix involved adding an extra test in computePauseTimeMetric that did not
 * begin reporting pause times until firstSampleReceived is TRUE.
 *
 * Revision 1.12  1994/05/31  18:00:46  markc
 * Added pvm messages to copy printf messages.
 *
 * Revision 1.11  1994/05/18  00:52:29  hollings
 * added ability to gather IO from application processes and forward it to
 * the paradyn proces.
 *
 * Revision 1.10  1994/05/16  22:31:52  hollings
 * added way to request unique resource name.
 *
 * Revision 1.9  1994/05/03  05:06:19  markc
 * Passed exit status to pvm.
 *
 * Revision 1.8  1994/04/11  23:25:25  hollings
 * Added pause_time metric.
 *
 * Revision 1.7  1994/04/09  18:34:55  hollings
 * Changed {pause,continue}Application to {pause,continue}AllProceses, and
 * made the RPC interfaces use these.  This makes the computation of pause
 * Time correct.
 *
 * Revision 1.6  1994/03/31  02:03:21  markc
 * paradyndPVM keeps a process at neonatal until the first breakpoint is
 * reached.  Moved PDYN_reportSIGCHLD to correct location.
 *
 * Revision 1.5  1994/03/26  20:50:49  jcargill
 * Changed the pause/continue code.  Now it really stops, instead of
 * spin looping.
 *
 * Revision 1.4  1994/03/20  01:53:10  markc
 * Added a buffer to each process structure to allow for multiple writers on the
 * traceStream.  Replaced old inst-pvm.C.  Changed addProcess to return type
 * int.
 *
 * Revision 1.3  1994/02/28  05:09:43  markc
 * Added pvm hooks and ifdefs.
 *
 * Revision 1.2  1994/02/24  04:32:35  markc
 * Changed header files to reflect igen changes.  main.C does not look at the number of command line arguments now.
 *
 * Revision 1.1  1994/01/27  20:31:35  hollings
 * Iinital version of paradynd speaking dynRPC igend protocol.
 *
 * Revision 1.16  1994/01/20  17:47:41  hollings
 * moved signal stuff into seperate functions.
 *
 * Revision 1.15  1993/12/14  17:57:24  jcargill
 * Added alignment sanity checking and fixup
 *
 * Revision 1.14  1993/10/19  15:27:54  hollings
 * AST based mini-tramp code generator.
 *
 * Revision 1.13  1993/10/04  21:38:22  hollings
 * added createResource.
 *
 * Revision 1.12  1993/09/08  23:02:32  hollings
 * added option to indicate if dumpCore should stop process.
 *
 * Revision 1.11  1993/08/30  18:25:40  hollings
 * added better checking for unused trace link fields.
 *
 * Revision 1.10  1993/08/26  19:51:40  hollings
 * make change for trace line >= 0 rather than non zero.
 *
 * Revision 1.9  1993/08/26  18:20:26  hollings
 * added code to dump binary on illegal instruction faults.
 *
 * Revision 1.8  1993/08/25  20:00:37  jcargill
 * Changes for new ptrace
 *
 * Revision 1.7  1993/08/11  01:57:26  hollings
 * added code for UNIX fork.
 *
 * Revision 1.6  1993/07/13  18:29:16  hollings
 * new include file syntax.
 *
 * Revision 1.5  1993/06/28  23:13:18  hollings
 * fixed process stopping.
 *
 * Revision 1.4  1993/06/24  16:18:06  hollings
 * global fixes.
 *
 * Revision 1.3  1993/06/22  19:00:01  hollings
 * global inst state.
 *
 * Revision 1.2  1993/06/08  20:14:34  hollings
 * state prior to bc net ptrace replacement.
 *
 * Revision 1.1  1993/03/19  22:45:45  hollings
 * Initial revision
 *
 *
 */


#ifdef PARADYND_PVM
extern "C" {
#include "pvm3.h"
}
#include "pvm_support.h"
#endif

#include "util/h/headers.h"
#include "rtinst/h/rtinst.h"
#include "rtinst/h/trace.h"
#include "symtab.h"
#include "process.h"
#include "inst.h"
#include "dyninstP.h"
#include "metric.h"
#include "primitives.h"
#include "util.h"
#include "comm.h"
#include "stats.h"
#include "debugger.h"
#include "main.h"
#include "association.h"
#include "init.h"
#include "context.h"
#include "perfStream.h"
#include "os.h"
#include "paradynd/src/mdld.h"
#include "showerror.h"
#include "main.h"

// TODO: this eliminates a warning but creates a conflict when compiling
// paradyndCM5.
//extern "C" void bzero(char *b, int length);

void createResource(int pid, traceHeader *header, struct _newresource *r);

bool CMMDhostless = false;
bool synchronousMode = false;
bool firstSampleReceived = false;

double cyclesPerSecond = 0;
time64 firstRecordTime = 0;

// Read output data from process curr. 
void processAppIO(process *curr)
{
    int ret;
    char lineBuf[256];

    ret = read(curr->ioLink, lineBuf, sizeof(lineBuf)-1);
    if (ret < 0) {
        //statusLine("read error");
	//showErrorCallback(23, "Read error");
	//cleanUpAndExit(-2);
        string msg = string("Read error on IO stream from PID=") +
	             string(curr->getPid()) + string(": ") +
		     string(sys_errlist[errno]) + 
		     string("\nNo more data will be received from this process.");
	showErrorCallback(23, msg);
	P_close(curr->ioLink);
	curr->ioLink = -1;
	return;
    } else if (ret == 0) {
	/* end of file */
        P_close(curr->ioLink);
	curr->ioLink = -1;
	return;
    }

    // null terminate it
    lineBuf[ret] = '\0';

    // forawrd the data to the paradyn process.
    tp->applicationIO(curr->pid, ret, lineBuf);

}


char errorLine[1024];

void logLine(const char *line)
{
    static char fullLine[1024];

    strcat(fullLine, line);
    if (fullLine[strlen(fullLine)-1] == '\n') {
	tp->applicationIO(0, strlen(fullLine), fullLine);
	fullLine[0] = '\0';
    }
}

void statusLine(const char *line)
{
  tp->reportStatus(line);
}

// New with paradynd-->paradyn buffering.  When true, it tells the
// buffering routine to flush (to paradyn); otherwise, it would flush
// only when the buffer was full, hurting response time.
extern bool BURST_HAS_COMPLETED;

// Read trace data from process curr.
void processTraceStream(process *curr)
{
    int ret;
    traceStream sid;
    char *recordData;
    traceHeader header;
    struct _association *a;

    ret = read(curr->traceLink, &(curr->buffer[curr->bufEnd]), 
	       sizeof(curr->buffer)-curr->bufEnd);

    if (ret < 0) {
        //statusLine("read error, exiting");
        //showErrorCallback(23, "Read error");
	//curr->traceLink = -1;
	//cleanUpAndExit(-2);
        string msg = string("Read error on trace stream from PID=") +
	             string(curr->getPid()) + string(": ") +
		     string(sys_errlist[errno]) + 
		     string("\nNo more data will be received from this process");
	showErrorCallback(23, msg);
	P_close(curr->traceLink);
	curr->traceLink = -1;
	return;
    } else if (ret == 0) {
	/* end of file */
	// process exited unexpectedly
	//string buffer = string("Process ") + string(curr->pid);
	//buffer += string(" has exited unexpectedly");
	//statusLine(P_strdup(buffer.string_of()));
	//showErrorCallback(11, P_strdup(buffer.string_of()));
	P_close(curr->traceLink);
  	curr->traceLink = -1;
	return;
    }

    curr->bufEnd += ret;
    curr->bufStart = 0;

    while (curr->bufStart < curr->bufEnd) {
	if (curr->bufEnd - curr->bufStart < (sizeof(traceStream) + sizeof(header))) {
	    break;
	}

	if (curr->bufStart % WORDSIZE != 0)	/* Word alignment check */
	    break;		        /* this will re-align by shifting */

	memcpy(&sid, &(curr->buffer[curr->bufStart]), sizeof(traceStream));
	curr->bufStart += sizeof(traceStream);

	memcpy(&header, &(curr->buffer[curr->bufStart]), sizeof(header));
	curr->bufStart += sizeof(header);

	curr->bufStart = ALIGN_TO_WORDSIZE(curr->bufStart);
	if (header.length % WORDSIZE != 0) {
	    sprintf(errorLine, "Warning: non-aligned length (%d) received on traceStream.  Type=%d\n", header.length, header.type);
	    logLine(errorLine);
	    showErrorCallback(36,(const char *) errorLine);
	}
	    
	if (curr->bufEnd - curr->bufStart < (unsigned)header.length) {
	    /* the whole record isn't here yet */
	    curr->bufStart -= sizeof(traceStream) + sizeof(header);
	    break;
	}

	recordData = &(curr->buffer[curr->bufStart]);
	curr->bufStart +=  header.length;

	if (!firstRecordTime)
	    firstRecordTime = header.wall;

	/*
	 * convert header to time based on first record.
	 *
	 */
	if (!curr->getFirstRecordTime()) {
	    double st;

	    curr->setFirstRecordTime(header.wall);
	    st = curr->getFirstRecordTime() / 1000000.0;
	    /* sprintf(errorLine, "started at %f\n", st);*/
	    /* logLine(errorLine);*/
	    // report sample here
	    
	    // If we are running a CM5 process, we don't send a first sample.
	    // The CM5 node daemon will send a sample when it is ready.
	    if (!CMMDhostless) {
	      tp->firstSampleCallback(curr->getPid(), (double) (header.wall/1000000.0));
	    }
	}
	// header.wall -= curr->firstRecordTime();
	switch (header.type) {
	    case TR_FORK:
		forkProcess((traceFork *) ((void*)recordData));
		break;

	    case TR_NEW_RESOURCE:
		createResource(curr->getPid(), &header, (struct _newresource *) ((void*)recordData));
		break;

	    case TR_NEW_ASSOCIATION:
		a = (struct _association *) ((void*)recordData);
		newAssoc(curr, a->abstraction, a->type, a->key, a->value);
		break;

	    case TR_MULTI_FORK:
		// logLine("got TR_MULTI_FORK record\n");
		CMMDhostless = true;
		forkNodeProcesses(curr, &header, (traceFork *) ((void*)recordData));
		statusLine("node daemon started");
		// the process stops itself after writing this trace record
		// and must wait until the daemon is ready.
		curr->waitingForNodeDaemon = true;
		curr->status_ = stopped;
		break;

	    case TR_SAMPLE:
		// sprintf(errorLine, "Got data from process %d\n", curr->pid);
		// logLine(errorLine);
		assert(curr->getFirstRecordTime());
		processSample(&header, (traceSample *) ((void*)recordData));
		firstSampleReceived = true;
		break;

	    case TR_EXIT:
		sprintf(errorLine, "process %d exited\n", curr->pid);
		logLine(errorLine);
		printAppStats((struct endStatsRec *) ((void*)recordData),
			      cyclesPerSecond);
		printDyninstStats();
  		P_close(curr->traceLink);
  		curr->traceLink = -1;
		// for processes that are our direct children, we handle exit
		// when we find that the process exited with waitpid.
		if (curr->parent)
		  handleProcessExit(curr, 0);
		break;

	    case TR_COST_UPDATE:
		processCost(curr, &header, (costUpdate *) ((void*)recordData));
		break;

	    case TR_CP_SAMPLE:
		extern void processCP(process *, traceHeader *, cpSample *);
		processCP(curr, &header, (cpSample *) recordData);
		break;

	    case TR_EXEC:
		{ traceExec *execData = (traceExec *) recordData;
		  process *p = findProcess(execData->pid);
		  p->inExec = true;
		  p->execFilePath = string(execData->path);
		}
		break;

	    case TR_EXEC_FAILED:
		{ int pid = *(int *)recordData;
		  process *p = findProcess(pid);
		  p->inExec = false;
		  p->execFilePath = string("");
		}
		break;

	    default:
		sprintf(errorLine, "Got record type %d on sid %d\n", 
		    header.type, sid);
		logLine(errorLine);
		sprintf(errorLine, "Received bad trace data from process %d.", curr->getPid());
		showErrorCallback(37,(const char *) errorLine);
	}
    }
    BURST_HAS_COMPLETED = true; // will force a batch-flush very soon

    /* copy those bits we have to the base */
    memcpy(curr->buffer, &(curr->buffer[curr->bufStart]), 
	curr->bufEnd - curr->bufStart);
    curr->bufEnd = curr->bufEnd - curr->bufStart;
}

// TODO -- make this a process method
int handleSigChild(int pid, int status)
{
    int sig;
    string buffer;

    // ignore signals from unknown processes
    process *curr = findProcess(pid);
    if (!curr) return -1;

    if (WIFSTOPPED(status)) {
	sig = WSTOPSIG(status);
	switch (sig) {

	    case SIGTSTP:
		sprintf(errorLine, "process %d got SIGTSTP", pid);
		statusLine(errorLine);
		curr->Stopped();
		break;

	    case SIGTRAP:
		/* trap at the start of a ptraced process 
		 *   continue past it.
		 */

		if (curr->inExec) {
		  // the process has executed a succesful exec, and is now
		  // stopped at the exit of the exec call.
		  curr->handleExec();
		  curr->inExec = false;
		  curr->reachedFirstBreak = false;
		}

		// the process is stopped as a result of the initial SIGTRAP
		curr->status_ = stopped;

		// query default instrumentation here - not done yet
		// We must check that this is the first trap since on machines
		//   where we use ptrace detach, a TRAP is generated on a pause.
		//   - jkh 7/7/95
		if (!curr->reachedFirstBreak) {
		    buffer = string("PID=") + string(pid);
		    buffer += string(", passed trap at start of program");
		    statusLine(P_strdup(buffer.string_of()));
		    installDefaultInst(curr, initialRequests);
		    curr->reachedFirstBreak = 1;

                    // propagate any metric that is already enabled to the new
                    // process
                    vector<metricDefinitionNode *> MIs = allMIs.values();
                    for (unsigned j = 0; j < MIs.size(); j++) {
			MIs[j]->propagateMetricInstance(curr);
		    }
		    costMetric::addProcessToAll(curr);

                    if (CMMDhostless) {
		       // This is a cm5 process and we must run it so it can start
		       // the node daemon.
		       if (!OS::osForwardSignal(pid, 0)) {
			 assert(0);
		       }
                       statusLine("starting node daemon ...");
		       // The application will stop itself after it starts the 
		       // node daemon. Setting waitingForNodeDaemon here will
		       // prevent the application from running again before
		       // the node daemon is ready.
		       curr->waitingForNodeDaemon = true;
		    }

		    tp->newProgramCallbackFunc(pid, curr->arg_list, 
					       machineResource->part_name());

		}
		break;

	    case SIGSTOP:
	    case SIGINT:

		if (curr->waitingForNodeDaemon) {
		  // CM5 kludge: the application stops after writing the TR_MULTI_FORK
		  // trace record to start the node daemon. It will be re-started
		  // by paradyn when the node daemon is ready.
		  // no need to update status here
		  break;
		}

		if (curr->reachedFirstBreak == false && curr->status() == neonatal) {
		  // forked process
		  curr->reachedFirstBreak = true;
		  curr->status_ = stopped;
		  metricDefinitionNode::handleFork(curr->parent, curr);
		  tp->newProgramCallbackFunc(pid, curr->arg_list,
					     machineResource->part_name());
		  break;
		}

		curr->Stopped();
		curr->reachedFirstBreak = 1;

		// The Unix process should be stopped already, 
		// since it's blocked on ptrace and we didn't forward
		// received the SIGSTOP...
		// But we need to pause the rest of the application
		//pauseAllProcesses();
		//statusLine("application paused");
		break;

	    case SIGIOT:
	    case SIGBUS:
	    case SIGILL:
		curr->status_ = stopped;
		dumpProcessImage(curr, true);
		OS::osDumpCore(pid, "core.real");
		//handleProcessExit(curr);
		// ???
		// should really log this to the error reporting system.
		// jkh - 6/25/96
		// now forward it to the process.
		OS::osForwardSignal(pid, WSTOPSIG(status));
		break;

	    case SIGCHLD:
	    case SIGUSR1:
	    case SIGUSR2:
	    case SIGALRM:
	    case SIGVTALRM:
	    case SIGCONT:
	    case SIGSEGV:	// treadmarks needs this signal
		// printf("caught signal, forwarding...  (sig=%d)\n", 
		//       WSTOPSIG(status));
		if (!OS::osForwardSignal(pid, WSTOPSIG(status))) {
                     logLine("error  in forwarding  signal\n");
		     showErrorCallback(38, "Error  in forwarding  signal");
                     //P_abort();
                }
		break;

#ifdef CM5_SIGXCPU_KLUDGE
	      // don't forward SIGXCPU so that applications may run for more
	      // than the max CPUtime limit
	      case SIGXCPU:
	      case SIGKILL:
//		sprintf(errorLine,"Process %d received signal SIGXCPU or SIGKILL. Not forwarded.\n",pid);
//		logLine(errorLine);
		OS::osForwardSignal(pid,0);
		break;
#endif

#ifdef notdef
	    // XXXX for debugging
	    case SIGSEGV:	// treadmarks needs this signal
		sprintf(errorLine, "DEBUG: forwarding signal (sig=%d, pid=%d)\n"
			, WSTOPSIG(status), pid);
		logLine(errorLine);
#endif
	    default:
		if (!OS::osForwardSignal(pid, WSTOPSIG(status))) {
                     logLine("error  in forwarding  signal\n");
                     P_abort();
                }
		break;

	}
    } else if (WIFEXITED(status)) {
#ifdef PARADYND_PVM
        //  if (pvm_running) {
        //         PDYN_reportSIGCHLD (pid, WEXITSTATUS(status));
	//}
#endif
	sprintf(errorLine, "Process %d has terminated\n", curr->pid);
	statusLine(errorLine);
	logLine(errorLine);

	printDyninstStats();
        handleProcessExit(curr, WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
	sprintf(errorLine, "process %d has terminated on signal %d\n", curr->pid,
	    WTERMSIG(status));
	logLine(errorLine);
	statusLine(errorLine);
	handleProcessExit(curr, WTERMSIG(status));
    } else {
	sprintf(errorLine, "Unknown state %d from process %d\n", status, curr->pid);
	logLine(errorLine);
	showErrorCallback(39,(const char *) errorLine);
    }
    return(0);
}

void ioFunc()
{
     printf("in SIG child func\n");
     fflush(stdout);
}


/*
 * Wait for a data from one of the inferriors or a request to come in.
 *
 */
void controllerMainLoop(bool check_buffer_first)
{
    int ct;
    int pid;
    int width;
    int status;
    fd_set readSet;
    fd_set errorSet;
    struct timeval pollTime;

    // TODO - i am the guilty party - this will go soon - mdc
#ifdef PARADYND_PVM
#ifdef notdef
    int fd_num, *fd_ptr;
    if (pvm_mytid() < 0)
      {
	printf("pvm not working\n");
	_exit(-1);
      }
    fd_num = pvm_getfds(&fd_ptr);
    assert(fd_num == 1);
#endif
#endif

    while (1) {
	FD_ZERO(&readSet);
	FD_ZERO(&errorSet);
	width = 0;
	unsigned p_size = processVec.size();
	for (unsigned u=0; u<p_size; u++) {
	    if (processVec[u]->traceLink >= 0)
	      FD_SET(processVec[u]->traceLink, &readSet);
	    if (processVec[u]->traceLink > width)
	      width = processVec[u]->traceLink;
	    if (processVec[u]->ioLink >= 0)
	      FD_SET(processVec[u]->ioLink, &readSet);
	    if (processVec[u]->ioLink > width)
	      width = processVec[u]->ioLink;
	}

	// add trace 
	extern int traceSocket_fd;
	if (traceSocket_fd > 0) FD_SET(traceSocket_fd, &readSet);
	if (traceSocket_fd > width) width = traceSocket_fd;

	// add connection to paradyn process.
	FD_SET(tp->get_fd(), &readSet);
	FD_SET(tp->get_fd(), &errorSet);
	if (tp->get_fd() > width) width = tp->get_fd();

#ifdef PARADYND_PVM
	// add connection to pvm daemon.
	/***
	  There is a problem here since pvm_getfds is not implemented on 
	  libpvmshmem which we use on solaris (a call to pvm_getfds returns
	  PvmNotImpl).
	  If we cannot use pvm_getfds here, the only alternative is to use polling.
	  To keep the code simple, I am using polling in all cases.
	***/
#ifdef notdef // not in use because pvm_getfds is not implemented on all platforms
	fd_num = pvm_getfds(&fd_ptr);
	assert(fd_num == 1);
	FD_SET(fd_ptr[0], &readSet);
	if (fd_ptr[0] > width)
	  width = fd_ptr[0];
#endif
#endif

	// poll for IO
	pollTime.tv_sec = 0;
	pollTime.tv_usec = 50000;

	// This fd may have been read from prior to entering this loop
	// There may be some bytes lying around
	if (check_buffer_first) {
	  bool no_stuff_there = P_xdrrec_eof(tp->net_obj());
	  while (!no_stuff_there) {
	    T_dyninstRPC::message_tags ret = tp->waitLoop();
	    if (ret == T_dyninstRPC::error) {
	      // assume the client has exited, and leave.
	      cleanUpAndExit(-1);
	    }
	    no_stuff_there = P_xdrrec_eof(tp->net_obj());
	  }
	}

	// TODO - move this into an os dependent area
	ct = P_select(width+1, &readSet, NULL, &errorSet, &pollTime);

	if (ct > 0) {

	    if (traceSocket_fd >= 0 && FD_ISSET(traceSocket_fd, &readSet)) {
	      // a process forked by a process we are tracing is trying
	      // to get a connection for the trace stream. 
	      // Accept the connection.
	      int fd = RPC_getConnect(traceSocket_fd);
	      assert(fd >= 0);
	      int pid;
	      if (read(fd, &pid, sizeof(pid)) != sizeof(pid))
		abort();
	      fprintf(stderr, "Connected to process %d\n", pid);
	      process *p = findProcess(pid);
	      assert(p);
	      p->traceLink = fd;
	    }

            unsigned p_size = processVec.size();
	    for (unsigned u=0; u<p_size; u++) {
		if ((processVec[u]->traceLink >= 0) && 
		    FD_ISSET(processVec[u]->traceLink, &readSet)) {
		    processTraceStream(processVec[u]);
		    /* clear it in case another process is sharing it */
		    if (processVec[u]->traceLink >= 0) 
			FD_CLR(processVec[u]->traceLink, &readSet);
		}

		if ((processVec[u]->ioLink >= 0) && 
		    FD_ISSET(processVec[u]->ioLink, &readSet)) {
		    processAppIO(processVec[u]);
		    /* clear it in case another process is sharing it */
		    if (processVec[u]->ioLink >= 0) 
			FD_CLR(processVec[u]->ioLink, &readSet);
		}
	    }
	    if (FD_ISSET(tp->get_fd(), &errorSet)) {
		// paradyn is gone so we got too.
	        cleanUpAndExit(-1);
	    }
	    if (FD_ISSET(tp->get_fd(), &readSet)) {
	      bool no_stuff_there = false;
	      while(!no_stuff_there) {
		T_dyninstRPC::message_tags ret = tp->waitLoop();
		if (ret == T_dyninstRPC::error) {
		  // assume the client has exited, and leave.
		  cleanUpAndExit(-1);
		}
		no_stuff_there = P_xdrrec_eof(tp->net_obj());
	      }
	    }
	    while (tp->buffered_requests()) {
	      T_dyninstRPC::message_tags ret = tp->process_buffered();
	      if (ret == T_dyninstRPC::error)
		cleanUpAndExit(-1);
	    }

#ifdef PARADYND_PVM
#ifdef notdef // not in use because of the problems with pvm_getfds. See comment above.
	    // message on pvmd channel
	    int res;
            fd_num = pvm_getfds(&fd_ptr);
	    assert(fd_num == 1);
	    if (FD_ISSET(fd_ptr[0], &readSet)) {
		// res == -1 --> error
		res = PDYN_handle_pvmd_message();
		// handle pvm message
	    }
#endif
#endif
	}
#ifdef PARADYND_PVM
	// poll for messages from the pvm daemon, and handle the message if 
	// there is one.
	// See comments above on the problems with pvm_getfds.
	if (pvm_running) {
	  PDYN_handle_pvmd_message();
	}
#endif
	processArchDependentTraceStream();

	/* generate internal metrics */
	reportInternalMetrics();

	/* check for status change on inferrior processes */
	pid = process::waitProcs(&status);
	if (pid > 0) {
	    handleSigChild(pid, status);
	}
    }
}


void createResource(int pid, traceHeader *header, struct _newresource *r)
{
    char *tmp;
    char *name;
    // resource *res;
    vector<string> parent_name;
    resource *parent = NULL;
    unsigned type;
    
    switch (r->type) {
    case RES_TYPE_STRING: type = MDL_T_STRING; break;
    case RES_TYPE_INT:    type = MDL_T_INT; break;
    default: 
      string msg = string("Invalid resource type reported on trace stream from PID=")
	           + string(pid);
      showErrorCallback(36,msg);
      return;
    }

    name = r->name;
    do {
	tmp = strchr(name, '/');
	if (tmp) {
	    *tmp = '\0';
	    tmp++;
	    parent_name += name;
	    name = tmp;
	}
    } while (tmp);

    if ((parent = resource::findResource(parent_name)) && name != r->name) {
      resource::newResource(parent, NULL, r->abstraction, name,
			    header->wall, "", type);
    }
    else {
      string msg = string("Unknown resource '") + string(r->name) +
	           string("' reported on trace stream from PID=") +
		   string(pid);
      showErrorCallback(36,msg);
    }

}

