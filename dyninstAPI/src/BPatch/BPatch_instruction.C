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

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "BPatch.h"
#include "BPatch_instruction.h"
#include "BPatch_basicBlock.h"
#include "BPatch_libInfo.h"
#include "BPatch_process.h"
#include "dyninst_visibility.h"
#include "function.h"
#include "instPoint.h"
#include "addressSpace.h"

/**************************************************************************
 * BPatch_instruction
 *************************************************************************/

#if defined(DYNINST_HOST_ARCH_X86) || defined(DYNINST_HOST_ARCH_X86_64)
const unsigned int BPatch_instruction::nmaxacc_NP = 2;
#else
const unsigned int BPatch_instruction::nmaxacc_NP = 1;
#endif

BPatch_instruction::BPatch_instruction(Address addr_)
 : nacc(0), parent(NULL), addr(addr_)
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

BPatch_instruction::~BPatch_instruction() {

   delete[] isLoad;
   delete[] isStore;
   delete[] preFcn;
   delete[] condition;
   delete[] nonTemporal;
}

BPatch_basicBlock *BPatch_instruction::getParent()
{
  return parent;
}

void *BPatch_instruction::getAddress()
{
  return (void *)addr;
}
BPatch_point *BPatch_instruction::getInstPoint()
{
   func_instance *ifunc = parent->ifunc();
   AddressSpace *proc = ifunc->proc();
   BPatch_addressSpace *bpproc = (BPatch_addressSpace *)proc->up_ptr();
   assert(bpproc);
   instPoint *point = instPoint::preInsn(ifunc,
                                         parent->block(),
                                         addr);
   
   BPatch_point *ret = bpproc->findOrCreateBPPoint(NULL, point, BPatch_locInstruction);
   
   if (!ret)
      fprintf(stderr, "%s[%d]:  getInstPoint failing!\n", FILE__, __LINE__);
   return ret;
}

std::string BPatch_register::name() const{
    return name_;
}

