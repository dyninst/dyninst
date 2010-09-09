#include "proccontrol_comp.h"
#include "communication.h"

class pc_detachMutator : public ProcControlMutator {
public:
  virtual test_results_t executeTest();
};

extern "C" DLLEXPORT TestMutator* pc_detach_factory()
{
  return new pc_detachMutator();
}

static bool signal_error = false;
Process::cb_ret_t on_signal(Event::const_ptr ev)
{
   //We should not see any of the signals if the detach worked
   logerror("Error. Recieved signal\n");
   signal_error = true;
   return Process::cbThreadContinue;
}

test_results_t pc_detachMutator::executeTest()
{
   std::vector<Process::ptr>::iterator i;
   bool error = false;
   comp->curgroup_self_cleaning = true;
   
   Process::registerEventCallback(EventType::Signal, on_signal);

   for (i = comp->procs.begin(); i != comp->procs.end(); i++) {
      Process::ptr proc = *i;
      bool result = proc->continueProc();
      if (!result) {
         logerror("Failed to continue process\n");
         error = true;
      }
   }

   for (i = comp->procs.begin(); i != comp->procs.end(); i++) {
      Process::ptr proc = *i;
      bool result = proc->detach();
      if (!result) {
         logerror("Failed to detach from processes\n");
         error = true;
      }
   }

   syncloc sync_point;
   sync_point.code = SYNCLOC_CODE;
   bool result = comp->send_broadcast((unsigned char *) &sync_point, sizeof(syncloc));
   if (!result) {
      logerror("Failed to send sync broadcast\n");
      return FAILED;
   }

   syncloc sync_points[NUM_PARALLEL_PROCS];
   result = comp->recv_broadcast((unsigned char *) sync_points, sizeof(syncloc));
   if (!result) {
      logerror("Failed to recieve sync broadcast\n");
      return FAILED;
   }

   unsigned j = 0;
   for (i = comp->procs.begin(); i != comp->procs.end(); i++, j++) {
      if (sync_points[j].code != SYNCLOC_CODE) {
         logerror("Recieved unexpected sync message\n");
         return FAILED;
      }
   }

   if (signal_error)
      error = true;
   Process::removeEventCallback(EventType::Signal);

   return error ? FAILED : PASSED;
}


