
/*
 * inst-sunos.C - sunos specifc code for paradynd.
 *
 * $Log: inst-sunos.C,v $
 * Revision 1.12  1994/08/17 18:11:59  markc
 * Changed the execv to execvp.
 * Changed arglist in forkProcess.
 *
 * Revision 1.11  1994/07/22  19:16:36  hollings
 * moved computePauseTimeMetric here, and added lib func calls for cmmd routines.
 *
 * Revision 1.10  1994/07/15  20:22:03  hollings
 * fixed 64 bit record to be 32 bits.
 *
 * Revision 1.9  1994/07/14  23:30:26  hollings
 * Hybrid cost model added.
 *
 * Revision 1.8  1994/07/12  19:46:57  jcargill
 * Removed old code, added ability for fork paradyndCM5 when nodes start.
 *
 * Revision 1.7  1994/07/05  03:26:04  hollings
 * observed cost model
 *
 * Revision 1.6  1994/06/29  02:52:29  hollings
 * Added metricDefs-common.{C,h}
 * Added module level performance data
 * cleanedup types of inferrior addresses instrumentation defintions
 * added firewalls for large branch displacements due to text+data over 2meg.
 * assorted bug fixes.
 *
 * Revision 1.5  1994/06/27  18:56:49  hollings
 * removed printfs.  Now use logLine so it works in the remote case.
 * added internalMetric class.
 * added extra paramter to metric info for aggregation.
 *
 * Revision 1.4  1994/03/26  20:50:47  jcargill
 * Changed the pause/continue code.  Now it really stops, instead of
 * spin looping.
 *
 * Revision 1.3  1994/03/22  21:03:14  hollings
 * Made it possible to add new processes (& paradynd's) via addExecutable.
 *
 * Revision 1.2  1994/03/20  01:53:07  markc
 * Added a buffer to each process structure to allow for multiple writers on the
 * traceStream.  Replaced old inst-pvm.C.  Changed addProcess to return type
 * int.
 *
 * Revision 1.1  1994/02/07  17:38:48  hollings
 * Added inst-sunos to split cm-5 code from standard sunos code.
 *
 *
 *
 */
char inst_sunos_ident[] = "@(#) $Header: /home/jaw/CVSROOT_20081103/CVSROOT/core/paradynd/src/Attic/inst-sunos.C,v 1.12 1994/08/17 18:11:59 markc Exp $";

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/param.h>
#include <sys/wait.h>
#include <sys/errno.h>
#include <machine/reg.h>

