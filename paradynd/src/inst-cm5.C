/*
 *  Copyright 1993 Jeff Hollingsworth.  All rights reserved.
 *
 */

#ifndef lint
static char Copyright[] = "@(#) Copyright (c) 1993 Jeff Hollingsowrth\
    All rights reserved.";

static char rcsid[] = "@(#) $Header: /home/jaw/CVSROOT_20081103/CVSROOT/core/paradynd/src/Attic/inst-cm5.C,v 1.2 1994/03/22 21:03:13 hollings Exp $";
#endif

/*
 * inst-cm5.C - runtime library specific files to inst on this machine.
 *
 * $Log: inst-cm5.C,v $
 * Revision 1.2  1994/03/22 21:03:13  hollings
 * Made it possible to add new processes (& paradynd's) via addExecutable.
 *
 * Revision 1.1  1994/01/27  20:31:20  hollings
 * Iinital version of paradynd speaking dynRPC igend protocol.
 *
 * Revision 1.21  1993/12/15  21:02:42  hollings
 * added PVM support.
 *
 * Revision 1.20  1993/12/13  19:53:15  hollings
 * added code to count ptrace operations.
 *
 * Revision 1.19  1993/11/01  22:50:32  hollings
 * changed io tags.
 *
 * Revision 1.18  1993/10/19  19:01:48  hollings
 * added inst to func cmcn_sleep.
 *
 * Revision 1.17  1993/10/19  15:27:54  hollings
 * AST based mini-tramp code generator.
 *
 * Revision 1.16  1993/10/07  19:22:50  jcargill
 * Added true combines for global instrumentation
 *
 * Revision 1.15  1993/10/04  21:36:50  hollings
 * removed read/write from XPU state change list for now.
 *
 * Revision 1.14  1993/10/01  21:29:41  hollings
 * Added resource discovery and filters.
 *
 * Revision 1.13  1993/09/10  20:33:20  hollings
 * corrected bug in numbering of CM-5 processes.
 *
 * Revision 1.12  1993/09/03  18:36:16  hollings
 * removed extra printfs.
 *
 * Revision 1.11  1993/09/03  15:45:43  jcargill
 * Include style change for ptrace_emul.h
 *
 * Revision 1.10  1993/08/30  18:25:22  hollings
 * small fixes in DUMPCORE
 *
 * Revision 1.9  1993/08/25  20:48:20  jcargill
 * Ptrace emulation stuff added
 *
 * Revision 1.8  1993/08/25  20:16:01  hollings
 * added fork support.
 * changed stopping a process to STOP from STP.
 *
 * Revision 1.7  1993/08/11  02:01:55  hollings
 * added predicted cost model.
 *
 * Revision 1.6  1993/07/13  18:27:15  hollings
 * new include file syntax.
 *
 * Revision 1.5  1993/06/25  22:23:28  hollings
 * added parent field to process.h
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
#include <assert.h>
#include <sys/param.h>
#include <sys/wait.h>
#include <sys/errno.h>
#include <machine/reg.h>

extern "C" {
#include <sys/unistd.h>
#include <cmsys/cm_ptrace.h>
#include <sys/ptrace.h>
int CMTS_ConnectToDaemon();
int ptrace();
}

#include "dyninst.h"
#include "rtinst/h/trace.h"
#include "symtab.h"
#include "process.h"
#include "inst.h"
#include "instP.h"
#include "ast.h"
#include "ptrace_emul.h"
#include "util.h"

libraryList msgFilterFunctions;
libraryList msgByteSentFunctions;
libraryList msgByteRecvFunctions;
libraryList msgByteFunctions;
libraryList fileByteFunctions;
libraryList libraryFunctions;

process *nodePseudoProcess;

/* Prototypes */
int emulatePtraceRequest (int request, process *proc, int cmPid, void *addr, 
		int data, void *addr2);

void addLibFunc(libraryList *list, char *name, int arg)
{
    libraryFunc *temp = new libraryFunc(name, arg);
    list->add(temp, (void *) temp->name);
}

/*
 * Define the various classes of library functions to inst. 
 *
 */
