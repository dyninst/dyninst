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

#if !defined(DTHREAD_H_)
#define DTHREAD_H_

#include <stdlib.h>
#include "util.h"
#include <cassert>

#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/variant/get.hpp>
#include <boost/variant/static_visitor.hpp>
#include <boost/variant/variant.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>

#if !defined(os_windows)
#define cap_pthreads
#include <pthread.h>
#else
#include <common/src/ntheaders.h>
#endif

#if !defined(WINAPI)
#define WINAPI
#endif



class PC_EXPORT DThread {
#if defined(cap_pthreads)
   pthread_t thrd;
 public:
   typedef void (*initial_func_t)(void *);
   typedef void dthread_ret_t;
#define DTHREAD_RET_VAL
#else
	HANDLE thrd;
	DWORD tid;
 public:
	typedef LPTHREAD_START_ROUTINE initial_func_t;
	typedef int dthread_ret_t;
	#define DTHREAD_RET_VAL 0
#endif
   bool live;   
 public:
   DThread();
   ~DThread();

   static long self();
   bool spawn(initial_func_t func, void *param);
   bool join();
   long id();
};

template<bool isRecursive>
struct boost_mutex_selector
{
	typedef boost::mutex mutex;
};
template<>
struct boost_mutex_selector<true>
{
	typedef boost::recursive_mutex mutex;
};

template <bool isRecursive = false>
class PC_EXPORT Mutex : public boost_mutex_selector<isRecursive>::mutex {
public:
	typedef Mutex<isRecursive> type;
   
};


template <typename mutex_t = Mutex<false> >
class PC_EXPORT CondVar {
   boost::condition_variable_any cond;
   mutex_t *mutex;
   bool created_mutex;
 public:
   CondVar(mutex_t * m = NULL) : cond(), created_mutex(false) { 
		if(m) {
			mutex = m;
		} else {
			mutex = new mutex_t;
			created_mutex = true;
		}
   }
   ~CondVar() { if(created_mutex) delete mutex; }

   CondVar(CondVar const&) = delete;
   CondVar& operator=(CondVar const&) = delete;
   CondVar(CondVar &&) = delete;
   CondVar& operator=(CondVar &&rhs) = delete;

   void unlock() { mutex->unlock(); }
   bool trylock() { return mutex->try_lock(); }
   void lock() { mutex->lock(); }
   void signal() { cond.notify_one(); }
   void broadcast() { cond.notify_all(); }
   void wait() { cond.wait(*mutex); }

};

template <class Mut = Mutex<false>>
class ScopeLock : public boost::interprocess::scoped_lock<Mut>
{
  public:
  ScopeLock(Mut &mut) : boost::interprocess::scoped_lock<Mut>(mut) {}
};



#endif
