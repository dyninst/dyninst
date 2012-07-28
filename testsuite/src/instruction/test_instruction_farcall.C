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

#include <boost/assign.hpp>
#include <boost/iterator/indirect_iterator.hpp>

using namespace Dyninst;
using namespace InstructionAPI;
using namespace boost;
using namespace std;

class test_instruction_farcall_Mutator : public InstructionMutator {
public:
   test_instruction_farcall_Mutator() { };
   virtual test_results_t executeTest();
};

extern "C" DLLEXPORT TestMutator* test_instruction_farcall_factory()
{
   return new test_instruction_farcall_Mutator();
}


test_results_t test_instruction_farcall_Mutator::executeTest()
{
  const unsigned char buffer[] = 
  {
    0x9A, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0xFF, 0xFE // CALL 0504030201, with FF/FE as fenceposts
  };
  unsigned int size = 7;
  unsigned int expectedInsns = 2;

#if defined(arch_x86_64_test)
    Architecture curArch = Arch_x86_64;
#elif defined(arch_x86_test)
    Architecture curArch = Arch_x86;
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
#if defined(arch_x86_64_test)
  if(decodedInsns.empty() || !decodedInsns[0] || !decodedInsns[0]->isValid() || decodedInsns[0]->isLegalInsn())
  {
    logerror("FAILED: %s\n", decodedInsns.empty() ? "no instructions decoded" : "first instruction was valid");
    return FAILED;
  }
  else
  {
    logerror("PASSED: far call invalid on AMD64\n");
    return PASSED;
  }
#else
  if(decodedInsns.size() != expectedInsns) // six valid, one invalid
  {
    logerror("FAILED: Expected %d instructions, decoded %d\n", expectedInsns, decodedInsns.size());
    for(std::vector<Instruction::Ptr>::iterator curInsn = decodedInsns.begin();
	curInsn != decodedInsns.end();
	++curInsn)
    {
      logerror("\t%s\t", (*curInsn)->format().c_str());
      for(unsigned j = 0; j < (*curInsn)->size(); ++j)
      {
	logerror("%x ", (*curInsn)->rawByte(j));
      }
      logerror("\n");
    }
    
    return FAILED;
  }
  if(decodedInsns.back() && decodedInsns.back()->isValid())
  {
    logerror("FAILED: Expected instructions to end with an invalid instruction, but they didn't");
    return FAILED;
  }
  return PASSED;
#endif
}