void initLibraryFunctions()
{
    addLibFunc(&msgByteSentFunctions, "CMPR_m_s_open_for_send", TAG_LIB_FUNC);
    addLibFunc(&msgByteSentFunctions, "CMPR_m_p_open_for_send", TAG_LIB_FUNC);
    addLibFunc(&msgByteRecvFunctions, "CMPR_m_s_open_for_receive",TAG_LIB_FUNC);
    addLibFunc(&msgByteRecvFunctions, "CMPR_m_p_open_for_receive",TAG_LIB_FUNC);

    /* should record waiting time in read/write, but have a conflict with
     *   use of these functions by our inst code.
     *   This happens when a CPUtimer that is stopped is stopped again by the
     *   write.  It is then started again at the end of the write and should
     *   not be running then.  We could let timers go negative, but this
     *   causes a problem when inst is inserted into already running code.
     *   Not sure what the best fix is - jkh 10/4/93
     *
     */
    addLibFunc(&fileByteFunctions, "write", 
		TAG_LIB_FUNC|TAG_IO_FUNC|TAG_CPU_STATE);
    addLibFunc(&fileByteFunctions, "read", 
		TAG_LIB_FUNC|TAG_IO_FUNC|TAG_CPU_STATE);

    addLibFunc(&libraryFunctions, "DYNINSTsampleValues", TAG_LIB_FUNC);
    addLibFunc(&libraryFunctions, "cmmd_debug", TAG_LIB_FUNC);
    addLibFunc(&libraryFunctions, "CMMD_init", TAG_LIB_FUNC);
    addLibFunc(&libraryFunctions, "exit", TAG_LIB_FUNC);
    addLibFunc(&libraryFunctions, "fork", TAG_LIB_FUNC);
    addLibFunc(&libraryFunctions, "main", 0);

    addLibFunc(&msgFilterFunctions, "CMMD_send", 
	TAG_LIB_FUNC|TAG_MSG_FUNC|TAG_CPU_STATE); 
    addLibFunc(&msgFilterFunctions, "CMMD_receive",
	TAG_LIB_FUNC|TAG_MSG_FUNC|TAG_CPU_STATE); 
    addLibFunc(&msgFilterFunctions, "CMMP_receive_block", 
	TAG_LIB_FUNC|TAG_MSG_FUNC|TAG_CPU_STATE); 
    addLibFunc(&msgFilterFunctions, "CMMP_send_block", 
	TAG_LIB_FUNC|TAG_MSG_FUNC|TAG_CPU_STATE); 
    addLibFunc(&msgFilterFunctions, "CMMP_send_async", 
	TAG_LIB_FUNC|TAG_MSG_FUNC|TAG_CPU_STATE); 
    addLibFunc(&msgFilterFunctions, "CMMP_receive_async", 
	TAG_LIB_FUNC|TAG_MSG_FUNC|TAG_CPU_STATE); 

    addLibFunc(&libraryFunctions, "CMMP_all_msgs_wait", 
	TAG_LIB_FUNC|TAG_MSG_FUNC|TAG_CPU_STATE); 
    addLibFunc(&libraryFunctions, "CMMP_msg_wait", 
	TAG_LIB_FUNC|TAG_MSG_FUNC|TAG_CPU_STATE);

    /* reduction library functions */
    addLibFunc(&libraryFunctions, "CMMD_reduce_int", 
	TAG_LIB_FUNC|TAG_MSG_FUNC|TAG_CPU_STATE);
    addLibFunc(&libraryFunctions, "CMMD_reduce_uint", 
	TAG_LIB_FUNC|TAG_MSG_FUNC|TAG_CPU_STATE);
    addLibFunc(&libraryFunctions, "CMMD_reduce_float", 
	TAG_LIB_FUNC|TAG_MSG_FUNC|TAG_CPU_STATE);
    addLibFunc(&libraryFunctions, "CMMD_reduce_double", 
	TAG_LIB_FUNC|TAG_MSG_FUNC|TAG_CPU_STATE);
    addLibFunc(&libraryFunctions, "CMMD_reduce_v", 
	TAG_LIB_FUNC|TAG_MSG_FUNC|TAG_CPU_STATE);

    /* scan operations */
    addLibFunc(&libraryFunctions, "CMMD_scan_int", 
	TAG_LIB_FUNC|TAG_MSG_FUNC|TAG_CPU_STATE);
    addLibFunc(&libraryFunctions, "CMMD_scan_uint", 
	TAG_LIB_FUNC|TAG_MSG_FUNC|TAG_CPU_STATE);
    addLibFunc(&libraryFunctions, "CMMD_scan_float", 
	TAG_LIB_FUNC|TAG_MSG_FUNC|TAG_CPU_STATE);
    addLibFunc(&libraryFunctions, "CMMD_scan_double", 
	TAG_LIB_FUNC|TAG_MSG_FUNC|TAG_CPU_STATE);
    addLibFunc(&libraryFunctions, "CMMD_scan_v", 
	TAG_LIB_FUNC|TAG_MSG_FUNC|TAG_CPU_STATE);

    /* global sync functions */
    addLibFunc(&libraryFunctions, "CMMD_sync_with_nodes", 
	TAG_LIB_FUNC|TAG_SYNC_FUNC|TAG_CPU_STATE);

    /* broadcast functions */
    addLibFunc(&libraryFunctions, "CMMD_bc_to_nodes", 
	TAG_LIB_FUNC|TAG_SYNC_FUNC|TAG_CPU_STATE);
    addLibFunc(&libraryFunctions, "CMMD_receive_bc_from_node", 
	TAG_LIB_FUNC|TAG_SYNC_FUNC|TAG_CPU_STATE);

    /* un-used node */
    addLibFunc(&libraryFunctions, "cmcn_sleep", 
	TAG_LIB_FUNC|TAG_CPU_STATE);

    libraryFunctions += fileByteFunctions;
    libraryFunctions += msgByteSentFunctions;
    libraryFunctions += msgByteRecvFunctions;
    libraryFunctions += msgFilterFunctions;

    msgByteFunctions += msgByteSentFunctions;
    msgByteFunctions += msgByteRecvFunctions;

}

