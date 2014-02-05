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

#include "common/src/dthread.h"
#include <pthread.h>
#include <assert.h>

DThread::DThread() :
   thrd(0),
   live(false)
{
}

DThread::~DThread()
{
}

typedef struct {
   DThread::initial_func_t func;
   void *param;
} initial_data;

static void *thread_init(void *d) {
   assert(d);
   initial_data *data = (initial_data *) d;
   DThread::initial_func_t func = data->func;
   void *param = data->param;
   delete data;
   func(param);
   return NULL;
}

bool DThread::spawn(initial_func_t func, void *param)
{
   initial_data *data = new initial_data();
   data->func = func;
   data->param = param;
   int result = pthread_create(&thrd, NULL, thread_init, data);
   assert(result == 0);
   if(result) return false;
   live = true;
   return true;
}

bool DThread::join()
{
   assert(live && pthread_self() != thrd);
   int result = pthread_join(thrd, NULL);
   return (result == 0);
}

long DThread::id()
{
   if (!live) return 0;
   return (long) thrd;
}

long DThread::self()
{
   return (long) pthread_self();
}

