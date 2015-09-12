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
#define INVALID_ENTRY aarch64_entry(aarch64_op_INVALID, "INVALID", NULL, operandSpec())

bool aarch64_entry::built_tables = false;

std::vector<aarch64_entry> aarch64_entry::aarch64_insn_table;
aarch64_table aarch64_entry::main_opcode_table;
aarch64_table aarch64_entry::ext_op_GroupDiBSys;

void aarch64_entry::buildTables()
{
    if(built_tables) return;
    main_opcode_table[0] = INVALID_ENTRY; //00
    // TMP invalid for now
    main_opcode_table[1] = INVALID_ENTRY; //01
    main_opcode_table[3] = INVALID_ENTRY; //11
    // end TMP
    main_opcode_table[2] = aarch64_entry(aarch64_op_extended, "DataImm_Branch_Sys", fn(ext_op_DiBSys), operandSpec()); //10, Data imm, Branch, Exception and System insns


    // Groups:
    // Group for Data imm, Branch, and System
    ext_op_GroupDiBSys[0] = aarch64_entry(aarch64_op_add, "add", NULL, list_of(fn(Rd))(fn(Rn))(fn(Imm12)));
    ext_op_GroupDiBSys[1] = aarch64_entry(aarch64_op_sub, "sub", NULL, list_of(fn(Rd))(fn(Rn))(fn(Imm12)));


    built_tables = true;
}