/* have we connected to the TS-deamon */
static int tsdConnected;

static AstNode tagArg(Param, (void *) 2);

instMaping defaultInst[] = {
    { "main", "DYNINSTinit", FUNC_ENTRY },
    { "main", "DYNINSTsampleValues", FUNC_EXIT },
    { "exit", "DYNINSTsampleValues", FUNC_ENTRY },
    { "DYNINSTsampleValues", "DYNINSTreportNewTags", FUNC_ENTRY },
    { "CMMD_send", "DYNINSTrecordTag", FUNC_ENTRY|FUNC_ARG, &tagArg },
    { "CMMD_receive", "DYNINSTrecordTag", FUNC_ENTRY|FUNC_ARG, &tagArg},
    { "CMMP_receive_block", "DYNINSTrecordTag", FUNC_ENTRY|FUNC_ARG, &tagArg },
    { "CMMP_send_block", "DYNINSTrecordTag", FUNC_ENTRY|FUNC_ARG, &tagArg },
    { "CMMP_send_async", "DYNINSTrecordTag", FUNC_ENTRY|FUNC_ARG, &tagArg },
    { "CMMP_receive_async", "DYNINSTrecordTag", FUNC_ENTRY|FUNC_ARG, &tagArg },
    { NULL, NULL, 0},
};

process *createGlobalPseudoProcess(process *sampleNodeProc)
{
    process *ret;

    ret = (process *) xcalloc(sizeof(process),1);
    ret->symbols = sampleNodeProc->symbols;
    ret->aggregate = True;
    ret->pid = -1;
    ret->parent = sampleNodeProc->parent;
    initInferiorHeap(ret, True);

    return(ret);
}


