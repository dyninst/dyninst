/*
 * Report statistics about dyninst and data collection.
 *
 * $Log: stats.C,v $
 * Revision 1.2  1994/07/05 03:26:18  hollings
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
int totalMiniTramps;
int metResPairsEnabled;
double timeCostLastChanged;
double totalPredictedCost;
HTable<resourceList> fociUsed;
HTable<metric> metricsUsed;
int ptraceOtherOps, ptraceOps, ptraceBytes;

void printDyninstStats()
{
    float now;
    extern internalMetric currentPredictedCost;

    now = getCurrentTime(FALSE);
    totalPredictedCost += (now - timeCostLastChanged) * 
	currentPredictedCost.value;
    sprintf(errorLine, "totalPredictedCost = %f\n", totalPredictedCost);
    logLine(errorLine);

    printf("%d total points used\n", pointsUsed);
    printf("%d mini-tramps used\n", totalMiniTramps);
    printf("%d metric/resource pairs enabled\n", metResPairsEnabled);
    printf("%d metrics used\n", metricsUsed.count());
    printf("%d foci used\n", fociUsed.count());
    printf("%d tramp bytes\n", trampBytes);
    printf("%d samples delivered\n", samplesDelivered);
    printf("%d ptrace other calls\n", ptraceOtherOps);
    printf("%d ptrace write calls\n", ptraceOps-ptraceOtherOps);
    printf("%d ptrace bytes written\n", ptraceBytes);
}
