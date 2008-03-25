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

// $Id: registerSpace.C,v 1.20 2008/03/25 19:24:39 bernat Exp $

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
#include <boost/assign/list_of.hpp>
using namespace boost::assign;


#if defined(arch_sparc)
#include "dyninstAPI/src/inst-sparc.h"
#elif defined(arch_power)
#include "dyninstAPI/src/inst-power.h"
#include "dyninstAPI/src/emit-power.h"
#elif defined(arch_x86) || defined(arch_x86_64)
#include "dyninstAPI/src/inst-x86.h"
#include "dyninstAPI/src/emit-x86.h"
#elif defined(arch_ia64)
#include "dyninstAPI/src/inst-ia64.h"
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
    return conservativeRegSpace(proc);
}

registerSpace *registerSpace::savedRegSpace(AddressSpace *proc) {
    registerSpace *ret = getRegisterSpace(proc);
    ret->specializeSpace(allSaved);
    return ret;
}

registerSpace *registerSpace::actualRegSpace(instPoint *iP, callWhen when) {
#if defined(cap_liveness)
    if (BPatch::bpatch->livenessAnalysisOn()) {
        assert(iP);
        registerSpace *ret = NULL;
        /*
#if defined(arch_power)
        // Power has some serious problems right now with GPR/FPR
        // liveness. Disabling.
        if ((iP->getPointType() == functionEntry) ||
            (iP->getPointType() == functionExit) ||
            (iP->getPointType() == callSite))
            ret = optimisticRegSpace(iP->proc());
        else
            ret = conservativeRegSpace(iP->proc());
        
        // Listen to the MQ register setting
        if ((iP->liveRegisters(when))[mq]) {
            ret->registers_[mq]->liveState = registerSlot::live;
        }
        else {
            ret->registers_[mq]->liveState = registerSlot::dead;
        }
        */

        liveness_printf("%s[%d] Asking for actualRegSpace for iP at 0x%lx, dumping info:\n",
                        FILE__, __LINE__, iP->addr());
        liveness_cerr << iP->liveRegisters(when) << endl;

        ret = getRegisterSpace(iP->proc());
        ret->specializeSpace(iP->liveRegisters(when));

        //#endif
        ret->cleanSpace();
        return ret;
    }
#endif
    // Use one of the other registerSpaces...
    // If we're entry/exit/call site, return the optimistic version
    if (iP->getPointType() == functionEntry)
        return optimisticRegSpace(iP->proc());
    if (iP->getPointType() == functionExit)
        return optimisticRegSpace(iP->proc());
    if (iP->getPointType() == callSite)
        return optimisticRegSpace(iP->proc());
    
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
        case registerSlot::GPR: 
            rs->GPRs_.push_back(registers[i]);
            break;
        case registerSlot::FPR:
            rs->FPRs_.push_back(registers[i]);
            break;
        case registerSlot::SPR:
            rs->SPRs_.push_back(registers[i]);
            break;
        default:
            assert(0);
            break;
        }
    }

}

bool registerSpace::allocateSpecificRegister(codeGen &gen, Register num, 
                                             bool noCost)
{
    registerSlot *tmp;
    assert(registers_.find(num, tmp));

    registerSlot *reg = registers_[num];
    if (reg->offLimits) return false;
    else if (reg->refCount > 0) return false;
    else if (reg->liveState == registerSlot::live) {
        if (!spillRegister(num, gen, noCost)) {
            return false;
        }
    }
    else if (reg->keptValue) {
        if (!stealRegister(num, gen, noCost)) return false;
    }
    
    reg->markUsed(true);

    regalloc_printf("Allocated register %d\n", num);

    return true;
}

Register registerSpace::getScratchRegister(codeGen &gen, bool noCost) {
    pdvector<Register> empty;
    return getScratchRegister(gen, empty, noCost);
}

