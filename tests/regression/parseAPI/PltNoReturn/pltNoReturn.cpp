#include <cstdlib>
#include <iostream>
#include <map>
#include <set>
#include <string>

#include "CFG.h"
#include "CodeObject.h"

namespace pa = Dyninst::ParseAPI;

/*
 * libplt_noret.so: exported functions that call each other through PLT stubs whose jump slots bind
 * to definitions in the same library.  fatal_inner calls abort, so it never returns; fatal calls
 * fatal_inner; caller calls ok and then fatal; tail calls fatal.  fatal and tail must not return,
 * ok must, and of the calls through stubs to functions of this library only those to ok may have a
 * fallthrough.
 */
int main(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "Usage " << argv[0] << ": file\n";
    return EXIT_FAILURE;
  }
  pa::SymtabCodeSource scs{argv[1]};
  pa::CodeObject co{&scs};
  co.parse();

  std::map<Dyninst::Address, std::string> const &stubs{scs.linkage()};
  std::map<std::string, pa::Function *> defs;
  for (pa::Function *f : co.funcs()) {
    if (stubs.find(f->addr()) == stubs.end())
      defs[f->name()] = f;
  }

  int bad{0};
  auto const expect = [&](char const *name, pa::FuncReturnStatus status) {
    auto const it{defs.find(name)};
    if (it == defs.end()) {
      std::cerr << "function " << name << " not found\n";
      ++bad;
    } else if (it->second->retstatus() != status) {
      std::cerr << "function " << name << ": return status " << it->second->retstatus() << ", expected " << status
                << '\n';
      ++bad;
    }
  };
  expect("fatal_inner", pa::NORETURN);
  expect("fatal", pa::NORETURN);
  expect("tail", pa::NORETURN);
  expect("ok", pa::RETURN);

  // A block can belong to more than one function, so each call block is checked once.
  std::set<pa::Block *> seen;
  std::set<std::string> called;
  for (auto const &d : defs) {
    for (pa::Block *b : d.second->blocks()) {
      if (!seen.insert(b).second)
        continue;
      pa::Edge *call{nullptr};
      bool hasFallthrough{false};
      for (pa::Edge *e : b->targets()) {
        if (e->type() == pa::CALL && !e->sinkEdge())
          call = e;
        if (e->type() == pa::CALL_FT)
          hasFallthrough = true;
      }
      if (call == nullptr)
        continue;
      auto const stub{stubs.find(call->trg()->start())};
      if (stub == stubs.end() || defs.find(stub->second) == defs.end())
        continue;
      called.insert(stub->second);
      bool const expected{stub->second == "ok"};
      if (hasFallthrough != expected) {
        std::cerr << std::hex << "call at 0x" << b->last() << " to " << stub->second << "@plt "
                  << (hasFallthrough ? "has" : "has no") << " fallthrough\n";
        ++bad;
      }
    }
  }
  for (char const *name : {"ok", "fatal", "fatal_inner"}) {
    if (called.find(name) == called.end()) {
      std::cerr << "no call through the PLT stub of " << name << " found\n";
      ++bad;
    }
  }
  return bad == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
