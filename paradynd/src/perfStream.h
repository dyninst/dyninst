

#ifndef PERF_STREAM_H
#define PERF_STREAM_H

/*
 * $Log: perfStream.h,v $
 * Revision 1.3  1995/02/16 08:53:56  markc
 * Corrected error in comments -- I put a "star slash" in the comment.
 *
 * Revision 1.2  1995/02/16  08:34:26  markc
 * Changed igen interfaces to use strings/vectors rather than char igen-arrays
 * Changed igen interfaces to use bool, not Boolean.
 * Cleaned up symbol table parsing - favor properly labeled symbol table objects
 * Updated binary search for modules
 * Moved machine dependnent ptrace code to architecture specific files.
 * Moved machine dependent code out of class process.
 * Removed almost all compiler warnings.
 * Use "posix" like library to remove compiler warnings
 *
 * Revision 1.1  1994/11/01  16:58:08  markc
 * Prototypes
 *
 */

#include "rtinst/h/rtinst.h"

extern void controllerMainLoop();
extern bool firstSampleReceived;
extern bool CMMDhostless;
extern bool synchronousMode;
extern double cyclesPerSecond;
extern time64 firstRecordTime;
extern void createResource(traceHeader *header, struct _newresource *r);
extern void processArchDependentTraceStream();

#endif
