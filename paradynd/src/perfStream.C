/*
 *  Copyright 1993 Jeff Hollingsworth.  All rights reserved.
 *
 */

#ifndef lint
static char Copyright[] = "@(#) Copyright (c) 1993 Jeff Hollingsowrth\
    All rights reserved.";

static char rcsid[] = "@(#) $Header: /home/jaw/CVSROOT_20081103/CVSROOT/core/paradynd/src/perfStream.C,v 1.14 1994/06/02 23:27:59 markc Exp $";
#endif

/*
 * perfStream.C - Manage performance streams.
 *
 * $Log: perfStream.C,v $
 * Revision 1.14  1994/06/02 23:27:59  markc
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <string.h>


// this is missing from ptrace.h
extern "C" {
    int ptrace(enum ptracereq request,	
	       int pid,
	       char *addr, 
	       int data,
	       char *addr2);
}

#include <assert.h>
#include <unistd.h>

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

#ifdef PARADYND_PVM
#include "pvm3.h"
extern int PDYN_handle_pvmd_message();
extern void PDYN_reportSIGCHLD (int pid, int exit_status);
#endif

extern pdRPC *tp;
extern void computePauseTimeMetric();
extern void forkNodeProcesses(process *curr, traceHeader *hr, traceFork *fr);
extern void processPtraceAck (traceHeader *header, ptraceAck *ackRecord);
extern void forkProcess(traceHeader *hr, traceFork *fr);
extern void sendPtraceBuffer(process *proc);
extern void createResource(traceHeader *header, struct _newresource *res);

extern "C" Boolean synchronousMode;
Boolean synchronousMode;
Boolean firstSampleReceived;


time64 firstRecordTime = 0.0;

void processAppIO(process *curr)
{
    int ret;
    char lineBuf[1024];

    ret = read(curr->ioLink, lineBuf, sizeof(lineBuf)-1);
    if (ret < 0) {
	perror("read error");
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

void processTraceStream(process *curr)
{
    int ret;
    traceStream sid;
    char *recordData;
    traceHeader header;

    // each process has its own buffer
    // int bufStart;		/* current starting point */

    // static char buffer[2048];	/* buffer for data */
    // static int bufEnd = 0;	/* last valid data in buffer */

    ret = read(curr->traceLink, &(curr->buffer[curr->bufEnd]), sizeof(curr->buffer)-curr->bufEnd);
    if (ret < 0) {
	perror("read error");
	exit(-2);
    } else if (ret == 0) {
	/* end of file */
	printf("got EOF on link %d\n", curr->traceLink);
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

	if (header.length % WORDSIZE != 0)
	    printf("Warning: non-aligned length (%d) received on traceStream.  Type=%d\n", header.length, header.type);
	    
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
	    printf("started at %f\n", st);
	}
	header.wall -= firstRecordTime;

	switch (header.type) {
	    case TR_FORK:
		forkProcess(&header, (traceFork *) recordData);
		break;

	    case TR_NEW_RESOURCE:
		createResource(&header, (struct _newresource *) recordData);
		break;

	    case TR_MULTI_FORK:
		forkNodeProcesses(curr, &header, (traceFork *) recordData);
		break;

	    case TR_SAMPLE:
		processSample(&header, (traceSample *) recordData);
		firstSampleReceived = True;
		break;

	    case TR_PTRACE_ACK:
		processPtraceAck (&header, (ptraceAck *) recordData);
		break;

	    case TR_HANDLER_READY:
		sendPtraceBuffer (curr);
		break;

	    case TR_EXIT:
		curr->status = exited;
		break;

	    default:
		printf("got record type %d on sid %d\n", header.type, sid);
	}
    }

    /* copy those bits we have to the base */
    memcpy(curr->buffer, &(curr->buffer[curr->bufStart]), curr->bufEnd - curr->bufStart);
    curr->bufEnd = curr->bufEnd - curr->bufStart;
}

extern void dumpProcessImage(process *, Boolean stopped);

