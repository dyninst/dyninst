/*
 *  Copyright 1993 Jeff Hollingsworth.  All rights reserved.
 *
 */

#ifndef lint
static char Copyright[] = "@(#) Copyright (c) 1993 Jeff Hollingsowrth\
    All rights reserved.";

static char rcsid[] = "@(#) $Header: /home/jaw/CVSROOT_20081103/CVSROOT/core/paradynd/src/Attic/inst-cm5.C,v 1.10 1994/07/14 23:30:23 hollings Exp $";
#endif

/*
 * inst-cm5.C - runtime library specific files to inst on this machine.
 *
 * $Log: inst-cm5.C,v $
 * Revision 1.10  1994/07/14 23:30:23  hollings
 * Hybrid cost model added.
 *
 * Revision 1.9  1994/07/14  14:41:39  jcargill
 * Major CM5 changes for new ptrace, and new transport.  Removed lots of
 * old/dead code.  Changes to default CMMD instrumentation (to handle
 * optimization)
 *
 * Revision 1.8  1994/07/05  03:26:02  hollings
 * observed cost model
 *
 * Revision 1.7  1994/06/29  02:52:25  hollings
 * Added metricDefs-common.{C,h}
 * Added module level performance data
 * cleanedup types of inferrior addresses instrumentation defintions
 * added firewalls for large branch displacements due to text+data over 2meg.
 * assorted bug fixes.
 *
 * Revision 1.6  1994/06/27  18:56:41  hollings
 * removed printfs.  Now use logLine so it works in the remote case.
 * added internalMetric class.
 * added extra paramter to metric info for aggregation.
 *
 * Revision 1.5  1994/05/18  00:52:27  hollings
 * added ability to gather IO from application processes and forward it to
 * the paradyn proces.
 *
 * Revision 1.4  1994/04/09  18:34:53  hollings
 * Changed {pause,continue}Application to {pause,continue}AllProceses, and
 * made the RPC interfaces use these.  This makes the computation of pause
 * Time correct.
 *
 * Revision 1.3  1994/03/26  20:50:41  jcargill
 * Changed the pause/continue code.  Now it really stops, instead of
 * spin looping.
 *
 * Revision 1.2  1994/03/22  21:03:13  hollings
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
#include <sys/ptrace.h>

#include "dyninst.h"
#include "rtinst/h/trace.h"
#include "symtab.h"
#include "process.h"
#include "inst.h"
#include "instP.h"
#include "ast.h"
#include "ptrace_emul.h"
#include "dyninstRPC.SRVR.h"
#include "util.h"

#include <cm/cmmd.h>

libraryList msgFilterFunctions;
libraryList msgByteSentFunctions;
libraryList msgByteRecvFunctions;
libraryList msgByteFunctions;
libraryList fileByteFunctions;
libraryList libraryFunctions;

/* Prototypes */
int nodePtrace (int request, process *proc, int scalarPid, int nodeId, 
		void *addr, int data, void *addr2);

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
    addLibFunc(&msgByteSentFunctions, "CMMD_send", 
	       TAG_LIB_FUNC|TAG_MSG_FUNC|TAG_CPU_STATE);
    addLibFunc(&msgByteSentFunctions, "CMMD_send_block", TAG_LIB_FUNC);
    addLibFunc(&msgByteSentFunctions, "CMMD_send_noblock", TAG_LIB_FUNC);
    addLibFunc(&msgByteSentFunctions, "CMMD_send_async", TAG_LIB_FUNC);

//    addLibFunc(&msgByteSentFunctions, "CMPR_m_p_open_for_send", 
//		 TAG_LIB_FUNC);
//    addLibFunc(&msgByteSentFunctions, "CMPR_m_s_open_for_send", 
//		 TAG_LIB_FUNC);

    addLibFunc(&msgByteRecvFunctions, "CMMD_receive",
	       TAG_LIB_FUNC|TAG_MSG_FUNC|TAG_CPU_STATE);
    addLibFunc(&msgByteRecvFunctions, "CMMD_receive_block", TAG_LIB_FUNC);
    addLibFunc(&msgByteRecvFunctions, "CMMD_receive_noblock", TAG_LIB_FUNC);
    addLibFunc(&msgByteRecvFunctions, "CMMD_receive_async", TAG_LIB_FUNC);

//    addLibFunc(&msgByteRecvFunctions, "CMPR_m_s_open_for_receive",
//		 TAG_LIB_FUNC);
//    addLibFunc(&msgByteRecvFunctions, "CMPR_m_p_open_for_receive",
//		 TAG_LIB_FUNC);

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

    addLibFunc(&libraryFunctions, "DYNINSTalarmExpire", TAG_LIB_FUNC);
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


extern char *pd_machine;
extern int pd_family;
extern int pd_type;
extern int pd_known_socket;
extern int pd_flag;

