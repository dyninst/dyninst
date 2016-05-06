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

#ifndef __TEST12_H__
#define __TEST12_H__

#include "test_lib_test9.h"

#define FAIL_MES(x,y) logerror("**Failed %s (%s)\n", x,y);
#define PASS_MES(x,y) logerror("Passed %s (%s)\n", x,y);
#define SKIP(x,y) logerror("Skipped %s (%s)\n", x,y);

#define TEST1_THREADS 10
#define TEST3_THREADS 10
#define TEST4_THREADS 10
#define TEST5_THREADS 10 
#define TEST6_THREADS 10
#define TEST7_THREADS 10
#define TEST8_THREADS 10


#define TEST7_NUMCALLS 10 /* number of callpoint messages we expect in subetst7 */
#define MAX_TEST 8 
#define TIMEOUT 15000 /* ms */
#define SLEEP_INTERVAL 10 /*100 ms*/
#if defined (os_windows_test)
#error
#else
#define MUTEX_INIT_FUNC "pthread_mutex_init"
#define MUTEX_LOCK_FUNC "pthread_mutex_lock"
#define MUTEX_UNLOCK_FUNC "pthread_mutex_unlock"
#define MUTEX_DESTROY_FUNC "pthread_mutex_destroy"
#endif
typedef enum {
   null_event = 3,
   mutex_init = 4,
   mutex_lock = 5,
   mutex_unlock = 6, 
   mutex_destroy = 7,
   func_entry = 8,
   func_callsite = 9,
   func_exit = 10,
   test3_event1,
   test3_event2,
   test3_event3
} user_event_t;

typedef struct {
  unsigned int id;
  user_event_t what; 
  unsigned long tid;
} user_msg_t;

#endif
