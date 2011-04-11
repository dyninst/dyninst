/*
 * Copyright (c) 1996-2011 Barton P. Miller
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
 
// $Id: reloc-func.C,v 1.41 2008/09/08 16:44:05 bernat Exp $

#include "common/h/Types.h"

#include "function.h"
#include "process.h"

#include "debug.h"
#include "codeRange.h"
#include "dyninstAPI/src/instPoint.h"
#include "dyninstAPI/src/multiTramp.h"
#include "common/h/arch.h"
#include "instructionAPI/h/InstructionDecoder.h"
#include "instructionAPI/h/Instruction.h"
#include "dyninstAPI/src/mapped_object.h"
#include "dyninstAPI/src/patch.h"

#include "arch-forward-decl.h" // instruction

#include "dyn_detail/boost/make_shared.hpp"

class int_basicBlock;

// We'll also try to limit this to relocation-capable platforms
// in the Makefile. Just in case, though....
#include "reloc-func.h"



// And happy fun time. Relocate a function: make a physical copy somewhere else
// in the address space that will execute as the original function did. 
// This creates a somewhat inefficient new version; relocation is done
// for correctness, not efficiency.

// Input: a list of things to change when we relocate the function.

// TODO: which version do we relocate? ;)

// 29.Nov.05 Okay, so things just got a little trickier. When relocating
// a function, co-owner functions (those that share blocks with this function)
// must also be relocated if we will overwrite any of the shared blocks.

bool int_function::relocationGenerate(pdvector<funcMod *> &mods,
                                      int sourceVersion /* = 0 */,
                                      pdvector< int_function *> &needReloc)
{
    bool ret;

    reloc_printf("%s[%d]: RELOCATION GENERATE FOR %s\n",
                 FILE__, __LINE__, prettyName().c_str());

#if defined(os_aix)
    // Once we've relocated once, we're good... there's a new function version
    // near the heap. The code will blithely relocate a bazillion times, too. 
    if (version() > 0)
        return true;
#endif

    // process this function (with mods)
    ret = relocationGenerateInt(mods,sourceVersion,needReloc);

    // process all functions in needReloc list -- functions that have
    // been previously processed or actually relocated (installed and
    // linked) must be skipped!

    reloc_printf("%s[%d] after internal relocation generation, %d also need work\n",
                 FILE__, __LINE__, needReloc.size());

    for(unsigned i = 0; i < needReloc.size(); i++)
    {
        // if the function has previously been relocated (any stage,
        // not just installed and linked), skip.
      if(needReloc[i]->linkedVersion_ > 0) {
	reloc_printf("Skipping dependant relocation of %s: function already relocated\n",
		     needReloc[i]->prettyName().c_str());
	needReloc[i] = needReloc.back();
	needReloc.pop_back();
	i--; // reprocess  
      }
    }

    reloc_printf("%s[%d]: RELOCATION GENERATE FOR %s, returning %s, %d in needReloc\n",
                 FILE__, __LINE__, prettyName().c_str(), ret ? "true" : "false", needReloc.size());


    return ret;
}

Address int_function::allocRelocation(unsigned size_required)
{
   // AIX: we try to target the data heap, since it's near instrumentation; 
   // we can do big branches at the start of a function, but not within. 
   // So amusingly, function relocation probably won't _enlarge_ the function,
   // just pick it up and move it nearer instrumentation. Bring the mountain 
   // to Mohammed, I guess.
#if defined(os_aix)
   // Also, fork() instrumentation needs to go in data.
   return proc()->inferiorMalloc(size_required, dataHeap);
#elif defined(arch_x86_64)
   return proc()->inferiorMalloc(size_required, anyHeap, getAddress());
#else
   // We're expandin'
   return proc()->inferiorMalloc(size_required);
#endif
}

