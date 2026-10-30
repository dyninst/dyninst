#include "BPatch.h"
#include "BPatch_basicBlock.h"
#include "BPatch_basicBlockLoop.h"
#include "BPatch_binaryEdit.h"
#include "BPatch_edge.h"
#include "BPatch_flowGraph.h"
#include "BPatch_function.h"
#include "BPatch_image.h"
#include "BPatch_point.h"
#include "CFG.h"

#include <cstdlib>
#include <iostream>
#include <set>
#include <string>
#include <vector>

// The input, dyninstAPI/parsing/sharedLoopBlock in test_binaries, has two
// x86_64 functions sharing a loop head: loop_entrant jumps into the loop of
// loop_owner, so the head has an incoming edge from a block outside
// loop_owner. BPatch_edge::getPoint must return no point for that edge instead
// of crashing, and points for loop_owner's other edges. Loop entry points,
// which come from the loop head's incoming edges, must then cover only the
// edge from inside loop_owner.

namespace {

  BPatch_function *find_function(BPatch_image *image, char const *name) {
    std::vector<BPatch_function *> funcs;
    image->findFunction(name, funcs);
    if (funcs.size() != 1) {
      std::cerr << "Expected one function '" << name << "', found " << funcs.size() << '\n';
      return nullptr;
    }
    if (!funcs[0]->isInstrumentable()) {
      std::cerr << "'" << name << "' is not instrumentable\n";
      return nullptr;
    }
    return funcs[0];
  }

  bool check_edge_points(BPatch_function *func) {
    Dyninst::ParseAPI::Function *pfunc = Dyninst::ParseAPI::convert(func);
    std::set<BPatch_basicBlock *> blocks;
    func->getCFG()->getAllBasicBlocks(blocks);

    std::set<BPatch_edge *> edges;
    for (auto *block : blocks) {
      std::vector<BPatch_edge *> block_edges;
      block->getIncomingEdges(block_edges);
      block->getOutgoingEdges(block_edges);
      edges.insert(block_edges.begin(), block_edges.end());
    }

    bool ok = true;
    int foreign = 0;
    for (auto *edge : edges) {
      Dyninst::ParseAPI::Edge *pedge = Dyninst::ParseAPI::convert(edge);
      bool const in_func = pfunc->contains(pedge->src()) && pfunc->contains(pedge->trg());
      foreign += in_func ? 0 : 1;
      BPatch_point *point = edge->getPoint();
      if (in_func != (point != nullptr)) {
        std::cerr << "Edge 0x" << std::hex << pedge->src()->last() << " -> 0x"
                  << pedge->trg()->start() << std::dec << ": expected "
                  << (in_func ? "a point" : "no point") << '\n';
        ok = false;
      }
    }
    if (foreign != 1) {
      std::cerr << "Expected one edge from outside the function, found " << foreign << '\n';
      ok = false;
    }
    return ok;
  }

  bool check_loop_entry_points(BPatch_function *func) {
    std::vector<BPatch_basicBlockLoop *> loops;
    func->getCFG()->getOuterLoops(loops);
    if (loops.size() != 1) {
      std::cerr << "Expected one loop, found " << loops.size() << '\n';
      return false;
    }
    auto *points = func->getCFG()->findLoopInstPoints(BPatch_locLoopEntry, loops[0]);
    if (!points || points->size() != 1 || !(*points)[0]) {
      std::cerr << "Expected one loop entry point, found " << (points ? points->size() : 0)
                << '\n';
      return false;
    }
    return true;
  }

}

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <input binary>\n";
    return EXIT_FAILURE;
  }

  BPatch bpatch;
  BPatch_binaryEdit *binary = bpatch.openBinary(argv[1], false);
  if (!binary) {
    std::cerr << "Unable to open '" << argv[1] << "'\n";
    return EXIT_FAILURE;
  }
  BPatch_image *image = binary->getImage();

  BPatch_function *owner = find_function(image, "loop_owner");
  if (!owner || !find_function(image, "loop_entrant")) {
    return EXIT_FAILURE;
  }

  bool ok = check_edge_points(owner);
  ok &= check_loop_entry_points(owner);
  return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
