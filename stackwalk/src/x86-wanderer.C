/*
 * Copyright (c) 1996-2008 Barton P. Miller
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "stackwalk/h/walker.h"
#include "stackwalk/h/frame.h"
#include "stackwalk/h/swk_errors.h"
#include "stackwalk/h/procstate.h"
#include "stackwalk/src/x86-swk.h"
#include "stackwalk/src/symtab-swk.h"

#include "common/h/Types.h"

#include "symtabAPI/h/Function.h"

using namespace Dyninst;
using namespace Stackwalker;


StepperWandererImpl::StepperWandererImpl(Walker *walker_,
                                         WandererHelper *whelper_,
                                         FrameFuncHelper *fhelper_,
                                         StepperWanderer *parent_) :
   FrameStepper(walker_),
   whelper(whelper_),
   fhelper(fhelper_),
   parent(parent_)
{
}

StepperWandererImpl::~StepperWandererImpl()
{
}

unsigned StepperWandererImpl::getPriority() const
{
   return 0x10030;
}

gcframe_ret_t StepperWandererImpl::getCallerFrame(const Frame &in, Frame &out)
{
   sw_printf("[%s:%u] - Wanderer attempting to walk from 0x%lx\n",
             __FILE__, __LINE__, in.getRA());

   const unsigned addr_width = getProcessState()->getAddressWidth();

   Address words[MAX_WANDERER_DEPTH];
   bool result = getTopWords(&(words[0]), in.getSP());
   if (!result) {
      sw_printf("[%s:%u] - getTopWords returned false\n", __FILE__, __LINE__);
      return gcf_not_me;
   }
   
   unsigned found_word = MAX_WANDERER_DEPTH;
   Address found_target = 0x0;
   std::vector<std::pair<unsigned, Address> > candidate;
   for (unsigned i=0; i<MAX_WANDERER_DEPTH; i++)
   {
      Address target;
      if (whelper->isPrevInstrACall(words[i], target))
      {
         if (whelper->isPCInFunc(target, in.getRA()))
         {
            sw_printf("[%s:%u] - Wanderer thinks word 0x%lx (0x%lx+%d) is return "
                      " address\n", __FILE__, __LINE__, words[i], in.getSP(), i);
            found_word = i;
            found_target = target;
         }
         else
         {
            candidate.push_back(std::pair<unsigned, Address>(i, target));
         }
      }
   }

   if (found_word != MAX_WANDERER_DEPTH && candidate.size()) {
      /**
       * I'm not sure what the best way to pick between candidates
       * would be.  For now, we're picking the first one found.
       **/
      found_word = candidate[0].first;
      found_target = candidate[0].second;
   }

   if (found_word == MAX_WANDERER_DEPTH) {
      sw_printf("[%s:%u] - Wanderer couldn't find anything in %u words\n",
                __FILE__, __LINE__, MAX_WANDERER_DEPTH);
      return gcf_not_me;
   }
   out.setRA(words[found_word]);
   out.setSP(in.getSP() + (found_word+1) * addr_width);
   
   if (fhelper->allocatesFrame(in.getRA()).first == FrameFuncHelper::savefp_only_frame) {
      Address new_fp;
      result = getProcessState()->readMem(&new_fp, out.getSP(), 
                                          getProcessState()->getAddressWidth());
      if (!result) {
         sw_printf("[%s:%u] - Error, couln't read from stack at %lx\n",
                   __FILE__, __LINE__, out.getSP());
         return gcf_error;
      }
      out.setFP(new_fp);
   }
   else {
      out.setFP(in.getFP());
   }

   return gcf_success;
   
}

bool StepperWandererImpl::getTopWords(Address *words_out, Address start)
{
   bool result;
   Address words[MAX_WANDERER_DEPTH];
   const unsigned addr_width = getProcessState()->getAddressWidth();

   result = getProcessState()->readMem(words, start, 
                                       MAX_WANDERER_DEPTH * addr_width);
   if (!result) {
      sw_printf("[%s:%u] - Wanderer couldn't read from stack at 0x%lx\n",
                __FILE__, __LINE__, start);
      return false;
   }

#if defined(arch_x86_64)
   if (addr_width == 4) {
      uint32_t *words_32 = (uint32_t *) (&(words[0]));
      for (unsigned i=0; i<MAX_WANDERER_DEPTH; i++)
      {
         words_out[i] = static_cast<Address>(words_32[i]);
      }
      return true;
   }
#endif

   for (unsigned i=0; i<MAX_WANDERER_DEPTH; i++)
   {
      words_out[i] = words[i];
   }

   return true;
}

