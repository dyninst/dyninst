#include "proccontrol_comp.h"
#include "communication.h"
#include "Process.h"
#include "Event.h"


#include <set>

using namespace std;

class pc_thread_contMutator : public ProcControlMutator {
public:
  virtual test_results_t executeTest();
};

extern "C" DLLEXPORT TestMutator* pc_thread_cont_factory()
{
  return new pc_thread_contMutator();
}

static std::set<Thread::const_ptr> expected_exits;
static std::set<Thread::const_ptr> expected_pre_exits;

static bool cb_error = false;

Process::cb_ret_t on_thread_exit(Event::const_ptr ev)
{
   if (ev->getEventType().code() != EventType::ThreadDestroy) {
      logerror("Got unexpected event type\n");
      cb_error = true;
      return Process::cbDefault;
   }

   if (ev->getEventType().time() == EventType::Pre) {
      set<Thread::const_ptr>::iterator i = expected_pre_exits.find(ev->getThread());
      if (i == expected_pre_exits.end()) {
         logerror("Got pre-exit for unknown thread\n");
         cb_error = true;
         return Process::cbDefault;         
      }
      expected_pre_exits.erase(i);
      return Process::cbDefault;
   }
   else if (ev->getEventType().time() == EventType::Post) {
      set<Thread::const_ptr>::iterator i = expected_pre_exits.find(ev->getThread());
      if (i != expected_pre_exits.end()) {
         logerror("Got exit without pre-exit\n");
         cb_error = true;
      }
      set<Thread::const_ptr>::iterator j = expected_exits.find(ev->getThread());
      if (j == expected_exits.end()) {
         logerror("Got exit for unknown thread\n");
         cb_error = true;
         return Process::cbDefault;
      }
      expected_exits.erase(j);
      return Process::cbDefault;
   }
   else {
      logerror("Got unexpected event time\n");
      cb_error = true;
      return Process::cbDefault;
   }
}

/**
 * After stopping all threads, continue each one one at a time until
 * exit.
 **/
test_results_t pc_thread_contMutator::executeTest()
{
   std::vector<Process::ptr>::iterator i;
   bool error = false;
   cb_error = false;

   int num_threads = 0;
   for (i = comp->procs.begin(); i != comp->procs.end(); i++) {
      Thread::ptr thr = (*i)->threads().getInitialThread();
      bool result = thr->continueThread();
      if (!result) {
         logerror("Error continuing initial thread\n");
         error = true;
      }
      if (!num_threads) {
         num_threads = (*i)->threads().size();
      }
   }

   EventType et(EventType::ThreadDestroy);
   Process::registerEventCallback(et, on_thread_exit);
      
   for (unsigned j=0; j<num_threads-1; j++) {
      expected_exits.clear();

      for (i = comp->procs.begin(); i != comp->procs.end(); i++) {
         Process::ptr proc = *i;
         ThreadPool::iterator k = proc->threads().begin();
         if (k == proc->threads().end()) {
            logerror("No threads in process list\n");
            error = true;
            continue;
         }
         if (!(*k)->isInitialThread()) {
            logerror("Initial thread wasn't first in iterator\n");
            error = true;
         }
         k++;
         if (k == proc->threads().end()) {
            logerror("Not enough threads left in list\n");
            error = true;
            continue;
         }
         Thread::ptr thrd = *k;
         if (thrd->isInitialThread()) {
            logerror("Multiple initial threads or broken iterator\n");
            error = true;
         }
         
         expected_exits.insert(thrd);
         expected_pre_exits.insert(thrd);
         bool result = thrd->continueThread();
         if (!result) {
            logerror("Failed to continue thread\n");
            error = true;
         }
      }
      
      while (!expected_exits.empty())
         comp->block_for_events();
   }

   allow_exit ae;
   ae.code = ALLOWEXIT_CODE;
   bool result = comp->send_broadcast((unsigned char *) &ae, sizeof(allow_exit));
   if (!result) {
      logerror("Could send exit broadcast to mutatee\n");
      error = true;
   }

   return (error || cb_error) ? FAILED : PASSED;
}