int handleSigChild(int pid, int status)
{
    int sig;
    process *curr;
    List<process*> pl;

    for (pl=processList; curr=*pl; pl++) {
	if (curr->pid == pid) break;
    }
    if (!curr) {
	// ignore signals from unkown processes
	return(-1);
    }
    if (WIFSTOPPED(status)) {
	sig = WSTOPSIG(status);
	switch (sig) {

	    case SIGTSTP:
		curr->status = stopped;
		break;

	    case SIGTRAP:
		/* trap at the start of a ptraced process 
		 *   continue past it.
		 */
		printf("passed trap at start of program\n");
		ptrace(PTRACE_CONT, pid, (char*)1, 0, 0);
#ifdef PARADYND_PVM
		curr->status = neonatal;
		pvm_perror("in SIGTRAP handler\n");
#else
		curr->status = running;
#endif
		break;

	    case SIGSTOP:
		printf("CONTROLLER: Breakpoint reached %d\n", pid);
#ifdef PARADYND_PVM
		pvm_perror("CONTROLLER: Breakpoint reached\n");
#endif
		curr->status = stopped;

		// The Unix process should be stopped already, 
		// since it's blocked on ptrace and we didn't forward
		// received the SIGSTOP...
		// But we need to pause the rest of the application
		pauseAllProcesses(); 

		/* force it into the stoped signal handler */
		// ptrace(PTRACE_CONT, pid, (char*)1, SIGPROF, 0);
		break;

	    case SIGIOT:
	    case SIGSEGV:
	    case SIGBUS:
	    case SIGILL:
		printf("caught fatal signal, dumping program image\n");
		dumpProcessImage(curr, True);
		ptrace(PTRACE_DUMPCORE, pid, "core.real", 0, 0);
		abort();
		break;

	    case SIGCHLD:
	    case SIGUSR1:
	    case SIGUSR2:
	    case SIGALRM:
	    case SIGCONT:
	    default:
		ptrace(PTRACE_CONT, pid, (char*)1, WSTOPSIG(status), 0);
		break;
	}
    } else if (WIFEXITED(status)) {
#ifdef PARADYND_PVM
		PDYN_reportSIGCHLD (pid, WEXITSTATUS(status));
#endif
	printf("process %d has terminated\n", curr->pid);
	curr->status = exited;
    } else if (WIFSIGNALED(status)) {
	printf("process %d has terminated on signal %d\n", curr->pid,
	    WTERMSIG(status));
    } else {
	printf("unknown state %d from process %d\n", status, curr->pid);
    }
    return(0);
}

#ifdef PARADYND_PVM
int PDYN_cs()
{
  struct timeval pollTime;
  int width;
  fd_set readSet;
  int *fd_ptr;
  int res, ct, fdnum;

  fdnum = pvm_getfds(&fd_ptr);

  FD_ZERO(&readSet);
  FD_SET(fd_ptr[0], &readSet);
  width = 0;

  width = fd_ptr[0];
  pollTime.tv_sec = 0;
  pollTime.tv_usec = 50000;
  ct = select(width+1, &readSet, NULL, &readSet, &pollTime);
  res = FD_ISSET(fd_ptr[0], &readSet);
  printf("\nIN PDYN_cs() PID=%d sel=%d, res=%d fdnum=%d fd=%d\n", getpid(), ct, res, fdnum, fd_ptr[0]);
  if (ct <= 0) return ct;
  return res;
}
#endif



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
    List<process*> pl;
    struct timeval pollTime;

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
	for (pl=processList; curr=*pl; pl++) {
	    if (curr->traceLink >= 0) FD_SET(curr->traceLink, &readSet);
	    if (curr->traceLink > width) width = curr->traceLink;

	    if (curr->ioLink >= 0) FD_SET(curr->ioLink, &readSet);
	    if (curr->ioLink > width) width = curr->ioLink;
	}

	// add connection to paradyn process.
	FD_SET(tp->fd, &readSet);
	FD_SET(tp->fd, &errorSet);
	if (tp->fd > width) width = tp->fd;

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
	    for (pl=processList; curr=*pl; pl++) {
		if ((curr->traceLink >= 0) && 
		    FD_ISSET(curr->traceLink, &readSet)) {
		    processTraceStream(curr);
		    /* clear it in case another process is sharing it */
		    if (curr->traceLink >= 0) FD_CLR(curr->traceLink, &readSet);
		}

		if ((curr->ioLink >= 0) && 
		    FD_ISSET(curr->ioLink, &readSet)) {
		    processAppIO(curr);
		}
	    }
	    if (FD_ISSET(tp->fd, &errorSet)) {
		// paradyn is gone so we got too.
		exit(-1);
	    }
	    if (FD_ISSET(tp->fd, &readSet)) {
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

	/* report pause time */
	computePauseTimeMetric();

	/* check for status change on inferrior processes */
	pid = wait3(&status, WNOHANG, NULL);
	if (pid > 0) {
	    handleSigChild(pid, status);
	}
    }
}


void createResource(traceHeader *header, struct _newresource *r)
{
    char *tmp;
    char *name;
    resource res;
    resource parent;

    name = r->name;
    parent = rootResource;
    while (name) {
	tmp = strchr(name, '/');
	if (tmp) {
	    *tmp = '\0';
	    tmp++;
	}
	res = newResource(parent, r->maping, name, header->wall, FALSE);
	parent = res;
	name = tmp;
    }
}
