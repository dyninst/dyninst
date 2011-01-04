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

// $Id: instPoint.C,v 1.55 2008/09/08 16:44:03 bernat Exp $
// instPoint code


#include <assert.h>
#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/util.h"
#include "common/h/stats.h"
#include "dyninstAPI/src/debug.h"
#include "dyninstAPI/src/instPoint.h"
#include "dyninstAPI/src/miniTramp.h"
#include "dyninstAPI/src/baseTramp.h"

#if defined(cap_instruction_api)
#include "instructionAPI/h/InstructionDecoder.h"
using namespace Dyninst::InstructionAPI;
Dyninst::Architecture instPointBase::arch = Dyninst::Arch_none;

#else
#include "parseAPI/src/InstrucIter.h"
#endif // defined(cap_instruction_api)

#include "dyninstAPI/src/function.h"
#include "dyninstAPI/src/image-func.h"
#include "common/h/arch.h"
#include "dyninstAPI/src/mapped_object.h"
#include "dyninstAPI/src/emitter.h"
#if defined(arch_x86_64)
// For 32/64-bit mode knowledge
#include "dyninstAPI/src/emit-x86.h"
#endif

unsigned int instPointBase::id_ctr = 1;

dictionary_hash <std::string, unsigned> primitiveCosts(::Dyninst::stringhash);

#if defined(rs6000_ibm_aix4_1)
  extern void resetBRL(process *p, Address loc, unsigned val); //inst-power.C
  extern void resetBR( process *p, Address loc);               //inst-power.C
#endif

miniTramp *instPoint::addInst(AstNodePtr ast,
                              callWhen when,
                              callOrder order,
                              bool trampRecursive,
                              bool noCost) {
    // This only adds a new minitramp; code generation and any actual
    // _work_ is put off until later.

    baseTramp *baseT = getBaseTramp(when);
    if (!baseT) return NULL;

    // Will complain if we have multiple miniTramps that don't agree
    baseT->setRecursive(trampRecursive);
    
    miniTramp *miniT = new miniTramp(when,
                                     ast,
                                     baseT,
                                     noCost);
    
    assert (miniT);
    
    // Sets prev and next members of the mt
    if (!baseT->addMiniTramp(miniT, order)) {
        inst_printf("Basetramp failed to add miniTramp, ret false\n");
        delete miniT;
        return NULL;
    }

    hasAnyInstrumentation_ = true;
    hasNewInstrumentation_ = true;
    
    return miniT;
}

bool instPoint::replaceCode(AstNodePtr ast) {
    // We inject a "replacedInstruction" into all known multitramps,
    // then trigger generation and installation.
    
    // Actually, the multiTramp is set up to pull information out
    // of the instPoints (rather than "push"), so we set up our 
    // data structures and let 'er rip.

    replacedCode_ = ast;

    // Set flags appropriately...
    hasAnyInstrumentation_ = true;
    hasNewInstrumentation_ = true;

    pdvector<instPoint *> dontcare;

    // The better way to do this would be to hand the "go!" logic
    // back up to BPatch. However, this is a bigger change than I want
    // to make right now...

    return func()->performInstrumentation(false, dontcare);

#if 0    
    if (!generateInst()) {
        fprintf(stderr, "Code generation failed in replaceCode, ret false\n");
        return false;
    }

    if (!installInst()) {
        fprintf(stderr, "Code installation failed in replaceCode, ret false\n");
        return false;
    }

    if (!linkInst()) {
        fprintf(stderr, "Code link failed in replaceCode, ret false\n");
        return false;
    }

    return true;
#endif
}

// Get the appropriate base tramp structure. Cannot rely on
// multiTramps existing.

baseTramp *instPoint::getBaseTramp(callWhen when) 
{
	switch(when) {
		case callPreInsn:
			if (!preBaseTramp_) 
			{
				preBaseTramp_ = new baseTramp(this, when);
			}
			return preBaseTramp_;
			break;
		case callPostInsn:
			if (!postBaseTramp_) 
			{
				postBaseTramp_ = new baseTramp(this, when);
			}
			return postBaseTramp_;
			break;
		case callBranchTargetInsn:
			if (!targetBaseTramp_) 
			{
				targetBaseTramp_ = new baseTramp(this, when);
			}
			return targetBaseTramp_;
			break;
		default:
			assert(0);
			break;
	}
	return NULL;
}

bool instPoint::match(Address a) const { 
	if (a == addr()) return true;

	for (unsigned i = 0; i < instances.size(); i++)
		if (instances[i]->addr() == a)
			return true;
	return false;
}

