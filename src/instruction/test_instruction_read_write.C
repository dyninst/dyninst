/*
 * Copyright (c) 1996-2009 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
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

//#include <dyn_detail/boost/assign/list_of.hpp>
//#include <dyn_detail/boost/iterator/indirect_iterator.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/iterator/indirect_iterator.hpp>
#include <deque>
using namespace Dyninst;
using namespace InstructionAPI;
using namespace boost;
using namespace boost::assign;

using namespace std;

class test_instruction_read_write_Mutator : public InstructionMutator {
public:
   test_instruction_read_write_Mutator() { };
   virtual test_results_t executeTest();
};

extern "C" DLLEXPORT TestMutator* test_instruction_read_write_factory()
{
   return new test_instruction_read_write_Mutator();
}


test_results_t test_instruction_read_write_Mutator::executeTest()
{
  const unsigned char buffer[] = 
  {
    0x05, 0xef, 0xbe, 0xad, 0xde, // ADD eAX, 0xDEADBEEF
    0x50, // PUSH rAX
    0x74, 0x10, // JZ +0x10(8)
    0xE8, 0x20, 0x00, 0x00, 0x00, // CALL +0x20(32)
    0xF8, // CLC
    0x04, 0x30, // ADD AL, 0x30(8)
    0xc7, 0x45, 0xfc, 0x01, 0x00, 0x00, 0x00, // MOVL 0x01, -0x4(EBP)
    0x88, 0x55, 0xcc, // MOVB DL, -0x34(EBP)
    0xF2, 0x0F, 0x12, 0xC0, // MOVDDUP XMM0, XMM1
    0x66, 0x0F, 0x7C, 0xC9  // HADDPD XMM1, XMM1
  };
  unsigned int size = 34;
  unsigned int expectedInsns = 11;
  dyn_detail::boost::shared_ptr<InstructionDecoder> d =
          makeDecoder(Dyninst::InstructionAPI::x86, buffer, size);
#if defined(arch_x86_64_test)
  d->setMode(true);
#endif
  std::deque<Instruction::Ptr> decodedInsns;
  Instruction::Ptr i;
  do
  {
    i = d->decode();
    decodedInsns.push_back(i);
  }
  while(i && i->isValid());
  if(decodedInsns.size() != expectedInsns)
  {
    logerror("FAILED: Expected %d instructions, decoded %d\n", expectedInsns, decodedInsns.size());
    for(std::deque<Instruction::Ptr>::iterator curInsn = decodedInsns.begin();
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
  RegisterAST::Ptr eax(new RegisterAST(r_EAX));
  RegisterAST::Ptr adjust(new RegisterAST(r_AF));
  RegisterAST::Ptr zero(new RegisterAST(r_ZF));
  RegisterAST::Ptr overflow(new RegisterAST(r_OF));
  RegisterAST::Ptr parity(new RegisterAST(r_PF));
  RegisterAST::Ptr sign(new RegisterAST(r_SF));
  RegisterAST::Ptr carry(new RegisterAST(r_CF));
  registerSet expectedRead, expectedWritten;
  expectedRead.insert(expectedRead.begin(), eax);
  expectedWritten = list_of(eax)(adjust)(zero)(overflow)(parity)(sign)(carry);
  
  test_results_t retVal = PASSED;
  
  retVal = failure_accumulator(retVal, verify_read_write_sets(decodedInsns.front(), expectedRead, expectedWritten));
  decodedInsns.pop_front();
  
  RegisterAST::Ptr rax(new RegisterAST(r_EAX));
  RegisterAST::Ptr esp(new RegisterAST(r_ESP));
  expectedRead.clear();
  expectedWritten.clear();
  expectedRead = list_of(esp)(rax);
  expectedWritten = list_of(esp);
  retVal = failure_accumulator(retVal, verify_read_write_sets(decodedInsns.front(), expectedRead, expectedWritten));
  decodedInsns.pop_front();
  
  expectedRead.clear();
  expectedWritten.clear();
  RegisterAST::Ptr ip(new RegisterAST(r_EIP));
  // Jccs are all documented as "may read zero, sign, carry, parity, overflow", so a JZ comes back as reading all
  // of these flags
  expectedRead = list_of(zero)(sign)(carry)(parity)(overflow)(ip);
  expectedWritten = list_of(ip);
  retVal = failure_accumulator(retVal, verify_read_write_sets(decodedInsns.front(), 
  							      expectedRead, expectedWritten));
  decodedInsns.pop_front();
  
  expectedRead.clear();
  expectedWritten.clear();
  expectedRead = list_of(esp)(ip);
  expectedWritten = list_of(esp)(ip);
  retVal = failure_accumulator(retVal, verify_read_write_sets(decodedInsns.front(), 
							      expectedRead, expectedWritten));
  Instruction::Ptr callInsn = decodedInsns.front();
  decodedInsns.pop_front();

  expectedRead.clear();
  expectedWritten.clear();
  expectedWritten = list_of(carry);
  retVal = failure_accumulator(retVal, verify_read_write_sets(decodedInsns.front(), 
							      expectedRead, expectedWritten));
  decodedInsns.pop_front();

  expectedRead.clear();
  expectedWritten.clear();
  RegisterAST::Ptr al(new RegisterAST(r_AL));
  expectedRead = list_of(al);
  expectedWritten = list_of(al)(zero)(carry)(sign)(overflow)(parity)(adjust);
  retVal = failure_accumulator(retVal, verify_read_write_sets(decodedInsns.front(), 
							      expectedRead, expectedWritten));
  decodedInsns.pop_front();

#if defined(arch_x86_64_test)
  RegisterAST::Ptr bp(new RegisterAST(r_RBP));
#else
  RegisterAST::Ptr bp(new RegisterAST(r_EBP));
#endif
  expectedRead.clear();
  expectedWritten.clear();
  expectedRead = list_of(bp);
  retVal = failure_accumulator(retVal, verify_read_write_sets(decodedInsns.front(), 
							      expectedRead, expectedWritten));
  decodedInsns.pop_front();
  
  
  RegisterAST::Ptr dl(new RegisterAST(r_DL));
  expectedRead.clear();
  expectedWritten.clear();
  expectedRead = list_of(bp)(dl);
  retVal = failure_accumulator(retVal, verify_read_write_sets(decodedInsns.front(), 
							      expectedRead, expectedWritten));
  decodedInsns.pop_front();
  

  RegisterAST::Ptr xmm0(new RegisterAST(r_XMM0));
  RegisterAST::Ptr xmm1(new RegisterAST(r_XMM1));
  expectedRead.clear();
  expectedWritten.clear();
  expectedRead = list_of(xmm0);
  expectedWritten = list_of(xmm0);
  retVal = failure_accumulator(retVal, verify_read_write_sets(decodedInsns.front(), 
							      expectedRead, expectedWritten));
  if(decodedInsns.front()->size() != 4) {
    logerror("FAILURE: movddup expected size 4, decoded to %s, had size %d\n", decodedInsns.front()->format().c_str(), decodedInsns.front()->size());
    retVal = FAILED;
  }
  decodedInsns.pop_front();

  expectedRead.clear();
  expectedWritten.clear();
  expectedRead = list_of(xmm1);
  expectedWritten = list_of(xmm1);
  retVal = failure_accumulator(retVal, verify_read_write_sets(decodedInsns.front(), expectedRead, expectedWritten));
  if(decodedInsns.front()->size() != 4) {
    logerror("FAILURE: haddpd expected size 4, decoded to %s, had size %d\n", decodedInsns.front()->format().c_str(), decodedInsns.front()->size());
    retVal = FAILED;
  }  
  decodedInsns.pop_front();

#if defined(arch_x86_64_test)
  const unsigned char amd64_specific[] = 
  {
    0x44, 0x89, 0x45, 0xc4
  };
  unsigned int amd64_size = 4;
  unsigned int amd64_num_valid_insns = 1;
  deque<Instruction::Ptr> amd64Insns;
  
  dyn_detail::boost::shared_ptr<InstructionDecoder> amd64_decoder =
          makeDecoder(Dyninst::InstructionAPI::x86, amd64_specific, amd64_size);
  amd64_decoder->setMode(true);
  
  Instruction::Ptr tmp;
  do
  {
    tmp = amd64_decoder->decode();
    amd64Insns.push_back(tmp);
  } while(tmp && tmp->isValid());
  amd64Insns.pop_back();
  if(amd64Insns.size() != amd64_num_valid_insns) 
  {
    logerror("FAILED: expected %d instructions in AMD64-specific part, got %d\n", amd64_num_valid_insns,
	     amd64Insns.size());
    return FAILED;
  }
  RegisterAST::Ptr r8(new RegisterAST(r_R8));
  
  expectedRead = list_of(bp)(r8);
  expectedWritten.clear();
  
  retVal = failure_accumulator(retVal, verify_read_write_sets(amd64Insns.front(), expectedRead, expectedWritten));
  amd64Insns.pop_front();
  
#endif

  Expression::Ptr cft = callInsn->getControlFlowTarget();
  if(!cft) {
    logerror("FAILED: call had no control flow target\n");
    return FAILED;
  }
  RegisterAST* the_ip = new RegisterAST(r_EIP);
  
  if(!cft->bind(the_ip, Result(u32, 0))) {
    logerror("FAILED: bind found no IP in call Jz CFT\n");
    return FAILED;
  }
  Result theTarget = cft->eval();
  if(!theTarget.defined) {
    logerror("FAILED: bind of IP on a Jz operand did not resolve all dependencies\n");
    return FAILED;
  }
  if(theTarget.type != u32) {
    logerror("FAILED: CFT was not address type\n");
    logerror("   %s\n", theTarget.format().c_str());
    return FAILED;
  }
  // Call target should be to IP + displacement + size
  if(theTarget.val.u32val != 0x25) {
    logerror("FAILED: expected call to %x, got call to %x\n", 0x20, theTarget.val.u32val);
    logerror("   %s\n", theTarget.format().c_str());
    return FAILED;
  }
  logerror("PASSED call CFT subtest\n");
  delete the_ip;

  return retVal;
}