void forkNodeProcesses(process *curr, traceHeader *hr, traceFork *fr)
{
    int i;
    int val;
    int pid;
    int callee;
    int status;
    process *ret;
    char name[80];
    process *parent;
    struct regs nodeRegisters;
    struct regs newRegisters;

    parent = findProcess(fr->ppid);
    assert(parent);

    /* make sure we have a TS-deamon connection */
    if (!tsdConnected) {
	tsdConnected = 1;
	if (CMTS_ConnectToDaemon() == -1) {
	    printf("Can't connect to TS Daemon\n");
	    free(ret);
	    return;
	}
    }

    /* attach to the process */
    val = CM_ptrace(0, PE_PTRACE_TRACEPID, fr->pid%MAXPID, 0, 0, 0);
    if (val != 0) {
	perror("nptrace");
	free(ret);
	return;
    }

#define TMC_SCHED_BUG_KLUDGE
#ifdef TMC_SCHED_BUG_KLUDGE

/*
 * This section of code is a kludge to get around the odd behavior of the
 * TMC's scheduler.  When the CP process is blocked for any reason, and 
 * doesn't reach the cm5_stopped_state (wait channel) in the kernel, the 
 * nodes never get scheduled if anyone else is using the partition.
 *
 * This caused a deadlock for us, during the processing of the TR_MULTI_FORK
 * record.  The state of the deadlock is as follows:
 *   - the CP program has reached a breakpoint, after the production of the
 *     TR_MULTI_FORK record.  The process tries to enter the breakpoint by
 *     sending itself a SIGSTOP.  The controller should receive the SIGSTOP,
 *     and forward a SIGPROF to the CP program, to force it into the
 *     spin-loop in DYNINSTpauseProcess.
 *
 *   - the "controller" process is busy processing the TR_MULTI_FORK record,
 *     which involves forcing each node to call an init routine.  So the 
 *     controller never returns to the controllerMainloop, where it would
 *     notice the SIGSTOP and pause the CP program.
 *
 *   - the nodes work at the init code for a while.  However, when the
 *     timeslice ends, the nodes never get rescheduled, since the CP 
 *
 * So to summarize: The CP is blocked in the kernel, so that nodes
 * never get scheduled to run, so they never complete the
 * initialization code, so they the controller never gets to continue,
 * and thus it can't forward the * SIGPROF to the CP to get it
 * started.  QED
 *
 * Solution for now is to wait for the CP to hit the breakpoint, and forward
 * the signal *before* we do any of the ptrace stuff to force the nodes to
 * init.
 */

retry:
    /*
     * Wait for the change of status only from the "interested party"
     * CP process.
     */
    pid = waitpid(fr->ppid, &status, NULL);

    /* make sure the process stopped.  What else could it reasonable do? */
    if (! WIFSTOPPED(status))	
      abort();
    
    switch (WSTOPSIG(status)) {
	case SIGALRM:
	    /* We might also get a sig alaram in here since there is a race 
	     * between the alaram handler and the stop.
	     * We just drop the alaram, and then go about our business.
	     * JKH - 8/9/93
	     */
	     ptrace(PTRACE_CONT, pid, (char*)1, SIGCONT, 0);
	     printf("got sigalarm eating it\n");
	     goto retry;

	case SIGSTOP:
	  printf("CONTROLLER: Breakpoint reached, continuing for node init\n");
	  curr->status = stopped;
	  /* force it into the stoped signal handler */
	  ptrace(PTRACE_CONT, pid, (char*)1, SIGPROF, 0);

	  /* pause the rest of the application */
	  /*
	   * or, that is, we would, but then the nodes wouldn't be able to run
	   * our initializaiton code...  Perhaps we should pauseApplication at
	   * the end of this routine, instead...
	   */
	  /*       pauseApplication(curr->appl); */   
	  break;

	default:
	  printf("CP process reached a non-stopped state (signal %d). Weird.\n",
	      WSTOPSIG(status));
	  abort();
    }

/*
 * Everything should be hunky-dory now.  We shouldn't deadlock, since
 * the CP will spin.  Now we can get back to init-ing the nodes.
 */

#endif  /* TMC_SCHED_BUG_KLUDGE */


    for (i=0; i < fr->npids; i++) {
	int newpid;

	sprintf(name, "%s.pn[%d]", parent->symbols->name, i);
	newpid = fr->pid + i * fr->stride;

	ret = allocateProcess(newpid, name);

	ret->pid = newpid;
	ret->symbols = parseImage(parent->symbols->file, 1);
	ret->traceLink = -1;
	ret->parent = parent;
	initInferiorHeap(ret, False);
    }

    /*
     * Get nodes to call DYNINSTinit.
     *
     */
    printf("forcing node inits\n");
    for (i=0; i < fr->npids; i++) {
	val = CM_ptrace(i, PE_PTRACE_INTERRUPT, fr->ppid, 0, 0, 0);
	if (val) {
	    printf("CM_ptrace: pn%d error PE_PTRACE_INTERRUPT\n",i);
	    exit(-1);
	}
	printf(".");
	fflush(stdout);
    }
    for (i=0; i < fr->npids; i++) {
	val = CM_ptrace(i, PE_PTRACE_GETREGS, fr->ppid, 
	    (char *) &nodeRegisters, 0, 0);
	if (val) {
	    printf("CM_ptrace: pn%d error PE_PTRACE_GETREGS\n",i);
	    exit(-1);
	}

	/* fix up regs */
	newRegisters = nodeRegisters;
	newRegisters.r_o7 = newRegisters.r_npc;
	callee = findInternalAddress(ret->symbols, "DYNINSTinitTraceLib", True);
	newRegisters.r_npc = callee;

	val = CM_ptrace(i, PE_PTRACE_SETREGS, fr->ppid, 
	    (char *) &newRegisters, 0, 0);
	if (val) {
	    printf("CM_ptrace: pn%d error PE_PTRACE_SETREGS\n",i);
	    exit(-1);
	}
	val = CM_ptrace(i, PE_PTRACE_CONT, fr->ppid, 0, 0, 0);
	if (val) {
	    printf("CM_ptrace: pn%d error PE_PTRACE_CONT\n",i);
	    exit(-1);
	}
	printf(".");
	fflush(stdout);
    }
    for (i=0; i < fr->npids; i++) {
	/* wait till it stops */
	val = 0;
	while (val != PE_STATUS_BREAK) {
	    val = CM_ptrace(i, PE_PTRACE_GETSTATUS, fr->ppid, 0, 0, 0);
	}
	// printf("process %d got break after init\n", i);

	val = CM_ptrace(i, PE_PTRACE_INTERRUPT, fr->ppid, 0, 0, 0);
	if (val) {
	    printf("CM_ptrace: pn%d error PE_PTRACE_INTERRUPT\n",i);
	    exit(-1);
	}

	val = CM_ptrace(i, PE_PTRACE_SETREGS, fr->ppid, 
	    (char *) &nodeRegisters, 0, 0);
	if (val) {
	    printf("CM_ptrace: pn%d error PE_PTRACE_SETREGS\n",i);
	    exit(-1);
	}
	val = CM_ptrace(i, PE_PTRACE_CONT, fr->ppid, 0, 0, 0);
	if (val) {
	    printf("CM_ptrace: pn%d error PE_PTRACE_CONT\n",i);
	    exit(-1);
	}

	printf(".");
	fflush(stdout);
	// printf("process %d was continued after init\n", i);
    }
    printf("node initing done\n");
    fflush(stdout);
    
    nodePseudoProcess = createGlobalPseudoProcess(ret);

    /*
     * install default instrumentation.
     *
     */
    installDefaultInst(nodePseudoProcess, defaultInst);

    pauseApplication();
    
}

