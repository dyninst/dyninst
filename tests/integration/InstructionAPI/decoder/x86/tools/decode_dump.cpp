// Ad-hoc decoder inspector: decode byte sequences with Dyninst's
// InstructionAPI and print everything an SDM comparison needs -- format,
// entry ID, length, register read/write sets, and memory access
// direction with the address registers.
//
// Usage:
//   decode_dump [-32] "f3 0f ae 37" "0f 01 f9" ...
//   echo "66 0f ae 37" | decode_dump [-32]
//
// Build (from the repo root, with a configured build/ directory):
//   g++ -std=c++11 -o decode_dump \
//       tests/integration/InstructionAPI/decoder/x86/tools/decode_dump.cpp \
//       -IinstructionAPI/h -Icommon/h -Ibuild/common/h \
//       -Lbuild/instructionAPI -Lbuild/common -linstructionAPI -lcommon \
//       -Wl,-rpath,$PWD/build/instructionAPI -Wl,-rpath,$PWD/build/common

#include "Architecture.h"
#include "Instruction.h"
#include "InstructionDecoder.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <vector>

using namespace Dyninst;
using namespace Dyninst::InstructionAPI;

static bool parse_hex(const std::string& text, std::vector<unsigned char>& out) {
  std::istringstream in(text);
  std::string tok;
  while(in >> tok) {
    char* end = nullptr;
    long v = strtol(tok.c_str(), &end, 16);
    if(end == tok.c_str() || *end != '\0' || v < 0 || v > 0xFF) {
      return false;
    }
    out.push_back(static_cast<unsigned char>(v));
  }
  return !out.empty();
}

static void dump(Architecture arch, const std::string& text) {
  std::vector<unsigned char> bytes;
  if(!parse_hex(text, bytes)) {
    printf("%-30s | cannot parse hex bytes\n", text.c_str());
    return;
  }

  InstructionDecoder d(bytes.data(), bytes.size(), arch);
  Instruction insn = d.decode();
  if(!insn.isValid()) {
    printf("%-30s | INVALID\n", text.c_str());
    return;
  }

  std::set<RegisterAST::Ptr> rs, ws;
  insn.getReadSet(rs);
  insn.getWriteSet(ws);
  std::string r, w;
  for(auto const& x : rs) {
    r += x->format() + " ";
  }
  for(auto const& x : ws) {
    w += x->format() + " ";
  }

  std::set<Expression::Ptr> mr, mw;
  insn.getMemoryReadOperands(mr);
  insn.getMemoryWriteOperands(mw);

  printf("%-30s | '%s' id=%d len=%u R:{%s} W:{%s} memR=%zu memW=%zu cft=%d\n", text.c_str(),
         insn.format().c_str(), insn.getOperation().getID(), (unsigned)insn.size(), r.c_str(),
         w.c_str(), mr.size(), mw.size(), insn.getControlFlowTarget() ? 1 : 0);
}

int main(int argc, char** argv) {
  Architecture arch = Arch_x86_64;
  int first = 1;
  if(argc > 1 && strcmp(argv[1], "-32") == 0) {
    arch = Arch_x86;
    first = 2;
  }

  if(first < argc) {
    for(int i = first; i < argc; i++) {
      dump(arch, argv[i]);
    }
    return 0;
  }

  std::string line;
  while(std::getline(std::cin, line)) {
    if(line.empty() || line[0] == '#') {
      continue;
    }
    dump(arch, line);
  }
  return 0;
}
