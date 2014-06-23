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

#include "dwarf/h/dwarfFrameParser.h"
#include "dwarf/h/dwarfExprParser.h"
#include "dwarf/h/dwarfResult.h"
#include "common/h/VariableLocation.h"
#include "common/src/Types.h"
#include "libdwarf.h"
#include <stdio.h>
#include <iostream>
#include "common/src/debug_common.h" // dwarf_printf

using namespace Dyninst;
using namespace Dwarf;
using namespace std;

struct frameParser_key
{
  Dwarf_Debug dbg;
  Architecture arch;
  frameParser_key(Dwarf_Debug d, Architecture a) : dbg(d), arch(a) 
  {
  }
  
  bool operator< (const frameParser_key& rhs) const
  {
    return (dbg < rhs.dbg) || (dbg == rhs.dbg && arch < rhs.arch);
  }
  
};

  

std::map<DwarfFrameParser::frameParser_key, DwarfFrameParser::Ptr> DwarfFrameParser::frameParsers;

DwarfFrameParser::Ptr DwarfFrameParser::create(Dwarf_Debug dbg, Architecture arch) {
  frameParser_key k(dbg, arch);
  
  auto iter = frameParsers.find(k);
  if (iter == frameParsers.end()) {
    Ptr newParser = Ptr(new DwarfFrameParser(dbg, arch));
    frameParsers[k] = newParser;
    return newParser;
   }
   else {
      return iter->second;
   }
}


DwarfFrameParser::DwarfFrameParser(Dwarf_Debug dbg_, Architecture arch_) :
   dbg(dbg_),
   arch(arch_),
   fde_dwarf_status(dwarf_status_uninitialized)
{
}

DwarfFrameParser::~DwarfFrameParser()
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

bool DwarfFrameParser::hasFrameDebugInfo()
{
   setupFdeData();
   return fde_dwarf_status == dwarf_status_ok;
}

bool DwarfFrameParser::getRegValueAtFrame(Address pc, 
                                 Dyninst::MachRegister reg,
                                 Dyninst::MachRegisterVal &reg_result,
                                 ProcessReader *reader,
                                 FrameErrors_t &err_result)
{
   ConcreteDwarfResult cons(reader, arch, pc, dbg);

   dwarf_printf("Getting concrete value for %s at 0x%lx\n",
                reg.name().c_str(), pc);
   if (!getRegAtFrame(pc, reg, cons, err_result)) {
     assert(err_result != FE_No_Error);
     dwarf_printf("\t Returning error from getRegValueAtFrame\n");
     return false;
   }
   if (cons.err()) {
      dwarf_printf("\t Computed dwarf result to an error\n");
      err_result = FE_Frame_Eval_Error;
      return false;
   }

   reg_result = cons.val();
   dwarf_printf("Returning result 0x%lx for reg %s at 0x%lx\n", 
                reg_result, reg.name().c_str(), pc);
   return true;
}

bool DwarfFrameParser::getRegRepAtFrame(Address pc,
                               Dyninst::MachRegister reg,
                               VariableLocation &loc,
                               FrameErrors_t &err_result) {
   SymbolicDwarfResult cons(loc, arch);
   
   dwarf_printf("Getting symbolic value for %s at 0x%lx\n",
                reg.name().c_str(), pc);
   if (!getRegAtFrame(pc, reg, cons, err_result)) {
     dwarf_printf("\t Returning error from getRegRepAtFrame\n");
     assert(err_result != FE_No_Error);
     return false;
   }
   
   if (cons.err()) {
      dwarf_printf("\t Computed dwarf result to an error\n");
      err_result = FE_Frame_Eval_Error;
      return false;
   }

   loc = cons.val();

   dwarf_printf("Returning symbolic result for reg %s at 0x%lx\n", 
                reg.name().c_str(), pc);

   return true;
}