static int buffered;

#ifdef notdef

static int errnos[DBG_MAX_BATCH_SIZE];
static int results[DBG_MAX_BATCH_SIZE];
static struct cmos_ptrace_req requests[DBG_MAX_BATCH_SIZE];



/*
 * flushPtrace was replace by a version which waits for an ACK from the
 * timeslice-handler...
 */

int flushPtrace()
{
    int ret;
    int scalarPid;
    extern int errno;

    if (buffered == 1) {
	errno = 0;
	ret = CM_ptrace(requests[0].pe, requests[0].request, requests[0].pid, 
	    requests[0].addr, requests[0].data, requests[0].addr2);
	if (errno) {
	    perror("read");
	    abort();
	}
	buffered = 0;
    } else if (buffered) {
	errno = 0;
	scalarPid = requests[0].pid;
	ret = CMOS_ptrace_batch(scalarPid, buffered, requests, 
	    results, errnos);
	if (errno) { 
	    perror("readBatch");
	    for (ret=0; ret < buffered; ret++) {
		if (errnos[ret]) {
		    abort();
		}
	    }
	    exit(-1);
	}
	ret = results[buffered-1];
	buffered = 0;
    }
    return(ret);
}
#endif


/*
 * OLD OUTDATED comment:  (but still worth keeping in mind)
 *
 * Some global variables, needed for the ptrace protocol.  They maintain a
 * sequence number per ptrace request, and maintain a sliding window
 * protocol, so that the controller won't block writing to the handlers.
 * This blocking is a bad thing, since it can lead to a deadlock.
 *
 * The deadlock is as follows: The controller is blocked, waiting for
 * the handler to read some data.  The handler can't read data until
 * the controller forwards the signal which causes handlers to fire.
 * The controller will never forward it, because it's blocked.  QED.
 * 
 * The solution is to never have more than window_threshold bytes of
 * ptrace reqeusts outstanding on the socket.  To do this, we call
 * checkInferiorProcs(), so that signals will be forwarded, and
 * handlers will fire.  We also call pollInferiorProcs, so that we
 * will read some ACKs and move the window.
 *
 * UPDATED comment:  (this is current)
 * 
 * After some more thought, the sliding window was discarded, and
 * synchronous protocol between the instrumentation controller and the
 * handlers was put in instead.  This way we know that when we're
 * writing ptrace data, and handlers is reading it, and when the
 * handler is writing samples to the pipe, we won't ever block writing
 * ptrace requests.  This seems much cleaner.  --jmc
 */


