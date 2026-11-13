// noreturn_scan <library> [<mangled-names-file> [<demangled-names-file>]]
//
// Parses <library> with the listed non-returning functions registered and
// prints, one per line:
//
//   EXPORT <mangled>           an exported function found to be non-returning
//   SUSPECT <mangled>\t<why>   an EXPORT whose verdict is doubtful (see below)
//   MISSING <count> <callee>   calls whose fall-through edge runs past the end
//                              of the calling function's symbol
//
// An EXPORT is doubtful when its symbol range holds a return instruction or a
// word the decoder cannot decode (parsing stops there, so a return after it is
// never seen), or when a call or tail call on one of its paths goes to a
// doubtful function, directly or through this library's own PLT ("via
// <callee>"). See README.md.

#include "CFG.h"
#include "CodeObject.h"
#include "CodeSource.h"
#include "InstructionDecoder.h"
#include "Symbol.h"
#include "Symtab.h"
#include "name_file.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace pa = Dyninst::ParseAPI;
namespace st = Dyninst::SymtabAPI;
namespace ia = Dyninst::InstructionAPI;
using Dyninst::Address;

namespace {

// Start -> end of every function symbol with a size (.cold parts are ST_CODE symbols of their own).
using SymbolRanges = std::map<Address, Address>;

SymbolRanges functionSymbolRanges(st::Symtab *obj) {
  SymbolRanges ranges;
  for(auto const type : {st::Symbol::ST_FUNCTION, st::Symbol::ST_CODE}) {
    std::vector<st::Symbol *> syms;
    obj->getAllSymbolsByType(syms, type);
    for(auto const *sym : syms) {
      Address const end = sym->getOffset() + sym->getSize();
      Address &cur = ranges[sym->getOffset()];
      if(cur < end) {
        cur = end;
      }
    }
  }
  return ranges;
}

// End of the sized function symbol that contains addr, or 0 if none does.
Address endOfSymbolContaining(SymbolRanges const &ranges, Address addr) {
  auto it = ranges.upper_bound(addr);
  while(it != ranges.begin()) {
    --it;
    if(it->second > it->first) {
      return addr < it->second ? it->second : 0;
    }
  }
  return 0;
}

std::string calleeName(pa::SymtabCodeSource *scs, pa::Edge *call) {
  if(call->sinkEdge()) {
    return "<indirect>";
  }
  auto const plt = scs->linkage().find(call->trg()->start());
  if(plt != scs->linkage().end()) {
    return plt->second;
  }
  std::vector<pa::Function *> callees;
  call->trg()->getFuncs(callees);
  return callees.empty() ? "<unknown>" : callees.front()->name();
}

// Callee name -> number of calls whose fall-through edge runs past the end of the
// caller's symbol: the callee does not return, but is not known as non-returning.
std::map<std::string, unsigned> escapingCalls(pa::CodeObject *co, pa::SymtabCodeSource *scs,
                                              SymbolRanges const &ranges) {
  std::map<std::string, unsigned> counts;
  std::set<Address> seen;
  for(auto *f : co->funcs()) {
    for(auto *b : f->blocks()) {
      pa::Edge *fallthrough{nullptr};
      pa::Edge *call{nullptr};
      for(auto *e : b->targets()) {
        if(e->type() == pa::CALL_FT && !e->sinkEdge()) {
          fallthrough = e;
        }
        if(e->type() == pa::CALL) {
          call = e;
        }
      }
      if(!fallthrough || !call || !seen.insert(b->last()).second) {
        continue;
      }
      Address const end = endOfSymbolContaining(ranges, b->last());
      if(end && fallthrough->trg()->start() >= end) {
        ++counts[calleeName(scs, call)];
      }
    }
  }
  return counts;
}

// Evidence against f being non-returning from a linear sweep over its symbol:
// "has-ret", "undecodable", both, or "" for none.
std::string evidenceInBytes(pa::Function *f, pa::SymtabCodeSource *scs, st::Symtab *obj,
                            SymbolRanges const &ranges) {
  Address size{0};
  for(auto const *sym : obj->findSymbolByOffset(f->addr())) {
    if(sym->isFunction() && sym->getSize() > size) {
      size = sym->getSize();
    }
  }
  if(size == 0) {
    // Hand-written assembly often has no .size: sweep up to the next symbol.
    auto const next = ranges.upper_bound(f->addr());
    if(next != ranges.end()) {
      size = next->first - f->addr();
    }
  }
  // Stay inside the code region holding the entry.
  std::set<pa::CodeRegion *> regions;
  scs->findRegions(f->addr(), regions);
  if(regions.size() != 1) {
    return "";
  }
  size = std::min(size, (*regions.begin())->high() - f->addr());
  if(size == 0 || !scs->isValidAddress(f->addr())) {
    return "";
  }
  ia::InstructionDecoder decoder{scs->getPtrToInstruction(f->addr()), size, scs->getArch()};
  bool hasReturn{false};
  for(Address offset{0}; offset < size;) {
    ia::Instruction const insn = decoder.decode();
    if(!insn.isValid() || insn.size() == 0) {
      // x86 cannot resynchronize after a bad word, so the sweep ends here on every architecture.
      return hasReturn ? "has-ret undecodable" : "undecodable";
    }
    if(insn.getCategory() == ia::c_ReturnInsn) {
      hasReturn = true;
    }
    offset += insn.size();
  }
  return hasReturn ? "has-ret" : "";
}

// Doubtful non-returning functions with the reason for each.
std::map<pa::Function *, std::string> doubtfulVerdicts(std::vector<pa::Function *> const &noreturn,
                                                       pa::SymtabCodeSource *scs, st::Symtab *obj,
                                                       SymbolRanges const &ranges) {
  std::map<pa::Function *, std::string> why;
  std::set<std::string> doubtfulNames;  // to follow calls through this library's own PLT stubs
  auto markDoubtful = [&](pa::Function *f, std::string reason) {
    why[f] = std::move(reason);
    for(auto const *sym : obj->findSymbolByOffset(f->addr())) {
      if(sym->isFunction()) {
        doubtfulNames.insert(sym->getMangledName());
      }
    }
  };
  for(auto *f : noreturn) {
    std::string reason = evidenceInBytes(f, scs, obj, ranges);
    if(!reason.empty()) {
      markDoubtful(f, std::move(reason));
    }
  }

  // A doubtful callee makes its non-returning callers doubtful; repeat until nothing changes.
  auto doubtfulCallee = [&](pa::Function *f, pa::Edge *e) -> std::string {
    auto const plt = scs->linkage().find(e->trg()->start());
    if(plt != scs->linkage().end() && doubtfulNames.count(plt->second)) {
      return plt->second;
    }
    std::vector<pa::Function *> callees;
    e->trg()->getFuncs(callees);
    for(auto *g : callees) {
      if(g != f && why.count(g)) {
        return g->name();
      }
    }
    return "";
  };
  for(bool changed{true}; changed;) {
    changed = false;
    for(auto *f : noreturn) {
      if(why.count(f)) {
        continue;
      }
      std::string via;
      for(auto *b : f->blocks()) {
        for(auto *e : b->targets()) {
          bool const isCall = e->type() == pa::CALL || (e->interproc() && e->type() == pa::DIRECT);
          if(isCall && !e->sinkEdge()) {
            via = doubtfulCallee(f, e);
          }
          if(!via.empty()) {
            break;
          }
        }
        if(!via.empty()) {
          break;
        }
      }
      if(!via.empty()) {
        markDoubtful(f, "via " + via);
        changed = true;
      }
    }
  }
  return why;
}

}