#if defined(cap_fixpoint_gen)
bool int_function::relocBlocks(Address baseInMutatee, 
                               pdvector<bblInstance *> &newInstances)
{
   pdvector<Address> addrs; //Parallel array to newInstances for storing current 
                            // addresses of basic blocks as we compute them

   Address curAddr = baseInMutatee;
   for (unsigned i=0; i<newInstances.size(); i++) {
     bblInstance *bbl = newInstances[i];
     reloc_printf("Initial address of block 0x%lx set to 0x%lx\n", 
                  bbl->block()->origInstance()->firstInsnAddr(),
                  curAddr);
     addrs.push_back(curAddr);
     if (i < (newInstances.size() - 1))
         curAddr += newInstances[i]->sizeRequired(newInstances[i+1]);
     else
         curAddr += newInstances[i]->sizeRequired(NULL);
   }

   for (;;) {
      reloc_printf("Computing address for all blocks\n");
      for (unsigned i=0; i<newInstances.size(); i++) {
         bblInstance *bbl = newInstances[i];
         bbl->setStartAddr(addrs[i]);
      }

      for (unsigned i=0; i<newInstances.size(); i++) {
         bblInstance *bbl = newInstances[i];
         reloc_printf("Fixpoint set block 0x%lx to 0x%lx (+%lu), generating...\n", 
                      bbl->block()->origInstance()->firstInsnAddr(),
                      addrs[i],
                      addrs[i] - baseInMutatee);
         if (i < (newInstances.size() - 1))
             bbl->generate(newInstances[i+1]);
         else
             bbl->generate(NULL);
         assert(!bbl->generatedBlock().hasPCRels());
         reloc_printf("Block 0x%lx generation done.  Used %lu bytes\n",
                      bbl->block()->origInstance()->firstInsnAddr(),
                      bbl->generatedBlock().used());
      }
         
      bool changeDetected = false;
      for (unsigned i=0; i<newInstances.size(); i++) {
         bblInstance *bbl = newInstances[i];

         if (i+1 == newInstances.size()) {
            //Size changes in the last block don't actually concern us
            continue;
         }

         Address newBlockEndAddr = addrs[i] + bbl->getSize();
         
         if (newBlockEndAddr > addrs[i+1])
         {
            //Uh-Oh.  Rare, but possible.  We'd previously shrunk this block, but 
            // the change in addresses (likely from previous blocks shrinking) caused 
            // us to have to grow the block again.  This is likely if the block is 
            // referencing an external object that isn't being relocated, and we were
            // shrunk too far away from that object.
            //We'll set the minSize of this block, so that we'll stop trying to shrink 
            // this one.  If we didn't do this, the fixpoint could threoritically 
            // infinite loop, as we could get into a scenario where we keep shrinking
            // then growing this block.
            bbl->minSize() = bbl->getSize();
         }
         if (newBlockEndAddr != addrs[i+1]) {
            changeDetected = true;
            reloc_printf("Fixpoint found block 0x%lx to be of size %u."
                         "\n\tMoving block from %lx (+%lu) to %lx (+%lu).\n", 
                         bbl->block()->origInstance()->firstInsnAddr(),
                         bbl->getSize(),
                         addrs[i+1],
                         addrs[i+1] - baseInMutatee,
                         newBlockEndAddr,
                         newBlockEndAddr - baseInMutatee);
            addrs[i+1] = newBlockEndAddr;
         }
      }
      
      if (!changeDetected) {
         reloc_printf("Fixpoint concluded\n");
         break;
      }
   }

   return true;
}
#else
bool int_function::relocBlocks(Address baseInMutatee, 
                               pdvector<bblInstance *> &newInstances)
{
   Address currAddr = baseInMutatee;
   // Inefficiency, part 1: we pin each block at a particular address
   // so that we can one-pass generate and get jumps done correctly.
   for (unsigned i = 0; i < newInstances.size(); i++) {
      reloc_printf("Pinning block %d to 0x%lx\n", i, currAddr);
      newInstances[i]->setStartAddr(currAddr);
      if (i < (newInstances.size() - 1))
          currAddr += newInstances[i]->sizeRequired(newInstances[i+1]);
      else
          currAddr += newInstances[i]->sizeRequired(NULL);
   }

   // Okay, so we have a set of "new" basicBlocks. Now go through and
   // generate code for each; we can do branches appropriately, since
   // we know where the targets will be.
   // This builds the codeGen member of the bblInstance
   bool success = true;
   for (unsigned i = 0; i < newInstances.size(); i++) {
      reloc_printf("... relocating block %d\n", newInstances[i]->block()->id());
      // Hand in the physical successor block
      if (i < (newInstances.size() - 1))
          success &= newInstances[i]->generate(newInstances[i+1]);
      else
          success &= newInstances[i]->generate(NULL);
      if (!success) break;
   }
   return true;
}
#endif

bool int_function::relocationGenerateInt(pdvector<funcMod *> &mods, 
                                      int sourceVersion,
                                      pdvector<int_function *> &needReloc) {

    if(!canBeRelocated()) {
        return false;
    }
   
    // If we call this function, we *probably* want to relocate whether
    // or not we need to actually modify anything. 
    //if (mods.size() == 0)
    //    return true;

    assert(sourceVersion <= version_);
    if (generatedVersion_ > version_) {
        // Odd case... we generated, but never installed or
        // linked. Nuke the "dangling" version.
        relocationInvalidate();
    }

    generatedVersion_++;


    reloc_printf("Relocating function %s, version %d, 0x%lx, size: 0x%lx\n",
                 symTabName().c_str(), sourceVersion,
                 getAddress(), getSize_NP());

    // Make sure the blocklist is created.
    blocks(); 

    // Make the basic block instances; they're placeholders for now.
    pdvector<bblInstance *> newInstances;
    set< int_basicBlock* , int_basicBlock::compare >::iterator bIter;
    for (bIter = blockList.begin(); bIter != blockList.end(); bIter++) {
        reloc_printf("Block at %lx, creating instance...\n", 
                     (*bIter)->origInstance()->firstInsnAddr());
        bblInstance *newInstance = new bblInstance(*bIter, generatedVersion_);
        assert(newInstance);
        newInstances.push_back(newInstance);
        (*bIter)->instances_.push_back(newInstance);
    }
    assert(newInstances.size() == blockList.size());

    // Whip through them and let people play with the sizes.
    // We can also keep a tally of how much space we'll need while
    // we're at it...
    unsigned size_required = 0;
    for (unsigned i = 0; i < newInstances.size(); i++) {
        reloc_printf("Calling newInst:relocationSetup(%d)\n",
                     sourceVersion);
        newInstances[i]->relocationSetup
            (newInstances[i]->block()->instVer(sourceVersion), mods);
        if (i < (newInstances.size() - 1))
            size_required += newInstances[i]->sizeRequired(newInstances[i+1]);
        else
            size_required += newInstances[i]->sizeRequired(NULL);

        reloc_printf("After block %d, %d bytes required\n",
                     i, size_required);
    }

    Address baseInMutatee = allocRelocation(size_required);
    if (!baseInMutatee) return false;
    reloc_printf("... new version at 0x%lx in mutatee\n", baseInMutatee);

    bool success = relocBlocks(baseInMutatee, newInstances);
    if (!success) {
        relocationInvalidate();
        return false;
    }

    // We use basicBlocks as labels.
    // TODO Since there is only one entry block to any function and the
    // image_function knows what it is, maybe it should be available at
    // this level so we didn't have to do all this.
    for (bIter = blockList.begin(); bIter != blockList.end(); bIter++) {
        if (!(*bIter)->needsJumpToNewVersion()) continue;
        functionReplacement *funcRep = new functionReplacement((*bIter), 
                                                             (*bIter),
                                                             sourceVersion,
                                                             generatedVersion_);
        if (funcRep->generateFuncRepJump(needReloc)) {            
            (*bIter)->instVer(generatedVersion_)->jumpToBlock() = funcRep;
        }
        else {
            // Try using traps...
            // Fix this later; we might over-relocate things that we bled into
            // before we decided a jump was bad news.
            //needReloc.clear();
            if (funcRep->generateFuncRepTrap(needReloc)) {
                (*bIter)->instVer(generatedVersion_)->jumpToBlock() = funcRep; 
            }
            else {
                relocationInvalidate();
                return false;
            }
        }
    }
    
    return success;
}

