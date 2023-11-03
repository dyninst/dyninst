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
#include "Architecture.h"
#include "registers/MachRegister.h"
#include "dwarfExprParser.h"
#include "dwarfResult.h"
#include "debug_common.h"
#include "VariableLocation.h"
#include "ProcReader.h"
#include "compiler_annotations.h"

using namespace std;

namespace Dyninst {
namespace DwarfDyninst {

bool decodeDwarfExpression(Dwarf_Op * expr,
        Dwarf_Sword listlen,
        long int *initialStackValue,
        VariableLocation &loc,
        Dyninst::Architecture arch) {
    SymbolicDwarfResult res(loc, arch);
    if (!decodeDwarfExpression(expr, listlen, initialStackValue, 
                res, arch)) return false;
    res.val();
    return true;
}

bool decodeDwarfExpression(Dwarf_Op * expr,
        Dwarf_Sword listlen,
        long int *initialStackValue,
        DwarfResult &cons,
        Dyninst::Architecture arch) {
    // This is basically a decode passthrough, with the work
    // being done by the DwarfResult. 

    dwarf_printf("Entry to decodeDwarfExpression\n");

    int addr_width = getArchAddressWidth(arch);
    if (initialStackValue != NULL) {
        dwarf_printf("\tInitializing expr stack with 0x%p\n", (void*)initialStackValue);
        cons.pushUnsignedVal((Dyninst::MachRegisterVal) *initialStackValue);
    }

    Dwarf_Op *locations = expr;
    unsigned count = listlen;
    for ( unsigned int i = 0; i < count; i++ ) 
    {
        dwarf_printf("\tAtom %u of %u: val 0x%x\n", i+1, count, locations[i].atom);
        /* lit0 - lit31 : the constants 0..31 */
        if ( DW_OP_lit0 <= locations[i].atom && locations[i].atom <= DW_OP_lit31 ) 
        {
            dwarf_printf("\t\t Pushing unsigned val 0x%lx\n", (unsigned long)(locations[i].atom - DW_OP_lit0));
            cons.pushUnsignedVal((Dyninst::MachRegisterVal) (locations[i].atom - DW_OP_lit0));
            continue;
        }

        /* reg0 - reg31: named registers (not their constants) */
        if ( DW_OP_reg0 <= locations[i].atom && locations[i].atom <= DW_OP_reg31 ) 
        {
            dwarf_printf("\t\t Pushing reg %s\n",MachRegister::DwarfEncToReg(locations[i].atom - DW_OP_reg0,
                        arch).name().c_str());

            cons.pushReg(MachRegister::DwarfEncToReg(locations[i].atom - DW_OP_reg0,
                        arch));
            continue;
        }

        /* breg0 - breg31: register contents plus an optional offset */
        if ( DW_OP_breg0 <= locations[i].atom && locations[i].atom <= DW_OP_breg31 ) 
        {
            dwarf_printf("\t\t Pushing reg %s + %lu\n",MachRegister::DwarfEncToReg(locations[i].atom - DW_OP_reg0,
                        arch).name().c_str(),
                    locations[i].number);
            cons.readReg(MachRegister::DwarfEncToReg(locations[i].atom - DW_OP_breg0,
                        arch));
            cons.pushSignedVal((Dyninst::MachRegisterVal) locations[i].number);
            cons.pushOp(DwarfResult::Add);
            continue;
        }

        switch( locations[i].atom ) {
            // This is the same thing as breg, just with an encoded instead of literal
            // register.
            // The register is in number
            // The offset is in number2
            case DW_OP_bregx:
                dwarf_printf("\t\t Pushing reg %s + %lu\n",MachRegister::DwarfEncToReg(locations[i].number,
                            arch).name().c_str(),
                        locations[i].number2);

                cons.readReg(MachRegister::DwarfEncToReg(locations[i].number, arch));
                cons.pushSignedVal(locations[i].number2);
                cons.pushOp(DwarfResult::Add);
                break;

            case DW_OP_regx:
                dwarf_printf("\t\t Pushing reg %s\n",MachRegister::DwarfEncToReg(locations[i].number,
                            arch).name().c_str());
                cons.pushReg(MachRegister::DwarfEncToReg(locations[i].number, arch));
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
                dwarf_printf("\t\t Pushing unsigned 0x%lx\n", locations[i].number);
                cons.pushUnsignedVal(locations[i].number);
                break;

            case DW_OP_const1s:
            case DW_OP_const2s:
            case DW_OP_const4s:
            case DW_OP_const8s:
            case DW_OP_consts:
                dwarf_printf("\t\t Pushing signed 0x%lx\n", locations[i].number);
                cons.pushSignedVal(locations[i].number);
                break;

            case DW_OP_fbreg:
                dwarf_printf("\t\t Pushing FB + 0x%lx\n", locations[i].number);
                cons.pushFrameBase();
                cons.pushSignedVal(locations[i].number);
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
                dwarf_printf("\t\t Pushing pick %lu\n", locations[i].number);
                cons.pushOp(DwarfResult::Pick, locations[i].number);
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
                dwarf_printf("\t\t Pushing deref %lu\n", locations[i].number);
                cons.pushOp(DwarfResult::Deref, locations[i].number);
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
                dwarf_printf("\t\t Pushing add 0x%lu\n", locations[i].number);
                cons.pushOp(DwarfResult::Add, locations[i].number);
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

            case DW_OP_bra: 
                {
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
                }
		DYNINST_FALLTHROUGH;
            case DW_OP_skip: 
                {
                    int bytes = (int)(Dwarf_Sword)locations[i].number;
                    unsigned int target = (unsigned int) locations[i].offset + bytes;

                    unsigned int j = i;
                    if ( bytes < 0 ) {
                        for ( j = i - 1; /*FIXME j >= 0*/; j-- ) {
                            if ( locations[j].offset == target ) { break; }
                        } /* end search backward */
                    } else {
                        for ( j = i + 1; j < count; j ++ ) {
                            if ( locations[j].offset == target ) { break; }
                        } /* end search forward */
                    } /* end if positive offset */

                    /* Because i will be incremented the next time around the loop. */
                    i = j - 1; 
                    break;
                }
            case DW_OP_piece:
                // Should detect multiple of these...
                continue;

            case DW_OP_stack_value:
                // the value is at the top of the stack
                continue;

            case DW_OP_entry_value:
            case DW_OP_GNU_entry_value:
                dwarf_printf("\t\t skipping GNU_entry_value\n");
                return false;

            case DW_OP_convert:
            case DW_OP_GNU_convert:

                dwarf_printf("\t\t skipping GNU_convert\n");
                return false;

            case DW_OP_implicit_pointer:
            case DW_OP_GNU_implicit_pointer:
                dwarf_printf("\t\t skipping GNU_implicit_pointer\n");
                return false;

            default:
                dwarf_printf("\t\t error: unrecognized dwarf operation 0x%d\n", locations[i].atom);
                return false;
        } /* end operand switch */
    } /* end iteration over Dwarf_Op entries. */

    return true;
}

}

}

