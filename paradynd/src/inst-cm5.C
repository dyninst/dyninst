/*
 *  Copyright 1993 Jeff Hollingsworth.  All rights reserved.
 *
 */

#ifndef lint
static char Copyright[] = "@(#) Copyright (c) 1993 Jeff Hollingsowrth
    All rights reserved.";

static char rcsid[] = "@(#) $Header: /home/jaw/CVSROOT_20081103/CVSROOT/core/paradynd/src/Attic/inst-cm5.C,v 1.29 1996/04/29 03:35:23 tamches Exp $";
#endif

/*
 * inst-cm5.C - runtime library specific files to inst on this machine.
 *
 * $Log: inst-cm5.C,v $
 * Revision 1.29  1996/04/29 03:35:23  tamches
 * computePauseTimeMetric now takes in a param (but doesn't use it), to conform
 * to new internalMetrics class
 *
 * Revision 1.28  1996/03/13 21:53:55  newhall
 * changed pause time to be computed like it is on all other platforms
 *
 * Revision 1.27  1996/02/15  14:56:58  naim
 * Laeving previous values for costs primitives on the CM-5 - naim
 *
 * Revision 1.26  1996/02/13  21:37:08  naim
 * Minor change related to the cost model for the CM-5 - naim
 *
 * Revision 1.25  1995/05/18  10:35:21  markc
 * Removed tag dictionary
 *
 * Revision 1.24  1995/02/16  08:53:16  markc
 * Corrected error in comments -- I put a "star slash" in the comment.
 *
 * Revision 1.23  1995/02/16  08:33:22  markc
 * Changed igen interfaces to use strings/vectors rather than char igen-arrays
 * Changed igen interfaces to use bool, not Boolean.
 * Cleaned up symbol table parsing - favor properly labeled symbol table objects
 * Updated binary search for modules
 * Moved machine dependnent ptrace code to architecture specific files.
 * Moved machine dependent code out of class process.
 * Removed almost all compiler warnings.
 * Use "posix" like library to remove compiler warnings
 *
 * Revision 1.22  1995/01/26  18:11:57  jcargill
 * Updated igen-generated includes to new naming convention
 *
 * Revision 1.21  1994/11/09  18:40:05  rbi
 * the "Don't Blame Me" commit
 *
 * Revision 1.20  1994/11/02  11:06:16  markc
 * Removed redundant code into inst.C
 * Provide "tag" dictionary for known functions.
 *
 * Revision 1.18  1994/09/30  19:47:02  rbi
 * Basic instrumentation for CMFortran
 *
 * Revision 1.17  1994/09/23  15:57:32  jcargill
 * Miniscule cleanup of node i/o polling
 *
 * Revision 1.16  1994/09/22  01:55:22  markc
 * Declare system includes as extern "C"
 * Remove libraryList typedef, use List<libraryFunc*>
 * Enter primtiveCosts handles into stringPools
 *
 * Revision 1.15  1994/08/02  18:21:28  hollings
 * changed costs to reflect new retries for race conditions in timers.
 *
 * Revision 1.14  1994/07/22  19:16:01  hollings
 * update cost data, move pausetime in here.
 *
 * Revision 1.13  1994/07/21  01:34:48  hollings
 * removed extra polls of the nodes for printfs.
 *
 * Revision 1.12  1994/07/20  23:23:17  hollings
 * Added real code for cost model.
 *
 * Revision 1.11  1994/07/15  20:22:00  hollings
 * fixed 64 bit record to be 32 bits.
 *
 * Revision 1.10  1994/07/14  23:30:23  hollings
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

#include <sys/param.h>

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
#include "os.h"

extern timeStamp startPause;
extern time64 firstRecordTime;
extern timeStamp elapsedPauseTime;
extern timeStamp getCurrentTime(bool firstRecordRelative);
extern bool isApplicationPaused();

/*
 * Define the various classes of library functions to inst. 
 *
 */
