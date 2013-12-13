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

#include <stack>
#include <stdio.h>
#include "common/h/dyn_regs.h"
#include "dwarf/h/dwarfExprParser.h"
#include "dwarf/h/dwarfResult.h"
#include "common/src/debug_common.h"
#include "common/h/VariableLocation.h"
#include "common/h/ProcReader.h"
#include "common/src/Types.h"

using namespace std;

namespace Dyninst {
namespace Dwarf {

bool decodeDwarfExpression(Dwarf_Locdesc *dwlocs,
                           long int *initialStackValue,
                           VariableLocation &loc,
                           Dyninst::Architecture arch) {
   SymbolicDwarfResult res(loc, arch);
   if (!decodeDwarfExpression(dwlocs, initialStackValue, 
                              res, arch)) return false;
   res.val();
   return true;
}

bool decodeDwarfExpression(Dwarf_Locdesc *dwlocs,
                           long int *initialStackValue,
                           ProcessReader *reader,
                           Address pc,
                           Dwarf_Debug dbg,
                           Dyninst::Architecture arch,
                           MachRegisterVal &end_result) {
   ConcreteDwarfResult res(reader, arch, pc, dbg);
   if (!decodeDwarfExpression(dwlocs, initialStackValue, 
                              res, arch)) return false;
   if (res.err()) return false;
   end_result = res.val();
   return true;
}



bool decodeDwarfExpression(Dwarf_Locdesc *dwlocs,
                           long int *initialStackValue,
                           DwarfResult &cons,
                           Dyninst::Architecture arch) {
   // This is basically a decode passthrough, with the work
   // being done by the DwarfResult. 

   dwarf_printf("Entry to decodeDwarfExpression\n");

   int addr_width = getArchAddressWidth(arch);
   if (initialStackValue != NULL) {
      dwarf_printf("\tInitializing expr stack with 0x%lx\n", initialStackValue);
      cons.pushUnsignedVal((Dyninst::MachRegisterVal) *initialStackValue);
   }

   Dwarf_Loc *locations = dwlocs->ld_s;
   unsigned count = dwlocs->ld_cents;
   for ( unsigned int i = 0; i < count; i++ ) 
   {
      dwarf_printf("\tAtom %d of %d: val 0x%x\n", i, count, locations[i].lr_atom);
      /* lit0 - lit31 : the constants 0..31 */
      if ( DW_OP_lit0 <= locations[i].lr_atom && locations[i].lr_atom <= DW_OP_lit31 ) 
      {
         dwarf_printf("\t\t Pushing unsigned val 0x%lx\n", locations[i].lr_atom - DW_OP_lit0);
         cons.pushUnsignedVal((Dyninst::MachRegisterVal) (locations[i].lr_atom - DW_OP_lit0));
         continue;
      }

      /* reg0 - reg31: named registers (not their constants) */
      if ( DW_OP_reg0 <= locations[i].lr_atom && locations[i].lr_atom <= DW_OP_reg31 ) 
      {
         dwarf_printf("\t\t Pushing reg %s\n",MachRegister::DwarfEncToReg(locations[i].lr_atom - DW_OP_reg0,
                                                                          arch).name().c_str());

         cons.pushReg(MachRegister::DwarfEncToReg(locations[i].lr_atom - DW_OP_reg0,
                                                  arch));
         continue;
      }

      /* breg0 - breg31: register contents plus an optional offset */
      if ( DW_OP_breg0 <= locations[i].lr_atom && locations[i].lr_atom <= DW_OP_breg31 ) 
      {
         dwarf_printf("\t\t Pushing reg %s + %d\n",MachRegister::DwarfEncToReg(locations[i].lr_atom - DW_OP_reg0,
                                                                               arch).name().c_str(),
                      locations[i].lr_number);
         cons.readReg(MachRegister::DwarfEncToReg(locations[i].lr_atom - DW_OP_breg0,
                                                  arch));
         cons.pushSignedVal((Dyninst::MachRegisterVal) locations[i].lr_number);
         cons.pushOp(DwarfResult::Add);
         continue;
      }

      switch( locations[i].lr_atom ) {
         // This is the same thing as breg, just with an encoded instead of literal
         // register.
         // The register is in lr_number
         // The offset is in lr_number2
         case DW_OP_bregx:
            dwarf_printf("\t\t Pushing reg %s + %d\n",MachRegister::DwarfEncToReg(locations[i].lr_number,
                                                                                  arch).name().c_str(),
                         locations[i].lr_number2);
            
            cons.readReg(MachRegister::DwarfEncToReg(locations[i].lr_number, arch));
            cons.pushSignedVal(locations[i].lr_number2);
            cons.pushOp(DwarfResult::Add);
            break;

         case DW_OP_regx:
            dwarf_printf("\t\t Pushing reg %s\n",MachRegister::DwarfEncToReg(locations[i].lr_number,
                                                                                  arch).name().c_str());
            cons.pushReg(MachRegister::DwarfEncToReg(locations[i].lr_number, arch));
            break;
            
         case DW_OP_nop:
            dwarf_printf("\t\t NOP\n");
            break;

         case DW_OP_addr:
         case DW_OP_const1u:
         case DW_OP_const2u:
         case DW_OP_const4u:
         case DW_OP_const8u:
         case DW_OP_constu:
            dwarf_printf("\t\t Pushing unsigned 0x%lx\n", locations[i].lr_number);
            cons.pushUnsignedVal(locations[i].lr_number);
            break;

         case DW_OP_const1s:
         case DW_OP_const2s:
         case DW_OP_const4s:
         case DW_OP_const8s:
         case DW_OP_consts:
            dwarf_printf("\t\t Pushing signed 0x%lx\n", locations[i].lr_number);
            cons.pushSignedVal(locations[i].lr_number);
            break;

         case DW_OP_fbreg:
            dwarf_printf("\t\t Pushing FB + 0x%lx\n", locations[i].lr_number);
            cons.pushFrameBase();
            cons.pushSignedVal(locations[i].lr_number);
            cons.pushOp(DwarfResult::Add);
            break;

         case DW_OP_dup: 
            dwarf_printf("\t\t Pushing dup\n");
           cons.pushOp(DwarfResult::Pick, 0);
            break;

         case DW_OP_drop:
            dwarf_printf("\t\t Pushing drop\n");
            cons.pushOp(DwarfResult::Drop, 0);
            break;

         case DW_OP_pick: 
            dwarf_printf("\t\t Pushing pick %d\n", locations[i].lr_number);
            cons.pushOp(DwarfResult::Pick, locations[i].lr_number);
            break;

         case DW_OP_over: 
            dwarf_printf("\t\t Pushing pick 1\n");
            cons.pushOp(DwarfResult::Pick, 1);
            break;

         case DW_OP_swap: 
            dwarf_printf("\t\t Pushing swap\n");
            cons.pushOp(DwarfResult::Pick, 1);
            cons.pushOp(DwarfResult::Drop, 2);
            break;

         case DW_OP_rot: 
            dwarf_printf("\t\t Pushing rotate\n");
            cons.pushOp(DwarfResult::Pick, 2);
            cons.pushOp(DwarfResult::Pick, 2);
            cons.pushOp(DwarfResult::Drop, 3);
            cons.pushOp(DwarfResult::Drop, 3);
            break;

         case DW_OP_deref:
            dwarf_printf("\t\t Pushing deref %d\n", addr_width);
            cons.pushOp(DwarfResult::Deref, addr_width);
            break;

         case DW_OP_deref_size:
            dwarf_printf("\t\t Pushing deref %d\n", locations[i].lr_number);
            cons.pushOp(DwarfResult::Deref, locations[i].lr_number);
            break;

         case DW_OP_call_frame_cfa:
            // This is a reference to the CFA as computed by stack walking information.
            // The current order is:
            // Variable: reference frame base (fbreg, above)
            // Frame base: reference CFA
            // CFA: offset from stack pointer
            dwarf_printf("\t\t Pushing CFA\n");
            cons.pushCFA();
            break;

         case DW_OP_abs:
            dwarf_printf("\t\t Pushing abs\n");
            cons.pushOp(DwarfResult::Abs);
            break;

         case DW_OP_and:
            dwarf_printf("\t\t Pushing and\n");
            cons.pushOp(DwarfResult::And);
            break;

         case DW_OP_div:
            dwarf_printf("\t\t Pushing div\n");
            cons.pushOp(DwarfResult::Div);
            break;

         case DW_OP_minus:
            dwarf_printf("\t\t Pushing sub\n");
            cons.pushOp(DwarfResult::Sub);
            break;

         case DW_OP_mod:
            cons.pushOp(DwarfResult::Mod);
            break;

         case DW_OP_mul:
            cons.pushOp(DwarfResult::Mul);
            break;

         case DW_OP_neg:
            cons.pushSignedVal(-1);
            cons.pushOp(DwarfResult::Mul);
            break;

         case DW_OP_not:
            cons.pushOp(DwarfResult::Not);
            break;

         case DW_OP_or:
            cons.pushOp(DwarfResult::Or);
            break;

         case DW_OP_plus:
            dwarf_printf("\t\t Pushing add\n");
            cons.pushOp(DwarfResult::Add);
            break;

         case DW_OP_plus_uconst:
            dwarf_printf("\t\t Pushing add 0x%x\n", locations[i].lr_number);

            cons.pushOp(DwarfResult::Add, locations[i].lr_number);
            break;

         case DW_OP_shl:
            cons.pushOp(DwarfResult::Shl);
            break;

         case DW_OP_shr:
            cons.pushOp(DwarfResult::Shr);
            break;
         
         case DW_OP_shra:
            cons.pushOp(DwarfResult::ShrArith);
            break;

         case DW_OP_xor:
            cons.pushOp(DwarfResult::Xor);
            break;

         case DW_OP_le:
            cons.pushOp(DwarfResult::LE);
            break;

         case DW_OP_ge:
            cons.pushOp(DwarfResult::GE);
            break;

         case DW_OP_eq:
            cons.pushOp(DwarfResult::Eq);
            break;

         case DW_OP_ne:
            cons.pushOp(DwarfResult::Neq);
            break;

         case DW_OP_lt:
            cons.pushOp(DwarfResult::LT);
            break;

         case DW_OP_gt:
            cons.pushOp(DwarfResult::GT);
            break;

         case DW_OP_bra: {
            // Conditional branch... 
            // It needs immediate evaluation so we can continue processing
            // the DWARF. 
            Dyninst::MachRegisterVal value;

            if (!cons.eval(value)) {
               // Error in dwarf, or we're doing static. I'm not worrying about
               // encoding a conditional branch in static eval right now. 
               return false;
            }
            
            if (value == 0) break;
            // Do not break; fall through to skip
         }
         case DW_OP_skip: {
            int bytes = (int)(Dwarf_Signed)locations[i].lr_number;
            unsigned int target = (unsigned int) locations[i].lr_offset + bytes;
            
            int j = i;
            if ( bytes < 0 ) {
               for ( j = i - 1; j >= 0; j-- ) {
                  if ( locations[j].lr_offset == target ) { break; }
               } /* end search backward */
            } else {
               for ( j = i + 1; j < dwlocs->ld_cents; j ++ ) {
                  if ( locations[j].lr_offset == target ) { break; }
               } /* end search forward */
            } /* end if positive offset */
            
            /* Because i will be incremented the next time around the loop. */
            i = j - 1; 
            break;
         }
      case DW_OP_piece:
	// Should detect multiple of these...
	continue;
         default:
            return false;
      } /* end operand switch */
   } /* end iteration over Dwarf_Loc entries. */

   return true;
}

}

}

