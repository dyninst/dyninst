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

class power_decode_Mutator : public InstructionMutator {
public:
    power_decode_Mutator() { };
   virtual test_results_t executeTest();
};

extern "C" DLLEXPORT TestMutator* power_decode_factory()
{
   return new power_decode_Mutator();
}

test_results_t power_decode_Mutator::executeTest()
{
  const unsigned char buffer[] = 
  {
      0x7d, 0x20, 0x42, 0x15, // add. r9, r0, r8
      0x7d, 0x20, 0x42, 0x14,  // add r9, r0, r8
      0x7d, 0x20, 0x46, 0x14,  // addo r9, r0, r8
      0xfc, 0x01, 0x10, 0x2a, // fadd fpr0, fpr1, fpr2
      0xfc, 0x01, 0x10, 0x2b, // fadd. fpr0, fpr1, fpr2
      0x38, 0x20, 0x00, 0x01, // addi r1, 0, 1
      0x38, 0x21, 0x00, 0x01, // addi r1, r1, 1
      0xf0, 0x00, 0xff, 0xfc, // stfq fpr0, 0, -1
      0xf0, 0x01, 0x00, 0x04, // stfq fpr0, r1, 1
      0xff, 0x80, 0x08, 0x00, // fcmpu fpscr7, fpr0, fpr1
      0x7f, 0x80, 0x08, 0x00, // fcmpu cr7, r0, r1
      0x7c, 0x0a, 0xa1, 0x20, // mtcrf cr0, cr2, cr4, cr6, r0
      0xfd, 0x54, 0x06, 0x06, // mtfsf fpscr0, fpscr2, fpscr4, fpscr6, fpr0
      0x80, 0x01, 0x00, 0x00, // lwz r0, 0(r1)
      0x84, 0x01, 0x00, 0x00, // lwzu r0, 0(r1)
      0x7c, 0x01, 0x10, 0x2e, // lwzx r0, r2(r1)
      0x7c, 0x01, 0x10, 0x6e, // lwzux r0, r2(r1)
      0x78, 0x01, 0x44, 0x0c, // rlimi r0, r1
      0x00, 0x01, 0x00, 0x90, // fpmul fpr0, fpr1
      0x00, 0x01, 0x00, 0x92, // fxmul fpr0, fpr1, or qvfxmadds 
      0x00, 0x01, 0x00, 0x94, // fxcpmul fpr0, fpr1
      0x00, 0x01, 0x00, 0x96, // fxcsmul fpr0, fpr1, or qvfxxnpmadds
      0x40, 0x01, 0x01, 0x01, // bdnzl cr0, +0x100
      0x40, 0x01, 0x01, 0x00, // bdnz cr0, +0x100
      0x7c, 0xa7, 0x4a, 0x6e, // lhzux r9, r7, r5
                
  };
  unsigned int expectedInsns = 25;
  unsigned int size = expectedInsns * 4;
  ++expectedInsns;
  InstructionDecoder d(buffer, size, Dyninst::Arch_ppc32);
  
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
  RegisterAST::Ptr r0(new RegisterAST(ppc32::r0));
  RegisterAST::Ptr r1(new RegisterAST(ppc32::r1));
  RegisterAST::Ptr r2(new RegisterAST(ppc32::r2));
  RegisterAST::Ptr r5(new RegisterAST(ppc32::r5));
  RegisterAST::Ptr r7(new RegisterAST(ppc32::r7));
  RegisterAST::Ptr r8(new RegisterAST(ppc32::r8));
  RegisterAST::Ptr r9(new RegisterAST(ppc32::r9));
  RegisterAST::Ptr cr0(new RegisterAST(ppc32::cr0));
  RegisterAST::Ptr cr2(new RegisterAST(ppc32::cr2));
  RegisterAST::Ptr cr4(new RegisterAST(ppc32::cr4));
  RegisterAST::Ptr cr6(new RegisterAST(ppc32::cr6));
  RegisterAST::Ptr cr7(new RegisterAST(ppc32::cr7));
  RegisterAST::Ptr xer(new RegisterAST(ppc32::xer));
  RegisterAST::Ptr fpr0(new RegisterAST(ppc32::fpr0));
  RegisterAST::Ptr fpr1(new RegisterAST(ppc32::fpr1));
  RegisterAST::Ptr fpr2(new RegisterAST(ppc32::fpr2));
  RegisterAST::Ptr fsr0(new RegisterAST(ppc32::fsr0));
  RegisterAST::Ptr fsr1(new RegisterAST(ppc32::fsr1));
  RegisterAST::Ptr fsr2(new RegisterAST(ppc32::fsr2));
  RegisterAST::Ptr fpscr(new RegisterAST(ppc32::fpscw));
  RegisterAST::Ptr fpscr0(new RegisterAST(ppc32::fpscw0));
  RegisterAST::Ptr fpscr2(new RegisterAST(ppc32::fpscw2));
  RegisterAST::Ptr fpscr4(new RegisterAST(ppc32::fpscw4));
  RegisterAST::Ptr fpscr6(new RegisterAST(ppc32::fpscw6));
  RegisterAST::Ptr fpscr7(new RegisterAST(ppc32::fpscw7));
  RegisterAST::Ptr pc(new RegisterAST(ppc32::pc));
  RegisterAST::Ptr ctr(new RegisterAST(ppc32::ctr));
  RegisterAST::Ptr lr(new RegisterAST(ppc32::lr));
  // add.
  test_results_t retVal = PASSED;
  
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
  tmpRead = { r0, r8 };
  tmpWritten = { r9, cr0 };
#else
  tmpRead = list_of(r0)(r8);
  tmpWritten = list_of(r9)(cr0);
#endif
  expectedRead.push_back(tmpRead);

  expectedWritten.push_back(tmpWritten);
  tmpRead.clear();
  tmpWritten.clear();
  // add
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
  tmpRead = { r0, r8 };
  tmpWritten = { r9 };
#else
  tmpRead = list_of(r0)(r8);
  tmpWritten = list_of(r9);
#endif
  expectedRead.push_back(tmpRead);

  expectedWritten.push_back(tmpWritten);
  tmpRead.clear();
  tmpWritten.clear();
  // addo
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
  tmpRead = { r0, r8 };
  tmpWritten = { r9, xer };
#else
  tmpRead = list_of(r0)(r8);
  tmpWritten = list_of(r9)(xer);
#endif
  expectedRead.push_back(tmpRead);
  expectedWritten.push_back(tmpWritten);
  tmpRead.clear();
  tmpWritten.clear();

  // fadd
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
  tmpRead = { fpr1, fpr2 };
  tmpWritten = { fpr0 };
#else
  tmpRead = list_of(fpr1)(fpr2);
  tmpWritten = list_of(fpr0);
#endif
  expectedRead.push_back(tmpRead);
  expectedWritten.push_back(tmpWritten);
  tmpRead.clear();
  tmpWritten.clear();
  // fadd.
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
  tmpRead = { fpr1, fpr2 };
  tmpWritten = { fpr0, fpscr };
#else
  tmpRead = list_of(fpr1)(fpr2);
  tmpWritten = list_of(fpr0)(fpscr);
#endif

  expectedRead.push_back(tmpRead);
  expectedWritten.push_back(tmpWritten);
  tmpRead.clear();
  tmpWritten.clear();
  // addi, r0

#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
  tmpWritten = { r1 };
#else
  tmpWritten = list_of(r1);
#endif

  expectedRead.push_back(tmpRead);
  expectedWritten.push_back(tmpWritten);
  tmpRead.clear();
  tmpWritten.clear();
  // addi, r1
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
  tmpRead = { r1 };
  tmpWritten = { r1 };
#else
  tmpRead = list_of(r1);
  tmpWritten = list_of(r1);
#endif
  expectedRead.push_back(tmpRead);
  expectedWritten.push_back(tmpWritten);
  tmpRead.clear();
  tmpWritten.clear();
  // stfq fpr0/fpr1, -1(0)
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
  tmpRead = { fpr0, fpr1 };
#else
  tmpRead = list_of(fpr0)(fpr1);
#endif

  expectedRead.push_back(tmpRead);
  expectedWritten.push_back(tmpWritten);
  tmpRead.clear();
  tmpWritten.clear();
  // stfq fpr0/fpr1, 1(r1)
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
  tmpRead = { fpr0, fpr1, r1 };
#else
  tmpRead = list_of(fpr0)(fpr1)(r1);
#endif
  expectedRead.push_back(tmpRead);
  expectedWritten.push_back(tmpWritten);
  tmpRead.clear();
  tmpWritten.clear();
  // fcmpu fpscr7, fpr0, fpr1
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
  tmpRead = { fpr0, fpr1 };
  tmpWritten = {fpscr7 };
#else
  tmpRead = list_of(fpr0)(fpr1);
  tmpWritten = list_of(fpscr7);
#endif
  expectedRead.push_back(tmpRead);
  expectedWritten.push_back(tmpWritten);
  tmpRead.clear();
  tmpWritten.clear();
  // cmp cr7, r0, r1
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
  tmpRead = { r0, r1 };
  tmpWritten = { cr7 };
#else
  tmpRead = list_of(r0)(r1);
  tmpWritten = list_of(cr7);
#endif
  expectedRead.push_back(tmpRead);
  expectedWritten.push_back(tmpWritten);
  tmpRead.clear();
  tmpWritten.clear();
  // mtcrf cr0, cr2, cr4, cr6, r0
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
  tmpRead = { r0 };
  tmpWritten = { cr0, cr2, cr4, cr6 };
#else
  tmpRead = list_of(r0);
  tmpWritten = list_of(cr0)(cr2)(cr4)(cr6);
#endif
  expectedRead.push_back(tmpRead);
  expectedWritten.push_back(tmpWritten);
  tmpRead.clear();
  tmpWritten.clear();
  // mtfsf fpscr0, fpscr2, fpscr4, fpscr6, fpr0
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
  tmpRead = { fpr0 };
  tmpWritten = { fpscr0, fpscr2, fpscr4, fpscr6 };
#else
  tmpRead = list_of(fpr0);
  tmpWritten = list_of(fpscr0)(fpscr2)(fpscr4)(fpscr6);
#endif
  expectedRead.push_back(tmpRead);
  expectedWritten.push_back(tmpWritten);
  tmpRead.clear();
  tmpWritten.clear();
  // lwz r0, 0(r1)
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
  tmpRead = { r1 };
  tmpWritten = { r0 };
#else
  tmpRead = list_of(r1);
  tmpWritten = list_of(r0);
#endif
  expectedRead.push_back(tmpRead);
  expectedWritten.push_back(tmpWritten);
  tmpRead.clear();
  tmpWritten.clear();
  // lwzu r0, 0(r1)
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
  tmpRead = { r1 };
  tmpWritten = { r0, r1 };
#else
  tmpRead = list_of(r1);
  tmpWritten = list_of(r0)(r1);
#endif
  expectedRead.push_back(tmpRead);
  expectedWritten.push_back(tmpWritten);
  tmpRead.clear();
  tmpWritten.clear();
  // lwzx r0, r2(r1)
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
  tmpRead = { r1, r2 };
  tmpWritten = { r0 };
#else
  tmpRead = list_of(r1)(r2);
  tmpWritten = list_of(r0);
#endif
  expectedRead.push_back(tmpRead);
  expectedWritten.push_back(tmpWritten);
  tmpRead.clear();
  tmpWritten.clear();
  // lwzux r0, r2(r1)
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
  tmpRead = { r1, r2 };
  tmpWritten = { r0, r1 };
#else
  tmpRead = list_of(r1)(r2);
  tmpWritten = list_of(r0)(r1);
#endif
  expectedRead.push_back(tmpRead);
  expectedWritten.push_back(tmpWritten);
  tmpRead.clear();
  tmpWritten.clear();
  // rlimi r0, r1
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
  tmpRead = { r0 };
  tmpWritten = { r1 };
#else
  tmpRead = list_of(r0);
  tmpWritten = list_of(r1);
#endif
  expectedRead.push_back(tmpRead);
  expectedWritten.push_back(tmpWritten);
  tmpRead.clear();
  tmpWritten.clear();
  // fpmul fpr0, fpr1, fpr2
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
  tmpRead = { fpr1, fpr2, fsr1, fsr2 };
  tmpWritten = { fpr0, fsr0 };
#else
  tmpRead = list_of(fpr1)(fpr2)(fsr1)(fsr2);
  tmpWritten = list_of(fpr0)(fsr0);
#endif
  expectedRead.push_back(tmpRead);
  expectedWritten.push_back(tmpWritten);
  tmpRead.clear();
  tmpWritten.clear();
#if defined(os_bgq_test)
  // qvfxmadds
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
  tmpRead = { fpr0, fpr1, fpr2, fsr0, fsr2 };
  tmpWritten = { fpr0, fsr0 };
#else
  tmpRead = list_of(fpr0)(fpr1)(fpr2)(fsr0)(fsr2);
  tmpWritten = list_of(fpr0)(fsr0);
#endif
  expectedRead.push_back(tmpRead);
  expectedWritten.push_back(tmpWritten);
  tmpRead.clear();
  tmpWritten.clear();
#else
  // fxmul fpr0, fpr1
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
  tmpRead = { fpr1, fpr2, fsr1, fsr2 };
  tmpWritten = { fpr0, fsr0 };
#else
  tmpRead = list_of(fpr1)(fpr2)(fsr1)(fsr2);
  tmpWritten = list_of(fpr0)(fsr0);
#endif
  expectedRead.push_back(tmpRead);
  expectedWritten.push_back(tmpWritten);
  tmpRead.clear();
  tmpWritten.clear();
#endif
  // fxcpmul fpr0, fpr1
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
  tmpRead = { fpr1, fpr2, fsr2 };
  tmpWritten = { fpr0, fsr0 };
#else
  tmpRead = list_of(fpr1)(fpr2)(fsr2);
  tmpWritten = list_of(fpr0)(fsr0);
#endif
  expectedRead.push_back(tmpRead);
  expectedWritten.push_back(tmpWritten);
  tmpRead.clear();
  tmpWritten.clear();
#if defined(os_bgq_test)
  // qvfxxnpmadds
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
  tmpRead = { fpr0, fpr1, fpr2, fsr0, fsr2 };
  tmpWritten = { fpr0, fsr0 };
#else
  tmpRead = list_of(fpr0)(fpr1)(fpr2)(fsr0)(fsr2);
  tmpWritten = list_of(fpr0)(fsr0);
#endif
  expectedRead.push_back(tmpRead);
  expectedWritten.push_back(tmpWritten);
  tmpRead.clear();
  tmpWritten.clear();
#else
  // fxcsmul fpr0, fpr1
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
  tmpRead = { fpr2, fsr1, fsr2 };
  tmpWritten = { fpr0, fsr0 };
#else
  tmpRead = list_of(fpr2)(fsr1)(fsr2);
  tmpWritten = list_of(fpr0)(fsr0);
#endif
  expectedRead.push_back(tmpRead);
  expectedWritten.push_back(tmpWritten);
  tmpRead.clear();
  tmpWritten.clear();
#endif
  // bdnzl cr0, +0x100
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
  tmpRead = { pc, cr0, ctr };
  tmpWritten = {pc, ctr, lr };
#else
  tmpRead = list_of(pc)(cr0)(ctr);
  tmpWritten = list_of(pc)(ctr)(lr);
#endif
  expectedRead.push_back(tmpRead);
  expectedWritten.push_back(tmpWritten);
  tmpRead.clear();
  tmpWritten.clear();
  // bdnz cr0, +0x100
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
  tmpRead = { pc, cr0, ctr };
  tmpWritten = { pc, ctr };
#else
  tmpRead = list_of(pc)(cr0)(ctr);
  tmpWritten = list_of(pc)(ctr);
#endif
  expectedRead.push_back(tmpRead);
  expectedWritten.push_back(tmpWritten);
  tmpRead.clear();
  tmpWritten.clear();
  // lhzux r5, r7, r9
#if !defined(NO_INITIALIZER_LIST_SUPPORT) && (!defined(os_windows) || _MSC_VER >= 1900)
  tmpRead = { r7, r9 };
  tmpWritten = { r5, r7 };
#else
  tmpRead = list_of(r7)(r9);
  tmpWritten = list_of(r5)(r7);
#endif
  expectedRead.push_back(tmpRead);
  expectedWritten.push_back(tmpWritten);
  tmpRead.clear();
  tmpWritten.clear();

  decodedInsns.pop_back();
  while(!decodedInsns.empty())
  {
      retVal = failure_accumulator(retVal, verify_read_write_sets(decodedInsns.front(), expectedRead.front(),
                                   expectedWritten.front()));
      // TEMP
      if(decodedInsns.size() == 1)
      {
          if(!decodedInsns.front()->readsMemory())
          {
              logerror("**FAILED**: insn %s did not read memory, expected lhzux r5, r7, r9\n",
                       decodedInsns.front()->format().c_str());
              return FAILED;
          }
      }
      decodedInsns.pop_front();
  
      expectedRead.pop_front();
      expectedWritten.pop_front();
  }

  return retVal;
}

