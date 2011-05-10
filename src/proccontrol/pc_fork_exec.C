#include "proccontrol_comp.h"
#include "communication.h"

using namespace std;

class pc_fork_execMutator : public ProcControlMutator {
public:
  virtual test_results_t executeTest();
};

extern "C" DLLEXPORT TestMutator* pc_fork_exec_factory()
{
  return new pc_fork_execMutator();
}

struct proc_info_forkexec {
   bool is_exited;
   std::string executable;

   proc_info_forkexec() :
      is_exited(false)
   {
   }
};

static std::map<Process::const_ptr, proc_info_forkexec> pinfo;
static bool myerror;
#define EXIT_CODE 4

char *exec_name = "pc_exec_targ";
char *libtestA = "libtestA";

static bool hasLibrary(std::string lib, Process::const_ptr proc)
{
   LibraryPool::const_iterator i = proc->libraries().begin();
   for (; i != proc->libraries().end(); i++) {
      Library::const_ptr lib = *i;
      if ((lib->getName().find(libtestA)) != std::string::npos) {
         return true;
      }
   }
   return false;
}

Process::cb_ret_t on_exec(Event::const_ptr ev)
{
   EventExec::const_ptr eexec = ev->getEventExec();
   Process::const_ptr proc = ev->getProcess();
   proc_info_forkexec &pi = pinfo[proc];

   pi.executable = eexec->getExecPath();

   if (hasLibrary(libtestA, proc)) {
      logerror("libtestA was in the exec'd process");
      myerror = true;
   }
   return Process::cbDefault;                         
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

   if (pinfo.find(child_proc) != pinfo.end()) {
      logerror("Got a child proc twice\n");
      myerror = true;
      return Process::cbDefault;
   }

   proc_info_forkexec &pi = pinfo[child_proc];

   if (child_proc->libraries().size() != parent_proc->libraries().size())
   {
      logerror("Parent and child procs do not have same libraries\n");
      myerror = true;
   }
   if (!hasLibrary(libtestA, child_proc)) {
      logerror("libtestA wasn't in the child process");
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
   
   map<Process::const_ptr, proc_info_forkexec>::iterator i = pinfo.find(ev->getProcess());
   if (i == pinfo.end()) {
      return Process::cbDefault;
   }

   proc_info_forkexec &pi = i->second;
   pi.is_exited = true;
   if (ee->getExitCode() != EXIT_CODE) {
      logerror("Process exited with unexpected code\n");
      myerror = true;
   }

   return Process::cbDefault;
}

test_results_t pc_fork_execMutator::executeTest()
{
   myerror = false;
   pinfo.clear();

   Process::registerEventCallback(EventType::Exec, on_exec);
   Process::registerEventCallback(EventType::Fork, on_fork);
   Process::registerEventCallback(EventType(EventType::Post, EventType::Exit), on_exit);

   for (std::vector<Process::ptr>::iterator i = comp->procs.begin(); 
        i!= comp->procs.end(); i++) {
      Process::ptr proc = *i;
      bool result = proc->continueProc();
      if (!result) {
         logerror("Failed to continue process\n");
         myerror = true;
      }
   }

   syncloc loc[NUM_PARALLEL_PROCS];
   bool result = comp->recv_broadcast((unsigned char *) loc, sizeof(syncloc));
   if (!result) {
      logerror("Failed to recieve sync broadcast\n");
      myerror = true;
   }
   for (unsigned j=0; j<comp->procs.size(); j++) {
      if (loc[j].code != SYNCLOC_CODE) {
         logerror("Recieved unexpected message code\n");
         myerror = true;
      }
   }

   if (pinfo.size() != comp->num_processes * (comp->num_threads+1)) {
      logerror("Did not recieve expected number of callbacks\n");
      myerror = true;
   }

   map<Process::const_ptr, proc_info_forkexec>::iterator i = pinfo.begin();
   for (; i != pinfo.end(); i++) {
      Process::const_ptr proc = i->first;
      proc_info_forkexec &pi = i->second;

      if (!pi.is_exited) {
         logerror("Process did not deliver exit callback\n");
         myerror = true;
      }
      if (pi.executable.find(exec_name) == std::string::npos) {
         logerror("Process had invalid exec name\n");
         myerror = true;
      }
   }

   Process::removeEventCallback(on_fork);
   Process::removeEventCallback(on_exec);
   Process::removeEventCallback(on_exit);

   return myerror ? FAILED : PASSED;
}
