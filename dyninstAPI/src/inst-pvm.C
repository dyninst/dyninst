
/*
 * inst-pvm.C - sunos specifc code for paradynd.
 *
 * $Log: inst-pvm.C,v $
 * Revision 1.16  1994/11/09 18:40:08  rbi
 * the "Don't Blame Me" commit
 *
 * Revision 1.15  1994/11/02  11:06:18  markc
 * Removed redundant code into inst.C
 * Provide "tag" dictionary for known functions.
 *
 * Revision 1.13  1994/09/22  01:56:17  markc
 * Changed libraryList to List<libraryFunc*>
 * make system includes extern "C"
 *
 * Revision 1.12  1994/08/17  18:10:59  markc
 * Added pvm_getrbuf and pvm_getsbuf to initLibraryFuncs for pvm since these
 * functions are used for message accounting.
 *
 * Revision 1.11  1994/08/01  00:24:13  markc
 * Added computePauseTimeMetric to allow paradyndPVM to compile.
 *
 * Revision 1.10  1994/07/12  20:11:06  jcargill
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
char inst_sunos_ident[] = "@(#) $Header: /home/jaw/CVSROOT_20081103/CVSROOT/core/dyninstAPI/src/Attic/inst-pvm.C,v 1.16 1994/11/09 18:40:08 rbi Exp $";

#include "util/h/kludges.h"

extern "C" {
#include "pvm3.h"
}

#include "dyninst.h"
#include "rtinst/h/trace.h"
#include "symtab.h"
#include "process.h"
#include "inst.h"
#include "instP.h"
#include "ast.h"
#include "util.h"
#include "util/h/String.h"
#include "util/h/Dictionary.h"
#include "context.h"
#include "perfStream.h"
#include "metric.h"
#include "os.h"

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
 * machine specific init for PVM.
 *
 */
void machineInit()
{

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

int flushPtrace() {
    return(0);
}

void forkNodeProcesses(process *curr, traceHeader *hr, traceFork *fr) {
  // should get this call - mdc
  // this would be called due to a cm5 application
  abort();
}


/*
 * Define the various classes of library functions to inst. 
 *
 */
void initLibraryFunctions()
{
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
    tagDict["write"] = TAG_LIB_FUNC | TAG_IO_OUT | TAG_CPU_STATE;
    tagDict["read"] =  TAG_LIB_FUNC | TAG_IO_IN | TAG_CPU_STATE;
    tagDict["DYNINSTsampleValues"] = TAG_LIB_FUNC;
    tagDict[EXIT_NAME] = TAG_LIB_FUNC;
    tagDict["fork"] = TAG_LIB_FUNC;
    tagDict["main"] = 0;
    tagDict["pvm_bufinfo"] = TAG_LIB_FUNC;
    tagDict["pvm_getsbuf"] = TAG_LIB_FUNC;
    tagDict["pvm_getrbuf"] = TAG_LIB_FUNC;
    // tagDict["pvm_barrier"] = TAG_LIB_FUNC | TAG_SYNC_FUNC | TAG_CPU_STATE;
    // tagDict["pvm_mcast"] =  TAG_LIB_FUNC | TAG_MSG_FUNC | TAG_CPU_STATE;

    tagDict["pvm_send"] = TAG_LIB_FUNC | TAG_MSG_SEND;
    tagDict["pvm_recv"] = TAG_LIB_FUNC | TAG_MSG_RECV;
}

void instCleanup()
{
    pvm_exit();
}


// 
// this has been copied from inst-sunos.C
//
float computePauseTimeMetric()
{
    timeStamp now;
    timeStamp elapsed;
    static timeStamp reportedPauseTime = 0;

    now = getCurrentTime(false);
    if (firstRecordTime && firstSampleReceived) {
	elapsed = elapsedPauseTime - reportedPauseTime;
	if (isApplicationPaused()) {
	    elapsed += now - startPause;
	}
	assert(elapsed >= 0.0); 
	reportedPauseTime += elapsed;
	return(elapsed);
    } else {
	return(0.0);
    }
}