/* Some of this should really be one per cm5 process...  XXXXX */

static int seqNumSent = 0;
static int seqNumAcked = 0;
static int ptraceBufferSize = 0;
static char *ptraceBuffer = NULL;
static int ptraceBufferOffset = 0;

/*
 * New version, which waits for an ACK from the handler.  Doesn't
 * actually wait at this point, since we can't read from the tracePipe
 * and handle signals if we block here.  And reading from the trace
 * pipe is not renetrant, since processTraceStream buffers data
 * itself.
 *
 * At some point we should think about this...   XXX   
 */

int flushPtrace()
{

/*     while (seqNumSent > seqNumAcked) */
	/* Check for data on any of the inferior processes trace streams */
/* 	pollInferiorProcs(); */

  return 0;
}


void processPtraceAck (traceHeader *header, ptraceAck *ackRecord)
{
    seqNumAcked = ackRecord->seqNumber;
    
    assert(seqNumAcked >= 0);
    assert(seqNumAcked <= seqNumSent);
    printf ("received Ptrace ack (seq #%d)\n", seqNumAcked);
}


extern int ptraceOtherOps, ptraceOps, ptraceBytes;

/* 
 * The performance consultant's ptrace, it calls CM_ptrace and ptrace as needed.
 *
 */
int PCptrace(int request, process *proc, void *addr, int data, void *addr2)
{
    int ret;
    int code;
    int cmPid;
    int scalarPid;
    struct regs regs;

    /* hack for node processes */
    if (request == PTRACE_INTERRUPT) {
	if (proc->pid < MAXPID) {
	    /* pause the process in the signal handler */
	    request = PTRACE_CONT;
	    data = SIGPROF;
	} else {
	    request = PTRACE_INTERRUPT;
	    data = 0;
	}
    } else if (request == PTRACE_CONT) {
	if (proc->pid < MAXPID) {
	    /* get the process out of the pause signal handler */
	    data = SIGUSR1;
	} else {
	    data = 0;
	}
    }

    if (proc->status == exited) {
	printf("attempt to ptrace exited process %d\n", proc->pid);
	return(-1);
    }

    /* stats */
    ptraceOps++;
    if (request == PTRACE_WRITEDATA) 
	ptraceBytes += data;
    else if (request == PTRACE_POKETEXT) 
	ptraceBytes += sizeof(int);
    else 
	ptraceOtherOps++;


    if (proc == nodePseudoProcess) {
	// do a broadcast ptrace.
	emulatePtraceRequest (request, proc, 0xffffffff, addr, data, addr2);
    } else if (proc->pid < MAXPID) {
	/* normal process */
	int sig;
	int status;
	int stopped;
	extern int errno;

	if (proc->status != neonatal && 
	    request != PTRACE_DUMPCORE) {
	    /* make sure the process is stopped in the eyes of ptrace */
	    kill(proc->pid, SIGSTOP);
	    stopped = 0;
	    while (!stopped) {
		ret = waitpid(proc->pid, &status, WUNTRACED);
		if ((ret == -1 && errno == ECHILD) || (WIFEXITED(status))) {
		    // the child is gone.
		    proc->status = exited;
		    return(0);
		}
		if (!WIFSTOPPED(status) && !WIFSIGNALED(status)) {
		    printf("problem stopping process\n");
		    abort();
		}
		sig = WSTOPSIG(status);
		if (sig == SIGSTOP) {
		    stopped = 1;
		} else {
		    ptrace(PTRACE_CONT, proc->pid,(char*)1, WSTOPSIG(status),0);
		}
	    }
	}
	/* INTERRUPT is pseudo request to stop a process. prev lines do this */
	if (request == PTRACE_INTERRUPT) return(0);
	errno = 0;
	ret = ptrace(request, proc->pid,(char*) addr, data, (char*) addr2);
	assert(errno == 0);
	if ((proc->status != neonatal) && (request != PTRACE_CONT)) {
	    (void) ptrace(PTRACE_CONT, proc->pid,(char*) 1, SIGCONT, (char*) 0);
	}
	errno = 0;
	return(ret);
    } else {
	cmPid = (proc->pid / MAXPID) - 1;
	scalarPid = proc->pid % MAXPID;

	switch (request) {
	    case PTRACE_INTERRUPT:
		code = PE_PTRACE_INTERRUPT;
		break;

	    case PTRACE_DUMPCORE:
		code = PE_PTRACE_DUMPCORE;
		break;

	    case PTRACE_CONT:
		/* need to advance pc past break point if at one */
		code = CM_ptrace(cmPid, PE_PTRACE_GETSTATUS, scalarPid,0,0,0);
		if (code == PE_STATUS_BREAK) {
		    code = CM_ptrace(cmPid, PE_PTRACE_GETREGS, 
			scalarPid, (char *) &regs,0,0);
		    regs.r_pc += 4;
		    code = CM_ptrace(cmPid, PE_PTRACE_SETREGS, scalarPid,
			(char *) &regs,0,0);
		}
		code = PE_PTRACE_CONT;
		addr = 0;
		data = 0;
		addr2 = 0;
		break;

	    case PTRACE_READDATA:
		// only used for debuging.
		code = PE_PTRACE_READDATA;
		break;

	    case PTRACE_POKETEXT:
		code = PE_PTRACE_POKETEXT;
		return (emulatePtraceRequest (request, proc, cmPid, 
					      addr, data, addr2));
		break;

	    case PTRACE_WRITEDATA:
		code = PE_PTRACE_WRITEDATA;
		return (emulatePtraceRequest (request, proc, cmPid, 
					      addr, data, addr2));
		break;

	    case PTRACE_POKEDATA:
		code = PE_PTRACE_POKEDATA;
		return (emulatePtraceRequest (request, proc, cmPid, 
					      addr, data, addr2));
		break;

	    case PTRACE_DETACH:
		return(0);

	    default:
		abort();
		break;
	}

	return (CM_ptrace(cmPid, code, scalarPid, (char *) addr, 
			  data, (char *) addr2));
    }
    return(0);
}


