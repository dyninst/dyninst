using namespace Dyninst;
using namespace DepGraphAPI;

// Assume this represents a function of interest
BPatch_function *func;
// And an address of an instruction of interest
Address insnAddr;
// And a register defined by the previous instruction
InstructionAPI::RegisterAST::Ptr reg;

// Access the PDG for this function
PDG::Ptr pdg = PDG::analyze(func);

// Find the node of interest
NodeIterator nodeBegin, nodeEnd;
pdg->find(insnAddr, reg, nodeBegin, nodeEnd);

// Make sure we found a node...
if (nodeBegin == nodeEnd) {
    // Complain
}

// Create the forward slice from the node of interest
NodeIterator sliceBegin, sliceEnd;
pdg->forwardClosure(*nodeBegin, sliceBegin, sliceEnd);

// Iterate over each node in the closure and do something
for (; sliceBegin != sliceEnd; sliceBegin++) {
    // ...
}