bool DwarfFrameParser::getRegsForFunction(Address entryPC,
                                          Dyninst::MachRegister reg,
                                          std::vector<VariableLocation> &locs,
                                          FrameErrors_t &err_result) {
   locs.clear();
   dwarf_printf("Entry to getRegsForFunction at 0x%lx, reg %s\n", entryPC, reg.name().c_str());
   err_result = FE_No_Error;

   /**
    * Initialize the FDE and CIE data.  This is only really done once, 
    * after which setupFdeData will immediately return.
    **/
   setupFdeData();
   if (!fde_data.size()) {
      err_result = FE_Bad_Frame_Data;
      dwarf_printf("\t No FDE data, returning error\n");
      return false;
   }

   /**
    * Get the FDE at this PC.  The FDE contains the rules for getting
    * registers at the given PC in this frame.
    **/
   Dwarf_Fde fde;
   Address low, high;
   if (!getFDE(entryPC, fde, low, high, err_result)) {
      dwarf_printf("\t Failed to find FDE for 0x%lx, returning error\n", entryPC);
     assert(err_result != FE_No_Error);
     return false;
   }

   dwarf_printf("\t Got FDE range 0x%lx..0x%lx\n", low, high);
   
   Dwarf_Half dwarf_reg;
   if (!getDwarfReg(reg, fde, dwarf_reg, err_result)) {
     assert(err_result != FE_No_Error);
     dwarf_printf("\t Failed to get dwarf register for %s\n", reg.name().c_str());
     return false;
   }

   Address worker = high;
   while (worker > low) {
      dwarf_printf("\t Getting register representation for 0x%lx (reg %s)\n", worker, reg.name().c_str());
      VariableLocation loc;
      SymbolicDwarfResult cons(loc, arch);
      Address next;
      if (!getRegAtFrame_aux(worker, fde, dwarf_reg, reg, cons, next, err_result)) {
	assert(err_result != FE_No_Error);
        dwarf_printf("\t Failed to get register representation, ret false\n");
         return false;
      }
      loc.lowPC = next;
      loc.hiPC = worker;

      worker = next - 1;
      locs.push_back(cons.val());
   }

   std::reverse(locs.begin(), locs.end());
   return true;
}

bool DwarfFrameParser::getRegAtFrame(Address pc,
                                     Dyninst::MachRegister reg,
                                     DwarfResult &cons,
                                     FrameErrors_t &err_result) {

   err_result = FE_No_Error;

   dwarf_printf("getRegAtFrame for 0x%lx, %s\n", pc, reg.name().c_str());
   /**
    * Initialize the FDE and CIE data.  This is only really done once, 
    * after which setupFdeData will immediately return.
    **/
   setupFdeData();
   if (!fde_data.size()) {
      dwarf_printf("\t No FDE data, ret false\n");
      err_result = FE_Bad_Frame_Data;
      return false;
   }

   /**
    * Get the FDE at this PC.  The FDE contains the rules for getting
    * registers at the given PC in this frame.
    **/
   Dwarf_Fde fde;
   Address u1, u2;
   if (!getFDE(pc, fde, u1, u2, err_result)) {
      dwarf_printf("\t No FDE at 0x%lx, ret false\n", pc);
      assert(err_result != FE_No_Error);
      return false;
   }

   Dwarf_Half dwarf_reg;
   if (!getDwarfReg(reg, fde, dwarf_reg, err_result)) {
      dwarf_printf("\t Failed to convert %s to dwarf reg, ret false\n",
                   reg.name().c_str());
      assert(err_result != FE_No_Error);
      return false;
   }

   Address ignored;
   return getRegAtFrame_aux(pc, fde, dwarf_reg, reg, cons, ignored, err_result);
}

