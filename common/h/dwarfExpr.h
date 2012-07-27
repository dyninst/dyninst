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

#if !defined(DWARF_EXPR_H)
#define DWARF_EXPR_H

#include <stack>
#include "dynutil/h/dyn_regs.h"

namespace Dyninst {

class VariableLocation;
class ProcessReader;

#if defined(arch_x86_64)

extern int Register_DWARFtoMachineEnc32(int n);
extern int Register_DWARFtoMachineEnc64(int n);

#endif

extern MachRegister DwarfToDynReg(signed long reg, Dyninst::Architecture arch);
extern signed long DynToDwarfReg(Dyninst::MachRegister reg);

}

#if defined(arch_x86_64)
#define DWARF_TO_MACHINE_ENC(n, proc)					\
  ((proc->getAddressWidth() == 4) ? Register_DWARFtoMachineEnc32((int) n) : Register_DWARFtoMachineEnc64((int) n))
#define DWARF_TO_MACHINE_ENC_W(n, w) \
  (w == 4) ? Register_DWARFtoMachineEnc32((int) n) : Register_DWARFtoMachineEnc64((int) n)
#define DWARF_TO_MACHINEREG_ENC_W(n, w) \
   (w == 4) ? MachRegister::DwarfEncToReg(n, Dyninst::Arch_x86) : MachRegister::DwarfEncToReg(n, Dyninst::Arch_x86_64);
#else
#define DWARF_TO_MACHINE_ENC(n, proc) (n)
#define DWARF_TO_MACHINE_ENC_W(n, w) (n)
#define DWARF_TO_MACHINEREG_ENC_W(n, w) MachRegister::DwarfEncToReg(n, Dyninst::Arch_x86);
#endif

#define DWARF_FALSE_IF(condition,...)		\
  if ( condition ) { return false; }
#define DWARF_RETURN_IF(condition,...)		\
  if ( condition ) { return; }
#define DWARF_NULL_IF(condition,...)		\
  if ( condition ) { return NULL; }
#define DWARF_NEXT_IF(condition, ...)					\
  if (condition) { if (depth != 1) { return false; } else {walk_error = true; break; } }

#if defined(COMPONENT_NAME) && !defined(libcommon)

#include "libdwarf.h"
#include "dwarf.h"

namespace Dyninst {
namespace COMPONENT_NAME {

bool decodeDwarfExpression(Dwarf_Locdesc *dwlocs,
                           long int *initialStackValue,
                           Dyninst::VariableLocation *loc, bool &isLocSet,
                           Dyninst::ProcessReader *reader,
                           Dyninst::Architecture arch,
                           long int &end_result);

}
}
#endif

#endif
