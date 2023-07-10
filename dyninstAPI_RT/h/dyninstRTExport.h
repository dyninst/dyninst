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
#ifndef _DYNINST_RT_EXPORT_H_
#define _DYNINST_RT_EXPORT_H_
#ifndef ASSEMBLER
  /*  This file contains function prototypes that may be useful for 
      dyninst users to directly have access to from their own runtime
      libraries.
  */

#include <stddef.h>

#if !defined(DLLEXPORT)
#if defined (_MSC_VER)
/* If we're on Windows, we need to explicetely export these functions: */
#define DLLEXPORT __declspec(dllexport) 
#else
#define DLLEXPORT  __attribute__ ((visibility ("default")))
#endif
#endif
  /*
    DYNINSTuserMessage(void *msg, unsigned int msg_size) may be used 
    in conjunction with the dyninstAPI method 
    BPatch_process::registerUserMessageCallback(), to implement a generic
    user-defined, asynchronous communications protocol from the mutatee
    (via this runtime library) to the mutator.

    Calls to DYNINSTuserMessage() will result in <msg> (of <msg_size> bytes)
    being sent to the mutator, and then passed to the callback function
    provided by the API user via registerUserMessageCallback().

    Returns zero on success, nonzero on failure.
  */
DLLEXPORT int DYNINSTuserMessage(void *msg, unsigned int msg_size);

/* Returns the number of threads DYNINST currently knows about.  (Which
   may differ at certain times from the number of threads actually present.) */
DLLEXPORT int DYNINSTthreadCount(void);

/**
 * These function implement a locking mechanism that can be used by 
 * a user's runtime library.
 * 
 * Be sure to always check for DYNINST_LIVE_LOCK and DYNINST_DEAD_LOCK.
 * When instrumenting multithread or signal-based application as these error
 * conditions can trigger even on simple synchronization mechanisms.
 **/

/* The contents of this structure are subject to change between
   dyninst versions.  Don't rely on it. */
typedef void * dyntid_t;
#define DYNINST_INITIAL_LOCK_PID ((void *)-129)

typedef struct {
   volatile int mutex;
   dyntid_t tid;
} dyninst_lock_t;

/* Return values for 'dyninst_lock' */
#define DYNINST_LIVE_LOCK      -1
#define DYNINST_DEAD_LOCK      -2

/* Declare a lock already initialized */
#define DECLARE_DYNINST_LOCK(lck) dyninst_lock_t lck = {0, DYNINST_INITIAL_LOCK_PID}

DLLEXPORT void dyninst_init_lock(dyninst_lock_t *lock);
DLLEXPORT void dyninst_free_lock(dyninst_lock_t *lock);
DLLEXPORT int dyninst_lock(dyninst_lock_t *lock);
DLLEXPORT void dyninst_unlock(dyninst_lock_t *lock);

/**
 * Internal functions that we export to ensure they show up.
 **/

DLLEXPORT void DYNINSTsafeBreakPoint(void);
DLLEXPORT void DYNINSTinit(void);
DLLEXPORT void DYNINST_snippetBreakpoint(void);
DLLEXPORT void DYNINST_stopThread(void *, void *, void *, void *);
DLLEXPORT void DYNINST_stopInterProc(void *, void *, void *, void *, void *, void *);
DLLEXPORT void *DYNINSTos_malloc(size_t, void *, void *); 
DLLEXPORT int DYNINSTloadLibrary(char *);

/** 
 * And variables
 **/

DLLEXPORT extern dyntid_t (*DYNINST_pthread_self)(void);
DLLEXPORT extern unsigned int DYNINSTobsCostLow;
DLLEXPORT extern int libdyninstAPI_RT_init_localCause;
DLLEXPORT extern int libdyninstAPI_RT_init_localPid;
DLLEXPORT extern int libdyninstAPI_RT_init_maxthreads;
DLLEXPORT extern int libdyninstAPI_RT_init_debug_flag;
DLLEXPORT extern struct DYNINST_bootstrapStruct DYNINST_bootstrap_info;

#endif
#endif