bool int_function::relocationInstall() {

    // Okay, we now have a new copy of the function. Go through 
    // the version to be replaced, and replace each basic block
    // with a "jump to new basic block" combo.
    // If we overlap a bbl (which we probably will), oops.

    reloc_printf("%s[%d]: RELOCATION INSTALL FOR %s\n",
                 FILE__, __LINE__, prettyName().c_str());

    if (installedVersion_ == generatedVersion_) {
        reloc_printf("%s[%d]: installedVersion_ %d == generatedVersion_ %d, returning\n",
                FILE__, __LINE__, installedVersion_, generatedVersion_);
        return true; // Nothing to do here...
    }

    bool success = true;
    set< int_basicBlock* , int_basicBlock::compare >::iterator bIter;
    for (bIter = blockList.begin(); bIter != blockList.end(); bIter++) {
        success &= (*bIter)->instVer(generatedVersion_)->install();
        if (!success) break;
        
        // Add all the basicBlocks to the process data range...
        proc()->addOrigRange((*bIter)->instVer(generatedVersion_));
        addBBLInstance((*bIter)->instVer(generatedVersion_));
    }
    if (!success) {
        fprintf(stderr, "Warning: installation of relocated function failed\n");
        return false;
    }

    installedVersion_ = generatedVersion_;
    version_ = installedVersion_;

    return success;
}

bool int_function::relocationCheck(pdvector<Address> &checkPCs) {

    assert(generatedVersion_ == installedVersion_);

    set< int_basicBlock* , int_basicBlock::compare >::iterator bIter;
    for (bIter = blockList.begin(); bIter != blockList.end(); bIter++) {
        if (!(*bIter)->instVer(installedVersion_)->check(checkPCs))
            return false;
    }
    return true;
}
        

bool int_function::relocationLink(pdvector<codeRange *> &overwritten_objs) {
    if (linkedVersion_ == installedVersion_) {
        assert(linkedVersion_ == version_);
        return true; // We're already done...
    }

    // If the assert fails, then we linked but did not
    // update the global function version. That's _BAD_.

    bool success = true;
    set< int_basicBlock* , int_basicBlock::compare >::iterator bIter;
    for (bIter = blockList.begin(); bIter != blockList.end(); bIter++) {
        success &= (*bIter)->instVer(installedVersion_)->link(overwritten_objs);
        if (!success)
            break;
    }
    if (!success) {
        // Uh oh...
        fprintf(stderr, "ERROR: linking relocated function failed!\n");
        assert(0);
    }

    linkedVersion_ = installedVersion_;
    assert(linkedVersion_ == version_);

    return true;
}


// unlinks active multiTramps and 
// returns true if the bbi has one or more active multitramps
static bool unlinkActiveMultis
(pdvector<bblInstance::reloc_info_t::relocInsn::Ptr> &relocs,
 map<Address,multiTramp*> &activeMultiMap)
{
    bool foundMulti = false;
    // see if the block contains an active multitramp
    for( unsigned int bIdx = 0; 
         bIdx < relocs.size(); 
         bIdx++) 
    {
        multiTramp *bMulti = activeMultiMap[relocs[bIdx]->relocAddr];
        if ( NULL != bMulti ) {
            bMulti->removeCode(NULL);
            foundMulti = true;
        }
    }
    return foundMulti;
}


// there could be more than one multiTramp in this block instance
// I made this a local function because it assumes that the bbi's are 
// relocated and it will only be used in relocationInvalidate
static void deleteBlockMultis
    ( pdvector<bblInstance::reloc_info_t::relocInsn::Ptr> &relocs,
  AddressSpace *addSpace )
{
    // iterate through each instruction in the block
    for( unsigned int bIdx = 0; 
         bIdx < relocs.size(); 
         bIdx++) 
    {
        multiTramp *bMulti = addSpace->findModByAddr
            ( relocs[bIdx]->relocAddr )->is_multitramp();
        if ( NULL != bMulti ) {
            delete bMulti;
        }
    }
}


