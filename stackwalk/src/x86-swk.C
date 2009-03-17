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
#include "symtabAPI/h/Function.h"

#if defined(cap_cache_func_starts)
#include "common/h/lru_cache.h"
#endif

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
  
  switch(reg)
  {
    case Dyninst::MachRegPC:
      val = (Dyninst::MachRegisterVal) frame_pointer[1];
      break;
    case Dyninst::MachRegFrameBase:
      val = (Dyninst::MachRegisterVal) frame_pointer[0];
      break;      
    case Dyninst::MachRegStackBase:
       val = (Dyninst::MachRegisterVal) (frame_pointer + 2);
      break;      
    default:
       sw_printf("[%s:%u] - Request for unsupported register %d\n",
                 __FILE__, __LINE__, reg);
       setLastError(err_badparam, "Unknown register passed in reg field");
  }

  return true;
}

gcframe_ret_t FrameFuncStepperImpl::getCallerFrame(const Frame &in, Frame &out)
{
  Address in_fp, out_sp;
  bool result;
  const unsigned addr_width = getProcessState()->getAddressWidth();

  struct {
    Address out_fp;
    Address out_ra;
  } ra_fp_pair;

  if (!in.getFP())
     return gcf_not_me;

  FrameFuncHelper::alloc_frame_t frame = helper->allocatesFrame(in.getRA());
  if (frame.first != FrameFuncHelper::standard_frame) {
     return gcf_not_me;
  }

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
     result = getProcessState()->readMem(&ra_fp_pair32, in_fp, 
                                         sizeof(ra_fp_pair32));
     ra_fp_pair.out_fp = (Address) ra_fp_pair32.out_fp;
     ra_fp_pair.out_ra = (Address) ra_fp_pair32.out_ra;
  }
  else
#endif
     result = getProcessState()->readMem(&ra_fp_pair, in_fp, 
                                         sizeof(ra_fp_pair));
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
 
class LookupFuncStart : public FrameFuncHelper
{
private:
   static std::map<Dyninst::PID, LookupFuncStart*> all_func_starts;
   LookupFuncStart(ProcessState *proc_);
   int ref_count;
private:
   void updateCache(Address addr, alloc_frame_t result);
   bool checkCache(Address addr, alloc_frame_t &result);
#if defined(cap_cache_func_starts)
   //We need some kind of re-entrant safe synhronization before we can
   // globally turn this caching on, but it would sure help things.
   static const unsigned int cache_size = 64;
   LRUCache<Address, alloc_frame_t> cache;
#endif
public:
   static LookupFuncStart *getLookupFuncStart(ProcessState *p);
   void releaseMe();
   virtual alloc_frame_t allocatesFrame(Address addr);
   ~LookupFuncStart();
};

std::map<Dyninst::PID, LookupFuncStart*> LookupFuncStart::all_func_starts;

#if defined(cap_cache_func_starts)
static int hash_address(Address a)
{
   return (int) a;
}
#endif

LookupFuncStart::LookupFuncStart(ProcessState *proc_) :
   FrameFuncHelper(proc_)
#if defined(cap_cache_func_starts)
   , cache(cache_size, hash_address)
#endif
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
  return 0x10020;
}

#if defined(cap_stackwalker_use_symtab)

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
   Symtab *symtab = NULL;
   SymtabAPI::Function *func = NULL;
   unsigned char mem[FUNCTION_PROLOG_TOCHECK];
   Address func_addr;
   unsigned cur;
   int push_ebp_pos = -1, mov_esp_ebp_pos = -1;
   alloc_frame_t res = alloc_frame_t(unknown_t, unknown_s);
   bool result;

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

   symtab = SymtabWrapper::getSymtab(lib.first);
   if (!symtab) {
      sw_printf("[%s:%u] - Error. SymtabAPI couldn't open %s\n",
                __FILE__, __LINE__, lib.first.c_str());
      goto done;
   }

   result = symtab->getNearestFunction(addr - lib.second, func);
   if (!result || !func) {
      sw_printf("[%s:%u] - Error.  Address %lx wasn't part of a function\n",
                __FILE__, __LINE__, addr);
      goto done;
   }
   func_addr = func->getAddress() + lib.second;
   
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

#if defined(cap_cache_func_starts)
void LookupFuncStart::updateCache(Address addr, alloc_frame_t result)
{
   cache.insert(addr, result);
}

bool LookupFuncStart::checkCache(Address addr, alloc_frame_t &result)
{
   return cache.lookup(addr, result);
}

#else
void LookupFuncStart::updateCache(Address /*addr*/, alloc_frame_t /*result*/)
{
   return;
}

bool LookupFuncStart::checkCache(Address /*addr*/, alloc_frame_t &/*result*/)
{
   return false;
}
#endif