Register registerSpace::getScratchRegister(codeGen &gen, pdvector<Register> &excluded, bool noCost) {
    unsigned scratchIndex = 0;
    pdvector<registerSlot *> couldBeStolen;
    pdvector<registerSlot *> couldBeSpilled;

    debugPrint();

    registerSlot *toUse = NULL;

    for (unsigned i = 0; i < GPRs_.size(); i++) {
        registerSlot *reg = GPRs_[i];

#if 0        
        regalloc_printf("%s[%d]: getting scratch register, examining %d of %d: reg %d, offLimits %d, refCount %d, liveState %s, keptValue %d\n",
                        FILE__, __LINE__, i, GPRs_.size(),
                        reg->number,
                        reg->offLimits,
                        reg->refCount,
                        (reg->liveState == registerSlot::live) ? "live" : ((reg->liveState == registerSlot::dead) ? "dead" : "spilled"),
                        reg->keptValue);
#endif                 

        if (find(excluded, reg->number, scratchIndex)) continue;
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
        debugPrint();
        assert(0 && "Failed to allocate register!");
        return REG_NULL;
    }

    toUse->markUsed(false);
    return toUse->number;
}

Register registerSpace::allocateRegister(codeGen &gen, 
                                         bool noCost) 
{
    regalloc_printf("Allocating and retaining register...\n");
    Register reg = getScratchRegister(gen, noCost);
    regalloc_printf("retaining register %d\n", reg);
    if (reg == REG_NULL) return REG_NULL;

    registers_[reg]->refCount = 1;
    regalloc_printf("Allocated register %d\n", reg);
    return reg;
}

