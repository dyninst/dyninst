/*
 *  Copyright 1993 Jeff Hollingsworth.  All rights reserved.
 *
 */

#ifndef lint
static char Copyright[] = "@(#) Copyright (c) 1993 Jeff Hollingsowrth\
    All rights reserved.";

static char rcsid[] = "@(#) $Header: /home/jaw/CVSROOT_20081103/CVSROOT/core/paradynd/src/perfStream.C,v 1.31 1994/11/02 11:14:21 markc Exp $";
#endif

/*
 * perfStream.C - Manage performance streams.
 *
 * $Log: perfStream.C,v $
 * Revision 1.31  1994/11/02 11:14:21  markc
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


#include "util/h/kludges.h"

#ifdef PARADYND_PVM
extern "C" {
#include "pvm3.h"
}
#include "paradyndPVM/h/pvm_support.h"
#endif

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

void createResource(traceHeader *header, struct _newresource *r);

bool CMMDhostless = false;
bool synchronousMode = false;
bool firstSampleReceived = false;

float cyclesPerSecond = 0;
time64 firstRecordTime = 0;

void processAppIO(process *curr)
{
    int ret;
    char lineBuf[1024];

    ret = read(curr->ioLink, lineBuf, sizeof(lineBuf)-1);
    if (ret < 0) {
        logLine("read error\n");
	exit(-2);
    } else if (ret == 0) {
	/* end of file */
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

void processTraceStream(process *curr)
{
    int ret;
    traceStream sid;
    char *recordData;
    traceHeader header;
    struct _association *a;

    // each process has its own buffer
    // int bufStart;		/* current starting point */

    // static char buffer[2048];	/* buffer for data */
    // static int bufEnd = 0;	/* last valid data in buffer */

    ret = read(curr->traceLink, &(curr->buffer[curr->bufEnd]), sizeof(curr->buffer)-curr->bufEnd);
    if (ret < 0) {
        logLine("read error, exiting");
	exit(-2);
    } else if (ret == 0) {
	/* end of file */
	sprintf(errorLine, "got EOF on link %d\n", curr->traceLink);
	logLine(errorLine);
	curr->traceLink = -1;
	curr->status = exited;
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
	}
	    
	if (curr->bufEnd - curr->bufStart < header.length) {
	    /* the whole record isn't here yet */
	    curr->bufStart -= sizeof(traceStream) + sizeof(header);
	    break;
	}

	recordData = &(curr->buffer[curr->bufStart]);
	curr->bufStart +=  header.length;

	/*
	 * convert header to time based on first record.
	 *
	 */
	if (!firstRecordTime) {
	    double st;

	    firstRecordTime = header.wall;
	    st = firstRecordTime/1000000.0;
	    sprintf(errorLine, "started at %f\n", st);
	    logLine(errorLine);
	}
	header.wall -= firstRecordTime;

	switch (header.type) {
	    case TR_FORK:
		forkProcess(&header, (traceFork *) recordData);
		break;

	    case TR_NEW_RESOURCE:
		createResource(&header, (struct _newresource *) recordData);
		break;

	    case TR_NEW_ASSOCIATION:
		a = (struct _association *) recordData;
		newAssoc(curr, a->abstraction, a->type, a->key, a->value);
		break;

	    case TR_MULTI_FORK:
		logLine("got TR_MULTI_FORK record\n");
		CMMDhostless = true;
		forkNodeProcesses(curr, &header, (traceFork *) recordData);
		break;

	    case TR_SAMPLE:
		// sprintf(errorLine, "Got data from process %d\n", curr->pid);
		// logLine(errorLine);
		processSample(&header, (traceSample *) recordData);
		firstSampleReceived = true;
		break;

	    case TR_EXIT:
		sprintf(errorLine, "process %d exited\n", curr->pid);
		logLine(errorLine);
		printAppStats((struct endStatsRec *) recordData, cyclesPerSecond);
		printDyninstStats();

		curr->status = exited;
		break;

	    case TR_COST_UPDATE:
		processCost(curr, &header, (costUpdate *) recordData);
		break;

	    default:
		sprintf(errorLine, "got record type %d on sid %d\n", 
		    header.type, sid);
		logLine(errorLine);
	}
    }

    /* copy those bits we have to the base */
    memcpy(curr->buffer, &(curr->buffer[curr->bufStart]), 
	curr->bufEnd - curr->bufStart);
    curr->bufEnd = curr->bufEnd - curr->bufStart;
}

