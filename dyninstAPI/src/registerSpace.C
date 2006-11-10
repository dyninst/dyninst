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

// $Id: registerSpace.C,v 1.4 2006/11/10 16:28:52 bernat Exp $

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
#include "dyninstAPI/src/emit-x86.h"

#elif defined(ia64_unknown_linux2_4) /* Why is this done here, instead of, e.g., inst.h? */
#include "dyninstAPI/src/inst-ia64.h"
#endif

#include "showerror.h"

extern registerSpace *regSpace;

/*
  registerSpace *registerSpace::regSpace() {
    return regSpace;
}
*/
registerSlot registerSlot::deadReg(Register i) {
    registerSlot retval;
    // Override number
    retval.number = i;
    // Everything else is okay.
    return retval;
}

registerSlot registerSlot::liveReg(Register i) {
    registerSlot retval;
    retval.number = i;
    // Override needsSaving and startsLive
    retval.startsLive = true;
    retval.needsSaving = true;
    return retval;
}

registerSlot registerSlot::thrIndexReg(Register i) {
    registerSlot retval = liveReg(i);
    // Implicit refCount so we don't allocate it elsewhere
    retval.refCount = 1;
    return retval;
}
    

registerSpace::registerSpace(const unsigned int deadCount, Register *dead, 
                             const unsigned int liveCount, Register *live,
                             bool multithreaded) :
    currStackPointer(0),
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
  flagsID = numRegisters + 1;
  spFlag = 1;   // Save SPR (right now for Power) unless we hear different
  numFPRegisters = 0;
  disregardLiveness = false;
  hasXMM = true;

  registerSlot deadReg;

  // load dead ones
  for (i=0; i < deadCount; i++) {
      registers.push_back(registerSlot::deadReg(dead[i]));
  }
  
   // load live ones;
  for (i=0; i < liveCount; i++) {
      
      if(is_multithreaded && 
         (live[i] == REG_MT_POS)) {
          registers.push_back(registerSlot::thrIndexReg(live[i]));
      }
      else {
          registers.push_back(registerSlot::liveReg(live[i]));
      }
  }
}

registerSpace::~registerSpace()
{
}

void registerSpace::initFloatingPointRegisters(const unsigned int count, Register *fp)
{
    numFPRegisters = count;
    for (unsigned i = 0; i < count; i++) {
        // All FPs start live!
        fpRegisters.push_back(registerSlot::liveReg(fp[i]));
    }
}

bool registerSpace::allocateSpecificRegister(codeGen &gen, Register reg, 
                                             bool noCost)
{
    for (unsigned i = 0; i < registers.size(); i++) {
        if (registers[i].number == reg) {
            if (registers[i].offLimits) return false;
            if (registers[i].refCount != 0) return false;
            
            registers[i].refCount = 1;
            
            if (registers[i].needsSaving) {
                spillRegister(i, gen, noCost);
            }
            return true;
        }
    }
    return true;
}

Register registerSpace::getScratchRegister(codeGen &gen, bool noCost) {
  unsigned i;
  unsigned spareReg = registers.size();
  
  for (i=0; i < numRegisters; i++) {
      if (registers[i].refCount == 0 && (!registers[i].offLimits)) {
          // Optimization: if this one needs saving, but we can use it, stick it
          // in spareReg for later.
          if (registers[i].needsSaving) {
              spareReg = i;
          }
          else {
              regalloc_printf("Returning scratch register %d\n", registers[i].number);
              return(registers[i].number);
          }
      }
  }
  
  if (spareReg != registers.size()) {
      if (spillRegister(spareReg, gen, noCost)) {
          return registers[spareReg].number;
      }
  }
  
  assert(0);
  return Null_Register;
}

