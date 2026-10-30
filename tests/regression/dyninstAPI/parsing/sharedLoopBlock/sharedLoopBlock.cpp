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
#include <initializer_list>
#include <iostream>
#include <set>
#include <string>
#include <vector>

// The input, dyninstAPI/parsing/sharedLoopBlock in test_binaries, has two
// x86_64 functions sharing a loop head: loop_entrant jumps into the loop of
// loop_owner, so the head is in both functions and has an incoming edge from a
// block that only loop_entrant has. That edge belongs to loop_entrant's flow
// graph, not loop_owner's. In each function's flow graph, every edge must have
// both ends in the function and an instrumentation point, and the loop must
// have exactly one entry point.

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
    for (auto *edge : edges) {
      Dyninst::ParseAPI::Edge *pedge = Dyninst::ParseAPI::convert(edge);
      bool const in_func = blocks.count(edge->getSource()) && blocks.count(edge->getTarget());
      if (!in_func || !edge->getPoint()) {
        std::cerr << func->getName() << ": edge 0x" << std::hex << pedge->src()->last()
                  << " -> 0x" << pedge->trg()->start() << std::dec
                  << (in_func ? " has no point" : " leaves the function") << '\n';
        ok = false;
      }
    }
    return ok;
  }

  bool check_loop_entry_points(BPatch_function *func) {
    std::vector<BPatch_basicBlockLoop *> loops;
    func->getCFG()->getOuterLoops(loops);
    if (loops.size() != 1) {
      std::cerr << func->getName() << ": expected one loop, found " << loops.size() << '\n';
      return false;
    }
    auto *points = func->getCFG()->findLoopInstPoints(BPatch_locLoopEntry, loops[0]);
    if (!points || points->size() != 1 || !(*points)[0]) {
      std::cerr << func->getName() << ": expected one loop entry point, found "
                << (points ? points->size() : 0) << '\n';
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

  bool ok = true;
  for (char const *name : {"loop_owner", "loop_entrant"}) {
    BPatch_function *func = find_function(image, name);
    if (!func) {
      return EXIT_FAILURE;
    }
    ok &= check_edge_points(func);
    ok &= check_loop_entry_points(func);
  }
  return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