bool int_function::relocationInvalidate() {
    // The increase pattern goes like so:
    // generatedVersion_++;
    // installedVersion_++;
    // version_++; -- so that instpoints will be updated
    // linkedVersion_++;
    reloc_printf("%s[%d]: relocationInvalidate for %s: linkedVersion %d, "
                 "installedVersion %d, generatedVersion %d, version %d\n",
                 FILE__, __LINE__, symTabName().c_str(), 
                 linkedVersion_,
                 installedVersion_,
                 generatedVersion_,
                 version_);

    assert(generatedVersion_ >= installedVersion_);
    assert(installedVersion_ >= version_);
    assert(version_ >= linkedVersion_);

    if (generatedVersion_ == linkedVersion_) {
        reloc_printf("%s[%d]: nothing to do, returning\n",
                     FILE__, __LINE__);
        return true;
    }


    //-- special-case code that allows us to invalidate a function --//
    //-- relocation even if it is currently on the call stack      --//
    bool exploratoryMode = false;
    bool freeMutateeReloc = true;
    if ( proc()->proc() && proc()->proc()->isExploratoryModeOn() ) {
        exploratoryMode = true;
    }
    set< int_basicBlock* > saveBlocks; // blocks that shouldn't be invalidated
    // set this flag to false if the relocated function has an active 
    // multiTramp with a call
    // extract from the process::activeMultis set, a map of 
    // installAddr->multiTramp pairs so we can detect when basicBlock 
    // instances have active multiTramps, we can't just look up the 
    // multiTramp at the block instance address, because if the multiTramp
    // has been unlinked, the process::findMultiTrampByAddr() lookup fails
    map<Address,multiTramp*> activeMultiAddrs;
    if ( proc()->proc() && proc()->proc()->isExploratoryModeOn() ) { 
        proc()->proc()->getActiveMultiMap( activeMultiAddrs );
    }        


    std::set< int_basicBlock* , int_basicBlock::compare >::iterator bIter;

    while (installedVersion_ > linkedVersion_) {
        reloc_printf("******* Removing installed version %d\n",
                     installedVersion_);
        Address funcRelocBase = 
            (*blockList.begin())->instVer(installedVersion_)->firstInsnAddr();

        int i=0;
        for (i=0, bIter = blockList.begin(); 
             bIter != blockList.end(); 
             bIter++,i++) 
        {
            reloc_printf("%s[%d]: Removing installed version %d of block %d\n",
                         FILE__, __LINE__, installedVersion_, i);

            // in exploratory mode, a block can have fewer than the expected 
            // number of instances if it is split since the previous relocation,
            // in which case we continue without invalidating the block
            if ((*bIter)->instances().size()-1 < (unsigned)installedVersion_) {
                assert( 1 == (*bIter)->instances().size() );
                mal_printf("Missing block instance due to block splitting, "
                           "no problems should arise %s[%d]\n",FILE__,__LINE__);
                continue; 
            }

            bblInstance *instance = (*bIter)->instVer(installedVersion_);
            assert(instance);
            
            bool isBlockActive = false;
            if ( exploratoryMode ) {
                isBlockActive = unlinkActiveMultis( instance->relocs(), 
                                                    activeMultiAddrs   );
                if ( isBlockActive ) {
                    // the subsequent loop will drop bblInstance from the 
                    // int_basicBlock's instance vector, it will remain
                    // in the process and function code range trees 
                    mal_printf("reloc invalidate will not delete block %lx"
                               "[%lx %lx] as it has an active multitramp that "
                               "is on the call stack %s[%d]\n",
                               instance->block()->origInstance()->
                               firstInsnAddr(), 
                               instance->firstInsnAddr(), instance->endAddr(), 
                               FILE__,__LINE__);
                    freeMutateeReloc = false;
                    instance->reloc_info->funcRelocBase_ = funcRelocBase;
                    saveBlocks.insert( *bIter );
                }
            }

            if ( ! isBlockActive ) { 
                proc()->removeOrigRange(instance);
                deleteBBLInstance(instance);
                // Nuke any attached multiTramps...
                deleteBlockMultis(instance->relocs(), proc());
            }
        }
        installedVersion_--;
    }
    
    while (generatedVersion_ > installedVersion_) {
        reloc_printf("******* Removing generated version %d\n",
                     generatedVersion_);

        if ( freeMutateeReloc ) {
            // in exploratory mode it's possible that an analysis
            // update has added a new block to this function that
            // precedes the block entry, so find the first block with
            // a BBI that exists in this relocation of the function
            for(bIter = blockList.begin(); 
                bIter != blockList.end() && 
                generatedVersion_ >= (int)(*bIter)->instances().size();
                bIter++) ;
            assert(bIter != blockList.end());

            proc()->inferiorFree((*bIter)->
                                 instVer(generatedVersion_)->
                                 firstInsnAddr());
            
            // delete all BBI's for the reloc
            for (bIter = blockList.begin(); bIter != blockList.end(); bIter++) {
                reloc_printf("%s[%d]: Removing generated version %d of block "
                             "at %lx\n", FILE__, __LINE__, generatedVersion_, 
                             (*bIter)->id());
                (*bIter)->removeVersion(generatedVersion_);
            }
        }
        
        else { // remove only those BBI's that are not on the call stack
            for (bIter = blockList.begin(); bIter != blockList.end(); bIter++) {
                reloc_printf("%s[%d]: Removing generated version %d of block "
                             "%d\n", FILE__, __LINE__, generatedVersion_, 
                             (*bIter)->id());
                if ( saveBlocks.end() == saveBlocks.find( *bIter ) ) {
                    (*bIter)->removeVersion(generatedVersion_,true);
                } else {
                    (*bIter)->removeVersion(generatedVersion_,false);
                }
            }
        }

        generatedVersion_--;
    }
    version_ = linkedVersion_;

    reloc_printf("%s[%d]: version %d, linked %d, installed %d, generated %d\n",
                 FILE__, __LINE__, version_, linkedVersion_, installedVersion_, 
                 generatedVersion_);
    for (bIter = blockList.begin(); bIter != blockList.end(); bIter++) {
        reloc_printf("%s[%d]: block %d has %d versions\n",
                     FILE__, __LINE__, (*bIter)->id(), 
                     (*bIter)->instances().size());
    }

    unsigned i=0;
    for (i = 0; i < entryPoints_.size(); i++)
        entryPoints_[i]->updateInstances();
    for (i = 0; i < exitPoints_.size(); i++)
        exitPoints_[i]->updateInstances();
    for (i = 0; i < callPoints_.size(); i++)
        callPoints_[i]->updateInstances();
    for (i = 0; i < arbitraryPoints_.size(); i++)
        arbitraryPoints_[i]->updateInstances();
    set<instPoint*>::iterator pIter = unresolvedPoints_.begin();
    for(; pIter != unresolvedPoints_.end(); pIter++) {
        (*pIter)->updateInstances();
    }
    for (pIter = abruptEnds_.begin(); pIter != abruptEnds_.end(); pIter++) {
        (*pIter)->updateInstances();
    }

    return true;
}

