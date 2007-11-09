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

// $Id: registerSpace.C,v 1.15 2007/11/09 20:11:02 bernat Exp $

#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/instPoint.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/util.h"
#include "dyninstAPI/src/debug.h"

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

registerSpace *registerSpace::globalRegSpace_ = NULL;
registerSpace *registerSpace::conservativeRegSpace_ = NULL;
registerSpace *registerSpace::optimisticRegSpace_ = NULL;
registerSpace *registerSpace::actualRegSpace_ = NULL;
registerSpace *registerSpace::savedRegSpace_ = NULL;
bool registerSpace::hasXMM = false;
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

void registerSlot::cleanSlot() {
    assert(this);
    // Undo anything set during code generation
    refCount = 0;
    // needsSaving...
    mustRestore = false;
    // startsLive...
    beenClobbered = false;
    keptValue = false;
    //offLimits = false;
    //origValueSpilled_ = unspilled;
    //saveOffset_ = 0;	
}
    
void registerSlot::resetSlot() {
	cleanSlot();
	offLimits = false;
	needsSaving = startsLive;
	origValueSpilled_ = unspilled;
	saveOffset_ = 0;
}

registerSpace *registerSpace::conservativeRegSpace(AddressSpace *proc) {
	if (conservativeRegSpace_ == NULL) initRegisters();	
#if defined(arch_x86_64)
   int addrWidth = proc->getAddressWidth();
   registerSpace::conservativeRegSpace_ = (addrWidth == 4) ?
      conservativeRegSpace32 : conservativeRegSpace64;
#endif
	assert(conservativeRegSpace_);
	conservativeRegSpace_->resetSpace();
	return conservativeRegSpace_;	
}

registerSpace *registerSpace::optimisticRegSpace(AddressSpace *proc) {
	if (optimisticRegSpace_ == NULL) initRegisters();
#if defined(arch_x86_64)
   int addrWidth = proc->getAddressWidth();
   registerSpace::optimisticRegSpace_ = (addrWidth == 4) ?
      optimisticRegSpace32 : optimisticRegSpace64;
#endif
	assert(optimisticRegSpace_);
	optimisticRegSpace_->resetSpace();
	return optimisticRegSpace_;	
}

registerSpace *registerSpace::irpcRegSpace(AddressSpace *proc) {
	return conservativeRegSpace(proc);
}

registerSpace *registerSpace::actualRegSpace(instPoint *iP) {
   assert(iP);

	if (actualRegSpace_ == NULL) initRegisters();
#if defined(arch_x86_64)
   actualRegSpace_ =  (iP->proc()->getAddressWidth() == 4) ?
      actualRegSpace32 : actualRegSpace64;
#endif
#if defined(arch_power) || defined(arch_x86_64)
	assert(actualRegSpace_);
	actualRegSpace_->resetLiveDeadInfo(iP->liveGPRegisters(),
                                           iP->liveFPRegisters(),
                                           iP->liveSPRegisters());
	actualRegSpace_->cleanSpace();
	return actualRegSpace_;
#else

	// Use one of the other registerSpaces...
	// If we're entry/exit/call site, return the optimistic version
    if (iP->getPointType() == functionEntry)
        return optimisticRegSpace(iP->proc());
    if (iP->getPointType() == functionExit)
        return optimisticRegSpace(iP->proc());
    if (iP->getPointType() == callSite)
        return optimisticRegSpace(iP->proc());

    return conservativeRegSpace(iP->proc());
#endif

}

registerSpace *registerSpace::savedRegSpace(AddressSpace *proc) {
	if (savedRegSpace_ == NULL) initRegisters();
#if defined(arch_x86_64)
   int addrWidth = proc->getAddressWidth();
   registerSpace::savedRegSpace_ = (addrWidth == 4) ?
      savedRegSpace32 : savedRegSpace64;
#endif
	assert(savedRegSpace_);
	savedRegSpace_->resetSpace();
	return savedRegSpace_;
}

registerSpace *registerSpace::specializeRegisterSpace(Register *deadRegs,
                                                      const unsigned int numDead) {
	assert(globalRegSpace_);
	registerSpace *newRegSpace = new registerSpace(*globalRegSpace_);
	assert(newRegSpace);
	
	// Now set the dead registers
	for (unsigned i = 0; i < numDead; i++) {
            for (unsigned j = 0; j < newRegSpace->registers.size(); j++) {
                if (newRegSpace->registers[j].number == deadRegs[i]) {
                    newRegSpace->registers[j] = registerSlot::deadReg(deadRegs[i]);
                    break;
                }
            }
	}
	return newRegSpace;
}

