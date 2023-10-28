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
#include "registers/ppc64_regs.h"
#include "registers/ppc32_regs.h"
#include "stackwalk/src/sw.h"

#include "get_trap_instruction.h"
using namespace Dyninst;
using namespace Dyninst::Stackwalker;

#if defined(os_linux)

#define GET_FRAME_BASE(spr) __asm__("or %0, %%r1, %%r1\n" : "=r"(spr))
typedef union {
   struct {
      uint32_t out_fp;
      uint32_t out_ra;
   } pair32;
   struct {
      uint64_t out_fp;
      uint64_t unused_cr;
      uint64_t out_ra;
   } pair64;
} ra_fp_pair_t;

#else

#error Unknown platform

#endif

bool ProcSelf::getRegValue(Dyninst::MachRegister reg, THR_ID, Dyninst::MachRegisterVal &val)
{
  ra_fp_pair_t **sp;
  ra_fp_pair_t *fp_ra;

  GET_FRAME_BASE(sp);

  bool found_reg = false;
  if (reg.isStackPointer()) {
    val = (Dyninst::MachRegisterVal) sp;
    found_reg = true;
  }

  fp_ra = *sp;
  if (reg.isFramePointer()) {
     val = (Dyninst::MachRegisterVal) fp_ra;
     found_reg = true;
  }

  if (reg.isPC() || reg == Dyninst::ReturnAddr) {
     if (getAddressWidth() == sizeof(uint64_t)) {
       val = fp_ra->pair64.out_ra;
     }
     else {
       val = fp_ra->pair32.out_ra;
     }
     found_reg = true;
  }

  if (found_reg)  {
     sw_printf("[%s:%d] - Returning value %lx for reg %s\n", 
            FILE__, __LINE__, val, reg.name().c_str());
  }  else  {
     sw_printf("[%s:%d] - Register not found\n", 
            FILE__, __LINE__);
  }

  return found_reg;
}

Dyninst::Architecture ProcSelf::getArchitecture()
{
   return Arch_ppc32;
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
}

gcframe_ret_t FrameFuncStepperImpl::getCallerFrame(const Frame &in, Frame &out)
{
  // TODO set RA location

  Address in_fp, out_sp, out_ra;
  bool result;

  ra_fp_pair_t this_frame_pair;
  ra_fp_pair_t last_frame_pair;
  ra_fp_pair_t *actual_frame_pair_p;

  unsigned addrWidth;

  addrWidth = getProcessState()->getAddressWidth();

  // Assume a standard frame layout if no analysis is available
  FrameFuncHelper::alloc_frame_t alloc_frame =  make_pair(FrameFuncHelper::standard_frame,
                                                          FrameFuncHelper::set_frame);

  if (helper && in.isTopFrame())
  {
    alloc_frame = helper->allocatesFrame(in.getRA());
    sw_printf("[%s:%d] - FrameFuncHelper for 0x%lx reports %d, %d\n", FILE__, __LINE__,
              in.getRA(), alloc_frame.first, alloc_frame.second);
  }

  if (!in.getFP())
    return gcf_stackbottom;

  in_fp = in.getFP();

  out_sp = in_fp;
  out.setSP(out_sp);
  
  // Read the current frame
  if (sizeof(uint64_t) == addrWidth) {
     result = getProcessState()->readMem(&this_frame_pair.pair64, in_fp,
                                         sizeof(this_frame_pair.pair64));
  }
  else {
     result = getProcessState()->readMem(&this_frame_pair.pair32, in_fp, 
                                         sizeof(this_frame_pair.pair32));
  }
  if (!result) {
    sw_printf("[%s:%d] - Couldn't read from %lx\n", FILE__, __LINE__, in_fp);
    return gcf_error;
  }

  // Read the previous frame
  if (sizeof(uint64_t) == addrWidth) {
    result = getProcessState()->readMem(&last_frame_pair.pair64, this_frame_pair.pair64.out_fp, 
                                        sizeof(last_frame_pair.pair64));
  }
  else {
    result = getProcessState()->readMem(&last_frame_pair.pair32, this_frame_pair.pair32.out_fp, 
                                        sizeof(last_frame_pair.pair32));
  }
  if (!result) {
    sw_printf("[%s:%d] - Couldn't read from %lx\n", FILE__, __LINE__,
	      out.getFP());
    return gcf_error;
  }

  // Set actual stack frame based on
  // whether the function creates a frame or not
  if (FrameFuncHelper::no_frame == alloc_frame.first)
  {
    actual_frame_pair_p = &this_frame_pair;    
  }
  else
  {
    actual_frame_pair_p = &last_frame_pair;
  }

  // Handle leaf functions
  if (FrameFuncHelper::unset_frame == alloc_frame.second)
  {
    // Leaf function - does not save return address
    // Get the RA from the PC register
    if (sizeof(uint64_t) == addrWidth)
    {
      result = getProcessState()->getRegValue(ppc64::lr, in.getThread(), out_ra);
    }
    else
    {
      result = getProcessState()->getRegValue(ppc32::lr, in.getThread(), out_ra);
    }
    if (!result) {
        sw_printf("[%s:%d] - Error getting PC value for thrd %d\n",
                  FILE__, __LINE__, (int) in.getThread());
        return gcf_error;
    }
  }
  else
  {
    // Function saves return address
    if (sizeof(uint64_t) == addrWidth)
    {
      out_ra = actual_frame_pair_p->pair64.out_ra;
    }
    else {
      out_ra = actual_frame_pair_p->pair32.out_ra;
    }
  }

  // Set new frame pointer
  if (FrameFuncHelper::no_frame == alloc_frame.first)
  {
    // frame pointer stays the same
    out.setFP(in_fp);
  }
  else
  {
    if (sizeof(uint64_t) == addrWidth) {
      out.setFP(this_frame_pair.pair64.out_fp); 
    }
    else {
      out.setFP(this_frame_pair.pair32.out_fp);
    }
  }

  if (!out_ra) {
    return gcf_stackbottom;
  }

  out.setRA(out_ra);

  return gcf_success;
}
 
