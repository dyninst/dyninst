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

// $Id: registerSpace.C,v 1.25 2008/10/27 17:23:53 mlam Exp $

#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/instPoint.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/util.h"
#include "dyninstAPI/src/debug.h"

#include "dyninstAPI/src/registerSpace.h"

#include "dyninstAPI/h/BPatch.h"
#include "dyninstAPI/src/BPatch_collections.h"
#include "dyninstAPI/h/BPatch_type.h"
#include "dyninstAPI/src/BPatch_libInfo.h" // For instPoint->BPatch_point mapping

#include "dyninstAPI/h/BPatch_point.h"
#include "dyninstAPI/h/BPatch_memoryAccess_NP.h"

#include <map>

#if defined(arch_sparc)
#include "dyninstAPI/src/inst-sparc.h"
#elif defined(arch_power)
#include "dyninstAPI/src/inst-power.h"
#include "dyninstAPI/src/emit-power.h"
#elif defined(arch_x86) || defined(arch_x86_64)
#include "dyninstAPI/src/inst-x86.h"
#include "dyninstAPI/src/emit-x86.h"
#endif

registerSpace *registerSpace::globalRegSpace_ = NULL;
registerSpace *registerSpace::globalRegSpace64_ = NULL;

#if defined(cap_liveness)
bitArray registerSpace::callRead_;
bitArray registerSpace::callWritten_;
bitArray registerSpace::returnRead_;
bitArray registerSpace::syscallRead_;
bitArray registerSpace::syscallWritten_;

bitArray registerSpace::callRead64_;
bitArray registerSpace::callWritten64_;
bitArray registerSpace::returnRead64_;
bitArray registerSpace::syscallRead64_;
bitArray registerSpace::syscallWritten64_;
bitArray registerSpace::allRegs_;
bitArray registerSpace::allRegs64_;
#endif

bool registerSpace::hasXMM = false;

void registerSlot::cleanSlot() {
    assert(this);
    // number does not change
    refCount = 0;
    //liveState = live;
    //liveState does not change
    keptValue = false;
    beenUsed = false;
    // initialState doesn't change
    // offLimits doesn't change
    spilledState = unspilled;
    saveOffset = 0;
    // type doesn't change
}

unsigned registerSlot::encoding() const {
    // Should write this for all platforms when the encoding is done.
#if defined(arch_power)
    switch (type) {
    case GPR:
        return registerSpace::GPR(number);
        break;
    case FPR:
        return registerSpace::FPR(number);
        break;
    case SPR:
        return registerSpace::SPR(number);
        break;
    default:
        assert(0);
        return REG_NULL;
        break;
    }
#elif defined(arch_x86) || defined(arch_x86_64)
    // Should do a mapping here from entire register space to "expected" encodings.
    return number;
#else
    assert(0);
    return 0;
#endif
}

registerSpace *registerSpace::getRegisterSpace(unsigned width) {
    if (globalRegSpace_ == NULL) initialize();
    registerSpace *ret = (width == 4) ? globalRegSpace_ : globalRegSpace64_;
    assert(ret);
    return ret;
}

registerSpace *registerSpace::getRegisterSpace(AddressSpace *as) {
    return getRegisterSpace(as->getAddressWidth());
}

registerSpace *registerSpace::conservativeRegSpace(AddressSpace *proc) {
    registerSpace *ret = getRegisterSpace(proc);
    ret->specializeSpace(arbitrary);
    return ret;
}

registerSpace *registerSpace::optimisticRegSpace(AddressSpace *proc) {
    registerSpace *ret = getRegisterSpace(proc);
    ret->specializeSpace(ABI_boundary);
    return ret;
}

registerSpace *registerSpace::irpcRegSpace(AddressSpace *proc) {
   registerSpace *rspace = conservativeRegSpace(proc);
   rspace->initRealRegSpace();
   return rspace;
}

registerSpace *registerSpace::savedRegSpace(AddressSpace *proc) {
    registerSpace *ret = getRegisterSpace(proc);
    ret->specializeSpace(allSaved);
    return ret;
}

registerSpace *registerSpace::actualRegSpace(instPoint *iP, 
                                             callWhen 
#if defined(cap_liveness)
                                             when 
#endif
                                             ) 
{
#if defined(cap_liveness)
    if (BPatch::bpatch->livenessAnalysisOn()) {
        assert(iP);
        registerSpace *ret = NULL;

        liveness_printf("%s[%d] Asking for actualRegSpace for iP at 0x%lx, dumping info:\n",
                        FILE__, __LINE__, iP->addr());
        liveness_cerr << iP->liveRegisters(when) << endl;

        ret = getRegisterSpace(iP->proc());
        ret->specializeSpace(iP->liveRegisters(when));

        ret->cleanSpace();
        return ret;
    }
#endif
    // Use one of the other registerSpaces...
    // If we're entry/exit/call site, return the optimistic version
    /*    if (iP->getPointType() == functionEntry)
        return optimisticRegSpace(iP->proc());
    if (iP->getPointType() == functionExit)
        return optimisticRegSpace(iP->proc());
    if (iP->getPointType() == callSite)
    return optimisticRegSpace(iP->proc());*/
    
    return conservativeRegSpace(iP->proc());
}

void registerSpace::overwriteRegisterSpace(Register,
                                           Register) {
    // This should _NOT_ be used; it's defined to catch errors.
    assert(0);
}

