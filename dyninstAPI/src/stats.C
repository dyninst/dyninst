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
 * Report statistics about dyninst and data collection.
 *
 * $Log: stats.C,v $
 * Revision 1.16  1996/08/16 21:19:52  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.15  1995/08/24 15:04:32  hollings
 * AIX/SP-2 port (including option for split instruction/data heaps)
 * Tracing of rexec (correctly spawns a paradynd if needed)
 * Added rtinst function to read getrusage stats (can now be used in metrics)
 * Critical Path
 * Improved Error reporting in MDL sematic checks
 * Fixed MDL Function call statement
 * Fixed bugs in TK usage (strings passed where UID expected)
 *
 * Revision 1.14  1995/02/16  08:54:16  markc
 * Corrected error in comments -- I put a "star slash" in the comment.
 *
 * Revision 1.13  1995/02/16  08:34:50  markc
 * Changed igen interfaces to use strings/vectors rather than char igen-arrays
 * Changed igen interfaces to use bool, not Boolean.
 * Cleaned up symbol table parsing - favor properly labeled symbol table objects
 * Updated binary search for modules
 * Moved machine dependnent ptrace code to architecture specific files.
 * Moved machine dependent code out of class process.
 * Removed almost all compiler warnings.
 * Use "posix" like library to remove compiler warnings
 *
 * Revision 1.12  1994/11/02  11:19:41  markc
 * Removed compiler warnings.
 *
 * Revision 1.11  1994/09/22  02:25:13  markc
 * Change names of resource classes
 *
 * Revision 1.10  1994/09/20  18:18:31  hollings
 * added code to use actual clock speed for cost model numbers.
 *
 */

#include "rtinst/h/rtinst.h"
#include "rtinst/h/trace.h"
#include "symtab.h"
#include "process.h"
#include "inst.h"
#include "instP.h"
#include "dyninstP.h"
#include "metric.h"
#include "ast.h"
#include "util.h"
#include "internalMetrics.h"
#include "dynrpc.h"
#include "init.h"
#include "util/h/Timer.h"

int trampBytes = 0;
int pointsUsed=0;
int samplesDelivered=0;
int insnGenerated = 0;
int totalMiniTramps=0;
int metResPairsEnabled=0;
double timeCostLastChanged=0;
// HTable<resourceListRec*> fociUsed;
// HTable<metric*> metricsUsed;
int metricsUsed = 0;
int fociUsed = 0;
int ptraceOtherOps=0, ptraceOps=0, ptraceBytes=0;
timer totalInstTime;

void printDyninstStats()
{
    float now;

    now = getCurrentTime(false);
    sprintf(errorLine, "    totalPredictedCost = %f\n", 
	totalPredictedCost->getValue());
    logLine(errorLine);

    sprintf(errorLine, "    %d total points used\n", pointsUsed);
    logLine(errorLine);
    sprintf(errorLine, "    %d mini-tramps used\n", totalMiniTramps);
    logLine(errorLine);
    sprintf(errorLine, "    %d metric/resource pairs enabled\n",metResPairsEnabled);
    logLine(errorLine);
    sprintf(errorLine, "    %d metrics used\n", metricsUsed);
    logLine(errorLine);
    sprintf(errorLine, "    %d foci used\n", fociUsed);
    logLine(errorLine);
    sprintf(errorLine, "    %d tramp bytes\n", trampBytes);
    logLine(errorLine);
    sprintf(errorLine, "    %d samples delivered\n", samplesDelivered);
    logLine(errorLine);
    sprintf(errorLine, "    %d ptrace other calls\n", ptraceOtherOps);
    logLine(errorLine);
    sprintf(errorLine, "    %d ptrace write calls\n", ptraceOps-ptraceOtherOps);
    logLine(errorLine);
    sprintf(errorLine, "    %d ptrace bytes written\n", ptraceBytes);
    logLine(errorLine);
    sprintf(errorLine, "    %d instructions generated\n", insnGenerated);
    logLine(errorLine);
    sprintf(errorLine, "    %f time used to generate instrumentation\n",
	totalInstTime.wsecs());
    logLine(errorLine);
}

void printAppStats(struct endStatsRec *stats, float clockSpeed)
{
    sprintf(errorLine, "    DYNINSTtotalAlaramExpires %d\n", stats->alarms);
    logLine(errorLine);
#ifdef notdef
    sprintf(errorLine, "    DYNINSTnumReported %d\n", stats->numReported);
    logLine(errorLine);
#endif
    sprintf(errorLine,"    Raw cycle count = %f\n", (double) stats->instCycles);
    logLine(errorLine);

    sprintf(errorLine,"    Total instrumentation (%dMhz clock) cost = %f\n", 
	(int) (clockSpeed/1000000.0), stats->instCycles/(clockSpeed));
    logLine(errorLine);
    sprintf(errorLine,"    Total inst (via prof) = %f\n", stats->instTicks/100.0);
    logLine(errorLine);
    sprintf(errorLine,"    Total handler cost = %f\n", stats->handlerCost);
    logLine(errorLine);
    sprintf(errorLine,"    Total cpu time of program %f\n", stats->totalCpuTime);
    logLine(errorLine);
    sprintf(errorLine,"     Total cpu time (via prof) = %f\n", stats->userTicks/100.0);
    logLine(errorLine);
    sprintf(errorLine,"    Elapsed wall time of program %f\n",stats->totalWallTime);
    logLine(errorLine);
    sprintf(errorLine,"    total data samples %d\n", stats->samplesReported);
    logLine(errorLine);
    sprintf(errorLine,"    sampling rate %f\n", stats->samplingRate);
    logLine(errorLine);
}

