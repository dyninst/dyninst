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

// $Id: registerSpace.C,v 1.2 2006/10/16 20:17:33 bernat Exp $

#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/instPoint.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/util.h"
#include "dyninstAPI/src/showerror.h"

#include "dyninstAPI/h/BPatch.h"
#include "dyninstAPI/src/BPatch_collections.h"
#include "dyninstAPI/h/BPatch_type.h"
#include "dyninstAPI/src/BPatch_libInfo.h" // For instPoint->BPatch_point mapping

#include "dyninstAPI/h/BPatch_point.h"
#include "dyninstAPI/h/BPatch_memoryAccess_NP.h"

#if defined(sparc_sun_sunos4_1_3) \
 || defined(sparc_sun_solaris2_4)
#include "dyninstAPI/src/inst-sparc.h"

#elif defined(hppa1_1_hp_hpux)
#include "dyninstAPI/src/inst-hppa.h"

#elif defined(rs6000_ibm_aix3_2) \
   || defined(rs6000_ibm_aix4_1)
#include "dyninstAPI/src/inst-power.h"

#elif defined(i386_unknown_solaris2_5) \
   || defined(i386_unknown_nt4_0) \
   || defined(i386_unknown_linux2_0) \
   || defined(x86_64_unknown_linux2_4)
#include "dyninstAPI/src/inst-x86.h"

#elif defined(ia64_unknown_linux2_4) /* Why is this done here, instead of, e.g., inst.h? */
#include "dyninstAPI/src/inst-ia64.h"
#endif

extern registerSpace *regSpace;


registerSpace::registerSpace(const unsigned int deadCount, Register *dead, 
                             const unsigned int liveCount, Register *live,
                             bool multithreaded) :
   highWaterRegister(0), registers(NULL), fpRegisters(NULL), 
   is_multithreaded(multithreaded)
{
#if defined(i386_unknown_solaris2_5) \
 || defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ \
 || defined(ia64_unknown_linux2_4) \
 || defined(os_windows)
  initTramps(is_multithreaded);
#endif

  
  unsigned i;
  numRegisters = deadCount + liveCount;
  registers = new registerSlot[numRegisters];
  spFlag = 1;   // Save SPR (right now for Power) unless we hear different
  numFPRegisters = 0;
  disregardLiveness = false;
  hasXMM = true;

   // load dead ones
   for (i=0; i < deadCount; i++) {
      registers[i].number = dead[i];
      registers[i].refCount = 0;
      registers[i].mustRestore = false;
      registers[i].needsSaving = false;
      registers[i].startsLive = false;
      registers[i].beenClobbered = false;
   }
   
   // load live ones;
   for (i=0; i < liveCount; i++) {
      registers[i+deadCount].number = live[i];
      registers[i+deadCount].refCount = 0;
      registers[i+deadCount].mustRestore = false;
      registers[i+deadCount].needsSaving = true;
      registers[i+deadCount].startsLive = true;
      registers[i+deadCount].beenClobbered = false;
      if(is_multithreaded) {
         if (registers[i+deadCount].number == REG_MT_POS) {
            registers[i+deadCount].refCount = 1;
            registers[i+deadCount].needsSaving = true;
         }
      }
   }
}

registerSpace::~registerSpace()
{
  if (registers)
     delete [] registers;
  if (fpRegisters)
     delete [] fpRegisters; 
}

void registerSpace::initFloatingPointRegisters(const unsigned int count, Register *fp)
{
  unsigned i;
  fpRegisters = new registerSlot[count];
  numFPRegisters = count;
  for (i = 0; i < count; i++)
    {
      fpRegisters[i].needsSaving = true;
      fpRegisters[i].startsLive = true;
      fpRegisters[i].number = fp[i];
      fpRegisters[i].beenClobbered = false;
    }
}

#if !defined(arch_power) && !defined(arch_ia64)
bool registerSpace::allocateSpecificRegister(codeGen &gen, Register reg, 
                                             bool noCost)
#else
bool registerSpace::allocateSpecificRegister(codeGen & /* gen */, Register reg, 
                                             bool /* noCost */)
#endif
{
    if (registers[reg].refCount != 0) {
        return false;
    }

    registers[reg].refCount = 1;
    if (reg > highWaterRegister)
        highWaterRegister = reg;

    if (registers[reg].needsSaving) {
#if !defined(arch_power) && !defined(arch_ia64)
        emitV(saveRegOp, registers[reg].number, 0, 0, gen, noCost);
        registers[reg].mustRestore = true;
#endif
            registers[reg].needsSaving = false;
    }

    return true;
}

