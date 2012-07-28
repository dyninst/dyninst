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

#include "../h/InstructionDecoder.h"
#include <iostream>
#include <boost/assign.hpp>
#include "../h/Register.h"

using namespace Dyninst::InstructionAPI;
using namespace std;
using namespace boost::assign;

template< typename strT >
strT& operator<<(strT& s, std::set<RegisterAST::Ptr> regs)
{
  for(std::set<RegisterAST::Ptr>::const_iterator i = regs.begin();
      i != regs.end();
      ++i)
  {
    s << (*i)->format() << " ";
  }
  s << endl;
  return s;
}


class testSuite
{
public:
  testSuite() : numTests(0), numFailures(0)
  {
  }
  
  bool testDecode(InstructionDecoder& testMe, const unsigned char* instruction, int size, const char* name)
  {
    Instruction tempInstruction = testMe.decode(instruction, size);
    return testAssertEqual(name, tempInstruction.format());
  }

  bool testRegisters(InstructionDecoder& testMe, const unsigned char* instruction, int size, 
		     set<RegisterAST::Ptr> expectedRead, set<RegisterAST::Ptr> expectedWritten)
  {
    std::set<RegisterAST::Ptr> actualRead, actualWritten;
    Instruction testInsn = testMe.decode(instruction, size);
    testInsn.getReadSet(actualRead);
    testInsn.getWriteSet(actualWritten);
    return testAssertEqual(expectedRead, actualRead) && testAssertEqual(expectedWritten, actualWritten);
  }
  template< typename T1, typename T2>
  bool testAssertEqual(boost::shared_ptr<T1> lhs, boost::shared_ptr<T2> rhs)
  {
    numTests++;
    if(*lhs == *rhs)
    {
      cout << ".";
      return true;
    }
    else
    {
      cout << "F";
      failureLog << "FAIL: expected " << *lhs << ", actual " << *rhs << endl;
      numFailures++;
      return false;
    }
  }
  

  template< typename T1, typename T2 >
  bool testAssertEqual(T1 lhs, T2 rhs)
  {
    numTests++;
    if(lhs == rhs)
    {
      cout << ".";
      return true;
    }
    else
    {
      cout << "F";
      failureLog << "FAIL: expected " << lhs << ", actual " << rhs << endl;
      numFailures++;
      return false;
    }
  }
  void printResultsSummary() const
  {
    cout << endl;
    cout << "RESULTS: " << numTests << " tests, " << numFailures << " failures" << endl;
    if(numFailures)
    {
      cout << "FAILURE DETAILS:" << endl;
      cout << failureLog.str() << endl;
    }
    else
    {
      cout << "PASSED!" << endl;
    }
  }
  
private:
  std::stringstream failureLog;
  int numTests;
  int numFailures;
  
};