// Ugly IA-64-age.
void registerSpace::overwriteRegisterSpace64(Register first,
                                             Register last) {
    delete globalRegSpace64_;
    globalRegSpace64_ = new registerSpace();
    
    pdvector<registerSlot *> regs;
    for (unsigned i = first; i <= last; i++) {
        char buf[128];
        sprintf(buf, "reg%d", i);
        regs.push_back(new registerSlot(i,
                                        buf,
                                        false,
                                        registerSlot::deadAlways,
                                        registerSlot::GPR));
    }
    createRegSpaceInt(regs, globalRegSpace64_);
}


registerSpace::registerSpace() :
    savedFlagSize(0),
    currStackPointer(0),
    registers_(uiHash),
    addr_width(0)
{
}

registerSpace::~registerSpace()
{
    for (regDictIter i = registers_.begin(); i != registers_.end(); i++) {
        delete i.currval();
    }
}

void registerSpace::createRegisterSpace(pdvector<registerSlot *> &registers) {
    // We need to initialize the following:
    // registers_ (dictionary_hash)
    // GPRs_ (vector of pointers to elements in registers_
    // FPRs_ (vector of pointers ...)
    // SPRs_ (...)

    assert(globalRegSpace_ == NULL);
    globalRegSpace_ = new registerSpace();
    globalRegSpace_->addr_width = 4;
    createRegSpaceInt(registers, globalRegSpace_);

}

void registerSpace::createRegisterSpace64(pdvector<registerSlot *> &registers) {
    // We need to initialize the following:
    // registers_ (dictionary_hash)
    // GPRs_ (vector of pointers to elements in registers_
    // FPRs_ (vector of pointers ...)
    // SPRs_ (...)

    assert(globalRegSpace64_ == NULL);
    globalRegSpace64_ = new registerSpace();
    globalRegSpace64_->addr_width = 8;
    createRegSpaceInt(registers, globalRegSpace64_);
    
}

void registerSpace::createRegSpaceInt(pdvector<registerSlot *> &registers,
                                      registerSpace *rs) {
    for (unsigned i = 0; i < registers.size(); i++) {
        Register reg = registers[i]->number;

        rs->registers_[reg] = registers[i];

        rs->registersByName[registers[i]->name] = registers[i]->number;

        switch (registers[i]->type) {
        case registerSlot::GPR: {
	  bool physical = true;
#if defined(arch_x86) || defined(arch_x86_64)
	  if (rs->addr_width == 4)
	    physical = false;
#endif
	  if (physical) rs->physicalRegisters_[reg] = registers[i];

	  rs->GPRs_.push_back(registers[i]);
	  break;
	}
        case registerSlot::FPR:
	  rs->FPRs_.push_back(registers[i]);
	  break;
        case registerSlot::SPR:
	  rs->SPRs_.push_back(registers[i]);
	  break;
        case registerSlot::realReg:
	  rs->physicalRegisters_[reg] = registers[i];
	  rs->realRegisters_.push_back(registers[i]);
	  break;
        default:
            fprintf(stderr, "Error: no match for %d\n", registers[i]->type);
            assert(0);
            break;
        }
    }

}

bool registerSpace::trySpecificRegister(codeGen &gen, Register num, 
					bool noCost)
{
    registerSlot *tmp;
    registers_.find(num, tmp);
    if (!tmp) return false;

    registerSlot *reg = registers_[num];
    if (reg->offLimits) return false;
    else if (reg->refCount > 0) return false;
    else if (reg->liveState == registerSlot::live) {
        if (!spillRegister(num, gen, noCost)) {
            return false;
        }
    }
    else if (reg->keptValue) {
      return false;
    }
    
    reg->markUsed(true);

    regalloc_printf("Allocated register %d\n", num);

    return true;
}

bool registerSpace::allocateSpecificRegister(codeGen &gen, Register num,
					     bool noCost) 
{
  regalloc_printf("Allocating specific register %d\n", num);

  debugPrint();

    registerSlot *tmp;
    registers_.find(num, tmp);
    if (!tmp) {
      regalloc_printf("Error: register does not exist!\n");
      return false;
    }

    registerSlot *reg = registers_[num];
    if (reg->offLimits) {
      regalloc_printf("Error: register off limits!\n");
      return false;
    }
    else if (reg->refCount > 0) {
      regalloc_printf("Error: register currently in use!\n");
      return false;
    }
    else if (reg->liveState == registerSlot::live) {
      if (!spillRegister(num, gen, noCost)) { 
	regalloc_printf("Error: specific register could not be spilled!\n");
	return false;
      }
    }
    else if (reg->keptValue) {
      if (!stealRegister(num, gen, noCost)) {
	regalloc_printf("Error: register has cached value, unable to steal!\n");
	return false;
      }
    }
    
    reg->markUsed(true);
    gen.markRegDefined(reg->number);
      

    regalloc_printf("Allocated register %d\n", num);

    return true;
}

Register registerSpace::getScratchRegister(codeGen &gen, bool noCost, bool realReg) {
    pdvector<Register> empty;
    return getScratchRegister(gen, empty, noCost, realReg);
}