instPoint *instPoint::createArbitraryInstPoint(Address addr, 
		AddressSpace *proc,
		int_function *func) 
{
	// See if we get lucky
	if (!func) 
		return NULL;

	//Create all non-arbitrary instPoints before creating arbitrary ones.
	func->funcEntries();
	func->funcExits();
	func->funcCalls();
  
  inst_printf("Creating arbitrary point at 0x%x\n", addr);
  instPoint *newIP = func->findInstPByAddr(addr);
  if (newIP) return newIP;

  // Check to see if we're creating the new instPoint on an
  // instruction boundary. First, get the instance...

    bblInstance *bbl = func->findBlockInstanceByAddr(addr);
    if (!bbl) {
        inst_printf("Address not in known code, ret null\n");
        fprintf(stderr, "%s[%d]: Address not in known code, ret null\n", FILE__, __LINE__);
        return NULL;
    }
    int_basicBlock *block = bbl->block();
    assert(block);

    // Some blocks cannot be relocated; since instrumentation requires
    // relocation of the block, don't even bother.
    if(!block->llb()->canBeRelocated())
    {
        inst_printf("Address is in unrelocatable block, ret null\n");
        return NULL;
    }    

    // For now: we constrain the address to be in the original instance
    // of the basic block.
    if (block->origInstance() != bbl) {
        fprintf(stderr, "%s[%d]: Address not in original basic block instance\n", FILE__, __LINE__);
        return NULL;
    }
#if defined(cap_instruction_api)
    if (!proc->isValidAddress(bbl->firstInsnAddr())) return NULL;

    const unsigned char* buffer = reinterpret_cast<unsigned char*>(proc->getPtrToInstruction(bbl->firstInsnAddr()));
    InstructionDecoder decoder(buffer, bbl->getSize(), proc->getArch());
    Instruction::Ptr i;
    Address currentInsn = bbl->firstInsnAddr();
    while((i = decoder.decode()) && (currentInsn < addr))
    {
        currentInsn += i->size();
    }
    if(currentInsn != addr)
    {
        inst_printf("Unaligned try for instruction iterator, ret null\n");
            fprintf(stderr, "%s[%d]: Unaligned try for instruction iterator, ret null\n", FILE__, __LINE__);
            return NULL; // Not aligned
    }
    newIP = new instPoint(proc,
                          i,
                          addr,
                          block);
#if defined(arch_sparc)
#error "Instruction API not yet implemented for SPARC, cap_instruction_api is illegal"
#endif // defined(arch_sparc)
#else
    InstrucIter newIter(bbl->firstInsnAddr(),bbl->getSize(),bbl->proc());
    while ((*newIter) < addr) newIter++;
    if (*newIter != addr) {
        inst_printf("Unaligned try for instruction iterator, ret null\n");
        fprintf(stderr, "%s[%d]: Unaligned try for instruction iterator at %lx (block start %lx), ret null\n", 
		FILE__, __LINE__, addr, bbl->firstInsnAddr());
        return NULL; // Not aligned
    }
#if defined(arch_sparc)
    // Can't instrument delay slots
    if (newIter.hasPrev()) {
      InstrucIter prevInsn(newIter);
      prevInsn--;      
      if (prevInsn.getInstruction().isDCTI()) {
	inst_printf("%s[%d]:  can't instrument delay slot\n", FILE__, __LINE__);
	fprintf(stderr, "%s[%d]:  can't instrument delay slot\n", FILE__, __LINE__);
	return NULL;
      }
    }
#endif // defined(arch_sparc)
    newIP = new instPoint(proc,
                          newIter.getInstruction(),
                          addr,
                          block);
#endif // defined(cap_instruction_api)
    
    if (!commonIPCreation(newIP)) {
        delete newIP;
        inst_printf("Failed common IP creation, ret null\n");
        return NULL;
    }
    
    func->addArbitraryPoint(newIP);

    return newIP;
}
 
bool instPoint::commonIPCreation(instPoint *ip) {
    // Delay until generation....
    //newIP->updateInstances();

    // But tell people we exist.
    ip->func()->registerInstPointAddr(ip->addr(), ip);

    return true;
}

// We do on-the-fly generation of Instances... as a result, we may have
// versions of functions (frex, relocation) that do not have associated
// Instances. This function updates the Instance list and makes sure
// that everything works right.

bool instPoint::updateInstances() {
    if (func()->version() == funcVersion)
        return false;

    // Otherwise there's work to do...

    if (updateInstancesBatch()) 
        updateInstancesFinalize();

    return true;
}

// Create the instPoint data structures, but don't do any instrumentation
// setup. Returns true if there is further work to be done.

