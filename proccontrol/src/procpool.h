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
#if !defined(PROCPOOL_H_)
#define PROCPOOL_H_

#include <set>
#include <map>
#include "dyntypes.h"
#include "common/src/dthread.h"

using namespace Dyninst;

class int_process;
class int_thread;

class ProcessPool
{
   friend ProcessPool *ProcPool();
 protected:
   std::set<Dyninst::LWP> deadThreads;
   std::map<Dyninst::PID, int_process *> procs;
   std::map<Dyninst::LWP, int_thread *> lwps;
   ProcessPool();
   CondVar<> var;
 public:
   ~ProcessPool();
   typedef bool(*ifunc)(int_process *, void *data);

   int_process *findProcByPid(Dyninst::PID pid);
   void addProcess(int_process *proc);
   void addThread(int_process *proc, int_thread *thr);
   void rmProcess(int_process *proc);
   void rmThread(int_thread *thr);
   int_thread *findThread(Dyninst::LWP lwp);
   // On Linux, we can get notifications for dead threads. Fun. 
   bool deadThread(Dyninst::LWP lwp);
   void addDeadThread(Dyninst::LWP lwp);
   void removeDeadThread(Dyninst::LWP lwp);
   unsigned numProcs();
   bool LWPIDsAreUnique();
   bool for_each(ifunc f, void *data = NULL);
   CondVar<> *condvar();
};

ProcessPool *ProcPool();

#endif
