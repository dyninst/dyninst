#if !defined(PROCPOOL_H_)
#define PROCPOOL_H_

#include <map>
#include "dyntypes.h"
#include "common/h/dthread.h"

using namespace Dyninst;

class int_process;
class int_thread;

class ProcessPool
{
   friend ProcessPool *ProcPool();
 protected:
   std::map<Dyninst::PID, int_process *> procs;
   std::map<Dyninst::LWP, int_thread *> lwps;
   ProcessPool();
   CondVar var;
 public:
   ~ProcessPool();
   typedef bool(*ifunc)(int_process *, void *data);

   int_process *findProcByPid(Dyninst::PID pid);
   void addProcess(int_process *proc);
   void addThread(int_process *proc, int_thread *thr);
   void rmProcess(int_process *proc);
   void rmThread(int_thread *thr);
   int_thread *findThread(Dyninst::LWP lwp);
   unsigned numProcs();
   bool LWPIDsAreUnique();
   bool for_each(ifunc f, void *data = NULL);
   CondVar *condvar();
};

ProcessPool *ProcPool();

#endif
