
/*
 * inst-aix.C - AIX specifc code for paradynd.
 *
 * XXX - The following functions seem to be less than OS dependent, but I
 *   have included them here to reduce the number of files changed in the
 *   AIX port. - jkh 6/30/95.
 *	process::getProcessStatus()
 *	computePauseTimeMetric()
 *
 * $log: inst-aix.C,v$
 * Revision 1.1  1995/08/24  15:03:55  hollings
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
    primitiveCosts["DYNINSTprintCost"] = 1;

    // this doesn't really take any time
    primitiveCosts["DYNINSTbreakPoint"] = 1;

    // this happens before we start keeping time.
    primitiveCosts["DYNINSTinit"] = 1;

    // nene acutal numbers from 1/26/96 -- jkh
    // 240 ns
    primitiveCosts["DYNINSTincrementCounter"] = 16;
    // 240 ns
    //primitiveCosts["DYNINSTdecrementCounter"] = 16;

    // 0.8 usec * 66 mhz (measured on a POWER-2 RS600)
    //primitiveCosts["DYNINSTstartWallTimer"] = 53;
    // 1.0 usec * 66 mhz (measured on a POWER-2 RS600)
    //primitiveCosts["DYNINSTstopWallTimer"] = 66;
    // 2.4 usec * 66 Mhz (measured on a POWER-2 RS600)
    //primitiveCosts["DYNINSTstartProcessTimer"] = 160;
    // 4.8 usec * 66 Mhz (measured on a POWER-2 RS600)
    //primitiveCosts["DYNINSTstopProcessTimer"] = 317;

    logLine("IBM platform\n");
    // Updated calculation of the cost for the following procedures.
    // Clock rate = 64 Mhz (rs6000) - naim
    // NOTE: These measurements may need some tune up (02-09-96) - naim
    // 1.5 usecs * 64 Mhz  
    primitiveCosts["DYNINSTstartWallTimer"] = 96;
    // 3.01 usecs * 64 Mhz
    primitiveCosts["DYNINSTstopWallTimer"] = 192;
    // 10.09 usecs * 64 Mhz
    primitiveCosts["DYNINSTstartProcessTimer"] = 645;
    // 20.28 usecs * 64 Mhz
    primitiveCosts["DYNINSTstopProcessTimer"] = 1297;    

    // These happen async of the rest of the system.
    // 33.74 usecs * 64 Mhz
    primitiveCosts["DYNINSTalarmExpire"] = 2159;
    // 0.38 usecs * 64 Mhz
    primitiveCosts["DYNINSTsampleValues"] = 24;
    // 11.52 usecs * 64 Mhz
    primitiveCosts["DYNINSTreportTimer"] = 737;
    // 1.14 usecs * 64 Mhz
    primitiveCosts["DYNINSTreportCounter"] = 72;
    // 2.07 usecs * 64 Mhz
    primitiveCosts["DYNINSTreportCost"] = 131;
    // 0.66 usecs * 64 Mhz
    primitiveCosts["DYNINSTreportNewTags"] = 42;
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

void osDependentInst(process *proc) { }