registerSpace *registerSpace::createAllLive(Register *liveList, unsigned int num) {
	registerSpace *rs = new registerSpace();

  	for (unsigned i=0; i < num; i++) {
    	rs->registers.push_back(registerSlot::liveReg(liveList[i]));
  	}
	return rs;
}

registerSpace *registerSpace::createAllDead(Register *liveList, unsigned int num) {
	registerSpace *rs = new registerSpace();

  	for (unsigned i=0; i < num; i++) {
    	rs->registers.push_back(registerSlot::deadReg(liveList[i]));
  	}
	return rs;
}

registerSpace::registerSpace() :
    currStackPointer(0),
    saveAllGPRs_(unknown), // All these initialize to true for safety.
    saveAllFPRs_(unknown),
    saveAllSPRs_(unknown)
{

#if defined(arch_x86_64)
    initSpecialPurposeRegisters();
#endif
  
  // Assume live unless told otherwise
}

registerSpace::registerSpace(const registerSpace &src) :
	currStackPointer(src.currStackPointer),
        saveAllGPRs_(src.saveAllGPRs_),
        saveAllFPRs_(src.saveAllFPRs_),
        saveAllSPRs_(src.saveAllSPRs_)
{
    for (unsigned i = 0; i < src.registers.size(); i++) {
        registers.push_back(src.registers[i]);
    }
    
    for (unsigned i = 0; i < src.fpRegisters.size(); i++) {
        fpRegisters.push_back(src.fpRegisters[i]);
  }

    for (unsigned i = 0; i < src.spRegisters.size(); i++) {
        spRegisters.push_back(src.spRegisters[i]);
    }

#if defined(arch_ia64)
    originalLocals = src.originalLocals;
    originalOutputs = src.originalOutputs;
    originalRotates = src.originalRotates;
    sizeOfStack = src.sizeOfStack;
    memcpy(storageMap, src.storageMap, BP_R_MAX * sizeof(int));
#endif
    
}

registerSpace::~registerSpace()
{
}

void registerSpace::initFloatingPointRegisters(const unsigned int count, Register *fp)
{
    for (unsigned i = 0; i < count; i++) {
        // All FPs start live!
        fpRegisters.push_back(registerSlot::liveReg(fp[i]));
    }
}

bool registerSpace::allocateSpecificRegister(codeGen &gen, Register reg, 
                                             bool noCost)
{
    unsigned index = registers.size();
    for (unsigned i = 0; i < registers.size(); i++) {
        
        if (registers[i].number == reg) {
            index = i;
            break;
        }
    }
    if (index == registers.size()) return false;

    if (registers[index].offLimits) return false;
    else if (registers[index].refCount > 0) {
		return false;
    }
    else if (registers[index].needsSaving) {
        assert(registers[index].refCount == 0);
        if (!spillRegister(index, gen, noCost)) {
            return false;
        }
    }
    else if (registers[index].keptValue) {
        if (!stealRegister(index, gen, noCost)) return false;
    }

    // So... we've either got a clear register, or we spilled it,
    // or we stole it. Nice. 
    assert(registers[index].refCount == 0);
    assert(registers[index].needsSaving == false);
    
    registers[index].refCount = 1;
    registers[index].beenClobbered = true;
    regalloc_printf("Allocated register %d\n", registers[index].number);

    return true;
}

Register registerSpace::getScratchRegister(codeGen &gen, bool noCost) {
    pdvector<Register> empty;
    return getScratchRegister(gen, empty, noCost);
}

