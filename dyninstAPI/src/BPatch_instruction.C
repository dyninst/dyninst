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

#include "common/h/Types.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "BPatch.h"
#include "BPatch_instruction.h"
#include "BPatch_basicBlock.h"
#include "BPatch_libInfo.h"
#include "BPatch_process.h"
#include "util.h"
#include "function.h"
#include "instPoint.h"
#include "addressSpace.h"

#include "legacy-instruction.h"

/**************************************************************************
 * BPatch_instruction
 *************************************************************************/

#if defined(arch_x86) || defined(arch_x86_64)
const unsigned int BPatch_instruction::nmaxacc_NP = 2;
#else
const unsigned int BPatch_instruction::nmaxacc_NP = 1;
#endif

BPatch_instruction::BPatch_instruction(internal_instruction *insn,
                                       Address addr_) : nacc(0), insn_(insn), addr(addr_)
{
  isLoad = new bool[nmaxacc_NP];
  isStore = new bool[nmaxacc_NP];
  preFcn = new int[nmaxacc_NP];
  condition = new int[nmaxacc_NP];
  nonTemporal = new bool[nmaxacc_NP];

  for (unsigned int i=0; i < nmaxacc_NP; i++) {
    isLoad[i] = false;
    isStore[i] = false;
    preFcn[i] = -1;
    condition[i] = -1;
    nonTemporal[i] = false;
  }

}

internal_instruction *BPatch_instruction::insn() { return insn_; }

BPatch_instruction::~BPatch_instruction() {

   delete[] isLoad;
   delete[] isStore;
   delete[] preFcn;
   delete[] condition;
   delete[] nonTemporal;

   if(insn_)
    delete insn_;
}

BPatch_basicBlock *BPatch_instruction::getParentInt()
{
  return parent;
}

void *BPatch_instruction::getAddressInt()
{
  return (void *)addr;
}
BPatch_point *BPatch_instruction::getInstPointInt()
{
  //const unsigned char *insn_ptr = ((instruction *)instr)->ptr();
  int_basicBlock *iblock = parent->iblock;
  AddressSpace *proc = iblock->proc();
  int_function *func = iblock->func();

  assert(proc);
  assert(func);

  //BPatch_process *bpproc = (BPatch_process *)proc->up_ptr();
  
  BPatch_addressSpace *bpproc = (BPatch_addressSpace *)proc->up_ptr();
  //  assert(bpproc); 
  assert(bpproc);

  // If it's in an uninstrumentable function, just return an error.
  if ( !func || !((int_function*)func)->isInstrumentable()){
    fprintf(stderr, "%s[%d]:  function is not instrumentable\n", FILE__, __LINE__);
    return NULL;
  }

 /* See if there is an instPoint at this address */
  instPoint *p = NULL;

  if ((p = func->findInstPByAddr((Address)addr))) {
    return bpproc->findOrCreateBPPoint(NULL, p, BPatch_locInstruction);
  }

  /* We don't have an instPoint for this address, so make one. */
  instPoint *newInstP = instPoint::createArbitraryInstPoint((Address)addr, proc, func);
  
  if (!newInstP) {
     fprintf(stderr, "%s[%d]:  createArbitraryInstPoint for %p failed\n", FILE__, __LINE__, (void *) addr);
     return NULL;
  }
  BPatch_point *ret = bpproc->findOrCreateBPPoint(NULL, newInstP, BPatch_locInstruction);

  if (!ret)
    fprintf(stderr, "%s[%d]:  getInstPoint failing!\n", FILE__, __LINE__);
  return ret;
}

std::string BPatch_register::nameInt() {
    return name_;
}

