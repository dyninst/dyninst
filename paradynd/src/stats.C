/*
 * Report statistics about dyninst and data collection.
 *
 * $Log: stats.C,v $
 * Revision 1.3  1994/07/14 23:30:32  hollings
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
int totalMiniTramps;
int metResPairsEnabled;
double timeCostLastChanged;
HTable<resourceList> fociUsed;
HTable<metric> metricsUsed;
extern internalMetric totalPredictedCost;
int ptraceOtherOps, ptraceOps, ptraceBytes;

void printDyninstStats()
{
    float now;
    extern internalMetric currentPredictedCost;

    now = getCurrentTime(FALSE);
    sprintf(errorLine, "totalPredictedCost = %f\n", 
	totalPredictedCost.getValue()/1000000.0);
    logLine(errorLine);

    sprintf(errorLine, "%d total points used\n", pointsUsed);
    logLine(errorLine);
    sprintf(errorLine, "%d mini-tramps used\n", totalMiniTramps);
    logLine(errorLine);
    sprintf(errorLine, "%d metric/resource pairs enabled\n",metResPairsEnabled);
    logLine(errorLine);
    sprintf(errorLine, "%d metrics used\n", metricsUsed.count());
    logLine(errorLine);
    sprintf(errorLine, "%d foci used\n", fociUsed.count());
    logLine(errorLine);
    sprintf(errorLine, "%d tramp bytes\n", trampBytes);
    logLine(errorLine);
    sprintf(errorLine, "%d samples delivered\n", samplesDelivered);
    logLine(errorLine);
    sprintf(errorLine, "%d ptrace other calls\n", ptraceOtherOps);
    logLine(errorLine);
    sprintf(errorLine, "%d ptrace write calls\n", ptraceOps-ptraceOtherOps);
    logLine(errorLine);
    sprintf(errorLine, "%d ptrace bytes written\n", ptraceBytes);
    logLine(errorLine);
}
