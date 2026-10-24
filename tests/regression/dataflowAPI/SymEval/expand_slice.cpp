/*
 * SymEval::expand of a slice with and without its visitors.
 *
 * The slice is that of 'mov x6, sp' in the _start of an aarch64 executable
 * (glibc's start.S passes the initial stack pointer to __libc_start_main in
 * x6). The expression for x6 is the stack pointer.
 *
 *  - Without the visitors (applyVisitors = false) the stack pointer must stay a
 *    plain input variable, and nothing may run StackAnalysis, which cannot
 *    analyze code on every build.
 *  - With them (the two-argument expand, unchanged), the stack pointer is
 *    replaced by a stack height where StackAnalysis can produce one. On builds
 *    whose StackAnalysis has no entry state for the code-generation
 *    architecture it throws "Unimplemented architecture"; that is accepted as
 *    the existing behaviour and reported.
 */
#include "AbslocInterface.h"
#include "CFG.h"
#include "CodeObject.h"
#include "CodeSource.h"
#include "DynAST.h"
#include "slicing.h"
#include "SymEval.h"
#include "Symtab.h"
#include "registers/aarch64_regs.h"

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

namespace pa = Dyninst::ParseAPI;
namespace st = Dyninst::SymtabAPI;
namespace df = Dyninst::DataflowAPI;

namespace {

  bool contains(Dyninst::AST::Ptr const& ast, Dyninst::AST::ID id) {
    if(!ast) {
      return false;
    }
    if(ast->getID() == id) {
      return true;
    }
    for(unsigned i = 0; i < ast->numChildren(); ++i) {
      if(contains(ast->child(i), id)) {
        return true;
      }
    }
    return false;
  }

  bool reads_sp(Dyninst::AST::Ptr const& ast) {
    if(!ast) {
      return false;
    }
    if(ast->getID() == Dyninst::AST::V_VariableAST) {
      Dyninst::Absloc const& loc = df::VariableAST::convert(ast)->val().reg.absloc();
      return loc.type() == Dyninst::Absloc::Register && loc.reg() == Dyninst::aarch64::sp;
    }
    for(unsigned i = 0; i < ast->numChildren(); ++i) {
      if(reads_sp(ast->child(i))) {
        return true;
      }
    }
    return false;
  }

  struct slice_of_x6 {
    Dyninst::GraphPtr graph;
    Dyninst::Address addr{};
    Dyninst::AbsRegion out;
  };

  // The slice of the instruction in f that writes x6
  bool slice_x6(pa::Function* f, slice_of_x6& s) {
    for(pa::Block* b : f->blocks()) {
      pa::Block::Insns insns;
      b->getInsns(insns);
      for(auto const& i : insns) {
        Dyninst::AssignmentConverter conv(true, false);
        std::vector<Dyninst::Assignment::Ptr> assigns;
        conv.convert(i.second, i.first, f, b, assigns);
        for(auto const& a : assigns) {
          if(a->out().absloc().type() == Dyninst::Absloc::Register &&
             a->out().absloc().reg() == Dyninst::aarch64::x6) {
            Dyninst::Slicer slicer(a, b, f, true, false);
            Dyninst::Slicer::Predicates preds;
            s.graph = slicer.backwardSlice(preds);
            s.addr = i.first;
            s.out = a->out();
            return true;
          }
        }
      }
    }
    return false;
  }

  // The expression expand gave the slice's node for the x6 assignment
  Dyninst::AST::Ptr value(slice_of_x6 const& s, df::Result_t& res) {
    Dyninst::NodeIterator b, e;
    s.graph->allNodes(b, e);
    for(; b != e; ++b) {
      auto node = boost::static_pointer_cast<Dyninst::SliceNode>(*b);
      if(node->assign() && node->assign()->addr() == s.addr && node->assign()->out() == s.out) {
        return res[node->assign()];
      }
    }
    return Dyninst::AST::Ptr();
  }

}

int main(int argc, char** argv) {
  if(argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <aarch64 executable with a _start symbol>\n";
    return EXIT_FAILURE;
  }

  st::Symtab* symtab{};
  if(!st::Symtab::openFile(symtab, argv[1])) {
    std::cerr << "cannot open " << argv[1] << "\n";
    return EXIT_FAILURE;
  }
  pa::SymtabCodeSource source(symtab);
  pa::CodeObject co(&source);
  const Dyninst::Address entry = symtab->getEntryOffset();
  co.parse(entry, false);
  std::set<pa::CodeRegion*> regions;
  source.findRegions(entry, regions);
  pa::Function* start = regions.empty() ? nullptr : co.findFuncByEntry(*regions.begin(), entry);
  if(!start) {
    std::cerr << "no function at the entry point 0x" << std::hex << entry << "\n";
    return EXIT_FAILURE;
  }

  slice_of_x6 s;
  if(!slice_x6(start, s)) {
    std::cerr << "_start does not write x6\n";
    return EXIT_FAILURE;
  }

  bool ok = true;

  {
    df::Result_t res;
    constexpr bool apply_visitors = false;
    try {
      df::SymEval::expand(s.graph, res, apply_visitors);
    } catch(std::exception const& e) {
      std::cerr << "expand without visitors threw: " << e.what() << "\n";
      return EXIT_FAILURE;
    }
    Dyninst::AST::Ptr x6 = value(s, res);
    std::cout << "without visitors: x6 = " << (x6 ? x6->format() : "<none>") << "\n";
    if(!reads_sp(x6) || contains(x6, Dyninst::AST::V_StackAST)) {
      std::cerr << "expected the plain stack pointer\n";
      ok = false;
    }
  }

  {
    df::Result_t res;
    try {
      df::SymEval::expand(s.graph, res);
      Dyninst::AST::Ptr x6 = value(s, res);
      std::cout << "with visitors: x6 = " << (x6 ? x6->format() : "<none>") << "\n";
      if(!contains(x6, Dyninst::AST::V_StackAST)) {
        std::cerr << "expected a stack height\n";
        ok = false;
      }
    } catch(std::runtime_error const& e) {
      std::cout << "with visitors: " << e.what() << "\n";
      if(std::string{e.what()}.find("Unimplemented architecture") == std::string::npos) {
        ok = false;
      }
    }
  }

  return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
