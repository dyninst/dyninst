#include "BPatch.h"
#include "BPatch_basicBlock.h"
#include "BPatch_basicBlockLoop.h"
#include "BPatch_binaryEdit.h"
#include "BPatch_edge.h"
#include "BPatch_flowGraph.h"
#include "BPatch_function.h"
#include "BPatch_image.h"
#include "BPatch_point.h"

#include <cstdlib>
#include <initializer_list>
#include <iostream>
#include <set>
#include <vector>

// The input, dyninstAPI/parsing/splitLoopBlock in test_binaries, has an x86_64
// function loop_func whose loop block is split by a jump from a function that
// Dyninst finds later by gap parsing. loop_func keeps only the first half of
// the block, so the loop's back edge starts in a block outside loop_func and
// can get no instrumentation point there. Each loop of loop_func must consist
// of blocks of its flow graph, every back edge must get a point, and
// findLoopInstPoints must return only points, for every loop location.

namespace {

  bool check_loop(BPatch_flowGraph *cfg, std::set<BPatch_basicBlock *> const &blocks,
                  BPatch_basicBlockLoop *loop) {
    bool ok = true;

    std::vector<BPatch_basicBlock *> loop_blocks;
    loop->getLoopBasicBlocks(loop_blocks);
    for (auto *block : loop_blocks) {
      if (!blocks.count(block)) {
        std::cerr << "Loop block 0x" << std::hex << block->getStartAddress() << std::dec
                  << " is not in the flow graph\n";
        ok = false;
      }
    }

    std::vector<BPatch_edge *> back_edges;
    loop->getBackEdges(back_edges);
    for (auto *edge : back_edges) {
      if (!blocks.count(edge->getSource()) || !blocks.count(edge->getTarget()) ||
          !edge->getPoint()) {
        std::cerr << "Back edge 0x" << std::hex << edge->getSource()->getLastInsnAddress()
                  << " -> 0x" << edge->getTarget()->getStartAddress() << std::dec
                  << " has no point\n";
        ok = false;
      }
    }

    for (auto loc : {BPatch_locLoopEntry, BPatch_locLoopExit, BPatch_locLoopEndIter}) {
      auto *points = cfg->findLoopInstPoints(loc, loop);
      if (!points) {
        continue;
      }
      for (auto *point : *points) {
        if (!point) {
          std::cerr << "findLoopInstPoints(" << loc << ") returned a NULL point\n";
          ok = false;
        }
      }
    }
    return ok;
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

  // Parse the whole binary first, as a tool that instruments every function
  // does; this is when gap parsing finds the function that splits the loop.
  std::vector<BPatch_function *> all_funcs;
  binary->getImage()->getProcedures(all_funcs, true);

  std::vector<BPatch_function *> funcs;
  binary->getImage()->findFunction("loop_func", funcs);
  if (funcs.size() != 1 || !funcs[0]->isInstrumentable()) {
    std::cerr << "Expected one instrumentable function 'loop_func', found " << funcs.size()
              << '\n';
    return EXIT_FAILURE;
  }

  BPatch_flowGraph *cfg = funcs[0]->getCFG();
  std::set<BPatch_basicBlock *> blocks;
  cfg->getAllBasicBlocks(blocks);

  std::vector<BPatch_basicBlockLoop *> loops;
  cfg->getLoops(loops);
  if (loops.empty()) {
    std::cerr << "Expected a loop in 'loop_func'\n";
    return EXIT_FAILURE;
  }

  bool ok = true;
  for (auto *loop : loops) {
    ok &= check_loop(cfg, blocks, loop);
  }
  return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
