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
#include "InstructionDecoder-power.h"
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
    cftExpected(bool d, unsigned int e) :
            defined(d), expected(e) {}
};

test_results_t power_cft_Mutator::executeTest()
{
  const unsigned char buffer[] = 
  {
        0x48, 0x00, 0x00, 0x10, // b +16                     
        0x42, 0xf0, 0x00, 0x20, // b +32
        0x4e, 0xf0, 0x04, 0x20, // bctr
        0x42, 0xf0, 0x00, 0x22, // b 32
        0x42, 0xf0, 0xff, 0xd0, // b -32
  };
  unsigned int expectedInsns = 4;
  unsigned int size = expectedInsns * 4;
  ++expectedInsns;
  dyn_detail::boost::shared_ptr<InstructionDecoder> d =
          makeDecoder(Dyninst::InstructionAPI::power, buffer, size);
  
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
  Expression* theIP = new RegisterAST(power_R_PC);
  Expression* count_reg = new RegisterAST(power_CTR);  
  Expression* link_reg = new RegisterAST(power_LR);
  

  std::list<cftExpected> cfts;
  cfts.push_back(cftExpected(true, 0x410));
  cfts.push_back(cftExpected(true, 0x420));
  cfts.push_back(cftExpected(true, 44));
  cfts.push_back(cftExpected(true, 0x20));
  cfts.push_back(cftExpected(true, 0x380));
  while(!decodedInsns.empty())
  {
      Expression::Ptr theCFT = decodedInsns.front()->getControlFlowTarget();
      if(theCFT)
      {
            theCFT->bind(theIP, Result(u32, 0x400));
            theCFT->bind(count_reg, Result(u32, 44));
            retVal = failure_accumulator(retVal, verifyCFT(theCFT, cfts.front().defined, cfts.front().expected, u32));
      }
      else
      {
          logerror("Instruction %s expected CFT, wasn't present", decodedInsns.front()->format().c_str());
          retVal = failure_accumulator(retVal, FAILED);
      }
      decodedInsns.pop_front();
      cfts.pop_front();
  }

  return retVal;
}

