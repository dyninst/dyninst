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
  const char *libcPath = "/lib/libc.so.6";

#if defined(os_freebsd_test)
  libcPath = "/usr/lib/libc.so";
#endif

  if(!Symtab::openFile(s, libcPath)) {
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
    if((*curReg)->getRegionSize() < 16) continue;
    const unsigned char* decodeBase = reinterpret_cast<const unsigned char*>((*curReg)->getPtrToRawData());
    
    std::vector<Instruction::Ptr > decodedInsns;
    Instruction::Ptr i;
    InstructionDecoder d(decodeBase, (*curReg)->getRegionSize(), Dyninst::Arch_x86);
    long offset = 0;
    
    // simulate parsing via vector-per-basic-block
    while(offset < (*curReg)->getRegionSize() - InstructionDecoder::maxInstructionLength)
    {
      i = d.decode(decodeBase + offset);
      //cout << endl << "\t\t" << i->format() << std::endl;
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
  //fprintf(stderr, "Instruction counts: %d total, %d valid, %d control-flow\n", total_count, valid_count, cf_count);
  BPatch bp;
  BPatch_addressSpace* libc = bp.openBinary(libcPath);
  if(!libc) {
    logerror("FAILED: Couldn't open libc for parse\n");
    return FAILED;
  }
  
  BPatch_Vector<BPatch_function*> funcs;
  libc->getImage()->getProcedures(funcs);
  return PASSED;
}