bool int_function::relocationInvalidateAll()
{
    mal_printf("%s[%d] in relocationInvalidateAll for function %lx\n",
               FILE__,__LINE__,getAddress());

    while (version() > 0) {
        mal_printf("%s[%d]: invalidating reloc version %d of the function\n",
                   FILE__,__LINE__,linkedVersion_);

        // remove funcReplacement jumps for all blocks
        set< int_basicBlock* , int_basicBlock::compare >::iterator bIter;
        for (bIter = blockList.begin(); bIter != blockList.end(); bIter++) {
            functionReplacement *jump = proc()->findFuncReplacement
                ((*bIter)->origInstance()->firstInsnAddr());
            if ( jump ) {
                proc()->removeFuncReplacement(jump);
            }

            if (dyn_debug_malware) {
                bblInstance *curInst = (*bIter)->instVer(version());
                printf("invalidating block %lx [%lx %lx] ", 
                       (*bIter)->origInstance()->firstInsnAddr(),
                       curInst->firstInsnAddr(),
                       curInst->endAddr());
                Address blockBegin = curInst->firstInsnAddr();
                Address blockSize = curInst->endAddr() - blockBegin;
                unsigned char * memBuf = (unsigned char *) malloc(blockSize);
                proc()->readDataSpace((void*)blockBegin, blockSize, memBuf, true);
                for (unsigned idx=0; idx < blockSize; idx++) {
                    printf("%02x", memBuf[idx]);
                }
                printf("\n");
                free(memBuf);
            }

        }
        
        // invalidate the current relocation
        if ( ! relocationInvalidate() ) {
            assert(0);
            return false;
        }

        linkedVersion_--;
    }

    return true;
}


bool int_function::expandForInstrumentation() {
    // Take the most recent version of the function, check the instPoints
    // registered. If one needs more room, create an expansion record.
    // When we're done, relocate the function (most recent version only).

    // Oh, only do that if there's instrumentation added at the point?
    reloc_printf("Function expandForInstrumentation, version %d\n",
                 version_);
    // Right now I'm basing everything off version 0; that is, if we
    // relocate multiple times, we will have discarded versions instead
    // of a long chain. 

    if (!canBeRelocated()) {
        return false;
    }

    set< int_basicBlock* , int_basicBlock::compare >::iterator bIter;
    for (bIter = blockList.begin(); bIter != blockList.end(); bIter++) {
        bblInstance *bblI = (*bIter)->origInstance();
        assert(bblI->block() == (*bIter));
        // Simplification: check if there's a multiTramp at the block.
        // If there isn't, then we don't care.
        multiTramp *multi = proc()->findMultiTrampByAddr(bblI->firstInsnAddr());
        if (!multi) continue;

        // multi->desiredSize() is set to size of trap ins'n if the
        // function is too short to accommodate a jump, but in
        // exploratory mode, the function can grow, making it possible
        // that we previously used a trap but could now use a jump
        unsigned desiredSize = multi->sizeDesired();
        unsigned jumpSize = instruction::maxJumpSize(proc()->getAddressWidth());
        if (proc()->proc() && 
            proc()->proc()->isExploratoryModeOn() && 
            desiredSize < jumpSize && 
            jumpSize <= ifunc()->getEndOffset() - ifunc()->getOffset()) 
        {
            desiredSize = jumpSize;
        }
        if (bblI->getSize() < desiredSize) {
            reloc_printf("Enlarging basic block %d; currently %d, %d bytes "
                         "required; start addr 0x%lx\n",
                         (*bIter)->id(), bblI->getSize(), desiredSize, 
                         bblI->firstInsnAddr());
            pdvector<bblInstance::reloc_info_t::relocInsn::Ptr> whocares;
            bool found = false;
            // Check to see if there's already a request for it...
            for (unsigned j = 0; j < enlargeMods_.size(); j++) {
                if (enlargeMods_[j]->update(bblI->block(), 
                                            whocares,
                                            desiredSize)) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                // Didn't find it...
                enlargeBlock *mod = new enlargeBlock(bblI->block(), multi->maxSizeRequired());
                enlargeMods_.push_back(mod);
            }
        }
    }
    return true;
}

// Return the absolute maximum size required to relocate this
// block somewhere else. If this looks very familiar, well, 
// _it is_. We should unify the instruction and relocatedInstruction
// classes.
// This is a bit inefficient, since we rapidly use and delete
// relocatedInstructions... ah, well :)
//
// in defensive mode we often don't parse past call blocks, so there
// may not be a fallthrough block; since we will instrument the call
// instruction to find out if there is a fallthrough block, we want
// there to be enough space to jump to a  multiTramp without having 
// to use a trap instruction
unsigned bblInstance::sizeRequired(bblInstance *nextBBL) {
    assert(getMaxSize());

    unsigned jumpToFallthrough = 0;
    if (nextBBL &&
        getFallthroughBBL() &&
        (nextBBL != getFallthroughBBL())) {
        jumpToFallthrough = instruction::maxJumpSize(proc()->getAddressWidth());
    }
    else if ( func()->ifunc()->img()->codeObject()->defensiveMode() &&
              !getFallthroughBBL() && 
              block()->containsCall() ) {
        jumpToFallthrough = instruction::maxJumpSize(proc()->getAddressWidth());
    }
    return getMaxSize() + jumpToFallthrough;
}

bool bblInstance::reset()
{
  return true;
}

