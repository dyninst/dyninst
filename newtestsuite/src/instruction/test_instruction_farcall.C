/*
 * Copyright (c) 1996-2008 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
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
  
  InstructionDecoder d(buffer, size);
#if defined(arch_x86_64_test)
  d.setMode(true);
#endif
  std::vector<Instruction> decodedInsns;
  Instruction i;
  do
  {
    i = d.decode();
    decodedInsns.push_back(i);
  }
  while(i.isValid());
#if defined(arch_x86_64_test)
  if(decodedInsns.empty() || !decodedInsns[0].isValid() || decodedInsns[0].isLegalInsn())
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
    for(std::vector<Instruction>::iterator curInsn = decodedInsns.begin();
	curInsn != decodedInsns.end();
	++curInsn)
    {
      logerror("\t%s\t", curInsn->format().c_str());
      for(unsigned j = 0; j < curInsn->size(); ++j)
      {
	logerror("%x ", curInsn->rawByte(j));
      }
      logerror("\n");
    }
    
    return FAILED;
  }
  if(decodedInsns.back().isValid())
  {
    logerror("FAILED: Expected instructions to end with an invalid instruction, but they didn't");
    return FAILED;
  }
  return PASSED;
#endif
}
