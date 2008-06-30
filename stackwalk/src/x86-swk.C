/*
 * Copyright (c) 1996-2007 Barton P. Miller
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

using namespace Dyninst;
using namespace Dyninst::Stackwalker;

bool ProcSelf::getRegValue(reg_t reg, THR_ID, regval_t &val)
{
  unsigned long *frame_pointer;

#if defined(arch_x86_64)
  __asm__("mov %%rbp, %0\n"
	  : "=r"(frame_pointer));
#else
  __asm__("movl %%ebp, %0\n"
	  : "=r"(frame_pointer));
#endif

  frame_pointer = (unsigned long *) *frame_pointer;
  
  switch(reg)
  {
    case REG_PC:
      val = (regval_t) frame_pointer[1];
      break;
    case REG_FP:
      val = (regval_t) frame_pointer[0];
      break;      
    case REG_SP:
      val = (regval_t) (frame_pointer - 1);
      break;      
    default:
       sw_printf("[%s:%u] - Request for unsupported register %d\n",
                 __FILE__, __LINE__, reg);
       setLastError(err_badparam, "Unknown register passed in reg field");
  }

  return true;
}

FrameFuncStepper::FrameFuncStepper(Walker *w) :
  FrameStepper(w)
{
}

gcframe_ret_t FrameFuncStepper::getCallerFrame(const Frame &in, Frame &out)
{
  Address in_fp, out_sp;
  bool result;
  unsigned addr_width = getProcessState()->getAddressWidth();

  struct {
    Address out_fp;
    Address out_ra;
  } ra_fp_pair;

#if defined(arch_x86_64)
  struct {
     unsigned out_fp;
     unsigned out_ra;
  } ra_fp_pair32;
#endif

  if (!in.getFP())
    return gcf_stackbottom;

  in_fp = in.getFP();
  out_sp = in_fp + addr_width;
#if defined(arch_x86_64)
  /**
   * On AMD64 we may be reading from a process with a different
   * address width than the current one.  We'll do the read at
   * the correct size, then convert the addresses into the 
   * local size
   **/
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
    sw_printf("[%s:%u] - Couldn't read from %x\n", __FILE__, __LINE__, out_sp);
    return gcf_error;
  }
  
  if (!ra_fp_pair.out_ra) {
    return gcf_stackbottom;
  }

  out.setFP(ra_fp_pair.out_fp);
  out.setRA(ra_fp_pair.out_ra);
  out.setSP(out_sp);

  return gcf_success;
}
 
unsigned FrameFuncStepper::getPriority()
{
  return 0x11000;
}

