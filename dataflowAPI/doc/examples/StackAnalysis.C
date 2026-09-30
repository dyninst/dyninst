#include "CFG.h"
#include "Absloc.h"
#include "stackanalysis.h"
using namespace Dyninst;
using namespace ParseAPI;

void StackHeight(Function* func, Block* block)
{
  // Get the address of the first instruction of the block
  Address addr = block->start();

  // Get the stack heights at that address
  StackAnalysis sa(func);
  std::vector<std::pair<Absloc, StackAnalysis::Height>> heights;
  sa.findDefinedHeights(block, addr, heights);

  // Print out the stack heights
  for (auto iter = heights.begin(); iter != heights.end(); iter++)  {
    const Absloc& loc = iter->first;
    const StackAnalysis::Height& height = iter->second;
    printf("%s := %s\n", loc.format().c_str(), height.format().c_str());
  }
}
