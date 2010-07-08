#include "proccontrol_comp.h"

class pc_launchMutator : public ProcControlMutator {
public:
  virtual test_results_t executeTest();
};

extern "C" DLLEXPORT TestMutator* pc_launch_factory()
{
  return new pc_launchMutator();
}

test_results_t pc_launchMutator::executeTest()
{
   std::vector<Process::ptr>::iterator i;
   bool error = false;

   for (i = comp->procs.begin(); i != comp->procs.end(); i++) {
      Process::ptr proc = *i;
      bool result = proc->continueProc();
      if (!result) {
         logerror("Failed to continue process\n");
         error = true;
      }
   }

   return error ? FAILED : PASSED;
}


