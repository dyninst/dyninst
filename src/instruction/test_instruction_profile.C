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
#include "Expression.h"
#include "Symtab.h"
#include "Region.h"
#include "BPatch.h"
#include "BPatch_addressSpace.h"
#include "BPatch_image.h"

using namespace Dyninst;
using namespace InstructionAPI;
using namespace SymtabAPI;

using namespace std;

class test_instruction_profile_Mutator : public InstructionMutator {
public:
   test_instruction_profile_Mutator() { };
   virtual test_results_t executeTest();
};

extern "C" DLLEXPORT TestMutator* test_instruction_profile_factory()
{
   return new test_instruction_profile_Mutator();
}

test_results_t test_instruction_profile_Mutator::executeTest()
{
  Symtab *s;
  if(!Symtab::openFile(s, "/lib/libc.so.6")) {
    logerror("FAILED: couldn't open libc for parsing\n");
    return FAILED;
  }
  
  std::vector<Region*> codeRegions;
  s->getCodeRegions(codeRegions);
  unsigned int cf_count = 0;
  unsigned int valid_count = 0;
  unsigned int total_count = 0;
  
  for(std::vector<Region*>::iterator curReg = codeRegions.begin();
      curReg != codeRegions.end();
      ++curReg)
  {
    const unsigned char* decodeBase = reinterpret_cast<const unsigned char*>((*curReg)->getPtrToRawData());
    
    std::vector<Instruction::Ptr > decodedInsns;
    Instruction::Ptr i;
    InstructionDecoder d;
    unsigned offset = 0;
    // simulate parsing via vector-per-basic-block
    while(offset < (*curReg)->getRegionSize() - 16)
    {
      i = d.decode(decodeBase + offset);
      total_count++;
      decodedInsns.push_back(i);
      if(i) {
	offset += i->size();
	valid_count++;
	if((i->getCategory() != c_NoCategory) && i->getControlFlowTarget())
	{
	  cf_count++;
	  decodedInsns.clear();
	}
      }
      else {
	offset++;
      }
    }
  }
  fprintf(stderr, "Instruction counts: %d total, %d valid, %d control-flow\n", total_count, valid_count, cf_count);
  
  BPatch bp;
  BPatch_addressSpace* libc = bp.openBinary("/lib/libc.so.6");
  if(!libc) {
    logerror("FAILED: Couldn't open libc for parse\n");
    return FAILED;
  }
  
  BPatch_Vector<BPatch_function*> funcs;
  libc->getImage()->getProcedures(funcs);
  
  return PASSED;
}

