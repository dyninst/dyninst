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
 * $Id: stats.C,v 1.24 2000/07/28 17:21:18 pcroth Exp $
 */

#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/dyninstP.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/util.h"

#ifndef BPATCH_LIBRARY
#include "common/h/Timer.h"
#include "rtinst/h/rtinst.h"
#include "rtinst/h/trace.h"
#include "paradynd/src/metric.h"
#include "paradynd/src/internalMetrics.h"
#include "paradynd/src/init.h"
#include "paradynd/src/dynrpc.h"
#endif

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
#ifndef BPATCH_LIBRARY
    sprintf(errorLine, "    totalPredictedCost = %f\n", 
	totalPredictedCost->getValue());
    logLine(errorLine);
#endif

    sprintf(errorLine, "    %d total points used\n", pointsUsed);
    logLine(errorLine);
    sprintf(errorLine, "    %d mini-tramps used\n", totalMiniTramps);
    logLine(errorLine);
#ifndef BPATCH_LIBRARY
    sprintf(errorLine, "    %d metric/resource pairs enabled\n",metResPairsEnabled);
    logLine(errorLine);
    sprintf(errorLine, "    %d metrics used\n", metricsUsed);
    logLine(errorLine);
    sprintf(errorLine, "    %d foci used\n", fociUsed);
    logLine(errorLine);
#endif
    sprintf(errorLine, "    %d tramp bytes\n", trampBytes);
    logLine(errorLine);
#ifndef BPATCH_LIBRARY
    sprintf(errorLine, "    %d samples delivered\n", samplesDelivered);
    logLine(errorLine);
#endif
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

#ifndef BPATCH_LIBRARY
void printAppStats(struct endStatsRec *stats, float clockSpeed)
{
  if (stats) {
    sprintf(errorLine, "    DYNINSTtotalAlaramExpires %d\n", stats->alarms);
    logLine(errorLine);
#ifdef notdef
    sprintf(errorLine, "    DYNINSTnumReported %d\n", stats->numReported);
    logLine(errorLine);
#endif
    sprintf(errorLine, "    Raw cycle count = %f\n", (double) stats->instCycles);
    logLine(errorLine);

    sprintf(errorLine, "    Total instrumentation (%dMHz clock) cost = %f\n", 
	(int) (clockSpeed/1000000.0), stats->instCycles/(clockSpeed));
    logLine(errorLine);
    sprintf(errorLine, "    Total inst (via prof) = %f\n", stats->instTicks/100.0);
    logLine(errorLine);
    sprintf(errorLine, "    Total handler cost = %f\n", stats->handlerCost);
    logLine(errorLine);
    sprintf(errorLine, "    Total cpu time of program %f\n", stats->totalCpuTime);
    logLine(errorLine);
    sprintf(errorLine, "    Total cpu time (via prof) = %f\n", stats->userTicks/100.0);
    logLine(errorLine);
    sprintf(errorLine, "    Elapsed wall time of program %f\n",stats->totalWallTime);
    logLine(errorLine);
    sprintf(errorLine, "    total data samples %d\n", stats->samplesReported);
    logLine(errorLine);
    sprintf(errorLine, "    sampling rate %f\n", stats->samplingRate);
    logLine(errorLine);
#if defined(i386_unknown_linux2_0)
    sprintf(errorLine, "    total traps hit %d\n", stats->totalTraps);
    logLine(errorLine);
#endif
  }
}
#endif