int main(int argc, char** argv)
{
  InstructionDecoder testMe;
  const unsigned char MovRegToReg[2] = { 0x89, 0xe5 }; // mov %esp, %ebp
  const unsigned char MovRegToRegIndirect[2] = { 0x89, 0x01 }; // mov %eax, *%ecx
  const unsigned char MovRegToRegIndirectOffset[3] = { 0x89, 0x45, 0xd4 }; // mov %eax, 0xffffffd4(%ebp)
  // mov 0x390(%ecx), %edx
  const unsigned char MovRegToRegIndirectLongOffset[6] = { 0x8b, 0x91, 0x90, 0x03, 0x00, 0x00 };
  const unsigned char IndirectCall[2] = { 0xff, 0xd0 }; // call *%eax
  const unsigned char ConditionalJump[2] = { 0x74, 0x02 }; // je +0x02
  const unsigned char MovRegToReg2[2] = {0x89, 0xe1}; // move %esp, %ecx
  const unsigned char UnconditionalJump[2] = {0xeb, 0x14}; // jmp +0x14
  const unsigned char SIBMove[3] = {0x8b, 0x14, 0xb0}; // mov (%eax, %esi, 4), %edx
  const unsigned char AddRegImmediate[3] = {0x83, 0xc0, 0x01}; // add $0x1, %eax
  const unsigned char SubRegImmediate[3] = {0x83, 0xe8, 0x01}; // add $0x1, %eax
  const unsigned char PushTest[1] = {0x56}; // push %ESI
  const unsigned char PopTest[1] = {0x5E}; // pop %ESI
  const unsigned char XCHGTest[1] = {0x91}; // xchg %ecx, %eax
  const unsigned char SIBDisplacementTest[7] = 
  {
    0x89, 0x84, 0x01, 0xef, 0xbe, 0xad, 0xde // mov eax, (%eax, %ecx, 1)+$0xdeadbeef
  };
  const unsigned char EightBitRegTest[2] = {0x2c, 0x01}; // sub $0x1, %al
  const unsigned char EightBitRegTest2[2] = {0x88, 0xc1}; // mov %al, %cl
  const unsigned char ReturnTest[1] = {0xc3}; // ret
  const unsigned char AddCarryTest[2] = {0x11, 0x01}; // adc %eax, *%ecx
  const unsigned char WordJumpTest[5] = 
  {
    0xe9, 0x0e, 0x00, 0x00, 0xee
  }; // jmp +0xee00000e
  const unsigned char LESTest[2] = 
  {
    0xc4, 0x02 // LES (%edx), %eax
  };
  const unsigned char OModeTest[5] =
  {
    0xa0, 0x03, 0x00, 0x00, 0x24 //  mov 0x24000003,%al
  };

  testSuite IA32Tests;
  
  IA32Tests.testDecode(testMe, EightBitRegTest2, 2, "mov CL, AL");
  IA32Tests.testDecode(testMe, MovRegToReg, 2, "mov %esp, %ebp");
  IA32Tests.testDecode(testMe, MovRegToRegIndirect, 2, "mov %eax, *%ecx");
  IA32Tests.testDecode(testMe, MovRegToRegIndirectOffset, 3, "mov %eax, 0xffffffd4(%ebp)");
  IA32Tests.testDecode(testMe, MovRegToReg2, 2, "mov %esp, %ecx");
  IA32Tests.testDecode(testMe, IndirectCall, 2, "call *%eax");
  IA32Tests.testDecode(testMe, ConditionalJump, 2, "je +0x02");
  IA32Tests.testDecode(testMe, UnconditionalJump, 2, "jmp +0x14");
  IA32Tests.testDecode(testMe, MovRegToRegIndirectLongOffset, 6, "mov 0x390(%ecx), %edx");
  IA32Tests.testDecode(testMe, SIBMove, 3, "mov (%eax, %esi, 4), %edx");
  IA32Tests.testDecode(testMe, AddRegImmediate, 3, "add $0x1, %eax");
  IA32Tests.testDecode(testMe, SubRegImmediate, 3, "sub $0x1, %eax");
  IA32Tests.testDecode(testMe, PushTest, 1, "push %esi");
  IA32Tests.testDecode(testMe, PopTest, 1, "pop %esi");
  IA32Tests.testDecode(testMe, XCHGTest, 1, "xchg %ecx, %eax");  
  IA32Tests.testDecode(testMe, SIBDisplacementTest, 7, "mov eax, (%eax, %ecx, 1)+$0xdeadbeef"); 
  IA32Tests.testDecode(testMe, EightBitRegTest, 2, "sub $0x1, %al");
  IA32Tests.testDecode(testMe, ReturnTest, 1, "ret");
  IA32Tests.testDecode(testMe, AddCarryTest, 2, "adc %eax, *%ecx");
  IA32Tests.testDecode(testMe, WordJumpTest, 5, "jmp +0xee00000e");
  IA32Tests.testDecode(testMe, LESTest, 2, "les (%edx), %eax");
  IA32Tests.testDecode(testMe, OModeTest, 5, "mov 0x24000003,%al");

  std::set<RegisterAST::Ptr> expectedRead, expectedWritten;
  expectedRead.insert(RegisterAST::Ptr(new RegisterAST(Dyninst::InstructionAPI::r_AL)));
  expectedWritten.insert(RegisterAST::Ptr(new RegisterAST(Dyninst::InstructionAPI::r_CL)));
  IA32Tests.testRegisters(testMe, EightBitRegTest2, 2, expectedRead, expectedWritten);
  expectedRead.clear();
  expectedWritten.clear();
  expectedRead = list_of(RegisterAST::Ptr(new RegisterAST(Dyninst::InstructionAPI::r_eAX)))(RegisterAST::Ptr(new RegisterAST((Dyninst::InstructionAPI::r_eCX))));
  IA32Tests.testRegisters(testMe, AddCarryTest, 2, expectedRead, expectedWritten);
  
  
  IA32Tests.printResultsSummary();
  
  return 0;
}