bool instPoint::updateInstancesBatch() {
    unsigned i;

    reloc_printf("updateInstancesBatch for instPoint at 0x%lx\n",
                 addr());

    if (func()->version() == funcVersion) {
        reloc_printf(".... func version %d == our version %d, no work, returning\n",
                     func()->version(), funcVersion);
        return false;
    }
    else if (func()->version() < funcVersion) {
        reloc_printf("DEBUG: func version %d, our version %d, block instances %d, our instances %d\n",
                func()->version(), funcVersion, 
                block()->instances().size(),
                instances.size());
        const pdvector<bblInstance *> &bbls = block()->instances();
        assert(bbls.size() <= instances.size());
        for (unsigned i = instances.size(); i > bbls.size(); i--) {
            instPointInstance *inst = instances[i-1];
            instances.pop_back();
            // Delete...
            func()->unregisterInstPointAddr(inst->addr(), this);
        }

        // Safety check....
        for (unsigned i = 0; i < instances.size(); i++) {
            reloc_printf("%s[%d]: checking IPI block %p against block %p, entry %d\n",
                         FILE__, __LINE__, instances[i]->block(), bbls[i], i);
            assert(instances[i]->block() == bbls[i]);
        }

        funcVersion = func()->version();
        return false;
    }
    else {
        // For each instance of the basic block we're attached to,
        // make an instPointInstance with the appropriate addr
        // Might be smaller as well...
        
        const pdvector<bblInstance *> &bbls = block()->instances();
        reloc_printf("Func version > our version, adding instances (us %d, func %d\n",
                     bbls.size(), instances.size());
        assert(instances.size() < bbls.size());
        for (i = instances.size(); i < bbls.size(); i++) {
            bblInstance *bblI = bbls[i];
            
            Address newAddr = bblI->equivAddr(0, addr());

            // However, check if we can do a multiTramp at this point (as we may have
            // overwritten with a jump). If we can't, then skip the instance.
            unsigned multiID_ = multiTramp::findOrCreateMultiTramp(newAddr, bblI);
            reloc_printf("... found multi ID %d for addl instance %d\n",
                         multiID_, i);
            if (multiID_) {
                instPointInstance *ipInst = new instPointInstance(newAddr,
                                                                  bblI,
                                                                  this);
                
                instances.push_back(ipInst);
                // Register with the process before asking for a multitramp
                inst_printf("Registering IP %p at 0x%lx (%d)\n",
                            this, newAddr, i);
                func()->registerInstPointAddr(newAddr, this);
            }
        }
        funcVersion = func()->version();
        return true;
    }
    return false;
}

bool instPoint::updateInstancesFinalize() {
    // We need all instances to stay in step; so if the first (default)
    // instance is generated/installed/linked, then make sure any new
    // instances are the same.
    
    // If we can't be instrumented - well, we shouldn't have an instPoint
    // here, but oh well. Just return.
    if (!instances.size()) return true;
    
    
    // Check whether there's something at my address...
    for (unsigned i = 0; i < instances.size(); i++) {
        if (!instances[i]->multi()) {
            instances[i]->multiID_ = multiTramp::findOrCreateMultiTramp
                ( instances[i]->addr() , instances[i]->block() );
            if (instances[i]->multi()) {
                assert( instances[i]->func() == instances[i]->multi()->func() );
                if (shouldGenerateNewInstances_) {
                    instances[i]->multi()->generateMultiTramp();
                }
                if (shouldInstallNewInstances_) {
                    instances[i]->multi()->installMultiTramp();
                }
                if (shouldLinkNewInstances_) {
                    instances[i]->multi()->linkMultiTramp();
                }
            }
        }
    }
    return true;
}

// Blah blah blah...
miniTramp *instPoint::instrument(AstNodePtr ast,
                                 callWhen when,
                                 callOrder order,
                                 bool trampRecursive,
                                 bool noCost) {
    miniTramp *mini = addInst(ast, when, order, trampRecursive, noCost);
    if (!mini) {
        cerr << "instPoint::instrument: failed addInst, ret NULL" << endl;
        return NULL;
    }

    pdvector<instPoint *> ignored;
    func()->performInstrumentation(false,
                                   ignored);

    // Obsolete version below... we now use function-level control.
#if 0
    if (!generateInst()) {
        cerr << "instPoint::instrument: failed generateInst, ret NULL" << endl;
        return NULL;
    }

    if (!installInst()) {
        cerr << "instPoint::instrument: failed installInst, ret NULL" << endl;
        return NULL;
    }

    if (!linkInst()) {
        cerr << "instPoint::instrument: failed linkInst, ret NULL" << endl;
        return NULL;
    }
#endif
    return mini;
}

// Generate the correct code for all baseTramps touched by the
// miniTramps at this point. May cause regeneration of the multiTramps
// as well. Complicated.
// The code generated by this is explicitly _not_ injected into the process;
// that is, while it might be copied into the address space, it is not linked
// by any jumps. This allows for us to atomically add multiple different
// instPoints at the same time.

// Return value... we used to return false if any generation failed. However,
// if we relocate a function then generating into the entry point of the original
// will fail, and that's 'bad'. So we now return true if anyone succeeded.

// Generating the initial instance (for the original function) may trigger relocation
// of the function. So we special-case the first instance to determine whether relocation
// is necessary. It's possible that later instances will want relocation as well; however,
// this should only happen if the initial instance required it as well. 

instPoint::result_t instPoint::generateInst() {
    stats_instru.startTimer(INST_GENERATE_TIMER);
    stats_instru.incrementCounter(INST_GENERATE_COUNTER);
    updateInstances();

    bool desireRelocation = false;
    bool success = false;
    bool noMultiTramp = false;

    for (unsigned i = 0; i < instances.size(); i++) {
        switch (instances[i]->generateInst()) {
        case instPointInstance::noMultiTramp:
            // This can happen if there is another modification
            // type at this location; this is only a failure
            // if nothing else covers it.
            noMultiTramp = true;
            break;
        case instPointInstance::instOfSharedBlock:
            // We've instrumented a shared block, which is not desired.
            // We want to force relocation of this function to ensure
            // that we can remove the sharing. This may indirectly cause
            // relocation of the sharing functions, but we don't care about
            // that. 
            desireRelocation = true;
            break;
        case instPointInstance::mTrampTooBig:
            // We need to branch, but the branch is bigger than
            // the block. Oops. 
            // We don't do any work here; instead, we'll pass this up the
            // chain and see how big the function needs to be from the function
            // level.
            desireRelocation = true;
            break;
        case instPointInstance::generateFailed:
            break;
        case instPointInstance::generateSucceeded:
            success = true;
            break;
        case instPointInstance::pointPreviouslyModified:
            // This case doesn't matter.
            break;
        default:
            assert(0 && "Impossible case in switch"); 
            break;
        }
    }
    shouldGenerateNewInstances_ = true;
    stats_instru.stopTimer(INST_GENERATE_TIMER);

    if (noMultiTramp) assert(0);
    if (desireRelocation) return tryRelocation;
    if (success) return generateSucceeded;
    return generateFailed;
}