#if 0

#if defined(arch_x86_64)

#define IA32_MAX_MAP 7
#define AMD64_MAX_MAP 15
static int const amd64_register_map[] =
  {
    0,  // RAX
    2,  // RDX
    1,  // RCX
    3,  // RBX
    6,  // RSI
    7,  // RDI
    5,  // RBP
    4,  // RSP
    8, 9, 10, 11, 12, 13, 14, 15    // gp 8 - 15
    /* This is incomplete. The x86_64 ABI specifies a mapping from
       dwarf numbers (0-66) to ("architecture number"). Without a
       corresponding mapping for the SVR4 dwarf-machine encoding for
       IA-32, however, it is not meaningful to provide this mapping. */
  };


int Dyninst::Register_DWARFtoMachineEnc32(int n)
{
   return n;
}


int Dyninst::Register_DWARFtoMachineEnc64(int n)
{
   if (n <= AMD64_MAX_MAP)
      return amd64_register_map[n];
   return n;
}

#endif

#endif



#if 0
bool Dyninst::decodeDwarfExpression(Dwarf_Locdesc *dwlocs,
                                    long int *initialStackValue,
                                    VariableLocation *loc, bool &isLocSet,
                                    ProcessReader *reader,
                                    Dyninst::Architecture arch,
                                    long int &end_result)
{
   /* Initialize the stack. */
   int addr_width = getArchAddressWidth(arch);
   std::stack< long int > opStack = std::stack<long int>();
   if ( initialStackValue != NULL ) { opStack.push( * initialStackValue ); }

   Dwarf_Loc *locations = dwlocs->ld_s;
   unsigned count = dwlocs->ld_cents;
   for ( unsigned int i = 0; i < count; i++ ) 
   {
      /* Handle the literals w/o 32 case statements. */
      if ( DW_OP_lit0 <= locations[i].lr_atom && locations[i].lr_atom <= DW_OP_lit31 ) 
      {
         dwarf_printf( "pushing named constant: %d\n", locations[i].lr_atom - DW_OP_lit0 );
         opStack.push( locations[i].lr_atom - DW_OP_lit0 );
         continue;
      }

      /* Haandle registers w/o 32 case statements. */
      if ( DW_OP_reg0 <= locations[i].lr_atom && locations[i].lr_atom <= DW_OP_reg31 ) 
      {
         /* storageReg is unimplemented, so do an offset of 0 from the named register instead. */
         dwarf_printf( "location is named register %d\n", 
                       DWARF_TO_MACHINE_ENC_W(locations[i].lr_atom - DW_OP_reg0, addr_width) );
         //loc->stClass = storageRegOffset;
         if (loc) 
         {
            loc->stClass = storageReg;
            loc->refClass = storageNoRef;
            loc->frameOffset = 0;
            loc->reg = DWARF_TO_MACHINE_ENC_W(locations[i].lr_atom - DW_OP_reg0, addr_width);
            loc->mr_reg = DWARF_TO_MACHINEREG_ENC_W(locations[i].lr_atom - DW_OP_reg0, addr_width);
            //loc->frameOffset = 0;
            isLocSet = true;
         }
         continue;
      }	

      /* Haandle registers w/o 32 case statements. */
      if ( DW_OP_breg0 <= locations[i].lr_atom && locations[i].lr_atom <= DW_OP_breg31 ) 
      {
         dwarf_printf( "setting storage class to named register, regNum to %d, offset %d\n", DWARF_TO_MACHINE_ENC_W(locations[i].lr_atom - DW_OP_breg0, addr_width), locations[i].lr_number );
         long int to_push = 0;
         if (loc) {
            loc->stClass = storageRegOffset;
            loc->refClass = storageNoRef;
            loc->frameOffset = locations[i].lr_number ;
            loc->reg = DWARF_TO_MACHINE_ENC_W(locations[i].lr_atom - DW_OP_breg0, addr_width);
            loc->mr_reg = DWARF_TO_MACHINEREG_ENC_W(locations[i].lr_atom - DW_OP_breg0, addr_width);
            to_push = static_cast<long int>(locations[i].lr_number);
         }
         else if (reader) {
            Dyninst::MachRegister r = DwarfToDynReg(locations[i].lr_atom - DW_OP_breg0,
                                                    arch);
            Dyninst::MachRegisterVal v;
            bool result = reader->GetReg(r, v);
            if (!result) {
               return false;
            }
            to_push = (long int) v + locations[i].lr_number;
         }
            
         opStack.push(to_push); 
         continue;
      }

      switch( locations[i].lr_atom ) 
      {
         case DW_OP_addr:
         case DW_OP_const1u:
         case DW_OP_const2u:
         case DW_OP_const4u:
         case DW_OP_const8u:
         case DW_OP_constu:
            dwarf_printf( "pushing unsigned constant %lu\n", 
                          (unsigned long)locations[i].lr_number );
            opStack.push(static_cast<long int>(locations[i].lr_number));
            break;

         case DW_OP_const1s:
         case DW_OP_const2s:
         case DW_OP_const4s:
         case DW_OP_const8s:
         case DW_OP_consts:
            dwarf_printf( "pushing signed constant %ld\n", 
                          (signed long)(locations[i].lr_number) );
            opStack.push(static_cast<long int>(locations[i].lr_number));
            break;

         case DW_OP_regx:
            /* storageReg is unimplemented, so do an offset of 0 from the named register instead. */
            dwarf_printf( "location is register %d\n", 
                          DWARF_TO_MACHINE_ENC_W(locations[i].lr_number, addr_width) );
            if (loc) {
               loc->stClass = storageReg;
               loc->refClass = storageNoRef;
               loc->reg = (int) DWARF_TO_MACHINE_ENC_W(locations[i].lr_number, addr_width); 
               loc->mr_reg = DWARF_TO_MACHINEREG_ENC_W(locations[i].lr_number, addr_width); 
               loc->frameOffset = 0;
               isLocSet = true;
            }
            break;

         case DW_OP_fbreg:
         {
            dwarf_printf( "setting storage class to frame base\n" );
            //if ( storageClass != NULL ) { * storageClass = storageFrameOffset; }
            long int to_push = 0;
            if (loc) {
               loc->stClass = storageRegOffset;
               loc->refClass = storageNoRef;
               loc->frameOffset = 0;
               loc->mr_reg = Dyninst::FrameBase;
               to_push = static_cast<long int>(locations[i].lr_number);
            }
            else if (reader) {
               Dyninst::MachRegister r = Dyninst::FrameBase;
               Dyninst::MachRegisterVal v;
               bool result = reader->GetReg(r, v);
               if (!result) {
                  return false;
               }
               to_push = (long int) v + locations[i].lr_number;               
            }
            opStack.push(to_push);
         } break;          
         case DW_OP_bregx: 
         {
            dwarf_printf( "setting storage class to register, regNum to %d\n", 
                          locations[i].lr_number );
            long int to_push = 0;
            if (loc) {
               loc->stClass = storageRegOffset;
               loc->refClass = storageNoRef;
               loc->reg = (int) DWARF_TO_MACHINE_ENC_W( locations[i].lr_number, addr_width );
               loc->mr_reg = DWARF_TO_MACHINEREG_ENC_W( locations[i].lr_number, addr_width );
               loc->frameOffset = 0;
               to_push = static_cast<long int>(locations[i].lr_number2);
            }
            else if (reader) {
               Dyninst::MachRegister r = DwarfToDynReg(locations[i].lr_number, arch);
               Dyninst::MachRegisterVal v;
               bool result = reader->GetReg(r, v);
               if (!result) {
                  return false;
               }
               to_push = (long int) v + locations[i].lr_number2;
            }
            opStack.push(to_push);
         } break;
         case DW_OP_dup:
            DWARF_FALSE_IF( opStack.size() < 1, 
                            "%s[%d]: invalid stack, returning false.\n", __FILE__, __LINE__ );
            opStack.push( opStack.top() );
            break;

         case DW_OP_drop:
            DWARF_FALSE_IF( opStack.size() < 1, 
                            "%s[%d]: invalid stack, returning false.\n", __FILE__, __LINE__ );
            opStack.pop();
            break;

         case DW_OP_pick: 
         {
            /* Duplicate the entry at index locations[i].lr_number. */
            std::stack< long int > temp = std::stack< long int >();
            
            for ( unsigned int j = 0; j < locations[i].lr_number; j++ ) 
            {
               temp.push( opStack.top() ); opStack.pop();
            }
            
            long int dup = opStack.top();
            
            for ( unsigned int j = 0; j < locations[i].lr_number; j++ ) 
            {
               opStack.push( temp.top() ); temp.pop();
            }
            
            opStack.push( dup );
         } break;

         case DW_OP_over: 
         {
            DWARF_FALSE_IF( opStack.size() < 2, 
                            "%s[%d]: invalid stack, returning false.\n", __FILE__, __LINE__ );
            long int first = opStack.top(); opStack.pop();
            long int second = opStack.top(); opStack.pop();
            opStack.push( second ); opStack.push( first ); opStack.push( second );
         } break;
         
         case DW_OP_swap: 
         {
            DWARF_FALSE_IF( opStack.size() < 2, 
                            "%s[%d]: invalid stack, returning false.\n", __FILE__, __LINE__ );
            long int first = opStack.top(); opStack.pop();
            long int second = opStack.top(); opStack.pop();
            opStack.push( first ); opStack.push( second );
         } break;
         
         case DW_OP_deref:
         {
            if (!reader)
               break;
            long int addr = opStack.top(); opStack.pop();
            unsigned long to_push = 0;
            bool bresult = false;
            if (addr_width == 4) {
               uint32_t v;
               bresult = reader->ReadMem(addr, &v, sizeof(v));
               to_push = (unsigned long) v;
            }
            else if (addr_width == 8) {
               uint64_t v;
               bresult = reader->ReadMem(addr, &v, sizeof(v));
               to_push = (unsigned long) v;
            }
            DWARF_FALSE_IF(!bresult,
                           "%s[%d]: Could not read from %lx\n", addr);
            opStack.push(to_push);
            break;
         }
         case DW_OP_deref_size:
         {
            if (!reader)
               break;
            long int addr = opStack.top(); opStack.pop();
            int width = locations[i].lr_number;
            unsigned long to_push = 0;
            bool bresult = false;
            if (width == 1) {
               uint8_t v;
               bresult = reader->ReadMem(addr, &v, sizeof(v));
               to_push = (unsigned long) v;
            }
            if (width == 2) {
               uint16_t v;
               bresult = reader->ReadMem(addr, &v, sizeof(v));
               to_push = (unsigned long) v;
            }
            if (width == 4) {
               uint32_t v;
               bresult = reader->ReadMem(addr, &v, sizeof(v));
               to_push = (unsigned long) v;
            }
            else if (width == 8) {
               uint64_t v;
               bresult = reader->ReadMem(addr, &v, sizeof(v));
               to_push = (long int) v;
            }
            DWARF_FALSE_IF(!bresult,
                           "%s[%d]: Could not read from %lx\n", addr);
            opStack.push(to_push);
            break;
         }

         case DW_OP_rot: 
         {
            DWARF_FALSE_IF( opStack.size() < 3, 
                            "%s[%d]: invalid stack, returning false.\n", __FILE__, __LINE__ );
            long int first = opStack.top(); opStack.pop();
            long int second = opStack.top(); opStack.pop();
            long int third = opStack.top(); opStack.pop();
            opStack.push( first ); opStack.push( third ); opStack.push( second );
         } break;

         case DW_OP_abs: 
         {
            DWARF_FALSE_IF( opStack.size() < 1, 
                            "%s[%d]: invalid stack, returning false.\n", __FILE__, __LINE__ );
            long int top = opStack.top(); opStack.pop();
            opStack.push( abs( top ) );
         } break;
         
         case DW_OP_and: 
         {
            DWARF_FALSE_IF( opStack.size() < 2, 
                            "%s[%d]: invalid stack, returning false.\n", __FILE__, __LINE__ );
            long int first = opStack.top(); opStack.pop();
            long int second = opStack.top(); opStack.pop();
            opStack.push( second & first );
         } break;

         case DW_OP_div: 
         {
            DWARF_FALSE_IF( opStack.size() < 2, 
                            "%s[%d]: invalid stack, returning false.\n", __FILE__, __LINE__ );
            long int first = opStack.top(); opStack.pop();
            long int second = opStack.top(); opStack.pop();
	    DWARF_FALSE_IF(((second != 0) && (first == 0)),
			   "%s[%d]: invalid stack, div operation with %d / %d, returning false\n",
			   __FILE__, __LINE__, second, first);
	    
	    if((second == 0) && (first == 0)) 
	      opStack.push(0);
	    else
	      opStack.push( second / first );
         } break;

         case DW_OP_minus: 
         {
            DWARF_FALSE_IF( opStack.size() < 2, 
                            "%s[%d]: invalid stack, returning false.\n", __FILE__, __LINE__ );
            long int first = opStack.top(); opStack.pop();
            long int second = opStack.top(); opStack.pop();
            opStack.push( second - first );
         } break;

         case DW_OP_mod: 
         {
            DWARF_FALSE_IF( opStack.size() < 2, 
                            "%s[%d]: invalid stack, returning false.\n", __FILE__, __LINE__ );
            long int first = opStack.top(); opStack.pop();
            long int second = opStack.top(); opStack.pop();
	    DWARF_FALSE_IF(((second != 0) && (first == 0)),
			   "%s[%d]: invalid stack, mod operation with %d mod %d, returning false\n",
			   __FILE__, __LINE__, second, first);
	    
	    if((second == 0) && (first == 0)) 
	      opStack.push(0);
	    else
	      opStack.push( second % first );
         } break;

         case DW_OP_mul: 
         {
            DWARF_FALSE_IF( opStack.size() < 2, 
                            "%s[%d]: invalid stack, returning false.\n", __FILE__, __LINE__ );
            long int first = opStack.top(); opStack.pop();
            long int second = opStack.top(); opStack.pop();
            opStack.push( second * first );
         } break;

         case DW_OP_neg: 
         {
            DWARF_FALSE_IF( opStack.size() < 1, 
                            "%s[%d]: invalid stack, returning false.\n", __FILE__, __LINE__ );
            long int first = opStack.top(); opStack.pop();
            opStack.push( first * (-1) );
         } break;
         
         case DW_OP_not: 
         {
            DWARF_FALSE_IF( opStack.size() < 1, 
                            "%s[%d]: invalid stack, returning false.\n", __FILE__, __LINE__ );
            long int first = opStack.top(); opStack.pop();
            opStack.push( ~ first );
         } break;

         case DW_OP_or: 
         {
            DWARF_FALSE_IF( opStack.size() < 2, 
                            "%s[%d]: invalid stack, returning false.\n", __FILE__, __LINE__ );
            long int first = opStack.top(); opStack.pop();
            long int second = opStack.top(); opStack.pop();
            opStack.push( second | first );
         } break;

         case DW_OP_plus: 
         {
            DWARF_FALSE_IF( opStack.size() < 2, 
                            "%s[%d]: invalid stack, returning false.\n", __FILE__, __LINE__ );
            long int first = opStack.top(); opStack.pop();
            long int second = opStack.top(); opStack.pop();
            opStack.push( second + first );
         } break;

         case DW_OP_plus_uconst: 
         {
            DWARF_FALSE_IF( opStack.size() < 1, 
                            "%s[%d]: invalid stack, returning false.\n", __FILE__, __LINE__ );
            long int first = opStack.top(); opStack.pop();
            opStack.push(static_cast<long int>(first + locations[i].lr_number));
         } break;
         
         case DW_OP_shl: 
         {
            DWARF_FALSE_IF( opStack.size() < 2, 
                            "%s[%d]: invalid stack, returning false.\n", __FILE__, __LINE__ );
            long int first = opStack.top(); opStack.pop();
            long int second = opStack.top(); opStack.pop();
            opStack.push( second << first );
         } break;

         case DW_OP_shr: 
         {
            DWARF_FALSE_IF( opStack.size() < 2, 
                            "%s[%d]: invalid stack, returning false.\n", __FILE__, __LINE__ );
            long int first = opStack.top(); opStack.pop();
            long int second = opStack.top(); opStack.pop();
            
            opStack.push( (long int)((unsigned long)second >> (unsigned long)first) );
         } break;
         
         case DW_OP_shra: 
         {
            DWARF_FALSE_IF( opStack.size() < 2, 
                            "%s[%d]: invalid stack, returning false.\n", __FILE__, __LINE__ );
            long int first = opStack.top(); opStack.pop();
            long int second = opStack.top(); opStack.pop();
            opStack.push( second >> first );
         } break;

         case DW_OP_xor: 
         {
            DWARF_FALSE_IF( opStack.size() < 2, 
                            "%s[%d]: invalid stack, returning false.\n", __FILE__, __LINE__ );
            long int first = opStack.top(); opStack.pop();
            long int second = opStack.top(); opStack.pop();
            opStack.push( second ^ first );
         } break;

         case DW_OP_le: 
         {
            DWARF_FALSE_IF( opStack.size() < 2, 
                            "%s[%d]: invalid stack, returning false.\n", __FILE__, __LINE__ );
            long int first = opStack.top(); opStack.pop();
            long int second = opStack.top(); opStack.pop();
            opStack.push( first <= second ? 1 : 0 );
         } break;

         case DW_OP_ge: 
         {
            DWARF_FALSE_IF( opStack.size() < 2, 
                            "%s[%d]: invalid stack, returning false.\n", __FILE__, __LINE__ );
            long int first = opStack.top(); opStack.pop();
            long int second = opStack.top(); opStack.pop();
            opStack.push( first >= second ? 1 : 0 );
         } break;

         case DW_OP_eq: 
         {
            DWARF_FALSE_IF( opStack.size() < 2, 
                            "%s[%d]: invalid stack, returning false.\n", __FILE__, __LINE__ );
            long int first = opStack.top(); opStack.pop();
            long int second = opStack.top(); opStack.pop();
            opStack.push( first == second ? 1 : 0 );
         } break;

         case DW_OP_lt: 
         {
            DWARF_FALSE_IF( opStack.size() < 2, 
                            "%s[%d]: invalid stack, returning false.\n", __FILE__, __LINE__ );
            long int first = opStack.top(); opStack.pop();
            long int second = opStack.top(); opStack.pop();
            opStack.push( first < second ? 1 : 0 );
         } break;

         case DW_OP_gt: 
         {
            DWARF_FALSE_IF( opStack.size() < 2, 
                            "%s[%d]: invalid stack, returning false.\n", __FILE__, __LINE__ );
            long int first = opStack.top(); opStack.pop();
            long int second = opStack.top(); opStack.pop();
            opStack.push( first > second ? 1 : 0 );
         } break;

         case DW_OP_ne: 
         {
            DWARF_FALSE_IF( opStack.size() < 2, 
                            "%s[%d]: invalid stack, returning false.\n", __FILE__, __LINE__ );
            long int first = opStack.top(); opStack.pop();
            long int second = opStack.top(); opStack.pop();
            opStack.push( first != second ? 1 : 0 );
         } break;

         case DW_OP_bra:
         {
            DWARF_FALSE_IF( opStack.size() < 1, 
                            "%s[%d]: invalid stack, returning false.\n", __FILE__, __LINE__ );
            if ( opStack.top() == 0 ) { break; }
            opStack.pop();
         }
         case DW_OP_skip: 
         {
            int bytes = (int)(Dwarf_Signed)locations[i].lr_number;
            unsigned int target = (unsigned int) locations[i].lr_offset + bytes;
            
            int j = i;
            if ( bytes < 0 ) {
               for ( j = i - 1; j >= 0; j-- ) {
                  if ( locations[j].lr_offset == target ) { break; }
               } /* end search backward */
            } else {
               for ( j = i + 1; j < dwlocs->ld_cents; j ++ ) {
                  if ( locations[j].lr_offset == target ) { break; }
               } /* end search forward */
            } /* end if positive offset */
            
            /* Because i will be incremented the next time around the loop. */
            i = j - 1;
         } break;

         case DW_OP_piece:
            /* For multi-part variables, which we don't handle. */
            //bperr ( "Warning: dyninst does not handle multi-part variables.\n" );
            break;

         case DW_OP_nop:
            break;

         case DW_OP_call_frame_cfa: {
            long int to_push = 0;
            if (loc) {
               loc->stClass = storageRegOffset;
               loc->refClass = storageNoRef;
               loc->frameOffset = 0;
               loc->mr_reg = Dyninst::FrameBase;
            }
            else if (reader) {
               Dyninst::MachRegister r = Dyninst::FrameBase;
               Dyninst::MachRegisterVal v;
               bool result = reader->GetReg(r, v);
               if (!result) {
                  return false;
               }
            }
            opStack.push(to_push);
            break;
         }
         default:
            printf( "Unrecognized or non-static location opcode 0x%x, aborting.\n", locations[i].lr_atom );
            return false;
            break;
      } /* end operand switch */
   } /* end iteration over Dwarf_Loc entries. */
   
   if (opStack.empty()) {
      dwarf_printf( "ignoring malformed location list (stack empty at end of list).\n" );
      return isLocSet;
   }
   dwarf_printf( "Dwarf expression returning %d\n", opStack.top() );
   end_result = opStack.top();
   return true;
}

#endif

