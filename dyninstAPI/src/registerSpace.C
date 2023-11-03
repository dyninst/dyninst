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

// $Id: registerSpace.C,v 1.25 2008/10/27 17:23:53 mlam Exp $

#include "dyninstAPI/src/image.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/instPoint.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/util.h"
#include "dyninstAPI/src/debug.h"
#include "dyninstAPI/src/addressSpace.h"

#include "dyninstAPI/src/function.h"
#include "dyninstAPI/src/mapped_object.h"
#include "dyninstAPI/src/registerSpace.h"

#include "dyninstAPI/h/BPatch.h"
#include "dyninstAPI/src/BPatch_collections.h"
#include "dyninstAPI/h/BPatch_type.h"
#include "dyninstAPI/src/BPatch_libInfo.h" // For instPoint->BPatch_point mapping

#include "dyninstAPI/h/BPatch_point.h"
#include "dyninstAPI/h/BPatch_memoryAccess_NP.h"

#include "liveness.h"
#include "dyninstAPI/src/RegisterConversion.h"
#include <map>

#if defined(arch_power)
#include "dyninstAPI/src/inst-power.h"
#include "dyninstAPI/src/emit-power.h"
#elif defined(arch_x86) || defined(arch_x86_64)
#include "dyninstAPI/src/inst-x86.h"
#include "dyninstAPI/src/emit-x86.h"
#elif defined (arch_aarch64)
#include "dyninstAPI/src/inst-aarch64.h"
#include "dyninstAPI/src/emit-aarch64.h"
#endif

registerSpace *registerSpace::globalRegSpace_ = NULL;
registerSpace *registerSpace::globalRegSpace64_ = NULL;

bool registerSpace::hasXMM = false;

void registerSlot::cleanSlot() {
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
        return Null_Register;
        break;
    }
#elif defined(arch_x86) || defined(arch_x86_64)
    // Should do a mapping here from entire register space to "expected" encodings.
    return number;
#elif defined(arch_aarch64) 
    switch (type) {
        case GPR:
            return registerSpace::GPR(number);
            break;
        case FPR:
            return registerSpace::FPR(number);
            break;
        default:
            assert(0);
            return Null_Register;
            break;
    }
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
    ret->initRealRegSpace();
    return ret;
}

registerSpace *registerSpace::optimisticRegSpace(AddressSpace *proc) {
    registerSpace *ret = getRegisterSpace(proc);
    ret->specializeSpace(ABI_boundary);
    ret->initRealRegSpace();

    return ret;
}

registerSpace *registerSpace::irpcRegSpace(AddressSpace *proc) {
   registerSpace *rspace = savedRegSpace(proc);
   rspace->initRealRegSpace();
   return rspace;
}

registerSpace *registerSpace::savedRegSpace(AddressSpace *proc) {
    registerSpace *ret = getRegisterSpace(proc);
    ret->specializeSpace(allSaved);
    ret->initRealRegSpace();
    return ret;
}

