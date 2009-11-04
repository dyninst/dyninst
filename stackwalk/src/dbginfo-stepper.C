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

#include "stackwalk/h/framestepper.h"
#include "stackwalk/h/frame.h"
#include "stackwalk/h/procstate.h"
#include "stackwalk/h/swk_errors.h"
#include "stackwalk/h/steppergroup.h"
#include "stackwalk/h/walker.h"
#include "stackwalk/src/symtab-swk.h"
#include "stackwalk/src/dbgstepper-impl.h"

#include "dynutil/h/dyntypes.h"


using namespace Dyninst;
using namespace Stackwalker;
using namespace SymtabAPI;

#if defined(cap_stackwalker_use_symtab)

DebugStepperImpl::DebugStepperImpl(Walker *w, DebugStepper *parent) :
   FrameStepper(w),
   parent_stepper(parent),
   cur_frame(NULL)
{
}

bool DebugStepperImpl::ReadMem(Address addr, void *buffer, unsigned size)
{
   return getProcessState()->readMem(buffer, addr, size);
}

bool DebugStepperImpl::GetReg(MachRegister reg, MachRegisterVal &val)
{
   if (isFrameRegister(reg)) {
      val = static_cast<MachRegisterVal>(cur_frame->getFP());
      return true;
   }

   if (isStackRegister(reg)) {
      val = static_cast<MachRegisterVal>(cur_frame->getSP());
      return true;
   }
   
   if (reg == MachRegReturn) {
      val = static_cast<MachRegisterVal>(cur_frame->getRA());
      return true;
   }

   return false;
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

   Symtab *symtab = SymtabWrapper::getSymtab(lib.first);
   if (!symtab) {
      sw_printf("[%s:%u] - Could not open file %s with SymtabAPI %s\n",
                 __FILE__, __LINE__, lib.first.c_str());
      setLastError(err_nofile, "Could not open file for Debugging stackwalker\n");
      return gcf_error;
   }
   
   if (!symtab->hasStackwalkDebugInfo())
   {
      sw_printf("[%s:%u] - Library %s does not have stackwalking debug info\n",
                 __FILE__, __LINE__, lib.first.c_str());
      return gcf_not_me;
   }
   
   Address pc = in.getRA() - lib.second;

   cur_frame = &in;
   gcframe_ret_t gcresult =  getCallerFrameArch(pc, in, out, symtab);
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
   return 0x10010;
}

DebugStepperImpl::~DebugStepperImpl()
{
}

#endif


