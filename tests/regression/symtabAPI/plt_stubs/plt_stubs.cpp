// Checks the address SymtabAPI records for each PLT stub, against what the
// linker put there.
//
// See test_binaries/symtabAPI/plt_stubs, which holds the binaries and, beside
// each one, a .expected file naming the stub address of every function it
// imports.  Those addresses are read from the linker's own record -- objdump's
// <name@plt> labels, the relocation a lazy .plt entry pushes, or the .plt_call
// symbol on a ppc64le stub -- so this compares Dyninst against the linker
// rather than against itself.
//
// The binaries are never run, and none is for the architecture this is built
// for: SymtabAPI reads any of them, which is what lets one runner check all
// five.
//
//   plt_stubs <binary> <expected file>
//
// Every pair in the expected file must appear in the binding table.  Extra
// entries are reported and allowed: on ppc64le the table holds the data .plt
// as well as the stubs.

#include "Symtab.h"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace st = Dyninst::SymtabAPI;

int main(int argc, char **argv) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <binary> <expected file>\n";
    return EXIT_FAILURE;
  }

  std::map<std::string, Dyninst::Offset> expected;
  {
    std::ifstream in(argv[2]);
    if (!in) {
      std::cerr << "Unable to read '" << argv[2] << "'\n";
      return EXIT_FAILURE;
    }
    std::string line;
    while (std::getline(in, line)) {
      if (line.empty() || line[0] == '#') {
        continue;
      }
      std::istringstream is(line);
      std::string addr, name;
      if (!(is >> addr >> name)) {
        std::cerr << "Malformed line in '" << argv[2] << "': " << line << '\n';
        return EXIT_FAILURE;
      }
      expected[name] = std::stoull(addr, nullptr, 16);
    }
  }
  if (expected.empty()) {
    std::cerr << "'" << argv[2] << "' names no stubs\n";
    return EXIT_FAILURE;
  }

  st::Symtab *symtab{};
  if (!st::Symtab::openFile(symtab, argv[1])) {
    std::cerr << "Unable to open '" << argv[1] << "'\n";
    return EXIT_FAILURE;
  }

  std::vector<st::relocationEntry> fbt;
  symtab->getFuncBindingTable(fbt);

  // By name, since the table is in relocation order and may hold more than
  // the stubs.  A name appearing twice at different addresses is itself a
  // failure: it is what a binary looks like when the stub addresses have
  // collapsed onto one another.
  std::map<std::string, std::vector<Dyninst::Offset>> got;
  for (auto const &r : fbt) {
    got[r.name()].push_back(r.target_addr());
  }

  int failures = 0;
  for (auto const &want : expected) {
    auto found = got.find(want.first);
    if (found == got.end()) {
      std::cerr << want.first << ": no entry in the function binding table\n";
      failures++;
      continue;
    }
    bool ok = false;
    for (auto addr : found->second) {
      if (addr == want.second) {
        ok = true;
      }
    }
    if (!ok) {
      std::cerr << want.first << ": expected stub at 0x" << std::hex
                << want.second << ", found";
      for (auto addr : found->second) {
        std::cerr << " 0x" << addr;
      }
      std::cerr << std::dec << '\n';
      failures++;
    }
  }

  // Distinct functions sharing a stub address is the shape of a PLT read with
  // the wrong stride: every relocation lands on the same place.
  std::map<Dyninst::Offset, std::string> byAddr;
  for (auto const &want : expected) {
    auto prev = byAddr.find(want.second);
    if (prev != byAddr.end()) {
      std::cerr << "0x" << std::hex << want.second << std::dec
                << ": expected for both " << prev->second << " and "
                << want.first << '\n';
      failures++;
    }
    byAddr[want.second] = want.first;
  }

  if (fbt.size() > expected.size()) {
    std::cout << argv[1] << ": " << fbt.size() << " entries for "
              << expected.size() << " stubs; the rest are not checked\n";
  }
  if (failures) {
    std::cerr << argv[1] << ": " << failures << " of " << expected.size()
              << " stubs wrong\n";
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
