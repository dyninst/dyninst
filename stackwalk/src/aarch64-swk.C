/*
 * See the dyninst/COPYRIGHT file for copyright information.
 *
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
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

#include "stackwalk/h/swk_errors.h"
#include "stackwalk/h/procstate.h"
#include "stackwalk/h/framestepper.h"
#include "stackwalk/h/basetypes.h"
#include "stackwalk/h/frame.h"
#include "stackwalk/h/walker.h"
#include "registers/abstract_regs.h"
#include "registers/aarch64_regs.h"
#include "stackwalk/src/sw.h"

#include "get_trap_instruction.h"

#include "stackwalk/src/aarch64-swk.h"

using namespace Dyninst;
using namespace Dyninst::Stackwalker;

#if defined(os_linux)
#define GET_FRAME_POINTER(spr)     __asm__("mov %0, x29;" : "=r"(spr))
#define GET_RET_ADDR(spr)       __asm__("mov %0, x30;" : "=r"(spr))
#define GET_STACK_POINTER(spr)  __asm__("mov %0, sp;"  : "=r"(spr))
#else
#error Unknown platform
#endif

struct ra_fp_pair_t  {
    unsigned long FP;
    unsigned long LR;
};

bool ProcSelf::getRegValue(Dyninst::MachRegister reg, THR_ID, Dyninst::MachRegisterVal &val)
{
  ra_fp_pair_t *framePointer;
  ra_fp_pair_t thisFramePair;
  ra_fp_pair_t stackWalkFramePair;

  bool found_reg = false;

  ra_fp_pair_t * sp;
  GET_STACK_POINTER(sp);

  framePointer = (ra_fp_pair_t *) sp;
  if(!framePointer) return false;
  if(!framePointer->FP) return false;
  thisFramePair = *framePointer;
  stackWalkFramePair = *( (ra_fp_pair_t*) (thisFramePair.FP));

  if (reg.isStackPointer() || reg == Dyninst::StackTop) {
    val = (Dyninst::MachRegisterVal) ((ra_fp_pair_t*)framePointer->FP)->FP;
    if(val != 0) found_reg = true;
  }

  if (reg.isFramePointer()) {
     val = (Dyninst::MachRegisterVal) ((ra_fp_pair_t*)framePointer->FP)->FP;
     if( val != 0) found_reg = true;
  }

  sw_printf("ARM_DEBUG: ra %p\n", (void *) stackWalkFramePair.LR);

  if (reg.isPC() || reg == Dyninst::ReturnAddr) {
     val = (Dyninst::MachRegisterVal) stackWalkFramePair.LR;
     if( val != 0) found_reg = true;
  }

  sw_printf("[%s:%d] - Returning value %lx for reg %s\n",
            FILE__, __LINE__, val, reg.name().c_str());
  return found_reg;
}

Dyninst::Architecture ProcSelf::getArchitecture()
{
   return Arch_aarch64;
}

bool Walker::checkValidFrame(const Frame & /*in*/, const Frame & /*out*/)
{
   return true;
}

FrameFuncStepperImpl::FrameFuncStepperImpl(Walker *w, FrameStepper *parent_,
                                           FrameFuncHelper *helper_) :
   FrameStepper(w),
   parent(parent_),
   helper(helper_)
{
   helper = helper_ ? helper_ : aarch64_LookupFuncStart::getLookupFuncStart(getProcessState());
}

