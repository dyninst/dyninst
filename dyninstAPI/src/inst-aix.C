
/*
 * inst-aix.C - AIX specifc code for paradynd.
 *
 * XXX - The following functions seem to be less than OS dependent, but I
 *   have included them here to reduce the number of files changed in the
 *   AIX port. - jkh 6/30/95.
 *	process::getProcessStatus()
 *	computePauseTimeMetric()
 *
 * $Log: inst-aix.C,v $
 * Revision 1.1  1995/08/24 15:03:55  hollings
 * AIX/SP-2 port (including option for split instruction/data heaps)
 * Tracing of rexec (correctly spawns a paradynd if needed)
 * Added rtinst function to read getrusage stats (can now be used in metrics)
 * Critical Path
 * Improved Error reporting in MDL sematic checks
 * Fixed MDL Function call statement
 * Fixed bugs in TK usage (strings passed where UID expected)
 *
 */

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
#include <sys/ldr.h>

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
}

int flushPtrace()
{
    return(0);
}

void forkNodeProcesses(process *curr, traceHeader *hr, traceFork *fr)
{
    abort();
}


/*
 * Define the various classes of library functions to inst. 
 *
 */
void initLibraryFunctions()
{
    /* XXXX - This function seems to be obsolete with the addition of MDL.
     *   Why is it still here? - jkh 6/30/95
     */
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

void osDependentInst(process *proc) { }
