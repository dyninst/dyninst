#include <cstdlib>
#include <iostream>

#include "CFG.h"
#include "CodeObject.h"

namespace pa = Dyninst::ParseAPI;

/*
 * jt_reread.so (test_binaries/parseAPI/JumpTableReread): fb's jump is first read in fa's context
 * without a bound, past its table into fc's, and later read again with the bound.  Every indirect
 * edge of a jump inside fb must stay inside fb: the over-read targets lie in fc.
 */
int main(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "Usage " << argv[0] << ": file\n";
    return EXIT_FAILURE;
  }
  pa::SymtabCodeSource scs{argv[1]};
  pa::CodeObject co{&scs};
  co.parse();

  Dyninst::Address fb{0};
  Dyninst::Address fc{0};
  for (pa::Function *f : co.funcs()) {
    if (f->name() == "fb")
      fb = f->addr();
    if (f->name() == "fc")
      fc = f->addr();
  }
  if (fb == 0 || fc == 0 || fc <= fb) {
    std::cerr << "functions fb and fc not found\n";
    return EXIT_FAILURE;
  }

  int bad{0};
  for (pa::Function *f : co.funcs()) {
    for (pa::Block *b : f->blocks()) {
      if (b->last() < fb || b->last() >= fc)
        continue;
      for (pa::Edge *e : b->targets()) {
        if (e->type() != pa::INDIRECT || e->sinkEdge())
          continue;
        if (e->trg_addr() < fb || e->trg_addr() >= fc) {
          std::cerr << std::hex << "jump at 0x" << b->last() << " (in " << f->name() << ") has the target 0x"
                    << e->trg_addr() << " outside fb [0x" << fb << ", 0x" << fc << ")\n";
          ++bad;
        }
      }
    }
  }
  return bad == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
