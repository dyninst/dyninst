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
#include "Register.h"
#include "dyn_regs.h"
#include <boost/assign/list_of.hpp>
#include <boost/iterator/indirect_iterator.hpp>
#include <deque>
using namespace Dyninst;
using namespace InstructionAPI;
using namespace boost;
using namespace boost::assign;

using namespace std;

class aarch64_decode_Mutator : public InstructionMutator {
public:
    aarch64_decode_Mutator() { };
   virtual test_results_t executeTest();
};

extern "C" DLLEXPORT TestMutator* aarch64_decode_factory()
{
   return new aarch64_decode_Mutator();
}

test_results_t aarch64_decode_Mutator::executeTest()
{
  const unsigned char buffer[] =
  {
    0x91, 0x00, 0x1c, 0x21,     // add x1, x1, #0x7;  0x91001c21
    0xd1, 0x00, 0x04, 0x41,     // sub x1, x2, #0x1;
    0x00, 0x00, 0x00, 0x00,     // invalid op
  };

  unsigned int size = sizeof(buffer);
  unsigned int expectedInsns = size/4;

  ++expectedInsns;
  InstructionDecoder d(buffer, size, Dyninst::Arch_aarch64);

  std::deque<Instruction::Ptr> decodedInsns;
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
    for(std::deque<Instruction::Ptr>::iterator curInsn = decodedInsns.begin();
	curInsn != decodedInsns.end();
	++curInsn)
    {
        if(*curInsn) logerror("\t%s\n", (*curInsn)->format().c_str());
    }

    return FAILED;
  }

  if(decodedInsns.back() && decodedInsns.back()->isValid())
  {
    logerror("FAILED: Expected instructions to end with an invalid instruction, but they didn't");
    return FAILED;
  }

  std::deque<registerSet> expectedRead, expectedWritten;
  registerSet tmpRead, tmpWritten;

  RegisterAST::Ptr x0 (new RegisterAST(aarch64::x0));
  RegisterAST::Ptr x1 (new RegisterAST(aarch64::x1));
  RegisterAST::Ptr x2 (new RegisterAST(aarch64::x2));
  RegisterAST::Ptr x3 (new RegisterAST(aarch64::x3));
  RegisterAST::Ptr x4 (new RegisterAST(aarch64::x4));
  RegisterAST::Ptr x5 (new RegisterAST(aarch64::x5));
  RegisterAST::Ptr x6 (new RegisterAST(aarch64::x6));
  RegisterAST::Ptr x7 (new RegisterAST(aarch64::x7));
  RegisterAST::Ptr x8 (new RegisterAST(aarch64::x8));
  RegisterAST::Ptr x9 (new RegisterAST(aarch64::x9));
  RegisterAST::Ptr x10(new RegisterAST(aarch64::x10));
  RegisterAST::Ptr x11(new RegisterAST(aarch64::x11));
  RegisterAST::Ptr x12(new RegisterAST(aarch64::x12));
  RegisterAST::Ptr x13(new RegisterAST(aarch64::x13));
  RegisterAST::Ptr x14(new RegisterAST(aarch64::x14));
  RegisterAST::Ptr x15(new RegisterAST(aarch64::x15));
  RegisterAST::Ptr x16(new RegisterAST(aarch64::x16));
  RegisterAST::Ptr x17(new RegisterAST(aarch64::x17));
  RegisterAST::Ptr x18(new RegisterAST(aarch64::x18));
  RegisterAST::Ptr x19(new RegisterAST(aarch64::x19));
  RegisterAST::Ptr x20(new RegisterAST(aarch64::x20));
  RegisterAST::Ptr x21(new RegisterAST(aarch64::x21));
  RegisterAST::Ptr x22(new RegisterAST(aarch64::x22));
  RegisterAST::Ptr x23(new RegisterAST(aarch64::x23));
  RegisterAST::Ptr x24(new RegisterAST(aarch64::x24));
  RegisterAST::Ptr x25(new RegisterAST(aarch64::x25));
  RegisterAST::Ptr x26(new RegisterAST(aarch64::x26));
  RegisterAST::Ptr x27(new RegisterAST(aarch64::x27));
  RegisterAST::Ptr x28(new RegisterAST(aarch64::x28));
  RegisterAST::Ptr x29(new RegisterAST(aarch64::x29));
  RegisterAST::Ptr x30(new RegisterAST(aarch64::x30));
  RegisterAST::Ptr sp (new RegisterAST(aarch64::sp));
  RegisterAST::Ptr pc (new RegisterAST(aarch64::pc));

  test_results_t retVal = PASSED;

  // build expected insns
  // add x1, x1, #0x7;
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
  tmpRead = { x1 };
  tmpWritten = { x1 };
#else
  tmpRead = list_of(x1);
  tmpWritten = list_of(x1);
#endif
  expectedRead.push_back(tmpRead);

  expectedWritten.push_back(tmpWritten);
  tmpRead.clear();
  tmpWritten.clear();
  //****************

  // sub x1, x1, x0
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && !defined(os_windows_test)
  tmpRead = { x1, x0 };
  tmpWritten = { x1 };
#else
  tmpRead = list_of(x1)(x0);
  tmpWritten = list_of(x1);
#endif
  expectedRead.push_back(tmpRead);

  expectedWritten.push_back(tmpWritten);
  tmpRead.clear();
  tmpWritten.clear();
  //****************

  decodedInsns.pop_back();
  while(!decodedInsns.empty())
  {
      retVal = failure_accumulator(retVal, verify_read_write_sets(decodedInsns.front(), expectedRead.front(),
                                   expectedWritten.front()));
      // TEMP commented out
      /*
      if(decodedInsns.size() == 1)
      {
          if(!decodedInsns.front()->readsMemory())
          {
              logerror("**FAILED**: insn %s did not read memory, expected lhzux r5, r7, r9\n",
                       decodedInsns.front()->format().c_str());
              return FAILED;
          }
      }
      */
      decodedInsns.pop_front();

      expectedRead.pop_front();
      expectedWritten.pop_front();
  }

  return retVal;
}

