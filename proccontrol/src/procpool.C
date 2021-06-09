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
#include "procpool.h"
#include "int_process.h"
#include "PCErrors.h"

#include <assert.h>

ProcessPool *ProcPool()
{
   static ProcessPool *ppool = NULL;
   if (!ppool) {
      ppool = new ProcessPool();
   }
   return ppool;
}

ProcessPool::ProcessPool()
{
}

ProcessPool::~ProcessPool()
{
}

int_process *ProcessPool::findProcByPid(Dyninst::PID pid)
{
   std::map<Dyninst::PID, int_process *>::iterator i = procs.find(pid);
   if (i == procs.end())
      return NULL;
   return (*i).second;
}
void ProcessPool::addProcess(int_process *proc)
{
   pthrd_printf("Adding process %d to pool\n", proc->getPid());

   std::map<Dyninst::PID, int_process *>::iterator i = procs.find(proc->getPid());
   assert(i == procs.end());
   procs[proc->getPid()] = proc;
}

void ProcessPool::rmProcess(int_process *proc)
{
   pthrd_printf("Removing process %d from pool\n", proc->getPid());
   std::map<Dyninst::PID, int_process *>::iterator i = procs.find(proc->getPid());
   assert(i != procs.end());
   procs.erase(i);

   int_threadPool *tpool = proc->threadPool();
   if (!tpool)
      return;
   for (auto t : *tpool) {
      rmThread(t);
   }
}

bool ProcessPool::for_each(ifunc f, void *data)
{
	condvar()->lock();
   std::map<Dyninst::PID, int_process *>::iterator i;
   for (i = procs.begin(); i != procs.end(); ++i) {
      bool result = f(i->second, data);
	  if (!result) {
			condvar()->broadcast();
			condvar()->unlock();
		  return false;
	  }
   }
	condvar()->broadcast();
	condvar()->unlock();
   return true;
}

CondVar<> *ProcessPool::condvar()
{
   return &var;
}

void ProcessPool::addThread(int_process * /*proc*/, int_thread *thr)
{
   if (!LWPIDsAreUnique())
      return;
   std::map<Dyninst::LWP, int_thread *>::iterator i = lwps.find(thr->getLWP());
   assert(i == lwps.end());
   lwps[thr->getLWP()] = thr;
   // Un-kill if a LWP has been recycled 
   // (because we've run long enough?)
   std::set<Dyninst::LWP>::iterator found = deadThreads.find(thr->getLWP());
   if(found != deadThreads.end()) deadThreads.erase(found);
   
}

void ProcessPool::rmThread(int_thread *thr)
{
   if (!LWPIDsAreUnique())
      return;
   std::map<Dyninst::LWP, int_thread *>::iterator i = lwps.find(thr->getLWP());
   addDeadThread(thr->getLWP());
   assert(i != lwps.end());
   lwps.erase(i);
}

int_thread *ProcessPool::findThread(Dyninst::LWP lwp)
{
   if (!LWPIDsAreUnique()) {
      return NULL;
   }
   std::map<Dyninst::LWP, int_thread *>::iterator i = lwps.find(lwp);
   if (i == lwps.end())
      return NULL;
   return (*i).second;
}

bool ProcessPool::deadThread(Dyninst::LWP lwp) {
   return (deadThreads.find(lwp) != deadThreads.end());
}

void ProcessPool::addDeadThread(Dyninst::LWP lwp) {
   deadThreads.insert(lwp);
}
void ProcessPool::removeDeadThread(Dyninst::LWP lwp) {
    // Called when we get a LWP create, as that had *better*
    // not be for an alread-dead thread.
    deadThreads.erase(lwp);
}

unsigned ProcessPool::numProcs()
{
   return (unsigned) procs.size();
}
