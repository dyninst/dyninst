#ifndef DYNINST_DYNINSTAPI_FINDMAINCOMMON_H
#define DYNINST_DYNINSTAPI_FINDMAINCOMMON_H

#include "CFG.h"
#include "debug.h"
#include "dyntypes.h"
#include "Instruction.h"
#include "registers/MachRegister.h"
#include "Register.h"

#include <algorithm>
#include <boost/shared_ptr.hpp>

#ifndef FIND_MAIN_FAIL
# define FIND_MAIN_FAIL(...)      \
  do {                            \
    startup_printf(__VA_ARGS__);  \
    return Dyninst::ADDR_NULL;    \
  } while(0);
#endif

namespace Dyninst { namespace DyninstAPI {

  namespace di = Dyninst::InstructionAPI;
  namespace pa = Dyninst::ParseAPI;

  struct instruction_t {
    Dyninst::Address address{};
    di::Instruction insn{};

    instruction_t(Dyninst::Address a, di::Instruction i) : address{a}, insn{i} {}
  };

  struct find_src_visitor final : di::Visitor {
    Dyninst::MachRegister reg{};
    di::BinaryFunction *bf{};
    di::Immediate *imm{};
    di::Dereference *deref{};

    void visit(di::RegisterAST *r) override {
      reg = r->getID();
    }

    void visit(di::Immediate *i) override {
      imm = i;
    }

    void visit(di::Dereference *d) override {
      deref = d;
    }

    void visit(di::BinaryFunction *b) override {
      bf = b;
    }

    void visit(di::MultiRegisterAST *) override {}
  };

  using rev_itr = pa::Block::Insns::reverse_iterator;

  static inline rev_itr find_mrw_to(rev_itr start, rev_itr end, Dyninst::MachRegister reg) {
    auto reg_ast = boost::make_shared<di::RegisterAST>(reg);

    // clang-format off
    return std::find_if(start, end,
      [&reg_ast](pa::Block::Insns::value_type const &val) {
        auto insn = val.second;
        return insn.isWritten(reg_ast);
      }
    );
    // clang-format on
  }

}}

#endif