gcframe_ret_t FrameFuncStepperImpl::getCallerFrame(const Frame &in, Frame &out)
{
  // TODO set RA location

  Address in_fp, out_sp, out_ra;
  location_t fp_loc, sp_loc, ra_loc;

  bool result;

  unsigned addrWidth;
  addrWidth = getProcessState()->getAddressWidth();

  ra_fp_pair_t this_frame_pair;
  ra_fp_pair_t *actual_frame_pair_p;

  // FrameFuncStepper needs an input FP
  if (!in.getFP())
  {
      sw_printf("[%s:%d] - in.getFP() %lx\n", FILE__, __LINE__, in.getFP());
      return gcf_not_me;
  }
  // Look for function prologue to see if it is a standard frame 
  FrameFuncHelper::alloc_frame_t alloc_frame;
  // Here we lookup return address minus 1 to handle the following special case:
  //
  // Suppose A calls B and B is a non-returnning function, and the call to B in A
  // is the last instruction of A. Then the compiler may generate code where 
  // another function C is immediately after A. In such case, the return address
  // will be the entry address of C. And if we look up frame type by the return
  // address, we will get information for C rather than A. 
  alloc_frame = helper->allocatesFrame(in.getRA() - 1);
  if (alloc_frame.first != FrameFuncHelper::standard_frame) {
      sw_printf("[%s:%d] - alloc_frame.first!=standard_frame (== %x)\n", FILE__, __LINE__, (unsigned int)alloc_frame.first);
      // If we are dealing with the first frame,
      // the frame information is in the register state.
      // We continue this function 
      if (in.getPrevFrame() != NULL)
          return gcf_not_me;
  }

  if (!in.getFP())
    return gcf_stackbottom;

  in_fp = in.getFP();

  out_sp = in_fp;
  out.setSP(out_sp);

  // Read the current frame
  if (sizeof(uint64_t) == addrWidth) {
     result = getProcessState()->readMem(&this_frame_pair, in_fp,
                                         sizeof(this_frame_pair));
  }
  else {
      assert(0);
  }

  if (!result) {
    sw_printf("[%s:%d] - Couldn't read from %lx\n", FILE__, __LINE__, in_fp);
    return gcf_error;
  }


  // Set actual stack frame
  actual_frame_pair_p = &this_frame_pair;

  // Handle leaf functions
  if (FrameFuncHelper::unset_frame == alloc_frame.second)
  {
    ra_loc.location = loc_register;
    ra_loc.val.reg = aarch64::x30;

    // Leaf function - does not save return address
    // Get the RA from the PC register
    if (sizeof(uint64_t) == addrWidth)
    {
      result = getProcessState()->getRegValue(aarch64::x30, in.getThread(), out_ra);
    }
    else
    {
		//aarch32 is not supported now
		assert(0);
        //result = getProcessState()->getRegValue(aarch32::lr, in.getThread(), out_ra);
    }
    if (!result) {
        sw_printf("[%s:%d] - Error getting PC value for thrd %d\n",
                  FILE__, __LINE__, (int) in.getThread());
        return gcf_error;
    }
  }
  else
  {
    // not leaf functions on ARM
    ra_loc.location = loc_address;
    ra_loc.val.addr = in_fp + sizeof(uint64_t); //&(actual_frame_pair_p->LR)

    // Function saves return address
    if (sizeof(uint64_t) == addrWidth)
    {
        out_ra = actual_frame_pair_p->LR;
    }
    else {
		//aarch32 is not supported now
		assert(0);
    }
  }

  // Set new frame pointer
  if (FrameFuncHelper::no_frame == alloc_frame.first)
  {
    // frame pointer stays the same
    fp_loc = in.getFPLocation();
    out.setFP(in_fp);
  }
  else
  {
    fp_loc.location = loc_address;
    fp_loc.val.addr = in_fp;

    if (sizeof(uint64_t) == addrWidth) {
        out.setFP(this_frame_pair.FP);
    }
    else {
	    assert(0);
    }
  }

  //on ARM, the ra of the bottom stack is not NULL
  //the following code dosen't work
  if (!out_ra) {
    return gcf_stackbottom;
  }

  out.setRA(out_ra);

  //try to set loc
  //sp_loc.location = loc_address;
  out.setRALocation( ra_loc);
  out.setFPLocation( fp_loc);
  out.setSPLocation( in.getFPLocation() );

  return gcf_success;
}

unsigned FrameFuncStepperImpl::getPriority() const
{
   return frame_priority;
}

FrameFuncStepperImpl::~FrameFuncStepperImpl()
{
   aarch64_LookupFuncStart *lookup = dynamic_cast<aarch64_LookupFuncStart*>(helper);
   if (lookup)
      lookup->releaseMe();
   else if (helper)
      delete helper;
}

WandererHelper::WandererHelper(ProcessState *proc_) :
   proc(proc_)
{
}

bool WandererHelper::isPrevInstrACall(Address, Address&)
{
   sw_printf("[%s:%d] - Unimplemented on this platform!\n", FILE__, __LINE__);
   assert(0);
   return false;
}

WandererHelper::pc_state WandererHelper::isPCInFunc(Address, Address)
{
   sw_printf("[%s:%d] - Unimplemented on this platform!\n", FILE__, __LINE__);
   assert(0);
   return unknown_s;
}

