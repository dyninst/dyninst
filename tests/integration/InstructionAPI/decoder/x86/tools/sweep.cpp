// Decode fixed-size slots of a binary blob with Dyninst and print
// offset|valid|length|mnemonic|format per slot, for differential
// comparison against objdump (driven by gen_and_compare.py).
//
// Usage: sweep <blob> <slotsize> [arch32]
//
// Build (from the repo root, with a configured build/ directory):
//   g++ -std=c++11 -O1 -o tests/integration/InstructionAPI/decoder/x86/tools/sweep \
//       tests/integration/InstructionAPI/decoder/x86/tools/sweep.cpp \
//       -IinstructionAPI/h -Icommon/h -Ibuild/common/h \
//       -Lbuild/instructionAPI -Lbuild/common -linstructionAPI -lcommon \
//       -Wl,-rpath,$PWD/build/instructionAPI -Wl,-rpath,$PWD/build/common
#include "InstructionDecoder.h"
#include "Instruction.h"
#include "Architecture.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

using namespace Dyninst;
using namespace Dyninst::InstructionAPI;

int main(int argc, char** argv) {
  if(argc < 3) {
    fprintf(stderr, "usage: %s blob slotsize [arch32]\n", argv[0]);
    return 1;
  }
  FILE* f = fopen(argv[1], "rb");
  if(!f) { perror("open"); return 1; }
  std::vector<unsigned char> blob;
  unsigned char buf[65536];
  size_t n;
  while((n = fread(buf, 1, sizeof buf, f)) > 0) blob.insert(blob.end(), buf, buf + n);
  fclose(f);
  size_t slot = strtoul(argv[2], nullptr, 0);
  Architecture arch = (argc > 3) ? Arch_x86 : Arch_x86_64;

  for(size_t off = 0; off + slot <= blob.size(); off += slot) {
    InstructionDecoder d(blob.data() + off, slot, arch);
    Instruction insn = d.decode();
    if(!insn.isValid()) {
      printf("%zx|0|0||\n", off);
      continue;
    }
    std::string fmt = insn.format();
    // mnemonic = first token of format
    std::string mnem = fmt.substr(0, fmt.find(' '));
    printf("%zx|1|%u|%s|%s\n", off, (unsigned)insn.size(), mnem.c_str(), fmt.c_str());
  }
  return 0;
}
