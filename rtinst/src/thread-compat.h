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
 * Thread compatibility header file
 */

#ifndef _THREAD_COMPAT_
#define _THREAD_COMPAT

#if defined(i386_unknown_linux2_0)
#include <syscall.h>
#endif

#if defined(rs6000_ibm_aix4_1) | defined(i386_unknown_linux2_0)
#include <pthread.h>
typedef pthread_key_t                     dyninst_key_t;
typedef pthread_cond_t                    dyninst_cond_t;
typedef pthread_mutex_t                   dyninst_mutex_t;
typedef pthread_t                         dyninst_t;
#if !defined(i386_unknown_linux2_0)
typedef pthread_rwlock_t                  dyninst_rwlock_t;
#endif

#define P_thread_getspecific(key)         pthread_getspecific(key)
#define P_thread_setspecific(key, val)    pthread_setspecific(key,val)
#define P_thread_key_create(key,dest)     pthread_key_create(key,dest)
#define P_thread_self()                   pthread_self()
#if defined(i386_unknown_linux2_0)
#define P_lwp_self()                      syscall(SYS_gettid)
#elif defined(rs6000_ibm_aix4_1)
#define P_lwp_self()                      thread_self()
#endif
#endif

#if defined(sparc_sun_solaris2_4)
#include <thread.h>
typedef thread_key_t                     dyninst_key_t;
typedef cond_t                           dyninst_cond_t;
typedef mutex_t                          dyninst_mutex_t;
typedef rwlock_t                         dyninst_rwlock_t;
typedef thread_t                         dyninst_t;
typedef sema_t                           dyninst_sema_t;

extern void *P_thread_getspecific(dyninst_key_t);
#define P_thread_setspecific(key, val)    thr_setspecific(key,val)
#define P_thread_key_create(key,dest)     thr_keycreate(key,dest)
#define P_thread_self()                   pthread_self()
#define P_lwp_self()                      _lwp_self()

#endif

#endif
