
#ifndef STATS_H
#define STATS_H

/*
 * $Log: stats.h,v $
 * Revision 1.1  1994/11/01 16:58:13  markc
 * Prototypes
 *
 */

extern void printDyninstStats();
extern time64 totalInstTime;
extern int insnGenerated;
extern int totalMiniTramps;
extern int trampBytes;
extern void printAppStats(endStatsRec *stats, float clock);
extern int samplesDelivered;
extern int metResPairsEnabled;
extern int ptraceOps;
extern int ptraceOtherOps;
extern int ptraceBytes;

#endif
