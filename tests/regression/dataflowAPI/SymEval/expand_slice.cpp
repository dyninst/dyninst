/*
 * SymEval::expand of a slice in the entry block of an executable.
 *
 * For each file: parse the function at the entry point, take the first
 * assignment to a register other than a flag in its entry block (the startup
 * code always writes one), slice backward from it and expand the slice with
 * the default simplification. The expansion must not throw and must give that
 * assignment an expression. The simplification runs StackAnalysis, which cannot analyze
 * every architecture's code on every build.
 *
 * Usage: expand_slice [--skip <reason>] <executable>...
 * With --skip, or without executables, the test reports a skip.
 */
#include "AbslocInterface.h"
#include "CFG.h"
#include "CodeObject.h"
#include "CodeSource.h"
#include "DynAST.h"
#include "slicing.h"
#include "SymEval.h"
#include "Symtab.h"

#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>
#include <vector>

namespace pa = Dyninst::ParseAPI;
namespace st = Dyninst::SymtabAPI;
namespace df = Dyninst::DataflowAPI;

namespace {

  // The first assignment to a register in b that is not a flag, and its address.
  // An instruction's flag assignments come in no fixed order.
  bool first_register_write(pa::Function* f, pa::Block* b, Dyninst::Assignment::Ptr& out, Dyninst::Address& addr) {
    pa::Block::Insns insns;
    b->getInsns(insns);
    for(auto const& i : insns) {
      Dyninst::AssignmentConverter conv(true, false);
      std::vector<Dyninst::Assignment::Ptr> assigns;
      conv.convert(i.second, i.first, f, b, assigns);
      for(auto const& a : assigns) {
        Dyninst::Absloc const& loc = a->out().absloc();
        if(loc.type() == Dyninst::Absloc::Register && !loc.reg().isFlag()) {
          out = a;
          addr = i.first;
          return true;
        }
      }
    }
    return false;
  }

  // The expression expand gave the slice's node for a
  Dyninst::AST::Ptr value(Dyninst::GraphPtr const& g, Dyninst::Assignment::Ptr const& a, df::Result_t& res) {
    Dyninst::NodeIterator b, e;
    g->allNodes(b, e);
    for(; b != e; ++b) {
      auto node = boost::static_pointer_cast<Dyninst::SliceNode>(*b);
      if(node->assign() && node->assign()->addr() == a->addr() && node->assign()->out() == a->out()) {
        return res[node->assign()];
      }
    }
    return Dyninst::AST::Ptr();
  }

  bool check(char const* path) {
    st::Symtab* symtab{};
    if(!st::Symtab::openFile(symtab, path)) {
      std::cerr << path << ": cannot open\n";
      return false;
    }
    pa::SymtabCodeSource source(symtab);
    pa::CodeObject co(&source);
    const Dyninst::Address entry = symtab->getEntryOffset();
    co.parse(entry, false);
    std::set<pa::CodeRegion*> regions;
    source.findRegions(entry, regions);
    pa::Function* f = regions.empty() ? nullptr : co.findFuncByEntry(*regions.begin(), entry);
    if(!f || !f->entry()) {
      std::cerr << path << ": no function at the entry point 0x" << std::hex << entry << std::dec << "\n";
      return false;
    }

    Dyninst::Assignment::Ptr a;
    Dyninst::Address addr{};
    if(!first_register_write(f, f->entry(), a, addr)) {
      std::cerr << path << ": the entry block writes no register\n";
      return false;
    }

    Dyninst::Slicer slicer(a, f->entry(), f, true, false);
    Dyninst::Slicer::Predicates preds;
    Dyninst::GraphPtr g = slicer.backwardSlice(preds);

    df::Result_t res;
    df::SymEval::Retval_t rv{};
    try {
      rv = df::SymEval::expand(g, res);
    } catch(std::exception const& e) {
      std::cerr << path << ": expand threw: " << e.what() << "\n";
      return false;
    }
    Dyninst::AST::Ptr ast = value(g, a, res);
    std::cout << path << ": 0x" << std::hex << addr << std::dec << " " << a->format() << " = "
              << (ast ? ast->format() : "<none>") << "\n";
    if(rv == df::SymEval::FAILED || !ast) {
      std::cerr << path << ": no expression for the assignment\n";
      return false;
    }
    return true;
  }

}

int main(int argc, char** argv) {
  if(argc >= 3 && std::string{argv[1]} == "--skip") {
    std::cout << "skipping: " << argv[2] << "\n";
    return EXIT_SUCCESS;
  }
  if(argc < 2) {
    std::cout << "skipping: needs the test binaries (DYNINST_ENABLE_TESTS=ALL)\n";
    return EXIT_SUCCESS;
  }
  bool ok = true;
  for(int i = 1; i < argc; ++i) {
    ok = check(argv[i]) && ok;
  }
  return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