/*
 * Ptrace emulation section.  We send packets of "Modify memory"
 * requests to the instrumentation controller.  The format of a
 * request is as follows:
 *
 * [ request ] - modify memory request.  Currently the only request.
 * [ req id  ] - Uniq id for purposes of Acknowledgement.
 * [ node id ] - Node id:  0-(partition_size-1) for unique node.  
 *                         0xffffffff to broadcast to all nodes in partition
 * [ address ] - Memory address to modify
 * [ length  ] - Number of words of data to write
 * [ data    ]
 * [ ...     ]
 * [ data    ]
 *
 * Everything between brackets above ([...]) is one word in length.
 * Since the instrumentation controller is running on the front-end
 * (so it can really use ptrace to control the application), we can
 * assume no byte-order problems.
 *
 *
 */

/*
 * This routine buffers ptrace requests, which will be sent to the
 * timeslice handler when it is ready for them.
 */

int emulatePtraceRequest (int request, process *proc, int cmPid, void *addr, 
		int data, void *addr2)
{
    int sock;
    ptraceReqHeader header;

    process *tempProc;
    char *sourceAddr;		/* source of the data to send */

    /*
     * Find the appropriate socket to write to.  We traverse up the parent
     * links until we find a NULL parent ptr.
     */
    tempProc = proc;
    while (tempProc->parent != NULL)
	tempProc = tempProc->parent;
    sock = tempProc->traceLink;

    /*
     * If this is a first ptrace request (none buffered), then we
     * should notify the handler, so it can get ready for the next
     * batch.
     */
    if (ptraceBufferOffset == 0) {
	/* Send a "ptrace data available" message */

	header.request = PTRACE_REQUESTS_AVAILABLE;
	write (sock, (char *) &header, sizeof (header));
    }

    /* Create the request header */
    header.request = MODIFY_TEXT;
    header.req_id = 0;
    header.dest = cmPid;
    header.address = (char *) addr;

    switch (request) {
    case PTRACE_POKETEXT:
    case PTRACE_POKEDATA:
	header.length = sizeof (int);
	sourceAddr = (char *) &data;
	break;
    case PTRACE_WRITEDATA:
	header.length = data;
	sourceAddr = (char *) addr2;
	break;
    }  

#ifdef DEBUG_PRINTS
    printf ("Buffering (%d, %d, %d)\n", cmPid, addr, header.length);
#endif
   
    /*
     * Check whether the ptrace-data buffer has enough room for the
     * current request.
     */
    while (sizeof(header)+header.length > ptraceBufferSize-ptraceBufferOffset) 
    {
	/* buffer is full or does not exist.  Realloc it. */

	if (ptraceBuffer == NULL) {
	    ptraceBufferSize = 1024 * 1024;
	    ptraceBuffer = (char *) malloc(ptraceBufferSize);
	}
	else {
	    
	    ptraceBufferSize *= 2;
	    ptraceBuffer = (char *) realloc (ptraceBuffer, ptraceBufferSize);
	}

	if (ptraceBuffer == NULL) 
	    abort();
    }
	

    /* Save the request header */
    memcpy (&ptraceBuffer[ptraceBufferOffset], 
	    (char *) &header, sizeof (header));
    ptraceBufferOffset += sizeof(header);
    
    /* Save the new text. */
    memcpy (&ptraceBuffer[ptraceBufferOffset], 
	    (char *) sourceAddr, header.length);
    ptraceBufferOffset += header.length;

    return 0;			/* WHAT SHOULD WE RETURN?  XXX */
}


