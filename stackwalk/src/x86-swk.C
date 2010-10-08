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

#include "stackwalk/h/basetypes.h"
#include "stackwalk/h/swk_errors.h"
#include "stackwalk/h/procstate.h"
#include "stackwalk/h/framestepper.h"
#include "stackwalk/h/frame.h"
#include "stackwalk/h/walker.h"

#include "stackwalk/src/symtab-swk.h"
#include "stackwalk/src/dbgstepper-impl.h"
#include "stackwalk/src/x86-swk.h"
#include "stackwalk/src/sw.h"
#include "stackwalk/src/libstate.h"

#include "common/h/lru_cache.h"

#include "dynutil/h/SymReader.h"

using namespace Dyninst;
using namespace Dyninst::Stackwalker;

bool ProcSelf::getRegValue(Dyninst::MachRegister reg, THR_ID, Dyninst::MachRegisterVal &val)
{
  unsigned long *frame_pointer;

#if defined(arch_x86_64) && defined(os_linux)
  __asm__("mov %%rbp, %0\n"
	  : "=r"(frame_pointer));
#elif defined(os_linux)
  __asm__("movl %%ebp, %0\n"
	  : "=r"(frame_pointer));
#elif defined(os_windows)
   __asm
   {
      mov frame_pointer, ebp ;
   }   
#endif

  frame_pointer = (unsigned long *) *frame_pointer;
  
  switch(reg.val())
  {
     case Dyninst::x86_64::irip:
     case Dyninst::x86::ieip:
     case Dyninst::iReturnAddr:
        val = (Dyninst::MachRegisterVal) frame_pointer[1];
        break;
     case Dyninst::iFrameBase:
        val = (Dyninst::MachRegisterVal) frame_pointer[0];
        break;
     case Dyninst::x86_64::irsp:
     case Dyninst::x86::iesp:
     case Dyninst::iStackTop:
        val = (Dyninst::MachRegisterVal) (frame_pointer + 2);
        break;      
     default:
        sw_printf("[%s:%u] - Request for unsupported register %s\n",
                  __FILE__, __LINE__, reg.name());
        setLastError(err_badparam, "Unknown register passed in reg field");
  }

  return true;
}

Dyninst::Architecture ProcSelf::getArchitecture()
{
   if (sizeof(void *) == 8)
      return Arch_x86_64;
   return Arch_x86;
}

static gcframe_ret_t HandleStandardFrame(const Frame &in, Frame &out, ProcessState *proc)
{
  Address in_fp, out_sp;
  const unsigned addr_width = proc->getAddressWidth();
  bool result;

  struct {
    Address out_fp;
    Address out_ra;
  } ra_fp_pair;

  in_fp = in.getFP();
  out_sp = in_fp + addr_width;

#if defined(arch_x86_64)
  /**
   * On AMD64 we may be reading from a process with a different
   * address width than the current one.  We'll do the read at
   * the correct size, then convert the addresses into the 
   * local size
   **/
  struct {
     unsigned out_fp;
     unsigned out_ra;
  } ra_fp_pair32;
  if (addr_width != sizeof(Address))
  {
     result = proc->readMem(&ra_fp_pair32, in_fp, 
			    sizeof(ra_fp_pair32));
     ra_fp_pair.out_fp = (Address) ra_fp_pair32.out_fp;
     ra_fp_pair.out_ra = (Address) ra_fp_pair32.out_ra;
  }
  else
#endif
  {
    result = proc->readMem(&ra_fp_pair, in_fp, sizeof(ra_fp_pair));
  }

  if (!result) {
    sw_printf("[%s:%u] - Couldn't read from %lx\n", __FILE__, __LINE__, out_sp);
    return gcf_error;
  }
  
  if (!ra_fp_pair.out_ra) {
     return gcf_not_me;
  }

  out.setFP(ra_fp_pair.out_fp);
  out.setRA(ra_fp_pair.out_ra);
  out.setSP(out_sp);

  return gcf_success;
}

bool Walker::checkValidFrame(const Frame &in, const Frame &out)
{
   if (out.getSP() <= in.getSP()) {
      sw_printf("[%s:%u] - Stackwalk went backwards, %lx to %lx\n",
                __FILE__, __LINE__, in.getSP(), out.getSP());
      return false;
   }
   return true;
}

gcframe_ret_t FrameFuncStepperImpl::getCallerFrame(const Frame &in, Frame &out)
{
  if (!in.getFP())
     return gcf_not_me;

  FrameFuncHelper::alloc_frame_t frame = helper->allocatesFrame(in.getRA());
  if (frame.first != FrameFuncHelper::standard_frame) {
     return gcf_not_me;
  }

  return HandleStandardFrame(in, out, getProcessState());
}
 
