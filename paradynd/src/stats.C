/*
 * Report statistics about dyninst and data collection.
 *
 * $Log: stats.C,v $
 * Revision 1.13  1995/02/16 08:34:50  markc
 * Changed igen interfaces to use strings/vectors rather than char*/igen-arrays
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
 * Revision 1.9  1994/08/02  18:24:34  hollings
 * added clock speed argument to stats.
 *
 * Revision 1.8  1994/07/22  19:21:08  hollings
 * removed mistaken divid by 1Meg for predicted cost.
 *
 * Revision 1.7  1994/07/20  23:29:46  hollings
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