// Make a copy of the basic block (from the original provided),
// applying the modifications stated in the vector of funcMod
// objects.
bool bblInstance::relocationSetup(bblInstance *orig, pdvector<funcMod *> &mods) {
   unsigned i;
   origInstance() = orig;
   assert(origInstance());
   // First, build the insns vector

   //shared_ptr now
   //for (i = 0; i < relocs().size(); i++) {
   //  delete relocs()[i];
   //}

   relocs().clear();

   // Keep a running count of how big things are...
   maxSize() = 0;
   minSize() = 0;
   using namespace Dyninst::InstructionAPI;
   unsigned char* buffer = reinterpret_cast<unsigned char*>(orig->proc()->getPtrToInstruction(orig->firstInsnAddr()));
   InstructionDecoder d(buffer, orig->getSize(), func()->ifunc()->isrc()->getArch());
   
   size_t offset = 0;
   while(offset < orig->getSize())
   {
     Instruction::Ptr tmp = d.decode();
     
     //reloc_info_t::relocInsn *reloc = new reloc_info_t::relocInsn;
     reloc_info_t::relocInsn::Ptr reloc = 
        dyn_detail::boost::make_shared<reloc_info_t::relocInsn>();

     reloc->origAddr = orig->firstInsnAddr() + offset;
     reloc->relocAddr = 0;
     reloc->origInsn = new instruction;
     reloc->origPtr = buffer + offset;
     reloc->origInsn->setInstruction((unsigned char*)reloc->origPtr, reloc->origAddr);
     reloc->relocTarget = 0;
     reloc->relocSize = 0;

     relocs().push_back(reloc);

     maxSize() += reloc->origInsn->spaceToRelocate();
     offset += tmp->size();
   }
   
  

   // Apply any hanging-around relocations from our previous instance
   for (i = 0; i < orig->appliedMods().size(); i++) {
      if (orig->appliedMods()[i]->modifyBBL(block_, relocs(), *this)) {
         appliedMods().push_back(orig->appliedMods()[i]);
      }
    }

    // So now we have a rough size and a list of insns. See if any of
    // those mods want to play.
    for (i = 0; i < mods.size(); i++) {
        if (mods[i]->modifyBBL(block_, relocs(), *this)) {
            // Store for possible further relocations.
            appliedMods().push_back(mods[i]);
        }
    }

    return true;
}

void bblInstance::setStartAddr(Address addr) {
  firstInsnAddr_ = addr;
}

bool bblInstance::generate(bblInstance *nextBBL) {
    assert(firstInsnAddr_);
    assert(relocs().size());
    assert(maxSize());
    assert(block_);
    assert(origInstance());
    unsigned i;

    unsigned fallthroughJumpSizeNeeded = 0;
    if (nextBBL &&
        getFallthroughBBL() &&
        (nextBBL != getFallthroughBBL())) {
        fallthroughJumpSizeNeeded = instruction::maxJumpSize(proc()->getAddressWidth());
    }
    

    generatedBlock().allocate(maxSize() + fallthroughJumpSizeNeeded);
    generatedBlock().setAddrSpace(proc());
    generatedBlock().setAddr(firstInsnAddr_);
    generatedBlock().setFunction(func());

    Address origAddr = origInstance()->firstInsnAddr();
    for (i = 0; i < relocs().size(); i++) {
      Address currAddr = generatedBlock().currAddr(firstInsnAddr_);
      relocs()[i]->relocAddr = currAddr;
      patchTarget *fallthroughOverride = NULL;
      patchTarget *targetOverride = NULL;
      if (i == (relocs().size()-1)) {
          targetOverride = getTargetBBL();
      }
      reloc_printf("... generating insn %d, orig addr 0x%lx, new addr 0x%lx, " 
                   "fallthrough 0x%lx, target 0x%lx\n",
                   i, origAddr, currAddr, 
                   fallthroughOverride ? fallthroughOverride->get_address() : 0, 
                   targetOverride ? targetOverride->get_address() : 0);
      unsigned usedBefore = generatedBlock().used();

      // Added 4APR08 to handle de-overlapping blocks; our fallthrough may
      // not be the contextual successor...
      bblInstance *fallthrough = getFallthroughBBL();
      
      if ((nextBBL != NULL) &&
          (fallthrough != NULL) &&
          (fallthrough != nextBBL)) {
          reloc_printf("%s[%d]: Handling case where fallthrough is not next "
                       "block; func %s, block at 0x%lx, fallthrough at 0x%lx, "
                       "next block at 0x%lx\n",
                       FILE__, __LINE__,
                       func()->prettyName().c_str(),
                       block()->origInstance()->firstInsnAddr(),
                       fallthrough->origInstance()->firstInsnAddr(),
                       nextBBL->origInstance()->firstInsnAddr());
          mal_printf("%s[%d]: Handling case where fallthrough is not next "
                     "block; func %s, block at 0x%lx, fallthrough at 0x%lx, "
                     "next block at 0x%lx\n",
                     FILE__, __LINE__,
                     func()->prettyName().c_str(),
                     block()->origInstance()->firstInsnAddr(),
                     fallthrough->origInstance()->firstInsnAddr(),
                     nextBBL->origInstance()->firstInsnAddr());
          fallthroughOverride = fallthrough;
      }

      insnCodeGen::generate(generatedBlock(),
                                      *relocs()[i]->origInsn,
                                      proc(),
                                      origAddr,
                                      currAddr,
                                      fallthroughOverride,
                                      targetOverride); // targetOverride
      
      relocs()[i]->relocTarget = targetOverride ? targetOverride->get_address() : 0;
      
      // And set the remaining bbl variables correctly
      // This may be overwritten multiple times, but will end
      // correct.
      lastInsnAddr_ = currAddr;
      
      relocs()[i]->relocSize = generatedBlock().used() - usedBefore;
      
      origAddr += relocs()[i]->origInsn->size();
    }


#if !defined(cap_fixpoint_gen)
    //Fill out unused space at end of block with NOPs
    generatedBlock().fillRemaining(codeGen::cgNOP);
    blockEndAddr_ = firstInsnAddr_ + maxSize();
#else
    //No unused space.  Only expand if we're under the minimum size
    // (which only happens if the block is being expanded.
    unsigned space_used = generatedBlock().used();
    if (space_used < minSize())
       generatedBlock().fill(minSize() - space_used, codeGen::cgNOP);
    blockEndAddr_ = firstInsnAddr_ + generatedBlock().used();
#endif

    relocs().back()->relocSize = blockEndAddr_ - lastInsnAddr_;
    
    // Post conditions
    assert(firstInsnAddr_);
    assert(lastInsnAddr_);
    assert(blockEndAddr_);
    
    return true;
}

