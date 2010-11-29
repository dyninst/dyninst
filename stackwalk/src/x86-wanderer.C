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

#include "stackwalk/h/walker.h"
#include "stackwalk/h/frame.h"
#include "stackwalk/h/swk_errors.h"
#include "stackwalk/h/procstate.h"

#include "stackwalk/src/x86-swk.h"
#include "stackwalk/src/symtab-swk.h"
#include "stackwalk/src/libstate.h"

#include "common/h/Types.h"

#include "dynutil/h/SymReader.h"

using namespace Dyninst;
using namespace Stackwalker;


StepperWandererImpl::StepperWandererImpl(Walker *walker_,
                                         StepperWanderer *parent_,
                                         WandererHelper *whelper_,
                                         FrameFuncHelper *fhelper_) :
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
   return wanderer_priority;
}

void StepperWandererImpl::registerStepperGroup(StepperGroup *group) {
  FrameStepper::registerStepperGroup(group);
}

gcframe_ret_t StepperWandererImpl::getCallerFrame(const Frame &in, Frame &out)
{
   sw_printf("[%s:%u] - Wanderer attempting to walk from 0x%lx\n",
             __FILE__, __LINE__, in.getRA());

   const unsigned addr_width = getProcessState()->getAddressWidth();
   std::vector<std::pair<Address, Address> > candidate;

   Address current_stack = in.getSP();
   unsigned num_words_tried = 0;
   Address word;
   bool result;
   bool found_exact_match = false;
   Address found_base = 0x0;
   Address found_ra = 0x0;
   FrameFuncHelper::alloc_frame_t alloc_res;

   do {
      result = getWord(word, current_stack);
      if (!result) {
         sw_printf("[%s:%u] - getWord returned false\n", __FILE__, __LINE__);
         return gcf_not_me;
      }
      
      Address target;
      if (whelper->isPrevInstrACall(word, target))
      {
         if (whelper->isPCInFunc(target, in.getRA()))
         {
            sw_printf("[%s:%u] - Wanderer thinks word 0x%lx at 0x%lx  is return "
                      " address\n", __FILE__, __LINE__, word, current_stack);
            found_base = current_stack;
            found_ra = word;
            found_exact_match = true;
            break;
         }
         else
         {
            candidate.push_back(std::pair<Address, Address>(word, current_stack));
         }
      }
      current_stack += addr_width;
      num_words_tried++;
   } while (num_words_tried < MAX_WANDERER_DEPTH);

   if (!found_exact_match && !candidate.size()) {
      sw_printf("[%s:%u] - Wanderer couldn't find anything in %u words\n",
                __FILE__, __LINE__, MAX_WANDERER_DEPTH);
      return gcf_not_me;
   }

   if (!found_exact_match && candidate.size()) {
      /**
       * If we ever want to rely on candidates, then uncomment the above
       * push_back and the below code.  This trades false negatives for
       * potential false positives, but I'm not sure it's worth it.
       **/
      //return gcf_not_me;
      found_ra = candidate[0].first;
      found_base = candidate[0].second;
   }

   out.setRA(found_ra);
   out.setSP(found_base + addr_width);

   alloc_res = fhelper->allocatesFrame(in.getRA());
   if (alloc_res.first == FrameFuncHelper::savefp_only_frame &&
       alloc_res.second != FrameFuncHelper::unset_frame) {
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

bool StepperWandererImpl::getWord(Address &word_out, Address start)
{
   const unsigned addr_width = getProcessState()->getAddressWidth();
   if (start < 1024) {
      sw_printf("[%s:%u] - %lx too low to be valid memory\n",
                __FILE__, __LINE__, start);
      return false;
   }
   word_out = 0x0;
   bool result = getProcessState()->readMem(&word_out, start, addr_width);
   if (!result) {
      sw_printf("[%s:%u] - Wanderer couldn't read from stack at 0x%lx\n",
                __FILE__, __LINE__, start);
      return false;
   }

   return true;
}

bool WandererHelper::isPrevInstrACall(Address addr, Address &target)
{
   const unsigned max_call_length = 5;
   bool result;
   unsigned char buffer[max_call_length];

   sw_printf("[%s:%u] - isPrevInstrACall on %lx\n", __FILE__, __LINE__, addr);
   Address start = addr - max_call_length;
   result = proc->readMem(buffer, start, max_call_length);
   if (!result)
   {
      sw_printf("[%s:%u] - Address 0x%lx is not a call--unreadable\n",
                __FILE__, __LINE__, addr);
      return false;
   }

   if (buffer[max_call_length - 5] == 0xe8) {
      int32_t disp = *((int32_t *) (buffer+1));
      target = addr + disp;
      sw_printf("[%s:%u] - Found call encoded by %x to %lx (addr = %lx, disp = %lx)\n",
                __FILE__, __LINE__, (int) buffer[0], target, addr, disp);
                
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
   if (!func_entry || !pc)
      return false;
   if (func_entry == pc)
      return true;

   SymReader *reader = NULL;
   LibAddrPair func_lib, pc_lib;
   LibraryState *tracker = proc->getLibraryTracker();
   Offset pc_offset, func_entry_offset, func_offset;
   bool result;
   Symbol_t func_symbol, pc_symbol;
   Section_t section;

   result = tracker->getLibraryAtAddr(func_entry, func_lib);
   if (!result) {
      sw_printf("[%s:%u] - Failed to find library at %lx\n",
                __FILE__, __LINE__, func_entry);
      return false;
   }
   func_entry_offset = func_entry - func_lib.second;

   reader = LibraryWrapper::getLibrary(func_lib.first);
   if (!reader) {
      sw_printf("[%s:%u] - Failed to open reader for %s\n", 
                __FILE__, __LINE__, func_lib.first.c_str());
      goto reader_fail;
   }
   
   section = reader->getSectionByAddress(func_entry_offset);
   if (reader->getSectionName(section) == std::string(".plt")) {
      sw_printf("[%s:%u] - %lx is a PLT entry, trying to map to real target\n",
                __FILE__, __LINE__, func_entry);
      int got_offset = -1;
      Address got_abs = 0x0;
      if (proc->getAddressWidth() == 4) {
         //32-bit mode.  Recognize common PLT idioms
         #define MAX_PLT32_IDIOM_SIZE 6
         unsigned char buffer[MAX_PLT32_IDIOM_SIZE];
         result = proc->readMem(buffer, func_entry, MAX_PLT32_IDIOM_SIZE);
         if (buffer[0] == 0xff && buffer[1] == 0xa3) {
            //Indirect jump off of ebx
            got_offset = *((int32_t*) (buffer+2));
         }
         else if (buffer[0] == 0xff && buffer[1] == 0x25) {
            //Indirect jump through absolute
            got_abs = *((uint32_t*) (buffer+2));
         }
         else {
            sw_printf("[%s:%u] - Unrecognized PLT idiom at %lx: ",
                      __FILE__, __LINE__, func_entry);
            for (unsigned i=0; i<MAX_PLT32_IDIOM_SIZE; i++) {
               sw_printf("%x ", buffer[i]);
            }
            sw_printf("\n");
         }
      }
      if (proc->getAddressWidth() == 8) {
         //32-bit mode.  Recognize common PLT idioms
#define MAX_PLT64_IDIOM_SIZE 6
         unsigned char buffer[MAX_PLT64_IDIOM_SIZE];
         result = proc->readMem(buffer, func_entry, MAX_PLT64_IDIOM_SIZE);
         if (buffer[0] == 0xff && buffer[1] == 0x25) {
            //PC Relative jump indirect
            got_abs = *((int32_t *) (buffer+2)) + func_entry + 6;
         }
         else {
            sw_printf("[%s:%u] - Unrecognized PLT idiom at %lx: ",
                      __FILE__, __LINE__, func_entry);
            for (unsigned i=0; i<MAX_PLT32_IDIOM_SIZE; i++) {
               sw_printf("%x ", buffer[i]);
            }
            sw_printf("\n");
         }
      }
      
      if (got_offset != -1) {
         sw_printf("[%s:%u] - Computed PLT to be going through GOT offset %d\n",
                   __FILE__, __LINE__, got_offset);
         Section_t got_section = reader->getSectionByName(".got");
         if (reader->isValidSection(got_section)) {
            got_abs = got_offset + reader->getSectionAddress(got_section) + 
                      func_lib.second;
         }
      }

      if (got_abs) {
         Address real_target;
         result = proc->readMem(&real_target, got_abs, proc->getAddressWidth());
         sw_printf("[%s:%u] - Computed PLT to be going through GOT abs %lx to "
                   "real target %lx\n", __FILE__, __LINE__, got_abs, real_target);
         return isPCInFunc(real_target, pc);
      }

   }

   result = tracker->getLibraryAtAddr(pc, pc_lib);
   if (!result) {
      sw_printf("[%s:%u] - Failed to find library at %lx\n",
                __FILE__, __LINE__, pc);
      return false;
   }
   pc_offset = pc - pc_lib.second;


   if (func_entry > pc) {
      sw_printf("[%s:%u] - func_entry %lx is greater than pc %lx\n",
                __FILE__, __LINE__, func_entry, pc);
      return false;
   }
   
   if (pc_lib != func_lib)
   {
      sw_printf("[%s:%u] - %lx and %lx are from different libraries\n",
                __FILE__, __LINE__, func_entry, pc);
      return false;
   }

   func_symbol = reader->getContainingSymbol(func_entry_offset);
   if (!reader->isValidSymbol(func_symbol)) {
      sw_printf("[%s:%u] - No functions begin at %lx\n", 
                __FILE__, __LINE__, func_entry_offset);
      goto reader_fail;
   }

   pc_symbol = reader->getContainingSymbol(pc_offset);
   if (!reader->isValidSymbol(pc_symbol)) {
      sw_printf("[%s:%u] - No functions begin at %lx\n", 
                __FILE__, __LINE__, func_entry_offset);
      goto reader_fail;
   }

   func_offset = reader->getSymbolOffset(func_symbol);
   pc_offset = reader->getSymbolOffset(pc_symbol);
   sw_printf("[%s:%u] - Decided func at offset %lx and pc-func at offset %lx\n",
             __FILE__, __LINE__, func_offset, pc_offset);
   return (func_offset == pc_offset);
 reader_fail:   

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
