/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "instruction_comp.h"
#include "test_lib.h"

#include "Instruction.h"
#include "InstructionDecoder.h"
#include "Expression.h"
#include "Result.h"

#include <boost/assign/list_of.hpp>
#include <boost/iterator/indirect_iterator.hpp>

using namespace Dyninst;
using namespace InstructionAPI;
using namespace boost::assign;
using namespace boost;

using namespace std;

class test_instruction_bind_eval_Mutator : public InstructionMutator {
public:
   test_instruction_bind_eval_Mutator() { };
   virtual test_results_t executeTest();
};

extern "C" DLLEXPORT TestMutator* test_instruction_bind_eval_factory()
{
   return new test_instruction_bind_eval_Mutator();
}





test_results_t test_instruction_bind_eval_Mutator::executeTest()
{
  const unsigned char buffer[] = 
  {
    0xFF, 0x94, 0xC1, 0xEF, 0xBE, 0xAD, 0xDE // call [8*EAX + ECX + 0xDEADBEEF]
  };
  unsigned int size = 7;
  unsigned int expectedInsns = 2;
#if defined(arch_x86_64_test)
    Architecture curArch = Arch_x86_64;
    using namespace Dyninst::x86_64;
#elif defined(arch_x86_test)
    Architecture curArch = Arch_x86;
    using namespace Dyninst::x86;
#else
    Architecture curArch = Arch_none;
#endif
    
  
    InstructionDecoder d(buffer, size, curArch);
    std::vector<Instruction::Ptr> decodedInsns;
    Instruction::Ptr i;
    do
    {
      i = d.decode();
      decodedInsns.push_back(i);
    }
    while(i && i->isValid());
    if(decodedInsns.size() != expectedInsns)
    {
      logerror("FAILED: Expected %d instructions, decoded %d\n", expectedInsns, decodedInsns.size());
      for(std::vector<Instruction::Ptr>::iterator curInsn = decodedInsns.begin();
	  curInsn != decodedInsns.end();
	  ++curInsn)
      {
      logerror("\t%s\n", (*curInsn)->format().c_str());
      }
      
      return FAILED;
    }
    if(decodedInsns.back() && decodedInsns.back()->isValid())
    {
      logerror("FAILED: Expected instructions to end with an invalid instruction, but they didn't");
      return FAILED;
    }

  Expression::Ptr theCFT = decodedInsns[0]->getControlFlowTarget();
  if(!theCFT) {
    logerror("FAILED: instruction %s decoded from call [8*EAX + ECX + 0xDEADBEEF], no CFT found\n", decodedInsns[0]->format().c_str());
    return FAILED;
  }
  if(verifyCFT(theCFT, false, 0x1000, u32) == FAILED)
  {
    return FAILED;
  }
  
#if defined(arch_x86_64_test)
  RegisterAST* r_eax = new RegisterAST(x86_64::eax);
  RegisterAST* r_ecx = new RegisterAST(x86_64::ecx);
#elif defined(arch_x86_test)
  RegisterAST* r_eax = new RegisterAST(x86::eax);
  RegisterAST* r_ecx = new RegisterAST(x86::ecx);
#else
#error "Test_instruction_bind_eval should be x86 only!"
#endif
    
  Result three(u32, 3);
  Result five(u32, 5);
  
  if(!theCFT->bind(r_eax, three)) {
      logerror("FAILED: bind of EAX failed (insn %s)\n", decodedInsns[0]->format().c_str());
    return FAILED;
  }
  if(verifyCFT(theCFT, false, 0x1000, u32) == FAILED)
  {
    return FAILED;
  }
  if(!theCFT->bind(r_ecx, five)) {
    logerror("FAILED: bind of ECX failed\n");
    return FAILED;
  }
  if(verifyCFT(theCFT, false, 0x1000, u32) == FAILED)
  {
    return FAILED;
  }
  vector<Expression::Ptr> tmp;
  theCFT->getChildren(tmp);
  if(tmp.size() != 1)
  {
    logerror("FAILED: expected dereference with one child, got %d children\n", tmp.size());
    return FAILED;
  }
  Expression::Ptr memRef = tmp[0];
  if(!memRef) {
    logerror("FAILED: memRef was not an expression\n");
    return FAILED;
  }
  if(verifyCFT(memRef, true, 0xDEADBEEF + (0x03 * 0x08 + 0x05), u32) == FAILED) {
    return FAILED;
  }

  return PASSED;
}

