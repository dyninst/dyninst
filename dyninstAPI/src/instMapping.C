#include "addressSpace.h"
#include "instMapping.h"
#include "instPoint.h"

#include <cassert>

namespace patch = Dyninst::PatchAPI;

namespace Dyninst { namespace DyninstAPI {

  instMapping::instMapping(const instMapping *parIM, AddressSpace *child)
      : func(parIM->func), inst(parIM->inst), where(parIM->where), order(parIM->order),
        useTrampGuard(parIM->useTrampGuard), allow_trap(parIM->allow_trap) {
    for(auto ast : parIM->args) {
      args.push_back(ast);
    }
    for(auto instance : parIM->instances) {
      patch::InstancePtr cMT = getChildInstance(instance, child);
      assert(cMT);
      instances.push_back(cMT);
    }
  }

}}
