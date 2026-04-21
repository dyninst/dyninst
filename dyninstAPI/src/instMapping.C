#include "addressSpace.h"
#include "instMapping.h"
#include "instPoint.h"

#include <cassert>

namespace patch = Dyninst::PatchAPI;

namespace Dyninst { namespace DyninstAPI {

  instMapping::instMapping(const instMapping *parIM, AddressSpace *child)
      : func(parIM->func), inst(parIM->inst), where(parIM->where), when(parIM->when),
        order(parIM->order), useTrampGuard(parIM->useTrampGuard), mt_only(parIM->mt_only),
        allow_trap(parIM->allow_trap) {
    for(unsigned i = 0; i < parIM->args.size(); i++) {
      args.push_back(parIM->args[i]);
    }
    for(unsigned j = 0; j < parIM->instances.size(); j++) {
      patch::InstancePtr cMT = getChildInstance(parIM->instances[j], child);
      assert(cMT);
      instances.push_back(cMT);
    }
  }

}}