bool DwarfFrameParser::getRegAtFrame_aux(Address pc,
                                         Dwarf_Fde fde,
                                         Dwarf_Half dwarf_reg,
                                         MachRegister orig_reg,
                                         DwarfResult &cons,
                                         Address &lowpc,
                                         FrameErrors_t &err_result) {
   int result;
   Dwarf_Error err;
   
   int width = getArchAddressWidth(arch);
   dwarf_printf("getRegAtFrame_aux for 0x%lx, addr width %d\n",
                pc, width);

   Dwarf_Small value_type;
   Dwarf_Signed offset_relevant, register_num, offset_or_block_len;
   Dwarf_Ptr block_ptr;
   Dwarf_Addr row_pc;

   /**
    * Decode the rule that describes how to get dwarf_reg at pc.
    **/
   if (dwarf_reg != DW_FRAME_CFA_COL3) {
      dwarf_printf("\tNot col3 reg, using default\n");
      result = dwarf_get_fde_info_for_reg3(fde, dwarf_reg, pc, &value_type, 
                                           &offset_relevant, &register_num,
                                           &offset_or_block_len,
                                           &block_ptr, &row_pc, &err);
   }
   else {
      dwarf_printf("\tcol3 reg, using CFA\n");
      result = dwarf_get_fde_info_for_cfa_reg3(fde, pc, &value_type, 
                                               &offset_relevant, &register_num,
                                               &offset_or_block_len,
                                               &block_ptr, &row_pc, &err);
   }
   if (result == DW_DLV_ERROR) {
      err_result = FE_Bad_Frame_Data;
      return false;
   }

   lowpc = (Address) row_pc;
   dwarf_printf("\t Got FDE data starting at 0x%lx; value_type %d, offset_relevant %d, offset/block len %d\n", 
                lowpc, (int) value_type, (int) offset_relevant, offset_or_block_len);

   /**
    * Interpret the rule and turn it into a real value.
    **/
   
   if (value_type == DW_EXPR_OFFSET || value_type == DW_EXPR_VAL_OFFSET)
   {
      
      bool done;
      dwarf_printf("\tHandling returned FDE as expression\n");
      if (!handleExpression(pc, register_num, orig_reg,
                            arch, cons, done, err_result)) {
         dwarf_printf("\t Error handling expression, ret false\n");
	assert(err_result != FE_No_Error);
	return false;
      }
      if (done) {
         dwarf_printf("\t Handled expression, indicated done, returning\n");
         return true;
      }
   }

   bool indirect = false;

   switch(value_type) {
      // For a val offset, the value of the register is (other_reg + const)
      case DW_EXPR_VAL_OFFSET:
      case DW_EXPR_OFFSET:
         dwarf_printf("\tHandling val_offset or offset\n");
         if (offset_relevant) {
            dwarf_printf("\t Offset relevant, adding %d\n", offset_or_block_len);
            cons.pushSignedVal(offset_or_block_len);
            cons.pushOp(DwarfResult::Add);
         }
         
         if (offset_relevant && 
             dwarf_reg != DW_FRAME_CFA_COL3) {
            dwarf_printf("\t Reg not CFA and offset relevant: indirect\n");
            indirect = true;
         }
         dwarf_printf("\t Done handling val_offset or offset\n");
         break;
      case DW_EXPR_VAL_EXPRESSION:
      case DW_EXPR_EXPRESSION: {
         dwarf_printf("\t Handling val_expression or expression\n");
         Dwarf_Locdesc *llbuf = NULL;
         Dwarf_Signed listlen = 0;
         result = dwarf_loclist_from_expr(dbg, block_ptr, offset_or_block_len, &llbuf, &listlen, &err);
         if (result != DW_DLV_OK) {
            dwarf_printf("\t Failed to get loclist, ret false\n");
            err_result = FE_Frame_Read_Error;
            return false;
         }
         
         if (!decodeDwarfExpression(llbuf, NULL, 
                                    cons,
                                    arch)) {
            dwarf_printf("\t Failed to decode dwarf expr, ret false\n");
            err_result = FE_Frame_Eval_Error;
            return false;
         }

         dwarf_dealloc(dbg, llbuf->ld_s, DW_DLA_LOC_BLOCK);
         dwarf_dealloc(dbg, llbuf, DW_DLA_LOCDESC);
         
         if (value_type == DW_EXPR_EXPRESSION) {
            dwarf_printf("\t Handling expression, adding indirect\n");
            indirect = true;
         }
         break;
      }
      default:
         err_result = FE_Bad_Frame_Data;
         return false;
   }
   
   if (indirect) {
      dwarf_printf("\t Adding a dereference to handle \"address of\" operator\n");
      cons.pushOp(DwarfResult::Deref, width);
   }

   return true;
}

