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
#include "stackwalk/src/linux-swk.h"
#include "stackwalk/src/libstate.h"
#include "dynutil/h/dyntypes.h"


using namespace Dyninst;
using namespace Stackwalker;

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

#include "dwarf.h"
#include "libdwarf.h"
#define setSymtabError(x) 
#define dwarf_printf sw_printf
#include "common/h/dwarfExpr.h"
#include "common/h/dwarfSW.h"
#include "common/h/Elf_X.h"

static DwarfSW *ll_getDwarfInfo(Elf_X *elfx)
{
   Elf *elf = elfx->e_elfp();
   Dwarf_Debug dbg;
   Dwarf_Error err;
   int status = dwarf_elf_init(elf, DW_DLC_READ, NULL, NULL, &dbg, &err);
   if (status != DW_DLV_OK) {
      sw_printf("Error opening dwarf information %u (0x%x): %s\n",
                (unsigned) dwarf_errno(err), (unsigned) dwarf_errno(err),
                dwarf_errmsg(err));
      return NULL;
   }
   return new DwarfSW(dbg, elfx->wordSize());
}

static DwarfSW *getDwarfInfo(std::string s)
{
   static std::map<std::string, DwarfSW *> dwarf_info;

   std::map<std::string, DwarfSW *>::iterator i = dwarf_info.find(s);
   if (i != dwarf_info.end())
      return i->second;

   Elf_X *elfx = getElfHandle(s);
   DwarfSW *result = ll_getDwarfInfo(elfx);
   dwarf_info[s] = result;
   return result;
}

static DwarfSW *getAuxDwarfInfo(std::string s)
{
   static std::map<std::string, DwarfSW *> dwarf_aux_info;

   std::map<std::string, DwarfSW *>::iterator i = dwarf_aux_info.find(s);
   if (i != dwarf_aux_info.end())
      return i->second;

   Elf_X *orig_elf = getElfHandle(s);
   if (!orig_elf) {
      sw_printf("[%s:%u] - Error. Could not find elf handle for file %s\n",
                __FILE__, __LINE__, s.c_str());
      dwarf_aux_info[s] = NULL;
      return NULL;
   }

   string dbg_name;
   char *dbg_buffer;
   unsigned long dbg_buffer_size;
   bool result = orig_elf->findDebugFile(s, dbg_name, dbg_buffer, dbg_buffer_size);
   if (!result) {
      sw_printf("[%s:%u] - No separate debug file associated with %s\n",
                __FILE__, __LINE__, s.c_str());
      dwarf_aux_info[s] = NULL;
      return NULL;
   }
   
   SymbolReaderFactory *fact = getDefaultSymbolReader();
   SymReader *reader = fact->openSymbolReader(dbg_buffer, dbg_buffer_size);
   if (!reader) {
      sw_printf("[%s:%u] - Error opening symbol reader for buffer associated with %s\n",
                __FILE__, __LINE__, dbg_name.c_str());
      dwarf_aux_info[s] = NULL;
      return NULL;
   }

   Elf_X *elfx = reader->getElfHandle();
   DwarfSW *dresult = ll_getDwarfInfo(elfx);
   dwarf_aux_info[s] = dresult;
   return dresult;
}


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
   if (reg.isFramePointer()) {
      val = static_cast<MachRegisterVal>(cur_frame->getFP());
      return true;
   }

   if (reg.isStackPointer()) {
      val = static_cast<MachRegisterVal>(cur_frame->getSP());
      return true;
   }
   
   if (reg.isPC()) {
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

   Address pc = in.getRA() - lib.second;

   /**
    * Some system libraries on some systems have their debug info split
    * into separate files, usually in /usr/lib/debug/.  Check these 
    * for DWARF debug info
    **/
   DwarfSW *dauxinfo = getAuxDwarfInfo(lib.first);
   if (dauxinfo && dauxinfo->hasFrameDebugInfo()) {
      sw_printf("[%s:%u] - Using separate DWARF debug file for %s", 
                __FILE__, __LINE__, lib.first.c_str());
      cur_frame = &in;
      gcframe_ret_t gcresult = getCallerFrameArch(pc, in, out, dauxinfo);
      cur_frame = NULL;
      if (gcresult == gcf_success) {
         sw_printf("[%s:%u] - Success walking with DWARF aux file\n",
                   __FILE__, __LINE__);
         return gcf_success;
      }
   }
   
   /**
    * Check the actual file for DWARF stackwalking data
    **/
   DwarfSW *dinfo = getDwarfInfo(lib.first);
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
   cur_frame = &in;
   gcframe_ret_t gcresult = getCallerFrameArch(pc, in, out, dinfo);
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
                                                   Frame &out, DwarfSW *dinfo)
{
   MachRegisterVal frame_value, stack_value, ret_value;
   bool result;

   Dyninst::Architecture arch;
   unsigned addr_width = getProcessState()->getAddressWidth();
   if (addr_width == 4)
      arch = Dyninst::Arch_x86;
   else
      arch = Dyninst::Arch_x86_64;

   result = dinfo->getRegValueAtFrame(pc, Dyninst::ReturnAddr,
                                      ret_value, arch, this);
   if (!result) {
      sw_printf("[%s:%u] - Couldn't get return debug info at %lx\n",
                __FILE__, __LINE__, in.getRA());
      return gcf_not_me;
   }
   
   Dyninst::MachRegister frame_reg;
   if (addr_width == 4)
      frame_reg = x86::ebp;
   else
      frame_reg = x86_64::rbp;

   result = dinfo->getRegValueAtFrame(pc, frame_reg,
                                      frame_value, arch, this);
   if (!result) {
      sw_printf("[%s:%u] - Couldn't get frame debug info at %lx\n",
                 __FILE__, __LINE__, in.getRA());
      return gcf_not_me;
   }

   result = dinfo->getRegValueAtFrame(pc, Dyninst::FrameBase,
                                      stack_value, arch, this);
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