unsigned FrameFuncStepperImpl::getPriority() const
{
   return frame_priority;
}

FrameFuncStepperImpl::~FrameFuncStepperImpl()
{
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
                                                            Address /*orig_ra*/, bool /*pEntryExit*/)
{
  bool result;
  Address in_fp, out_ra;
  ra_fp_pair_t ra_fp_pair;
  location_t raLocation;
  unsigned addrWidth;

  addrWidth = getProcessState()->getAddressWidth();

  if (!in.getFP())
    return gcf_stackbottom;

  in_fp = in.getFP();
  out.setSP(in_fp);
  
  if (sizeof(uint64_t) == addrWidth) {
    result = getProcessState()->readMem(&ra_fp_pair.pair64, in_fp, 
                                        sizeof(ra_fp_pair.pair64));
  }
  else {
    result = getProcessState()->readMem(&ra_fp_pair.pair32, in_fp, 
                                        sizeof(ra_fp_pair.pair32));
  }
  if (!result) {
    sw_printf("[%s:%d] - Couldn't read frame from %lx\n", FILE__, __LINE__, in_fp);
    return gcf_error;
  }
  if (sizeof(uint64_t) == addrWidth) {
    out.setFP(ra_fp_pair.pair64.out_fp);
  }
  else {
    out.setFP(ra_fp_pair.pair32.out_fp);
  }
  
  raLocation.location = loc_address;
  raLocation.val.addr = in_fp + stack_height; // stack_height is the offset to the saved RA
  out.setRALocation(raLocation);
  
  // TODO make 32-bit compatible
  result = getProcessState()->readMem(&out_ra, raLocation.val.addr, 
                                      sizeof(out_ra));
  if (!result) {
    sw_printf("[%s:%d] - Couldn't read instrumentation RA from %lx\n", FILE__, __LINE__, raLocation.val.addr);
    return gcf_error;
  }
  out.setRA(out_ra);

  return gcf_success;
}

namespace Dyninst {
  namespace Stackwalker {

    void getTrapInstruction(char *buffer, unsigned buf_size, 
                            unsigned &actual_len, bool include_return)
    {
      assert(buf_size >= 4);
      buffer[0] = 0x7d;
      buffer[1] = 0x82;
      buffer[2] = 0x10;
      buffer[3] = 0x08;
      actual_len = 4;
      if (include_return)
      {   
        assert(buf_size >= 8);
        buffer[4] = 0x4e;
        buffer[5] = 0x80;
        buffer[6] = 0x00;
        buffer[7] = 0x20;
        actual_len = 8;
        return;
      }
  
      assert(buf_size >= 1);
      actual_len = 1;
      return;
    }
  }
}