Register registerSpace::allocateRegister(codeGen &gen, bool noCost) 
{

  // Technically, we could have failed to spill this guy... but I
  // assume in that case we can't spill anything.
  
  // DEBUG

  unsigned i;
  unsigned spareReg = registers.size();
  
  for (i=0; i < numRegisters; i++) {
      if (registers[i].refCount == 0 && (!registers[i].offLimits)) {
          // Optimization: if this one needs saving, but we can use it, stick it
          // in spareReg for later.
          if (registers[i].needsSaving) {
              spareReg = i;
          }
          else {
              registers[i].refCount = 1;
              clobberRegister(registers[i].number);
              regalloc_printf("Allocated register %d\n", registers[i].number);
              return(registers[i].number);
          }
      }
  }
  
  if (spareReg != registers.size()) {
      if (spillRegister(spareReg, gen, noCost)) {
          registers[spareReg].refCount = 1; // We're using it...
          clobberRegister(registers[spareReg].number);
          return registers[spareReg].number;
      }
  }
 

  logLine("==> WARNING! run out of registers...\n");
  debugPrint();
  abort();
  return(Null_Register);
}

bool registerSpace::spillRegister(unsigned index, codeGen &gen, bool noCost) {
    assert(index < registers.size());
    assert(!registers[index].offLimits);

    // TODO for other platforms that build a stack frame for saving

    emitV(saveRegOp, registers[index].number, 0, 0, gen, noCost);
    
    registers[index].mustRestore = true; // Need to restore later
    registers[index].needsSaving = false; // And don't save at func call (?)
    registers[index].origValueSpilled_ = registerSlot::stackPointer;
    registers[index].saveOffset_ = ++currStackPointer;

    return true;
}

// This might mean something different later, but for now means
// "Save special purpose registers". We may want to define a "volatile"
// later - something like "can be unintentionally nuked". For example,
// x86 flags register. 
bool registerSpace::saveVolatileRegisters(codeGen &gen) {
    // Two things: first, save the flags register(s)
    // Second: add this to the save list for order-important platforms.

	assert(spRegisters.size());

#if defined(arch_x86) || defined(arch_x86_64)
	// Wave of the future, man....
	assert(spRegisters.size() == 1);
	for (unsigned i = 0; i < spRegisters.size(); i++) {
		// Should decide whether to push or store in a frame...
		code_emitter->emitPushFlags(gen);
		spRegisters[i].origValueSpilled_ = registerSlot::stackPointer;
		spRegisters[i].saveOffset_ = ++currStackPointer;
	}
#else
	assert(0 && "Unimplemented");
#endif
    return true;
}

bool registerSpace::restoreVolatileRegisters(codeGen &gen) {
    // Okay, we need to figure out where we stuck the flags
    // register. Assume other things have been saved since. We go
    // through the saveOrder vector and pull out how far we are from
    // the end.
    assert(spRegisters.size());
    
    for (unsigned i = 0; i < spRegisters.size(); i++) {
        registerSlot &reg = spRegisters[i];
        if (reg.origValueSpilled_ == registerSlot::unspilled) continue;

        // TODO: find out where it was pushed compared to current, then emit code to
        // restore it (possibly higher up the stack)
        assert (reg.saveOffset_ <= currStackPointer);
        int difference = currStackPointer - reg.saveOffset_;
#if defined(arch_x86) || defined(arch_x86_64)
        code_emitter->emitRestoreFlags(gen, difference);
#else
        assert(0 && "Unimplemented");
#endif

        reg.origValueSpilled_ = registerSlot::unspilled;
        reg.saveOffset_ = 0;
    }
    return true;
}


// Free the specified register (decrement its refCount)
void registerSpace::freeRegister(Register reg) 
{
    if (reg == REG_NULL) {
        // This is okay - it's easier to unconditionally free
        // even if the guy didn't allocate. So just return.
        return;
    }

    for (u_int i=0; i < numRegisters; i++) {
       if (registers[i].number == reg) {
          registers[i].refCount--;
          regalloc_printf("Freed register %d: refcount now %d\n", registers[i].number, registers[i].refCount);
#if defined(ia64_unknown_linux2_4)
          if( registers[i].refCount < 0 ) {
             bperr( "Freed free register!\n" );
             registers[i].refCount = 0;
          }
#endif
          return;
       }
    }
    // Hrm... let's squawk.
    fprintf(stderr, "[%s:%d] WARNING: attempt to free unknown register %u\n", __FILE__, __LINE__, reg);
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
         regalloc_printf("Incrementing refcount on register %d: %d\n", registers[i].number, 
                         registers[i].refCount);
         return;
      }
   }
    // assert(false && "Can't find register");
}

