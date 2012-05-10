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

#if defined(arch_x86_64) && (defined(os_linux) || defined(os_freebsd))
  __asm__("mov %%rbp, %0\n"
	  : "=r"(frame_pointer));
#elif defined(os_linux) || defined(os_freebsd)
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
                  __FILE__, __LINE__, reg.name().c_str());
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
  out_sp = in_fp + (2 * addr_width);

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
    sw_printf("[%s:%u] - Couldn't read from %lx\n", __FILE__, __LINE__, in_fp);
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

   result = proc->readMem(mem, func_addr, FUNCTION_PROLOG_TOCHECK);
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
   Address in_sp = in.getSP(),
           in_fp = in.getFP(),
           out_sp = 0,
           out_ra = 0,
           out_ra_addr = 0,
           out_fp = 0,
           out_fp_addr = 0;
   StackAnalysis::Height sp_height = height.first;
   StackAnalysis::Height fp_height = height.second;
   location_t out_ra_loc, out_fp_loc;

   ProcessState *proc = getProcessState();

   if (sp_height == StackAnalysis::Height::bottom) {
      sw_printf("[%s:%u] - Analysis didn't find a stack height\n", 
                __FILE__, __LINE__);
      return gcf_not_me;
   }

   // SP height is the distance from the last SP of the previous frame
   // to the SP in this frame at the current offset.
   // Since we are walking to the previous frame,
   // we subtract this height to get the outgoing SP
   out_sp = in_sp - sp_height.height();

   // Since we know the outgoing SP,
   // the outgoing RA must be located just below it
   out_ra_addr = out_sp - proc->getAddressWidth();
   out_ra_loc.location = loc_address;
   out_ra_loc.val.addr = out_ra_addr;

   bool result = proc->readMem(&out_ra, out_ra_addr, proc->getAddressWidth());
   if (!result) {
      sw_printf("[%s:%u] - Error reading from return location %lx on stack\n",
                __FILE__, __LINE__, out_ra_addr);
      return gcf_not_me;
   }

   if (fp_height != StackAnalysis::Height::bottom) {
      // FP height is the distance from the last SP of the previous frame
      // to the FP in this frame at the current offset.
      // If analysis finds this height,
      // then out SP + FP height should equal in FP.
      // We then assume that in FP points to out FP.
      out_fp_addr = out_sp + fp_height.height();

      if (out_fp_addr != in_fp) {
         sw_printf(
            "[%s:%u] - Warning - current FP %lx does not point to next FP located at %lx\n",
            __FILE__, __LINE__, in_fp, out_fp_addr);
      }

      result = proc->readMem(&out_fp, out_fp_addr, proc->getAddressWidth());
      if (result) {
         out_fp_loc.location = loc_address;
         out_fp_loc.val.addr = out_fp_addr;

         out.setFPLocation(out_fp_loc);
         out.setFP(out_fp);
      }
      else {
         sw_printf("[%s:%u] - Failed to read FP value\n", __FILE__, __LINE__);
      }
   }
   else {
      sw_printf("[%s:%u] - Did not find frame pointer in analysis\n",
                __FILE__, __LINE__);
   }

   out.setSP(out_sp);
   out.setRALocation(out_ra_loc);
   out.setRA(out_ra);

   return gcf_success;
}

gcframe_ret_t DyninstDynamicStepperImpl::getCallerFrameArch(const Frame &in, Frame &out, 
                                                            Address /*base*/, Address lib_base,
                                                            unsigned /*size*/, unsigned stack_height,
                                                            Address orig_ra, bool pEntryExit)
{
  bool result = false;
  const unsigned addr_width = getProcessState()->getAddressWidth();
  unsigned long sp_value = 0x0;
  Address sp_addr = 0x0;

  // Handle frameless instrumentation
  if (0x0 != orig_ra)
  {
    location_t unknownLocation;
    unknownLocation.location = loc_unknown;
    out.setRA(orig_ra);
    out.setFP(in.getFP());
    out.setSP(in.getSP()); //Not really correct, but difficult to compute and unlikely to matter
    out.setRALocation(unknownLocation);
    sw_printf("[%s:%u] - DyninstDynamicStepper handled frameless instrumentation\n",
              __FILE__, __LINE__);
    return gcf_success;
  }

  // Handle case where *previous* frame was entry/exit instrumentation
  if (pEntryExit)
  {
    Address ra_value = 0x0;

    // RA is pointed to by input SP
    // TODO may have an additional offset in some cases...
    Address newRAAddr = in.getSP();

    location_t raLocation;
    raLocation.location = loc_address;
    raLocation.val.addr = newRAAddr;
    out.setRALocation(raLocation);

    // TODO handle 64-bit mutator / 32-bit mutatee

    // get value of RA
    result = getProcessState()->readMem(&ra_value, newRAAddr, addr_width);

    if (!result) {
      sw_printf("[%s:%u] - Couldn't read from %lx\n", __FILE__, __LINE__, newRAAddr);
      return gcf_error;
    }

    out.setRA(ra_value);
    out.setFP(in.getFP()); // FP stays the same
    out.setSP(newRAAddr + addr_width);
    sw_printf("[%s:%u] - DyninstDynamicStepper handled post entry/exit instrumentation\n",
              __FILE__, __LINE__);
    return gcf_success;
  }

  gcframe_ret_t ret = HandleStandardFrame(in, out, getProcessState());
  if (ret != gcf_success)
    return ret;
  out.setRA(out.getRA() + lib_base);

  // For tramps with frames, read the saved stack pointer
  // TODO does this apply to static instrumentation?
  if (stack_height)
  {
    sp_addr = in.getFP() + stack_height;
    result = getProcessState()->readMem(&sp_value, sp_addr, addr_width);

    if (!result) {
      sw_printf("[%s:%u] - Couldn't read from %lx\n", __FILE__, __LINE__, sp_addr);
      return gcf_error;
    }

    sw_printf("[%s:%u] - Read SP %p from addr %p, using stack height of 0x%lx\n",
              __FILE__, __LINE__, sp_value, sp_addr, stack_height);
    out.setSP(sp_value);
  }

  sw_printf("[%s:%u] - DyninstDynamicStepper handled normal instrumentation\n",
            __FILE__, __LINE__);
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
      buffer[0] = (char) 0xcc; //trap
      buffer[1] = (char) 0xc3; //ret
      actual_len = 2;
      return;
   }
   assert(buf_size >= 1);
   buffer[0] = (char) 0xcc; //trap
   actual_len = 1;
   return;
}
}
}