bool WandererHelper::requireExactMatch()
{
   sw_printf("[%s:%d] - Unimplemented on this platform!\n", FILE__, __LINE__);
   assert(0);
   return true;
}

WandererHelper::~WandererHelper()
{
}

gcframe_ret_t DyninstInstrStepperImpl::getCallerFrameArch(const Frame &/*in*/, Frame &/*out*/,
                                                          Address /*base*/, Address /*lib_base*/,
                                                          unsigned /*size*/, unsigned /*stack_height*/)
{
  return gcf_not_me;
}

gcframe_ret_t DyninstDynamicStepperImpl::getCallerFrameArch(const Frame &in, Frame &out,
                                                            Address /*base*/, Address /*lib_base*/,
                                                            unsigned /*size*/, unsigned stack_height,
                                                            bool /* aligned */,
                                                            Address /*orig_ra*/, bool)
{
  bool result;
  Address in_fp, out_fp_loc;
  ra_fp_pair_t ra_fp_pair;
  //uint64_t fp;
  location_t raLocation;
  unsigned addrWidth;

  addrWidth = getProcessState()->getAddressWidth();

  if (!in.getFP())
    return gcf_stackbottom;

  // Dyninst's instrumentation frame set up makes sure that
  // current SP is the same as the FP retrived in the previous frame
  // 
  // Note: If the implementation of baseTramp::generateSaves changed
  // in dyninstAPI/src/inst-aarch64.C. This function may need to
  // be changed accordingly.
  in_fp = in.getFP();
  out.setSP(in_fp);
  
  // stack_height is the offset to the saved FP and RA
  out_fp_loc = in_fp + stack_height; 

  raLocation.location = loc_address;
  raLocation.val.addr = out_fp_loc + addrWidth;
  out.setRALocation(raLocation);

  result = getProcessState()->readMem(&ra_fp_pair, out_fp_loc,
                                      sizeof(ra_fp_pair));
  if (!result) {
    sw_printf("[%s:%d] - Couldn't read instrumentation FP and RA from %lx\n", FILE__, __LINE__, out_fp_loc);
    return gcf_error;
  }
  out.setRA(ra_fp_pair.LR);
  out.setFP(ra_fp_pair.FP);

  return gcf_success;
}

std::map<Dyninst::PID, aarch64_LookupFuncStart*> aarch64_LookupFuncStart::all_func_starts;

static int hash_address(Address a)
{
   return (int) a;
}

// to handle leaf functions
aarch64_LookupFuncStart::aarch64_LookupFuncStart(ProcessState *proc_) :
   FrameFuncHelper(proc_),
   cache(cache_size, hash_address)
{
   all_func_starts[proc->getProcessId()] = this;
   ref_count = 1;
}

aarch64_LookupFuncStart::~aarch64_LookupFuncStart()
{
   Dyninst::PID pid = proc->getProcessId();
   all_func_starts.erase(pid);
}

aarch64_LookupFuncStart *aarch64_LookupFuncStart::getLookupFuncStart(ProcessState *p)
{
   Dyninst::PID pid = p->getProcessId();
   std::map<Dyninst::PID, aarch64_LookupFuncStart*>::iterator i = all_func_starts.find(pid);
   if (i == all_func_starts.end()) {
      return new aarch64_LookupFuncStart(p);
   }
   (*i).second->ref_count++;
   return (*i).second;
}

void aarch64_LookupFuncStart::releaseMe()
{
   ref_count--;
   if (!ref_count)
      delete this;
}


// WARNING: this is not safe as the function prolog can be 
// interleaved with arbitrary number of other instructions...
// A better solution is use ParseAPI to examine the first block, 
// but this will involve apply parseAPI to the mutatee...
// in bytes
#define FUNCTION_PROLOG_TOCHECK 20
static const unsigned int push_fp_ra      = 0xa9807bfd ; // stp x29, x30, [sp, #x]!
static const unsigned int mov_sp_fp       = 0x910003fd ; // mov x29(fp), sp
static const unsigned int push_fp_ra_mask = 0xffc07fff ; // mask for push_fp_ra