void registerSpace::copyInfo( registerSpace *rs ) const {
   /* Not quite sure why this isn't a copy constructor. */
   rs->numRegisters = this->numRegisters;
   rs->numFPRegisters = this->numFPRegisters;
   rs->flagsID = this->flagsID;

   rs->registers = registers;
   rs->fpRegisters = fpRegisters;
   rs->spRegisters = spRegisters;
   rs->currStackPointer = currStackPointer;

   rs->is_multithreaded = this->is_multithreaded;
   rs->spFlag = this->spFlag;

#if defined(arch_ia64)
   rs->originalLocals = this->originalLocals;
   rs->originalOutputs = this->originalOutputs;
   rs->originalRotates = this->originalRotates;
   rs->sizeOfStack = this->sizeOfStack;
   memcpy(rs->storageMap, this->storageMap, BP_R_MAX * sizeof(int));
#endif

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
    regalloc_printf("============== RESET %p ==============\n", this);
   for (u_int i=0; i < registers.size(); i++) {

      // Drew, do you still want this for anything?  -- TLM ( 03/18/2002 )
      // (Should be #if defined(MT__THREAD) - protected, if you do.)
      //        if (registers[i].inUse && (registers[i].number != REG_MT_POS)) {
      //sprintf(errorLine,"WARNING: register %d is still in use\n",registers[i].number);
      //logLine(errorLine);
      //        }
      
      registers[i].refCount = 0;
      registers[i].mustRestore = false;
      registers[i].offLimits = false;
      //registers[i].beenClobbered = false;
      registers[i].needsSaving = registers[i].startsLive;
      if(is_multithreaded) {
          if (registers[i].number == REG_MT_POS) {
              registers[i].refCount = 1;
              registers[i].needsSaving = true;
          }
      }

      registers[i].origValueSpilled_ = registerSlot::unspilled;
      registers[i].saveOffset_ = 0;

	  registers[i].offLimits = false;

   }
}