Register registerSpace::getScratchRegister(codeGen &gen, pdvector<Register> &excluded, bool noCost, bool realReg) {
  static int num_allocs = 0;
  
  pdvector<registerSlot *> couldBeStolen;
  pdvector<registerSlot *> couldBeSpilled;
  
  debugPrint();

  registerSlot *toUse = NULL;

  regalloc_printf("Allocating register: selection is %s\n",
		  realReg ? (realRegisters_.empty() ? "GPRS" : "Real registers") : "GPRs");

  pdvector<registerSlot *> &regs = (realReg ? (realRegisters_.empty() ? GPRs_ : realRegisters_ ) : GPRs_ );
  regalloc_printf("%d options in registers\n", regs.size());

  for (unsigned i = 0; i < regs.size(); i++) {
    registerSlot *reg = regs[i];
      
    regalloc_printf("%s[%d]: getting scratch register, examining %d of %d: reg %d (%s), offLimits %d, refCount %d, liveState %s, keptValue %d\n",
		    FILE__, __LINE__, i, regs.size(),
		    reg->number,
		    reg->name.c_str(),
		    reg->offLimits,
		    reg->refCount,
		    (reg->liveState == registerSlot::live) ? "live" : ((reg->liveState == registerSlot::dead) ? "dead" : "spilled"),
		    reg->keptValue);
      
    bool found = false;
    for (unsigned int i = 0; i < excluded.size(); ++i) {
      Register &ex_reg = excluded[i];
      if (reg->number == ex_reg) {
	found = true;
	break;
      }
    }

        if (found) continue;

        if (reg->offLimits) continue;
        if (reg->refCount > 0) continue;
        if (reg->liveState == registerSlot::live) {
            // Don't do anything for now, but add to the "could be" list
            couldBeSpilled.push_back(reg);
            continue;
        }
        if (reg->keptValue) {
            // As above
            couldBeStolen.push_back(reg);
            continue;
        }
        // Hey, got one.
        toUse = reg;
        break;
    }

    if (toUse == NULL) {
        // Argh. Let's assume spilling is cheaper
        for (unsigned i = 0; i < couldBeSpilled.size(); i++) {
            if (spillRegister(couldBeSpilled[i]->number, gen, noCost)) {
                toUse = couldBeSpilled[i];
                break;
            }
        }
    }
    
    // Still?
    if (toUse == NULL) {
        for (unsigned i = 0; i < couldBeStolen.size(); i++) {
            if (stealRegister(couldBeStolen[i]->number, gen, noCost)) {
                toUse = couldBeStolen[i];
                break;
            }
        }
    }

    if (toUse == NULL) {
        // Crap.
      // debugPrint();
        return REG_NULL;
    }

    toUse->alloc_num = num_allocs;
    num_allocs++;

  toUse->markUsed(false);

  gen.markRegDefined(toUse->number);

  return toUse->number;
}

Register registerSpace::allocateRegister(codeGen &gen, 
                                         bool noCost,
					 bool realReg) 
{
  regalloc_printf("Allocating and retaining register...\n");
  Register reg = getScratchRegister(gen, noCost, realReg);
  regalloc_printf("retaining register %d\n", reg);
  if (reg == REG_NULL) return REG_NULL;
  if (realReg) {
    physicalRegs(reg)->refCount = 1;
  }
  else {
    registers_[reg]->refCount = 1;
  }
  regalloc_printf("Allocated register %d\n", reg);
  return reg;
}

bool registerSpace::spillRegister(Register reg, codeGen &, bool /*noCost*/) {
  assert(!registers_[reg]->offLimits);

  //assert(0 && "Unimplemented!");
  return false;
}

bool registerSpace::stealRegister(Register reg, codeGen &gen, bool /*noCost*/) {
    // Can be made a return false; this for correctness.
    assert(registers_[reg]->refCount == 0);
    assert(registers_[reg]->keptValue == true);
    assert(registers_[reg]->liveState != registerSlot::live);

    regalloc_printf("Stealing register %d\n", reg);

    // Let the AST know it just lost...
    if (!gen.tracker()->stealKeptRegister(registers_[reg]->number)) return false;
    registers_[reg]->keptValue = false;
    
    return true;
}


// This might mean something different later, but for now means
// "Save special purpose registers". We may want to define a "volatile"
// later - something like "can be unintentionally nuked". For example,
// x86 flags register. 
#if defined(arch_x86_64) || defined(arch_x86)
bool registerSpace::checkVolatileRegisters(codeGen &gen,
                                           registerSlot::livenessState_t state)
{
    if (addr_width == 8) {
        for (unsigned i = REGNUM_OF; i <= REGNUM_RF; i++) {
            if (registers_[i]->liveState == state)
                return true;
        }
        return false;
    }

    assert(addr_width == 4);
    return (registers_[IA32_FLAG_VIRTUAL_REGISTER]->liveState == state);
}
#else
bool registerSpace::checkVolatileRegisters(codeGen &,
                                           registerSlot::livenessState_t)
{
    assert(0);
    return false;
}
#endif

#if defined(arch_x86_64) || defined(arch_x86)
bool registerSpace::saveVolatileRegisters(codeGen &gen)
{
    savedFlagSize = 0;
    if (!checkVolatileRegisters(gen, registerSlot::live))
        return false;

    // Okay, save.
    if (addr_width == 8) {
        // save flags (PUSHFQ)
        emitSimpleInsn(0x9C, gen);

        // And mark flags as spilled.
        for (unsigned i = REGNUM_OF; i <= REGNUM_RF; i++) {
            registers_[i]->liveState = registerSlot::spilled;
        }

    } else {
        assert(addr_width == 4);

        emitPush(RealRegister(REGNUM_EAX), gen);
        emitSimpleInsn(0x9f, gen);
        emitSaveO(gen);
        gen.markRegDefined(REGNUM_EAX);
        //emitSimpleInsn(PUSHFD, gen);
        registers_[IA32_FLAG_VIRTUAL_REGISTER]->liveState =
            registerSlot::spilled;
    }

    savedFlagSize = addr_width;
    return true;
}
#else
bool registerSpace::saveVolatileRegisters(codeGen &) {
    assert(0);
    return true;
}
#endif

