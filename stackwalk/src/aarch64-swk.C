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

#include "stackwalk/src/sw.h"

#include "get_trap_instruction.h"

using namespace Dyninst;
using namespace Dyninst::Stackwalker;

#if defined(os_linux) || defined(os_bg)

//get fp value
#define GET_FRAME_BASE(spr)     __asm__("mov x0, x29\n" : "=r"(spr))
#define GET_RET_ADDR(spr)       __asm__("mov x0, x30\n" : "=r"(spr))
#define GET_STACK_POINTER(spr)  __asm__("mov x0, sp\n" : "=r"(spr))

#else

#error Unknown platform

#endif

typedef struct{
    uint64_t FP;
    uint64_t LR;
} ra_fp_pair_t;

bool ProcSelf::getRegValue(Dyninst::MachRegister reg, THR_ID, Dyninst::MachRegisterVal &val)
{
  uint64_t *sp;
  ra_fp_pair_t *framePointer;
  uint64_t *retAddr;

  bool found_reg = false;
  GET_FRAME_BASE(framePointer);

  if (reg.isStackPointer()) {
    GET_STACK_POINTER(sp);
    val = (Dyninst::MachRegisterVal) sp;
    found_reg = false;
  }

  if (reg.isFramePointer()) {
     //GET_FRAME_BASE(framePointer);
     val = (Dyninst::MachRegisterVal) framePointer->FP;
     found_reg = true;
  }

  if (reg.isPC() || reg == Dyninst::ReturnAddr) {
     if (getAddressWidth() == sizeof(uint64_t)) {
        val = (Dyninst::MachRegisterVal) framePointer->LR;
     }
     else {
         assert(0);
     }
     found_reg = true;
  }

  sw_printf("[%s:%u] - Returning value %lx for reg %s\n",
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
}

gcframe_ret_t FrameFuncStepperImpl::getCallerFrame(const Frame &in, Frame &out)
{
  // TODO set RA location

  Address in_fp, out_sp, out_ra;
  bool result;

  unsigned addrWidth;
  addrWidth = getProcessState()->getAddressWidth();

  ra_fp_pair_t this_frame_pair;
  ra_fp_pair_t last_frame_pair;
  ra_fp_pair_t *actual_frame_pair_p;

  // Assume a standard frame layout if no analysis is available
  FrameFuncHelper::alloc_frame_t alloc_frame =  make_pair(FrameFuncHelper::standard_frame,
                                                          FrameFuncHelper::set_frame);

  if (helper && in.isTopFrame())
  {
    alloc_frame = helper->allocatesFrame(in.getRA());
    sw_printf("[%s:%u] - FrameFuncHelper for 0x%lx reports %d, %d\n", FILE__, __LINE__,
              in.getRA(), alloc_frame.first, alloc_frame.second);
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
    sw_printf("[%s:%u] - Couldn't read from %lx\n", FILE__, __LINE__, in_fp);
    return gcf_error;
  }

  // Read the previous frame
  if (sizeof(uint64_t) == addrWidth) {
    result = getProcessState()->readMem(&last_frame_pair, this_frame_pair.FP,
                                        sizeof(last_frame_pair));
  }
  else {
      assert(0);
  }
  if (!result) {
    sw_printf("[%s:%u] - Couldn't read from %lx\n", FILE__, __LINE__,
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
      result = getProcessState()->getRegValue(aarch64::x30, in.getThread(), out_ra);
    }
    else
    {
		//aarch32 is not supported now
		assert(0);
        //result = getProcessState()->getRegValue(aarch32::lr, in.getThread(), out_ra);
    }
    if (!result) {
        sw_printf("[%s:%u] - Error getting PC value for thrd %d\n",
                  FILE__, __LINE__, (int) in.getThread());
        return gcf_error;
    }
  }
  else
  {
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
    out.setFP(in_fp);
  }
  else
  {
    if (sizeof(uint64_t) == addrWidth) {
        out.setFP(this_frame_pair.FP);
    }
    else {
		//aarch32 is not supported now
	    assert(0);
        //out.setFP(this_frame_pair.pair32.out_fp);
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
   sw_printf("[%s:%u] - Unimplemented on this platform!\n");
   assert(0);
   return false;
}

WandererHelper::pc_state WandererHelper::isPCInFunc(Address, Address)
{
   sw_printf("[%s:%u] - Unimplemented on this platform!\n");
   assert(0);
   return unknown_s;
}

bool WandererHelper::requireExactMatch()
{
   sw_printf("[%s:%u] - Unimplemented on this platform!\n");
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
                                                            Address /*orig_ra*/, bool pEntryExit)
{
  bool result;
  Address in_fp, out_ra;
  ra_fp_pair_t ra_fp_pair;
  //uint64_t fp;
  location_t raLocation;
  unsigned addrWidth;

  addrWidth = getProcessState()->getAddressWidth();

  if (!in.getFP())
    return gcf_stackbottom;

  in_fp = in.getFP();
  out.setSP(in_fp);

  if (sizeof(uint64_t) == addrWidth) {
    result = getProcessState()->readMem(&ra_fp_pair, in_fp,
                                        sizeof(ra_fp_pair));
  }
  else {
		//aarch32 is not supported now
		assert(0);
  }
  if (!result) {
    sw_printf("[%s:%u] - Couldn't read frame from %lx\n", FILE__, __LINE__, in_fp);
    return gcf_error;
  }

  if (sizeof(uint64_t) == addrWidth) {
    out.setFP(ra_fp_pair.FP);
  }
  else {
      assert(0);
  }

  raLocation.location = loc_address;
  raLocation.val.addr = in_fp + stack_height; // stack_height is the offset to the saved RA
  out.setRALocation(raLocation);

  // TODO make 32-bit compatible
  result = getProcessState()->readMem(&out_ra, raLocation.val.addr,
                                      sizeof(out_ra));
  if (!result) {
    sw_printf("[%s:%u] - Couldn't read instrumentation RA from %lx\n", FILE__, __LINE__, raLocation.val.addr);
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