void initLibraryFunctions()
{
#ifdef notdef
  tagDict["CMMD_send"] = TAG_LIB_FUNC | TAG_MSG_SEND | TAG_CPU_STATE | TAG_MSG_FILT;

  // TODO - why not TAG_CPU_STATE for the following 
  tagDict["CMMD_send_block"] = TAG_LIB_FUNC | TAG_MSG_SEND;
  tagDict["CMMD_send_noblock"] = TAG_LIB_FUNC | TAG_MSG_SEND;
  tagDict["CMMD_send_async"] = TAG_LIB_FUNC | TAG_MSG_SEND;

//    addLibFunc(&msgByteSentFunctions, "CMPR_m_p_open_for_send", 
//		 TAG_LIB_FUNC);
//    addLibFunc(&msgByteSentFunctions, "CMPR_m_s_open_for_send", 
//		 TAG_LIB_FUNC);

  tagDict["CMMD_receive"] = TAG_LIB_FUNC | TAG_MSG_RECV | TAG_CPU_STATE | TAG_MSG_FILT;

  // TODO - why not TAG_CPU state for the following
  tagDict["CMMD_receive_block"] = TAG_LIB_FUNC | TAG_MSG_RECV;
  tagDict["CMMD_receive_noblock"] = TAG_LIB_FUNC | TAG_MSG_RECV;
  tagDict["CMMD_receive_async"] = TAG_LIB_FUNC | TAG_MSG_RECV;

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
  tagDict["write"] = TAG_LIB_FUNC | TAG_IO_OUT | TAG_CPU_STATE;
  tagDict["read"] = TAG_LIB_FUNC | TAG_IO_IN | TAG_CPU_STATE;

  tagDict["CMPE_CMCOM_pe_init"] = TAG_LIB_FUNC;
  tagDict["pe_main_default"] = TAG_LIB_FUNC;
  tagDict["CMNA_dispatch_idle"] = TAG_LIB_FUNC;
  tagDict["DYNINSTalarmExpire"] = TAG_LIB_FUNC;
  tagDict["DYNINSTsampleValues"] = TAG_LIB_FUNC;
  tagDict["cmmd_debug"] = TAG_LIB_FUNC;
  tagDict["CMMD_init"] = TAG_LIB_FUNC;
  tagDict[EXIT_NAME] = TAG_LIB_FUNC;
  tagDict["fork"] = TAG_LIB_FUNC;
  tagDict["main"] = 0;

  tagDict["CMMP_receive_block"] = TAG_LIB_FUNC | TAG_CPU_STATE | TAG_MSG_FILT; 
  tagDict["CMMP_send_block"] = TAG_LIB_FUNC | TAG_MSG_FILT | TAG_CPU_STATE; 
  tagDict["CMMP_send_async"] = TAG_LIB_FUNC | TAG_MSG_FILT | TAG_CPU_STATE; 
  tagDict["CMMP_receive_async"] = TAG_LIB_FUNC | TAG_MSG_FILT | TAG_CPU_STATE; 
  tagDict["CMMP_all_msgs_wait"] = TAG_LIB_FUNC | TAG_MSG_FILT | TAG_CPU_STATE; 
  tagDict["CMMP_msg_wait"] = TAG_LIB_FUNC | TAG_MSG_FILT | TAG_CPU_STATE;

  /* reduction library functions */
  tagDict["CMMD_reduce_int"] = TAG_LIB_FUNC | TAG_MSG_FILT | TAG_CPU_STATE;
  tagDict["CMMD_reduce_uint"] = TAG_LIB_FUNC | TAG_MSG_FILT | TAG_CPU_STATE;
  tagDict["CMMD_reduce_float"] = TAG_LIB_FUNC | TAG_MSG_FILT | TAG_CPU_STATE;
  tagDict["CMMD_reduce_double"] = TAG_LIB_FUNC | TAG_MSG_FILT | TAG_CPU_STATE;
  tagDict["CMMD_reduce_v"] = TAG_LIB_FUNC | TAG_MSG_FILT | TAG_CPU_STATE;

  /* scan operations */
  tagDict["CMMD_scan_int"] = TAG_LIB_FUNC | TAG_MSG_FILT | TAG_CPU_STATE;
  tagDict["CMMD_scan_uint"] = TAG_LIB_FUNC | TAG_MSG_FILT | TAG_CPU_STATE;
  tagDict["CMMD_scan_float"] = TAG_LIB_FUNC | TAG_MSG_FILT | TAG_CPU_STATE;
  tagDict["CMMD_scan_double"] = TAG_LIB_FUNC | TAG_MSG_FILT | TAG_CPU_STATE;
  tagDict["CMMD_scan_v"] = TAG_LIB_FUNC | TAG_MSG_FILT | TAG_CPU_STATE;

  /* global sync functions */
  tagDict["CMMD_sync_with_nodes"] = TAG_LIB_FUNC | TAG_SYNC_FUNC | TAG_CPU_STATE;

  /* broadcast functions */
  tagDict["CMMD_bc_to_nodes"] = TAG_LIB_FUNC | TAG_SYNC_FUNC | TAG_CPU_STATE;
  tagDict["CMMD_receive_bc_from_node"] = TAG_LIB_FUNC | TAG_SYNC_FUNC | TAG_CPU_STATE;

  /* un-used node */
  tagDict["cmcn_sleep"] = TAG_LIB_FUNC | TAG_CPU_STATE;
#endif
}

