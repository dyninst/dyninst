

#ifndef PERF_STREAM_H
#define PERF_STREAM_H

/*
 * $Log: perfStream.h,v $
 * Revision 1.1  1994/11/01 16:58:08  markc
 * Prototypes
 *
 */

#include "rtinst/h/rtinst.h"

extern void controllerMainLoop();
extern bool firstSampleReceived;
extern bool CMMDhostless;
extern bool synchronousMode;
extern float cyclesPerSecond;
extern time64 firstRecordTime;
extern void createResource(traceHeader *header, struct _newresource *r);
extern void processArchDependentTraceStream();

#endif