// See above return value comment...

instPoint::result_t instPoint::installInst() {
    stats_instru.startTimer(INST_INSTALL_TIMER);
    stats_instru.incrementCounter(INST_INSTALL_COUNTER);
    bool success = false;
    bool noMT = false;
    for (unsigned i = 0; i < instances.size(); i++) {
        switch (instances[i]->installInst()) {
        case instPointInstance::installSucceeded:
            success = true;
            break;
        case instPointInstance::installFailed:
            break;
        case instPointInstance::noMultiTramp:
            noMT = true;
            break;
        default:
            assert(0);
        }
    }
    shouldInstallNewInstances_ = true;
    stats_instru.stopTimer(INST_INSTALL_TIMER);

    if (success)
        return installSucceeded;
    else if (noMT)
        return wasntGenerated;
    else
        return installFailed;
}

// Return false if the PC is within the jump range of any of our
// multiTramps
bool instPoint::checkInst(pdvector<Address> &checkPCs) 
{
    
    for (unsigned sI = 0; sI < checkPCs.size(); sI++) {
        Address pc = checkPCs[sI];
        for (unsigned iI = 0; iI < instances.size(); iI++) {
            multiTramp *mt = instances[iI]->multi();
            // No multi -> not installed.
            if (!mt) continue;
            if ((pc > mt->instAddr()) &&
                (pc < (mt->instAddr() + mt->instSize()))) {
                // We have a conflict. Now, we may still be able to make this 
                // work; if we're not conflicting on the actual branch, we
                // may have trap-filled the remainder which allows us to
                // catch and transfer.
                if (pc < (mt->instAddr() + mt->branchSize())) {
                    // We're in the jump area, conflict.
                    fprintf(stderr, "MT conflict (MT from 0x%p to 0x%p, 0x%p to 0x%p dangerous), PC 0x%p\n",
                            (void *)mt->instAddr(),
                            (void *)(mt->instAddr() + mt->instSize()), 
                            (void *)mt->instAddr(),
                            (void *)(mt->instAddr() + mt->branchSize()),
                            (void *)pc);
                    return false;
                }
            }
        }
    }
#if defined(cap_relocation)
    // Yay check relocation
    if (!func()->relocationCheck(checkPCs))
        return false;
#endif

    return true;
}

// See comment in generateInst w.r.t. return value
//  If update_trap_table is true then update the mutatee's trap
//   table with any traps that we added.  We may not want to do
//   this yet if we're dealing with insertion sets, since large
//   updates are more efficient than smaller ones.
instPoint::result_t instPoint::linkInst(bool update_trap_table) {
    bool success = false;
    bool noMT = false;
    stats_instru.startTimer(INST_LINK_TIMER);
    stats_instru.incrementCounter(INST_LINK_COUNTER);

    for (unsigned i = 0; i < instances.size(); i++) {
        switch (instances[i]->linkInst()) {
        case instPointInstance::linkSucceeded:
            success = true;
            break;
        case instPointInstance::linkFailed:
            break;
        case instPointInstance::noMultiTramp:
            noMT = true;
            break;
        default:
            assert(0);
        }
    }

    if (update_trap_table) {
      proc()->trapMapping.flush();
    }

    shouldLinkNewInstances_ = true;

    stats_instru.stopTimer(INST_LINK_TIMER);

    // It's been installed, so nothing new.
    hasNewInstrumentation_ = false;
    
    if (success)
        return linkSucceeded;
    else if (noMT)
        return wasntInstalled;
    else
        return linkFailed;
}

instPointInstance *instPoint::getInstInstance(Address addr) {
    for (unsigned i = 0; i < instances.size(); i++) {
        if (instances[i]->addr() == addr)
            return instances[i];
    }
    return NULL;
}

int instPoint_count = 0;

instPoint::instPoint(AddressSpace *proc,
#if defined(cap_instruction_api)
    Dyninst::InstructionAPI::Instruction::Ptr insn,
#else
                        instruction insn,
#endif
                     Address addr,
                     int_basicBlock *block) :
    instPointBase(insn, 
                otherPoint),
    funcVersion(-1),
    callee_(NULL),
    isDynamic_(false),
    preBaseTramp_(NULL),
    postBaseTramp_(NULL),
    targetBaseTramp_(NULL),
    replacedCode_(),
    proc_(proc),
    img_p_(NULL),
    block_(block),
    addr_(addr),
    shouldGenerateNewInstances_(false),
    shouldInstallNewInstances_(false),
    shouldLinkNewInstances_(false),
    hasNewInstrumentation_(false),
    hasAnyInstrumentation_(false),
    savedTarget_(0)
{
#if defined(ROUGH_MEMORY_PROFILE)
    instPoint_count++;
    if ((instPoint_count % 10) == 0)
        fprintf(stderr, "instPoint_count: %d (%d)\n",
                instPoint_count, instPoint_count*sizeof(instPoint));
#endif
}

