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

/* $Id: inst-aix.C,v 1.17 2002/01/17 16:22:53 schendel Exp $
 * inst-aix.C - AIX-specific code for paradynd.
 *
 * XXX - The following functions seem to be less than OS dependent, but I
 *   have included them here to reduce the number of files changed in the
 *   AIX port. - jkh 6/30/95.
 *	process::getProcessStatus()
 *	computePauseTimeMetric()
 */

#include "dyninstAPI/src/os.h"
#include "dyninstAPI/src/dyninst.h"
#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/ptrace_emul.h"
#include "dyninstAPI/src/util.h"
#include "dyninstAPI/src/stats.h"
#include <sys/ldr.h>

#ifndef BPATCH_LIBRARY
#include "paradynd/src/metric.h"
#include "dyninstRPC.xdr.SRVR.h"
#include "paradynd/src/main.h"
#include "paradynd/src/perfStream.h"
#include "paradynd/src/context.h"
#include "dyninstAPI/src/dyninstP.h" // isApplicationPaused
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

    logLine("IBM platform\n");
    // Updated calculation of the cost for the following procedures.

    // Values (in cycles) benchmarked on a PowerPC POWER3 375MHz
    // Level 1 - Hardware Level (on by default)
    primitiveCosts["DYNINSTstartWallTimer"] = 321;
    primitiveCosts["DYNINSTstopWallTimer"]  = 325;

    // Values (in cycles) benchmarked on a PowerPC POWER3 375MHz
    // Level 2 - Software Level
    //primitiveCosts["DYNINSTstartWallTimer"]  = 1285;
    //primitiveCosts["DYNINSTstopWallTimer"]   = 1297;
    primitiveCosts["DYNINSTstartProcessTimer"] = 1307;
    primitiveCosts["DYNINSTstopProcessTimer"]  = 1311;

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
 