Register registerSpace::getScratchRegister(codeGen &gen, pdvector<Register> &excluded, bool noCost) {
    unsigned toUse = registers.size(); 
    unsigned scratchIndex = 0;

    regalloc_printf("Allocating scratch register...\n");
    for (unsigned i=0; i < registers.size(); i++) {
        regalloc_printf("%d: reg %d, refCount %d, needsSaving %d, keptValue %d, offLimits %d\n",
                        i, registers[i].number,
                        registers[i].refCount,
                        registers[i].needsSaving,
                        registers[i].keptValue,
                        registers[i].offLimits);
    }
    
    for (unsigned i=0; i < registers.size(); i++) {
        if (find(excluded, registers[i].number, scratchIndex)) continue;
        if (registers[i].offLimits) continue;
        if (registers[i].needsSaving) continue;
		if (registers[i].keptValue) continue;
        if (registers[i].refCount == 0) {
            toUse = i; 
            break;
        }
    }
    
	// TODO: spill vs. recalculate?
    if (toUse == registers.size()) {
        for (unsigned i = 0; i < registers.size(); i++) {
            // Okay, time to look for a spillable one...
            if (find(excluded, registers[i].number, scratchIndex)) continue;
            if (registers[i].offLimits) continue;
            if (registers[i].refCount > 0) continue;
            if (registers[i].keptValue) continue;
            assert(registers[i].needsSaving);
            // We're going to spill... this is bad for testing
            debugPrint();
            if (spillRegister(i, gen, noCost)) {
                toUse = i;
                break;
            }
        }
    }
    if (toUse == registers.size()) {
        // Getting desperate...
        for (unsigned i = 0; i < registers.size(); i++) {
            if (find(excluded, registers[i].number, scratchIndex)) continue;
            if (registers[i].offLimits) continue;
            if (registers[i].refCount > 0) continue;
            if (registers[i].keptValue) {
                // Kick him out. TODO: priority mechanism. 
                if (stealRegister(i, gen, noCost)) {
                    toUse = i;
                    break;
                }
            }
        }
    }
    
    if (toUse == registers.size()) {
        // Crap.
        debugPrint();
        assert(0 && "Failed to allocate register!");
        return REG_NULL;
    }
    
    assert(registers[toUse].refCount == 0);
    assert(registers[toUse].needsSaving == false);
    registers[toUse].beenClobbered = true;
	regalloc_printf("Returning register %d\n", registers[toUse].number);
    return registers[toUse].number;
}

Register registerSpace::allocateRegister(codeGen &gen, 
                                         bool noCost) 
{
    regalloc_printf("Allocating and keeping register...\n");
    Register reg = getScratchRegister(gen, noCost);
    regalloc_printf("Keeping register %d\n", reg);
    if (reg == REG_NULL) return REG_NULL;

    // Argh stupid iteration
    for (unsigned i = 0; i < registers.size(); i++) {
        if (registers[i].number == reg) { 
            registers[i].refCount = 1;
            regalloc_printf("Allocated register %d\n", registers[i].number);
            return(registers[i].number);
        }
    }
	assert(0); // This means we got a register, but couldn't find it again.
	return REG_NULL;
}

bool registerSpace::spillRegister(unsigned index, codeGen &gen, bool noCost) {
    assert(index < registers.size());
    assert(!registers[index].offLimits);

    // TODO for other platforms that build a stack frame for saving

    regalloc_printf("Spilling register %d\n", registers[index].number);

    emitV(saveRegOp, registers[index].number, 0, 0, gen, noCost);
    
    registers[index].mustRestore = true; // Need to restore later
    registers[index].needsSaving = false; // And don't save at func call (?)
    registers[index].origValueSpilled_ = registerSlot::stackPointer;
    registers[index].saveOffset_ = ++currStackPointer;

    return true;
}

bool registerSpace::stealRegister(unsigned index, codeGen &gen, bool /*noCost*/) {
    // Can be made a return false; this for correctness.
    assert(registers[index].refCount == 0);
    assert(registers[index].needsSaving == false);
	assert(registers[index].keptValue == true);

    // Let the AST know it just lost...
	if (!gen.tracker()->stealKeptRegister(registers[index].number)) return false;
	registers[index].keptValue = false;

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
		gen.codeEmitter()->emitPushFlags(gen);
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
        gen.codeEmitter()->emitRestoreFlags(gen, difference);
        if (difference == 0) {
            // This is a little annoying... emitRestoreFlags will just pop
            // if the difference is 0, which means our stack just moved. Oy.
            // This should be encapsulated....
            currStackPointer--;
        }
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

    for (u_int i=0; i < registers.size(); i++) {
       if (registers[i].number == reg) {
          registers[i].refCount--;
          regalloc_printf("Freed register %d: refcount now %d\n", registers[i].number, registers[i].refCount);
          if( registers[i].refCount < 0 ) {
             bperr( "Freed free register!\n" );
             registers[i].refCount = 0;
          }
          return;
       }
    }
    // Hrm... let's squawk.
#if !defined(arch_ia64)
    // IA-64 gets this with the (actual) frame pointer...
    fprintf(stderr, "[%s:%d] WARNING: attempt to free unknown register %u\n", __FILE__, __LINE__, reg);
#endif
}

