#include "CFG.h"
#include "debug.h"
#include "find_main.h"
#include "Instruction.h"
#include "Register.h"
#include "registers/x86_64_regs.h"
#include "registers/x86_regs.h"

#include <algorithm>
#include <boost/shared_ptr.hpp>

namespace Dyninst { namespace DyninstAPI { namespace x86_64 {

  namespace pa = Dyninst::ParseAPI;
  namespace di = Dyninst::InstructionAPI;

  struct instruction_t {
    Dyninst::Address address{};
    di::Instruction insn{};

    instruction_t(Dyninst::Address a, di::Instruction i) : address{a}, insn{i} {}
  };

  /*
   * Assuming the last function call in 'entry_point' is to __libc_start_main,
   * the address of 'main' is held in rdi. For example,
   *
   *    lea  rdi,[rip+0xffffffffffffffb1]
   *    call __libc_start_main
   *
   *  or
   *
   *    mov  rdi,0x401050
   *    call QWORD PTR [rip+0x2f4b]
   */
  Dyninst::Address find_main(pa::Function *entry_point) {

    // There is at least one call (e.g., __libc_start_main)
    const auto &call_edges = entry_point->callEdges();
    if(call_edges.empty()) {
      startup_printf("find_main: no call edges\n");
      return Dyninst::ADDR_NULL;
    }

    // __ASSUME__ it's the last call (and so in the last block of the function)
    pa::Edge *call_edge = [&call_edges]() {
      // clang-format off
      auto itr = std::max_element(call_edges.begin(), call_edges.end(),
        [](pa::Edge *e1, pa::Edge *e2){
          return e1->src()->start() > e2->src()->start();
        }
      );
      // clang-format on
      return *itr;
    }();

    // Get all the instructions in the block
    pa::Block::Insns instructions{};
    call_edge->src()->getInsns(instructions);
    if(instructions.size() < 2UL) {
      startup_printf("find_main: block [0x%lx, 0x%lx] has too few instructions\n", call_edge->src()->start(),
                     call_edge->src()->end());
      return Dyninst::ADDR_NULL;
    }
    if(instructions.empty()) {
      startup_printf("find_main: no instructions found\n");
      return Dyninst::ADDR_NULL;
    }

    // Get the last call instruction in the block
    auto callsite_itr = [&instructions]() {
      // clang-format off
      return std::find_if(instructions.rbegin(), instructions.rend(),
        [](pa::Block::Insns::value_type const &val) {
          auto insn = val.second;
          return insn.isCall();
        }
      );
      // clang-format on
    }();
    if(callsite_itr == instructions.rend()) {
      startup_printf("find_main: no call instruction found\n");
      return Dyninst::ADDR_NULL;
    }

    instruction_t callsite{callsite_itr->first, callsite_itr->second};
    std::cerr << std::hex << "Found callsite [0x" << callsite.address << "] " << callsite.insn.format() << "\n";

    auto parameter_register{boost::make_shared<di::RegisterAST>(Dyninst::x86_64::rdi)};

    // Search backward from the call callsite to find the first instruction
    // that writes to the parameter register.
    auto parameter_site_itr = std::find_if(callsite_itr, instructions.rend(),
      // clang-format off
      [&parameter_register](pa::Block::Insns::value_type const &val) {
        auto const &insn = val.second;
        return insn.isWritten(parameter_register);
      }
      // clang-format on
    );
    if(parameter_site_itr == instructions.rend()) {
      startup_printf("find_main: unable to find parameter\n");
      return Dyninst::ADDR_NULL;
    }

    instruction_t parameter_site{parameter_site_itr->first, parameter_site_itr->second};
    std::cerr << std::hex << "Found parameter site [0x" << parameter_site.address << "] "
              << parameter_site.insn.format() << "\n";

    auto operands = parameter_site.insn.getAllOperands();
    if(operands.size() != 2UL) {
      startup_printf("find_main: found %lu explicit operands, expected 2\n", operands.size());
      return Dyninst::ADDR_NULL;
    }

    auto source_operand = [&]() -> di::Expression::Ptr {
      for(di::Operand o : operands) {
        if(*o.getValue() == *parameter_register) {
          // skip the destination operand
          continue;
        }
        return o.getValue();
      }
      return {};
    }();
    if(!source_operand) {
      startup_printf("find_main: couldn't find source operand\n");
      return Dyninst::ADDR_NULL;
    }

    std::cerr << "Found source operand '" << source_operand->format() << "'\n";

    /* Find the parameter's value
     *
     * Can be direct or indirect addressing:
     *
     *    lea rdi,[rip+OFFSET]
     *  or
     *    mov rdi,MAIN
     */
    struct find_address_visitor final : di::Visitor {
      Dyninst::Address imm{Dyninst::ADDR_NULL};
      di::RegisterAST *ast{};

      void visit(di::Immediate *i) override {
        imm = i->eval().convert<Dyninst::Address>();
      }

      void visit(di::RegisterAST *r) override {
        ast = r;
      }

      void visit(di::Dereference *) override {}

      void visit(di::BinaryFunction *) override {}

      void visit(di::MultiRegisterAST *) override {}
    };

    find_address_visitor vis{};
    source_operand->apply(&vis);

    std::cerr << "Visitor results {imm=0x" << std::hex << vis.imm;
    if(vis.ast) {
      std::cerr << ", " << "ast=" << vis.ast->format();
    }
    std::cerr << "}\n";

    if(vis.imm == Dyninst::ADDR_NULL) {
      // This would be a nonsense instruction like 'lea [rip]'
      startup_printf("main_find: parameter calculation dereference has no offset\n");
      return Dyninst::ADDR_NULL;
    }

    // Direct case
    //  NOTE: could be NULL_ADDR if something weird happened and
    //        no immediate was encountered.
    if(!vis.ast) {
      return vis.imm;
    }

    // Indirect case
    return callsite.address + vis.imm;

  }

}}}
