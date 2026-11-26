#include <cstdlib>
#include <iostream>
#include <string>

#include "CFG.h"
#include "CodeObject.h"
#include "InstructionDecoder.h"

namespace pa = Dyninst::ParseAPI;
namespace ia = Dyninst::InstructionAPI;

/*
 * With Dyninst built with Zydis, an x86-64 instruction InstructionAPI cannot decode is returned by
 * the built-in unknown-instruction callback as a valid no-op of the length Zydis decodes, so
 * decoding and parsing continue past it.  libzydis_unknown_insn.so's past_unknown is two such
 * instructions and a ret.
 */
int main(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "Usage " << argv[0] << ": file\n";
    return EXIT_FAILURE;
  }
  int bad{0};

  // The decoder steps over both instructions and decodes the ret after them.
  unsigned char const bytes[]{0xc5, 0xfc, 0x29, 0x84, 0x24, 0xe0, 0x00, 0x00, 0x00, // vmovaps %ymm0,0xe0(%rsp)
                              0xc5, 0xf9, 0xd6, 0x43, 0x20,                         // vmovq %xmm0,0x20(%rbx)
                              0xc3};                                                // ret
  ia::InstructionDecoder dec{bytes, sizeof(bytes), Dyninst::Arch_x86_64};
  unsigned const sizes[]{9, 5, 1};
  for (unsigned const size : sizes) {
    ia::Instruction const insn{dec.decode()};
    if (!insn.isValid() || insn.size() != size) {
      std::cerr << "decoded an instruction that is " << (insn.isValid() ? "valid" : "not valid") << ", size "
                << insn.size() << ", expected a valid instruction of size " << size << '\n';
      ++bad;
    }
  }

  // The parse reaches past_unknown's ret: the function returns, and no block ends in the sink except
  // through the ret's own return edge.
  pa::SymtabCodeSource scs{argv[1]};
  pa::CodeObject co{&scs};
  co.parse();
  pa::Function *f{nullptr};
  for (pa::Function *g : co.funcs()) {
    if (g->name() == "past_unknown")
      f = g;
  }
  if (f == nullptr) {
    std::cerr << "function past_unknown not found\n";
    return EXIT_FAILURE;
  }
  if (f->retstatus() != pa::RETURN) {
    std::cerr << "past_unknown: return status " << f->retstatus() << ", expected RETURN\n";
    ++bad;
  }
  for (pa::Block *b : f->blocks()) {
    for (pa::Edge *e : b->targets()) {
      if (e->sinkEdge() && e->type() != pa::RET) {
        std::cerr << std::hex << "block 0x" << b->start() << " ends in the sink at 0x" << b->last() << '\n';
        ++bad;
      }
    }
  }
  return bad == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