int handleSigChild(int pid, int status)
{
    int sig;
    process *curr;

    // ignore signals from unknown processes
    if (!processMap.defines(pid))
      return -1;

    curr = processMap[pid];

    if (WIFSTOPPED(status)) {
	sig = WSTOPSIG(status);
	switch (sig) {

	    case SIGTSTP:
		sprintf(errorLine, "process %d got SIGTSTP\n", pid);
		logLine(errorLine);
		curr->status = stopped;
		break;

	    case SIGTRAP:
		/* trap at the start of a ptraced process 
		 *   continue past it.
		 */
		sprintf(errorLine, "PID=%d, passed trap at start of program\n", pid);
		logLine(errorLine);

		// the process is stopped as a result of the initial SIGTRAP
		curr->status = stopped;

		// query default instrumentation here - not done yet
		installDefaultInst(curr, initialRequests);

		if (!osForwardSignal(pid, 0)) {
		  abort();
		}

		// If this is a CM-process, we don't want to label it as
		// running until the nodes get init'ed.  We need to test
		// based on magic number, I guess...   XXXXXX

		curr->status = running;
		break;

	    case SIGSTOP:
		sprintf(errorLine, "PID=%d, CONTROLLER: Breakpoint reached\n",pid);
		logLine(errorLine);

		curr->status = stopped;
		curr->reachedFirstBreak = 1;

		// The Unix process should be stopped already, 
		// since it's blocked on ptrace and we didn't forward
		// received the SIGSTOP...
		// But we need to pause the rest of the application
		pauseAllProcesses(); 
		break;

	    case SIGIOT:
	    case SIGSEGV:
	    case SIGBUS:
	    case SIGILL:
		dumpProcessImage(curr, true);
		osDumpCore(pid, "core.real");
		curr->status = exited;
		// ???
		// should really log this to the error reporting system.
		// jkh - 6/25/96
		logLine("caught fatal signal, dumping program image\n");
		// now forward it to the process.
		osForwardSignal(pid, WSTOPSIG(status));
		break;

	    case SIGCHLD:
	    case SIGUSR1:
	    case SIGUSR2:
	    case SIGALRM:
	    case SIGVTALRM:
	    case SIGCONT:
		// printf("caught signal, forwarding...  (sig=%d)\n", 
		//       WSTOPSIG(status));
		osForwardSignal(pid, WSTOPSIG(status));
		break;

	    default:
		sprintf(errorLine, "ERROR: unhandled signal, not forwarding.  (sig=%d, pid=%d)\n", 
			WSTOPSIG(status), pid);
		logLine(errorLine);
		osForwardSignal(pid, 0);
		break;

	}
    } else if (WIFEXITED(status)) {
#ifdef PARADYND_PVM
		PDYN_reportSIGCHLD (pid, WEXITSTATUS(status));
#endif
	sprintf(errorLine, "process %d has terminated\n", curr->pid);
	logLine(errorLine);

	printDyninstStats();
	curr->status = exited;
    } else if (WIFSIGNALED(status)) {
	sprintf(errorLine, "process %d has terminated on signal %d\n", curr->pid,
	    WTERMSIG(status));
	logLine(errorLine);
    } else {
	sprintf(errorLine, "unknown state %d from process %d\n", status, curr->pid);
	logLine(errorLine);
    }
    return(0);
}

/*
 * Wait for a data from one of the inferriors or a request to come in.
 *
 */
void controllerMainLoop()
{
    int ct;
    int pid;
    int ret;
    int width;
    int status;
    process *curr;
    fd_set readSet;
    fd_set errorSet;
    struct timeval pollTime;

    // TODO - i am the guilty party - this will go soon - mdc
#ifdef PARADYND_PVM
    int fd_num, *fd_ptr;
    if (pvm_mytid() < 0)
      {
	printf("pvm not working\n");
	_exit(-1);
      }
    fd_num = pvm_getfds(&fd_ptr);
    assert(fd_num == 1);
#endif
    
    while (1) {
	FD_ZERO(&readSet);
	FD_ZERO(&errorSet);
	width = 0;
	int i;
	dictionary_hash_iter<int, process*> pi(processMap);
	while (pi.next(i, curr)) {
	    if (curr->traceLink >= 0) FD_SET(curr->traceLink, &readSet);
	    if (curr->traceLink > width) width = curr->traceLink;

	    if (curr->ioLink >= 0) FD_SET(curr->ioLink, &readSet);
	    if (curr->ioLink > width) width = curr->ioLink;
	}

	// add connection to paradyn process.
	FD_SET(tp->getFd(), &readSet);
	FD_SET(tp->getFd(), &errorSet);
	if (tp->getFd() > width) width = tp->getFd();

#ifdef PARADYND_PVM
	fd_num = pvm_getfds(&fd_ptr);
	assert(fd_num == 1);
	FD_SET(fd_ptr[0], &readSet);
	if (fd_ptr[0] > width)
	  width = fd_ptr[0];
#endif
	pollTime.tv_sec = 0;
	pollTime.tv_usec = 50000;
	ct = select(width+1, &readSet, NULL, &errorSet, &pollTime);
	if (ct > 0) {
	    int i;
	    dictionary_hash_iter <int, process*> pi(processMap);
	    while (pi.next(i, curr)) {
		if ((curr->traceLink >= 0) && 
		    FD_ISSET(curr->traceLink, &readSet)) {
		    processTraceStream(curr);
		    /* clear it in case another process is sharing it */
		    if (curr->traceLink >= 0) 
			FD_CLR(curr->traceLink, &readSet);
		}

		if ((curr->ioLink >= 0) && 
		    FD_ISSET(curr->ioLink, &readSet)) {
		    processAppIO(curr);
		}
	    }
	    if (FD_ISSET(tp->getFd(), &errorSet)) {
		// paradyn is gone so we got too.
		exit(-1);
	    }
	    if (FD_ISSET(tp->getFd(), &readSet)) {
		ret = tp->mainLoop();
		if (ret < 0) {
		    // assume the client has exited, and leave.
		    exit(-1);
		}
	    }

#ifdef PARADYND_PVM
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
	}

	processArchDependentTraceStream();

	/* generate internal metrics */
	reportInternalMetrics();

	/* check for status change on inferrior processes */
#if defined(sparc_sun_sunos4_1_3) || defined(sparc_tmc_cmost7_3)
	pid = wait3(&status, WNOHANG, NULL);
#elif defined(sparc_sun_solaris2_3)
	pid = waitpid(0, &status, WNOHANG);
#endif
	if (pid > 0) {
	    handleSigChild(pid, status);
	}
    }
}


void createResource(traceHeader *header, struct _newresource *r)
{
    char *tmp;
    char *name;
    resource *res;
    resource *parent;

    name = r->name;
    parent = rootResource;
    while (name) {
	tmp = strchr(name, '/');
	if (tmp) {
	    *tmp = '\0';
	    tmp++;
	}
	res = newResource(parent, NULL, r->abstraction, name, 
			  header->wall, false);
	parent = res;
	name = tmp;
    }
}

