#include "Absloc.h"
#include "AbslocInterface.h"
#include "CFG.h"
#include "CodeObject.h"
#include "Graph.h"
#include "slicing.h"

#include "../../parseAPI/memory_code_source.h"

#include <cstdlib>
#include <iostream>
#include <vector>

/*
 * A forward slice through the end of a block whose out-edges are not a plain
 * branch:
 *
 * - a system call is a call site with no callee, so the slice continues at its
 *   CALL_FT without widening;
 * - a ppc conditional return also falls through, so the slice follows the
 *   not-taken path.
 *
 * The code is hand-encoded, so both are tested on any host.
 */

namespace {
  namespace pa = Dyninst::ParseAPI;
  namespace mcs = memory_code_source;
  using Dyninst::Address;

  struct Slice {
    bool reached;  // a node for the instruction at `use`
    bool widened;  // the slicer gave up somewhere
  };

  bool forward_slice(char const *name, Dyninst::Architecture arch, std::vector<mcs::Insn> const &insns,
                     Address def, Address use, Slice &out) {
    auto *src = new mcs::Source(arch, insns, {{0, "f"}});
    auto *co = new pa::CodeObject(src);
    co->parse();
    bool ok = false;
    pa::Function *f = co->findFuncByEntry(src->region(), 0);
    if(!f) {
      std::cerr << name << ": no function\n";
    } else {
      // The register that the instruction at `def` writes
      Dyninst::Assignment::Ptr a;
      pa::Block *b = f->entry();
      pa::Block::Insns bi;
      b->getInsns(bi);
      Dyninst::AssignmentConverter conv(true, false);
      std::vector<Dyninst::Assignment::Ptr> as;
      conv.convert(bi.at(def), def, f, b, as);
      for(auto const &x : as) {
        Dyninst::Absloc const &loc = x->out().absloc();
        if(loc.type() == Dyninst::Absloc::Register && !loc.reg().isFlag()) {
          a = x;
          break;
        }
      }
      if(!a) {
        std::cerr << name << ": the instruction at 0x" << std::hex << def << " writes no register\n";
      } else {
        Dyninst::Slicer slicer(a, b, f, false, false);
        Dyninst::Slicer::Predicates preds;
        Dyninst::GraphPtr g = slicer.forwardSlice(preds);
        out = {false, false};
        Dyninst::NodeIterator nb, ne;
        g->allNodes(nb, ne);
        for(; nb != ne; ++nb) {
          auto node = boost::dynamic_pointer_cast<Dyninst::SliceNode>(*nb);
          if(!node) {
            continue;
          }
          if(!node->assign()) {
            out.widened = true;
          } else if(node->assign()->addr() == use) {
            out.reached = true;
          }
        }
        ok = true;
      }
    }
    delete co;
    delete src;
    return ok;
  }

  bool syscall_x86_64() {
    // clang-format off
    std::vector<mcs::Insn> const insns = {
      {0x0, {0xbb, 0x01, 0x00, 0x00, 0x00}},  // mov $1, %ebx
      {0x5, {0x0f, 0x05}},                    // syscall
      {0x7, {0x89, 0xd9}},                    // mov %ebx, %ecx
      {0x9, {0xc3}},                          // ret
    };
    // clang-format on
    Slice s{};
    if(!forward_slice("x86_64 syscall", Dyninst::Arch_x86_64, insns, 0x0, 0x7, s)) {
      return false;
    }
    bool ok = true;
    if(!s.reached) {
      std::cerr << "x86_64 syscall: the slice of %ebx does not reach the instruction after the syscall\n";
      ok = false;
    }
    if(s.widened) {
      std::cerr << "x86_64 syscall: the slice widens at the syscall\n";
      ok = false;
    }
    std::cerr << "x86_64 syscall: " << (ok ? "ok" : "FAILED") << "\n";
    return ok;
  }

  bool conditional_return_ppc64le() {
    // clang-format off
    std::vector<mcs::Insn> const insns = {
      {0x00, {0x01, 0x00, 0x80, 0x38}},  // li r4, 1
      {0x04, {0x00, 0x00, 0xa3, 0x2f}},  // cmpdi cr7, r3, 0
      {0x08, {0x20, 0x00, 0x9e, 0x4d}},  // beqlr cr7
      {0x0c, {0x78, 0x23, 0x85, 0x7c}},  // mr r5, r4
      {0x10, {0x20, 0x00, 0x80, 0x4e}},  // blr
    };
    // clang-format on
    Slice s{};
    if(!forward_slice("ppc64le beqlr", Dyninst::Arch_ppc64, insns, 0x00, 0x0c, s)) {
      return false;
    }
    bool ok = s.reached;
    if(!ok) {
      std::cerr << "ppc64le beqlr: the slice of r4 does not reach the not-taken path\n";
    }
    std::cerr << "ppc64le beqlr: " << (ok ? "ok" : "FAILED") << "\n";
    return ok;
  }
}

int main() {
  bool ok = true;
  ok = syscall_x86_64() && ok;
  ok = conditional_return_ppc64le() && ok;
  return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
