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

#include "stackwalk/h/framestepper.h"
#include "stackwalk/h/frame.h"
#include "stackwalk/h/procstate.h"
#include "stackwalk/h/swk_errors.h"
#include "stackwalk/h/steppergroup.h"
#include "stackwalk/h/walker.h"
#include "stackwalk/src/dbgstepper-impl.h"
#include "stackwalk/src/linuxbsd-swk.h"
#include "dynutil/h/dyntypes.h"
#include "common/h/Types.h"

#include "symtabAPI/h/Symtab.h"

using namespace Dyninst;
using namespace Stackwalker;

static std::map<std::string, DwarfSW *> dwarf_info;

typedef enum {
   storageAddr,
   storageReg,
   storageRegOffset
} storageClass;

typedef enum {
   storageRef,
   storageNoRef
} storageRefClass;

class VariableLocation {
public:
   storageClass stClass;
   storageRefClass refClass;
   int reg;
   MachRegister mr_reg;
   long frameOffset;
   Address lowPC;
   Address hiPC;
};

#include <stdarg.h>
#include "dwarf.h"
#include "libdwarf.h"
#include "common/h/dwarfExpr.h"
#include "common/h/dwarfSW.h"
#include "common/h/Elf_X.h"

DwarfSW *getDwarfInfo(std::string s, unsigned addr_width)
{
   Dwarf_Debug dbg;
   DwarfSW *ret;
   static std::map<std::string, DwarfSW *> dwarf_info;
   std::map<std::string, DwarfSW *>::iterator i = dwarf_info.find(s);
   if (i != dwarf_info.end())
      return i->second;

   bool result = getDwarfDebug(s, &dbg);
   if (!result) goto done;

   ret = new DwarfSW(dbg, addr_width);

  done:
   dwarf_info[s] = ret;
   return ret;
}

DebugStepperImpl::DebugStepperImpl(Walker *w, DebugStepper *parent) :
   FrameStepper(w),
   parent_stepper(parent),
   cur_frame(NULL),
   depth_frame(NULL)
{
}

bool DebugStepperImpl::ReadMem(Address addr, void *buffer, unsigned size)
{
   return getProcessState()->readMem(buffer, addr, size);
}

bool DebugStepperImpl::GetReg(MachRegister reg, MachRegisterVal &val)
{
   using namespace SymtabAPI;
   
   const Frame *prevDepthFrame = depth_frame;
  
   if (reg.isFramePointer()) {
      val = static_cast<MachRegisterVal>(depth_frame->getFP());
      return true;
   }

   if (reg.isStackPointer()) {
      val = static_cast<MachRegisterVal>(depth_frame->getSP());
      return true;
   }
   
   if (reg.isPC()) {
      val = static_cast<MachRegisterVal>(depth_frame->getRA());
      return true;
   }

   depth_frame = depth_frame->getPrevFrame();
   if (!depth_frame)
   {
      bool bres =  getProcessState()->getRegValue(reg, cur_frame->getThread(), val);
      depth_frame = prevDepthFrame;
      return bres;
   }

   Offset offset;
   void *symtab_v;
   std::string lib;
   depth_frame->getLibOffset(lib, offset, symtab_v);
   Symtab *symtab = (Symtab*) symtab_v;
   if (!symtab)
   {
     depth_frame = prevDepthFrame;
     return false;
   }

   bool result = symtab->getRegValueAtFrame(offset, reg, val, this);
   depth_frame = prevDepthFrame;
   return result;
}

