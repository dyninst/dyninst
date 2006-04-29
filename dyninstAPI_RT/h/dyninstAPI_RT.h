/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
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
 * $Id: dyninstAPI_RT.h,v 1.37 2006/04/29 19:04:30 chadd Exp $
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
#define DYNINST_BREAKPOINT_SIGNUM SIGUSR2

#include <stdio.h>
#include "dyninstRTExport.h"
#include "common/h/Types.h"

/* If we must make up a boolean type, we should make it unique */
typedef unsigned char RT_Boolean;
static const RT_Boolean RT_TRUE=1;
static const RT_Boolean RT_FALSE=0;

extern char gLoadLibraryErrorString[];
extern void *gBRKptr;

struct DYNINST_bootstrapStruct {
   int event; /* "event" values:
		 0 --> nothing
		 1 --> end of DYNINSTinit (normal)
		 2 --> end of DYNINSTinit (forked process)
		 3 --> start of DYNINSTexec (before exec) 
	      */
   int pid;
   int ppid; /* parent of forked process */
};

typedef enum {DSE_undefined, DSE_forkEntry, DSE_forkExit, DSE_execEntry, DSE_execExit, DSE_exitEntry, DSE_loadLibrary, DSE_lwpExit} DYNINST_synch_event_t;

extern int DYNINSTdebugPrintRT; /* control run-time lib debug/trace prints */
#if !defined(RTprintf)
#define RTprintf                if (DYNINSTdebugPrintRT) printf
#endif

#define THREAD_AWAITING_DELETION -2

#define ERROR_STRING_LENGTH 256
typedef enum {
  rtBPatch_nullEvent,
  rtBPatch_newConnectionEvent,
  rtBPatch_internalShutDownEvent,
  rtBPatch_threadCreateEvent,
  rtBPatch_threadDestroyEvent,
  rtBPatch_dynamicCallEvent,
  rtBPatch_userEvent
} rtBPatch_asyncEventType;
char *asyncEventType2str(rtBPatch_asyncEventType);

typedef struct {
  unsigned int pid;
  rtBPatch_asyncEventType type;
  unsigned int event_fd;
  unsigned int size;
} rtBPatch_asyncEventRecord;


typedef struct {
  void *call_site_addr;
  void *call_target;
} BPatch_dynamicCallRecord;

typedef struct {
   int ppid;         //Parent process's pid
   dyntid_t tid;     //Thread library ID for thread
   int lwp;          //OS id for thread
   int index;        //The dyninst index for this thread
   void *stack_addr; //The top of this thread's stack
   void *start_pc;   //The pc of this threads initial function
} BPatch_newThreadEventRecord;

#if defined(arch_x86_64) /* cannot use MUTATEE_32 here b/c libdyninstAPI.so compiles this */
/*these are the 32 bit structures for use with 32 bit mutatees on AMD64*/
typedef struct {
  unsigned int call_site_addr;
  unsigned int call_target;
} BPatch_dynamicCallRecord32;

typedef struct {
   int ppid;         //Parent process's pid
   unsigned int tid;     //Thread library ID for thread
   int lwp;          //OS id for thread
   int index;        //The dyninst index for this thread
   unsigned int stack_addr; //The top of this thread's stack
   unsigned int start_pc;   //The pc of this threads initial function
} BPatch_newThreadEventRecord32;
#endif


typedef struct {
   int index;        //Index of the dead thread
} BPatch_deleteThreadEventRecord;

#define UNALLOC 0
#define THREAD_ACTIVE 1
#define THREAD_COMPLETE 2
#define LWP_EXITED 3

typedef struct {
    dyntid_t tid;
    int thread_state;
    int next_free;
} dyninst_thread_t;

extern dyninst_thread_t *DYNINST_thread_structs;
/* We keep the name here so that the Dyninst lib can use it - this means we
   don't have to worry about parallel changes */

#define DYNINST_thread_structs_name "DYNINST_thread_structs"
extern int *DYNINST_thread_hash;
extern unsigned DYNINST_thread_hash_size;

/* Let's define some constants for, well, everything.... */
/* These should be different to avoid unexpected collisions */

#define DYNINST_SINGLETHREADED -128
#define DYNINST_TRACEPIPE_ERRVAL -1
#define DYNINST_PRINTF_ERRVAL -2

#include "dyninstRTExport.h"
#endif /* _DYNINSTAPI_RT_H */
