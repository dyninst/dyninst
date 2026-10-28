#include "AbslocInterface.h"
#include "CFG.h"
#include "CodeObject.h"
#include "Instruction.h"

#include <cstdlib>
#include <iostream>
#include <vector>

/*
 * Parse riscv64 binaries and convert every jalr and conditional branch to
 * assignments. The decoder gives jalr two implicit operands (pc and the branch
 * target), and both used to be counted against the explicit ones.
 *
 *   jalr rd, rs, imm     pc <- rs, rd <- pc
 *   bxx  rs1, rs2, imm   pc <- rs1, rs2
 */

namespace {
  namespace di = Dyninst::InstructionAPI;
  namespace pa = Dyninst::ParseAPI;

  bool is_branch(entryID id) {
    switch(id) {
      case riscv64_op_beq:
      case riscv64_op_bne:
      case riscv64_op_blt:
      case riscv64_op_bge:
      case riscv64_op_bltu:
      case riscv64_op_bgeu:
        return true;
      default:
        return false;
    }
  }

  Dyninst::AbsRegion reg(di::Operand const &o) {
    auto r = boost::dynamic_pointer_cast<di::RegisterAST>(o.getValue());
    return r ? Dyninst::AbsRegion(r->getID()) : Dyninst::AbsRegion();
  }

  bool has_input(Dyninst::Assignment::Ptr const &a, Dyninst::AbsRegion const &r) {
    for(auto const &i : a->inputs()) {
      if(i == r) {
        return true;
      }
    }
    return false;
  }

  bool check(char *file) {
    auto *sts = new pa::SymtabCodeSource(file);
    auto *co = new pa::CodeObject(sts);
    co->parse();

    Dyninst::AbsRegion const pc(Dyninst::Absloc::makePC(sts->getArch()));
    Dyninst::AssignmentConverter ac(true, false);
    int jalrs = 0, branches = 0;
    bool ok = true;
    for(auto *f : co->funcs()) {
      for(auto *b : f->blocks()) {
        pa::Block::Insns insns;
        b->getInsns(insns);
        for(auto const &e : insns) {
          auto const id = e.second.getOperation().getID();
          bool const jalr = id == riscv64_op_jalr;
          if(!jalr && !is_branch(id)) {
            continue;
          }
          auto const ops = e.second.getExplicitOperands();
          std::vector<Dyninst::Assignment::Ptr> as;
          ac.convert(e.second, e.first, f, b, as);

          bool good;
          if(jalr) {
            jalrs++;
            // pc <- rs, rd <- pc
            good = as.size() == 2 && as[0]->out() == pc && has_input(as[0], reg(ops[1])) &&
                   as[1]->out() == reg(ops[0]) && has_input(as[1], pc);
          } else {
            branches++;
            good = as.size() == 1 && as[0]->out() == pc && has_input(as[0], reg(ops[0])) &&
                   has_input(as[0], reg(ops[1]));
          }
          if(!good) {
            std::cerr << file << ": 0x" << std::hex << e.first << std::dec << " '"
                      << e.second.format() << "': wrong assignments\n";
            ok = false;
          }
        }
      }
    }
    std::cerr << file << ": " << jalrs << " jalr, " << branches << " branches\n";
    if(jalrs == 0 || branches == 0) {
      std::cerr << file << ": no jalr or no conditional branch parsed\n";
      ok = false;
    }
    return ok;
  }
}

int main(int argc, char **argv) {
  if(argc == 1) {
    std::cerr << "usage: " << argv[0] << " <riscv64 binary>...\n";
    return EXIT_FAILURE;
  }

  bool ok = true;
  for(int i = 1; i < argc; i++) {
    ok = check(argv[i]) && ok;
  }
  return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
