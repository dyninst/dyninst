#include "proccontrol_comp.h"
#include "communication.h"

#include <cstdio>

using namespace std;

class pc_forkMutator : public ProcControlMutator {
public:
  virtual test_results_t executeTest();
};

extern "C" DLLEXPORT TestMutator* pc_fork_factory()
{
  return new pc_forkMutator();
}

struct proc_info {
   bool got_breakpoint;
   bool is_threaded;
   bool is_exited;
   Process::const_ptr parent;
   Process::const_ptr child;

   proc_info() :
      got_breakpoint(false),
      is_threaded(false),
      is_exited(false),
      parent(Process::const_ptr()),
      child(Process::const_ptr())
   {
   }
};

static std::map<Dyninst::PID, proc_info> pinfo;
static bool myerror;
static Breakpoint::ptr bp;
#define EXIT_CODE 4

Process::cb_ret_t on_breakpoint(Event::const_ptr ev)
{
   EventBreakpoint::const_ptr ebp = ev->getEventBreakpoint();
   std::vector<Breakpoint::const_ptr> bps;
   ebp->getBreakpoints(bps);
   if (bps.size() != 1 && bps[0] != bp) {
      logerror("Got unexpected breakpoint\n");
      myerror = true;
   }
   proc_info &pi = pinfo[ev->getProcess()->getPid()];
   if (pi.got_breakpoint) {
      logerror("Breakpoint hit twice\n");
      myerror = true;
   }
   pi.got_breakpoint = true;
   return Process::cbProcContinue;
}

Process::cb_ret_t on_fork(Event::const_ptr ev)
{
   EventFork::const_ptr efork = ev->getEventFork();
   Process::const_ptr child_proc = efork->getChildProcess();
   Process::const_ptr parent_proc = ev->getProcess();
   if (child_proc == parent_proc) {
      logerror("Got child proc equal to parent\n");
      myerror = true;
      return Process::cbDefault;
   }

   if (pinfo.find(child_proc->getPid()) != pinfo.end()) {
      logerror("Got a child proc twice\n");
      myerror = true;
      return Process::cbDefault;
   }

   proc_info &pi = pinfo[child_proc->getPid()];
   pi.is_threaded = child_proc->threads().size() > 1;
   pi.parent = parent_proc;
   pi.child = child_proc;
   if (child_proc->libraries().size() != parent_proc->libraries().size())
   {
      logerror("Parent and child procs do not have same libraries\n");
      myerror = true;
   }

   return Process::cb_ret_t(Process::cbDefault, Process::cbProcContinue);
}

Process::cb_ret_t on_exit(Event::const_ptr ev)
{
   EventExit::const_ptr ee = ev->getEventExit();
   if (!ev->getProcess()->isExited()) {
      logerror("Exit event on not-exited process\n");
      myerror = true;
   }
   proc_info &pi = pinfo[ev->getProcess()->getPid()];
   pi.is_exited = true;

   return Process::cbDefault;
}

test_results_t pc_forkMutator::executeTest()
{
   myerror = false;
   pinfo.clear();
   bp = Breakpoint::newBreakpoint();

   Process::registerEventCallback(EventType::Breakpoint, on_breakpoint);
   Process::registerEventCallback(EventType::Fork, on_fork);
   Process::registerEventCallback(EventType(EventType::Post, EventType::Exit),on_exit);

   for (std::vector<Process::ptr>::iterator i = comp->procs.begin(); 
        i != comp->procs.end(); i++) {
      Process::ptr proc = *i;
      bool result = proc->continueProc();
      if (!result) {
         logerror("Failed to continue process\n");
         myerror = true;
      }

      send_addr addr;
      result = comp->recv_message((unsigned char *) &addr, sizeof(send_addr), 
                                       proc);
      if (!result) {
         logerror("Failed to recieve addr message\n");
         myerror = true;
      }
      if (addr.code != SENDADDR_CODE) {
         logerror("Unexpected addr code\n");
         myerror = true;
      }
      Address bp_addr = addr.addr;
      
      result = proc->stopProc();
      if (!result) {
         logerror("Failed to stop process\n");
         myerror = true;
      }

      result = proc->addBreakpoint(bp_addr, bp);
      if (!result) {
         logerror("Failed to insert breakpoint\n");
         myerror = true;
      }
      
      syncloc sync_msg;
      sync_msg.code = SYNCLOC_CODE;
      result = comp->send_message((unsigned char *) &sync_msg, sizeof(sync_msg), 
                                       proc);
      if (!result) {
         logerror("Failed to send sync message to process\n");
         myerror = true;
      }
   }

   for (std::vector<Process::ptr>::iterator i = comp->procs.begin(); 
        i!= comp->procs.end(); i++) {
      Process::ptr proc = *i;
      bool result = proc->continueProc();
      if (!result) {
         logerror("Failed to continue process\n");
         myerror = true;
      }
   }

   for (std::vector<Process::ptr>::iterator i = comp->procs.begin(); 
        i!= comp->procs.end(); i++) 
   {
      Process::ptr proc = *i;
      bool done = false;
      while (!done) {
         forkinfo fork_data;
         bool result = comp->recv_message((unsigned char *) &fork_data, 
                                          sizeof(send_addr),
                                          proc);
         if (!result) {
            logerror("Failed to recieve fork message\n");
            myerror = true;
            break;
         }
         if (fork_data.code != FORKINFO_CODE) {
            logerror("Unexpected fork code\n");
            myerror = true;
            break;
         }
         done = (fork_data.is_done != 0);
         proc_info &pi = pinfo[fork_data.pid];
         if (pi.parent != proc) {
            fprintf(stderr, "pi.parent = %p\n", pi.parent.get());
            fprintf(stderr, "proc = %p\n", proc.get());
            fprintf(stderr, "pi.child = %p\n", pi.child.get());
            fprintf(stderr, "pi.parent = %d\n", pi.parent->getPid());
            fprintf(stderr, "proc = %d\n", proc->getPid());
            fprintf(stderr, "pi.child = %d\n", pi.child->getPid());
            logerror("Unexpected parent thread\n");
            myerror = true;
            continue;
         }
         if (pi.child->getPid() != fork_data.pid) {
            logerror("Unexpected pid\n");
            myerror = true;
            continue;
         }
         if (!pi.got_breakpoint) {
            logerror("Child did not execute breakpoint\n");
            myerror = true;
            continue;
         }
         if (!pi.is_exited) {
            logerror("Child did not exit\n");
            myerror = true;
            continue;
         }
         if (!pi.child->isExited()) {
            logerror("Process was not marked as exited\n");
            myerror = true;
            continue;
         }
         if (pi.child->getExitCode() != EXIT_CODE) {
            logerror("Invalid exit code for process\n");
            myerror = true;
            continue;
         }
         if (pi.is_threaded != (fork_data.is_threaded != 0)) {
            logerror("Mutator and mutatee do not agree on threading\n");
            myerror = true;
            continue;
         }
      }
   }

   Process::removeEventCallback(on_fork);
   Process::removeEventCallback(on_breakpoint);
   Process::removeEventCallback(on_exit);

   return myerror ? FAILED : PASSED;
}