gcframe_ret_t DebugStepperImpl::getCallerFrame(const Frame &in, Frame &out)
{
   LibAddrPair lib;
   bool result;

   result = getProcessState()->getLibraryTracker()->getLibraryAtAddr(in.getRA(), lib);
   if (!result) {
      sw_printf("[%s:%u] - Stackwalking through an invalid PC at %lx\n",
                __FILE__, __LINE__, in.getRA());
      return gcf_stackbottom;
   }
   
   DwarfSW *dinfo = getDwarfInfo(lib.first, walker->getProcessState()->getAddressWidth());
   if (!dinfo) {
      sw_printf("[%s:%u] - Could not open file %s for DWARF info\n",
                __FILE__, __LINE__, lib.first.c_str());
      setLastError(err_nofile, "Could not open file for Debugging stackwalker\n");
      return gcf_error;
   }
   if (!dinfo->hasFrameDebugInfo())
   {
      sw_printf("[%s:%u] - Library %s does not have stackwalking debug info\n",
                 __FILE__, __LINE__, lib.first.c_str());
      return gcf_not_me;
   }   
   Address pc = in.getRA() - lib.second;

   bool isVsyscallPage = false;
#if defined(os_linux)
   isVsyscallPage = (strstr(lib.first.c_str(), "[vsyscall-") != NULL);
#endif

   cur_frame = &in;
   gcframe_ret_t gcresult = getCallerFrameArch(pc, in, out, dinfo, isVsyscallPage);
   cur_frame = NULL;
   return gcresult;
}

void DebugStepperImpl::registerStepperGroup(StepperGroup *group)
{
   unsigned addr_width = group->getWalker()->getProcessState()->getAddressWidth();
   if (addr_width == 4)
      group->addStepper(parent_stepper, 0, 0xffffffff);
#if defined(arch_64bit)
   else if (addr_width == 8)
      group->addStepper(parent_stepper, 0, 0xffffffffffffffff);
#endif
   else
      assert(0 && "Unknown architecture word size");
}

unsigned DebugStepperImpl::getPriority() const
{
   return debugstepper_priority;
}

DebugStepperImpl::~DebugStepperImpl()
{
}

#if defined(arch_x86) || defined(arch_x86_64)
gcframe_ret_t DebugStepperImpl::getCallerFrameArch(Address pc, const Frame &in, 
                                                   Frame &out, DwarfSW *dinfo,
                                                   bool isVsyscallPage)
{
   MachRegisterVal frame_value, stack_value, ret_value;
   bool result;
   FrameErrors_t frame_error = FE_No_Error;

   Dyninst::Architecture arch;
   unsigned addr_width = getProcessState()->getAddressWidth();
   if (addr_width == 4)
      arch = Dyninst::Arch_x86;
   else
      arch = Dyninst::Arch_x86_64;

   depth_frame = cur_frame;

   result = dinfo->getRegValueAtFrame(pc, Dyninst::ReturnAddr,
                                      ret_value, arch, this, frame_error);

   if (!result && frame_error == FE_No_Frame_Entry && isVsyscallPage) {
      //Work-around kernel bug.  The vsyscall page location was randomized, but
      // the debug info still has addresses from the old, pre-randomized days.
      // See if we get any hits by assuming the address corresponds to the 
      // old PC.
      pc += 0xffffe000;
      result = dinfo->getRegValueAtFrame(pc, Dyninst::ReturnAddr,
                                         ret_value, arch, this, frame_error);

   }
   if (!result) {
      sw_printf("[%s:%u] - Couldn't get return debug info at %lx, error: %u\n",
                __FILE__, __LINE__, in.getRA(), frame_error);
      return gcf_not_me;
   }

 
   
   Dyninst::MachRegister frame_reg;
   if (addr_width == 4)
      frame_reg = x86::ebp;
   else
      frame_reg = x86_64::rbp;

   result = dinfo->getRegValueAtFrame(pc, frame_reg,
                                      frame_value, arch, this, frame_error);
   if (!result) {
      sw_printf("[%s:%u] - Couldn't get frame debug info at %lx\n",
                 __FILE__, __LINE__, in.getRA());
      return gcf_not_me;
   }

   result = dinfo->getRegValueAtFrame(pc, Dyninst::FrameBase,
                                      stack_value, arch, this, frame_error);
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
#endif

