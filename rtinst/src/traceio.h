/*
 * Copyright (c) 1996 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

/*
 * traceio.h
 *
 * This file is part of the real-time instrumentation.  The TRACE
 * macro gets called to buffer performance data on the nodes until it
 * gets sucked out * asynchronously.
 *
 * $Log: traceio.h,v $
 * Revision 1.4  1996/08/16 21:27:53  tamches
 * updated copyright for release 1.1
 *
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

