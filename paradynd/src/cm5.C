
/*
 * $Log: cm5.C,v $
 * Revision 1.2  1995/01/26 18:11:50  jcargill
 * Updated igen-generated includes to new naming convention
 *
 * Revision 1.1  1994/11/01  16:49:25  markc
 * Initial files that will provide os support.  This should limit os
 * specific features to these files.
 *
 */

#include "os.h"
#include "util/h/kludges.h"

extern "C" {
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/param.h>
#include <sys/wait.h>
#include <sys/errno.h>
#include <machine/reg.h>
#include <cm/cmmd.h>
}

#include "dyninst.h"
#include "rtinst/h/trace.h"
#include "symtab.h"
#include "process.h"
#include "inst.h"
#include "instP.h"
#include "ast.h"
#include "dyninstRPC.xdr.SRVR.h"
#include "util.h"
#include "stats.h"
#include "ptrace_emul.h"

int inNodePtrace = 0;

/* Prototypes */
int nodePtrace (enum ptracereq request, process *proc, int scalarPid,
		int nodeId, void *addr, int data, void *addr2);

bool osAttach(int process_id) {
  return (!ptrace(PTRACE_ATTACH, process_id, 0, 0, 0));
}

bool osStop(int pid) {
  return (!kill(pid, SIGSTOP));
}

bool osDumpCore( int pid,  char *fileTo) {
  return (!ptrace(PTRACE_DUMPCORE, pid, "core.real", 0, 0));
}

bool osForwardSignal ( int pid,  int stat) {
  return (!ptrace(PTRACE_CONT, pid, (char*)1, stat, 0));
}

bool osDumpImage(const string &imageFileName,  int pid, const Address off) {
  logLine("dumpcore not yet available\n");
  return false;
}

void osTraceMe() {
  ptrace(PTRACE_TRACEME, 0, 0, 0, 0);
}

/* 
 * The performance consultant's ptrace, it calls CM_ptrace and ptrace as needed.
 *
 */
int PCptrace(int request, process *proc, char *addr, int data, char *addr2)
{
    int ret;
//    int code;
    int cmPid;
    int scalarPid;
//    struct regs regs;
    extern int errno;

    errno = 0;

    if (proc->status == exited) {
	sprintf (errorLine, 
		 "attempt to ptrace exited process %d\n", proc->pid);
	logLine(errorLine);
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
	nodePtrace (request, proc, proc->pid%MAXPID, 0xffffffff, addr, 
		    data, addr2);
    } else if (proc->pid < MAXPID) {
	/* normal process */
	int sig;
	int status;
	int isStopped, wasStopped;

	wasStopped = (proc->status == stopped);
	if (proc->status != neonatal && !wasStopped &&
	    request != PTRACE_DUMPCORE) {
	    /* make sure the process is stopped in the eyes of ptrace */
	    kill(proc->pid, SIGSTOP);
	    isStopped = 0;
	    while (!isStopped) {
		ret = waitpid(proc->pid, &status, WUNTRACED);
		if ((ret == -1 && errno == ECHILD) || (WIFEXITED(status))) {
		    // the child is gone.
		    proc->status = exited;
		    return(0);
		}
		if (!WIFSTOPPED(status) && !WIFSIGNALED(status)) {
		    logLine("problem stopping process\n");
		    abort();
		}
		sig = WSTOPSIG(status);
		if (sig == SIGSTOP) {
		    isStopped = 1;
		} else {
		    sprintf(errorLine, "PCptrace forwarded signal %d while waiting for stop\n",
			    WSTOPSIG(status));
		    logLine(errorLine);
		    ptrace(PTRACE_CONT, proc->pid,(char*)1, WSTOPSIG(status),0);
		}
	    }
	}
	/* INTERRUPT is pseudo request to stop a process. prev lines do this */
	if (request == PTRACE_INTERRUPT) return(0);
	errno = 0;
	ret = ptrace(request, proc->pid,(char*) addr, data, (char*) addr2);
	assert(errno == 0);

	if ((proc->status != neonatal) && (request != PTRACE_CONT) &&
	    (!wasStopped)) {
	    (void) ptrace(PTRACE_CONT, proc->pid,(char*) 1, 0, (char*) 0);
	}
	errno = 0;
	return(ret);
    } else {
	cmPid = (proc->pid / MAXPID) - 1;
	scalarPid = proc->pid % MAXPID;

	/*
	 * Keeping this continue code around for a while, since we may need 
         * to do the same type of thing on the nodes in the future...
	 */

//	    case PTRACE_CONT:
//		/* need to advance pc past break point if at one */
//		code = CM_ptrace(cmPid, PE_PTRACE_GETSTATUS, scalarPid,0,0,0);
//		if (code == PE_STATUS_BREAK) {
//		    code = CM_ptrace(cmPid, PE_PTRACE_GETREGS, 
//			scalarPid, (char *) &regs,0,0);
//		    regs.r_pc += 4;
//		    code = CM_ptrace(cmPid, PE_PTRACE_SETREGS, scalarPid,
//			(char *) &regs,0,0);
//		}

	return (nodePtrace (request, proc, proc->pid%MAXPID, cmPid, addr, 
			    data, addr2));
    }
    return(0);
}

extern int pendingSampleNodes;
extern void sampleNodes();

/*
 * This routine send ptrace requests via CMMD to the nodes for processing.
 */
int nodePtrace (enum ptracereq request, process *proc, int scalarPid, int nodeId, 
		void *addr, int data, void *addr2)
{
    ptraceReqHeader header;
    extern int errno;

    inNodePtrace = 1;

#ifdef notdef
    {
      /* Check for node I/O */
      int i;
      for (i=0; i< 1000; i++)
	while (CMMD_poll_for_services() == 1)
	  ;			/* TEMPORARY:    XXXXXX */
    }
#endif

    /* Create the request header */
    header.request = request;
    header.pid = scalarPid;
    header.nodeNum = nodeId;
    header.addr = (char *) addr;
    header.data = data;
    header.addr2 = (char *) addr2;

    /* Send the ptrace request to the nodes for execution */
    CMMD_bc_from_host (&header, sizeof(header));

    /* Send optional addition data for the request */
    switch (request) {
      case PTRACE_WRITEDATA:
	CMMD_bc_from_host (addr2, data);
	break;
      default:
	// No "optional" data gets sent by default...
	break;
    }  

#ifdef DEBUG_PRINTS
    logLine ("Sent ptrace request (%d, %d, %d)\n", nodeId, addr, header.length);
#endif
   
    inNodePtrace = 0;
#ifdef notdef
    if (pendingSampleNodes)
	sampleNodes();
#endif

    errno = 0;			/* can be hosed by CMMD_poll_for_services() */
    return 0;			/* WHAT SHOULD WE RETURN?  XXX */
}