#if !defined(rs6000_ibm_aix4_1) \
 && !defined(ia64_unknown_linux2_4) \
 && !defined(arch_x86_64)
Register registerSpace::allocateRegister(codeGen &gen, bool noCost) 
#else
Register registerSpace::allocateRegister(codeGen & /* gen */, bool /* noCost */) 
#endif
{

  unsigned i;

    for (i=0; i < numRegisters; i++) {
	if (registers[i].refCount == 0 && !registers[i].needsSaving) {
	    registers[i].refCount = 1;
	    highWaterRegister = (highWaterRegister > i) ? 
		 highWaterRegister : i;
	    clobberRegister(registers[i].number);
	    return(registers[i].number);
	}
    }

    // now consider ones that need saving
    for (i=0; i < numRegisters; i++) {
      if (registers[i].refCount == 0) {
#if !defined(rs6000_ibm_aix4_1) \
 && !defined(ia64_unknown_linux2_4) \
 && !defined(arch_x86_64)
	// MT_AIX: we are not saving registers on demand on the power
	// architecture. Instead, we save/restore registers in the base
	// trampoline - naim
	// 
	// Same goes for ia64 - rchen
	// And x86_64 - rutar
	emitV(saveRegOp, registers[i].number, 0, 0, gen, noCost);
#endif
	registers[i].refCount = 1;
#if !defined(rs6000_ibm_aix4_1) \
 && !defined(ia64_unknown_linux2_4) \
 && !defined(arch_x86_64)
	// MT_AIX
	registers[i].mustRestore = true;
#endif
	// prevent general spill (func call) from saving this register.
	registers[i].needsSaving = false;
	highWaterRegister = (highWaterRegister > i) ? 
	  highWaterRegister : i;
	clobberRegister(registers[i].number);
	return(registers[i].number);
      }
    }
    
    logLine("==> WARNING! run out of registers...\n");
    abort();
    return(Null_Register);
}

// Free the specified register (decrement its refCount)
void registerSpace::freeRegister(Register reg) 
{
    for (u_int i=0; i < numRegisters; i++) {
       if (registers[i].number == reg) {
          registers[i].refCount--;
#if defined(ia64_unknown_linux2_4)
          if( registers[i].refCount < 0 ) {
             bperr( "Freed free register!\n" );
             registers[i].refCount = 0;
          }
#endif
          return;
       }
    }
}

// Free the register even if its refCount is greater that 1
void registerSpace::forceFreeRegister(Register reg) 
{
   for (u_int i=0; i < numRegisters; i++) {
      if (registers[i].number == reg) {
         registers[i].refCount = 0;
         return;
      }
   }
}

bool registerSpace::isFreeRegister(Register reg) {
    for (u_int i=0; i < numRegisters; i++) {
        if ((registers[i].number == reg) &&
            (registers[i].refCount > 0)) {
            return false;
        }
    }
    return true;
}

// Manually set the reference count of the specified register
// we need to do so when reusing an already-allocated register
void registerSpace::fixRefCount(Register reg, int iRefCount)
{
   for (u_int i=0; i < numRegisters; i++) {
      if (registers[i].number == reg) {
         registers[i].refCount = iRefCount;
         return;
      }
   }
}

// Bump up the reference count. Occasionally, we underestimate it
// and call this routine to correct this.
void registerSpace::incRefCount(Register reg)
{
   for (u_int i=0; i < numRegisters; i++) {
      if (registers[i].number == reg) {
         registers[i].refCount++;
         return;
      }
   }
    // assert(false && "Can't find register");
}

