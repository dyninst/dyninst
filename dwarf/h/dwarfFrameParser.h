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

#if !defined(DWARF_SW_H_)
#define DWARF_SW_H_

#include <stack>
#include <vector>
#include "dyntypes.h"
#include "dyn_regs.h"
#include "ProcReader.h"
#include "libdwarf.h"
#include "util.h"

namespace Dyninst {

class VariableLocation;

namespace Dwarf {
   class DwarfResult;
typedef enum {
   FE_Bad_Frame_Data = 15,   /* to coincide with equivalent SymtabError */
   FE_No_Frame_Entry,
   FE_Frame_Read_Error,
   FE_Frame_Eval_Error,
   FE_No_Error
} FrameErrors_t;

typedef struct {
  Dwarf_Fde *fde_data;
  Dwarf_Signed fde_count;
  Dwarf_Cie *cie_data;
  Dwarf_Signed cie_count;   
} fde_cie_data;


class DYNDWARF_EXPORT DwarfFrameParser {
  public:

   typedef boost::shared_ptr<DwarfFrameParser> Ptr;

   static Ptr create(Dwarf_Debug dbg, Architecture arch);

   DwarfFrameParser(Dwarf_Debug dbg_, Architecture arch);
   ~DwarfFrameParser();

   bool hasFrameDebugInfo();
   
   bool getRegRepAtFrame(Address pc,
                         MachRegister reg,
                         VariableLocation &loc,
                         FrameErrors_t &err_result);

   bool getRegValueAtFrame(Address pc, 
                           MachRegister reg, 
                           MachRegisterVal &reg_result,
                           ProcessReader *reader,
                           FrameErrors_t &err_result);

   bool getRegAtFrame(Address pc,
                      MachRegister reg,
                      DwarfResult &cons,
                      FrameErrors_t &err_result);

   // Returns whatever Dwarf claims the function covers. 
   // We use an entryPC (actually, can be any PC in the function)
   // because common has no idea what a Function is and I don't want
   // to move this to Symtab. 
   bool getRegsForFunction(Address entryPC,
                           MachRegister reg,
                           std::vector<VariableLocation> &locs,
                           FrameErrors_t &err_result);


  private:

   bool getRegAtFrame_aux(Address pc,
                          Dwarf_Fde fde,
                          Dwarf_Half dwarf_reg,
                          MachRegister orig_reg,
                          DwarfResult &cons,
                          Address &lowpc,
                          FrameErrors_t &err_result);

   bool getFDE(Address pc, 
               Dwarf_Fde &fde, 
               Address &low,
               Address &high,
               FrameErrors_t &err_result);

   bool getDwarfReg(MachRegister reg,
                    Dwarf_Fde &fde,
                    Dwarf_Half &dwarf_reg,
                    FrameErrors_t &err_result);
   
   bool handleExpression(Address pc,
                         Dwarf_Signed registerNum,
                         MachRegister origReg,
                         Architecture arch,
                         DwarfResult &cons,
                         bool &done,
                         FrameErrors_t &err_result);
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
   static std::map<frameParser_key, Ptr> frameParsers;

   typedef enum {
      dwarf_status_uninitialized,
      dwarf_status_error,
      dwarf_status_ok
   } dwarf_status_t;
   Dwarf_Debug dbg;
   Architecture arch;
   dwarf_status_t fde_dwarf_status;

   std::vector<fde_cie_data> fde_data;
   void setupFdeData();



};

}

}
#endif