bool bblInstance::install() {
    assert(firstInsnAddr_);
    assert(generatedBlock() != NULL);
    assert(maxSize());

    reloc_printf("%s[%d]: Writing from 0x%lx 0x%lx to 0x%lx 0x%lx\n",
                 FILE__, __LINE__,
                 generatedBlock().start_ptr(), 
                 (long) generatedBlock().start_ptr() + generatedBlock().used(),
                 firstInsnAddr_,
                 firstInsnAddr_ + generatedBlock().used());
    
    bool success = proc()->writeTextSpace((void *)firstInsnAddr_,
                                          generatedBlock().used(),
                                          generatedBlock().start_ptr());
    if (success) {
        return true;
    }
    else 
        return false;
}

bool bblInstance::check(pdvector<Address> &checkPCs) {
    if (!getJumpToBlock()) return true;
    return jumpToBlock()->checkFuncRep(checkPCs);
}

bool bblInstance::link(pdvector<codeRange *> &overwrittenObjs) {
    if (!getJumpToBlock()) return true;
    return jumpToBlock()->linkFuncRep(overwrittenObjs);
}

bool enlargeBlock::modifyBBL(int_basicBlock *block,
                             pdvector<bblInstance::reloc_info_t::relocInsn::Ptr> &,
                             bblInstance &bbl)
{
   if (block == targetBlock_) {
      if (targetSize_ == (unsigned) -1) {
         return true;
      }
      
      if (bbl.maxSize() < targetSize_) {
         bbl.maxSize() = targetSize_;
      }
      if (bbl.minSize() < targetSize_) {
         bbl.minSize() = targetSize_;
      }
      
      return true;
   }
   return false;
}

bool enlargeBlock::update(int_basicBlock *block,
                          pdvector<bblInstance::reloc_info_t::relocInsn::Ptr> &,
                          unsigned size) {
    if (block == targetBlock_) {
        if (size == (unsigned) -1) {
            // Nothing we can do about it, we're just fudging...
            return true;
        }
        targetSize_ = (targetSize_ > size) ? targetSize_ : size;
        return true;
    }
    return false;
}

functionReplacement::functionReplacement(int_basicBlock *sourceBlock,
                                         int_basicBlock *targetBlock,
                                         unsigned sourceVersion /* =0 */,
                                         unsigned targetVersion /* =0 */) :
    sourceBlock_(sourceBlock),
    targetBlock_(targetBlock),
    sourceVersion_(sourceVersion),
    targetVersion_(targetVersion),
    overwritesMultipleBlocks_(false),
    usesTrap_(false)
{}
    
Address functionReplacement::get_address() const {
    assert(sourceBlock_);
    return sourceBlock_->instVer(sourceVersion_)->firstInsnAddr();
}

unsigned functionReplacement::get_size() const {
    if (jumpToRelocated != NULL)
        return jumpToRelocated.used();
    else
        return 0;
}

// Will potentially append to needReloc (indicating that
// other functions must be relocated)
bool functionReplacement::generateFuncRepJump(pdvector<int_function *> &needReloc)
{
    
    assert(sourceBlock_);
    assert(targetBlock_);
    assert(jumpToRelocated == NULL);

    usesTrap_ = false;

    // TODO: if check modules and do ToC if not the same one.

    bblInstance *sourceInst = sourceBlock_->instVer(sourceVersion_);
    assert(sourceInst);
    bblInstance *targetInst = targetBlock_->instVer(targetVersion_);
    assert(targetInst);

    unsigned addr_width = proc()->getAddressWidth();
    jumpToRelocated.allocate(instruction::maxInterFunctionJumpSize(addr_width));
    jumpToRelocated.setAddrSpace(proc());
    reloc_printf("******* generating interFunctionJump from 0x%lx (%d) to 0x%lx (%d)\n",
		 sourceInst->firstInsnAddr(),
		 sourceVersion_,
		 targetInst->firstInsnAddr(),
		 targetVersion_);

    insnCodeGen::generateInterFunctionBranch(jumpToRelocated,
                                             sourceInst->firstInsnAddr(),
                                             targetInst->firstInsnAddr());

    // Determine whether relocation of this function will force relocation
    // of any other functions:
    // If the inter-function jump will overwrite any shared blocks,
    // the "co-owner" functions that are associated with those blocks
    // must be relocated before the jump can be written.
    //


    if(sourceBlock_->hasSharedBase())
    {
        reloc_printf("%s[%d]: odd case, function with shared entry block\n",
                     FILE__, __LINE__);

        // if this entry block is shared...
        sourceBlock_->func()->getSharingFuncs(sourceBlock_,
                                              needReloc);
    }

    if (jumpToRelocated.used() > sourceInst->getSize()) {
        // Okay, things are going to get ugly. There are two things we
        // can't do:
        // 1) Overwrite another entry point
        // 2) Overwrite a different function
        // So start seeing where this jump is going to run into...
        
        // FIXME there seems to be something fundamentally unsound about
        // going ahead and installing instrumentation over the top of
        // other functions! 
	//
	// And so, now, we don't.  Return false in this case, and in the
	// case where we would normally write into unclaimed space.
	//
        unsigned overflow = jumpToRelocated.used() - sourceInst->getSize();
        Address currAddr = sourceInst->endAddr();

        while (overflow > 0) {
            bblInstance *curInst = sourceBlock_->func()->findBlockInstanceByAddr(currAddr);
            reloc_printf("%s[%d]: jump overflowed into block %p at 0x%lx\n", 
                         FILE__, __LINE__, curInst, currAddr);
            if (curInst) {
                // Okay, we've got another block in this function. Check
                // to see if it's shared.
                if (curInst->block()->hasSharedBase()) {
                    reloc_printf("%s[%d]: block is shared, checking if it is an entry function\n",
                                 FILE__, __LINE__);
		  // This can get painful. If we're the entry block for another
		  // function (e.g., __write_nocancel on Linux), we _really_ don't
		  // want to be writing a jump here. So, check to see if the
		  // internal block is an entry for a function that is _not_ us.
		  image_func *possibleEntry = curInst->block()->llb()->getEntryFunc();

		  if (possibleEntry && possibleEntry != sourceBlock_->func()->ifunc()) {
		    // Yeah, this ain't gonna work
                      reloc_printf("%s[%d]: Found function %s that shares with this block at 0x%lx, returning failure\n",
                                   FILE__, __LINE__, possibleEntry->prettyName().c_str(), currAddr);
		    return false;
		  }

		  // add functions to needReloc list
		  curInst->block()->func()->getSharingFuncs(curInst->block(),
							    needReloc);
                } 

                if (curInst->block()->needsJumpToNewVersion()) {
                    reloc_printf("%s[%d]: Block needs jump to new version, failing\n",
                                 FILE__, __LINE__);
                    // Ooopsie... we're going to stop on another block
                    // that jumps over. This we cannot do.
                    return false;
                }

                // Otherwise keep going
                // Inefficient...
                currAddr = curInst->endAddr();
                if (curInst->getSize() > overflow)
                    overflow = 0;
                else
                    overflow -= curInst->getSize();
            }
            else {
                // Ummm... see if anyone else claimed this space.

                // NTS: we want any basic block that matches this address range
                // as part of any image in the proc(). hmmmm.... this means
                // that the process needs to have knowledge of all
                // int_basicBlocks.
                int_basicBlock *block =
                        sourceBlock_->proc()->findBasicBlockByAddr(currAddr);

                if(block)
                {
                    // Consistency check...
                    assert(block->func() != sourceBlock_->func());
		    return false;
                }
                else {
                    // Ummm... empty space.  Let's not try to write here.
		    return false;
                }
            }
        }
        overwritesMultipleBlocks_ = true;
    }

    return true;
}