std::map<Dyninst::PID, LookupFuncStart*> LookupFuncStart::all_func_starts;

static int hash_address(Address a)
{
   return (int) a;
}

LookupFuncStart::LookupFuncStart(ProcessState *proc_) :
   FrameFuncHelper(proc_),
   cache(cache_size, hash_address)
{
   all_func_starts[proc->getProcessId()] = this;
   ref_count = 1;
}

LookupFuncStart::~LookupFuncStart()
{
   Dyninst::PID pid = proc->getProcessId();
   all_func_starts.erase(pid);
}

LookupFuncStart *LookupFuncStart::getLookupFuncStart(ProcessState *p)
{
   Dyninst::PID pid = p->getProcessId();
   std::map<Dyninst::PID, LookupFuncStart*>::iterator i = all_func_starts.find(pid);
   if (i == all_func_starts.end()) {
      return new LookupFuncStart(p);
   }
   (*i).second->ref_count++;
   return (*i).second;
}

void LookupFuncStart::releaseMe()
{
   ref_count--;
   if (!ref_count)
      delete this;
}

FrameFuncStepperImpl::FrameFuncStepperImpl(Walker *w, FrameStepper *parent_,
                                           FrameFuncHelper *helper_) :
   FrameStepper(w),
   parent(parent_),
   helper(helper_)
{
   helper = helper_ ? helper_ : LookupFuncStart::getLookupFuncStart(getProcessState());
}

FrameFuncStepperImpl::~FrameFuncStepperImpl()
{
   LookupFuncStart *lookup = dynamic_cast<LookupFuncStart*>(helper);
   if (lookup)
      lookup->releaseMe();
   else if (helper)
      delete helper;
}

unsigned FrameFuncStepperImpl::getPriority() const
{
  return frame_priority;
}

/**
 * Look at the first few bytes in the function and see if they contain
 * the standard set to allocate a stack frame.
 **/
#define FUNCTION_PROLOG_TOCHECK 12
static unsigned char push_ebp = 0x55;
static unsigned char mov_esp_ebp[2][2] = { { 0x89, 0xe5 },
                                           { 0x8b, 0xec } };
static unsigned char rex_mov_esp_ebp = 0x48;

FrameFuncHelper::alloc_frame_t LookupFuncStart::allocatesFrame(Address addr)
{
   LibAddrPair lib;
   unsigned char mem[FUNCTION_PROLOG_TOCHECK];
   Address func_addr;
   unsigned cur;
   int push_ebp_pos = -1, mov_esp_ebp_pos = -1;
   alloc_frame_t res = alloc_frame_t(unknown_t, unknown_s);
   bool result;
   SymReader *reader;
   Offset off;
   Symbol_t sym;

   result = checkCache(addr, res);
   if (result) {
      sw_printf("[%s:%u] - Cached value for %lx is %d/%d\n",
                __FILE__, __LINE__, addr, (int) res.first, (int) res.second);
      return res;
   }

   result = proc->getLibraryTracker()->getLibraryAtAddr(addr, lib);
   if (!result)
   {
      sw_printf("[%s:%u] - No library at %lx\n", __FILE__, __LINE__, addr);
      goto done;
   }

   reader = LibraryWrapper::getLibrary(lib.first);
   if (!reader) {
      sw_printf("[%s:%u] - Failed to open symbol reader %s\n",
                __FILE__, __LINE__, lib.first.c_str() );
      goto done;
   }   
   off = addr - lib.second;
   sym = reader->getContainingSymbol(off);
   if (!reader->isValidSymbol(sym)) {
      sw_printf("[%s:%u] - Could not find symbol in binary\n", __FILE__, __LINE__);
      goto done;
   }
   func_addr = reader->getSymbolOffset(sym) + lib.second;

   if (!result) {
      sw_printf("[%s:%u] - Error.  Couldn't read from memory at %lx\n",
                __FILE__, __LINE__, func_addr);
      goto done;
   }
   
   //Try to find a 'push (r|e)bp'
   for (cur=0; cur<FUNCTION_PROLOG_TOCHECK; cur++)
   {
      if (mem[cur] == push_ebp)
      {
         push_ebp_pos = cur;
         break;
      }
   }

   //Try to find the mov esp->ebp
   for (; cur<FUNCTION_PROLOG_TOCHECK; cur++)
   {
      if (proc->getAddressWidth() == 8) {
         if (mem[cur] != rex_mov_esp_ebp)
            continue;
         cur++;
      }
      
      if (cur+1 >= FUNCTION_PROLOG_TOCHECK) 
         break;
      if ((mem[cur] == mov_esp_ebp[0][0] && mem[cur+1] == mov_esp_ebp[0][1]) || 
          (mem[cur] == mov_esp_ebp[1][0] && mem[cur+1] == mov_esp_ebp[1][1])) {
         if (proc->getAddressWidth() == 8) 
            mov_esp_ebp_pos = cur-1;
         else
            mov_esp_ebp_pos = cur;
         break;
      }
   }

   if (push_ebp_pos != -1 && mov_esp_ebp_pos != -1)
      res.first = standard_frame;
   else if (push_ebp_pos != -1 && mov_esp_ebp_pos == -1)
      res.first = savefp_only_frame;
   else 
      res.first = no_frame;
   
   if (push_ebp_pos != -1 && addr <= func_addr + push_ebp_pos)
      res.second = unset_frame;
   else if (mov_esp_ebp_pos != -1 && addr <= func_addr + mov_esp_ebp_pos)
      res.second = halfset_frame;
   else
      res.second = set_frame;

 done:
   sw_printf("[%s:%u] - Function containing %lx has frame type %d/%d\n",
             __FILE__, __LINE__, addr, (int) res.first, (int) res.second);
   updateCache(addr, res);
   return res;
}