void forkNodeProcesses(process *curr, traceHeader *hr, traceFork *fr) {
    // CM-5 app should not spawn cm-5 apps!
    abort();
}

//
// All costs are based on 30ns clock (~33MHz) and stats reported in the
//   SHPCC paper.
//
void initPrimitiveCost()
{
    /* Need to add code here to collect values for other machines */

    //
    //TODO: these values need to be updated as it has been done for the
    //      other platforms - naim (02-13-96)
    //

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

    /* based on measured values for the CM-5. */
    /* Need to add code here to collect values for other machines */
    // cm-5 no assembly numbers.
    // with fixed getProcess time -- jkh 8/26/94
    primitiveCosts["DYNINSTstartProcessTimer"] = 69;
    primitiveCosts["DYNINSTstopProcessTimer"] = 176;
    primitiveCosts["DYNINSTstartWallTimer"] = 43;
    primitiveCosts["DYNINSTstopWallTimer"] = 70;

#ifdef notdef
    // -- paper numbers
    primitiveCosts["DYNINSTstartProcessTimer"] = 37;
    primitiveCosts["DYNINSTstopProcessTimer"] = 71;
    primitiveCosts["DYNINSTstartWallTimer"] = 32;
    primitiveCosts["DYNINSTstopWallTimer"] = 55;
#endif
}


/*
 * Not required on this platform.
 *
 */
void instCleanup()
{
}

string process::getProcessStatus() const
{
    static char ret[80];
    int local_status;

    sprintf (errorLine, "getting status for process proc=%x, pid=%d\n", 
	     (unsigned)this, pid);
    logLine(errorLine);
    if (pid > MAXPID) {

      // PCptrace (PTRACE_STATUS, proc, 0, 0, 0);

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
	sprintf(ret, "Not SUPPORTED");
	return (ret);
    } else {
        int local_pid = waitpid (pid, &local_status, WNOHANG);
	sprintf(errorLine, "status of real unix process is:  %x  (ret = %d, pid = %d)\n", 
		local_status, local_pid, pid);
	logLine(errorLine);

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
}

#ifdef ndef
float computePauseTimeMetric()
{
    float max=0.0;

    unsigned size = processVec.size();
    for (unsigned u=0; u<size; u++) {
      if (processVec[u]->pauseTime > max) {
	max = processVec[u]->pauseTime;
      }
    }
    return(max);
}
#endif

float computePauseTimeMetric(const metricDefinitionNode *) {
    // we don't need to use the metricDefinitionNode
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
