
#ifndef CONTEXT_HDR
#define CONTEXT_HDR

/*
 * $Log: context.h,v $
 * Revision 1.1  1994/11/02 11:02:55  markc
 * Prototypes for context.C
 *
 */

#include "rtinst/h/trace.h"
#include "dyninst.h"

extern timeStamp startPause;
extern timeStamp endPause;
extern bool applicationPaused;
extern timeStamp elapsedPauseTime;
extern void forkProcess(traceHeader *hr, traceFork *fr);

#endif