FrameFuncHelper::alloc_frame_t aarch64_LookupFuncStart::allocatesFrame(Address addr)
{
   LibAddrPair lib;
   unsigned int mem[FUNCTION_PROLOG_TOCHECK/4];
   Address func_addr;
   unsigned cur;
   int push_fp_ra_pos = -1, mov_sp_fp_pos = -1;
   alloc_frame_t res = alloc_frame_t(unknown_t, unknown_s);
   bool result;
   SymReader *reader;
   Offset off;
   Symbol_t sym;

   result = checkCache(addr, res);
   if (result) {
      sw_printf("[%s:%d] - Cached value for %lx is %d/%d\n",
                FILE__, __LINE__, addr, (int) res.first, (int) res.second);
      return res;
   }

   result = proc->getLibraryTracker()->getLibraryAtAddr(addr, lib);
   if (!result)
   {
      sw_printf("[%s:%d] - No library at %lx\n", FILE__, __LINE__, addr);
      goto done;
   }

   reader = LibraryWrapper::getLibrary(lib.first);
   if (!reader) {
      sw_printf("[%s:%d] - Failed to open symbol reader %s\n",
                FILE__, __LINE__, lib.first.c_str() );
      goto done;
   }
   off = addr - lib.second;
   sym = reader->getContainingSymbol(off);
   if (!reader->isValidSymbol(sym)) {
      sw_printf("[%s:%d] - Could not find symbol in binary\n", FILE__, __LINE__);
      goto done;
   }
   func_addr = reader->getSymbolOffset(sym) + lib.second;

   result = proc->readMem(mem, func_addr, FUNCTION_PROLOG_TOCHECK);
   if (!result) {
      sw_printf("[%s:%d] - Error.  Couldn't read from memory at %lx\n",
                FILE__, __LINE__, func_addr);
      goto done;
   }

   //Try to find a 'push (r|e)bp'
   for (cur=0; cur<FUNCTION_PROLOG_TOCHECK/4; cur++)
   {
      if ( (mem[cur]&push_fp_ra_mask) == push_fp_ra )
      {
         push_fp_ra_pos = cur*4;
         break;
      }
   }

   //Try to find the mov esp->ebp
   for (cur = cur+1; cur<FUNCTION_PROLOG_TOCHECK/4; cur++)
   {
       if(mem[cur] == mov_sp_fp){
            mov_sp_fp_pos = cur*4;
            break;
       }
   }

   if ((push_fp_ra_pos != -1) && (mov_sp_fp_pos != -1))
      res.first = standard_frame;
   else if ((push_fp_ra_pos != -1) && (mov_sp_fp_pos == -1))
      res.first = savefp_only_frame;
   else
      res.first = no_frame;

   // if the addr is earlier than the frame building instructions
   // mark it as unset_frame.
   // on ARMv8, if the target process is not broken by BP,
   // the frame type should always be set_frame, since the current
   // PC should pointed to the function body, but never to the prologe.
   if ((push_fp_ra_pos != -1) && (addr <= func_addr + push_fp_ra_pos))
      res.second = unset_frame;
   else if ((mov_sp_fp_pos != -1) && (addr <= func_addr + mov_sp_fp_pos))
      res.second = halfset_frame;
   else
      res.second = set_frame;

 done:
   sw_printf("[%s:%d] - Function containing %lx has frame type %d/%d\n",
             FILE__, __LINE__, addr, (int) res.first, (int) res.second);
   updateCache(addr, res);
   return res;
}

void aarch64_LookupFuncStart::updateCache(Address addr, alloc_frame_t result)
{
   cache.insert(addr, result);
}

bool aarch64_LookupFuncStart::checkCache(Address addr, alloc_frame_t &result)
{
   return cache.lookup(addr, result);
}

namespace Dyninst {
  namespace Stackwalker {

    void getTrapInstruction(char *buffer, unsigned buf_size,
                            unsigned &actual_len, bool include_return)
    {
        //trap
        //ret
      assert(buf_size >= 4);
      buffer[0] = 0x00;
      buffer[1] = 0x00;
      buffer[2] = 0x20;
      buffer[3] = 0xd4;
      actual_len = 4;
      if (include_return)
      {
        assert(buf_size >= 8);
        buffer[4] = 0xc0;
        buffer[5] = 0x03;
        buffer[6] = 0x5f;
        buffer[7] = 0xd6;
        actual_len = 8;
        return;
      }

      assert(buf_size >= 1);
      actual_len = 1;
      return;
    }
  }
}

