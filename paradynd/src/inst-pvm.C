
/*
 * inst-pvm.C - sunos specifc code for paradynd.
 *
 * $Log: inst-pvm.C,v $
 * Revision 1.10  1994/07/12 20:11:06  jcargill
 * Removed some old/dead code
 *
 * Revision 1.9  1994/07/05  03:53:42  hollings
 * fixed return type of getPrimtiveCost().
 *
 * Revision 1.8  1994/06/29  02:52:27  hollings
 * Added metricDefs-common.{C,h}
 * Added module level performance data
 * cleanedup types of inferrior addresses instrumentation defintions
 * added firewalls for large branch displacements due to text+data over 2meg.
 * assorted bug fixes.
 *
 * Revision 1.7  1994/06/27  18:56:44  hollings
 * removed printfs.  Now use logLine so it works in the remote case.
 * added internalMetric class.
 * added extra paramter to metric info for aggregation.
 *
 * Revision 1.6  1994/05/31  18:10:12  markc
 * Added default instrumentation requests and modified the library functions
 * that get are instrumented.
 *
 * Revision 1.5  1994/04/13  03:08:59  markc
 * Turned off pause_metric reporting for paradyndPVM because the metricDefNode is
 * not setup properly.  Updated inst-pvm.C and metricDefs-pvm.C to reflect changes
 * in cm5 versions.
 *
 * Revision 1.4  1994/03/31  01:49:34  markc
 * Duplicated changes in inst-sunos.C.
 *
 * Revision 1.3  1994/03/26  20:50:45  jcargill
 * Changed the pause/continue code.  Now it really stops, instead of
 * spin looping.
 *
 * Revision 1.2  1994/03/20  01:53:06  markc
 * Added a buffer to each process structure to allow for multiple writers on the
 * traceStream.  Replaced old inst-pvm.C.  Changed addProcess to return type
 * int.
 *
 *
 */
char inst_sunos_ident[] = "@(#) $Header: /home/jaw/CVSROOT_20081103/CVSROOT/core/paradynd/src/Attic/inst-pvm.C,v 1.10 1994/07/12 20:11:06 jcargill Exp $";

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
#include "pvm3.h"

libraryList msgFilterFunctions;
libraryList msgByteSentFunctions;
libraryList msgByteRecvFunctions;
libraryList msgByteFunctions;
libraryList fileByteFunctions;
libraryList libraryFunctions;

process *nodePseudoProcess;
resource machineResource;

#define NS_TO_SEC       1000000000.0

void addLibFunc(libraryList *list, char *name, int arg)
{
    libraryFunc *temp = new libraryFunc(name, arg);
    list->add(temp, (void *) temp->name);
}

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


/*
 * return the time required to execute the passed primitive.
 *
 */
int getPrimitiveCost(char *name)
{
    int ret;

    ret = 0;
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
    // TODO - changed from old
    int ret;
    int sig;
    int status;
    int isStopped, wasStopped;
    extern int errno;

    if (proc->status == exited) {
        sprintf(errorLine, "attempt to ptrace exited process %d\n", proc->pid);
	logLine(errorLine);
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
		logLine("problem stopping process\n");
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



void forkNodeProcesses(process *curr, traceHeader *hr, traceFork *fr)
{
  // TODO - old aborted here
}

/*
 * Define the various classes of library functions to inst. 
 *
 */
void initLibraryFunctions()
{
    extern void machineInit();
    machineInit();

    /* should record waiting time in read/write, but have a conflict with
     *   use of these functions by our inst code.
     *   This happens when a CPUtimer that is stopped is stopped again by the
     *   write.  It is then started again at the end of the write and should
     *   not be running then.  We could let timers go negative, but this
     *   causes a problem when inst is inserted into already running code.
     *   Not sure what the best fix is - jkh 10/4/93
     *
     */
    /*
    addLibFunc(&msgByteSentFunctions, "pvm_send", TAG_LIB_FUNC);
    addLibFunc(&msgByteRecvFunctions, "pvm_recv", TAG_LIB_FUNC);
    addLibFunc(&msgByteSentFunctions, "pvm_mcast", TAG_LIB_FUNC);
    */
    
    addLibFunc(&fileByteFunctions, "write",
	    TAG_LIB_FUNC|TAG_IO_FUNC|TAG_CPU_STATE);
    addLibFunc(&fileByteFunctions, "read",
	    TAG_LIB_FUNC|TAG_IO_FUNC|TAG_CPU_STATE);

    addLibFunc(&libraryFunctions, "DYNINSTsampleValues", TAG_LIB_FUNC);
    addLibFunc(&libraryFunctions, "exit", TAG_LIB_FUNC);
    addLibFunc(&libraryFunctions, "fork", TAG_LIB_FUNC);
    addLibFunc(&libraryFunctions, "main", 0);
    addLibFunc(&libraryFunctions, "pvm_bufinfo", TAG_LIB_FUNC);

    addLibFunc(&msgByteSentFunctions, "pvm_barrier", 
	TAG_LIB_FUNC|TAG_SYNC_FUNC|TAG_CPU_STATE);

    /* DONT COLLECT DATA FOR THIS NOW
       addLibFunc(&msgByteSentFunctions, "pvm_mcast", 
       TAG_LIB_FUNC|TAG_MSG_FUNC|TAG_CPU_STATE);
       */
    addLibFunc(&msgByteSentFunctions, "pvm_send", 
	TAG_LIB_FUNC|TAG_MSG_FUNC|TAG_CPU_STATE);
    addLibFunc(&msgByteRecvFunctions, "pvm_recv", 
	TAG_LIB_FUNC|TAG_MSG_FUNC|TAG_CPU_STATE);

    msgFilterFunctions += msgByteSentFunctions;
    msgFilterFunctions += msgByteRecvFunctions;

    libraryFunctions += fileByteFunctions;
    libraryFunctions += msgFilterFunctions;

    // msgByteFunctions is not being used, since bytes sent and bytes
    // received have separate metric generating functions in inst-pvm.C
    // but it is being updated because it had been updated
    msgByteFunctions += msgByteSentFunctions;
    msgByteFunctions += msgByteRecvFunctions;
}

int findNodeOffset(char *file, int offset)
{
  assert (offset == 0);
  return (0);
}

/*
 * machine specific init for PVM.
 *
 */
void machineInit()
{

}

void instCleanup()
{
    pvm_exit();
}



