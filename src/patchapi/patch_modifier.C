/*
 * #Name: patch2_1
 * #Desc: Mutator Side - Remove Snippets at Function Entry
 * #Dep:
 * #Notes:
 *     batch               operation               result
 *     -----               ----------              ------
 *     patcher1            push_back 1             1
 *     patcher1            push_back 2             1,2
 *     patcher1            push_back 3             1,2,3
 *     patcher1            push_back 4             1,2,3,4
 *     patcher2            remove 2                1,3,4
 *     patcher2            remove 4                1,3
 *     patcher2            remove 1                3
 */

#include "test_lib.h"
#include "patchapi_comp.h"
#include "PatchCFG.h"
#include "CFG.h"
#include "PatchModifier.h"

using namespace Dyninst;
using namespace PatchAPI;

using Dyninst::PatchAPI::PatchFunction;
using Dyninst::PatchAPI::Patcher;
using Dyninst::PatchAPI::PushFrontCommand;
using Dyninst::PatchAPI::PushBackCommand;
using Dyninst::PatchAPI::Point;
using Dyninst::PatchAPI::Snippet;
using Dyninst::PatchAPI::Command;
using Dyninst::PatchAPI::RemoveSnippetCommand;
using Dyninst::PatchAPI::InstancePtr;

class patchModifier_Mutator : public PatchApiMutator {
  virtual test_results_t executeTest();
   bool splitBlock();
   bool redirect();
   bool insert();
};

char insert_buffer[] = {
   0x50, 0x54, 0x52, 
   0x68, 0x90, 0x83, 0x04, 0x08, 
   0x68, 0xa0, 0x83, 0x04, 0x08,
   0x51, 0x56};

unsigned buf_size = 15;

extern "C" DLLEXPORT TestMutator* patch_modifier_factory() {
  return new patchModifier_Mutator();
}

/*
 * The mutatee has a series of small functions
 */

bool patchModifier_Mutator::splitBlock() {
   PatchFunction *func = findFunction("patchMod_split");
   if (func == NULL) {
      logerror("** Failed patch_modifier: failed to find patchMod_split\n");
      return false;
   }

   PatchBlock *entry = func->entry();
   if (!entry) {
      logerror("** Failed patch_modifier: failed to find entry block to split\n");
      return false;
   }

   PatchBlock::Insns insns;
   entry->getInsns(insns);

   Address oldEnd = entry->end();

   if (insns.empty()) {
      logerror("** Failed patch_modifier: split block reported no instructions\n");
      return false;
   }

   PatchBlock::Insns::iterator iter = insns.begin();
   // Don't split at the first instruction...
   ++iter;
   if (iter == insns.end()) return false;

   // Split the block at the first instruction
   PatchBlock *split = PatchModifier::split(entry, iter->first, true, entry->start());
   if (!split) {
      logerror("** Failed patch_modifier: splitting block at first instruction failed\n");
      return false;
   }

   // Let's double check
   PatchBlock::Insns i2, i3;
   entry->getInsns(i2);
   split->getInsns(i3);

   if (entry->end() != split->start() ||
       split->end() != oldEnd) {
      logerror("** Failed patch_modifier: incorrect basic block address ranges\n");
      return false;
   }

   if (entry->getSources().size() != 1) {
      logerror("** Failed patch_modifier: split block has more than one out-edge\n");
      return false;
   }


   if (insns.size() != (i2.size() + i3.size())) {
      logerror("** Failed patch_modifier: incorrect number of instructions in split blocks\n");
      logerror("**   Expected %d and %d (matching original %d), got %d and %d\n",
               1, insns.size() - 1, 
               insns.size(), 
               i2.size(),
               i3.size());
      return false;
   }


   if (!func->consistency()) {
      logerror("** Failed patch_modifier: function failed consistency check\n");
      return false;
   }
   
   return true;
}

bool patchModifier_Mutator::redirect() {
   PatchFunction *func = findFunction("patchMod_redirect");

   if (func == NULL) {
      logerror("** Failed patch_modifier: failed to find patchMod_redirect\n");
      return false;
   }

   PatchBlock *entry = func->entry();
   if (!entry) {
      logerror("** Failed patch_modifier: failed to find entry block to redirect\n");
      return false;
   }

   if (entry->getTargets().size() != 2) {
      logerror("** Failed patch_modifier: entry block has %d out-edges instead of 2, inconsistency\n",
               entry->getTargets().size());
      return false;
   }

   PatchEdge *taken = entry->findTarget(ParseAPI::COND_TAKEN);
   if (!taken) {
      logerror("** Failed patch_modifier: entry block has no conditional taken edge\n");
      return false;
   }

   PatchEdge *ft = entry->findTarget(ParseAPI::COND_NOT_TAKEN);
   if (!ft) {
      logerror("** Failed patch_modifier: entry block has no conditional fallthrough edge\n");
      return false;
   }
      
   PatchBlock *target = ft->target();
   if (!target) {
      logerror("** Failed patch_modifier: conditional fallthrough has no target block\n");
      return false;
   }
   
   if (!PatchModifier::redirect(taken, target)) {
      logerror("** Failed patch_modifier: redirect taken to fallthrough failed\n");
      return false;
   }

   // Verify consistency and that things are what we want. 
   for (PatchBlock::edgelist::const_iterator iter = entry->getTargets().begin();
        iter != entry->getTargets().end(); ++iter) {
      if ((*iter)->target() != target) {
         logerror("** Failed patch_modifier: post-redirect, found edge with incorrect target block; wanted 0x%lx (%p) and found 0x%lx (%p)\n",
                  target->start(), target,
                  (*iter)->target()->start(),
                  (*iter)->target());
         return false;
      }
   }
   for (PatchBlock::edgelist::const_iterator iter = target->getSources().begin();
        iter != target->getSources().end(); ++iter) {
      if ((*iter)->source() != entry) {
         logerror("** Failed patch_modifier: post-redirect, found edge with incorrect source block\n");
         return false;
      }
   }
   return true;

}

bool patchModifier_Mutator::insert() {
   PatchFunction *func = findFunction("patchMod_insert");

   if (func == NULL) {
      logerror("** Failed patch_modifier: failed to find patchMod_insert\n");
      return false;
   }

   PatchBlock *block = PatchModifier::insert(func->obj(), insert_buffer, buf_size);
   if (!block) {
      logerror("** Failed patch_modifier: insertion failed\n");
      return false;
   }

   PatchBlock::Insns insns;
   block->getInsns(insns);
   for (PatchBlock::Insns::iterator iter = insns.begin(); iter != insns.end(); ++iter) {
      std::cerr << std::hex << iter->first << " : " << iter->second->format() << std::dec << std::endl;
   }

   std::cerr << "Block has " << block->getSources().size() << " incoming edges and " 
             << block->getTargets().size() << " outgoing edges." << std::endl;
   for (PatchBlock::edgelist::const_iterator iter = block->getTargets().begin();
        iter != block->getTargets().end(); ++iter) {
      std::cerr << "Target edge: "
                << ((*iter)->sinkEdge() ? "<sink>" : "")
#if defined(_MSC_VER)
				<< std::endl;
#else
				<< Dyninst::ParseAPI::format((*iter)->type()) << std::endl;
#endif
   }

   return true;

}


test_results_t patchModifier_Mutator::executeTest() {
   return ::SKIPPED;

   // TODO reimplement...
   if (!redirect()) return ::FAILED;

   if (!splitBlock()) return ::FAILED;

   if (!insert()) return ::FAILED;
   
   return ::PASSED;
}