bool WandererHelper::isPrevInstrACall(Address addr, Address &target)
{
   const unsigned max_call_length = 5;
   bool result;
   unsigned char buffer[max_call_length];

   Address start = addr - max_call_length;
   result = proc->readMem(buffer, start, max_call_length);
   if (!result)
   {
      sw_printf("[%s:%u] - Address 0x%lx is not a call--unreadable\n",
                __FILE__, __LINE__, addr);
      return false;
   }

   if (buffer[max_call_length - 5] == 0xe8) {
      int32_t disp = *((uint32_t *) buffer+1);
      target = addr + disp;
      sw_printf("[%s:%u] - Found call encoded by %x to %lx\n",
                __FILE__, __LINE__, (int) buffer[0], target);
                
      return true;
   }

   target = 0x0;
   for (unsigned i=0; i<max_call_length-1; i++)
   {
      if (buffer[i] != 0xff) 
         continue;
      int modrm_reg = buffer[i+1] >> 3 & 7;
      if (modrm_reg != 2)
         continue;

      /**
       * Compute the size of the x86 instruction.
       **/
      int modrm_mod = buffer[i+1] >> 6;
      int modrm_rm = buffer[i+1] & 7;
      unsigned size = 2; //Opcode + MOD/RM
      switch (modrm_mod)
      {
         case 0:
            if (modrm_rm == 5)
               size += 4; //disp32
            if (modrm_rm == 4)
               size += 1; //SIB
            break;
         case 1:
            size += 1; //disp8
            if (modrm_rm == 4)
               size += 1; //SIB
            break;
         case 2:
            size += 4; //disp32
            if (modrm_rm == 4)
               size += 1; //SIB
            break;
         case 3:
            break;
      }

      if (i + size == max_call_length)
      {
         sw_printf("[%s:%u] - Found call of size %d encoded by: ",
                   __FILE__, __LINE__, size);
         for (unsigned j=i; j<i+size; j++) {
            sw_printf("%x ", buffer[j]);
         }
         sw_printf("\n");

         return true;
      }
   }

   return false;
}

bool WandererHelper::isPCInFunc(Address func_entry, Address pc)
{
   if (!func_entry || !pc || func_entry > pc)
      return false;
   if (func_entry == pc)
      return true;
#if defined(cap_stackwalker_use_symtab)
   Symtab *symtab = NULL;
   LibAddrPair func_lib, pc_lib;
   LibraryState *tracker = proc->getLibraryTracker();
   std::vector<Symbol *> funcs;

   bool result = tracker->getLibraryAtAddr(func_entry, func_lib);
   if (!result) {
      sw_printf("[%s:%u] - Failed to find library at %lx\n",
                __FILE__, __LINE__, func_entry);
      return false;
   }
      
   result = tracker->getLibraryAtAddr(pc, pc_lib);
   if (!result || pc_lib != func_lib)
   {
      sw_printf("[%s:%u] - %lx and %lx are from different libraries\n",
                __FILE__, __LINE__, func_entry, pc);
      return false;
   }

   Offset func_entry_offset = func_entry - func_lib.second;
   Offset pc_offset = pc - pc_lib.second;
   SymtabAPI::Function *func_symbol = NULL;
   SymtabAPI::Function *pc_symbol = NULL;

   symtab = SymtabWrapper::getSymtab(func_lib.first);
   if (!symtab) {
      sw_printf("[%s:%u] - Failed to open symtab for %s\n", 
                __FILE__, __LINE__, func_lib.first.c_str());
      goto symtab_fail;
   }

   result = symtab->getContainingFunction(func_entry_offset, func_symbol);
   if (!result || !func_symbol) {
      sw_printf("[%s:%u] - No functions begin at %lx\n", 
                __FILE__, __LINE__, func_entry_offset);
      goto symtab_fail;
   }

   result = symtab->getContainingFunction(pc_offset, pc_symbol);
   if (!result || !pc_symbol) {
      sw_printf("[%s:%u] - No functions begin at %lx\n", 
                __FILE__, __LINE__, func_entry_offset);
      goto symtab_fail;
   }

   return (func_symbol->getOffset() == pc_symbol->getOffset());
 symtab_fail:   
#endif
   //We don't have much to work with.  This is all heuristics anyway, so assume
   // that if pc is within 8k of func_entry that it's a part of the function.
   return (pc >= func_entry && pc < func_entry + 8192);
}

WandererHelper::WandererHelper(ProcessState *proc_) :
   proc(proc_)
{
}

WandererHelper::~WandererHelper()
{
}

StepperWanderer::StepperWanderer(Walker *w, WandererHelper *whelper, 
                                 FrameFuncHelper *fhelper) :
   FrameStepper(w)
{
   impl = new StepperWandererImpl(w, whelper, fhelper, this);
}

gcframe_ret_t StepperWanderer::getCallerFrame(const Frame &in, Frame &out)
{
   assert(impl);
   return impl->getCallerFrame(in, out);
}

unsigned StepperWanderer::getPriority() const
{
   assert(impl);
   return impl->getPriority();
}

StepperWanderer::~StepperWanderer()
{
   assert(impl);
   delete impl;
   impl = NULL;
}
