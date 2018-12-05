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

#include "inst.h"
#include "instP.h"
#include "instPoint.h"
#include "function.h" // func_instance
#include "codeRange.h"

#include "mapped_module.h"

#include "BPatch_libInfo.h"
#include "BPatch.h"
#include "BPatch_thread.h"
#include "BPatch_function.h"
#include "BPatch_parRegion.h"
#include "addressSpace.h"
#include "instructionAPI/h/Instruction.h"
#include "instructionAPI/h/InstructionDecoder.h"

BPatch_parRegion::BPatch_parRegion(int_parRegion * _parReg, BPatch_function * _func)
{
  parReg = _parReg;
  func = _func;
  instructions = NULL;
}

BPatch_parRegion::~BPatch_parRegion()
{  }

void BPatch_parRegion::printDetails()
{
  parReg->printDetails();
}
BPatch_Vector<BPatch_instruction*> *BPatch_parRegion::getInstructions(void) {

	return NULL;
}

bool BPatch_parRegion::getInstructions(std::vector<InstructionAPI::Instruction>& insns) {
  using namespace InstructionAPI;
  const unsigned char* buffer = 
  (const unsigned char*)(lowlevel_region()->intFunc()->proc()->getPtrToInstruction(getStartAddress()));
  
  InstructionDecoder d(buffer, size(),
        lowlevel_region()->intFunc()->proc()->getArch());
  Instruction curInsn = d.decode();
  while(curInsn.isValid())
  {
    insns.push_back(curInsn);
    curInsn = d.decode();
  }
  return !insns.empty();
  
}

unsigned long BPatch_parRegion::getStartAddress() const 
{
   return parReg->firstInsnAddr();
}

unsigned long BPatch_parRegion::getEndAddress() const
{
   return parReg->endAddr();
}

unsigned BPatch_parRegion::size() const
{
   return getEndAddress() - getStartAddress();
}

int BPatch_parRegion::getClause(const char * key) 
{  
  return  parReg->getClause(key); 

}

int BPatch_parRegion::replaceOMPParameter(const char * key, int value)
{
  return parReg->replaceOMPParameter(key,value);
}

