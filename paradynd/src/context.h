
#ifndef CONTEXT_HDR
#define CONTEXT_HDR

/*
 * $Log: context.h,v $
 * Revision 1.2  1994/11/09 18:39:56  rbi
 * the "Don't Blame Me" commit
 *
 * Revision 1.1  1994/11/02  11:02:55  markc
 * Prototypes for context.C
 *
 */

#include "rtinst/h/trace.h"
#include "dyninst.h"

extern timeStamp startPause;
extern timeStamp endPause;
extern timeStamp elapsedPauseTime;
extern void forkProcess(traceHeader *hr, traceFork *fr);

#endif