// Will potentially append to needReloc (indicating that
// other functions must be relocated)
bool functionReplacement::generateFuncRepTrap(pdvector<int_function *> &needReloc)
{
    assert(sourceBlock_);
    assert(targetBlock_);

    if (!proc()->canUseTraps())
        return false;

    jumpToRelocated.invalidate();

    assert(usesTrap_ == false);

    usesTrap_ = true;

    // TODO: if check modules and do ToC if not the same one.

    bblInstance *sourceInst = sourceBlock_->instVer(sourceVersion_);
    assert(sourceInst);
    bblInstance *targetInst = targetBlock_->instVer(targetVersion_);
    assert(targetInst);

    unsigned addr_width = proc()->getAddressWidth();
    jumpToRelocated.allocate(instruction::maxInterFunctionJumpSize(addr_width));
    reloc_printf("******* generating interFunctionTrap from 0x%lx (%d) to 0x%lx (%d)\n",
		 sourceInst->firstInsnAddr(),
		 sourceVersion_,
		 targetInst->firstInsnAddr(),
		 targetVersion_);

    insnCodeGen::generateTrap(jumpToRelocated);

    // Determine whether relocation of this function will force relocation
    // of any other functions:
    // If the inter-function jump will overwrite any shared blocks,
    // the "co-owner" functions that are associated with those blocks
    // must be relocated before the jump can be written.
    //


    if(sourceBlock_->hasSharedBase())
    {
        // if this entry block is shared...
        sourceBlock_->func()->getSharingFuncs(sourceBlock_,
                                              needReloc);
    }

    return true;
}

AddressSpace *functionReplacement::proc() const {
    assert(sourceBlock_);
    return sourceBlock_->proc();
}

bool functionReplacement::installFuncRep() {
    // Nothing to do here unless we go to a springboard model.


    return true;
}

// TODO: jumps that overwrite multiple basic blocks...
bool functionReplacement::checkFuncRep(pdvector<Address> &checkPCs) {
    unsigned i;

    Address start = get_address();
    Address end = get_address() + get_size();
    for (i = 0; i < checkPCs.size(); i++) {
        if ((checkPCs[i] > start) &&
            (checkPCs[i] < end))
            return false;
    }
    return true;
}

bool functionReplacement::linkFuncRep(pdvector<codeRange *> &/*overwrittenObjs*/) {
    if (sourceBlock_->proc()->writeTextSpace((void *)get_address(),
                                             jumpToRelocated.used(),
                                             jumpToRelocated.start_ptr())) {
        sourceBlock_->proc()->addFuncReplacement(this); 

        if (usesTrap_) {
            bblInstance *sourceInst = sourceBlock_->instVer(sourceVersion_);
            assert(sourceInst);
            bblInstance *targetInst = targetBlock_->instVer(targetVersion_);
            assert(targetInst);

            AddressSpace *as = sourceBlock_->proc();
            as->trapMapping.addTrapMapping(sourceInst->firstInsnAddr(),
                                           targetInst->firstInsnAddr(),
                                           true);
        }       

        return true;
    }
    else
        return false;
}


// If we're an entry block, we need a jump (to catch the
// entry point). Also true if we are the target of an indirect jump.

bool int_basicBlock::needsJumpToNewVersion() {
    if (isEntryBlock())
        return true;
    
    assert(ib_);
    pdvector<int_basicBlock *> sources;
    getSources(sources);
    for (unsigned i = 0; i < sources.size(); i++) {
        if (getSourceEdgeType(sources[i]) == ParseAPI::INDIRECT)
            return true;
    }
    return false;
}
    

