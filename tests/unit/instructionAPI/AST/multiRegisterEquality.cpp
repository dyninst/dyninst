#include "dyn_regs.h"
#include "MultiRegister.h"
#include "Operand.h"
#include "Register.h"
#include "registers/MachRegister.h"

#include <boost/make_shared.hpp>
#include <cstdlib>
#include <iostream>
#include <vector>

/*
 *  MultiRegisterAST equality, ordering, and use
 *
 *  Two MultiRegisterASTs that name the same registers must compare
 *  equal even when they were constructed separately (and so hold
 *  distinct RegisterAST objects).
 */

namespace di = Dyninst::InstructionAPI;

static bool failed = false;

static void check(bool cond, char const* what) {
  if(!cond) {
    std::cerr << "FAILED: " << what << "\n";
    failed = true;
  }
}

int main() {
  auto s0 = Dyninst::amdgpu_gfx908::s0;
  auto s1 = Dyninst::amdgpu_gfx908::s1;
  auto s2 = Dyninst::amdgpu_gfx908::s2;
  auto s5 = Dyninst::amdgpu_gfx908::s5;

  // s[0:1], built twice
  auto a = boost::make_shared<di::MultiRegisterAST>(s0, 2);
  auto b = boost::make_shared<di::MultiRegisterAST>(s0, 2);

  check(*a == *b, "s[0:1] == s[0:1]");
  check(a->isUsed(b), "s[0:1] uses s[0:1]");
  check(!(*a < *b) && !(*b < *a), "s[0:1] and s[0:1] are equivalent under operator<");

  // Constituent registers
  check(a->isUsed(boost::make_shared<di::RegisterAST>(s0)), "s[0:1] uses s0");
  check(a->isUsed(boost::make_shared<di::RegisterAST>(s1)), "s[0:1] uses s1");
  check(!a->isUsed(boost::make_shared<di::RegisterAST>(s5)), "s[0:1] does not use s5");

  // Overlapping, but not equal
  auto f = boost::make_shared<di::MultiRegisterAST>(s1, 2);
  check(!(*a == *f), "s[0:1] != s[1:2]");
  check(a->isUsed(f), "s[0:1] uses s[1:2]");

  // Different base register
  auto c = boost::make_shared<di::MultiRegisterAST>(s2, 2);
  check(!(*a == *c), "s[0:1] != s[2:3]");
  check(!a->isUsed(c), "s[0:1] does not use s[2:3]");
  check(*a < *c && !(*c < *a), "s[0:1] < s[2:3]");

  // Different register count
  auto d = boost::make_shared<di::MultiRegisterAST>(s0, 3);
  check(!(*a == *d), "s[0:1] != s[0:2]");
  check(*a < *d && !(*d < *a), "s[0:1] < s[0:2]");

  // The same registers given as a vector
  std::vector<di::RegisterAST::Ptr> regs{
      boost::make_shared<di::RegisterAST>(s0, 0, s0.size() * 8, 2),
      boost::make_shared<di::RegisterAST>(s1, 0, s1.size() * 8, 2)};
  auto e = boost::make_shared<di::MultiRegisterAST>(regs);
  check(*a == *e, "s[0:1] == [s0, s1]");

  // Operand-level queries with a separately built candidate
  di::Operand op(a, true, true);
  check(op.isRead(b), "Operand(s[0:1]).isRead(s[0:1])");
  check(op.isWritten(b), "Operand(s[0:1]).isWritten(s[0:1])");
  check(op.isRead(boost::make_shared<di::RegisterAST>(s1)), "Operand(s[0:1]).isRead(s1)");
  check(!op.isWritten(c), "!Operand(s[0:1]).isWritten(s[2:3])");

  return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}
