
/*
 * Copyright (c) 1996 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

/*
 * inst-pvm.C - sunos specifc code for paradynd.
 *
 * $Log: inst-pvm.C,v $
 * Revision 1.27  1997/09/28 22:22:30  buck
 * Added some more #ifdef BPATCH_LIBRARYs to eliminate some Dyninst API
 * library dependencies on files in rtinst.
 *
 * Revision 1.26  1997/02/21 20:13:28  naim
 * Moving files from paradynd to dyninstAPI + moving references to dataReqNode
 * out of the ast class. The is the first pre-dyninstAPI commit! - naim
 *
 * Revision 1.25  1997/01/16 22:04:34  tamches
 * removed flushPtrace
 *
 * Revision 1.24  1996/10/31 08:48:12  tamches
 * removed forkNodeProcess
 *
 * Revision 1.23  1996/08/16 21:18:57  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.22  1996/04/29 03:36:22  tamches
 * computePauseTimeMetric now takes in a metric (but doesn't use it)
 *
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
 */

extern "C" {
#include "pvm3.h"
}

#include "dyninstAPI/src/dyninst.h"
#ifndef BPATCH_LIBRARY
#include "rtinst/h/trace.h"
#endif
#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/util.h"
#include "util/h/String.h"
#include "util/h/Dictionary.h"
#include "paradynd/src/context.h"
#include "paradynd/src/perfStream.h"
#include "paradynd/src/metric.h"
#include "dyninstAPI/src/os.h"

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
