#include "Location.h"
#include "liveness.h"
#include "bitArray.h"
#include "registers/x86_64_regs.h"
using namespace std;
using namespace Dyninst;
using namespace Dyninst::ParseAPI;

void LivenessAnalysis(Function* f, Block* b)
{
  // Construct a liveness analyzer based on the address width of the mutatee.
  // 32-bit code and 64-bit code have different ABI.
  LivenessAnalyzer la(f->obj()->cs()->getAddressWidth());

  // Construct a liveness query location
  Location loc(f, b);

  // Query live registers at the block entry
  bitArray liveEntry;
  if (!la.query(loc, LivenessAnalyzer::Before, liveEntry))  {
    printf("Cannot look up live registers at block entry\n");
  }

  printf("There are %lu registers live at the block entry\n", liveEntry.count());

  // Query live register at the block exit
  bitArray liveExit;
  if (!la.query(loc, LivenessAnalyzer::After, liveExit))  {
    printf("Cannot look up live registers at block exit\n");
  }

  printf("rbx is live or not at the block exit: %d\n",
         liveExit.test(la.getIndex(x86_64::rbx)));
}
