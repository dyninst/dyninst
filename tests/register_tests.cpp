#include "register_tests.h"

#include "Architecture.h"
#include "InstructionDecoder.h"

#include <iostream>

namespace Dyninst { namespace InstructionAPI {

  bool verify(Instruction const &insn, register_rw_test const &expected) {
    auto convert = [](std::set<RegisterAST::Ptr> const &regs) {
      register_set rs;
      for(auto r : regs) {
        rs.insert(r->getID());
      }
      return rs;
    };
    auto actual_read = [&insn, &convert]() {
      std::set<RegisterAST::Ptr> regs;
      insn.getReadSet(regs);
      return convert(regs);
    }();
    auto actual_written = [&insn, &convert]() {
      std::set<RegisterAST::Ptr> regs;
      insn.getWriteSet(regs);
      return convert(regs);
    }();

    bool failed = false;

    if(actual_read != expected.read) {
      std::cerr << "Mismatched register READ set\n";
      show_register_diff_set(actual_read, expected.read);
      std::cerr << '\n';
      failed = true;
    }

    if(actual_written != expected.written) {
      std::cerr << "Mismatched register WRITE set\n";
      show_register_diff_set(actual_written, expected.written);
      std::cerr << '\n';
      failed = true;
    }

    return !failed;
  }

  // clang-format off
  void show_register_diff_set(Dyninst::register_set const &actual, Dyninst::register_set const &expected) {
    auto intersection = actual & expected;
    bool found_unused = false;
    {
      auto unused = actual - intersection;
      if(!unused.is_empty()) {
        std::cerr << "Used but not expected: { ";
        for(auto const &r : unused) {
          std::cerr << r.name() << ", ";
        }
        std::cerr << "}";
        found_unused = true;
      }
    }
    if(found_unused) {
      std::cerr << "\n";
    }
    {
      auto unused = expected - intersection;
      if(!unused.is_empty()) {
        std::cerr << "Expected but not used: { ";
        for(auto const &r : unused) {
          std::cerr << r.name() << ", ";
        }
        std::cerr << "}";
        found_unused = true;
      }
    }
    if(found_unused) {
      std::cerr << "\n";
    }
  }

  // clang-format on

}}