// Free the register even if its refCount is greater that 1
void registerSpace::forceFreeRegister(Register reg) 
{
   for (u_int i=0; i < registers.size(); i++) {
      if (registers[i].number == reg) {
         registers[i].refCount = 0;
         return;
      }
   }
}

bool registerSpace::isFreeRegister(Register reg) {
    for (u_int i=0; i < registers.size(); i++) {
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
   for (u_int i=0; i < registers.size(); i++) {
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
   for (u_int i=0; i < registers.size(); i++) {
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

   rs->registers = registers;
   rs->fpRegisters = fpRegisters;
   rs->spRegisters = spRegisters;
   rs->currStackPointer = currStackPointer;

   rs->saveAllGPRs_ = saveAllGPRs_;
   rs->saveAllFPRs_ = saveAllFPRs_;
   rs->saveAllSPRs_ = saveAllSPRs_;

#if defined(arch_ia64)
   rs->originalLocals = this->originalLocals;
   rs->originalOutputs = this->originalOutputs;
   rs->originalRotates = this->originalRotates;
   rs->sizeOfStack = this->sizeOfStack;
   memcpy(rs->storageMap, this->storageMap, BP_R_MAX * sizeof(int));
#endif

} /* end registerSpace::copyInfo() */


void registerSpace::resetSpace() {
    assert(this);
    regalloc_printf("============== RESET %p ==============\n", this);
    for (u_int i=0; i < registers.size(); i++) {
        registers[i].resetSlot();
    }
    saveAllGPRs_ = unknown;
    saveAllFPRs_ = unknown;
    saveAllSPRs_ = unknown;
}

void registerSpace::cleanSpace() {
    regalloc_printf("============== CLEAN ==============\n");
    for (u_int i=0; i < registers.size(); i++) {
        registers[i].cleanSlot();
   }
}


bool registerSpace::isRegStartsLive(Register reg)
{
  return registers[reg].startsLive;
}


int registerSpace::fillDeadRegs(Register * deadRegs, int num)
{
  int filled = 0;
  for (u_int i=0; i < registers.size() && filled < num; i++) {
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
  for (i=0; i < registers.size(); i++)
    {
      registers[i].beenClobbered = false;
    }

  for (i=0; i < fpRegisters.size(); i++)
    {
      fpRegisters[i].beenClobbered = false;
    }
}


void registerSpace::unClobberRegister(Register reg)
{
  for (u_int i=0; i < registers.size(); i++) {
    if (registers[i].number == reg) {
      registers[i].beenClobbered = false;
    }
  }
}

void registerSpace::unClobberFPRegister(Register reg)
{
  for (u_int i=0; i < fpRegisters.size(); i++) {
    if (fpRegisters[i].number == reg) {
      fpRegisters[i].beenClobbered = false;
    }
  }
}


// Make sure that no registers remain allocated, except "to_exclude"
// Used for assertion checking.
void registerSpace::checkLeaks(Register to_exclude) 
{
    for (u_int i=0; i<registers.size(); i++) {
	assert(registers[i].refCount == 0 || 
	       registers[i].number == to_exclude);
    }
}

registerSpace &registerSpace::operator=(const registerSpace &src)
{
    saveAllGPRs_ = src.saveAllGPRs_;
    saveAllFPRs_ = src.saveAllFPRs_;
    saveAllSPRs_ = src.saveAllSPRs_;
    currStackPointer = src.currStackPointer;

    for (unsigned i = 0; i < src.registers.size(); i++) {
        registers.push_back(src.registers[i]);
    }

    for (unsigned i = 0; i < src.fpRegisters.size(); i++) {
        fpRegisters.push_back(src.fpRegisters[i]);
    }

    for (unsigned i = 0; i < src.spRegisters.size(); i++) {
        spRegisters.push_back(src.spRegisters[i]);
    }

#if defined(arch_ia64)
    originalLocals = src.originalLocals;
    originalOutputs = src.originalOutputs;
    originalRotates = src.originalRotates;
    sizeOfStack = src.sizeOfStack;
    memcpy(storageMap, src.storageMap, BP_R_MAX * sizeof(int));
#endif
    

    return *this;
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

	int *pushedReg = (int *) malloc(sizeof(int) * numPushed);
	for (int i = 0; i < numPushed; i++) pushedReg[i] = -1;

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
	for (int i = 0; i < numPushed; i++) {
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
		gen.codeEmitter()->emitAdjustStackPointer(currStackPointer, gen);
	}
#endif
    currStackPointer = 0;

	free(pushedReg);
    return true;
}

bool registerSpace::restoreRegister(unsigned index, codeGen &gen, bool /*noCost*/) 
{
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
            gen.codeEmitter()->emitMoveRegToReg(source, destination, gen);
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
        gen.codeEmitter()->emitLoadRelative(destination, s->saveOffset_, REGNUM_RSP, gen);
        return true;
    }
    else if (s->origValueSpilled_ == registerSlot::framePointer) {
        // We can't use existing mechanisms because they're all built
        // off the "non-instrumented" case - emit a load from the
        // "original" frame pointer, whereas we want the current one. 
        gen.codeEmitter()->emitLoadRelative(destination, s->saveOffset_, REGNUM_RBP, gen);
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
        // We get this on platforms where we save registers we don't use in
        // code generation... like, say, RSP or RBP on AMD-64.
        //fprintf(stderr, "ERROR: unable to find register %d\n", num);
        return false;
    }

    if (s->origValueSpilled_ != registerSlot::unspilled) {
        // Things to do... add this check in, yo. Right now we don't clean
        // properly.
        //assert(0);
    }

    s->needsSaving = false;
    // We used an alternative storage method - so to us it's dead.
    s->mustRestore = false;

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
			registers.size(), registers.size(), fpRegisters.size(), fpRegisters.size(), spRegisters.size());
	fprintf(stderr, "Stack pointer is at %d, saveAll: %d/%d/%d\n",
                currStackPointer, 
                saveAllGPRs_,
                saveAllFPRs_,
                saveAllSPRs_);
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


void registerSpace::printAllocedRegisters() {
    for (unsigned i = 0; i < registers.size(); i++) {
        if (registers[i].refCount > 0)
            fprintf(stderr, "Register %d is in use (%d references)\n",
                    registers[i].number, registers[i].refCount);
		if (registers[i].keptValue)
			fprintf(stderr, "Register %d contains a kept value\n", registers[i].number);
    }
}

bool registerSpace::markKeptRegister(Register reg) {
	regalloc_printf("Marking register %d as kept\n", reg);
	registerSlot *r = findRegister(reg);
	assert(r);
	r->keptValue = true;
	return true;
}

void registerSpace::unKeepRegister(Register reg) {
	regalloc_printf("Marking register %d as unkept\n", reg);
	registerSlot *r = findRegister(reg);
	assert(r);
	r->keptValue = false;
}

bool registerSpace::clobberAllRegisters() {
    for (unsigned i = 0; i < registers.size(); i++) {
        registers[i].beenClobbered = true;
    }
    for (unsigned i = 0; i < fpRegisters.size(); i++) {
        fpRegisters[i].beenClobbered = true;
    }
    return true;
}

bool registerSpace::saveAllGPRs() const {
// The value of saveAllGPRs_ can be one of five things:
// unknown: have not analyzed, so return true (save all)
// killed: our instrumentation uses them, so return true
// unused: our instrumentation doesn't touch, ret false
// live: live at this point, ret true
// dead: dead at this point, ret false

// Really, we have a two-dimensional system; but we can't represent that
// with one variable.
	if (saveAllGPRs_ == unknown) return true;
	if (saveAllGPRs_ == killed) return true;
	if (saveAllGPRs_ == live) return true;
	return false;
}

bool registerSpace::saveAllFPRs() const {
// The value of saveAllFPRs_ can be one of five things:
// unknown: have not analyzed, so return true (save all)
// killed: our instrumentation uses them, so return true
// unused: our instrumentation doesn't touch, ret false
// live: live at this point, ret true
// dead: dead at this point, ret false
// Really, we have a two-dimensional system; but we can't represent that
// with one variable.
    if (saveAllFPRs_ == unknown) return true;
    if (saveAllFPRs_ == killed) return true;
    if (saveAllFPRs_ == live) return true;
    return false;
}

bool registerSpace::saveAllSPRs() const {
// The value of saveAllSPRs_ can be one of five things:
// unknown: have not analyzed, so return true (save all)
// killed: our instrumentation uses them, so return true
// unused: our instrumentation doesn't touch, ret false
// live: live at this point, ret true
// dead: dead at this point, ret false

// Really, we have a two-dimensional system; but we can't represent that
// with one variable.
	if (saveAllSPRs_ == unknown) return true;
	if (saveAllSPRs_ == killed) return true;
	if (saveAllSPRs_ == live) return true;
	return false;
}

