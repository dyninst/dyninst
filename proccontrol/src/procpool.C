#include "proccontrol/src/procpool.h"
#include "proccontrol/src/int_process.h"
#include "proccontrol/h/PCErrors.h"

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
   for (int_threadPool::iterator i = tpool->begin(); i != tpool->end(); i++) {
      rmThread(*i);
   }
}

bool ProcessPool::for_each(ifunc f, void *data)
{
	condvar()->lock();
   std::map<Dyninst::PID, int_process *>::iterator i;
   for (i = procs.begin(); i != procs.end(); i++) {
      bool result = f(i->second, data);
	  if (!result) {
			condvar()->signal();
			condvar()->unlock();
		  return false;
	  }
   }
	condvar()->signal();
	condvar()->unlock();
   return true;
}

CondVar *ProcessPool::condvar()
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
}

void ProcessPool::rmThread(int_thread *thr)
{
   if (!LWPIDsAreUnique())
      return;
   std::map<Dyninst::LWP, int_thread *>::iterator i = lwps.find(thr->getLWP());
   assert(i != lwps.end());
   lwps.erase(i);
}

int_thread *ProcessPool::findThread(Dyninst::LWP lwp)
{
   assert(LWPIDsAreUnique());
   std::map<Dyninst::LWP, int_thread *>::iterator i = lwps.find(lwp);
   if (i == lwps.end())
      return NULL;
   return (*i).second;
}

unsigned ProcessPool::numProcs()
{
   return (unsigned) procs.size();
}
