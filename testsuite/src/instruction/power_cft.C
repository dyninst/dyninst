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

#include <boost/assign/list_of.hpp>
#include <boost/iterator/indirect_iterator.hpp>
#include <deque>
using namespace Dyninst;
using namespace InstructionAPI;
using namespace boost;
using namespace boost::assign;

using namespace std;

class power_cft_Mutator : public InstructionMutator {
public:
    power_cft_Mutator() { };
   virtual test_results_t executeTest();
};

extern "C" DLLEXPORT TestMutator* power_cft_factory()
{
   return new power_cft_Mutator();
}

struct cftExpected
{
    bool defined;
    unsigned int expected;
    bool call;
    bool conditional;
    bool indirect;
    bool fallthrough;
    cftExpected(bool d, unsigned int e, bool isCall, bool isCond, bool isIndir, bool isFT) :
            defined(d), expected(e), call(isCall), conditional(isCond), indirect(isIndir),
            fallthrough(isFT) {}
};

test_results_t verifyTargetType(const Instruction::CFT& actual, const cftExpected& expected)
{
    if(actual.isCall != expected.call) {
        logerror("FAILED: expected call = %d, actual = %d\n", expected.call, actual.isCall);
        return FAILED;
    }
    if(actual.isIndirect != expected.indirect) {
        logerror("FAILED: expected indirect = %d, actual = %d\n", expected.indirect, actual.isIndirect);
        return FAILED;
    }
    if(actual.isConditional != expected.conditional) {
        logerror("FAILED: expected conditional = %d, actual = %d\n", expected.conditional, actual.isConditional);
        return FAILED;
    }
    if(actual.isFallthrough != expected.fallthrough) {
        logerror("FAILED: expected fallthrough = %d, actual = %d\n", expected.fallthrough, actual.isFallthrough);
        return FAILED;
    }
    return PASSED;
}

test_results_t power_cft_Mutator::executeTest()
{
  const unsigned char buffer[] = 
  {
        0x48, 0x00, 0x00, 0x10, // b +16                     
        0x42, 0xf0, 0x00, 0x20, // b +32
        0x4e, 0xf0, 0x04, 0x20, // bctr
        0x42, 0xf0, 0x00, 0x22, // b 32
        0x42, 0xf0, 0xff, 0xd0, // b -32
        0x4e, 0xf0, 0x00, 0x20, // blr
        0x40, 0x01, 0x01, 0x01, // bdnzl cr0, +0x100
        0x40, 0x01, 0x01, 0x00, // bdnz cr0, +0x100
        0x4e, 0xf0, 0x04, 0x21, // bctrl
        0x4c, 0xa3, 0x00, 0x20, // bnslr+
  };
  unsigned int expectedInsns = 10;
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
  while(i);
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

  test_results_t retVal = PASSED;
  
  decodedInsns.pop_back();
  Expression* theIP = new RegisterAST(ppc32::pc);
  Expression* count_reg = new RegisterAST(ppc32::ctr);
  Expression* link_reg = new RegisterAST(ppc32::lr);
  

  std::list<cftExpected> cfts;
  cfts.push_back(cftExpected(true, 0x410, false, false, false, false));
  cfts.push_back(cftExpected(true, 0x420, false, false, false, false));
  cfts.push_back(cftExpected(true, 44, false, false, true, false));
  cfts.push_back(cftExpected(true, 0x20, false, false, false, false));
  cfts.push_back(cftExpected(true, 0x3d0, false, false, false, false));
  cfts.push_back(cftExpected(true, 0x200, false, false, true, false));
  cfts.push_back(cftExpected(true, 0x500, true, true, false, false));
  cfts.push_back(cftExpected(true, 0x404, false, false, false, true));
  cfts.push_back(cftExpected(true, 0x500, false, true, false, false));
  cfts.push_back(cftExpected(true, 0x404, false, false, false, true));
  cfts.push_back(cftExpected(true, 44, true, false, true, false));
  cfts.push_back(cftExpected(true, 0x200, false, true, true, false));
  cfts.push_back(cftExpected(true, 0x404, false, false, false, true));
  while(!decodedInsns.empty())
  {
      (void)(decodedInsns.front()->getControlFlowTarget());
      for(Instruction::cftConstIter curCFT = decodedInsns.front()->cft_begin();
          curCFT != decodedInsns.front()->cft_end();
          ++curCFT)
      {
          Expression::Ptr theCFT = curCFT->target;
          if(theCFT)
          {
              theCFT->bind(theIP, Result(u32, 0x400));
              theCFT->bind(count_reg, Result(u32, 44));
              theCFT->bind(link_reg, Result(u32, 0x200));
              retVal = failure_accumulator(retVal, verifyCFT(theCFT, cfts.front().defined, cfts.front().expected, u32));
              retVal = failure_accumulator(retVal, verifyTargetType(*curCFT, cfts.front()));
          }
          else
          {
              logerror("FAILED: instruction %s expected CFT, wasn't present", decodedInsns.front()->format().c_str());
              retVal = failure_accumulator(retVal, FAILED);
          }
          cfts.pop_front();
      }
      
      decodedInsns.pop_front();
  }

  if(!cfts.empty())
  {
      logerror("FAILED: didn't consume all expected CFTs, %d remain\n", cfts.size());
      return FAILED;
  }
  return retVal;
}

