/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * $Id: dyninstAPI_RT.h,v 1.45 2008/04/15 16:43:43 roundy Exp $
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

#if !defined(target_smallmem)
#define SYN_INST_BUF_SIZE (1024*1024*4)
#else
#define SYN_INST_BUF_SIZE (1024*1024*1)
#endif

#define DYNINST_BREAKPOINT_SIGNUM (SIGRTMIN+4)

#include <stdio.h>
#include <stdint.h>
#include "dyninstRTExport.h"
#include "common/h/compiler_diagnostics.h"

/* If we must make up a boolean type, we should make it unique */
typedef unsigned char RT_Boolean;
static const RT_Boolean RT_TRUE=1;
static const RT_Boolean RT_FALSE=0;

DLLEXPORT extern char gLoadLibraryErrorString[];
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

typedef enum {DSE_undefined, DSE_forkEntry, DSE_forkExit, DSE_execEntry, DSE_execExit, DSE_exitEntry, DSE_loadLibrary, DSE_lwpExit, DSE_snippetBreakpoint, DSE_stopThread,
DSE_userMessage, DSE_dynFuncCall } DYNINST_synch_event_t;

extern int DYNINSTdebugPrintRT; /* control run-time lib debug/trace prints */
#if !defined(RTprintf)
#define RTprintf                if (DYNINSTdebugPrintRT) printf
#endif

#define TARGET_CACHE_WIDTH 128
#define TARGET_CACHE_WAYS 2

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
const char *asyncEventType2str(rtBPatch_asyncEventType);

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
   int ppid;         /*Parent process's pid*/
   dyntid_t tid;     /*Thread library ID for thread*/
   int lwp;          /*OS id for thread*/
   int index;        /*The dyninst index for this thread*/
   void *stack_addr; /*The top of this thread's stack*/
   void *start_pc;   /*The pc of this threads initial function*/
} BPatch_newThreadEventRecord;

#if defined(arch_x86_64) /* cannot use MUTATEE_32 here b/c libdyninstAPI.so compiles this */
/*these are the 32 bit structures for use with 32 bit mutatees on AMD64*/
typedef struct {
  unsigned int call_site_addr;
  unsigned int call_target;
} BPatch_dynamicCallRecord32;

typedef struct {
   int ppid;         /*Parent process's pid*/
   unsigned int tid;     /*Thread library ID for thread*/
   int lwp;          /*OS id for thread*/
   int index;        /*The dyninst index for this thread*/
   unsigned int stack_addr; /*The top of this thread's stack*/
   unsigned int start_pc;   /*The pc of this threads initial function*/
} BPatch_newThreadEventRecord32;
#endif


typedef struct {
   int index;        /*Index of the dead thread*/
} BPatch_deleteThreadEventRecord;

/* Let's define some constants for, well, everything.... */
/* These should be different to avoid unexpected collisions */

#if !defined(DYNINST_SINGLETHREADED)
#define DYNINST_SINGLETHREADED -128
#endif
#define DYNINST_TRACEPIPE_ERRVAL -1
#define DYNINST_PRINTF_ERRVAL -2

#define DYNINST_NOT_IN_HASHTABLE ((unsigned)-1)

DLLEXPORT extern int DYNINST_break_point_event;

typedef struct {
   void *source;
   void *target;
} trapMapping_t;

#define TRAP_HEADER_SIG 0x759191D6
#define DT_DYNINST 0x6D191957

// Suppress warning about flexible array members not valid in C++
// FIXME: invalid flexible array member, traps[], in structure below
DYNINST_DIAGNOSTIC_BEGIN_SUPPRESS_FLEX_ARRAY

struct trap_mapping_header {
   uint32_t signature;
   uint32_t num_entries;
   int32_t pos;
  uint32_t padding;
   uint64_t low_entry;
   uint64_t high_entry;
   trapMapping_t traps[]; //Don't change this to a pointer, despite any compiler warnings
};

DYNINST_DIAGNOSTIC_END_SUPPRESS_FLEX_ARRAY

#define MAX_MEMORY_MAPPER_ELEMENTS 1024

typedef struct {
    long start;
    long size;
} MemoryMapperCopyElement;

typedef struct {
   unsigned long lo;
   unsigned long hi;
   long shift;
   MemoryMapperCopyElement *copyList;
} MemoryMapperElement;

struct MemoryMapper {
   int guard1;
   int guard2;
   int size;
   int padding;
   MemoryMapperElement elements[MAX_MEMORY_MAPPER_ELEMENTS];
};

/* 32/64 bit versions for the mutator */

typedef struct {
   uint32_t lo;
   uint32_t hi;
   uint32_t shift;
   void *copyList;
} MemoryMapperElement32;

typedef struct {
   uint64_t lo;
   uint64_t hi;
   uint64_t shift;
   void *copyList;
} MemoryMapperElement64;

struct MemoryMapper32 {
   int guard1;
   int guard2;
   int size;
   int padding;
   MemoryMapperElement32 elements[MAX_MEMORY_MAPPER_ELEMENTS];
};

struct MemoryMapper64 {
   int guard1;
   int guard2;
   int size;
   int padding;
   MemoryMapperElement64 elements[MAX_MEMORY_MAPPER_ELEMENTS];
};

DLLEXPORT extern struct MemoryMapper RTmemoryMapper;

#include "dyninstRTExport.h"
#endif /* _DYNINSTAPI_RT_H */