// Process specialization of a parse-time instPoint
instPoint::instPoint(AddressSpace *proc,
                     image_instPoint *img_p,
                     Address addr,
                     int_basicBlock *block) :
        instPointBase(img_p->insn(),
                  img_p->getPointType(),
                  img_p->id()),
    funcVersion(-1),
    callee_(NULL),
    isDynamic_(img_p->isDynamic()),
    preBaseTramp_(NULL),
    postBaseTramp_(NULL),
    targetBaseTramp_(NULL),
    replacedCode_(),
     proc_(proc),
    img_p_(img_p),
    block_(block),
    addr_(addr),
    shouldGenerateNewInstances_(false),
    shouldInstallNewInstances_(false),
    shouldLinkNewInstances_(false),
    hasNewInstrumentation_(false),
    hasAnyInstrumentation_(false),
    savedTarget_(0)
{
#if defined(ROUGH_MEMORY_PROFILE)
    instPoint_count++;
    if ((instPoint_count % 10) == 0)
        fprintf(stderr, "instPoint_count: %d (%d)\n",
                instPoint_count, instPoint_count*sizeof(instPoint));
#endif
}

// Copying over from fork
instPoint::instPoint(instPoint *parP,
                     int_basicBlock *child,
                     process *childP) :
        instPointBase(parP->insn(),
                  parP->getPointType(),
                  parP->id()),
    funcVersion(parP->funcVersion),
    callee_(NULL), // Will get set later
    isDynamic_(parP->isDynamic_),
    preBaseTramp_(NULL),
    postBaseTramp_(NULL),
    targetBaseTramp_(NULL),
    replacedCode_(parP->replacedCode_),
    proc_(childP),
    img_p_(parP->img_p_),
    block_(child),
    addr_(parP->addr()),
    shouldGenerateNewInstances_(parP->shouldGenerateNewInstances_),
    shouldInstallNewInstances_(parP->shouldInstallNewInstances_),
    shouldLinkNewInstances_(parP->shouldLinkNewInstances_),
    hasNewInstrumentation_(parP->hasNewInstrumentation_),
    hasAnyInstrumentation_(parP->hasAnyInstrumentation_),
    savedTarget_(parP->savedTarget_)
{
}
                  

instPoint *instPoint::createParsePoint(int_function *func,
                                       image_instPoint *img_p) {    
    // Now we need the addr and block so we can toss this to
    // commonIPCreation.

    inst_printf("Creating parse point for function %s, type %d\n",
                func->symTabName().c_str(),
                img_p->getPointType());

   // Madhavi 2010: This condition is no longer true. We could have
   // instPoints above the function entry point that belongs to a function
   // (especially with shared code)
#if 0
    //assert(img_p->offset() >= func->ifunc->getOffset());
    if(img_p->offset() < func->ifunc()->getOffset()) {
        inst_printf("  -- Failed: image_instPoint offset %lx preceeds "
                    "image_func offset %lx: untested case\n",
                    img_p->offset(),func->ifunc()->getOffset());
        return NULL;
    }
#endif 
    Address offsetInFunc = img_p->offset() - func->ifunc()->getOffset();
    Address absAddr = offsetInFunc + func->getAddress();

    instPoint *newIP = func->findInstPByAddr(absAddr);
    if (newIP) {
       //fprintf(stderr, "WARNING: already have parsed point at addr 0x%lx\n",
       //absAddr);
       return NULL;
    }
    inst_printf("Parsed offset: 0x%x, in func 0x%x, absolute addr 0x%x\n",
                img_p->offset(),
                offsetInFunc,
                absAddr);
    
    int_basicBlock *block = func->findBlockByAddr(absAddr);
    if (!block) return NULL; // Not in the function...
    assert(block);

    newIP = new instPoint(func->proc(),
                          img_p,
                          absAddr,
                          block);
    
    if (!commonIPCreation(newIP)) {
        delete newIP;
        return NULL;
    }

    return newIP;
}