gcframe_ret_t DebugStepperImpl::getCallerFrameArch(Address pc, const Frame &in, 
                                                   Frame &out, Symtab *symtab)
{
   MachRegisterVal frame_value, stack_value, ret_value;
   bool result;

   result = symtab->getRegValueAtFrame(pc, MachRegReturn,
                                       ret_value, this);
   if (!result) {
      sw_printf("[%s:%u] - Couldn't get return debug info at %lx\n",
                __FILE__, __LINE__, in.getRA());
      return gcf_not_me;
   }

   unsigned addr_width = getProcessState()->getAddressWidth();
   
   
   Dyninst::MachRegister frame_reg;
   if (addr_width == 4)
      frame_reg = EBP;
   else
      frame_reg = RBP;

   result = symtab->getRegValueAtFrame(pc, frame_reg,
                                       frame_value, this);
   if (!result) {
      sw_printf("[%s:%u] - Couldn't get frame debug info at %lx\n",
                 __FILE__, __LINE__, in.getRA());
      return gcf_not_me;
   }

   result = symtab->getRegValueAtFrame(pc, MachRegFrameBase,
                                       stack_value, this);
   if (!result) {
      sw_printf("[%s:%u] - Couldn't get stack debug info at %lx\n",
                 __FILE__, __LINE__, in.getRA());
      return gcf_not_me;
   }

   out.setRA(ret_value);
   out.setFP(frame_value);
   out.setSP(stack_value);

   return gcf_success;
}
#else

FrameFuncHelper::alloc_frame_t LookupFuncStart::allocatesFrame(Address /*addr*/)
{
   return FrameFuncHelper::alloc_frame_t(unknown_t, unknown_s);
}
#endif

gcframe_ret_t UninitFrameStepperImpl::getCallerFrame(const Frame &in, Frame &out)
{
   bool result;
   int offset;
   if (!alloc_frame)
      return gcf_not_me;
   FrameFuncHelper::alloc_frame_t frame_state = alloc_frame->allocatesFrame(in.getRA());

   if (frame_state.second != FrameFuncHelper::unset_frame && 
       frame_state.second != FrameFuncHelper::halfset_frame)
      return gcf_not_me;

   if (frame_state.second == FrameFuncHelper::unset_frame)
      offset = getProcessState()->getAddressWidth();
   else //frame_state == FrameFuncHelper::halfset_frame
      offset = getProcessState()->getAddressWidth()*2;
   out.setSP(in.getSP() + offset);
      
   Address pc;
   location_t pc_loc;
   
   pc_loc.location = loc_address;
   pc_loc.val.addr = offset + in.getSP();
   result = getProcessState()->readMem(&pc, pc_loc.val.addr, 
                                       getProcessState()->getAddressWidth());
   if (!result) {
      sw_printf("[%s:%u] - Error reading from stack at address %x\n",
                __FILE__, __LINE__, pc_loc.val.addr);
      return gcf_error;
   }
   out.setRALocation(pc_loc);
   out.setRA(pc);

   location_t fp_loc;
   fp_loc.location = loc_register;
   if (getProcessState()->getAddressWidth() == 4)
      fp_loc.val.reg = EBP;
   else 
      fp_loc.val.reg = RBP;
   out.setFP(in.getFP());
   out.setFPLocation(fp_loc);

   out.setSP(pc_loc.val.addr + getProcessState()->getAddressWidth());

   return gcf_success;
}

UninitFrameStepperImpl::UninitFrameStepperImpl(Walker *w, 
                                               FrameFuncHelper *f,
                                               FrameStepper *parent_) :
   FrameStepper(w),
   parent(parent_)
{
   alloc_frame = f ? f : LookupFuncStart::getLookupFuncStart(w->getProcessState());
}

UninitFrameStepperImpl::~UninitFrameStepperImpl()
{
   LookupFuncStart *lookup = dynamic_cast<LookupFuncStart*>(alloc_frame);
   if (lookup)
      lookup->releaseMe();
   else if (alloc_frame)
      delete alloc_frame;
}

UninitFrameStepper::UninitFrameStepper(Walker *w, FrameFuncHelper *f) :
   FrameStepper(w)
{
   impl = new UninitFrameStepperImpl(w, f, this);
}

gcframe_ret_t UninitFrameStepper::getCallerFrame(const Frame &in, Frame &out)
{
   if (impl)
      return impl->getCallerFrame(in, out);
   sw_printf("[%s:%u] - Error,  UninitFrame used on unsupported platform\n",
             __FILE__, __LINE__);
   setLastError(err_unsupported, "Uninitialized frame walking not supported " 
                "on this platform");
   return gcf_error;
}

unsigned UninitFrameStepper::getPriority() const
{
   if (impl)
      return impl->getPriority();
   sw_printf("[%s:%u] - Error,  UninitFrame used on unsupported platform\n",
             __FILE__, __LINE__);
   setLastError(err_unsupported, "Uninitialized frame walking not supported " 
                "on this platform");
   return 0;
}

void UninitFrameStepper::registerStepperGroup(StepperGroup *group)
{
   if (impl) {
      impl->registerStepperGroup(group);
      return;
   }
   sw_printf("[%s:%u] - Error,  UninitFrame used on unsupported platform\n",
             __FILE__, __LINE__);
   setLastError(err_unsupported, "Uninitialized frame walking not supported " 
                "on this platform");
}

UninitFrameStepper::~UninitFrameStepper()
{
   if (impl)
      delete impl;
   impl = NULL;
}

unsigned UninitFrameStepperImpl::getPriority() const
{
   return 0x10950;
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