void registerSpace::copyInfo( registerSpace *rs ) const {
   /* Not quite sure why this isn't a copy constructor. */
   rs->numRegisters = this->numRegisters;
   rs->numFPRegisters = this->numFPRegisters;
   rs->highWaterRegister = this->highWaterRegister;
   
   rs->registers = this->numRegisters ? 
      new registerSlot[ this->numRegisters ] :
      NULL;
   rs->fpRegisters = this->numFPRegisters ? 
      new registerSlot[ this->numFPRegisters ] :
      NULL;
   
   rs->is_multithreaded = this->is_multithreaded;
   rs->spFlag = this->spFlag;

#if defined(arch_ia64)
   rs->originalLocals = this->originalLocals;
   rs->originalOutputs = this->originalOutputs;
   rs->originalRotates = this->originalRotates;
   rs->sizeOfStack = this->sizeOfStack;
   memcpy(rs->storageMap, this->storageMap, BP_R_MAX * sizeof(int));
#endif

   /* Duplicate the registerSlot arrays. */
   for( unsigned int i = 0; i < this->numRegisters; i++ ) {
      rs->registers[i].number = registers[i].number;
      rs->registers[i].refCount = registers[i].refCount;
      rs->registers[i].needsSaving = registers[i].needsSaving;
      rs->registers[i].mustRestore = registers[i].mustRestore;
      rs->registers[i].startsLive = registers[i].startsLive;
      rs->registers[i].beenClobbered = registers[i].beenClobbered;
   }
   
   for( unsigned int j = 0; j < this->numFPRegisters; j++ ) {
      rs->fpRegisters[j].number = fpRegisters[j].number;
      rs->fpRegisters[j].refCount = fpRegisters[j].refCount;
      rs->fpRegisters[j].needsSaving = fpRegisters[j].needsSaving;
      rs->fpRegisters[j].mustRestore = fpRegisters[j].mustRestore;
      rs->fpRegisters[j].startsLive = fpRegisters[j].startsLive;
      rs->fpRegisters[j].beenClobbered = fpRegisters[j].beenClobbered;
   }
} /* end registerSpace::copyInfo() */


void registerSpace::setAllLive(){
  for (u_int i = 0; i < numRegisters; i++)
    {
      registers[i].needsSaving = true;
      registers[i].startsLive = true;
    }
  for (u_int i = 0; i < numFPRegisters; i++)
    {
      fpRegisters[i].needsSaving = true;
      fpRegisters[i].startsLive = true;
    }
  spFlag = true;
}


void registerSpace::resetSpace() {
   for (u_int i=0; i < numRegisters; i++) {

      // Drew, do you still want this for anything?  -- TLM ( 03/18/2002 )
      // (Should be #if defined(MT__THREAD) - protected, if you do.)
      //        if (registers[i].inUse && (registers[i].number != REG_MT_POS)) {
      //sprintf(errorLine,"WARNING: register %d is still in use\n",registers[i].number);
      //logLine(errorLine);
      //        }
      
      registers[i].refCount = 0;
      registers[i].mustRestore = false;
      //registers[i].beenClobbered = false;
      registers[i].needsSaving = registers[i].startsLive;
      if(is_multithreaded) {
         if (registers[i].number == REG_MT_POS) {
            registers[i].refCount = 1;
            registers[i].needsSaving = true;
         }
      }
   }
   highWaterRegister = 0;
}


bool registerSpace::isRegStartsLive(Register reg)
{
  return registers[reg].startsLive;
}


int registerSpace::fillDeadRegs(Register * deadRegs, int num)
{
  int filled = 0;
  for (u_int i=0; i < numRegisters && filled < num; i++) {
    if (registers[i].startsLive == false) {
      deadRegs[filled] = registers[i].number;
      filled++;
    }
  }
  return filled;
}


void registerSpace::resetClobbers()
{
  u_int i;
  for (i=0; i < numRegisters; i++)
    {
      registers[i].beenClobbered = false;
    }

  for (i=0; i < numFPRegisters; i++)
    {
      fpRegisters[i].beenClobbered = false;
    }
}


void registerSpace::unClobberRegister(Register reg)
{
  for (u_int i=0; i < numRegisters; i++) {
    if (registers[i].number == reg) {
      registers[i].beenClobbered = false;
    }
  }
}

void registerSpace::unClobberFPRegister(Register reg)
{
  for (u_int i=0; i < numFPRegisters; i++) {
    if (fpRegisters[i].number == reg) {
      fpRegisters[i].beenClobbered = false;
    }
  }
}


// Make sure that no registers remain allocated, except "to_exclude"
// Used for assertion checking.
void registerSpace::checkLeaks(Register to_exclude) 
{
    for (u_int i=0; i<numRegisters; i++) {
	assert(registers[i].refCount == 0 || 
	       registers[i].number == to_exclude);
    }
}

registerSpace &registerSpace::operator=(const registerSpace &src)
{
   src.copyInfo(this);
   return *this;
}

bool registerSpace::getDisregardLiveness() {
    // Let BPatch override, then go to local flag.
    if (BPatch::bpatch->disregardLiveness_NP())
        return true;
    return disregardLiveness;
}

