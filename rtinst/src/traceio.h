/*
 * traceio.h
 *
 * This file is part of the real-time instrumentation.  The TRACE
 * macro gets called to buffer performance data on the nodes until it
 * gets sucked out * asynchronously.
 *
 * $Log: traceio.h,v $
 * Revision 1.3  1995/12/05 03:25:32  zhichen
 * I fixed the bug the the marco TRACE update TRACELIBcurrPtr even though
 * the buffer is already filled up. (The consequence is a PN bus error)
 *
 * Revision 1.2  1994/07/11  22:47:54  jcargill
 * Major CM5 commit: include syntax changes, some timer changes, removal
 * of old aggregation code, old pause code, added signal-driven sampling
 * within node processes
 *
 */

#define TRACE_BUF_SIZE   65535

#define TRACE(dataPtr, dataLen)                         \
  do {                                                  \
        TRACELIBmustRetry = 0;                          \
        TRACELIBcurrPtr = TRACELIBfreePtr;              \
        TRACELIBfreePtr += dataLen;                     \
        if (TRACELIBendPtr < TRACELIBfreePtr) {         \
            must_end_timeslice();                       \
            TRACELIBfreePtr -= dataLen;                 \
            continue;     /* go back and try again */   \
        }                                               \
        bcopy (dataPtr, TRACELIBcurrPtr, dataLen);      \
  } while (TRACELIBmustRetry);

/********************************************************
 * IT IS USED TO BE AS FOLLOWS,                         *
 * I CHANGED IT TO GET RID OF A pn bUS ERROR            *
 ********************************************************/
/*
#define TRACE(dataPtr, dataLen)                         \
  do {                                                  \
        TRACELIBmustRetry = 0;                          \
        TRACELIBcurrPtr = TRACELIBfreePtr;              \
        TRACELIBfreePtr += dataLen;                     \
        if (TRACELIBendPtr < TRACELIBfreePtr) {         \
            must_end_timeslice();                       \
            continue;      * go back and try again *    \
        }                                               \
        bcopy (dataPtr, TRACELIBcurrPtr, dataLen);      \
        TRACELIBcurrPtr = TRACELIBfreePtr;              \
  } while (TRACELIBmustRetry);
*/

extern char *TRACELIBcurrPtr;	   /* current pointer in buffer  */
extern char *TRACELIBfreePtr;	   /* pointer to next free byte in buffer */
extern char *TRACELIBendPtr;	   /* last byte in trace buffer */
extern char *TRACELIBtraceBuffer;  /* beginning of trace buffer */
extern int TRACELIBmustRetry;	   /* signal variable from consumer -> producer */
				   /* or to put it another way, */
				   /* Handler -> Trace macro */

