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

    // And record this function as being modified
    proc()->addModifiedFunction(func());
    
    return miniT;
}

bool instPoint::replaceCode(AstNodePtr ast) {
   // TODO
   assert(0);
   return true;
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

    int_block *bbl = func->findOneBlockByAddr(addr);
    if (!bbl) {
        inst_printf("Address not in known code, ret null\n");
        fprintf(stderr, "%s[%d]: Address not in known code, ret null\n", FILE__, __LINE__);
        return NULL;
    }
    int_block *block = bbl;
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
    if (block != bbl) {
        fprintf(stderr, "%s[%d]: Address not in original basic block instance\n", FILE__, __LINE__);
        return NULL;
    }
    if (!proc->isValidAddress(bbl->start())) return NULL;

    const unsigned char* buffer = reinterpret_cast<unsigned char*>(proc->getPtrToInstruction(bbl->start()));
    InstructionDecoder decoder(buffer, bbl->size(), proc->getArch());
    Instruction::Ptr i;
    Address currentInsn = bbl->start();
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
    
    if (!commonIPCreation(newIP)) {
        delete newIP;
        inst_printf("Failed common IP creation, ret null\n");
        return NULL;
    }
    
    func->addArbitraryPoint(newIP);

    return newIP;
}
 
bool instPoint::commonIPCreation(instPoint *ip) {

    // But tell people we exist.
    ip->func()->registerInstPointAddr(ip->addr(), ip);

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

    proc()->relocate();

    return mini;
}

int instPoint_count = 0;

instPoint::instPoint(AddressSpace *proc,
                     Dyninst::InstructionAPI::Instruction::Ptr insn,
                     Address addr,
                     int_block *block) :
    instPointBase(insn, 
                otherPoint),
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
                     int_block *block) :
        instPointBase(img_p->insn(),
                  img_p->getPointType(),
                  img_p->id()),
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
    img_p->owners.insert(this);
}

// Copying over from fork
instPoint::instPoint(instPoint *parP,
                     int_block *child,
                     process *childP) :
        instPointBase(parP->insn(),
                  parP->getPointType(),
                  parP->id()),
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
    
    int_block *bbi = func->findOneBlockByAddr(absAddr);
    if (!bbi) return NULL; // Not in the function...

    newIP = new instPoint(func->proc(),
                          img_p,
                          absAddr,
                          bbi);
    
    if (!commonIPCreation(newIP)) {
        delete newIP;
        return NULL;
    }

    return newIP;
}

instPoint *instPoint::createForkedPoint(instPoint *parP,
                                        int_block *childB,
                                        process *childP) {
    int_function *func = childB->func();
    instPoint *existingInstP = func->findInstPByAddr(parP->addr());
    if (existingInstP) {
       //One instPoint may be covering multiple instPointTypes, e.g.
       // a one instruction function with an entry and exit point at
       // the same point.
       assert(existingInstP->block() == childB);
       return existingInstP; 
    }

    // Make a copy of the parent instPoint. We don't have multiTramps yet,
    // which is okay; just get the ID right.
    instPoint *newIP = new instPoint(parP, childB, childP);

    func->registerInstPointAddr(newIP->addr(), newIP);

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
    if (preBaseTramp_) delete preBaseTramp_;
    if (postBaseTramp_) delete postBaseTramp_;
    if (targetBaseTramp_) delete targetBaseTramp_; 
}



bool instPoint::instrSideEffect(Frame &frame)
{
   return false;

#if 0
   // Unimplemented - keeping for reference
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
#endif
}

instPoint::catchup_result_t instPoint::catchupRequired(Address pc,
                                                       miniTramp *mt,
                                                       bool active) 
{
   // Unimplemented!
   return noMatch_c;

#if 0
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
#endif
}

int_block *instPoint::block() const { 
    assert(block_);
    return block_;
}

int_function *instPoint::func() const {
    return block()->func();
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


void instPoint::setBlock( int_block* newBlock )
{
    // update block (not sure what to do with instPointInstance block mappings)
    block_ = newBlock;
}

Address instPoint::getSavedTarget()
{
    if (0 == savedTarget_) { 
        // if the target is not set, see if there's a point at this
        // address in another function whose target has been saved,
        // retrieve that
        vector<ParseAPI::Function *> allfuncs;
        
        image_basicBlock *imgBlock = block()->llb();
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

    using namespace InstructionAPI;
    InstructionDecoder decoder
        ( func()->obj()->getPtrToInstruction(addr()),
          ( func()->obj()->codeAbs() + func()->obj()->imageSize() ) - addr(),
          func()->proc()->getArch() );
    Instruction::Ptr curInsn = decoder.decode();
    return c_ReturnInsn == curInsn->getCategory();
}

// returns false if the point was already resolved
bool instPoint::setResolved()
{
    if (img_p_->isUnresolved()) {
        img_p_->setUnresolved(false);
        func()->setPointResolved( this );
        return true;
    }

    return false;
}
