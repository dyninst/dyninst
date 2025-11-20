#include "memory_tests.h"

#include "Architecture.h"
#include "InstructionAST.h"
#include "InstructionDecoder.h"

#include <iostream>
#include <vector>

namespace di = Dyninst::InstructionAPI;

static Dyninst::register_set convert(std::set<di::Expression::Ptr> exprs) {
  Dyninst::register_set regs;
  for(auto &&e : exprs) {
    for(auto &&r : getUsedRegisters(e)) {
      regs.insert(r->getID());
    }
  }
  return regs;
}

namespace Dyninst { namespace InstructionAPI {

    bool verify(Instruction const &insn, mem_test const &expected) {
      bool failed = false;

      if(insn.readsMemory() != expected.readsMemory) {
        std::cerr << std::boolalpha << "Expected readsMemory = " << expected.readsMemory << ", got '"
                  << insn.readsMemory() << "'\n";
        failed = true;
      }

      if(insn.writesMemory() != expected.writesMemory) {
        std::cerr << std::boolalpha << "Expected writesMemory = " << expected.writesMemory << ", got '"
                  << insn.writesMemory() << "'\n";
        failed = true;
      }

      auto read_ops = [&insn]() {
        std::set<di::Expression::Ptr> exprs;
        insn.getMemoryReadOperands(exprs);
        return convert(exprs);
      }();
      if(read_ops != expected.regs.read) {
        std::cerr << "Mismatched memory READ set\n";
        di::show_register_diff_set(read_ops, expected.regs.read);
        std::cerr << '\n';
        failed = true;
      }

      auto written_ops = [&insn]() {
        std::set<di::Expression::Ptr> exprs;
        insn.getMemoryWriteOperands(exprs);
        return convert(exprs);
      }();
      if(written_ops != expected.regs.written) {
        std::cerr << "Mismatched memory WRITE set \n";
        show_register_diff_set(written_ops, expected.regs.written);
        std::cerr << '\n';
        failed = true;
      }

      return !failed;
    }

}}