void registerSpace::cleanSpace() {
    regalloc_printf("============== CLEAN ==============\n");
   for (u_int i=0; i < registers.size(); i++) {
      registers[i].refCount = 0;
      if(is_multithreaded) {
          if (registers[i].number == REG_MT_POS) {
              registers[i].refCount = 1;
              registers[i].needsSaving = true;
          }
      }
   }
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

bool registerSpace::restoreAllRegisters(codeGen &gen, bool noCost) {
	// This will only restore "saved" registers, not "spilled" registers.
	// We should do both, but the individual "restoreRegister" can't build
	// the list necessary for popping registers...

	// We have some registers saved off the frame pointer, some saved
	// on the stack, and some unsaved. We want to go over the list
	// of registers and build up the spill list (in order so we can pop
	// em), then pop all and then spill from the frame pointer.
	
	int numPushed = currStackPointer;

	int pushedReg[numPushed];
	for (unsigned i = 0; i < numPushed; i++) pushedReg[i] = -1;

	for (unsigned i = 0; i < registers.size(); i++) {
		// Each register may be:
		switch(registers[i].origValueSpilled_) {
			case registerSlot::unspilled:
				break;
			case registerSlot::stackPointer: {
                            assert(registers[i].saveOffset_ <= numPushed);
                            pushedReg[registers[i].saveOffset_] = i;
                            break;
			}
			case registerSlot::framePointer:
				restoreRegister(GPRindex(i), gen, noCost);
				break;
			default:
				assert(0);
				break;
		}
	}
	for (unsigned i = 0; i < numPushed; i++) {
		if (pushedReg[i] != -1)
			popRegister(pushedReg[i], gen, noCost);
	}

    for (unsigned i = 0; i < registers.size(); i++) {
        restoreRegister(GPRindex(i), gen, noCost);
    }

	// Oh, right. FP and SPR. :)
    for (unsigned i = 0; i < fpRegisters.size(); i++) {
        restoreRegister(FPRindex(i), gen, noCost);
    }
    for (unsigned i = 0; i < spRegisters.size(); i++) {
        restoreRegister(SPRindex(i), gen, noCost);
    }

#if defined(arch_x86) || defined(arch_x86_64)
	if (currStackPointer > 0) {
		code_emitter->emitAdjustStackPointer(currStackPointer, gen);
	}
#endif
    currStackPointer = 0;

    return true;
}

bool registerSpace::restoreRegister(unsigned index, codeGen &gen, bool noCost) {
    // We can get an index > than the number of registers - we use those as fake
    // slots for (e.g.) flags register.
    // TODO: push register info and methods into a register slot class... hey....

    if (index >= registers.size()) 
        return true; // Don't do FPRs or SPRs yet

	if (registers[index].origValueSpilled_ == registerSlot::unspilled) return true;
	if (registers[index].origValueSpilled_ == registerSlot::stackPointer) {
		// We don't know how to handle this in an individual restore yet...
		return false;
	}
    
    if (!readRegister(gen, registers[index].number, registers[index].number))
        return false;

    registers[index].mustRestore = false;
    registers[index].needsSaving = true;
    registers[index].origValueSpilled_ = registerSlot::unspilled;
    
    return true;
}

bool registerSpace::popRegister(unsigned index, codeGen &gen, bool noCost) {
    assert(index < registers.size());
    assert(!registers[index].offLimits);

	// Make sure we're at the right point... currStackPointer should
	// be 1 greater than the register.
	while (currStackPointer > (registers[index].saveOffset_+1)) {
		emitV(loadRegOp, 0, 0, registers[index].number, gen, noCost);
		currStackPointer--;
	}

	assert((currStackPointer == (registers[index].saveOffset_-1)) ||
		   (currStackPointer == (registers[index].saveOffset_+1)));

    // TODO for other platforms that build a stack frame for saving

    emitV(loadRegOp, 0, 0, registers[index].number, gen, noCost);
    
    registers[index].mustRestore = false; // Need to restore later
    registers[index].needsSaving = true; // And don't save at func call (?)
    registers[index].origValueSpilled_ = registerSlot::unspilled;
    registers[index].saveOffset_ = 0;

    // Push architecture, so popping modified the SP
    currStackPointer--;

    return true;
}


bool registerSpace::readOnlyRegister(Register num) {
    // This was apparently useful on Alpha...

    for (unsigned i = 0; i < registers.size(); i++) {
        if (registers[i].number == num) {
            return registers[i].offLimits;
        }
    }
    return false;
}

bool registerSpace::markReadOnly(Register num) {
    for (unsigned i = 0; i < registers.size(); i++) {
        if (registers[i].number == num) {
            registers[i].offLimits = true;
            return true;
        }
    }
    return false;
}

bool registerSpace::readRegister(codeGen &gen, 
                                 Register source,
                                 Register destination) {
#if defined(arch_x86) || defined(arch_x86_64)

    // First step: identify the registerSlot that contains information
    // about the source register.

    registerSlot *s = findRegister(source);
    if (s == NULL) {
        fprintf(stderr, "No information about register %d!\n", source);
        return false;
    }

    // Okay. We need to know where the register is compared with our
    // current location vis-a-vis the stack pointer. Now, on most
    // platforms this doesn't matter, as the SP never moves. Well, not
    // so x86. 


    // Option 1: still alive
    if (s->origValueSpilled_ == registerSlot::unspilled) {
        if (source != destination)
            code_emitter->emitMoveRegToReg(source, destination, gen);
        return true;
    }
    else if (s->origValueSpilled_ == registerSlot::stackPointer) {
        // Emit load from stack pointer...
        // The math here is straightforward: saveOffset_ in the
        // registerSlot is the offset where the guy was stored (from
        // theoretical stack pointer 0), and currStackPointer is the
        // offset of the stack pointer from that 0. So the actual
        // displacement is (saveOffset_ - currStackPointer). 
        int offset = s->saveOffset_ - currStackPointer;
        if (offset < 0) { 
            // Uhh... below the stack pointer?
            fprintf(stderr, "WARNING: weird math in readRegister, offset %d and SP %d\n", s->saveOffset_, currStackPointer);
        }
        code_emitter->emitLoadRelative(destination, s->saveOffset_, REGNUM_RSP, gen);
        return true;
    }
    else if (s->origValueSpilled_ == registerSlot::framePointer) {
        // We can't use existing mechanisms because they're all built
        // off the "non-instrumented" case - emit a load from the
        // "original" frame pointer, whereas we want the current one. 
        code_emitter->emitLoadRelative(destination, s->saveOffset_, REGNUM_RBP, gen);
        return true;
    }

    // Can't be reached
    assert(0);
    return false;
#else
    assert(0 && "Unimplemented");
    return false;
#endif
}

registerSlot *registerSpace::findRegister(Register source) {
    // Oh, oops... we're handed a register number... and we can't tell if it's
    // GPR, FPR, or SPR...
    for (unsigned i = 0; i < registers.size(); i++) {
        if (registers[i].number == source) {
            return &(registers[i]);
        }
    }

#if 0
    } else if (source < (registers.size() + fpRegisters.size())) {
        for (unsigned i = 0; i < fpRegisters.size(); i++) {
            if (fpRegisters[i].number == source) {
                return &(fpRegisters[i]);
            }
        }
    }        
    else if (source < (registers.size() + fpRegisters.size() + spRegisters.size())) {
        for (unsigned i = 0; i < spRegisters.size(); i++) {
            if (spRegisters[i].number == source) {
                return &(spRegisters[i]);
            }
        }
    }        
#endif
    // DEBUG
//debugPrint();

    return NULL;
}

