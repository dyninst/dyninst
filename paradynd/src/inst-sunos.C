
/*
 * inst-sunos.C - sunos specifc code for paradynd.
 *
 * $Log: inst-sunos.C,v $
 * Revision 1.6  1994/06/29 02:52:29  hollings
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
char inst_sunos_ident[] = "@(#) $Header: /home/jaw/CVSROOT_20081103/CVSROOT/core/paradynd/src/Attic/inst-sunos.C,v 1.6 1994/06/29 02:52:29 hollings Exp $";

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/param.h>
#include <sys/wait.h>
#include <sys/errno.h>
#include <machine/reg.h>

extern "C" {
#include <sys/unistd.h>
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
    /* based on measured values for the CM-5. */
    /* Need to add code here to collect values for other machines */
    primitiveCosts.add(240, (void *) "DYNINSTincrementCounter");
    primitiveCosts.add(240, (void *) "DYNINSTdecrementCounter");
    primitiveCosts.add(4990, (void *) "DYNINSTstartWallTimer");
    primitiveCosts.add(5020, (void *) "DYNINSTstopWallTimer");
    primitiveCosts.add(1150, (void *) "DYNINSTstartProcessTimer");
    primitiveCosts.add(1510, (void *) "DYNINSTstopProcessTimer");
}


/*
 * return the time required to execute the passed primitive.
 *
 */
float getPrimitiveCost(char *name)
{
    float ret;
    static int init;

    if (!init) { init = 1; initPrimitiveCost(); }

    ret = primitiveCosts.find(name)/NS_TO_SEC;
    if (ret == 0.0) {
        printf("no cost value for primitive %s, using 10 usec\n", name);
        ret = 10000/NS_TO_SEC;
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

    if (proc->status == exited) {
        printf("attempt to ptrace exited process %d\n", proc->pid);
        return(-1);
    }
	
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

void sendPtraceBuffer(process *proc)
{
}

void processPtraceAck (traceHeader *header, ptraceAck *ackRecord)
{
}

void forkNodeProcesses(process *curr, traceHeader *hr, traceFork *fr)
{
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

    addLibFunc(&libraryFunctions, "DYNINSTsampleValues", TAG_LIB_FUNC);
    addLibFunc(&libraryFunctions, "exit", TAG_LIB_FUNC);
    addLibFunc(&libraryFunctions, "fork", TAG_LIB_FUNC);
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



