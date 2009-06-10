using namespace Dyninst;
using namespace DepGraphAPI;

// Assume these represent a function and block of interest
BPatch_function *func;
BPatch_basicBlock *block;

// Access the DDG
DDG::Ptr ddg = DDG::analyze(func);

// Get the list of instructions (and their addresses) from the block

typedef std::pair<InstructionAPI::Instruction, Address> InsnInstance;
std::vector<InsnInstance> insnInstances;
block->getInstructions(insnInstances);

// For each instruction, look up the DDG node and see if it has itself as a target
for (std::vector<InsnInstance>::iterator iter = insnInstances.begin();
     iter != insnInstances.end(); iter++) {
  Address addr = iter->second;
  
	  NodeIterator nodeBegin, nodeEnd;
  ddg->find(addr, nodeBegin, nodeEnd);
  for (; nodeBegin != nodeEnd; nodeBegin++) {
    NodeIterator targetBegin, targetEnd;
    (*nodeBegin)->getTargets(targetBegin, targetEnd);
    for (; targetBegin != targetEnd; targetBegin++) {
      if (*targetBegin == *nodeBegin) {
        // Found a node that has itself as a target
        actOnSelfDefiningNode(*nodeBegin);
      }
    }
  }
}