instPoint *instPoint::createForkedPoint(instPoint *parP,
                                        int_basicBlock *childB,
                                        process *childP) {
    int_function *func = childB->func();
    instPoint *existingInstP = func->findInstPByAddr(parP->addr());
    if (existingInstP) {
       //One instPoint may be covering multiple instPointTypes, e.g.
       // a one instruction function with an entry and exit point at
       // the same point.
       return existingInstP; 
    }

    // Make a copy of the parent instPoint. We don't have multiTramps yet,
    // which is okay; just get the ID right.
    instPoint *newIP = new instPoint(parP, childB, childP);

    // Add to the process
    if (parP->instances.size() == 0) {
        // We created but never actually instrumented. However, 
        // we still need to let the function know we exist, mimicing the
        // behavior in commonIPcreation
        func->registerInstPointAddr(newIP->addr(), newIP);
    }
    else {
        for (unsigned i = 0; i < parP->instances.size(); i++) {
            instPointInstance *pI = parP->instances[i];
            instPointInstance *nI = new instPointInstance(pI->addr_,
                                                          childB->instVer(i), 
                                                          newIP);
            // could also call childB->func()->findBlockInstance...
            
            nI->multiID_ = pI->multiID_;
            newIP->instances.push_back(nI);
            func->registerInstPointAddr(pI->addr_, newIP);
        }
    }

    // And make baseTramp-age. If we share one, the first guy
    // waits and the second guy makes, so we can be absolutely
    // sure that we've made instPoints before we go trying to 
    // make baseTramps.
    
    baseTramp *parPre = parP->preBaseTramp_;
    if (parPre) {
        assert(parPre->instP() == parP);
        
	newIP->preBaseTramp_ = new baseTramp(parPre, childP);
	newIP->preBaseTramp_->instP_ = newIP;
    }


    baseTramp *parPost = parP->postBaseTramp_;
    if (parPost) {
        assert(parPost->instP() == parP);
        
	newIP->postBaseTramp_ = new baseTramp(parPost, childP);
	newIP->postBaseTramp_->instP_ = newIP;
    }

    baseTramp *parTarget = parP->targetBaseTramp_;
    if (parTarget) {
        assert(parTarget->instP() == parP);

        // Unlike others, can't share, so make now.
        newIP->targetBaseTramp_ = new baseTramp(parTarget, childP);
        newIP->targetBaseTramp_->instP_ = newIP;
    }

    return newIP;
}    
    
    
instPoint::~instPoint() {
	for (unsigned i = 0; i < instances.size(); i++) {
        delete instances[i];
    }
    instances.clear();
    // callee isn't ours...
    // multitramps will get deleted themselves...

    

    if (preBaseTramp_) delete preBaseTramp_;
    if (postBaseTramp_) delete postBaseTramp_;
    if (targetBaseTramp_) delete targetBaseTramp_;
    
}



bool instPoint::instrSideEffect(Frame &frame)
{
    bool modified = false;
    
    for (unsigned i = 0; i < instances.size(); i++) {
        instPointInstance *target = instances[i];

        // May not exist if instrumentation was overridden by (say)
        // a relocated function.
        if (!target->multi()) {
            continue;
        }
        
        // Question: generalize into "if the PC is in instrumentation,
        // move to the equivalent address?" 
        // Sure....
        
        // See previous call-specific version below; however, this
        // _should_ work.

        Address newPC = target->multi()->uninstToInstAddr(frame.getPC());
        if (newPC) {
            if (!frame.setPC(newPC)) {
                mal_printf("setting active frame's PC from %lx to %lx %s[%d]\n", 
                           frame.getPC(), newPC, FILE__,__LINE__);
                frame.getLWP()->changePC(newPC,NULL);
            }
            if (frame.setPC(newPC)) 
                modified = true;
        }
        // That's if we want to move into instrumentation. Mental note:
        // check to see if we're handling return points correctly; we should
        // be. If calls ever end basic blocks, we'll have to fix this.
    }
    return modified;
}

instPoint::catchup_result_t instPoint::catchupRequired(Address pc,
                                                       miniTramp *mt,
                                                       bool active) 
{
    // If the PC isn't in a multiTramp that corresponds to one of
    // our instances, return noMatch_c

    // If the PC is in an older version of the current multiTramp,
    // return missed
    
    // If we're in a miniTramp chain, then hand it off to the 
    // multitramp.

    // Otherwise, hand it off to the multiTramp....
    codeRange *range = proc()->findOrigByAddr(pc);
    assert(range);

    multiTramp *rangeMT = range->is_multitramp();
    miniTrampInstance *rangeMTI = range->is_minitramp();

    if ((rangeMT == NULL) &&
        (rangeMTI == NULL)) {
        // We _cannot_ be in the jump footprint. So we missed.
        catchup_printf("%s[%d]: Could not find instrumentation match for pc at 0x%lx\n",
                       FILE__, __LINE__, pc);
        //range->print_range(pc);
        

        return noMatch_c;
    }

    if (rangeMTI) {
        // Back out to the multiTramp for now
        rangeMT = rangeMTI->baseTI->multiT;
    }

    assert(rangeMT != NULL);

    unsigned curID = rangeMT->id();

    catchup_printf("%s[%d]: PC in instrumentation, multiTramp ID %d\n", 
                   FILE__, __LINE__, curID);

    bool found = false;
    
    for (unsigned i = 0; i < instances.size(); i++) {
        catchup_printf("%s[%d]: checking instance %d against target %d\n",
                       FILE__, __LINE__, instances[i]->multiID(), curID);
        if (instances[i]->multiID() == curID) {
            found = true;
            // If not the same one, we replaced. Return missed.
            if (instances[i]->multi() != rangeMT) {
                catchup_printf("%s[%d]: Found multiTramp, pointers different - replaced code, ret missed\n",
                               FILE__, __LINE__);
                return missed_c;
            }
            else {
                // It is the same one; toss into low-level logic
                if (rangeMT->catchupRequired(pc, mt, active, range)) {
                    catchup_printf("%s[%d]: Found multiTramp, instance returns catchup required, ret missed\n",
                                   FILE__, __LINE__);
                    return missed_c;
                }
                else {
                    catchup_printf("%s[%d]: Found multiTramp, instance returns catchup unnecessary, ret not missed\n",
                                   FILE__, __LINE__);                    
                    return notMissed_c;
                }
            }
            break;
        }
    }
    
    assert(!found);

    // This means we must be in an old multiTramp... possibly one on the deleted
    // list due to replacement. Let's return that we missed.

    catchup_printf("%s[%d]: multiTramp instance not found, returning noMatch\n", FILE__, __LINE__);

    return missed_c;
}

