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
#include "stackwalk/h/procstate.h"
#include "stackwalk/h/framestepper.h"
#include "stackwalk/h/basetypes.h"
#include "stackwalk/h/frame.h"

#include "stackwalk/src/symtab-swk.h"
#include "stackwalk/src/linuxbsd-swk.h"
#include "stackwalk/src/dbgstepper-impl.h"
#include "stackwalk/src/x86-swk.h"

#include <sys/user.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

using namespace Dyninst;
using namespace Dyninst::Stackwalker;

static const int sp_offset_32 = 120;
static const int fp_offset_32 = 76;
static const int pc_offset_32 = 108;
static const int sp_offset_64 = 216;
static const int fp_offset_64 = 104;
static const int pc_offset_64 = 192;

gcframe_ret_t SigHandlerStepperImpl::getCallerFrame(const Frame &in, Frame &out)
{
   int fp_offset;
   int pc_offset;
   int sp_offset;
   bool result;
   int addr_size = getProcessState()->getAddressWidth();
   if (addr_size == 4) {
      fp_offset = fp_offset_32;
      pc_offset = pc_offset_32;
      sp_offset = sp_offset_32;
   }
   else {
      fp_offset = fp_offset_64;
      pc_offset = pc_offset_64;
      sp_offset = sp_offset_64;
  }

   location_t fp_loc;
   Address fp = 0x0;
   fp_loc.location = loc_address;
   fp_loc.val.addr = in.getSP() + fp_offset;
   sw_printf("[%s:%u] - SigHandler Reading FP from %lx\n",
             FILE__, __LINE__, fp_loc.val.addr);
   result = getProcessState()->readMem(&fp, fp_loc.val.addr, addr_size);
   if (!result) {
      return gcf_error;
   }

   location_t pc_loc;
   Address pc = 0x0;
   pc_loc.location = loc_address;
   pc_loc.val.addr = in.getSP() + pc_offset;
   sw_printf("[%s:%u] - SigHandler Reading PC from %lx\n",
             FILE__, __LINE__, pc_loc.val.addr);
   result = getProcessState()->readMem(&pc, pc_loc.val.addr, addr_size);
   if (!result) {
      return gcf_error;
   }

   location_t sp_loc;
   Address sp = 0x0;
   sp_loc.location = loc_address;
   sp_loc.val.addr = in.getSP() + sp_offset;
   sw_printf("[%s:%u] - SigHandler Reading PC from %lx\n",
             FILE__, __LINE__, sp_loc.val.addr);
   result = getProcessState()->readMem(&sp, sp_loc.val.addr, addr_size);
   if (!result) {
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

void SigHandlerStepperImpl::registerStepperGroup(StepperGroup *group)
{
   int addr_size = getProcessState()->getAddressWidth();

   // FreeBSD signal handler tramps are in a static location
   // at the top of the user space stack
   if (addr_size == 4) {
     group->addStepper(parent_stepper, 0xbfbfffb4, 0xbfbfffb4+1);
   }
#if defined(arch_64bit)
   else {
     group->addStepper(parent_stepper, 0x00007fffffffffc4, 0x00007fffffffffc4+1);
   }  
#endif
}
