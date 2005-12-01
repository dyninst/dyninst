/*
 * Copyright (c) 1996-2004 Barton P. Miller
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

#include "common/h/Types.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "BPatch.h"
#include "BPatch_instruction.h"
#include "BPatch_basicBlock.h"
#include "BPatch_libInfo.h"
#include "BPatch_process.h"
#include "arch.h"
#include "util.h"
#include "function.h"
#include "process.h"
#include "instPoint.h"

/**************************************************************************
 * BPatch_instruction
 *************************************************************************/

#if defined(arch_x86) || defined(arch_x86_64)
unsigned int BPatch_instruction::nmaxacc_NP = 2;
#else
unsigned int BPatch_instruction::nmaxacc_NP = 1;
#endif

BPatch_instruction::BPatch_instruction(const void *_buffer,
				       unsigned char _length,
                                       Address addr_) : nacc(0), length(_length), addr(addr_)
{
  assert(_buffer);

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

  buffer = new unsigned char[length];
  memcpy(buffer, _buffer, length * sizeof(unsigned char));
}

BPatch_instruction::~BPatch_instruction() {
   if (buffer)
      delete[] buffer;

   delete isLoad;
   delete isStore;
   delete preFcn;
   delete condition;
   delete nonTemporal;
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
  process *proc = iblock->proc();
  int_function *func = iblock->func();

  assert(proc);
  assert(func);

  BPatch_process *bpproc = BPatch::bpatch->getProcessByPid(proc->getPid());
  assert(bpproc); 

  // If it's in an uninstrumentable function, just return an error.
  if ( !func || !((int_function*)func)->isInstrumentable()){
    fprintf(stderr, "%s[%d]:  function is not instrumentable\n", FILE__, __LINE__);
    return NULL;
  }

 /* See if there is an instPoint at this address */
  instPoint *p = NULL;

  if ((p = proc->findInstPByAddr((Address)addr))) {
    fprintf(stderr, "%s[%d]:  point exists at requested address\n", FILE__, __LINE__);
    return bpproc->findOrCreateBPPoint(NULL, p, BPatch_locInstruction);
  }

  /* We don't have an instPoint for this address, so make one. */
  instPoint *newInstP = instPoint::createArbitraryInstPoint((Address)addr, proc);
  
  if (!newInstP) {
     fprintf(stderr, "%s[%d]:  createArbitraryInstPoint for %p failed\n", FILE__, __LINE__, (void *) addr);
     return NULL;
  }
  BPatch_point *ret = bpproc->findOrCreateBPPoint(NULL, newInstP, BPatch_locInstruction);

  if (!ret)
    fprintf(stderr, "%s[%d]:  getInstPoint failing!\n", FILE__, __LINE__);
  return ret;
}