#if defined(arch_x86_64) || defined(arch_x86)
bool registerSpace::restoreVolatileRegisters(codeGen &gen)
{
    if (!checkVolatileRegisters(gen, registerSlot::spilled))
        return false;

    // Okay, restore.
    if (addr_width == 8) {
        // restore flags (POPFQ)
        emitSimpleInsn(0x9D, gen);

        // Don't care about their state...

    } else {
        assert(addr_width == 4);

        emitRestoreO(gen);
        emitSimpleInsn(0x9E, gen);
        emitPop(RealRegister(REGNUM_EAX), gen);
        // State stays at spilled, which is incorrect - but will never matter.
    }

    return true;
}
#else
bool registerSpace::restoreVolatileRegisters(codeGen &) {
    assert(0);
    return true;
}
#endif

// Free the specified register (decrement its refCount)
void registerSpace::freeRegister(Register num) 
{
    registerSlot *reg = findRegister(num);
    if (!reg) return;

    reg->refCount--;
    regalloc_printf("Freed register %d: refcount now %d\n", num, reg->refCount);

    if( reg->refCount < 0 ) {
        //bperr( "Freed free register!\n" );
        reg->refCount = 0;
    }

#if defined(arch_x86)
    if (reg->refCount == 0 && !registers_[num]->keptValue) {
       markVirtualDead(num);
    }
#endif
    return;

}

// Free the register even if its refCount is greater that 1
void registerSpace::forceFreeRegister(Register num) 
{
    registerSlot *reg = findRegister(num);
    if (!reg) return;

    reg->refCount = 0;
}

// DO NOT USE THIS!!!! to tell if you can use a register as
// a scratch register; do that with trySpecificRegister
// or allocateSpecificRegister. This is _ONLY_ to determine
// if a register should be saved (e.g., over a call).
bool registerSpace::isFreeRegister(Register num) {
    registerSlot *reg = findRegister(num);
    if (!reg) return false;

    if (reg->refCount > 0)
        return false; 
    if (reg->keptValue) 
      return false;
    return true;
}

// Bump up the reference count. Occasionally, we underestimate it
// and call this routine to correct this.
void registerSpace::incRefCount(Register num)
{
    registerSlot *reg = findRegister(num);
    if (!reg) return;
    reg->refCount++;
}

void registerSpace::cleanSpace() {
    regalloc_printf("============== CLEAN ==============\n");

    for (regDictIter i = registers_.begin(); i != registers_.end(); i++) {
        i.currval()->cleanSlot();
    }
    for (unsigned i=0; i<realRegisters_.size(); i++) {
       realRegisters_[i]->cleanSlot();
    }
}

bool registerSpace::restoreAllRegisters(codeGen &, bool) {
    assert(0);
    return true;
}

bool registerSpace::restoreRegister(Register, codeGen &, bool /*noCost*/) 
{
    assert(0);
    return true;
}

bool registerSpace::popRegister(Register, codeGen &, bool) {
    assert(0);
    return true;
}

bool registerSpace::markReadOnly(Register) {
    assert(0);
    return true;
}

bool registerSpace::readProgramRegister(codeGen &gen,
                                        Register source,
                                        Register destination,
                                        unsigned 
#if !defined(arch_power)
                                        size
#endif  
) 
{
#if !defined(arch_power)
    emitLoadPreviousStackFrameRegister((Address)source,
                                       destination,
                                       gen,
                                       size,
                                       true);
    return true;
#else
    // Real version that uses stored information

    // First step: identify the registerSlot that contains information
    // about the source register.
    // cap_emitter

    registerSlot *src = registers_[source];
    assert(src);
    registerSlot *dest = registers_[destination];
    assert(dest);

    // Okay. We need to know where the register is compared with our
    // current location vis-a-vis the stack pointer. Now, on most
    // platforms this doesn't matter, as the SP never moves. Well, not
    // so x86. 
    switch (src->spilledState) {
    case registerSlot::unspilled:
	    printf(" emitMovRegToReg source %d dest %d \n", source, destination);
		    printf(" emitMovRegToReg source %d dest %d GPR %d SPR %d \n", src->type, dest->type, registerSlot::GPR, registerSlot::SPR);
			    printf(" emitMovRegToReg source %d dest %d \n", src->number, dest->number);

        gen.codeEmitter()->emitMoveRegToReg(src, dest, gen);
        return true;
        break;
    case registerSlot::framePointer: {
        registerSlot *frame = registers_[framePointer()];
        assert(frame);

        // We can't use existing mechanisms because they're all built
        // off the "non-instrumented" case - emit a load from the
        // "original" frame pointer, whereas we want the current one. 
        gen.codeEmitter()->emitLoadRelative(destination, src->saveOffset, framePointer(),  gen.addrSpace()->getAddressWidth(), gen);
        return true;
        break;
    }
    default:
        assert(0);
        return false;
        break;
    }
#endif         
    
}
bool registerSpace::writeProgramRegister(codeGen &gen,
                                         Register destination,
                                         Register source,
                                         unsigned 
#if !defined(arch_x86) && !defined(arch_power)
                                         size
#endif
) {
#if !defined(arch_x86) && !defined(arch_power)
    emitStorePreviousStackFrameRegister((Address) destination,
                                        source,
                                        gen,
                                        size,
                                        true);
    return true;
#else
    registerSlot *src = registers_[source];
    assert(source);
    registerSlot *dest = registers_[destination];
    assert(dest);

    // Okay. We need to know where the register is compared with our
    // current location vis-a-vis the stack pointer. Now, on most
    // platforms this doesn't matter, as the SP never moves. Well, not
    // so x86. 

    switch (src->spilledState) {
    case registerSlot::unspilled:
        if (source != destination)
            gen.codeEmitter()->emitMoveRegToReg(source, destination, gen);
        return true;
        break;
    case registerSlot::framePointer: {
        registerSlot *frame = registers_[framePointer()];
        assert(frame);

        // When this register was saved we stored its offset from the base pointer.
        // Use that to load it. 
        gen.codeEmitter()->emitStoreRelative(source, dest->saveOffset, framePointer(), gen.addrSpace()->getAddressWidth(), gen);
        return true;
        break;
    }
    default:
        assert(0);
        return false;
        break;
    }
#endif
}


