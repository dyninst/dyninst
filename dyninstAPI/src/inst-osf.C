
/*
 * inst-osf.C - osf specifc code for paradynd.
 *
 * $Log: inst-osf.C,v $
 * Revision 1.2  1998/09/15 20:50:59  buck
 * Change to not include files from rtinst/h when compiling for Dyninst API.
 *
 * Revision 1.1  1998/08/25 19:35:07  buck
 * Initial commit of DEC Alpha port.
 *
 *
 */

#include "os.h"
#ifndef BPATCH_LIBRARY
#include "metric.h"
#endif
#include "dyninst.h"
#ifdef BPATCH_LIBRARY
#include "dyninstAPI_RT/h/trace.h"
#else
#include "rtinst/h/trace.h"
#endif
#include "symtab.h"
#include "process.h"
#include "inst.h"

#ifndef BPATCH_LIBRARY
#include "perfStream.h"
#include "context.h"
#endif
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
// These probably should be adjusted for an alpha
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
    // 1.80 usec * 70 Mhz (measured on a SS-5)
    // 25 cycles (read clock) +  26 (startProcessTimer)
    primitiveCosts["DYNINSTstartProcessTimer"] = 51;
    // 3.46 usec * 70 mhz (measured on a SS-5)
    // 61 cycles + 2*25 cycles to read clock
    primitiveCosts["DYNINSTstopProcessTimer"] = 111;

}

#ifdef notdef
int flushPtrace()
{
    return(0);
}

void forkNodeProcesses(process *curr, 
traceHeader *hr, 
traceFork *fr)
{
  abort();
}
#endif

/*
 * Define the various classes of library functions to inst. 
 *
 */
void initLibraryFunctions()
{

}

#ifndef BPATCH_LIBRARY
float computePauseTimeMetric(const metricDefinitionNode *)
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
#endif

#ifdef notdef
void osDependentInst(process *proc) {
}
#endif
