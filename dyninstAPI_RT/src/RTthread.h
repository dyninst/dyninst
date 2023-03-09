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

#ifndef _RTTHREAD_H_
#define _RTTHREAD_H_

#include "dyninstAPI_RT/h/dyninstAPI_RT.h"
#include "dyninstAPI_RT/h/dyninstRTExport.h"

DLLEXPORT dyntid_t dyn_pthread_self(void);    /*Thread library identifier*/
int dyn_lwp_self(void);    /*LWP used by the kernel identifier*/
int dyn_pid_self(void);    /*PID identifier representing the containing process*/

extern int DYNINST_multithread_capable;

typedef dyninst_lock_t tc_lock_t;

#define DECLARE_TC_LOCK(l)         tc_lock_t l={0 ,(dyntid_t)-1}

int tc_lock_init(tc_lock_t*);
int tc_lock_lock(tc_lock_t*);
int tc_lock_unlock(tc_lock_t*);
int tc_lock_destroy(tc_lock_t*);


int DYNINST_am_initial_thread(dyntid_t tid);

#endif