int_basicBlock *instPoint::block() const { 
    assert(block_);
    return block_;
}

int_function *instPoint::func() const {
    return block()->func();
}

// allowTrap shouldn't really go here; problem is, 
// multiple instPoints might use the same multiTramp,
// and I'm not sure how to handle it.
instPointInstance::result_t instPointInstance::generateInst() {
    // Do all the hard work; this will create a complete
    // instrumentation structure and copy it into the address
    // space, but not link it up. We do that later. It may also
    // change the multiTramp we have a pointer to, though that's
    // handled through the multi() wrapper.    

    // Putting this in here for now...
    // We may be trying to instrument a point that was already modified
    // for another reason; in particular, a jump to a relocated function.
    // In this case we don't have a multiTramp, but we don't really
    // want one. I'm going to look that up here, since the state of the
    // data structures is _weird_ in this case. 

    if (!multi()) {//UNSAFE? this can retrieve the wrong multiTramp if there's 
                   //shared code
        multiID_ = multiTramp::findOrCreateMultiTramp(addr(),block());
    }
    if (!multi()) {
        if (multiID_ != 0) {
            // Okay, we have a multiTramp ID, but nothing matching it. Check to see if someone
            // already got to our block.
            codeRange *range = proc()->findModByAddr(block()->firstInsnAddr());
            if (range) {
                // See if it's a multiTramp
                if (!proc()->findMultiTrampByAddr(block()->firstInsnAddr())) {
                    // We have a non-multitramp-thingie. Return that we're not
                    // generating, and why.
                    return instPointInstance::pointPreviouslyModified;
                }
            }
        }
        return noMultiTramp;
    }
    multiTramp::mtErrorCode_t errCode = multi()->generateMultiTramp();

    if (errCode == multiTramp::mtError) {
        return generateFailed;
    }
    // Can and will set our multiTramp ID if there isn't one already;
    // if there is, will reuse that slot in the multiTramp dictionary.
    // This allows us to regenerate multiTramps without having to 
    // worry about changing pointers.

#if defined(cap_relocation)

    // We may need to relocate if either of the following is true:
    // 1) The block we instrumented is shared; we relocate to eliminate
    // the sharing.
    // 2) The branch to the tramp (e.g., "tramp size") is larger than
    // the block. 
    // 
    // Relocation is tricky to get right; in particular, we desire the following:
    // 1) Determine the relocation requirements of _all_ blocks in the function before
    // creating the prototype relocated function. This is an efficiency requirement.
    // 2) The state of all instances of an instPoint should be equivalent. 
    //
    // So we determine the relocation needs of the function here, and pass that
    // back to instPoint::generate; it can summarize the requirements for its
    // instances and hand that back up the chain. 

    if (block_->block()->needsRelocation()) {
        // We're part of a shared block.
        return instOfSharedBlock;
    }

    if (errCode == multiTramp::mtTryRelocation) {
        return mTrampTooBig;
    }

#endif

    return generateSucceeded;
}

instPointInstance::result_t instPointInstance::installInst() {
#if 0
#if defined(cap_relocation)
    // This is harmless to call if there isn't a relocation in-flight

    reloc_printf("%s[%d]: instPointInstance calling relocationInstall for primary func %s\n",
                 FILE__, __LINE__, func()->prettyName().c_str());

    func()->relocationInstall();

    // the original relocation may force others; install them too
    for(unsigned i=0; i < force_reloc.size(); i++)
    {
        reloc_printf("%s[%d]: instPointInstance calling relocationInstall for forced func %s\n",
                     FILE__, __LINE__, force_reloc[i]->prettyName().c_str());
        force_reloc[i]->relocationInstall();
    }
#endif
#endif

    if (!multi()) {
        // Alternative: keep a set of sequence #s for generated/
        // installed/linked. We tried to generate and failed (prolly
        // due to stepping on a relocated function), so fail here
        // but don't assert.
        return noMultiTramp;
    }

    // We now "install", that is copy the generated code into the 
    // addr space. This doesn't link.
    
    if (multi()->installMultiTramp() != multiTramp::mtSuccess) {
        return installFailed;
    }
    return installSucceeded;
}

