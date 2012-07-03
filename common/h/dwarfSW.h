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

#if !defined(DWARF_SW_H_)
#define DWARF_SW_H_

#include <vector>
#include "dynutil/h/dyntypes.h"
#include "dynutil/h/ProcReader.h"
#include "libdwarf.h"

namespace Dyninst {
namespace COMPONENT_NAME {

typedef enum {
   FE_Bad_Frame_Data = 15,   /* to coincide with equivalent SymtabError */
   FE_No_Frame_Entry,
   FE_Frame_Read_Error,
   FE_No_Error
} FrameErrors_t;

typedef struct {
  Dwarf_Fde *fde_data;
  Dwarf_Signed fde_count;
  Dwarf_Cie *cie_data;
  Dwarf_Signed cie_count;   
} fde_cie_data;

class DwarfSW {
   typedef enum {
      dwarf_status_uninitialized,
      dwarf_status_error,
      dwarf_status_ok
   } dwarf_status_t;
   Dwarf_Debug dbg;
   unsigned addr_width;
   dwarf_status_t fde_dwarf_status;

   std::vector<fde_cie_data> fde_data;
   void setupFdeData();
  public:
   DwarfSW(Dwarf_Debug dbg_, unsigned addr_width_);
   ~DwarfSW();

   bool hasFrameDebugInfo();
   bool getRegValueAtFrame(Address pc, 
                           Dyninst::MachRegister reg, 
                           Dyninst::MachRegisterVal &reg_result,
                           Dyninst::Architecture arch,
                           ProcessReader *reader,
                           FrameErrors_t &err_result);
};

}
}

#endif