void sendPtraceBuffer(process *proc)
{
    ptraceReqHeader header;
    process *tempProc;
    int sock;

    /*
     * Find the appropriate socket to write to.  We traverse up the parent
     * links until we find a NULL parent ptr.
     */
    tempProc = proc;
    while (tempProc->parent != NULL)
	tempProc = tempProc->parent;
    sock = tempProc->traceLink;

    /* Send the buffered Ptrace data. */
    if (write (sock, ptraceBuffer, ptraceBufferOffset) != ptraceBufferOffset)
	perror ("write didn't send all");

    /* Indicate that no more ptrace requests are coming */
    header.req_id = seqNumSent++;
    header.length = ptraceBufferOffset;
    header.request = PTRACE_REQUESTS_DONE;
    write (sock, (char *) &header, sizeof (header));

    /* Free up the ptraceBuffer */
    ptraceBufferOffset = 0;

}

#define NS_TO_SEC	1000000000.0

StringList<int> primitiveCosts;

void initPrimitiveCost()
{
    /* based on measured values for the CM-5. */
    /* Need to add code here to collect values for other machines */
    primitiveCosts.add(728, (void *) "DYNINSTincrementCounter");
    primitiveCosts.add(728, (void *) "DYNINSTdecrementCounter");
    primitiveCosts.add(1159, (void *) "DYNINSTstartWallTimer");
    primitiveCosts.add(1939, (void *) "DYNINSTstopWallTimer");
    primitiveCosts.add(1296, (void *) "DYNINSTstartProcessTimer");
    primitiveCosts.add(2365, (void *) "DYNINSTstopProcessTimer");
}

/*
 * return the time required to execute the passed primitive.
 *
 */
float getPrimitiveCost(char *name)
{
    float ret;

    ret = primitiveCosts.find(name)/NS_TO_SEC;
    if (ret == 0.0) {
	printf("no cost value for primitive %s, using 10 usec\n", name);
	ret = 10000/NS_TO_SEC;
    }
    ret = 0.0;
    return(ret);
}

/*
 * Not required on this platform.
 *
 */
void instCleanup()
{
}

char *getProcessStatus(process *proc)
{
    int val;
    static char ret[80];

    if (proc->pid > MAXPID) {
        val = CM_ptrace((proc->pid / MAXPID) - 1, PE_PTRACE_GETSTATUS,
            proc->pid % MAXPID, 0, 0, 0);
        if (val == PE_STATUS_RUNNING)
            return("state = PE_STATUS_RUNNING");
        else if (val == PE_STATUS_BREAK)
            return("state = PE_STATUS_BREAK");
        else if (val == PE_STATUS_ERROR)
            return("state = PE_STATUS_ERROR");
        else {
            sprintf(ret, "state = %d\n", val);
	    return(ret);
	}
    } else {
	switch (proc->status) {
	    case running:
		sprintf(ret, "%d running", proc->pid);
		break;
	    case neonatal:
		sprintf(ret, "%d neonatal", proc->pid);
		break;
	    case stopped:
		sprintf(ret, "%d stopped", proc->pid);
		break;
	    case exited:
		sprintf(ret, "%d exited", proc->pid);
		break;
	    default:
		sprintf(ret, "%d UNKNOWN State", proc->pid);
		break;
	}
	return(ret);
    }
}
