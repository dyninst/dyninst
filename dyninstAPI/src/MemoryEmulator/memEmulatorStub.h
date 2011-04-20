#if !defined(MEM_EMULATOR_STUB)
#define MEM_EMULATOR_STUB

#if defined(cap_mem_emulation)
#error
#endif

class mapped_object;

namespace Dyninst {
class MemoryEmulator {
 public:
  MemoryEmulator(AddressSpace *) {};
  void addSpringboard(SymtabAPI::Region *, Address, int) {};
  void addAllocatedRegion(Address, unsigned) {};
  void addRegion(mapped_object *) {};
  void removeRegion(Address, unsigned) {};
  void removeRegion(mapped_object *) {};
  void update() {};
  void synchShadowOrig(bool) {};
  std::pair<bool, Address> translate(Address) { return std::make_pair(false, 0);  }

};
};

#endif
