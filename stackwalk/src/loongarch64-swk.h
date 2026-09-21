#ifndef LOONGARCH64_SWK_H
#define LOONGARCH64_SWK_H

#include "stackwalk/h/steppergroup.h"
#include "stackwalk/h/framestepper.h"

#include "common/h/dyntypes.h"

namespace Dyninst {
namespace Stackwalker {

class loongarch64_LookupFuncStart : public FrameFuncHelper
{
private:
   static std::map<Dyninst::PID, loongarch64_LookupFuncStart*> all_func_starts;
   loongarch64_LookupFuncStart(ProcessState *proc_);
   int ref_count;

   void updateCache(Address addr, FrameFuncHelper::alloc_frame_t result);
   bool checkCache(Address addr, FrameFuncHelper::alloc_frame_t &result);
   static const unsigned int cache_size = 64;
public:
   static loongarch64_LookupFuncStart *getLookupFuncStart(ProcessState *p);
   void releaseMe();
   virtual FrameFuncHelper::alloc_frame_t allocatesFrame(Address addr);
   ~loongarch64_LookupFuncStart();
   static void clear_func_mapping(Dyninst::PID);
};

}
}

#endif // LOONGARCH64_SWK_H