bool registerSpace::spillRegister(Register reg, codeGen &gen, bool noCost) {
    assert(!registers_[reg]->offLimits);

    assert(0 && "Unimplemented!");

    return true;
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
bool registerSpace::saveVolatileRegisters(codeGen &gen) {

#if defined(arch_x86_64) || defined(arch_x86)
    bool doWeSave = false; 
    if (addr_width == 8) {
        for (unsigned i = REGNUM_OF; i <= REGNUM_RF; i++) {
            registerSlot *reg = registers_[i];
            
            if (reg->liveState == registerSlot::live) {
                doWeSave = true;
                break;
            }
        }
        if (!doWeSave) return false; // All done
        
        // Okay, save.
        
        // save flags (PUSHFQ)
        emitSimpleInsn(0x9C, gen);
        
        // And mark flags as spilled. Another for loop.
        for (unsigned i = REGNUM_OF; i <= REGNUM_RF; i++) {
            registerSlot *reg = registers_[i];
            reg->liveState = registerSlot::spilled;
        }
        return true;
    }
    else {
        assert(addr_width == 4);
        if (registers_[IA32_FLAG_VIRTUAL_REGISTER]->liveState == registerSlot::live) {
            emitSimpleInsn(PUSHFD, gen);
            registers_[IA32_FLAG_VIRTUAL_REGISTER]->liveState = registerSlot::spilled;
            return true;
        }
        else {
            return false;
        }
    }
    
#else
    assert(0);
    return true;
#endif
}

bool registerSpace::restoreVolatileRegisters(codeGen &gen) {
#if defined(arch_x86_64) || defined(arch_x86)
    if (addr_width == 8) {
        bool doWeRestore = false;
        for (unsigned i = REGNUM_OF; i <= REGNUM_RF; i++) {
            registerSlot *reg = registers_[i];
            
            if (reg->liveState == registerSlot::spilled) {
                doWeRestore = true;
                break;
            }
        }
        if (!doWeRestore) return false; // All done
        
        // Okay, save.
        
        // save flags (POPFQ)
        emitSimpleInsn(0x9D, gen);
        
        // Don't care about their state...
        return true;
    }
    else {
        assert(addr_width == 4);
        if (registers_[IA32_FLAG_VIRTUAL_REGISTER]->liveState == registerSlot::spilled) {
            emitSimpleInsn(POPFD, gen);
            // State stays at spilled, which is incorrect - but will never matter.
            return true;
        }
        else
            return false;
    }
#else
    assert(0);
    return true;
#endif
}


// Free the specified register (decrement its refCount)
void registerSpace::freeRegister(Register num) 
{
    registerSlot *reg = findRegister(num);
    if (!reg) return;

    reg->refCount--;
    regalloc_printf("Freed register %d: refcount now %d\n", num, reg->refCount);

    if( reg->refCount < 0 ) {
#if !defined(arch_ia_64)
        // IA-64 gets this with the frame pointer...
        bperr( "Freed free register!\n" );
#endif
        reg->refCount = 0;
    }
    return;

}

// Free the register even if its refCount is greater that 1
void registerSpace::forceFreeRegister(Register num) 
{
    registerSlot *reg = findRegister(num);
    if (!reg) return;

    reg->refCount = 0;
}

bool registerSpace::isFreeRegister(Register num) {
    registerSlot *reg = findRegister(num);
    if (!reg) return false;

    if (reg->refCount > 0)
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
}

bool registerSpace::restoreAllRegisters(codeGen &, bool) {
#if 0
    // This will only restore "saved" registers, not "spilled" registers.
    // We should do both, but the individual "restoreRegister" can't build
    // the list necessary for popping registers...
    
    // We have some registers saved off the frame pointer, some saved
    // on the stack, and some unsaved. We want to go over the list
    // of registers and build up the spill list (in order so we can pop
    // em), then pop all and then spill from the frame pointer.
    
    int numPushed = currStackPointer;

    int pushedReg[numPushed];

    for (int i = 0; i < numPushed; i++) pushedReg[i] = -1;
    
    for (regDictIter i = registers.begin(); i != registers.end(); i++) {
        switch(i.currval().origValueSpilled_) {
        case registerSlot::unspilled:
            break;
         case registerSlot::stackPointer: {
             assert(i.currval().saveOffset <= numPushed);
             pushedReg[i.currval().saveOffset] = i.currkey();
             break;
        }
        case registerSlot::framePointer:
            restoreRegister(i.currkey(), gen, noCost);
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
    
    // Oh, right. FP and SPR. :)
    for (regDictIter i = fpRegisters.begin(); i != fpRegisters.end(); i++) {
        restoreRegister(i.currkey(), gen, noCost);
    }
    for (regDictIter i = spRegisters.begin(); i != spRegisters.end(); i++) {
        restoreRegister(i.currkey(), gen, noCost);
    }
    
#if defined(arch_x86) || defined(arch_x86_64)
    if (currStackPointer > 0) {
        gen.codeEmitter()->emitAdjustStackPointer(currStackPointer, gen);
    }
#endif
    currStackPointer = 0;
#endif
    assert(0);
    return true;
}

bool registerSpace::restoreRegister(Register, codeGen &, bool /*noCost*/) 
{
#if 0
    // We can get an index > than the number of registers - we use those as fake
    // slots for (e.g.) flags register.
    // TODO: push register info and methods into a register slot class... hey....

    if (registers[reg].origValueSpilled_ == registerSlot::unspilled) return true;
    if (registers[reg].origValueSpilled_ == registerSlot::stackPointer) {
        // We don't know how to handle this in an individual restore yet...
        return false;
    }
    
    if (!readRegister(gen, reg, reg))
        return false;
    
    registers[reg].mustRestore = false;
    registers[reg].needsSaving = true;
    registers[reg].origValueSpilled_ = registerSlot::unspilled;
#endif
    assert(0);
    return true;
}

bool registerSpace::popRegister(Register, codeGen &, bool) {
#if 0
    assert(!registers[reg].offLimits);
    
    // Make sure we're at the right point... currStackPointer should
    // be 1 greater than the register.
    while (currStackPointer > (registers[reg].saveOffset+1)) {
        emitV(loadRegOp, 0, 0, registers[reg].number, gen, noCost);
        currStackPointer--;
    }
    
    assert((currStackPointer == (registers[reg].saveOffset-1)) ||
           (currStackPointer == (registers[reg].saveOffset+1)));
    
    // TODO for other platforms that build a stack frame for saving
    
    emitV(loadRegOp, 0, 0, registers[reg].number, gen, noCost);
    
    registers[reg].mustRestore = false; // Need to restore later
    registers[reg].needsSaving = true; // And don't save at func call (?)
    registers[reg].origValueSpilled_ = registerSlot::unspilled;
    registers[reg].saveOffset = 0;

    // Push architecture, so popping modified the SP
    currStackPointer--;
#endif
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
                                        unsigned size) {
#if !defined(arch_x86_64) && !defined(arch_power)
    emitLoadPreviousStackFrameRegister((Address)source,
                                       destination,
                                       gen,
                                       size,
                                       true);
    return true;
#else
    // Real version that uses stored information

#if defined(arch_x86_64) 
    // If we're in 32-bit mode, use emitLoadPrevious...
    if (gen.addrSpace()->getAddressWidth() == 4) {
        emitLoadPreviousStackFrameRegister((Address) source,
                                           destination,
                                           gen,
                                           size,
                                           true);
        return true;
    }
#endif

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
        gen.codeEmitter()->emitMoveRegToReg(src, dest, gen);
        return true;
        break;
    case registerSlot::framePointer: {
        registerSlot *frame = registers_[FRAME_POINTER];
        assert(frame);

        // We can't use existing mechanisms because they're all built
        // off the "non-instrumented" case - emit a load from the
        // "original" frame pointer, whereas we want the current one. 
        gen.codeEmitter()->emitLoadRelative(dest, src->saveOffset, frame, gen);
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
                                         unsigned size) {
#if !defined(arch_x86_64) && !defined(arch_power)
    emitStorePreviousStackFrameRegister((Address) destination,
                                        source,
                                        gen,
                                        size,
                                        true);
    return true;
#else

#if defined(arch_x86_64) 
    // If we're in 32-bit mode, use emitLoadPrevious...
    if (gen.addrSpace()->getAddressWidth() == 4) {
        emitStorePreviousStackFrameRegister((Address) destination,
                                            source,
                                            gen,
                                            size,
                                            true);
        return true;
    }
#endif

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
        registerSlot *frame = registers_[FRAME_POINTER];
        assert(frame);

        // When this register was saved we stored its offset from the base pointer.
        // Use that to load it. 
        gen.codeEmitter()->emitStoreRelative(src, dest->saveOffset, frame, gen);
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

    if (s->spilledState != registerSlot::unspilled) {
        // Things to do... add this check in, yo. Right now we don't clean
        // properly.
        
        assert(0);
    }

    s->liveState = registerSlot::spilled;

    s->spilledState = registerSlot::framePointer;
    s->saveOffset = offsetFromFP;
    return true;
}

void registerSlot::debugPrint(char *prefix) {
    if (!dyn_debug_regalloc) return;

	if (prefix) fprintf(stderr, "%s", prefix);
	fprintf(stderr, "Num: %d, name %s, type %s, refCount %d, liveState %s, beenUsed %d, initialState %s, offLimits %d, keptValue %d\n",
                number, 
                name.c_str(),
                (type == GPR) ? "GPR" : ((type == FPR) ? "FPR" : "SPR"),
                refCount, 
                (liveState == live ) ? "live" : ((liveState == spilled) ? "spilled" : "dead"),
                beenUsed, 
                (initialState == deadAlways) ? "always dead" : ((initialState == deadABI) ? "ABI dead" : "always live"),
                offLimits, 
                keptValue);
}

void registerSpace::debugPrint() {
    if (!dyn_debug_regalloc) return;

	// Dump out our data
	fprintf(stderr, "Beginning debug print of registerSpace at %p...", this);
	fprintf(stderr, "GPRs: %d, FPRs: %d, SPRs: %d\n", 
                GPRs_.size(), FPRs_.size(), SPRs_.size());
	fprintf(stderr, "Stack pointer is at %d\n",
                currStackPointer);
	fprintf(stderr, "Register dump:");
	fprintf(stderr, "=====GPRs=====\n");
	for (unsigned i = 0; i < GPRs_.size(); i++) {
            GPRs_[i]->debugPrint("\t");
        }
	for (unsigned i = 0; i < FPRs_.size(); i++) {
            FPRs_[i]->debugPrint("\t");
        }
	for (unsigned i = 0; i < SPRs_.size(); i++) {
            SPRs_[i]->debugPrint("\t");
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
        if (FPRs_[i]->liveState == registerSlot::dead)
            return true;
    }
    return false;
}

bool registerSpace::anyLiveSPRsAtEntry() const {
    for (unsigned i = 0; i < SPRs_.size(); i++) {
        if (SPRs_[i]->liveState == registerSlot::dead)
            return true;
    }
    return false;
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

bitArray registerSpace::getBitArray()  {
#if defined(arch_x86) || defined(arch_x86_64)
    return bitArray(REGNUM_LAST+1);
#elif defined(arch_power)
    return bitArray(lastReg);
#else
    assert(0);
    return bitArray();
#endif
}

#endif


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

