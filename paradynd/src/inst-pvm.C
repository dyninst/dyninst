
/*
 * inst-pvm.C - sunos specifc code for paradynd.
 *
 * $Log: inst-pvm.C,v $
 * Revision 1.21  1995/08/24 15:03:59  hollings
 * AIX/SP-2 port (including option for split instruction/data heaps)
 * Tracing of rexec (correctly spawns a paradynd if needed)
 * Added rtinst function to read getrusage stats (can now be used in metrics)
 * Critical Path
 * Improved Error reporting in MDL sematic checks
 * Fixed MDL Function call statement
 * Fixed bugs in TK usage (strings passed where UID expected)
 *
 * Revision 1.20  1995/05/18  10:35:23  markc
 * Removed tag dictionary
 *
 * Revision 1.19  1995/02/26  22:45:36  markc
 * Updated to compile under new system.
 *
 * Revision 1.18  1995/02/16  08:33:24  markc
 * Changed igen interfaces to use strings/vectors rather than char* igen-arrays
 * Changed igen interfaces to use bool, not Boolean.
 * Cleaned up symbol table parsing - favor properly labeled symbol table objects
 * Updated binary search for modules
 * Moved machine dependnent ptrace code to architecture specific files.
 * Moved machine dependent code out of class process.
 * Removed almost all compiler warnings.
 * Use "posix" like library to remove compiler warnings
 *
 * Revision 1.17  1994/11/10  18:58:01  jcargill
 * The "Don't Blame Me Either" commit
 *
 * Revision 1.16  1994/11/09  18:40:08  rbi
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
char inst_sunos_ident[] = "@(#) /p/paradyn/CVSROOT/core/paradynd/src/inst-pvm.C,v 1.20 1995/05/18 10:35:23 markc Exp";

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

string process::getProcessStatus() const
{
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

    // 23.39 usec * 85 mhz (SS-5)
    // 20 cycles (DYNINSTstartWallTimer) +  17+8 (getWallTime)
    primitiveCosts["DYNINSTstartWallTimer"] = 45;
    // 42 cycles (DYNINSTstopWallTimer) +  2(17+8) (getWallTime)
    primitiveCosts["DYNINSTstopWallTimer"] = 92;
    // 1.61 usec * 85 Mhz (measured on a SS-5)
    // 25 cycles (read clock) +  26 (startProcessTimer)
    primitiveCosts["DYNINSTstartProcessTimer"] = 51;
     // 3.38 usec * 85 mhz (measured on a SS-5)
    // 61 cycles + 2*25 cycles to read clock
    primitiveCosts["DYNINSTstopProcessTimer"] = 111;
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



