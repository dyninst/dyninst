#include "dyn_regs.h"
#include "InstructionAST.h"
#include "registers/MachRegister.h"
#include "Result.h"

#include <algorithm>
#include <boost/make_shared.hpp>
#include <cstdlib>
#include <iostream>
#include <vector>

/*
 *  InstructionAPI::getUsedRegisters
 *
 *  This uses a non-sensical collection of x86, ppc, and aarch64
 *  registers because the algorithm shouldn't depend on the
 *  architecture being used.
 */

namespace di = Dyninst::InstructionAPI;

di::Expression::Ptr make_reg(Dyninst::MachRegister reg) {
  return boost::make_shared<di::RegisterAST>(reg, 0, 8, 1);
}

di::Expression::Ptr make_add(di::Expression::Ptr lhs, di::Expression::Ptr rhs) {
  di::BinaryFunction::funcT::Ptr adder(new di::BinaryFunction::addResult());
  return boost::make_shared<di::BinaryFunction>(lhs, rhs, di::u32, adder);
}

int main() {
  auto r0 = Dyninst::ppc64::r0;
  auto rax = Dyninst::x86_64::rax;
  auto x29 = Dyninst::aarch64::x29;

  // r0 + rax + x29
  auto expr = make_add(make_reg(r0), make_add(make_reg(rax), make_reg(x29)));

  auto used_regs = di::getUsedRegisters(expr);

  auto _contains = [&used_regs](Dyninst::MachRegister r) {
    auto itr = std::find_if(used_regs.begin(), used_regs.end(), [r](di::RegisterAST::Ptr reg) {
      return reg->getID() == r;
    });
    return itr != used_regs.end();
  };

  bool failed = false;
  for(auto &&r : {r0, rax, x29}) {
    if(!_contains(r)) {
      std::cerr << "Didn't find register " << r.name() << "\n";
      failed = true;
    }
  }

  return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}