bool registerSpace::markSavedRegister(Register num, int offsetFromFP) {
    regalloc_printf("Marking register %d as saved, %d from frame pointer\n",
                    num, offsetFromFP);
    // Find the register slot
    registerSlot *s = findRegister(num);
    if (s == NULL)  {
        fprintf(stderr, "ERROR: unable to find register %d\n", num);
        return false;
    }

    if (s->origValueSpilled_ != registerSlot::unspilled) {
        // Huh?
        assert(0);
    }

    s->needsSaving = false;
    // We used an alternative storage method - so to us it's dead.
    s->mustRestore = false;
    s->startsLive = false;

    s->origValueSpilled_ = registerSlot::framePointer;
    s->saveOffset_ = offsetFromFP;
    return true;
}



// If x86...
#if defined(arch_x86) || defined(arch_x86_64) 
void registerSpace::initSpecialPurposeRegisters() {
    // We only have one: flags register. Technically we should push
    // the save/restore logic into here too. For now, we just have 1. 
    
    registerSlot sprReg;
    sprReg.number = 1;
    sprReg.offLimits = true;
    spRegisters.push_back(sprReg);
}
#endif

void registerSlot::debugPrint(char *prefix) {
	if (prefix) fprintf(stderr, "%s", prefix);
	fprintf(stderr, "Num: %d, ref %d, needsSaving %d, mustRestore %d, startsLive %d\n", 
			number, refCount, needsSaving, mustRestore, startsLive);
	if (prefix) fprintf(stderr, "%s", prefix);
	fprintf(stderr, "beenClobbered %d, offLimits %d, origVSpilled %d, saveOffset %d, type %d\n",
			beenClobbered, offLimits, origValueSpilled_, saveOffset_, type_);	
}

void registerSpace::debugPrint() {
	// Dump out our data
	fprintf(stderr, "Beginning debug print of registerSpace at %p...", this);
	fprintf(stderr, "GPRs: %d (%d), FPRs: %d (%d), SPRs: (%d)\n", 
			numRegisters, registers.size(), numFPRegisters, fpRegisters.size(), spRegisters.size());
	fprintf(stderr, "Stack pointer is at %d, spFlag %d, disregardLiveness %d, is_multithreaded %d",
			currStackPointer, spFlag, disregardLiveness, is_multithreaded);
	fprintf(stderr, "Register dump:");
	fprintf(stderr, "=====GPRs=====\n");
	for (unsigned i = 0; i < registers.size(); i++) {
		registers[i].debugPrint("\t");
	}
	fprintf(stderr, "=====FPRs=====\n");
	for (unsigned i = 0; i < fpRegisters.size(); i++) {
		fpRegisters[i].debugPrint("\t");
	}
	fprintf(stderr, "=====SPRs=====\n");
	for (unsigned i = 0; i < spRegisters.size(); i++) {
		spRegisters[i].debugPrint("\t");
	}
}