registerSlot *registerSpace::findRegister(Register source) {
    // Oh, oops... we're handed a register number... and we can't tell if it's
    // GPR, FPR, or SPR...
    if (source == REG_NULL) return NULL;
    registerSlot *reg = NULL;
    if (!registers_.find(source, reg)) return NULL;
    return reg;
}

registerSlot *registerSpace::findRegister(RealRegister source) {
    return realRegisters_[source.reg()];
}

bool registerSpace::markSavedRegister(RealRegister num, int offsetFromFP) {
   regalloc_printf("Marking register %d as saved, %d from frame pointer\n",
                   num.reg(), offsetFromFP);
   registerSlot *s = findRegister(num);
   return markSavedRegister(s, offsetFromFP);   
}

bool registerSpace::markSavedRegister(Register num, int offsetFromFP) {
    regalloc_printf("Marking register %d as saved, %d from frame pointer\n",
                    num, offsetFromFP);
    // Find the register slot
    registerSlot *s = findRegister(num);
    return markSavedRegister(s, offsetFromFP);
}

bool registerSpace::markSavedRegister(registerSlot *s, int offsetFromFP) {
    if (s == NULL)  {
        // We get this on platforms where we save registers we don't use in
        // code generation... like, say, RSP or RBP on AMD-64.
        //fprintf(stderr, "ERROR: unable to find register %d\n", num);
        return false;
    }

    if (s->spilledState != registerSlot::unspilled) {
        // Things to do... add this check in, yo. Right now we don't clean
        // properly.
        
//        assert(0);
    }

    s->liveState = registerSlot::spilled;

    s->spilledState = registerSlot::framePointer;
    s->saveOffset = offsetFromFP;
    return true;
}

void registerSlot::debugPrint(const char *prefix) {
    if (!dyn_debug_regalloc) return;

	if (prefix) fprintf(stderr, "%s", prefix);
	fprintf(stderr, "Num: %d, name %s, type %s, refCount %d, liveState %s, beenUsed %d, initialState %s, offLimits %d, keptValue %d, alloc %d\n",
                number, 
                name.c_str(),
                (type == GPR) ? "GPR" : ((type == FPR) ? "FPR" : "SPR"),
                refCount, 
                (liveState == live ) ? "live" : ((liveState == spilled) ? "spilled" : "dead"),
                beenUsed, 
                (initialState == deadAlways) ? "always dead" : ((initialState == deadABI) ? "ABI dead" : "always live"),
                offLimits, 
                keptValue,
                alloc_num);
}

void registerSpace::debugPrint() {
  if (!dyn_debug_regalloc) return;
  
  // Dump out our data
  fprintf(stderr, "Beginning debug print of registerSpace at %p...", this);
  fprintf(stderr, "GPRs: %ld, FPRs: %ld, SPRs: %ld\n", 
	  (long) GPRs_.size(), (long) FPRs_.size(), (long) SPRs_.size());
  fprintf(stderr, "Stack pointer is at %d\n",
	  currStackPointer);
  fprintf(stderr, "Register dump:");
  fprintf(stderr, "=====GPRs=====\n");
  for (unsigned i = 0; i < GPRs_.size(); i++) {
    GPRs_[i]->debugPrint("\t");
  }
  fprintf(stderr, "=====FPRs=====\n");
  for (unsigned i = 0; i < FPRs_.size(); i++) {
    FPRs_[i]->debugPrint("\t");
  }
  fprintf(stderr, "=====SPRs=====\n");
  for (unsigned i = 0; i < SPRs_.size(); i++) {
    SPRs_[i]->debugPrint("\t");
  }
  fprintf(stderr, "=====RealRegs=====\n");
  for (unsigned i = 0; i < realRegisters_.size(); i++) {
    realRegisters_[i]->debugPrint("\t");
  }
}

bool registerSpace::markKeptRegister(Register reg) {
	regalloc_printf("Marking register %d as kept\n", reg);
   registers_[reg]->keptValue = true;
   return false;
}

void registerSpace::unKeepRegister(Register reg) {
	regalloc_printf("Marking register %d as unkept\n", reg);
   registers_[reg]->keptValue = false;
}


