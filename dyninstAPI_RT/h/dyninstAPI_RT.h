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
 * $Id: dyninstAPI_RT.h,v 1.6 2000/08/07 00:59:36 wylie Exp $
 * This file contains the standard instrumentation functions that are provided
 *   by the run-time instrumentation layer.
 */

#ifndef _DYNINSTAPI_RT_H
#define _DYNINSTAPI_RT_H

/*
 * Define the size of the per process data area.
 *
 *  This should be a power of two to reduce paging and caching shifts.
 *  Note that larger sizes may result in requiring longjumps within
 *  mini-trampolines to reach within this area.
 */

#define SYN_INST_BUF_SIZE (1024*1024*4)

/* We sometimes include this into assembly files, so guard the struct defs. */
#if !defined(__ASSEMBLER__)

#include <stdio.h>
#include "common/h/Types.h"

/* If we must make up a boolean type, we should make it unique */
typedef unsigned char RT_Boolean;
static const RT_Boolean RT_TRUE=1;
static const RT_Boolean RT_FALSE=0;

struct DYNINST_bootstrapStruct {
   int event; /* "event" values:
		 0 --> nothing
		 1 --> end of DYNINSTinit (normal)
		 2 --> end of DYNINSTinit (forked process)
		 3 --> start of DYNINSTexec (before exec) 
	      */
   int pid;
   int ppid; /* parent of forked process */

   char path[512]; /* only used in exec */
};

struct endStatsRec {
    int alarms;
    int numReported;
    float instCycles;
    float instTime;
    float handlerCost;
    float totalCpuTime;
    int samplesReported;
    float samplingRate;
    float totalWallTime;
    int userTicks;
    int instTicks;
    int totalTraps;
};

#if defined(i386_unknown_solaris2_5) || defined(i386_unknown_linux2_0) \
 || defined(i386_unknown_nt4_0)
/*
   The tramp table is used when we need to insert traps in instrumentation
   points. It is used by the trap handler to lookup the base tramp for
   an address (point).

   The table is updated by the paradyn daemon.
*/

#define TRAMPTABLESZ (4096)

#define HASH1(x) ((x) % TRAMPTABLESZ)
#define HASH2(x) (((x) % TRAMPTABLESZ-1) | 1)

typedef struct trampTableEntryStruct trampTableEntry;
struct trampTableEntryStruct {
  unsigned key;
  unsigned val;
};

#endif

struct rpcInfoStruct {
  int runningInferiorRPC;  /* 1 running irpc, 0 not running */
  unsigned begRPCAddr;
  unsigned endRPCAddr;
};
typedef struct rpcInfoStruct rpcInfo;
extern rpcInfo curRPC;
extern unsigned pcAtLastIRPC;
extern int trapNotHandled;  /* 1 a trap hasn't been handled, 0 traps handled */

extern int DYNINSTdebugPrintRT; /* control run-time lib debug/trace prints */
#define RTprintf                if (DYNINSTdebugPrintRT) printf
                                /* Yes, this is crude, but this is plain C */

#endif /* !defined(__ASSEMBLER__) */

#endif /* _DYNINSTAPI_RT_H */
