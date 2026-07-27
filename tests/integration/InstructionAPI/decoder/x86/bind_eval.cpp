#include "Architecture.h"
#include "Expression.h"
#include "Instruction.h"
#include "InstructionDecoder.h"
#include "Register.h"
#include "registers/x86_64_regs.h"
#include "registers/x86_regs.h"
#include "Result.h"

#include <array>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <vector>

/*
 *  x86/x86_64 expression bind/eval tests
 *
 *  Decodes 'call [8*EAX + ECX + 0xDEADBEEF]' and verifies that the
 *  control-flow target expression stays undefined until all of its
 *  terms are bound (the target is a memory dereference, so it can
 *  never be fully evaluated), while the address computation inside the
 *  dereference evaluates to the expected value once EAX and ECX are
 *  bound.
 */

using namespace Dyninst;
using namespace Dyninst::InstructionAPI;

namespace {

constexpr auto num_tests = 1;

std::array<const uint8_t, 7> buffer = {{
    0xFF, 0x94, 0xC1, 0xEF, 0xBE, 0xAD, 0xDE // call [8*EAX + ECX + 0xDEADBEEF]
}};

bool verifyCFT(Expression::Ptr cft, bool expectedDefined, unsigned long expectedValue, Result_Type expectedType) {
  Result cftResult = cft->eval();
  if(cftResult.defined != expectedDefined) {
    std::cerr << std::boolalpha << "CFT '" << cft->format() << "': expected result defined " << expectedDefined
              << ", actual " << cftResult.defined << '\n';
    return false;
  }
  if(expectedDefined) {
    if(cftResult.type != expectedType) {
      std::cerr << "CFT '" << cft->format() << "': expected result type " << expectedType << ", actual "
                << cftResult.type << '\n';
      return false;
    }
    if(cftResult.convert<unsigned long long int>() != expectedValue) {
      std::cerr << std::hex << "CFT '" << cft->format() << "': expected result value 0x" << expectedValue
                << ", actual 0x" << cftResult.convert<unsigned long>() << std::dec << '\n';
      return false;
    }
  }
  return true;
}

bool run(Dyninst::Architecture arch) {
  InstructionDecoder d(buffer.data(), buffer.size(), arch);

  std::vector<Instruction> decodedInsns;
  decodedInsns.reserve(num_tests);
  for(int idx = 0; idx < num_tests; idx++) {
    Instruction insn = d.decode();
    if(!insn.isValid()) {
      std::cerr << "Failed to decode test " << (idx + 1) << '\n';
      return false;
    }
    decodedInsns.push_back(insn);
  }

  const auto is_64 = (arch == Dyninst::Arch_x86_64);
  RegisterAST ax(is_64 ? x86_64::rax : x86::eax);
  RegisterAST cx(is_64 ? x86_64::rcx : x86::ecx);

  for(auto &&insn : decodedInsns) {
    Expression::Ptr theCFT = insn.getControlFlowTarget();
    if(!theCFT) {
      std::cerr << "No CFT found for '" << insn.format() << "'\n";
      return false;
    }
    if(!verifyCFT(theCFT, false, 0x1000, u32)) {
      return false;
    }

    if(!theCFT->bind(&ax, Result(u32, 3))) {
      std::cerr << "Bind of EAX failed (insn '" << insn.format() << "')\n";
      return false;
    }
    if(!verifyCFT(theCFT, false, 0x1000, u32)) {
      return false;
    }
    if(!theCFT->bind(&cx, Result(u32, 5))) {
      std::cerr << "Bind of ECX failed\n";
      return false;
    }
    if(!verifyCFT(theCFT, false, 0x1000, u32)) {
      return false;
    }
    auto subExpressions = theCFT->getSubexpressions();
    if(subExpressions.size() != 1) {
      std::cerr << "Expected dereference with one child, got " << subExpressions.size() << " children\n";
      return false;
    }
    Expression::Ptr memRef = subExpressions[0];
    if(!memRef) {
      std::cerr << "memRef was not an expression\n";
      return false;
    }
    using res_t = typename Result_type2type<u32>::type;
    auto expected_value = static_cast<res_t>(0xDEADBEEF + (0x03 * 0x08 + 0x05));
    if(!verifyCFT(memRef, true, expected_value, u32)) {
      return false;
    }
  }
  return true;
}

} // namespace

int main() {
  bool ok = true;
  {
    std::clog << "**** Running x86 ********\n";
    if(!run(Dyninst::Arch_x86)) {
      ok = false;
    }
  }
  {
    std::clog << "**** Running x86_64 ********\n";
    if(!run(Dyninst::Arch_x86_64)) {
      ok = false;
    }
  }
  return !ok ? EXIT_FAILURE : EXIT_SUCCESS;
}