registerSpace *registerSpace::actualRegSpace(instPoint *iP)
{
    // We just can't trust liveness in defensive mode.
    if (BPatch_defensiveMode == iP->func()->obj()->hybridMode()) {
        return conservativeRegSpace(iP->proc());
    }
   if (BPatch::bpatch->livenessAnalysisOn()) {
      assert(iP);
      registerSpace *ret = NULL;

      ret = getRegisterSpace(iP->proc());
      ret->specializeSpace(iP->liveRegisters());

      ret->cleanSpace();
      ret->initRealRegSpace();
      return ret;
   }
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


registerSpace::registerSpace() :
    pc_rel_reg(0),
    pc_rel_use_count(0),
    instFrameSize_(0),
    savedFlagSize(0),
    currStackPointer(0),
    addr_width(0)
{
}

registerSpace::~registerSpace()
{
    for (auto i = registers_.begin(); i != registers_.end(); i++) {
       delete i->second;
    }
}

void registerSpace::createRegisterSpace(std::vector<registerSlot *> &registers) {
    // We need to initialize the following:
    // registers_ (std::unordered_map)
    // GPRs_ (vector of pointers to elements in registers_
    // FPRs_ (vector of pointers ...)
    // SPRs_ (...)

    assert(globalRegSpace_ == NULL);
    globalRegSpace_ = new registerSpace();
    globalRegSpace_->addr_width = 4;
    createRegSpaceInt(registers, globalRegSpace_);

}

void registerSpace::createRegisterSpace64(std::vector<registerSlot *> &registers) {
    // We need to initialize the following:
    // registers_ (std::unordered_map)
    // GPRs_ (vector of pointers to elements in registers_
    // FPRs_ (vector of pointers ...)
    // SPRs_ (...)

    assert(globalRegSpace64_ == NULL);
    globalRegSpace64_ = new registerSpace();
    globalRegSpace64_->addr_width = 8;
    createRegSpaceInt(registers, globalRegSpace64_);

}

void registerSpace::createRegSpaceInt(std::vector<registerSlot *> &registers,
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
  auto iter = registers_.find(num);
  if (iter == registers_.end()) return false;
  registerSlot *reg = iter->second;

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

  regalloc_printf("Allocated register %u\n", num);

  return true;
}

bool registerSpace::allocateSpecificRegister(codeGen &gen, Register num,
					     bool noCost)
{
  regalloc_printf("Allocating specific register %u\n", num);

  debugPrint();

  auto iter = registers_.find(num);
  if (iter == registers_.end()) {
     regalloc_printf("Error: register does not exist!\n");
     return false;
  }

  registerSlot *reg = iter->second;
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


    regalloc_printf("Allocated register %u\n", num);

    return true;
}

Register registerSpace::getScratchRegister(codeGen &gen, bool noCost, bool realReg) {
    std::vector<Register> empty;
    return getScratchRegister(gen, empty, noCost, realReg);
}

Register registerSpace::getScratchRegister(codeGen &gen, std::vector<Register> &excluded, bool noCost, bool realReg) {
  static int num_allocs = 0;

  std::vector<registerSlot *> couldBeStolen;
  std::vector<registerSlot *> couldBeSpilled;

  debugPrint();

  registerSlot *toUse = NULL;

  regalloc_printf("Allocating register: selection is %s\n",
		  realReg ? (realRegisters_.empty() ? "GPRS" : "Real registers") : "GPRs");

  std::vector<registerSlot *> &regs = (realReg ? (realRegisters_.empty() ? GPRs_ : realRegisters_ ) : GPRs_ );
  regalloc_printf("%lu options in registers\n", regs.size());

  for (unsigned i = 0; i < regs.size(); i++) {
    registerSlot *reg = regs[i];

    regalloc_printf("%s[%d]: getting scratch register, examining %u of %lu: reg %u (%s), offLimits %d, refCount %d, liveState %s, keptValue %d\n",
		    FILE__, __LINE__, i, regs.size(),
		    reg->number,
		    reg->name.c_str(),
		    reg->offLimits,
		    reg->refCount,
		    (reg->liveState == registerSlot::live) ? "live" : ((reg->liveState == registerSlot::dead) ? "dead" : "spilled"),
		    reg->keptValue);

    bool found = false;
    for (unsigned int j = 0; j < excluded.size(); ++j) {
      Register &ex_reg = excluded[j];
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
        return Null_Register;
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
  regalloc_printf("retaining register %u\n", reg);
  if (reg == Null_Register) return Null_Register;
  if (realReg) {
    physicalRegs(reg)->refCount = 1;
  }
  else {
    registers_[reg]->refCount = 1;
  }
  regalloc_printf("Allocated register %u\n", reg);
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

    regalloc_printf("Stealing register %u\n", reg);

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
bool registerSpace::checkVolatileRegisters(codeGen & /*gen*/,
                                           registerSlot::livenessState_t state)
{
   if (addr_width == 8) {
      if (registers_[REGNUM_EFLAGS]->liveState == state) {
         // Must have been an override from somewhere...
         return true;
      }
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
       //emitSimpleInsn(0x9C, gen);
       bool override = false;
       if (registers_[REGNUM_EFLAGS]->liveState == registerSlot::live) {
          override = true;
          registers_[REGNUM_EFLAGS]->liveState = registerSlot::spilled;
       }
       if (registers_[REGNUM_SF]->liveState == registerSlot::live ||
           registers_[REGNUM_ZF]->liveState == registerSlot::live ||
           registers_[REGNUM_AF]->liveState == registerSlot::live ||
           registers_[REGNUM_PF]->liveState == registerSlot::live ||
           registers_[REGNUM_CF]->liveState == registerSlot::live ||
           override) {
          emitSimpleInsn(0x9f, gen);
          registers_[REGNUM_SF]->liveState = registerSlot::spilled;
          registers_[REGNUM_ZF]->liveState = registerSlot::spilled;
          registers_[REGNUM_AF]->liveState = registerSlot::spilled;
          registers_[REGNUM_PF]->liveState = registerSlot::spilled;
          registers_[REGNUM_CF]->liveState = registerSlot::spilled;
          registers_[REGNUM_CF]->liveState = registerSlot::spilled;
       }
       if (registers_[REGNUM_OF]->liveState == registerSlot::live ||
           override) {
          emitSaveO(gen);
          registers_[REGNUM_OF]->liveState = registerSlot::spilled;
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
       if (registers_[REGNUM_OF]->liveState == registerSlot::spilled) {
          emitRestoreO(gen);
       }
       if (registers_[REGNUM_SF]->liveState == registerSlot::spilled ||
           registers_[REGNUM_ZF]->liveState == registerSlot::spilled ||
           registers_[REGNUM_AF]->liveState == registerSlot::spilled ||
           registers_[REGNUM_PF]->liveState == registerSlot::spilled ||
           registers_[REGNUM_CF]->liveState == registerSlot::spilled) {
          emitSimpleInsn(0x9e, gen);
       }

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
    regalloc_printf("Freed register %u: refcount now %d\n", num, reg->refCount);

    if( reg->refCount < 0 ) {
        //bperr( "Freed free register!\n" );
        reg->refCount = 0;
    }

#if defined(arch_x86) || defined(arch_x86_64)
    if (addr_width == 4) {
      if (reg->refCount == 0 && !registers_[num]->keptValue) {
	markVirtualDead(num);
      }
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

    for (auto i = registers_.begin(); i != registers_.end(); i++) {
        i->second->cleanSlot();
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
#elif defined(arch_aarch64)
//#warning "This fucntion is not implemented yet!"
		assert(0);
		return false;
#else
    // Real version that uses stored information

    // First step: identify the registerSlot that contains information
    // about the source register.
    // cap_emitter


    registerSlot *src = registers_[source];
    // If we didn't find src we just corrupted registers_; assert fail.
    // AND FIX THE STRUCTURE.
    assert(src);
    registerSlot *dest = registers_[destination];
    assert(dest);

    // Okay. We need to know where the register is compared with our
    // current location vis-a-vis the stack pointer. Now, on most
    // platforms this doesn't matter, as the SP never moves. Well, not
    // so x86.
    switch (src->spilledState) {
    case registerSlot::unspilled:

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
                                         unsigned) {
#if defined(arch_aarch64)
		// Silence compiler warnings
		(void)gen;
		(void)destination;
		(void)source;

		//not implemented yet
		return false;
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
    if (source == Null_Register) return NULL;

    auto iter = registers_.find(source);
    if (iter == registers_.end()) return NULL;
    return iter->second;
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
    regalloc_printf("Marking register %u as saved, %d from frame pointer\n",
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
	fprintf(stderr, "Num: %u, name %s, type %s, refCount %d, liveState %s, beenUsed %d, initialState %s, offLimits %d, keptValue %d, alloc %d\n",
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
  fprintf(stderr, "Beginning debug print of registerSpace at %p...", (void*)this);
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
	regalloc_printf("Marking register %u as kept\n", reg);
   registers_[reg]->keptValue = true;
   return false;
}

void registerSpace::unKeepRegister(Register reg) {
	regalloc_printf("Marking register %u as unkept\n", reg);
   registers_[reg]->keptValue = false;
}


void registerSpace::specializeSpace(rs_location_t state) {
    for (auto i = registers_.begin(); i != registers_.end(); i++) {
        registerSlot *reg = i->second;
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

std::vector<registerSlot *>& registerSpace::trampRegs()
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
        return Null_Register;
    return (*cur).second;
}

std::string registerSpace::getRegByNumber(Register reg) {
    return registers_[reg]->name;
}

// If we have defined realRegisters_ (IA-32 and 32-bit mode AMD-64)
// return that. Otherwise return GPRs.
std::vector<registerSlot *> &registerSpace::realRegs() {
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

   if (regState()[r.reg()].contains->refCount == 0 &&
       !regState()[r.reg()].contains->keptValue) {
      // Sure, it's spilled...
      freeReal(r);
      return;
   }

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
  regalloc_printf("Freeing register %d\n", r.reg());
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
  regalloc_printf("Noting virtual %s in real reg %d\n",
		  (v_r ? v_r->name.c_str() : "<NULL>"), r_r.reg());

   int reg = r_r.reg();

   bool already_allocd;
   RealRegister old_location = findReal(v_r, already_allocd);
   if (already_allocd) {
      regState()[old_location.reg()].contains = NULL;
      regState()[old_location.reg()].last_used = timeline()++;
   }

   if (regState()[reg].contains) {
     assert(0);
   }
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
   if (!regStateStack.size())
      initRealRegSpace();
   regStateStack[regStateStack.size()-1]->stack_height = val;
}

int registerSpace::getStackHeight()
{
   if (!regStateStack.size())
      initRealRegSpace();
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


void registerSpace::specializeSpace(const bitArray &liveRegs) {
    // Liveness info is stored as a single bitarray for all registers.

#if defined(arch_x86) || defined(arch_x86_64)
    // We use "virtual" registers on the IA-32 platform (or AMD-64 in
    // 32-bit mode), and thus the registerSlot objects have _no_ relation
    // to the liveRegs input set. We handle this as a special case, and
    // look only for the flags representation (which is used to set
    // the IA32_FLAG_VIRTUAL_REGISTER "register"
   if (addr_width == 4) {
      for (unsigned i = 1; i <= NUM_VIRTUAL_REGISTERS; i++) {
         registers_[i]->liveState = registerSlot::dead;
      }
      registers_[IA32_FLAG_VIRTUAL_REGISTER]->liveState = registerSlot::dead;
      for (unsigned i = REGNUM_OF; i <= REGNUM_RF; i++) {
         if (checkLive(i, liveRegs)) {
            registers_[IA32_FLAG_VIRTUAL_REGISTER]->liveState = registerSlot::live;
            break;
         }
      }

      for (unsigned i = 0; i < realRegisters_.size(); ++i) {
         if (checkLive(realRegisters_[i]->number, liveRegs))
	    realRegisters_[i]->liveState = registerSlot::live;
         else
	    realRegisters_[i]->liveState = registerSlot::dead;
      }

      // All we care about for now.
      return;
   }
#endif
   for (auto i = registers_.begin(); i != registers_.end(); i++) {
      if (checkLive(i->second->number, liveRegs)) {
         i->second->liveState = registerSlot::live;
      }
      else
      {
         i->second->liveState = registerSlot::dead;
      }
   }
}

bool registerSpace::checkLive(Register reg, const bitArray &liveRegs){
	static LivenessAnalyzer live1(4);
	static LivenessAnalyzer live2(8);
	std::pair<std::multimap<Register, MachRegister>::iterator, std::multimap<Register, MachRegister>::iterator> range;
	LivenessAnalyzer *live;
	if (addr_width == 4){
#if defined(arch_aarch64)
	assert(0);
	//#error "aarch64 should not be 32bit long"
#else
		range = regToMachReg32.equal_range(reg);
		live = &live1;
#endif
	}
	else {
		range = regToMachReg64.equal_range(reg);
		live = &live2;
	}
	if (range.first == range.second) assert(0);
	for (std::multimap<Register, MachRegister>::iterator iter = range.first; iter != range.second; ++iter)
		if (liveRegs[live->getIndex(iter->second)]) return true;

	return false;
}
