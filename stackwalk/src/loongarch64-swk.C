/* See the dyninst/COPYRIGHT file for copyright information. */

#include "loongarch64-swk.h"
#include "stackwalk/h/swk_errors.h"
#include "stackwalk/h/walker.h"
#include "stackwalk/h/basetypes.h"
#include "stackwalk/h/procstate.h"
#include "stackwalk/h/framestepper.h"
#include "common/src/arch-loongarch64.h"

using namespace Dyninst;
using namespace Dyninst::Stackwalker;
using namespace NS_loongarch64;

std::map<Dyninst::PID, loongarch64_LookupFuncStart*> loongarch64_LookupFuncStart::all_func_starts;

loongarch64_LookupFuncStart::loongarch64_LookupFuncStart(ProcessState *p) :
   FrameFuncHelper(p),
   ref_count(1) {
}

loongarch64_LookupFuncStart::~loongarch64_LookupFuncStart() {
}

loongarch64_LookupFuncStart *loongarch64_LookupFuncStart::getLookupFuncStart(ProcessState *p) {
   Dyninst::PID pid = p->getProcessId();
   auto i = all_func_starts.find(pid);
   if (i != all_func_starts.end()) {
      loongarch64_LookupFuncStart *result = i->second;
      result->ref_count++;
      return result;
   }
   loongarch64_LookupFuncStart *result = new loongarch64_LookupFuncStart(p);
   all_func_starts[pid] = result;
   return result;
}

void loongarch64_LookupFuncStart::releaseMe() {
   ref_count--;
   if (ref_count <= 0) {
      all_func_starts.erase(proc->getProcessId());
      delete this;
   }
}

void loongarch64_LookupFuncStart::updateCache(Address, FrameFuncHelper::alloc_frame_t) {
}

bool loongarch64_LookupFuncStart::checkCache(Address, FrameFuncHelper::alloc_frame_t &) {
   return false;
}

FrameFuncHelper::alloc_frame_t loongarch64_LookupFuncStart::allocatesFrame(Address) {
   alloc_frame_t result;
   result.first = standard_frame;
   result.second = set_frame;
   return result;
}

void loongarch64_LookupFuncStart::clear_func_mapping(Dyninst::PID) {
}