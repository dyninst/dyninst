
/*
 * inst-sunos.C - sunos specifc code for paradynd.
 *
 * $Log: inst-sunos.C,v $
 * Revision 1.29  1995/08/24 15:04:03  hollings
 * AIX/SP-2 port (including option for split instruction/data heaps)
 * Tracing of rexec (correctly spawns a paradynd if needed)
 * Added rtinst function to read getrusage stats (can now be used in metrics)
 * Critical Path
 * Improved Error reporting in MDL sematic checks
 * Fixed MDL Function call statement
 * Fixed bugs in TK usage (strings passed where UID expected)
 *
 * Revision 1.28  1995/05/30  05:04:58  krisna
 * upgrade from solaris-2.3 to solaris-2.4.
 * architecture-os based include protection of header files.
 * removed architecture-os dependencies in generic sources.
 * changed ST_* symbol names to PDST_* (to avoid conflict on HPUX)
 *
 * Revision 1.27  1995/05/18  10:35:25  markc
 * Removed tag dictionary
 *
 * Revision 1.26  1995/03/10  19:33:49  hollings
 * Fixed several aspects realted to the cost model:
 *     track the cost of the base tramp not just mini-tramps
 *     correctly handle inst cost greater than an imm format on sparc
 *     print starts at end of pvm apps.
 *     added option to read a file with more accurate data for predicted cost.
 *
 * Revision 1.25  1995/02/16  08:53:29  markc
 * Corrected error in comments -- I put a "star slash" in the comment.
 *
 * Revision 1.24  1995/02/16  08:33:28  markc
 * Changed igen interfaces to use strings/vectors rather than char igen-arrays
 * Changed igen interfaces to use bool, not Boolean.
 * Cleaned up symbol table parsing - favor properly labeled symbol table objects
 * Updated binary search for modules
 * Moved machine dependnent ptrace code to architecture specific files.
 * Moved machine dependent code out of class process.
 * Removed almost all compiler warnings.
 * Use "posix" like library to remove compiler warnings
 *
 * Revision 1.23  1995/01/26  18:12:00  jcargill
 * Updated igen-generated includes to new naming convention
 *
 * Revision 1.22  1994/11/11  10:44:03  markc
 * Remove non-emergency prints
 * Changed others to use statusLine
 *
 * Revision 1.21  1994/11/11  10:11:40  markc
 * Used correct arg order for RPC_make_arg_list
 *
 * Revision 1.20  1994/11/11  07:04:55  markc
 * Added code to bundle extra command line argument.
 *
 * Revision 1.19  1994/11/10  18:58:02  jcargill
 * The "Don't Blame Me Either" commit
 *
 * Revision 1.18  1994/11/09  18:40:10  rbi
 * the "Don't Blame Me" commit
 *
 * Revision 1.17  1994/11/02  11:06:19  markc
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
char inst_sunos_ident[] = "@(#) /p/paradyn/CVSROOT/core/paradynd/src/inst-sunos.C,v 1.28 1995/05/30 05:04:58 krisna Exp";

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
#include "dyninstRPC.xdr.SRVR.h"
#include "util.h"
#include "stats.h"
#include "main.h"
#include "perfStream.h"
#include "context.h"

string process::getProcessStatus() const {
   char ret[80];

   switch (status()) {
	case running:
	    sprintf(ret, "%d running", pid);
	    break;
	case neonatal:
	    sprintf(ret, "%d neonatal", pid);
	    break;
	case stopped:
	    sprintf(ret, "%d stopped", pid);
	    break;
	case exited:
	    sprintf(ret, "%d exited", pid);
	    break;
	default:
	    sprintf(ret, "%d UNKNOWN State", pid);
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
#ifdef sparc_sun_solaris2_4
    // Solaris measured 
    // clock == 39.173MHz
    // cost per call DYNINSTstartWallTimer 11.700000 usec 
    // cost per call DYNINSTstopWallTimer 22.550000 usec 
    // cost per call DYNINSTstartProcessTimer 14.650000 usec 
    // cost per call DYNINSTstopProcessTimer 24.550000 usec 
    logLine("Solaris dyninst costs being used");

    primitiveCosts["DYNINSTstartWallTimer"] = 468;
    primitiveCosts["DYNINSTstopWallTimer"] = 900;
    primitiveCosts["DYNINSTstartProcessTimer"] = 574;
    primitiveCosts["DYNINSTstopProcessTimer"] = 961;
#else
    // 23.39 usec * 85 mhz (SS-5)
    primitiveCosts["DYNINSTstartWallTimer"] = 1988;
    // 48.05 usec * 85 mhz (SS-5)
    primitiveCosts["DYNINSTstopWallTimer"] = 4084;
    // 1.61 usec * 85 Mhz (measured on a SS-5)
    // 25 cycles (read clock) +  26 (startProcessTimer)
    primitiveCosts["DYNINSTstartProcessTimer"] = 51;
     // 3.38 usec * 85 mhz (measured on a SS-5)
    // 61 cycles + 2*25 cycles to read clock
    primitiveCosts["DYNINSTstopProcessTimer"] = 111;
#endif
}

int flushPtrace()
{
    return(0);
}

void forkNodeProcesses(process *curr, traceHeader *hr, traceFork *fr)
{
    int childPid;
    char command[256];
    char application[256];
    char app_pid[20];
    char num_nodes[20];	

    process *parent = findProcess(fr->ppid);
    if (!parent) {
      sprintf(errorLine, "In forkNodeProcesses, parent id %d unknown", fr->ppid);
      statusLine(errorLine);
      return;
    }

    /* Build arglist */
    sprintf (command, "%sCM5", process::programName.string_of());
    sprintf (application, "%s", (curr->symbols->file()).string_of());
    sprintf (app_pid, "%d", curr->pid);
    sprintf (num_nodes, "%d", fr->npids);

    /*
     * It would be nice if this weren't sensitive to the size of
     * arg_list.  For the moment, only arg_list[0] --> arg_list[6]
     * are written by RPC_make_arg_list
     * This is a small-time hack.
     */

    char *argv[20];
    argv[0] = command;
    argv[1] = application;
    argv[2] = app_pid;
    argv[3] = num_nodes;

    // IF these are change, check out the delete below
    argv[4] = P_strdup(process::arg_list[0].string_of());
    argv[5] = P_strdup(process::arg_list[1].string_of());
    argv[6] = P_strdup(process::arg_list[2].string_of());
    argv[7] = P_strdup(process::arg_list[3].string_of());
    argv[8] = P_strdup(process::arg_list[4].string_of());
    argv[9] = P_strdup(process::arg_list[5].string_of());
    argv[10] = P_strdup(process::arg_list[6].string_of());
    argv[11] = NULL;

    if ((childPid=fork()) == 0) {		/* child */
      P_execvp (command, argv);
      logLine("Exec failed in paradynd to start paradyndCM5\n");
      P_abort();
    } else {			/* parent */
      sprintf (errorLine, "forked child process (pid=%d)", childPid);
      statusLine(errorLine);
    }

    for (int di=4; di<10; di++)
      delete argv[di];

    /* Mark the cm-process as running now */
    // curr->status = running;
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
#ifdef notdef
    tagDict["write"] = TAG_LIB_FUNC | TAG_IO_OUT;
    tagDict["read"] = TAG_LIB_FUNC | TAG_IO_IN;

    tagDict["send"] = TAG_LIB_FUNC | TAG_CPU_STATE | TAG_MSG_SEND;
    tagDict["sendmsg"] = TAG_LIB_FUNC | TAG_CPU_STATE | TAG_MSG_SEND;
    tagDict["sendto"] = TAG_LIB_FUNC | TAG_CPU_STATE | TAG_MSG_SEND;

    tagDict["rev"] = TAG_LIB_FUNC | TAG_CPU_STATE | TAG_MSG_RECV;
    tagDict["recvmsg"] = TAG_LIB_FUNC | TAG_CPU_STATE | TAG_MSG_RECV;
    tagDict["recvfrom"] = TAG_LIB_FUNC | TAG_CPU_STATE | TAG_MSG_RECV;

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
#endif
}
 
float computePauseTimeMetric()
{
    timeStamp now;
    timeStamp elapsed=0.0;

    now = getCurrentTime(false);
    if (firstRecordTime) {
	elapsed = elapsedPauseTime;
	if (isApplicationPaused())
	    elapsed += now - startPause;

	assert(elapsed >= 0.0); 
	return(elapsed);
    } else {
	return(0.0);
    }
}

void osDependentInst(process *proc) {
}