void forkNodeProcesses(process *curr, traceHeader *hr, traceFork *fr)
{
    int childPid;
    process *parent;
    char **arg_list;
    char command[80];
    char application[256];
    char app_pid[20];
    char num_nodes[20];	
    char *argv[20];

    parent = findProcess(fr->ppid);
    assert(parent);

    if ((childPid=fork()) == 0) {		/* child */
	/* Build arglist */
	arg_list = RPC_make_arg_list (pd_family, pd_type, 
				      pd_known_socket, pd_flag);
	sprintf (command, "paradyndCM5");
	sprintf (application, "%s", curr->symbols->file);
	sprintf (app_pid, "%d", curr->pid);
	sprintf (num_nodes, "%d", fr->npids);

	/*
	 * It would be nice if this weren't sensitive to the size of
	 * arg_list.  For the moment, only 6 are written by
	 * make_arg_list; this could be cleaner.
	 */
	argv[0] = command;
	argv[1] = application;
	argv[2] = app_pid;
	argv[3] = num_nodes;
	argv[4] = arg_list[1];
	argv[5] = arg_list[2];
	argv[6] = arg_list[3];
	argv[7] = arg_list[4];
	argv[8] = arg_list[5];
	argv[9] = arg_list[6];
	argv[10] = arg_list[7];
	argv[11] = 0;

	ptrace (0, 0, 0, 0, 0);

	execv (command, argv);
    }
    else {			/* parent */
	printf ("forked child process (pid=%d).\n", childPid);
    }

    pauseAllProcesses();
}



extern int ptraceOtherOps, ptraceOps, ptraceBytes;
extern process *nodePseudoProcess;

/* 
 * The performance consultant's ptrace, it calls CM_ptrace and ptrace as needed.
 *
 */
int PCptrace(int request, process *proc, void *addr, int data, void *addr2)
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
		    printf ("PCptrace forwarded signal %d while waiting for stop\n", WSTOPSIG(status));
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


/*
 * This routine send ptrace requests via CMMD to the nodes for processing.
 */
int nodePtrace (int request, process *proc, int scalarPid, int nodeId, 
		void *addr, int data, void *addr2)
{
    ptraceReqHeader header;
    extern int errno;

    /* Check for node I/O */
    int i;

    for (i=0; i< 1000; i++)
	while (CMMD_poll_for_services() == 1)
	    ;			/* TEMPORARY:    XXXXXX */

    /* Create the request header */
    header.request = request;
    header.pid = scalarPid;
    header.nodeNum = nodeId;
    header.addr = (char *) addr;
    header.data = (char *) data;
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
   
    errno = 0;			/* can be hosed by CMMD_poll_for_services() */
    return 0;			/* WHAT SHOULD WE RETURN?  XXX */
}


#define NS_TO_SEC	1000000000.0

#ifdef notdef
-- need to make this truely cm-5 specific!!! not even on front ends.
StringList<int> primitiveCosts;

//
// All costs are based on 30ns clock (~33MHz) and stats reported in the
//   SHPCC paper.
//
void initPrimitiveCost()
{
    /* based on measured values for the CM-5. */
    /* Need to add code here to collect values for other machines */
    primitiveCosts.add(240, (void *) "DYNINSTincrementCounter");
    primitiveCosts.add(240, (void *) "DYNINSTdecrementCounter");
    primitiveCosts.add(1056, (void *) "DYNINSTstartWallTimer");
    primitiveCosts.add(1650, (void *) "DYNINSTstopWallTimer");
    primitiveCosts.add(1221, (void *) "DYNINSTstartProcessTimer");
    primitiveCosts.add(2130, (void *) "DYNINSTstopProcessTimer");
}

/*
 * return the time required to execute the passed primitive.
 *
 */
float getPrimitiveCost(char *name)
{
    float ret;

    ret = primitiveCosts.find(name);
    if (ret == 0.0) {
	sprintf(errorLine, "no cost value for primitive %s, using 10 usec\n", name);
	logLine(errorLine);
	ret = 10000/NS_TO_SEC;
    }
    ret = 0.0;
    return(ret);
}
#endif


/*
 * TEMPORARY:  to get things to compile... -jmc
 *
 */
int getPrimitiveCost(char *name)
{
    int ret;
    ret = 0;
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
    static char ret[80];
    int status, pid;

    printf ("getting status for process proc=%x, pid=%d\n", proc, proc->pid);
    if (proc->pid > MAXPID) {

	PCptrace (PTRACE_STATUS, proc, 0, 0, 0);

//        val = CM_ptrace((proc->pid / MAXPID) - 1, PE_PTRACE_GETSTATUS,
//            proc->pid % MAXPID, 0, 0, 0);
//        if (val == PE_STATUS_RUNNING)
//            return("state = PE_STATUS_RUNNING");
//        else if (val == PE_STATUS_BREAK)
//            return("state = PE_STATUS_BREAK");
//        else if (val == PE_STATUS_ERROR)
//            return("state = PE_STATUS_ERROR");
//        else {
//            sprintf(ret, "state = %d\n", val);
//	    return(ret);
//	}

    } else {
	pid = waitpid (proc->pid, &status, WNOHANG);
	printf("status of real unix process is:  %x  (ret = %d, pid = %d)\n", 
	       status, pid, proc->pid);

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