instPointInstance::result_t instPointInstance::linkInst() {
#if 0
#if defined(cap_relocation)
    // This is ignored (for now), is handled in updateInstInstances...
    pdvector<codeRange *> overwrittenObjs;
    // This is harmless to call if there isn't a relocation in-flight
    func()->relocationLink(overwrittenObjs);

    for(unsigned i=0; i < force_reloc.size(); i++)
    {
        force_reloc[i]->relocationLink(overwrittenObjs);
    }
#endif
#endif

    if (!multi()) return noMultiTramp;
    
    if (multi()->linkMultiTramp() != multiTramp::mtSuccess) {
        return linkFailed;
    }
    return linkSucceeded;
}

multiTramp *instPointInstance::multi() const {
    if (multiID_ == 0)
        return NULL;
    return multiTramp::getMulti(multiID_, proc());
}

void instPointInstance::updateMulti(unsigned id) {
    if (multiID_)
        assert(id == multiID_);
    else {
        multiID_ = id;
        if (func() != multi()->func()) {
            multiTramp *mt = multi();
            fprintf(stderr,"ERROR: mismatch, func %p at %lx != multifunc %p "
                    "at %lx %s[%d]\n", func(), func()->getAddress(), 
                    mt->func(), mt->func()->getAddress(),FILE__,__LINE__);
        }
    }
}

AddressSpace *instPointInstance::proc() const {
    return point->proc();
}

int_function *instPointInstance::func() const {
    return point->func();
}

Address instPoint::callTarget() const {
    if (img_p_->callTarget() == 0) return 0;

    // We can have an absolute addr... lovely.
    if (img_p_->targetIsAbsolute())
        return img_p_->callTarget();

    // Return the shifted kind...
    return img_p_->callTarget() + func()->obj()->codeBase();
}

bool instPoint::optimizeBaseTramps(callWhen when) 
{
   baseTramp *tramp = getBaseTramp(when);

   if (tramp)
      return tramp->doOptimizations();

   return false;
}

std::string instPoint::getCalleeName()
{
   int_function *f = findCallee();
   if (f)
      return f->symTabName();
   return img_p_->getCalleeName();
}


void instPoint::setBlock( int_basicBlock* newBlock )
{
    // update block (not sure what to do with instPointInstance block mappings)
    block_ = newBlock;
    while (instances.size()) {
        // leave the original instPoint addr registered
        if (instances.size() != 1) {
            func()->unregisterInstPointAddr(instances.back()->addr(), this);
        }
        // but we don't want instPointInstances registered, the instrumentation
        //  should be gone and the instance will have incorrect block info
        instances.pop_back();
        
    }
    funcVersion = -1;
}

Address instPoint::getSavedTarget()
{
    if (0 == savedTarget_) { 
        // if the target is not set, see if there's a point at this
        // address in another function whose target has been saved,
        // retrieve that
        vector<ParseAPI::Function *> allfuncs;
        image_basicBlock *imgBlock = func()->findBlockByAddr(addr())->llb();
        imgBlock->getFuncs(allfuncs);
        if (allfuncs.size() > 1) {
            for(vector<ParseAPI::Function*>::iterator fIter = allfuncs.begin();
                fIter != allfuncs.end(); 
                fIter++) 
            {
                instPoint *curPoint = proc()->
                    findFuncByInternalFunc(dynamic_cast<image_func*>(*fIter))->
                    findInstPByAddr(addr());
                if (savedTarget_ !=0 && curPoint != NULL &&
                    savedTarget_ != curPoint->savedTarget_) 
                {
                    savedTarget_ = (Address) -1;
                }
                else if (curPoint != NULL) { 
                    savedTarget_ = curPoint->savedTarget_;
                }
            }
        }
    }
    return savedTarget_;
}

void instPoint::setSavedTarget(Address st_)
{
    Address newVal = st_;
    // savedTarget_ == 0 means it hasn't been set yet
    if (0 != savedTarget_ && savedTarget_ != st_) {
        mal_printf("WARNING!!! instPoint at %lx resolves to more than "
                "one target, %lx and now %lx, setting target to -1 %s[%d]\n", 
                addr_, savedTarget_, st_, FILE__,__LINE__);
        newVal = (Address)-1;
    }
    savedTarget_ = newVal;
}

bool instPoint::isReturnInstruction()
{
    if (ipType_ != functionExit) {
        return false;
    }
    if ( ! instances.size() ) {
        updateInstances();
    }

#if defined(cap_instruction_api)
    using namespace InstructionAPI;
    InstructionDecoder decoder
        ( func()->obj()->getPtrToInstruction(addr()),
          ( func()->obj()->codeAbs() + func()->obj()->imageSize() ) - addr(),
          func()->proc()->getArch() );
    Instruction::Ptr curInsn = decoder.decode();
    return c_ReturnInsn == curInsn->getCategory();
#else
    InstrucIter iter(addr(),proc());
    return iter.isAReturnInstruction();
#endif
}

// returns false if the point was already resolved
bool instPoint::setResolved()
{
    if (img_p_->isUnresolved()) {
        img_p_->setResolved();
        func()->setPointResolved( this );
        return true;
    }

    return false;
}

void instPoint::removeMultiTramps()
{
    for (unsigned iIdx=0; iIdx < instances.size(); iIdx++) {
        instPointInstance *curInst = instances[iIdx];
        if (curInst->multi()) {
            curInst->multi()->removeCode(NULL);
        }
    }
}

