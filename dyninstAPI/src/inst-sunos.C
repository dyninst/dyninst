
/*
 * inst-sunos.C - sunos specifc code for paradynd.
 *
 * $Log: inst-sunos.C,v $
 * Revision 1.17  1994/11/02 11:06:19  markc
 * Removed redundant code into inst.C
 * Provide "tag" dictionary for known functions.
 *
 * Revision 1.16  1994/10/13  07:24:45  krisna
 * solaris porting and updates
 *
 * Revision 1.15  1994/09/30  19:47:05  rbi
 * Basic instrumentation for CMFortran
 *
 * Revision 1.14  1994/09/22  01:58:53  markc
 * Enter handles for primitiveCosts into stringPool
 * changed libraryList to List<libraryFunc*>
 *
 * Revision 1.13  1994/09/20  18:18:25  hollings
 * added code to use actual clock speed for cost model numbers.
 *
 * Revision 1.12  1994/08/17  18:11:59  markc
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
char inst_sunos_ident[] = "@(#) $Header: /home/jaw/CVSROOT_20081103/CVSROOT/core/dyninstAPI/src/inst-sunos.C,v 1.17 1994/11/02 11:06:19 markc Exp $";

#include "util/h/kludges.h"
#include "os.h"
#include "metric.h"
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
#include "stats.h"
#include "main.h"
#include "perfStream.h"
#include "kludges.h"
#include "context.h"

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

//
// All costs are based on Measurements on a SPARC station 10/40.
//
void initPrimitiveCost()
{
    /* Need to add code here to collect values for other machines */

    // these happen async of the rest of the system.
    primitiveCosts["DYNINSTalarmExpire"] = 1;
    primitiveCosts["DYNINSTsampleValues"] = 1;
    primitiveCosts["DYNINSTreportTimer"] = 1;
    primitiveCosts["DYNINSTreportCounter"] = 1;
    primitiveCosts["DYNINSTreportCost"] = 1;
    primitiveCosts["DYNINSTreportNewTags"] = 1;
    primitiveCosts["DYNINSTprintCost"] = 1;

    // this doesn't really take any time
    primitiveCosts["DYNINSTbreakPoint"] = 1;

    // this happens before we start keeping time.
    primitiveCosts["DYNINSTinit"] = 1;

    // isthmus acutal numbers from 7/3/94 -- jkh
    // 240 ns
    primitiveCosts["DYNINSTincrementCounter"] = 16;
    // 240 ns
    primitiveCosts["DYNINSTdecrementCounter"] = 16;
    // 7.4 usec * 70 mhz (SS-5)
    primitiveCosts["DYNINSTstartWallTimer"] = 518;
    // 9.6 usec * 70 mhz (SS-5)
    primitiveCosts["DYNINSTstopWallTimer"] = 841;
    // 1.80 usec * 70 Mhz (measured on a SS-5)
    primitiveCosts["DYNINSTstartProcessTimer"] = 126;
    // 3.46 usec * 70 mhz (measured on a SS-5)
    primitiveCosts["DYNINSTstopProcessTimer"] = 242;

}

int flushPtrace()
{
    return(0);
}

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

    if (!processMap.defines(fr->ppid)) {
      sprintf(errorLine, "In forkNodeProcesses, parent id %d unknown\n", fr->ppid);
      logLine(errorLine);
      return;
    }
    parent = processMap[fr->ppid];
    assert(parent);

    /* Build arglist */
    arg_list = RPC_make_arg_list (pd_family, pd_type, 
				  pd_known_socket, pd_flag, pd_machine);

    sprintf (command, "%sCM5", programName);
    sprintf (application, "%s", (curr->symbols->file).string_of());
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
        logLine("Exec failed in paradynd to start paradyndCM5\n");
	abort();
    }
    else {			/* parent */
	printf ("forked child process (pid=%d).\n", childPid);
    }

    /* Mark the cm-process as running now */
//    curr->status = running;

    pauseAllProcesses();
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
    tagDict["write"] = TAG_LIB_FUNC | TAG_IO_OUT | TAG_CPU_STATE;
    tagDict["read"] = TAG_LIB_FUNC | TAG_IO_IN | TAG_CPU_STATE;


    tagDict["DYNINSTalarmExpire"] = TAG_LIB_FUNC;
    tagDict["DYNINSTsampleValues"] = TAG_LIB_FUNC;
    tagDict[EXIT_NAME] = TAG_LIB_FUNC;
    tagDict["fork"] = TAG_LIB_FUNC;

    tagDict["cmmd_debug"] = TAG_LIB_FUNC;
    tagDict["CMRT_init"] = TAG_LIB_FUNC;
    tagDict["CMMD_send"] = TAG_LIB_FUNC;
    tagDict["CMMD_receive"] = TAG_LIB_FUNC;
    tagDict["CMMD_receive_block"] = TAG_LIB_FUNC;
    tagDict["CMMD_send_block"] = TAG_LIB_FUNC;
    tagDict["CMMD_send_async"] = TAG_LIB_FUNC;
    tagDict["CMMD_send_async"] = TAG_LIB_FUNC;

    tagDict["main"] = 0;
}
 
float computePauseTimeMetric()
{
    timeStamp now;
    timeStamp elapsed;
    static timeStamp reportedPauseTime = 0;

    now = getCurrentTime(false);
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

