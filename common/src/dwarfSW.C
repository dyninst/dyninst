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

#include "common/h/dwarfSW.h"
#include "common/h/dwarfExpr.h"
#include "common/h/Types.h"
#include "libdwarf.h"

using namespace Dyninst;
using namespace COMPONENT_NAME;

DwarfSW::DwarfSW(Dwarf_Debug dbg_, unsigned addr_width_) :
   dbg(dbg_),
   addr_width(addr_width_),
   fde_dwarf_status(dwarf_status_uninitialized)
{
}

DwarfSW::~DwarfSW()
{
   if (fde_dwarf_status != dwarf_status_ok) 
      return;
   for (unsigned i=0; i<fde_data.size(); i++)
   {
      dwarf_fde_cie_list_dealloc(dbg, 
                                 fde_data[i].cie_data, fde_data[i].cie_count, 
                                 fde_data[i].fde_data, fde_data[i].fde_count);
   }
}

bool DwarfSW::hasFrameDebugInfo()
{
   setupFdeData();
   return fde_dwarf_status == dwarf_status_ok;
}

bool DwarfSW::getRegValueAtFrame(Address pc, 
                                        Dyninst::MachRegister reg,
                                        Dyninst::MachRegisterVal &reg_result,
                                        Dyninst::Architecture arch,
                                        ProcessReader *reader,
                                        FrameErrors_t &err_result)
{
	int result;
	Dwarf_Fde fde;
	Dwarf_Addr lowpc, hipc;
	Dwarf_Error err;

        err_result = FE_No_Error;

	/**
	 * Initialize the FDE and CIE data.  This is only really done once, 
	 * after which setupFdeData will immediately return.
    **/
   setupFdeData();
   if (!fde_data.size()) {
      err_result = FE_Bad_Frame_Data;
      return false;
   }


   /**
    * Get the FDE at this PC.  The FDE contains the rules for getting
    * registers at the given PC in this frame.
    **/
   bool found = false;
   unsigned cur_fde;
   for (cur_fde=0; cur_fde<fde_data.size(); cur_fde++)
   {
      result = dwarf_get_fde_at_pc(fde_data[cur_fde].fde_data, 
                                   (Dwarf_Addr) pc, &fde, &lowpc, &hipc, &err);
      if (result == DW_DLV_ERROR)
      {
         err_result = FE_Bad_Frame_Data;
         return false;
      }
      else if (result == DW_DLV_OK) {
         found = true;
         break;
      }
   }
   if (!found)
   {
      err_result = FE_No_Frame_Entry;
      return false;
   }

   Dwarf_Half dwarf_reg;
   if (reg == Dyninst::ReturnAddr) {
      /**
       * We should be getting the return address for the stack frame.  
       * This is treated as a virtual register in the
       * FDE.  We can look up the virtual register in the CIE.
       **/
      Dwarf_Cie cie;
      result = dwarf_get_cie_of_fde(fde, &cie, &err);
      if (result != DW_DLV_OK) {
         err_result = FE_Bad_Frame_Data;
         return false;
      }

      Dwarf_Unsigned bytes_in_cie;
      result = dwarf_get_cie_info(cie, &bytes_in_cie, NULL, NULL, NULL, NULL, 
                                  &dwarf_reg, NULL, NULL, &err);
      if (result != DW_DLV_OK) {
         err_result = FE_Bad_Frame_Data;
         return false;
      }
   }
   else if (reg == Dyninst::FrameBase) {
      dwarf_reg = DW_FRAME_CFA_COL3;
   }
   else {
      dwarf_reg = DynToDwarfReg(reg);
   }

   Dwarf_Small value_type;
   Dwarf_Signed offset_relevant, register_num, offset_or_block_len;
   Dwarf_Ptr block_ptr;
   Dwarf_Addr row_pc;

   /**
    * Decode the rule that describes how to get dwarf_reg at pc.
    **/
   if (dwarf_reg != DW_FRAME_CFA_COL3) {
      result = dwarf_get_fde_info_for_reg3(fde, dwarf_reg, pc, &value_type, 
                                           &offset_relevant, &register_num,
                                           &offset_or_block_len,
                                           &block_ptr, &row_pc, &err);
   }
   else {
      result = dwarf_get_fde_info_for_cfa_reg3(fde, pc, &value_type, 
                                               &offset_relevant, &register_num,
                                               &offset_or_block_len,
                                               &block_ptr, &row_pc, &err);
   }
   if (result == DW_DLV_ERROR) {
      err_result = FE_Bad_Frame_Data;
      return false;
   }

   /**
    * Interpret the rule and turn it into a real value.
    **/
   unsigned word_size = addr_width;

   Dyninst::Address res_addr = 0;
   Dyninst::MachRegisterVal register_val;
   
   if (value_type == DW_EXPR_OFFSET || value_type == DW_EXPR_VAL_OFFSET)
   {
      if (register_num == DW_FRAME_CFA_COL3) {
         FrameErrors_t rec_err = FE_No_Error;
         bool bresult = getRegValueAtFrame(pc, Dyninst::FrameBase,
                                           register_val, arch, reader, rec_err);
         if (!bresult) {
            err_result = rec_err;
            return false;
         }
      }
      else if (register_num == DW_FRAME_SAME_VAL) {
         bool bresult = reader->GetReg(reg, register_val);
         if (!bresult) {
            err_result = FE_Frame_Read_Error;
            return false;
         }
         reg_result = register_val;
         return true;
      }
      else {
         Dyninst::MachRegister dyn_registr = DwarfToDynReg(register_num, arch);
         bool bresult = reader->GetReg(dyn_registr, register_val);
         if (!bresult) {
            err_result = FE_Frame_Read_Error;
            return false;
         }
      }
   }

   if ((value_type == DW_EXPR_VAL_OFFSET) || 
       (value_type == DW_EXPR_OFFSET && dwarf_reg == DW_FRAME_CFA_COL3)) {
      assert(offset_relevant);
      reg_result = (Dyninst::MachRegisterVal) (register_val + offset_or_block_len);
      return true;
   }
   else if (value_type == DW_EXPR_OFFSET && offset_relevant) {
      res_addr = (Dyninst::Address) (register_val + offset_or_block_len);
   }
   else if (value_type == DW_EXPR_OFFSET && !offset_relevant)
   {
      reg_result = register_val;
      return true;
   } 
   else if (value_type == DW_EXPR_VAL_EXPRESSION || value_type == DW_EXPR_EXPRESSION)
   {
      Dwarf_Locdesc *llbuf = NULL;
      Dwarf_Signed listlen = 0;
      result = dwarf_loclist_from_expr(dbg, block_ptr, offset_or_block_len, &llbuf, &listlen, &err);
      if (result != DW_DLV_OK) {
         return false;
      }
      
      bool locset = false;
      long int end_result = 0;
      bool bresult = decodeDwarfExpression(llbuf, NULL, NULL, locset, reader, 
                                           arch, end_result);
      dwarf_dealloc(dbg, llbuf->ld_s, DW_DLA_LOC_BLOCK);
      dwarf_dealloc(dbg, llbuf, DW_DLA_LOCDESC);
      if (bresult && value_type == DW_EXPR_EXPRESSION) {
         if (word_size == 4) {
            uint32_t r;
            bresult = reader->ReadMem((Dyninst::Address) end_result, &r, 4);
            reg_result = r;
         }
         else if (word_size == 8) {
            uint64_t r;
            bresult = reader->ReadMem((Dyninst::Address) end_result, &r, 8);
            reg_result = r;
         }
         else {
            assert(0); //Unknown word size
         }
      }
      else if (bresult && value_type == DW_EXPR_VAL_EXPRESSION) {
         reg_result = end_result;
      }
      return bresult;
   }
   else
   {
      err_result = FE_Bad_Frame_Data;
      return false;
   }
   
   assert(res_addr);
   bool bresult = false;
   if (word_size == 4) {
      uint32_t i;
      bresult = reader->ReadMem(res_addr, &i, word_size);
      reg_result = (Dyninst::MachRegisterVal) i;
   }
   else if (word_size == 8) {
      uint64_t i;
      bresult = reader->ReadMem(res_addr, &i, word_size);
      reg_result = (Dyninst::MachRegisterVal) i;
   }
   if (!bresult) {
      err_result = FE_Frame_Read_Error;
      return false;
   }

   return true;
}

void DwarfSW::setupFdeData()
{
   Dwarf_Error err;
   
   if (fde_dwarf_status == dwarf_status_ok || 
       fde_dwarf_status == dwarf_status_error)
      return;

   if (!dbg) {
      fde_dwarf_status = dwarf_status_error;
      return;
   }

#if defined(dwarf_has_setframe)
   dwarf_set_frame_cfa_value(dbg, DW_FRAME_CFA_COL3);
#endif

   fde_cie_data fc;
   int result = dwarf_get_fde_list(dbg, 
                                   &fc.cie_data, &fc.cie_count,
                                   &fc.fde_data, &fc.fde_count,
                                   &err);
   if (result == DW_DLV_OK) {
      fde_data.push_back(fc);
   }

   result = dwarf_get_fde_list_eh(dbg, 
                                  &fc.cie_data, &fc.cie_count,
                                  &fc.fde_data, &fc.fde_count,
                                  &err);
   if (result == DW_DLV_OK) {
      fde_data.push_back(fc);
   }
   

   if (!fde_data.size()) {
      fde_dwarf_status = dwarf_status_error;
   }
   
   fde_dwarf_status = dwarf_status_ok;
}