void registerSpace::specializeSpace(rs_location_t state) {
    for (regDictIter i = registers_.begin(); i != registers_.end(); i++) {
        registerSlot *reg = i.currval();
        switch (state) {
        case arbitrary:
            if (reg->initialState == registerSlot::deadAlways)
                reg->liveState = registerSlot::dead;
            else
                reg->liveState = registerSlot::live;
            break;
        case ABI_boundary:
            if ((reg->initialState == registerSlot::deadABI) ||
                (reg->initialState == registerSlot::deadAlways))
                reg->liveState = registerSlot::dead;
            else
                reg->liveState = registerSlot::live;
            break;
        case allSaved:
            reg->liveState = registerSlot::dead;
            break;
        default:
            assert(0);
        }
    }
    cleanSpace();

    regalloc_printf("%s[%d]: specialize space done with argument %d\n", FILE__, __LINE__, state);
}

bool registerSpace::anyLiveGPRsAtEntry() const {
    for (unsigned i = 0; i < GPRs_.size(); i++) {
        if (GPRs_[i]->liveState != registerSlot::dead)
            return true;
    }
    return false;
}

bool registerSpace::anyLiveFPRsAtEntry() const {
    for (unsigned i = 0; i < FPRs_.size(); i++) {
        if (FPRs_[i]->liveState != registerSlot::dead)
            return true;
    }
    return false;
}

bool registerSpace::anyLiveSPRsAtEntry() const {
    for (unsigned i = 0; i < SPRs_.size(); i++) {
        if (SPRs_[i]->liveState != registerSlot::dead)
            return true;
    }
    return false;
}

pdvector<registerSlot *>& registerSpace::trampRegs()
{
#if defined(arch_x86) || defined(arch_x86_64)
   if (addr_width == 4)
      return realRegs();
#endif
   return GPRs_;
}

registerSlot *registerSpace::operator[](Register reg) {
    return registers_[reg];
}

#if defined(cap_liveness)
const bitArray &registerSpace::getCallReadRegisters() const {
    if (addr_width == 4)
        return callRead_;
    else if (addr_width == 8)
        return callRead64_;
    else {
        assert(0);
        return callRead_;
    }
}
    

const bitArray &registerSpace::getCallWrittenRegisters() const {
    if (addr_width == 4)
        return callWritten_;
    else if (addr_width == 8)
        return callWritten64_;
    else {
        assert(0);
        return callWritten_;
    }
}
    
const bitArray &registerSpace::getReturnReadRegisters() const {
    if (addr_width == 4)
        return returnRead_;
    else if (addr_width == 8)
        return returnRead64_;
    else {
        assert(0);
        return returnRead_;
    }
}

const bitArray &registerSpace::getSyscallReadRegisters() const {
    if (addr_width == 4)
        return syscallRead_;
    else if (addr_width == 8)
        return syscallRead64_;
    else {
        assert(0);
        return syscallRead_;
    }
}
const bitArray &registerSpace::getSyscallWrittenRegisters() const {
    if (addr_width == 4)
        return syscallWritten_;
    else if (addr_width == 8)
        return syscallWritten64_;
    else {
        assert(0);
        return syscallWritten_;
    }
}

const bitArray &registerSpace::getAllRegs() const
{
   if (addr_width == 4)
      return allRegs_;
   else if (addr_width == 8)
      return allRegs64_;
   else {
      assert(0);
      return allRegs_;
   }   
}

#endif

bitArray registerSpace::getBitArray()  {
#if defined(arch_x86) || defined(arch_x86_64)
    return bitArray(REGNUM_IGNORED+1);
#elif defined(arch_power)
    return bitArray(lastReg);
#else
    return bitArray();
#endif
}

// Big honkin' name->register map

void registerSpace::getAllRegisterNames(std::vector<std::string> &ret) {
    // Currently GPR only
    for (unsigned i = 0; i < GPRs_.size(); i++) {
        ret.push_back(GPRs_[i]->name);
    }
}

Register registerSpace::getRegByName(const std::string name) {
    map<std::string,Register>::iterator cur = registersByName.find(name);
    if (cur == registersByName.end())
        return REG_NULL;
    return (*cur).second;
}

std::string registerSpace::getRegByNumber(Register reg) {
    return registers_[reg]->name;
}

// If we have defined realRegisters_ (IA-32 and 32-bit mode AMD-64) 
// return that. Otherwise return GPRs. 
pdvector<registerSlot *> &registerSpace::realRegs() {
    if (realRegisters_.size())
       return realRegisters_;
    else
       return GPRs();
}

RealRegister registerSpace::findReal(registerSlot *virt_r, bool &already_setup)
{
   assert(virt_r);
   
   int oldest_reg = -1;
   int oldest_free_reg = -1;
   int used_free_reg = -1;
   already_setup = false;
   
   for (unsigned i=0; i<regState().size(); i++) {
      RealRegsState &real_reg = regState()[i];
      if (!real_reg.is_allocatable)
         continue;

      if (virt_r == real_reg.contains) {
         //We already have this virtual register stored in a real register.
         // just return it.
         real_reg.last_used = timeline()++;
         already_setup = true;
         return RealRegister(i);
      }
      if (!real_reg.contains &&
          real_reg.been_used)
      {
         used_free_reg = i;
      }
      if (!real_reg.contains && 
          (oldest_free_reg == -1 || 
           real_reg.last_used < regState()[oldest_free_reg].last_used)) 
      {
         //There's a free register, remember it.
         oldest_free_reg = i;
      }
      if (real_reg.contains && 
          (oldest_reg == -1 ||
           real_reg.last_used < regState()[oldest_reg].last_used))
      {
         //Keep track of the LRU register, in case we don't
         // have anything free.  LRU isn't ideal for register
         // allocation, but we're trying to keep this single pass.
         oldest_reg = i;
      }
   }

   //We shouldn't have no free registers and no oldest alloced registers.
   assert(oldest_free_reg != -1 || oldest_reg != -1 || used_free_reg != -1);
   if (used_free_reg != -1)
      return RealRegister(used_free_reg);
   if (oldest_free_reg != -1) {
      return RealRegister(oldest_free_reg);
   }
   return RealRegister(oldest_reg);
}

