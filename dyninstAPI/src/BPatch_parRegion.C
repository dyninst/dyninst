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

#include "process.h"
#include "EventHandler.h"
#include "mailbox.h"
#include "signalgenerator.h"
#include "inst.h"
#include "instP.h"
#include "instPoint.h"
#include "function.h" // int_function
#include "codeRange.h"
#include "dyn_thread.h"
#include "miniTramp.h"

#include "mapped_module.h"

#include "BPatch_libInfo.h"
#include "BPatch_asyncEventHandler.h"
#include "BPatch.h"
#include "BPatch_thread.h"
#include "LineInformation.h"
#include "BPatch_function.h"
#include "BPatch_parRegion.h"
#include "InstrucIter.h"
#include "callbacks.h"

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

BPatch_Vector<BPatch_instruction*> *BPatch_parRegion::getInstructionsInt(void) {

  if (!instructions) {

    instructions = new BPatch_Vector<BPatch_instruction*>;

    InstrucIter ii(this);
    
    while(ii.hasMore()) {

      BPatch_instruction *instr = ii.getBPInstruction();
      instructions->push_back(instr);
      ii++;
    }
    
  }

  return instructions;
}

unsigned long BPatch_parRegion::getStartAddressInt() CONST_EXPORT 
{
   return parReg->firstInsnAddr();
}

unsigned long BPatch_parRegion::getEndAddressInt() CONST_EXPORT
{
   return parReg->endAddr();
}

unsigned BPatch_parRegion::sizeInt() CONST_EXPORT
{
   return getEndAddress() - getStartAddress();
}

int BPatch_parRegion::getClauseInt(const char * key) 
{  
  return  parReg->getClause(key); 

}

int BPatch_parRegion::replaceOMPParameterInt(const char * key, int value)
{
  return parReg->replaceOMPParameter(key,value);
}

