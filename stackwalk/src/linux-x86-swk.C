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
#include <sys/ucontext.h>


using namespace Dyninst;
using namespace Dyninst::Stackwalker;

struct sigframe_offsets {
   int fp_offset;
   int pc_offset;
   int sp_offset;
   int frame_size;
};

int frames_32_size = 3;
sigframe_offsets frames_32[] = { {28, 60, 32, 728}, {28, 60, 32, 736}, 
                                 {28, 60, 32, 0} };

int frames_64_size = 6;
sigframe_offsets frames_64[] = { {120, 168, 160, 568}, {120, 168, 160, 1096}, 
                                 {120, 168, 160, 1112}, {120, 168, 160, 1180},
                                 {120, 168, 160, 1120}, {120, 168, 160, 0} };


gcframe_ret_t SigHandlerStepperImpl::getCallerFrame(const Frame &in, Frame &out)
{
   bool result;
   int addr_size = getProcessState()->getAddressWidth();

   unsigned frames_size;
   sigframe_offsets *frames;
   if (addr_size == 4) {
      frames_size = frames_32_size;
      frames = frames_32;
   }
   else {
      frames_size = frames_64_size;
      frames = frames_64;
   }

   Address last_read_sp_addr = 0;
   Address last_read_sp_val = 0;

   for (unsigned i = 0; i < frames_size; i++) {
      location_t sp_loc;
      sp_loc.location = loc_address;
      sp_loc.val.addr = in.getSP() + frames[i].sp_offset;
      Address sp = 0;
      if (last_read_sp_addr != sp_loc.val.addr)
      {
         result = getProcessState()->readMem(&sp, sp_loc.val.addr, addr_size);
         if (!result) {
            sw_printf("[%s:%d] Unexpected error reading from stack memory 0x%lx for signal frame\n",
                      FILE__, __LINE__, last_read_sp_addr);
            return gcf_error;
         }
         last_read_sp_addr = sp_loc.val.addr;
         last_read_sp_val = sp;
      }
      else {
         sp = last_read_sp_val;
      }

      if (frames[i].frame_size && (sp != in.getSP() + frames[i].frame_size)) {
         sw_printf("[%s:%d] - Signal frame candidate %u does not fit (%lx != %lx). Trying another.\n",
                   FILE__, __LINE__, i, sp, in.getSP() + frames[i].frame_size);
         continue;
      }
      sw_printf("[%s:%d] - Using signal frame candidate %u\n", FILE__, __LINE__, i);

      location_t fp_loc;
      Address fp = 0x0;
      fp_loc.location = loc_address;
      fp_loc.val.addr = in.getSP() + frames[i].fp_offset;
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
      pc_loc.val.addr = in.getSP() + frames[i].pc_offset;
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
   
   sw_printf("[%s:%d] - Could not find matching candidate for signal frame\n", FILE__, __LINE__);
   return gcf_not_me;
}