void registerSpace::spillReal(RealRegister r, codeGen &gen)
{
   if (!regState()[r.reg()].is_allocatable)
      return;
   if (!regState()[r.reg()].contains)
      return;
   regs_been_spilled.insert(regState()[r.reg()].contains);
   spillToVReg(r, regState()[r.reg()].contains, gen);
   freeReal(r);
}

void registerSpace::loadReal(RealRegister r, registerSlot *virt_r, codeGen &gen)
{
   
   assert(!regState()[r.reg()].contains);
   if (regs_been_spilled.count(virt_r)) {
      movVRegToReal(virt_r, r, gen);
   }
   regState()[r.reg()].contains = virt_r;
   regState()[r.reg()].last_used = timeline()++;
}

void registerSpace::freeReal(RealRegister r)
{
   assert(regState()[r.reg()].contains);
   regState()[r.reg()].contains = NULL;
   regState()[r.reg()].last_used = timeline()++;
}

RealRegister registerSpace::loadVirtual(registerSlot *virt_r, codeGen &gen)
{
   assert(virt_r);
   bool done;

   RealRegister reg_to_use = findReal(virt_r, done);
   if (done) {
      return reg_to_use;
   }

   spillReal(reg_to_use, gen);
   loadReal(reg_to_use, virt_r, gen);

   return reg_to_use;
}

RealRegister registerSpace::loadVirtual(Register virt_r, codeGen &gen)
{
   return loadVirtual((*this)[virt_r], gen);
}

RealRegister registerSpace::loadVirtualForWrite(registerSlot *virt_r, codeGen &gen)
{
   assert(virt_r);
   bool done;

   RealRegister reg_to_use = findReal(virt_r, done);
   if (done) {
      return reg_to_use;
   }

   spillReal(reg_to_use, gen);
   regState()[reg_to_use.reg()].contains = virt_r;
   regState()[reg_to_use.reg()].last_used = timeline()++;

   gen.markRegDefined(reg_to_use.reg());
   return reg_to_use;
}

RealRegister registerSpace::loadVirtualForWrite(Register virt_r, codeGen &gen)
{
   return loadVirtualForWrite((*this)[virt_r], gen);
}

void registerSpace::makeRegisterAvail(RealRegister r, codeGen &gen) {
   spillReal(r, gen);
   gen.markRegDefined(r.reg());
}

void registerSpace::noteVirtualInReal(registerSlot *v_r, RealRegister r_r)
{
   int reg = r_r.reg();

   bool already_allocd;
   RealRegister old_location = findReal(v_r, already_allocd);
   if (already_allocd) {
      regState()[old_location.reg()].contains = NULL;
      regState()[old_location.reg()].last_used = timeline()++;
   }

   assert (!regState()[reg].contains);
   regState()[reg].contains = v_r;
   regState()[reg].last_used = timeline()++;
}

void registerSpace::noteVirtualInReal(Register v_r, RealRegister r_r)
{
   noteVirtualInReal((*this)[v_r], r_r);
}

void registerSpace::loadVirtualToSpecific(registerSlot *virt_r, RealRegister real_r, codeGen &gen)
{
   bool already_in_reg;
   RealRegister old_loc = findReal(virt_r, already_in_reg);
   if (already_in_reg && old_loc.reg() == real_r.reg()) {
      return;
   }

   spillReal(real_r, gen);

   if (already_in_reg) {
      movRegToReg(real_r, old_loc, gen);
      freeReal(old_loc);
   }
   else {
      loadReal(real_r, virt_r, gen);
   }
}

void registerSpace::loadVirtualToSpecific(Register virt_r, RealRegister real_r, codeGen &gen)
{
   loadVirtualToSpecific((*this)[virt_r], real_r, gen);
}

void registerSpace::markVirtualDead(Register num)
{
   registerSlot *slot = (*this)[num];
   for (unsigned i=0; i<regState().size(); i++) {
      if (regState()[i].contains == slot) {
         regState()[i].contains = NULL;
         regState()[i].last_used = 0;
      }
   }
}

bool registerSpace::spilledAnything()
{
   return regs_been_spilled.size() != 0;
}

/**
 * This handles merging register states at merges
 * in the generated code CFG.  Used for things like
 * 'if' statements.  
 * Takes the top level registerState (e.g, the code that was
 * generated in an 'if') and emits the saves/restores such to
 * takes us back to the preceding registerState (e.g, the code
 * we would be in if the 'if' hadn't executed).
 **/
