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
#include "stackwalk/h/walker.h"
#include "stackwalk/h/basetypes.h"
#include "stackwalk/h/procstate.h"
#include "stackwalk/h/framestepper.h"
#include "stackwalk/h/frame.h"

#include "stackwalk/src/linuxbsd-swk.h"
#include "stackwalk/src/dbgstepper-impl.h"
#include "registers/aarch64_regs.h"
#include "registers/MachRegister.h"
#include "frame.h"

#include <sys/user.h>
#include <sys/ptrace.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <sys/ucontext.h>



using namespace Dyninst;
using namespace Dyninst::Stackwalker;

bool Walker::createDefaultSteppers()
{
    FrameStepper *stepper;
    bool result;

    stepper = new FrameFuncStepper(this);
    result = addStepper(stepper);
    if (!result) {
        sw_printf("[%s:%d] - Error adding stepper %p\n", FILE__, __LINE__,
                  (void*)stepper);
        return false;
    } else {
        sw_printf("[%s:%d] - Stepper %p is FrameFuncStepper\n",
                  FILE__, __LINE__, (void*)stepper);
    }

    stepper = new DebugStepper(this);
    result = addStepper(stepper);
    if (!result){
        sw_printf("[%s:%d] - Error adding stepper %p\n", FILE__, __LINE__,
                  (void*)stepper);
        return false;
    } else{
        sw_printf("[%s:%d] - Stepper %p is DebugStepper\n",
                  FILE__, __LINE__, (void*)stepper);
    }

    stepper = new SigHandlerStepper(this);
    result = addStepper(stepper);
    if (!result) {
        sw_printf("[%s:%d] - Error adding stepper %p\n", FILE__, __LINE__,
                  (void*)stepper);
        return false;
    }else {
        sw_printf("[%s:%d] - Stepper %p is SigHandlerStepper\n",
                  FILE__, __LINE__, (void*)stepper);
    }

    stepper = new BottomOfStackStepper(this);
    result = addStepper(stepper);
    if (!result){
        sw_printf("[%s:%d] - Error adding stepper %p\n", FILE__, __LINE__,
                  (void*)stepper);
        return false;
    }else{
        sw_printf("[%s:%d] - Stepper %p is BottomOfStackStepper\n",
                  FILE__, __LINE__, (void*)stepper);
    }

    return true;
}

bool DebugStepperImpl::isFrameRegister(MachRegister reg)
{
   if (getProcessState()->getAddressWidth() == 4){
       assert(0);
      return (reg == aarch64::x29);
   }
   else
      return (reg == aarch64::x29);
}

bool DebugStepperImpl::isStackRegister(MachRegister reg)
{
   if (getProcessState()->getAddressWidth() == 4){
       assert(0);
      return (reg == aarch64::sp);
   }
   else
      return (reg == aarch64::sp);
}

static     ucontext_t dummy_context;
static int sp_offset = (char*)&(dummy_context.uc_mcontext.sp)       - (char*)&dummy_context;
static int fp_offset = (char*)&(dummy_context.uc_mcontext.regs[29]) - (char*)&dummy_context;
static int pc_offset = (char*)&(dummy_context.uc_mcontext.pc)       - (char*)&dummy_context;

gcframe_ret_t SigHandlerStepperImpl::getCallerFrame(const Frame & in,
                                                    Frame & out)
{
    // This function assumes there is FP in "Frame in"
    // And assumes that ucontext is the first object on the stack frame
    bool result;

    Address last_read_sp_addr = 0;
    Address last_read_sp_val = 0;
    int addr_size = 8;
    location_t sp_loc;
    sp_loc.location = loc_address;
    sp_loc.val.addr = in.getFP() + sp_offset - sizeof(dummy_context);

    Address sp = 0;
    sw_printf("In frame sp %lx, fp %lx, pc %lx\n", in.getSP(), in.getFP(), in.getRA());
    if (last_read_sp_addr != sp_loc.val.addr)
    {
        result = getProcessState()->readMem(&sp, sp_loc.val.addr, addr_size);
        if (!result) {
            sw_printf("[%s:%d] Unexpected error reading from stack memory 0x%lx for signal frame\n",
                      FILE__, __LINE__, sp_loc.val.addr);
            return gcf_error;
        }
        last_read_sp_addr = sp_loc.val.addr;
        last_read_sp_val = sp;
    }
    else {
        sp = last_read_sp_val;
    }


    location_t fp_loc;
    Address fp = 0x0;
    fp_loc.location = loc_address;
    fp_loc.val.addr = in.getFP() + fp_offset - sizeof(dummy_context);
    sw_printf("[%s:%d] - SigHandler Reading FP from %lx\n",
              FILE__, __LINE__, fp_loc.val.addr);
    result = getProcessState()->readMem(&fp, fp_loc.val.addr, addr_size);
    if (!result) {
        sw_printf("[%s:%d] Unexpected error reading from stack memory 0x%lx for signal frame\n",
                  FILE__, __LINE__, fp_loc.val.addr);
        return gcf_error;
    }

    location_t pc_loc;
    Address pc = 0x0;
    pc_loc.location = loc_address;
    pc_loc.val.addr = in.getFP() + pc_offset - sizeof(dummy_context);
    sw_printf("[%s:%d] - SigHandler Reading PC from %lx\n",
              FILE__, __LINE__, pc_loc.val.addr);
    result = getProcessState()->readMem(&pc, pc_loc.val.addr, addr_size);
    if (!result) {
        sw_printf("[%s:%d] Unexpected error reading from stack memory 0x%lx for signal frame\n",
                  FILE__, __LINE__, pc_loc.val.addr);
        return gcf_error;
    }

    out.setRA((Dyninst::MachRegisterVal) pc);
    out.setFP((Dyninst::MachRegisterVal) fp);
    out.setSP((Dyninst::MachRegisterVal) sp);
    out.setRALocation(pc_loc);
    out.setFPLocation(fp_loc);
    out.setSPLocation(sp_loc);
    out.setNonCall();
    return gcf_success;

}
