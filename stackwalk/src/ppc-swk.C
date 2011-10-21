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

#if defined(os_linux) || defined(os_bg) && !defined(arch_64bit)

#define GET_FRAME_BASE(spr) __asm__("or %0, %%r1, %%r1\n" : "=r"(spr))
typedef struct {
   Address out_fp;
   Address out_ra;
} ra_fp_pair_t;

#elif defined(os_aix)

#define GET_FRAME_BASE(spr) __asm__("or %0, 1, 1\n" : "=r"(spr))
typedef struct {
   Address out_fp;
   Address unused;
   Address out_ra;
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
     val = fp_ra->out_ra;
     found_reg = true;
  }

  sw_printf("[%s:%u] - Returning value %lx for reg %s\n", 
            __FILE__, __LINE__, val, reg.name().c_str());
  return true;
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
                                           FrameFuncHelper *) :
   FrameStepper(w),
   parent(parent_),
   helper(NULL)
{
}

gcframe_ret_t FrameFuncStepperImpl::getCallerFrame(const Frame &in, Frame &out)
{
  Address in_fp, out_sp;
  bool result;
  ra_fp_pair_t ra_fp_pair;

  if (!in.getFP())
    return gcf_stackbottom;

  in_fp = in.getFP();

  out_sp = in_fp;
  out.setSP(out_sp);
  
  result = getProcessState()->readMem(&ra_fp_pair, in_fp, 
                                      sizeof(ra_fp_pair_t));
  if (!result) {
    sw_printf("[%s:%u] - Couldn't read from %lx\n", __FILE__, __LINE__, in_fp);
    return gcf_error;
  }
  out.setFP(ra_fp_pair.out_fp);

  result = getProcessState()->readMem(&ra_fp_pair, ra_fp_pair.out_fp, 
                                      sizeof(ra_fp_pair_t));
  if (!result) {
    sw_printf("[%s:%u] - Couldn't read from %lx\n", __FILE__, __LINE__,
	      ra_fp_pair.out_fp);
    return gcf_error;
  }
  if (!ra_fp_pair.out_ra) {
    return gcf_stackbottom;
  }

  out.setRA(ra_fp_pair.out_ra);

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

bool WandererHelper::isPCInFunc(Address, Address)
{
   sw_printf("[%s:%u] - Unimplemented on this platform!\n");
   assert(0);
   return false;
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