int main(int argc, char **argv) {
  if(argc < 2 || argc > 4) {
    std::fprintf(stderr, "usage: %s <library> [<mangled-names-file> [<demangled-names-file>]]\n", argv[0]);
    return EXIT_FAILURE;
  }
  if(argc > 2 && noreturn_tools::readNames(argv[2], pa::CodeSource::addNonReturningMangledName) < 0) {
    return EXIT_FAILURE;
  }
  if(argc > 3 && noreturn_tools::readNames(argv[3], pa::CodeSource::addNonReturningDemangledName) < 0) {
    return EXIT_FAILURE;
  }

  auto *scs = new pa::SymtabCodeSource(argv[1]);
  auto *co = new pa::CodeObject(scs);
  co->parse();
  st::Symtab *obj = scs->getSymtabObject();
  SymbolRanges const ranges = functionSymbolRanges(obj);

  std::vector<pa::Function *> noreturn;
  for(auto *f : co->funcs()) {
    if(f->retstatus() == pa::NORETURN) {
      noreturn.push_back(f);
    }
  }
  auto const why = doubtfulVerdicts(noreturn, scs, obj, ranges);

  for(auto *f : noreturn) {
    for(auto const *sym : obj->findSymbolByOffset(f->addr())) {
      if(!sym->isInDynSymtab() || !sym->isFunction() || !sym->getRegion()) {
        continue;
      }
      std::printf("EXPORT %s\n", sym->getMangledName().c_str());
      auto const reason = why.find(f);
      if(reason != why.end()) {
        std::printf("SUSPECT %s\t%s\n", sym->getMangledName().c_str(), reason->second.c_str());
      }
    }
  }
  for(auto const &callee : escapingCalls(co, scs, ranges)) {
    std::printf("MISSING %u %s\n", callee.second, callee.first.c_str());
  }
  return EXIT_SUCCESS;
}
