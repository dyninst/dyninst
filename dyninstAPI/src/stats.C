/*
 * Report statistics about dyninst and data collection.
 *
 * $Log: stats.C,v $
 * Revision 1.7  1994/07/20 23:29:46  hollings
 * made time in code gen stat stadard.
 *
 * Revision 1.6  1994/07/20  23:23:40  hollings
 * added insn generated metric.
 *
 * Revision 1.5  1994/07/16  03:38:50  hollings
 * fixed stats to not devidi by 1meg, fixed negative time problem.
 *
 * Revision 1.4  1994/07/15  04:19:13  hollings
 * moved dyninst stats to stats.C
 *
 * Revision 1.3  1994/07/14  23:30:32  hollings
 * Hybrid cost model added.
 *
 * Revision 1.2  1994/07/05  03:26:18  hollings
 * observed cost model
 *
 * Revision 1.1  1994/01/27  20:31:42  hollings
 * Iinital version of paradynd speaking dynRPC igend protocol.
 *
 * Revision 1.2  1993/12/14  23:00:54  jcargill
 * Nothing important -- just typos
 *
 * Revision 1.1  1993/12/13  19:55:59  hollings
 * Initial revision
 *
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

int trampBytes;
int pointsUsed;
int samplesDelivered;
int insnGenerated;
int totalMiniTramps;
int metResPairsEnabled;
double timeCostLastChanged;
HTable<resourceList> fociUsed;
HTable<metric> metricsUsed;
extern internalMetric totalPredictedCost;
int ptraceOtherOps, ptraceOps, ptraceBytes;



time64 totalInstTime;

void printDyninstStats()
{
    float now;

    now = getCurrentTime(FALSE);
    sprintf(errorLine, "    totalPredictedCost = %f\n", 
	totalPredictedCost.getValue()/1000000.0);
    logLine(errorLine);

    sprintf(errorLine, "    %d total points used\n", pointsUsed);
    logLine(errorLine);
    sprintf(errorLine, "    %d mini-tramps used\n", totalMiniTramps);
    logLine(errorLine);
    sprintf(errorLine, "    %d metric/resource pairs enabled\n",metResPairsEnabled);
    logLine(errorLine);
    sprintf(errorLine, "    %d metrics used\n", metricsUsed.count());
    logLine(errorLine);
    sprintf(errorLine, "    %d foci used\n", fociUsed.count());
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
	((double) totalInstTime)/1000000.0);
    logLine(errorLine);
}

void printAppStats(struct endStatsRec *stats)
{
    sprintf(errorLine, "    DYNINSTtotalAlaramExpires %d\n", stats->alarms);
    logLine(errorLine);
#ifdef notdef
    sprintf(errorLine, "    DYNINSTnumReported %d\n", stats->numReported);
    logLine(errorLine);
#endif
    sprintf(errorLine,"    Raw cycle count = %f\n", (double) stats->instCycles);
    logLine(errorLine);

    // for ss-10 use 60 MHZ clock.
    sprintf(errorLine,"    Total instrumentation (60Mhz clock) cost = %f\n", 
	stats->instCycles/60000000.0);
    logLine(errorLine);
    sprintf(errorLine,"    Total handler cost = %f\n", stats->handlerCost);
    logLine(errorLine);
    sprintf(errorLine,"    Total cpu time of program %f\n", stats->totalCpuTime);
    logLine(errorLine);
    sprintf(errorLine,"    Elapsed wall time of program %f\n",stats->totalWallTime);
    logLine(errorLine);
    sprintf(errorLine,"    total data samples %d\n", stats->samplesReported);
    logLine(errorLine);
    sprintf(errorLine,"    sampling rate %f\n", stats->samplingRate);
    logLine(errorLine);
}