extern "C" {
#include <sys/ptrace.h>
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
#include "dyninstRPC.SRVR.h"
#include "util.h"


process *nodePseudoProcess;

char *getProcessStatus(process *proc)
{
   char ret[80];

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

#define NS_TO_SEC       1000000000.0

StringList<int> primitiveCosts;


//
// All costs are based on Measurements on a SPARC station 10/40.
//
void initPrimitiveCost()
{
    /* Need to add code here to collect values for other machines */

    // these happen async of the rest of the system.
    primitiveCosts.add(1, (void *) "DYNINSTalarmExpire");
    primitiveCosts.add(1, (void *) "DYNINSTsampleValues");
    primitiveCosts.add(1, (void *) "DYNINSTreportTimer");
    primitiveCosts.add(1, (void *) "DYNINSTreportCounter");
    primitiveCosts.add(1, (void *) "DYNINSTreportCost");
    primitiveCosts.add(1, (void *) "DYNINSTreportNewTags");
    primitiveCosts.add(1, (void *) "DYNINSTprintCost");

    // this doesn't really take any time
    primitiveCosts.add(1, (void *) "DYNINSTbreakPoint");

    // this happens before we start keeping time.
    primitiveCosts.add(1, (void *) "DYNINSTinit");

    // isthmus acutal numbers from 7/3/94 -- jkh
    // 240 ns
    primitiveCosts.add(16, (void *) "DYNINSTincrementCounter");
    // 240 ns
    primitiveCosts.add(16, (void *) "DYNINSTdecrementCounter");
    // 2.82 usec * 40 mhz
    primitiveCosts.add(113, (void *) "DYNINSTstartWallTimer");
    // 5.22 usec * 40 mhz
    primitiveCosts.add(208, (void *) "DYNINSTstopWallTimer");
    // 2.9 usec * 40 mhz
    // counted instructions == 84 cycles + 2 * 14 for cache miss ???
    primitiveCosts.add(116, (void *) "DYNINSTstartProcessTimer");
    // 3.74 usec * 40 mhz
    primitiveCosts.add(150, (void *) "DYNINSTstopProcessTimer");
}


/*
 * return the time required to execute the passed primitive.
 *
 */
int getPrimitiveCost(char *name)
{
    int ret;
    static int init;

    if (!init) { init = 1; initPrimitiveCost(); }

    ret = primitiveCosts.find(name);
    if (ret == 0) {
        ret = 1000;
    }
    return(ret);
}

int flushPtrace()
{
    return(0);
}

/*
 * The performance consultant's ptrace, it calls CM_ptrace and ptrace as needed.
 *
 */
int PCptrace(int request, process *proc, void *addr, int data, void *addr2)
{
    int ret;
    int sig;
    int status;
    int isStopped, wasStopped;
    extern int errno;
    extern int ptraceOtherOps, ptraceOps, ptraceBytes;

    if (proc->status == exited) {
        printf("attempt to ptrace exited process %d\n", proc->pid);
        return(-1);
    }
	
    ptraceOps++;
    if (request == PTRACE_WRITEDATA)
	ptraceBytes += data;
    else if (request == PTRACE_POKETEXT)
	ptraceBytes += sizeof(int);
    else
	ptraceOtherOps++;

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
		printf("problem stopping process\n");
		abort();
	    }
	    sig = WSTOPSIG(status);
	    if (sig == SIGSTOP) {
		isStopped = 1;
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

    if ((proc->status != neonatal) && (request != PTRACE_CONT) &&
	(!wasStopped)) {
	(void) ptrace(PTRACE_CONT, proc->pid,(char*) 1, SIGCONT, (char*) 0);
    }
    errno = 0;
    return(ret);
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
    char command[256];
    char application[256];
    char app_pid[20];
    char num_nodes[20];	
    char *argv[20];
    extern char *programName;

    parent = findProcess(fr->ppid);
    assert(parent);

    /* Build arglist */
    arg_list = RPC_make_arg_list (pd_family, pd_type, 
				  pd_known_socket, pd_flag, pd_machine);
    sprintf (command, "%sCM5", programName);
    sprintf (application, "%s", curr->symbols->file);
    sprintf (app_pid, "%d", curr->pid);
    sprintf (num_nodes, "%d", fr->npids);

    /*
     * It would be nice if this weren't sensitive to the size of
     * arg_list.  For the moment, only arg_list[0] --> arg_list[4]
     * are written by RPC_make_arg_list (arg_list[5] is NULL).
     * This is a small-time hack.
     */
    argv[0] = command;
    argv[1] = application;
    argv[2] = app_pid;
    argv[3] = num_nodes;
    argv[4] = arg_list[0];
    argv[5] = arg_list[1];
    argv[6] = arg_list[2];
    argv[7] = arg_list[3];
    argv[8] = arg_list[4];
    argv[9] = 0;

    if ((childPid=fork()) == 0) {		/* child */

/* 	ptrace (0, 0, 0, 0, 0); */

	execvp (command, argv);
	abort();
    }
    else {			/* parent */
	printf ("forked child process (pid=%d).\n", childPid);
    }

    /* Mark the cm-process as running now */
//    curr->status = running;

    pauseAllProcesses();
}



libraryList msgFilterFunctions;
libraryList msgByteSentFunctions;
libraryList msgByteRecvFunctions;
libraryList msgByteFunctions;
libraryList fileByteFunctions;
libraryList libraryFunctions;

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
    addLibFunc(&libraryFunctions, "exit", TAG_LIB_FUNC);
    addLibFunc(&libraryFunctions, "fork", TAG_LIB_FUNC);

    addLibFunc(&libraryFunctions, "cmmd_debug", TAG_LIB_FUNC);
    addLibFunc(&libraryFunctions, "CMMD_send", TAG_LIB_FUNC);
    addLibFunc(&libraryFunctions, "CMMD_receive", TAG_LIB_FUNC);
    addLibFunc(&libraryFunctions, "CMMD_receive_block", TAG_LIB_FUNC);
    addLibFunc(&libraryFunctions, "CMMD_send_block", TAG_LIB_FUNC);
    addLibFunc(&libraryFunctions, "CMMD_send_async", TAG_LIB_FUNC);
    addLibFunc(&libraryFunctions, "CMMD_send_async", TAG_LIB_FUNC);

    addLibFunc(&libraryFunctions, "main", 0);

    libraryFunctions += fileByteFunctions;
    libraryFunctions += msgByteSentFunctions;
    libraryFunctions += msgByteRecvFunctions;
    libraryFunctions += msgFilterFunctions;

    msgByteFunctions += msgByteSentFunctions;
    msgByteFunctions += msgByteRecvFunctions;
}

int findNodeOffset(char *file, int offset)
{
    return(0);
}

float computePauseTimeMetric()
{
    timeStamp now;
    timeStamp elapsed;
    extern timeStamp startPause;
    extern time64 firstRecordTime;
    extern Boolean firstSampleReceived;
    extern Boolean applicationPaused;
    extern timeStamp elapsedPauseTime;
    static timeStamp reportedPauseTime = 0;
    extern timeStamp getCurrentTime(Boolean firstRecordRelative);

    now = getCurrentTime(FALSE);
    if (firstRecordTime && firstSampleReceived) {
	elapsed = elapsedPauseTime - reportedPauseTime;
	if (applicationPaused) {
	    elapsed += now - startPause;
	}
	assert(elapsed >= 0.0); 
	reportedPauseTime += elapsed;
	return(elapsed);
    } else {
	return(0.0);
    }
}

