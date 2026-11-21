#include <cstdlib>
#include <iostream>
#include <unordered_set>
#include <vector>

#include "CFG.h"
#include "CodeObject.h"

namespace pa = Dyninst::ParseAPI;

/*
 * jt_sink.so (test_binaries/parseAPI/SinkEdgeCleanup): an over-read jump-table target whose block
 * ends with a sink edge is deleted by finalize_jump_tables.  Every edge in the sink block's sources
 * must still be in its source block's targets.
 */
int main(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "Usage " << argv[0] << ": file\n";
    return EXIT_FAILURE;
  }
  pa::SymtabCodeSource scs{argv[1]};
  pa::CodeObject co{&scs};
  co.parse();

  pa::Block *sink{nullptr};
  for (pa::Function *f : co.funcs()) {
    for (pa::Block *b : f->blocks()) {
      for (pa::Edge *e : b->targets()) {
        if (e->sinkEdge()) {
          sink = e->trg();
          break;
        }
      }
      if (sink != nullptr)
        break;
    }
    if (sink != nullptr)
      break;
  }
  if (sink == nullptr) {
    std::cerr << "no sink edge found\n";
    return EXIT_FAILURE;
  }

  int bad{0};
  for (pa::Edge *e : sink->sources()) {
    const pa::Block::edgelist &targets{e->src()->targets()};
    if (targets.find(e) == targets.end()) {
      std::cerr << std::hex << "sink edge from block 0x" << e->src()->start()
                << " is in the sink's sources but not in its source's targets\n";
      ++bad;
    }
  }
  return bad == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