void LookupFuncStart::updateCache(Address addr, alloc_frame_t result)
{
   cache.insert(addr, result);
}

bool LookupFuncStart::checkCache(Address addr, alloc_frame_t &result)
{
   return cache.lookup(addr, result);
}

void LookupFuncStart::clear_func_mapping(Dyninst::PID pid)
{
   std::map<Dyninst::PID, LookupFuncStart *>::iterator i;
   i = all_func_starts.find(pid);
   if (i == all_func_starts.end())
      return;

   LookupFuncStart *fs = (*i).second;
   all_func_starts.erase(i);
   
   delete fs;
}

gcframe_ret_t DyninstInstrStepperImpl::getCallerFrameArch(const Frame &in, Frame &out, 
                                                          Address /*base*/, Address lib_base,
                                                          unsigned /*size*/, unsigned stack_height)
{
  gcframe_ret_t ret = HandleStandardFrame(in, out, getProcessState());
  if (ret != gcf_success)
    return ret;
  out.setRA(out.getRA() + lib_base);
  out.setSP(out.getSP() + stack_height);
  return gcf_success;
}

#include "analysis_stepper.h"
gcframe_ret_t AnalysisStepperImpl::getCallerFrameArch(height_pair_t height,
                                                      const Frame &in, Frame &out)
{
   Address in_sp = in.getSP();
   StackAnalysis::Height pc_height = height.first;
   StackAnalysis::Height fp_height = height.second;

   ProcessState *proc = getProcessState();

   Address ret_addr = 0;
   
   if (pc_height == StackAnalysis::Height::bottom) {
      sw_printf("[%s:%u] - Analysis didn't find a stack height\n", 
                __FILE__, __LINE__);
      return gcf_not_me;
   }

   Address ret_loc = in_sp - pc_height.height() - proc->getAddressWidth();

   bool result = proc->readMem(&ret_addr, ret_loc, proc->getAddressWidth());
   if (!result) {
      sw_printf("[%s:%u] - Error reading from return location %lx on stack\n",
                __FILE__, __LINE__, ret_addr);
      return gcf_not_me;
   }
   location_t ra_loc;
   ra_loc.val.addr = ret_loc;
   ra_loc.location = loc_address;
   out.setRALocation(ra_loc);
   out.setRA(ret_addr);
   out.setSP(ret_loc + proc->getAddressWidth());

   Address fp_addr = 0;
   Address fp_loc = 0;
   if (fp_height != StackAnalysis::Height::bottom) {
      fp_loc = ret_loc + fp_height.height() - proc->getAddressWidth();
      result = proc->readMem(&fp_addr, fp_loc, proc->getAddressWidth());
      if (result) {
         out.setFP(fp_addr);
         location_t fp_loc;
         ra_loc.val.addr = fp_addr;
         ra_loc.location = loc_address;
         out.setFPLocation(fp_loc);
      }
      else { 
         sw_printf("[%s:%u] - Failed to read FP value\n", __FILE__, __LINE__);
      }
   }
   else {
      sw_printf("[%s:%u] - Did not find frame pointer in analysis\n",
                __FILE__, __LINE__);
   }

   return gcf_success;
}

namespace Dyninst {
namespace Stackwalker {
void getTrapInstruction(char *buffer, unsigned buf_size, 
                        unsigned &actual_len, bool include_return)
{
   if (include_return)
   {
      assert(buf_size >= 2);
      buffer[0] = 0xcc; //trap
      buffer[1] = 0xc3; //ret
      actual_len = 2;
      return;
   }
   assert(buf_size >= 1);
   buffer[0] = 0xcc; //trap
   actual_len = 1;
   return;
}
}
}