void registerSpace::unifyTopRegStates(codeGen &gen)
{
   if (addr_width == 8) {
      //Not implemented on x86_64 yet
      return;
   }
   if (regStateStack.size() == 0)
      return;
   assert(regStateStack.size() >= 2);
   regState_t *src = regStateStack[regStateStack.size() - 1];
   regState_t *dest = regStateStack[regStateStack.size() - 2];

   assert(src->registerStates.size() == dest->registerStates.size());
   
   //Make a map of loaded dest virtuals -> reals
   std::map<registerSlot*, unsigned> dest_virts;
   for (unsigned i=0; i<dest->registerStates.size(); i++) {
      RealRegsState &rrs = dest->registerStates[i];
      if (!rrs.is_allocatable || !rrs.contains)
         continue;
      dest_virts[rrs.contains] = i;
   }

   //For any loaded virtual in src that's not loaded or not loaded in the 
   // proper place in dest,  unload that virtual in src.
   for (unsigned i=0; i<src->registerStates.size(); i++) {
      RealRegsState &rrs = src->registerStates[i];
      if (!rrs.is_allocatable || !rrs.contains) {
         //Unloaded in src or uninteresting register
         continue;
      }
      std::map<registerSlot*, unsigned>::iterator j = dest_virts.find(rrs.contains);
      if (j != dest_virts.end() && (*j).second == i) {
         //Loaded in the same place in dest and src
         continue;
      }
      spillReal(RealRegister(i), gen);
   }

   //Make a map of loaded src virtuals -> reals
   std::map<registerSlot*, unsigned> src_virts;
   for (unsigned i=0; i<src->registerStates.size(); i++) {
      RealRegsState &rrs = src->registerStates[i];
      if (!rrs.is_allocatable || !rrs.contains)
         continue;
      src_virts[rrs.contains] = i;
   }

   //For any loaded virtual in dest that's not loaded in src, 
   // load that virtual in src. 
   for (unsigned i=0; i<dest->registerStates.size(); i++) {
      RealRegsState &rrs = dest->registerStates[i];
      if (!rrs.is_allocatable || !rrs.contains) {
         //Unloaded in src or uninteresting register
         continue;
      }
      std::map<registerSlot*, unsigned>::iterator j = src_virts.find(rrs.contains);
      if (j != src_virts.end()) {
         //Loaded in src
         assert((*j).second == i); //assert it's loaded in the same place
         continue;
      }
      loadReal(RealRegister(i), rrs.contains, gen);
   }

   for (unsigned i=0; i<src->registerStates.size(); i++) {
      assert(src->registerStates[i].contains == dest->registerStates[i].contains);
   }

   if (dest->pc_rel_offset == -1 && src->pc_rel_offset != -1) {
      //src allocated a register for the PCREL operations.  We'll
      // have to free that bad boy.
      gen.rs()->freeRegister(gen.rs()->pc_rel_reg);
   }
   regStateStack.pop_back();
   delete src;
}

void registerSpace::pushNewRegState()
{
   if (regStateStack.size() == 0)
      return;
   regState_t *new_top = new regState_t();
   regState_t *old_top = regStateStack[regStateStack.size()-1];
   new_top->pc_rel_offset = old_top->pc_rel_offset;
   new_top->timeline = old_top->timeline;
   new_top->registerStates = old_top->registerStates;
   new_top->stack_height = old_top->stack_height;
   regStateStack.push_back(new_top);
}

#if defined(arch_x86) || defined(arch_x86_64)
int& registerSpace::pc_rel_offset()
{
   if (!regStateStack.size())
      initRealRegSpace();
   return regStateStack[regStateStack.size()-1]->pc_rel_offset;
}

int& registerSpace::timeline()
{
   if (!regStateStack.size())
      initRealRegSpace();
   return regStateStack[regStateStack.size()-1]->timeline;
}

std::vector<RealRegsState>& registerSpace::regState()
{
   if (!regStateStack.size())
      initRealRegSpace();

   return regStateStack[regStateStack.size()-1]->registerStates;
}

void registerSpace::incStack(int val) {
   if (!regStateStack.size())
      initRealRegSpace();
   regStateStack[regStateStack.size()-1]->stack_height += val;
}

void registerSpace::setStackHeight(int val) {
   cerr << "Setting stack height (" << regStateStack.size() << ") to " << val << endl;
   if (!regStateStack.size())
      initRealRegSpace();
   regStateStack[regStateStack.size()-1]->stack_height = val;
}

int registerSpace::getStackHeight()
{
   if (!regStateStack.size())
      initRealRegSpace();
   cerr << "Getting stack height (" << regStateStack.size() << "): " << regStateStack[regStateStack.size()-1]->stack_height;
   return regStateStack[regStateStack.size()-1]->stack_height;
}

void registerSpace::setInstFrameSize(int val) {
    instFrameSize_ = val;
}

int registerSpace::getInstFrameSize() {
    return instFrameSize_;
}


#endif

#if !defined(arch_x86) && !defined(arch_x86_64)
void registerSpace::initRealRegSpace()
{
}

void registerSpace::spillToVReg(RealRegister /*reg*/, registerSlot * /*v_reg*/, 
                                codeGen & /*gen*/)
{
   assert(0);
}

void registerSpace::movVRegToReal(registerSlot * /*v_reg*/, RealRegister /*r*/, 
                                  codeGen & /*gen*/)
{
   assert(0);
}

void registerSpace::movRegToReg(RealRegister /*dest*/, RealRegister /*src*/, codeGen & /*gen*/)
{
   assert(0);
}

regState_t::regState_t()
{
   assert(0);
}

int& registerSpace::pc_rel_offset()
{
   static int none = 0;
   return none;
}

int& registerSpace::timeline()
{
   static int none = 0;
   return none;
}

std::vector<RealRegsState>& registerSpace::regState()
{
   static std::vector<RealRegsState> none;
   return none;
}

void registerSpace::incStack(int) {
}

void registerSpace::setStackHeight(int) {
}

int registerSpace::getStackHeight()
{
   return 0;
}
#endif
