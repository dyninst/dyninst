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
#include <boost/assign/list_of.hpp>
#include <deque>
using namespace Dyninst;
using namespace InstructionAPI;
using namespace boost;
using namespace boost::assign;

using namespace std;

class mov_size_details_Mutator : public InstructionMutator {
public:
   mov_size_details_Mutator() { };
   virtual test_results_t executeTest();
};

extern "C" DLLEXPORT TestMutator* mov_size_details_factory()
{
   return new mov_size_details_Mutator();
}


test_results_t mov_size_details_Mutator::executeTest()
{
  const unsigned char buffer[] = 
  {
    0x66, 0x8c, 0xe8
  };
  unsigned int size = 3;
  unsigned int expectedInsns = 2;
  InstructionDecoder d(buffer, size, Dyninst::Arch_x86);
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
      logerror("\t%s\n", (*curInsn)->format().c_str());
    }
    
    return FAILED;
  }
  if(decodedInsns.back() && decodedInsns.back()->isValid())
  {
    logerror("FAILED: Expected instructions to end with an invalid instruction, but they didn't");
    return FAILED;
  }
  
  Architecture curArch = Arch_x86;
  Instruction::Ptr mov = decodedInsns.front();

  Expression::Ptr lhs, rhs;
  lhs = mov->getOperand(0).getValue();
  rhs = mov->getOperand(1).getValue();
  
  if(lhs->size() != 2)
  {
    logerror("LHS expected 16-bit, actual %d-bit (%s)\n", lhs->size() * 8, lhs->format().c_str());
    return FAILED;
  }
  if(rhs->size() != 2)
  {
    logerror("RHS expected 16-bit, actual %d-bit (%s)\n", rhs->size() * 8, rhs->format().c_str());
    return FAILED;
  }


  return PASSED;
}

