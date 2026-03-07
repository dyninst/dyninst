#include "find_main_common.h"
#include "registers/x86_regs.h"

namespace Dyninst { namespace DyninstAPI { namespace x86 {

  namespace pa = Dyninst::ParseAPI;
  namespace di = Dyninst::InstructionAPI;

  Dyninst::Address find_main(pa::Function *entry_point) {

    // There is at least one call (e.g., __libc_start_main)
    const auto &call_edges = entry_point->callEdges();
    if(call_edges.empty()) {
      FIND_MAIN_FAIL("find_main: no call edges\n");
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
      auto *src = call_edge->src();
      FIND_MAIN_FAIL("find_main: too few instructions in block [0x%lx, 0x%lx]\n", src->start(), src->end());
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
      FIND_MAIN_FAIL("find_main: no call instruction found\n");
    }

    std::cerr << std::hex << "Found callsite [0x" << callsite_itr->first << "] " << callsite_itr->second.format()
              << "\n";

    // 32-bit __libc_start_main expects the address of `main` to be on the stack
    rev_itr stack_param_itr = [&callsite_itr, &instructions]() {
      auto sp = MachRegister::getStackPointer(Dyninst::Arch_x86);

      // The stack pointer is written at the callsite, so start checking from the previous instruction
      auto start = std::prev(callsite_itr);

      return find_mrw_to(start, instructions.rend(), sp);
    }();
    if(stack_param_itr == instructions.rend()) {
      FIND_MAIN_FAIL("find_main: no stack parameter found\n");
    }

    std::cerr << std::hex << "find_main_indirect: Found stack param [0x" << stack_param_itr->first << "] "
              << stack_param_itr->second.format() << "\n";

    // Get the source pushed onto the stack
    find_src_visitor stack_param{};
    auto stack_insn = stack_param_itr->second;
    {
      auto operands = stack_insn.getExplicitOperands();
      operands[0].getValue()->apply(&stack_param);
    }

    /*  case 1: address is an immediate
     *
     *    push ADDRESS
     *    call __libc_start_main
     *
     */
    if(!stack_param.reg.isValid()) {
      if(stack_param.deref) {
        // Assume there isn't something weird like `push [ADDRESS]`
        FIND_MAIN_FAIL("find_main: unexpected stack passing: '%s'\n", stack_insn.format().c_str());
      }
      if(!stack_param.imm) {
        FIND_MAIN_FAIL("find_main: failed to determine stack passing: '%s'\n", stack_insn.format().c_str());
      }
      return stack_param.imm->eval().convert<Dyninst::Address>();
    }

    std::cerr << "find_main_indirect: Found stack reg " << stack_param.reg.name() << "\n";

    // Find the most-recent write to the stack parameter register
    rev_itr mwr_to_source_itr = find_mrw_to(stack_param_itr, instructions.rend(), stack_param.reg);
    if(mwr_to_source_itr == instructions.rend()) {
      FIND_MAIN_FAIL("find_main: unable to find write to parameter register '%s'\n", stack_param.reg.name().c_str());
    }

    find_src_visitor stack_param_src{};
    auto stack_param_insn = mwr_to_source_itr->second;
    {
      auto operands = stack_param_insn.getExplicitOperands();

      // The first operand is the register pushed to the stack
      operands[1].getValue()->apply(&stack_param_src);
    }

    std::cerr << "Found mwr to stack source param: " << stack_param_insn.format() << "\n";

    /*  case 2: address is stored into a register
     *
     *    mov  eax, ADDRESS
     *    push eax
     *    call __libc_start_main
     */
    if(!stack_param_src.bf && !stack_param_src.deref) {
      if(!stack_param_src.imm) {
        FIND_MAIN_FAIL("find_main: failed to determine stack passing: '%s'\n", stack_param_insn.format().c_str());
      }
      return stack_param_src.imm->eval().convert<Dyninst::Address>();
    }

    /*
     *  case 3: address is computed
     *
     *        call   36cd
     *        add    ebx,0xa8898
     *    ...
     *        <ADDRESS STORE>
     *        call   __libc_start_main
     *        hlt
     *  36cd: mov    ebx,DWORD PTR [esp]
     *        ret
     *
     *    (a) stored directly on the stack
     *
     *      push DWORD PTR [ebx+0x20]
     *      call __libc_start_main
     *
     *    (b) stored in a register
     *
     *      lea  eax,[ebx-OFFSET]
     *      push eax
     *      call __libc_start_main
     */
    auto stack_param_src_reg = boost::make_shared<di::RegisterAST>(stack_param_src.reg);
    if(stack_param_insn.isWritten(stack_param_src_reg)) {
      if(!stack_param_src.bf) {
        // There's no base+offset calculation, assume it's just an immediate
        if(!stack_param_src.imm) {
          FIND_MAIN_FAIL("find_main: bad stack source; no immediate\n");
        }
        return stack_param_src.imm->eval().convert<Dyninst::Address>();
      }
    }

    return Dyninst::ADDR_NULL;
  }

}}}
