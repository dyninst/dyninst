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

Mutex::Mutex(bool recursive)
{
   pthread_mutexattr_t attr;
   pthread_mutexattr_init(&attr);
   if (recursive) {
      pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
   }
   int result = pthread_mutex_init(&mutex, &attr);
   assert(result == 0);

   pthread_mutexattr_destroy(&attr);
}

#include <stdio.h>
#include <errno.h>
#include <string.h>


Mutex::~Mutex()
{
   int result = pthread_mutex_destroy(&mutex);
   assert(result == 0);
}

bool Mutex::lock()
{
   int result = pthread_mutex_lock(&mutex);
   return (result == 0);
}

bool Mutex::trylock()
{
  int result = pthread_mutex_trylock(&mutex);
  return (result == 0);
}

bool Mutex::unlock()
{
   int result = pthread_mutex_unlock(&mutex);
   return (result == 0);
}

CondVar::CondVar(Mutex *m)
{
   if (!m) {
      created_mutex = true;
      mutex = new Mutex();
   }
   else {
      created_mutex = false;
      mutex = static_cast<Mutex *>(m);
   }
   pthread_cond_init(&cond, NULL);
}

CondVar::~CondVar()
{
   if (created_mutex)
      delete mutex;
   int result = pthread_cond_destroy(&cond);
   assert(result == 0);
}

bool CondVar::unlock()
{
   return mutex->unlock();
}

bool CondVar::lock()
{
   return mutex->lock();
}

bool CondVar::trylock()
{
  return mutex->trylock();
}

bool CondVar::signal()
{
   int result = pthread_cond_signal(&cond);
   return result == 0;
}

bool CondVar::broadcast()
{
   int result = pthread_cond_broadcast(&cond);
   return result == 0;
}

bool CondVar::wait()
{
   int result = pthread_cond_wait(&cond, &mutex->mutex);
   return result == 0;
}