void DwarfFrameParser::setupFdeData()
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

bool DwarfFrameParser::getFDE(Address pc, Dwarf_Fde &fde, 
                              Address &low, Address &high, 
                              FrameErrors_t &err_result) {
   Dwarf_Addr lowpc = 0, hipc = 0;
   dwarf_printf("Getting FDE for 0x%lx\n", pc);
   bool found = false;
   unsigned cur_fde;
   for (cur_fde=0; cur_fde<fde_data.size(); cur_fde++) {
      int result = dwarf_get_fde_at_pc(fde_data[cur_fde].fde_data, 
                                       (Dwarf_Addr) pc, &fde, &lowpc, &hipc, NULL);
      if (result == DW_DLV_ERROR) {
         dwarf_printf("\t Got ERROR return\n");
         err_result = FE_Bad_Frame_Data;
         return false;
      }
      else if (pc < lowpc || pc > hipc) 
      {
	continue;
      }
      else if (result == DW_DLV_OK) {
         dwarf_printf("\t Got range 0x%lx..0x%lx\n", 
                      lowpc, hipc);
         low = (Address) lowpc;
         high = (Address) hipc;
         found = true;
         break;
      }
   }
   if (!found)
   {
      dwarf_printf("\tEntry not found, ret false\n");
      err_result = FE_No_Frame_Entry;
      return false;
   }
   return true;
}

bool DwarfFrameParser::getDwarfReg(Dyninst::MachRegister reg,
                                   Dwarf_Fde &fde,
                                   Dwarf_Half &dwarf_reg,
                                   FrameErrors_t &err_result) {
   Dwarf_Error err;
   if (reg == Dyninst::ReturnAddr) {
      /**
       * We should be getting the return address for the stack frame.  
       * This is treated as a virtual register in the
       * FDE.  We can look up the virtual register in the CIE.
       **/
      Dwarf_Cie cie;
      int result = dwarf_get_cie_of_fde(fde, &cie, &err);
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
   else if (reg == Dyninst::FrameBase || reg == Dyninst::CFA) {
      dwarf_reg = DW_FRAME_CFA_COL3;
   }
   else {
      dwarf_reg = reg.getDwarfEnc(); 
   }
   return true;
}

bool DwarfFrameParser::handleExpression(Address pc,
                               Dwarf_Signed registerNum,
                               Dyninst::MachRegister origReg,
                               Dyninst::Architecture arch,
                               DwarfResult &cons,
                               bool &done,
                               FrameErrors_t &err_result) {
  dwarf_printf("HandleExpression\n");
  
   done = false;
   switch (registerNum) {
      case DW_FRAME_CFA_COL3:
	dwarf_printf("\t Getting frame base\n");
	
         err_result = FE_No_Error;
         if (!getRegAtFrame(pc, Dyninst::FrameBase,
                            cons, err_result)) {
	   assert(err_result != FE_No_Error);
            return false;
         }
         break;
      case DW_FRAME_SAME_VAL:
	dwarf_printf("\t Getting %s\n", origReg.name().c_str());
	
         cons.readReg(origReg);
         done = true;
         break;
      default: {
         Dyninst::MachRegister dyn_register = MachRegister::DwarfEncToReg(registerNum, arch);
	 dwarf_printf("\t Getting %s\n", dyn_register.name().c_str());
	 
         cons.readReg(dyn_register);
         break;
      }
   }
   return true;
}  

