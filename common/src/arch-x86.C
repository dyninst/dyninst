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

// $Id: arch-x86.C,v 1.5 2008/09/04 21:06:48 bill Exp $

// Official documentation used:    - IA-32 Intel Architecture Software Developer Manual (2001 ed.)
//                                 - AMD x86-64 Architecture Programmer's Manual (rev 3.00, 1/2002)
// Unofficial documentation used:  - www.sandpile.org/ia32
//                                 - NASM documentation
// Note: Unless specified "book" refers to Intel's manual

/**
 *
 * Notes on Intel decoding and where to start:
 *
 * The descriptions for the instructions in the Intel instruction set are defined
 * in a struct in arch-x86.h. The struct can be summarized as follows:
 *
 * 1. id (Entry ID)
 *
 * This is used to identify the instruction. Multiple rows can have the same ID
 * if they have the same instruction mnemonic. This ID should map with an entry
 * in the dyn_hash_map which stores the actual string for the instruction
 * mnemonics.
 *
 * 2. otable (Next Opcode Table)
 *
 * This is used during the decoding process. It the identifier for the next
 * table that should be used in the decoding process. This is explained more
 * in ia32_decode_opcode.
 *
 * 3. tabidx (Opcode Table Index)
 *
 * This is also used during the decoding process and specifies the index for the
 * next table that should be used. Depending on the table, this value is ignored
 * and other logic is used to make the decision as to which table index should
 * be used next.
 *
 * 4. hasModRM (Whether or not this instruction has a ModR/M byte)
 *
 * This designates whether or not the instruction being described has a ModR/M
 * byte or not. NOTE: This MUST be set to true for instructions that have operands
 * that use the ModR/M byte even if you specify it has operands that use the
 * ModR/M byte.
 *
 * 5. operands[3] (Instruction Operands)
 *
 * This is an array of descriptors for the first 3 operands. Please look at the
 * Intel manual to see which addressing modes and operand sizes are available.
 * We follow the same format as the Intel manual except for a couple of very
 * rare cases.
 *
 * 6. legacyType (Legacy information)
 *
 * This is generally used for dataflow analysis and other semantic information. You
 * shouldn't have to mess with this assuming that intel doesn't add any new instructions
 * that are capable of changing the RIP/EIP like jumps, calls, ret, ect.
 *
 * 7. opsema (Operand Read/Write Semantics)
 *
 * This describes which operands and read and written. Search arch-x86.h for
 * 'operand semantic' and you should be able to find the options for this.
 *
 * 8. impl_dec (Implicit Operand Description)
 *
 * This is a mask that should be used to mark implicit operands. If an operand
 * should not be printed in AT&T syntax, then you should mask it here.
 *
 * The Decoding Process
 * --------------------
 *
 * The main decoding function is ia32_decode, which calls a bunch of helper functions
 * that are for the most part extremely well commented. The overall flow of the
 * decoding process is like this:
 *
 * 1. Decode any prefixes
 *
 *      This step is really easy. When we first look at an instruction, we have
 * to decode which bytes are prefix bytes. Usually, there is only one prefix,
 * however some instructions can have 2 or more prefixes. All VEX instructions
 * can only have one prefix: The VEX2, VEX3 or EVEX prefix.
 *
 * 2. Decode the opcode and determine the instruction description
 *
 *      In this step we start at some initial decoding table. The starting table
 * is determined by which prefixes are present. If there are no prefixes present,
 * we start in the twoByteMap. We can go through several tables in the decoding
 * process and the logic for each table is a bit different.
 *
 * 3. Decode operands
 *
 *      In this step we look at the description we got from step 2. The operand
 * descriptions in the ia32_entry entry are used to determine what to look for
 * and how long the final instruction length should be.
 *
 * The main objective of the decoding process is to get the length of the instruction
 * correct. Because x86 has variable length instructions, getting the instruction
 * length wrong will mess up the decoding for the instructions that follow.
 *
 *
 */


// This include *really must* come first in the file.
#include "common/src/vgannotations.h"

#include <assert.h>
#include <stdio.h>
#include <map>
#include <string>
#include <iostream>
#include <mutex> // once_flag, call_once

#include "boost/assign/list_of.hpp"
#include "boost/assign/std/vector.hpp"
#include "boost/assign/std/set.hpp"
#include <boost/thread/lock_guard.hpp>
#include <boost/thread/mutex.hpp>
#include "common/src/arch-x86.h"
#include "registers/x86_regs.h"
#include "registers/x86_64_regs.h"
#include "compiler_annotations.h"
#include "unaligned_memory_access.h"
#include <cstdint>

// #define VEX_DEBUG
// #define VEX_PEDANTIC

using namespace std;
using namespace boost::assign;
using namespace Dyninst;

namespace NS_x86 {

unsigned int swapBytesIfNeeded(unsigned int i)
{
    return i;
}

// groups
enum {
  Grp1a=0, Grp1b, Grp1c, Grp1d, Grp2, Grp3a, Grp3b, Grp4, Grp5, Grp6, Grp7,
  Grp8, Grp9, Grp11, Grp12, Grp13, Grp14, Grp15, Grp16, Grp17, GrpAMD
};

// SSE
/** START_DYNINST_TABLE_DEF(sse_table, SSE, NO) */
enum {
  SSE10 = 0, SSE11, SSE12, SSE13, SSE14, SSE15, SSE16, SSE17, 
  SSE28, SSE29, SSE2A, SSE2B, SSE2C, SSE2D, SSE2E, SSE2F,
         SSE41, SSE42,        SSE44, SSE45, SSE46, SSE47,
  SSE4A, SSE4B,
  SSE50, SSE51, SSE52, SSE53, SSE54, SSE55, SSE56, SSE57,
  SSE58, SSE59, SSE5A, SSE5B, SSE5C, SSE5D, SSE5E, SSE5F,
  SSE60, SSE61, SSE62, SSE63, SSE64, SSE65, SSE66, SSE67,
  SSE68, SSE69, SSE6A, SSE6B, SSE6C, SSE6D, SSE6E, SSE6F,
  SSE70, SSE71, SSE72, SSE73, SSE74, SSE75, SSE76, SSE77,
  SSE78, SSE79, SSE7A, SSE7B, SSE7C, SSE7D, SSE7E, SSE7F,
  SSE90, SSE91, SSE92, SSE93,
  SSE98, SSE99,
  SSEB8,                             SSEBD,
                SSEC2, SSEC4, SSEC5, SSEC6,
  SSED0, SSED1, SSED2, SSED3, SSED4, SSED5, SSED6, SSED7,
  SSED8, SSED9, SSEDA, SSEDB, SSEDC, SSEDD, SSEDE, SSEDF,
  SSEE0, SSEE1, SSEE2, SSEE3, SSEE4, SSEE5, SSEE6, SSEE7,
  SSEE8, SSEE9, SSEEA, SSEEB, SSEEC, SSEED, SSEEE, SSEEF,
  SSEF0, SSEF1, SSEF2, SSEF3, SSEF4, SSEF5, SSEF6, SSEF7,
  SSEF8, SSEF9, SSEFA, SSEFB, SSEFC, SSEFD, SSEFE, SSEFF
};
/** END_DYNINST_TABLE_DEF */

/** Table that multiplexes between VEX and non VEX sse instructions */
/** START_DYNINST_TABLE_DEF(sse_vex_mult_table, SSEVEX, NO) */
enum {
    SSEVEX41 = 0, SSEVEX42, SSEVEX44, SSEVEX45, SSEVEX46, SSEVEX47,
    SSEVEX4A, SSEVEX4B,
    SSEVEX73, 
    SSEVEX78,
    SSEVEX90, SSEVEX91, SSEVEX92, SSEVEX93, 
    SSEVEX98, SSEVEX99
};
/** END_DYNINST_TABLE_DEF*/

// SSE VEX multiplexing table
/** START_DYNINST_TABLE_DEF(sse_vex_table, SSE, YES) */
enum { /** AUTOGENERATED */
  SSE10_66 = 0, SSE10_F2, SSE10_F3, SSE10_NO,
  SSE12_F2,           SSE12_F3, SSE12_NO,
  SSE13_66,                     SSE13_NO,
  SSE14_66,                     SSE14_NO,
  SSE15_66,                     SSE15_NO,
  SSE16_66,           SSE16_F3, SSE16_NO,
  SSE28_66,                     SSE28_NO,
  SSE2A_F2,           SSE2A_F3,
  SSE2B_66,                     SSE2B_NO,
  SSE2C_F2,           SSE2C_F3,
  SSE2D_F2,           SSE2D_F3,
  SSE2E_66,                     SSE2E_NO,
  SSE2F_66,                     SSE2F_NO,
  SSE41_66,                     SSE41_NO,
  SSE42_66,                     SSE42_NO,
  SSE44_66,                     SSE44_NO,
  SSE45_66,                     SSE45_NO,
  SSE46_66,                     SSE46_NO,
  SSE47_66,                     SSE47_NO,
  SSE4A_66,                     SSE4A_NO,
  SSE4B_66,                     SSE4B_NO,
  SSE51_66, SSE51_F2, SSE51_F3, SSE51_NO,
  SSE54_66,                     SSE54_NO,
  SSE55_66,                     SSE55_NO,
  SSE56_66,                     SSE56_NO,
  SSE57_66,                     SSE57_NO,
  SSE58_66, SSE58_F2, SSE58_F3, SSE58_NO,
  SSE59_66, SSE59_F2, SSE59_F3, SSE59_NO,
  SSE5A_66, SSE5A_F2, SSE5A_F3, SSE5A_NO,
  SSE5B_66, SSE5B_F3,           SSE5B_NO,
  SSE5C_66, SSE5C_F2, SSE5C_F3, SSE5C_NO,
  SSE5D_66, SSE5D_F2, SSE5D_F3, SSE5D_NO,
  SSE5E_66, SSE5E_F2, SSE5E_F3, SSE5E_NO,
  SSE5F_66, SSE5F_F2, SSE5F_F3, SSE5F_NO,
  SSE60_66,
  SSE61_66,
  SSE62_66,
  SSE63_66,
  SSE64_66,
  SSE65_66,
  SSE66_66,
  SSE67_66,
  SSE68_66,
  SSE69_66,
  SSE6A_66,
  SSE6B_66,
  SSE6C_66,
  SSE6D_66,
  SSE6F_66, SSE6F_F2, SSE6F_F3,
  SSE70_66, SSE70_F2, SSE70_F3,
  SSE71_66,
  SSE72_66,
  SSE73_66,
  SSE74_66,
  SSE75_66,
  SSE76_66,
  SSE78_66, SSE78_F2, SSE78_F3, SSE78_NO,
  SSE79_66, SSE79_F2, SSE79_F3, SSE79_NO,
  SSE7A_66, SSE7A_F2, SSE7A_F3,
  SSE7B_66, SSE7B_F2, SSE7B_F3,
                      SSE7E_F3,
  SSE7F_66,
  SSE90_66,                     SSE90_NO,
  SSE91_66,                     SSE91_NO,
                                SSE92_NO,
  SSE93_66, SSE93_F2,           SSE93_NO,
  SSE98_66,                     SSE98_NO,
  SSE99_66,                     SSE99_NO,
  SSEC2_66, SSEC2_F2, SSEC2_F3, SSEC2_NO,
  SSEC4_66,
  SSEC5_66,
  SSEC6_66,                     SSEC6_NO,
  SSED1_66,
  SSED2_66,
  SSED3_66,
  SSED4_66,
  SSED5_66,
  SSED8_66,
  SSED9_66,
  SSEDA_66,
  SSEDB_66,
  SSEDC_66,
  SSEDD_66,
  SSEDE_66,
  SSEDF_66,
  SSEE0_66,
  SSEE1_66,
  SSEE2_66,
  SSEE3_66,
  SSEE4_66,
  SSEE5_66,
  SSEE6_66, SSEE6_F2, SSEE6_F3,
  SSEE7_66,
  SSEE8_66,
  SSEE9_66,
  SSEEA_66,
  SSEEB_66,
  SSEEC_66,
  SSEED_66,
  SSEEE_66,
  SSEEF_66,
  SSEF1_66,
  SSEF2_66,
  SSEF3_66,
  SSEF4_66,
  SSEF5_66,
  SSEF6_66,
  SSEF8_66,
  SSEF9_66,
  SSEFA_66,
  SSEFB_66,
  SSEFC_66,
  SSEFD_66,
  SSEFE_66,
};
/** END_DYNINST_TABLE_DEF */

// SSE BIS
/** START_DYNINST_TABLE_DEF(sse_bis_table, SSEB, YES) */
enum {
SSEB00 = 0, SSEB01, SSEB02, SSEB03, SSEB04, SSEB05, SSEB06, SSEB07,
    SSEB08, SSEB09,	SSEB0A, SSEB0B, SSEB0C, SSEB0D, SSEB0E, SSEB0F,
    SSEB10, SSEB11, SSEB12, SSEB13, SSEB14, SSEB15, SSEB16, SSEB17,
    SSEB18, SSEB19, SSEB1A, SSEB1B, SSEB1C, SSEB1D, SSEB1E, SSEB1F,
	SSEB20, SSEB21, SSEB22, SSEB23, SSEB24, SSEB25, SSEB26, SSEB27,
	SSEB28, SSEB29, SSEB2A, SSEB2B, SSEB2C, SSEB2D, SSEB2E, SSEB2F,
	SSEB30, SSEB31, SSEB32, SSEB33, SSEB34, SSEB35, SSEB36, SSEB37,
	SSEB38, SSEB39,	SSEB3A, SSEB3B, SSEB3C, SSEB3D, SSEB3E, SSEB3F,
	SSEB40, SSEB41, SSEB42, SSEB43, SSEB44, SSEB45, SSEB46, SSEB47,
                                    SSEB4C, SSEB4D, SSEB4E, SSEB4F,
    SSEB58, SSEB59, SSEB5A,
                                            SSEB65, SSEB66,
                                            SSEB75, SSEB76, SSEB77,
    SSEB78, SSEB79,                 SSEB7C, SSEB7D, SSEB7E, SSEB7F,
                            SSEB83,
    SSEB88, SSEB89,         SSEB8B, SSEB8C, SSEB8D, SSEB8E,
    SSEB90, SSEB91, SSEB92, SSEB93,
                                                    SSEB96, SSEB97,
    SSEB98, SSEB99, SSEB9A, SSEB9B, SSEB9C, SSEB9D, SSEB9E, SSEB9F,
    SSEBA0, SSEBA1, SSEBA2, SSEBA3,                 SSEBA6, SSEBA7,
    SSEBA8, SSEBA9, SSEBAA, SSEBAB, SSEBAC, SSEBAD, SSEBAE, SSEBAF,
                                    SSEBB4, SSEBB5, SSEBB6, SSEBB7,
    SSEBB8, SSEBB9, SSEBBA, SSEBBB, SSEBBC, SSEBBD, SSEBBE, SSEBBF,
                                    SSEBC4,         SSEBC6, SSEBC7,
    SSEBC8,         SSEBCA, SSEBCB, SSEBCC, SSEBCD,
                            SSEBDB, SSEBDC, SSEBDD, SSEBDE, SSEBDF,
    SSEBF0, SSEBF1, SSEBF2,                 SSEBF5, SSEBF6, SSEBF7
};
/** END_DYNINST_TABLE_DEF */


// SSEB rows: not, F3, 66, F2, 66&F2
// SSE BIS VEX mult table
/** START_DYNINST_TABLE_DEF(sse_bis_vex_table, SSEB, YES) */
enum { /** AUTOGENERATED */
    SSEB00_66 = 0,
    SSEB01_66,
    SSEB02_66,
    SSEB03_66,
    SSEB04_66,
    SSEB05_66,
    SSEB06_66,
    SSEB07_66,
    SSEB0B_66,
    SSEB0C_66,
    SSEB0D_66,
    SSEB10_66,              SSEB10_F3,
    SSEB11_66,              SSEB11_F3,
    SSEB12_66,              SSEB12_F3,
    SSEB13_66,              SSEB13_F3,
    SSEB14_66,              SSEB14_F3,
    SSEB15_66,              SSEB15_F3,
    SSEB16_66,
    SSEB18_66,
    SSEB19_66,
    SSEB1A_66,
    SSEB1C_66,
    SSEB1D_66,
    SSEB1E_66,
    SSEB1F_66,
    SSEB20_66,              SSEB20_F3,
    SSEB21_66,              SSEB21_F3,
    SSEB22_66,              SSEB22_F3,
    SSEB23_66,              SSEB23_F3,
    SSEB24_66,              SSEB24_F3,
    SSEB25_66,              SSEB25_F3,
    SSEB26_F3,
    SSEB27_66,              SSEB27_F3,
    SSEB28_66,              SSEB28_F3,
    SSEB29_66,              SSEB29_F3,
    SSEB2A_66,              SSEB2A_F3,
    SSEB2B_66,
    SSEB30_66,              SSEB30_F3,
    SSEB31_66,              SSEB31_F3,
    SSEB32_66,              SSEB32_F3,
    SSEB33_66,              SSEB33_F3,
    SSEB34_66,              SSEB34_F3,
    SSEB35_66,              SSEB35_F3,
    SSEB36_66,
    SSEB37_66,
    SSEB38_66,              SSEB38_F3,
    SSEB39_66,              SSEB39_F3,
    SSEB3A_66,
    SSEB3B_66,
    SSEB3C_66,
    SSEB3D_66,
    SSEB3E_66,
    SSEB3F_66,
    SSEB40_66,
    SSEB42_66,
    SSEB43_66,
    SSEB44_66,
    SSEB45_66,
    SSEB46_66,
    SSEB47_66,
    SSEB4C_66,
    SSEB4D_66,
    SSEB4E_66,
    SSEB4F_66,
    SSEB65_66,
    SSEB66_66,
    SSEB75_66,
    SSEB76_66,
    SSEB77_66,
    SSEB7C_66,
    SSEB7D_66,
    SSEB7E_66,
    SSEB7F_66,
    SSEB83_66,
    SSEB88_66,
    SSEB89_66,
    SSEB8B_66,
    SSEB8C_66,
    SSEB8D_66,
    SSEB8E_66,
    SSEB90_66,
    SSEB91_66,
    SSEB92_66,
    SSEB93_66,
    SSEB96_66,
    SSEB97_66,
    SSEB98_66,
    SSEB99_66,
    SSEB9A_66,
    SSEB9B_66,
    SSEB9C_66,
    SSEB9D_66,
    SSEB9E_66,
    SSEB9F_66,
    SSEBA0_66,
    SSEBA1_66,
    SSEBA2_66,
    SSEBA3_66,
    SSEBA6_66,
    SSEBA7_66,
    SSEBA8_66,
    SSEBA9_66,
    SSEBAA_66,
    SSEBAB_66,
    SSEBAC_66,
    SSEBAD_66,
    SSEBAE_66,
    SSEBAF_66,
    SSEBB4_66,
    SSEBB5_66,
    SSEBB6_66,
    SSEBB7_66,
    SSEBB8_66,
    SSEBB9_66,
    SSEBBA_66,
    SSEBBB_66,
    SSEBBC_66,
    SSEBBD_66,
    SSEBBE_66,
    SSEBBF_66,
    SSEBC4_66,
    SSEBC6_66,
    SSEBC7_66,
    SSEBC8_66,
    SSEBCA_66,
    SSEBCB_66,
    SSEBCC_66,
    SSEBCD_66,
    SSEBDB_66,
    SSEBDC_66,
    SSEBDD_66,
    SSEBDE_66,
    SSEBDF_66,
                                     SSEBF2_NO,
               SSEBF5_F2, SSEBF5_F3, SSEBF5_NO,
               SSEBF6_F2, SSEBF6_F3,
    SSEBF7_66, SSEBF7_F2, SSEBF7_F3,  SSEBF7_NO
};
/** END_DYNINST_TABLE_DEF */


// SSE TER 
/** START_DYNINST_TABLE_DEF(sse_ter_table, SSET, NO) */
enum {
SSET00 = 0, SSET01, SSET02, SSET03, SSET04, SSET05, SSET06,
    SSET08, SSET09, SSET0A, SSET0B, SSET0C, SSET0D, SSET0E, SSET0F,
	                                SSET14, SSET15, SSET16, SSET17,
	SSET18, SSET19, SSET1A, SSET1B,         SSET1D, SSET1E, SSET1F,
	SSET20, SSET21, SSET22, SSET23,         SSET25, SSET26, SSET27,
    SSET30, SSET31, SSET32, SSET33,
    SSET38, SSET39, SSET3A, SSET3B,                 SSET3E, SSET3F,
	SSET40, SSET41, SSET42,         SSET44,         SSET46,
                    SSET4A, SSET4B, SSET4C,
    SSET50, SSET51,                 SSET54, SSET55, SSET56, SSET57,
	SSET60, SSET61, SSET62, SSET63,                 SSET66, SSET67,
            SSET69,
                                                            SSETDF,
    SSETF0
};
/** END_DYNINST_TABLE_DEF */

// SSET rows:  not, 66, F2
// SSE TER VEX Mult
/** START_DYNINST_TABLE_DEF(sse_vex_ter_table, SSET, NO) */
enum { /** AUTOGENERATED */
  SSET00_66 = 0,
  SSET01_66,
  SSET03_66,
  SSET04_66,
  SSET05_66,
  SSET08_66,
  SSET09_66,
  SSET0A_66,
  SSET0B_66,
  SSET0C_66,
  SSET0F_66,
  SSET14_66,
  SSET16_66,
  SSET17_66,
  SSET18_66,
  SSET19_66,
  SSET1A_66,
  SSET1B_66,
  SSET1D_66,
  SSET1E_66,
  SSET1F_66,
  SSET20_66,
  SSET21_66,
  SSET22_66,
  SSET23_66,
  SSET25_66,
  SSET26_66,
  SSET27_66,
  SSET30_66,
  SSET31_66,
  SSET32_66,
  SSET33_66,
  SSET38_66,
  SSET39_66,
  SSET3A_66,
  SSET3B_66,
  SSET3E_66,
  SSET3F_66,
  SSET42_66,
  SSET44_66,
  SSET4A_66,
  SSET4B_66,
  SSET4C_66,
  SSET50_66,
  SSET51_66,
  SSET54_66,
  SSET55_66,
  SSET56_66,
  SSET57_66,
  SSET66_66,
  SSET67_66,
  SSET69_66,
  SSETDF_66,
  SSETF0_F2
};


/* FMA4 rows */
enum {
FMA46A,
FMA46B,
FMA46D,
FMA46F,
FMA479,
FMA47B
};
/** END_DYNINST_TABLE_DEF */

// SSE groups
/** START_DYNINST_TABLE_DEF(sse_grp_map, G, NO) */
enum {
  G12SSE010B = 0, G12SSE100B, G12SSE110B,
  G13SSE010B, G13SSE100B, G13SSE110B,
  G14SSE010B, G14SSE011B, G14SSE110B, G14SSE111B,
};
/** END_DYNINST_TABLE_DEF */

enum {
    GrpD8=0, GrpD9, GrpDA, GrpDB, GrpDC, GrpDD, GrpDE, GrpDF
};

// VEX table
/** START_DYNINST_TABLE_DEF(vexl_table, VEXL, NO) */
enum {
VEXL00 = 0
};
/** END_DYNINST_TABLE_DEF */

/* Vex instructions that need extra decoding with the W bit */
/** START_DYNINST_TABLE_DEF(vex_w_table, VEXW, NO) */
enum {
VEXW00 = 0, VEXW01, VEXW02, VEXW03, VEXW04, VEXW05, VEXW06, VEXW07,
  VEXW08, VEXW09, VEXW0A, VEXW0B, VEXW0C, VEXW0D, VEXW0E, VEXW0F,
  VEXW10, VEXW11, VEXW12, VEXW13, VEXW14, VEXW15, VEXW16, VEXW17,
  VEXW18, VEXW19, VEXW1A, VEXW1B, VEXW1C, VEXW1D, VEXW1E, VEXW1F,
  VEXW20, VEXW21, VEXW22, VEXW23, VEXW24, VEXW25, VEXW26, VEXW27,
  VEXW28, VEXW29, VEXW2A, VEXW2B, VEXW2C, VEXW2D, VEXW2E, VEXW2F,
  VEXW30, VEXW31, VEXW32, VEXW33, VEXW34, VEXW35, VEXW36, VEXW37,
  VEXW38, VEXW39, VEXW3A, VEXW3B, VEXW3C, VEXW3D, VEXW3E, VEXW3F,
  VEXW40, VEXW41, VEXW42, VEXW43, VEXW44, VEXW45, VEXW46, VEXW47,
  VEXW48, VEXW49, VEXW4A, VEXW4B, VEXW4C, VEXW4D, VEXW4E, VEXW4F,
  VEXW50, VEXW51, VEXW52, VEXW53, VEXW54, VEXW55, VEXW56, VEXW57,
  VEXW58, VEXW59, VEXW5A, VEXW5B, VEXW5C, VEXW5D, VEXW5E, VEXW5F,
  VEXW60, VEXW61, VEXW62, VEXW63, VEXW64, VEXW65, VEXW66, VEXW67,
  VEXW68, VEXW69, VEXW6A, VEXW6B, VEXW6C, VEXW6D, VEXW6E, VEXW6F,
  VEXW70, VEXW71, VEXW72, VEXW73, VEXW74, VEXW75, VEXW76, VEXW77,
  VEXW78, VEXW79, VEXW7A, VEXW7B, VEXW7C, VEXW7D, VEXW7E, VEXW7F,
  VEXW80, VEXW81, VEXW82, VEXW83, VEXW84, VEXW85, VEXW86, VEXW87,
  VEXW88, VEXW89, VEXW8A, VEXW8B, VEXW8C, VEXW8D, VEXW8E, VEXW8F,
  VEXW90, VEXW91, VEXW92, VEXW93, VEXW94, VEXW95, VEXW96
};
/** END_DYNINST_TABLE_DEF */


#define VEXW_MAX VEXW96

/* XOP8 instructions that use xop.w as selector */
enum{
XOP8_A2,
XOP8_A3
};

/* XOP9 instructions that use xop.w as selector */
enum {
XOP9_9A
};

/* SIMD op conversion table */
static int vex3_simdop_convert[3][4] = {
  {0, 2,  1, 3},
  {0, 2,  1, 3},
  {0, 1, -1, 2}
};

/**
 * Operand descriptors:
 *
 * These are used to describe the addressing mode and the size of
 * the operand.
 */

#define Zz   { 0, 0 }
#define ImplImm { am_ImplImm, op_b }
#define Ap   { am_A, op_p }
#define Bv   { am_B, op_v}
#define Cd   { am_C, op_d }
#define Dd   { am_D, op_d }
#define Eb   { am_E, op_b }
#define Ed   { am_E, op_d }
#define Ef   { am_E, op_f }
#define Efd  { am_E, op_dbl }
#define Ep   { am_E, op_p }
#define Ev   { am_E, op_v }
#define Ew   { am_E, op_w }
#define Ey	 { am_E, op_y }
#define Fv   { am_F, op_v }
#define Gb   { am_G, op_b }
#define Gd   { am_G, op_d }
#define Gv   { am_G, op_v }
#define Gw   { am_G, op_w }
#define Gf   { am_G, op_f }
#define Gfd  { am_G, op_dbl }
#define Hps  { am_H, op_ps }
#define Hpd  { am_H, op_pd }
#define Hss  { am_H, op_ss }
#define Hsd  { am_H, op_sd }
#define Hdq  { am_H, op_dq }
#define Hqq  { am_H, op_qq }
#define HK   { am_HK, op_b }
#define Ib   { am_I, op_b }
#define IK   { am_I, op_b }
#define Iv   { am_I, op_v }
#define Iw   { am_I, op_w }
#define Iz   { am_I, op_z }
#define Jb   { am_J, op_b }
#define Jv   { am_J, op_v }
#define Jz   { am_J, op_z }
#define Lb   { am_L, op_b }
#define Ma   { am_M, op_a }
#define Mb   { am_M, op_b }
#define Mlea { am_M, op_lea }
#define Mp   { am_M, op_p }
#define Ms   { am_M, op_s }
#define Md   { am_M, op_d }
#define Mq   { am_M, op_q }
#define Mdq  { am_M, op_dq }
#define M512 { am_M, op_512 }
#define Mf   { am_M, op_f }
#define Mfd  { am_M, op_dbl }
#define M14  { am_M, op_14 }
#define Mv   { am_M, op_v }
#define Nss  { am_N, op_ss }
#define Ob   { am_O, op_b }
#define Ov   { am_O, op_v }
#define Pd   { am_P, op_d }
#define Pdq  { am_P, op_dq }
#define Ppi  { am_P, op_pi }
#define Pq   { am_P, op_q }
#define Qdq  { am_Q, op_dq }
#define Qd   { am_Q, op_d }
#define Qpi  { am_Q, op_pi }
#define Qq   { am_Q, op_q }
#define Rd   { am_R, op_d }
#define RMb  { am_RM, op_b }
#define RMw  { am_RM, op_w }
#define Td   { am_T, op_d }
#define UMd	 { am_UM, op_d }
#define Ups  { am_U, op_ps }
#define Upd  { am_U, op_pd }
#define Sw   { am_S, op_w }
#define Vd   { am_V, op_d }
#define Vdq  { am_V, op_dq }
#define Vpd  { am_V, op_pd }
#define Vps  { am_V, op_ps }
#define Vq   { am_V, op_q }
#define Vss  { am_V, op_ss }
#define Vsd  { am_V, op_sd }
#define VK   { am_VK, op_b }
#define Wdq  { am_W, op_dq }
#define Wpd  { am_W, op_pd }
#define Wqq  { am_W, op_qq }
#define Wps  { am_W, op_ps }
#define Wq   { am_W, op_q }
#define Wb   { am_W, op_b }
#define Ww   { am_W, op_w }
#define Wd   { am_W, op_d }
#define Ws   { am_W, op_s }
#define Wsd  { am_W, op_sd }
#define Wss  { am_W, op_ss }
#define WK   { am_WK, op_ps }
#define Xb   { am_X, op_b }
#define Xv   { am_X, op_v }
#define Yb   { am_Y, op_b }
#define Yv   { am_Y, op_v }
#define STHb { am_stackH, op_b }
#define STPb { am_stackP, op_b }
#define STHv { am_stackH, op_v }
#define STPv { am_stackP, op_v }
#define STHw { am_stackH, op_w }
#define STPw { am_stackP, op_w }
#define STHd { am_stackH, op_d }
#define STPd { am_stackP, op_d }
#define STHa { am_stackH, op_allgprs }
#define STPa { am_stackP, op_allgprs }

#define STKb { am_stack, op_b }
#define STKv { am_stack, op_v }
#define STKw { am_stack, op_w }
#define STKd { am_stack, op_d }
#define STKa { am_stack, op_allgprs }


#define GPRS { am_allgprs, op_allgprs }

#define AH   { am_reg, x86::iah }
#define AX   { am_reg, x86::iax }
#define BH   { am_reg, x86::ibh }
#define CH   { am_reg, x86::ich }
#define DH   { am_reg, x86::idh }
#define AL   { am_reg, x86::ial }
#define BL   { am_reg, x86::ibl }
#define CL   { am_reg, x86::icl }
#define CS   { am_reg, x86::ics }
#define DL   { am_reg, x86::idl }
#define DX   { am_reg, x86::idx }
#define eAX  { am_reg, x86::ieax }
#define eBX  { am_reg, x86::iebx }
#define eCX  { am_reg, x86::iecx }
#define eDX  { am_reg, x86::iedx }
#define EAX  { am_reg, x86::ieax }
#define EBX  { am_reg, x86::iebx }
#define ECX  { am_reg, x86::iecx }
#define EDX  { am_reg, x86::iedx }
#define DS   { am_reg, x86::ids }
#define ES   { am_reg, x86::ies }
#define FS   { am_reg, x86::ifs }
#define GS   { am_reg, x86::igs }
#define SS   { am_reg, x86::iss }
#define eSP  { am_reg, x86::iesp }
#define eBP  { am_reg, x86::iebp }
#define eSI  { am_reg, x86::iesi }
#define eDI  { am_reg, x86::iedi }
#define ESP  { am_reg, x86::iesp }
#define EBP  { am_reg, x86::iebp }
#define ESI  { am_reg, x86::iesi }
#define EDI  { am_reg, x86::iedi }
#define ECXEBX { am_tworeghack, op_ecxebx }
#define EDXEAX { am_tworeghack, op_edxeax }
#define rAX  { am_reg, x86::ieax }
#define rBX  { am_reg, x86::iebx }
#define rCX  { am_reg, x86::iecx }
#define rDX  { am_reg, x86::iedx }
#define rSP  { am_reg, x86::iesp }
#define rBP  { am_reg, x86::iebp }
#define rSI  { am_reg, x86::iesi }
#define rDI  { am_reg, x86::iedi }
#define ST0  { am_reg, x86::ist0 }
#define ST1  { am_reg, x86::ist1 }
#define ST2  { am_reg, x86::ist2 }
#define ST3  { am_reg, x86::ist3 }
#define ST4  { am_reg, x86::ist4 }
#define ST5  { am_reg, x86::ist5 }
#define ST6  { am_reg, x86::ist6 }
#define ST7  { am_reg, x86::ist7 }
#define FPOS 17

enum {
  fNT=1,   // non-temporal
  fPREFETCHNT,
  fPREFETCHT0,
  fPREFETCHT1,
  fPREFETCHT2,
  fPREFETCHAMDE,
  fPREFETCHAMDW,
  fCALL,
  fNEARRET,
  fFARRET,
  fIRET,
  fENTER,
  fLEAVE,
  fXLAT,
  fIO,
  fSEGDESC,
  fCOND,
  fCMPXCH,
  fCMPXCH8,
  fINDIRCALL,
  fINDIRJUMP,
  fFXSAVE,
  fFXRSTOR,
  fCLFLUSH,
  fREP,   // only rep prefix allowed: ins, movs, outs, lods, stos
  fSCAS,
  fCMPS
};

COMMON_EXPORT dyn_hash_map<entryID, std::string> entryNames_IAPI = map_list_of
  (e_aaa, "aaa")
  (e_aad, "aad")
  (e_aam, "aam")
  (e_aas, "aas")
  (e_adc, "adc")
  (e_add, "add")
  (e_addpd, "addpd")
  (e_addps, "addps")
  (e_addsd, "addsd")
  (e_addss, "addss")
  (e_addsubpd, "addsubpd")
  (e_addsubps, "addsubps")
  (e_aesenc, "aesenc")
  (e_aesenclast, "aesenclast")
  (e_aesdec, "aesdec")
  (e_aesdeclast, "aesdeclast")
  (e_aeskeygenassist, "aeskeygenassist")
  (e_aesimc, "aesimc")
  (e_pclmulqdq, "pclmulqdq")
  (e_and, "and")
  (e_andnpd, "andnpd")
  (e_andnps, "andnps")
  (e_andpd, "andpd")
  (e_andps, "andps")
  (e_arpl, "arpl")
  (e_blendpd,"blendpd")
  (e_blendps, "blendps")
  (e_blendvpd, "blendvpd")
  (e_blendvps, "blendvps")
  (e_bound, "bound")
  (e_bsf, "bsf")
  (e_bsr, "bsr")
  (e_bswap, "bswap")
  (e_bt, "bt")
  (e_btc, "btc")
  (e_btr, "btr")
  (e_bts, "bts")
  (e_call, "call")
  (e_cbw, "cbw")
  (e_cdq, "cdq")
  (e_clc, "clc")
  (e_cld, "cld")
  (e_clflush, "clflush")
  (e_cli, "cli")
  (e_clts, "clts")
  (e_cmc, "cmc")
  (e_cmovbe, "cmovbe")
  (e_cmove, "cmove")
  (e_cmovb, "cmovb")
  (e_cmovae, "cmovae")
  (e_cmova, "cmova")
  (e_cmovne, "cmovne")
  (e_cmovle, "cmovle")
  (e_cmovl, "cmovl")
  (e_cmovge, "cmovge")
  (e_cmovno, "cmovno")
  (e_cmovns, "cmovns")
  (e_cmovo, "cmovo")
  (e_cmovp, "cmovp")
  (e_cmovnp, "cmovnp")
  (e_cmovs, "cmovs")
  (e_cmp, "cmp")
  (e_cmppd, "cmppd")
  (e_cmpps, "cmpps")
  (e_cmpsb, "cmpsb")
  (e_cmpsd, "cmpsd")
  (e_cmpss, "cmpss")
  (e_cmpsw, "cmpsw")
  (e_cmpxchg, "cmpxchg")
  (e_cmpxchg8b, "cmpxchg8b")
  (e_comisd, "comisd")
  (e_comiss, "comiss")
  (e_cpuid, "cpuid")
  (e_crc32, "crc32")
  (e_cvtdq2pd, "cvtdq2pd")
  (e_cvtdq2ps, "cvtdq2ps")
  (e_cvtpd2dq, "cvtpd2dq")
  (e_cvtpd2pi, "cvtpd2pi")
  (e_cvtpd2ps, "cvtpd2ps")
  (e_cvtpi2pd, "cvtpi2pd")
  (e_cvtpi2ps, "cvtpi2ps")
  (e_cvtps2dq, "cvtps2dq")
  (e_cvtps2pd, "cvtps2pd")
  (e_cvtps2pi, "cvtps2pi")
  (e_cvtsd2si, "cvtsd2si")
  (e_cvtsd2ss, "cvtsd2ss")
  (e_cvtsi2sd, "cvtsi2sd")
  (e_cvtsi2ss, "cvtsi2ss")
  (e_cvtss2sd, "cvtss2sd")
  (e_cvtss2si, "cvtss2si")
  (e_cvttpd2dq, "cvttpd2dq")
  (e_cvttpd2pi, "cvttpd2pi")
  (e_cvttps2dq, "cvttps2dq")
  (e_cvttps2pi, "cvttps2pi")
  (e_cvttsd2si, "cvttsd2si")
  (e_cvttss2si, "cvttss2si")
  (e_cwd, "cwd")
  (e_cwde, "cwde")
  (e_daa, "daa")
  (e_das, "das")
  (e_dec, "dec")
  (e_div, "div")
  (e_divpd, "divpd")
  (e_divps, "divps")
  (e_divsd, "divsd")
  (e_divss, "divss")
  (e_dppd, "dppd")
  (e_vdppd, "vdppd")
  (e_dpps, "dpps")
  (e_emms, "emms")
  (e_endbr32, "endbr32")
  (e_endbr64, "endbr64")
  (e_enter, "enter")
  (e_extractps, "extractps")
  (e_extrq, "extrq")
  (e_fadd, "fadd")
  (e_faddp, "faddp")
 (e_f2xm1, "f2xm1")
  (e_fbld, "fbld")
  (e_fbstp, "fbstp")
 (e_fchs, "fchs")
 (e_fcmovb, "fcmovb")
 (e_fcmovbe, "fcmovbe")
 (e_fcmove, "fcmove")
 (e_fcmovne, "fcmovne")
 (e_fcmovu, "fcmovu")
 (e_fcmovnu, "fcmovnu")
 (e_fcmovnb, "fcmovnb")
 (e_fcmovnbe, "fcmovnbe")
  (e_fcom, "fcom")
  (e_fcomi, "fcomi")
  (e_fcompi, "fcompi")
  (e_fcomp, "fcomp")
  (e_fcompp, "fcompp")
  (e_fdiv, "fdiv")
  (e_fdivp, "fdivp")
  (e_fdivr, "fdivr")
  (e_fdivrp, "fdivrp")
  (e_femms, "femms")
 (e_ffree, "ffree")
 (e_ffreep, "ffreep")
  (e_fiadd, "fiadd")
  (e_ficom, "ficom")
  (e_ficomp, "ficomp")
  (e_fidiv, "fidiv")
  (e_fidivr, "fidivr")
  (e_fild, "fild")
  (e_fimul, "fimul")
  (e_fist, "fist")
  (e_fistp, "fistp")
  (e_fisttp, "fisttp")
  (e_fisub, "fisub")
  (e_fisubr, "fisubr")
  (e_fld, "fld")
 (e_fld1, "fld1")
  (e_fldcw, "fldcw")
  (e_fldenv, "fldenv")
  (e_fmul, "fmul")
  (e_fmulp, "fmulp")
  (e_fnop, "fnop")
 (e_fprem, "fprem")
  (e_frstor, "frstor")
  (e_fnsave, "fnsave")
  (e_fst, "fst")
  (e_fnstcw, "fnstcw")
  (e_fnstenv, "fnstenv")
  (e_fstp, "fstp")
  (e_fnstsw, "fnstsw")
  (e_fsub, "fsub")
  (e_fsubp, "fsubp")
  (e_fsubr, "fsubr")
  (e_fsubrp, "fsubrp")
  (e_fucom, "fucom")
  (e_fucomp, "fucomp")
  (e_fucomi, "fucomi")
  (e_fucompi, "fucompi")
  (e_fucompp, "fucompp")
 (e_fxch, "fxch")
  (e_fxrstor, "fxrstor")
  (e_fxsave, "fxsave")
  (e_xsave, "xsave")
  (e_xsavec, "xsavec")
  (e_xrstor, "xrstor")
  (e_getsec, "getsec")
  (e_xbegin, "xbegin")
  (e_xabort, "xabort")
  (e_xrstors, "xrstors")
  (e_haddpd, "haddpd")
  (e_haddps, "haddps")
  (e_hlt, "hlt")
  (e_hsubpd, "hsubpd")
  (e_hsubps, "hsubps")
  (e_idiv, "idiv")
  (e_imul, "imul")
  (e_in, "in")
  (e_inc, "inc")
  (e_insb, "insb")
  (e_insd, "insd")
  (e_insertps, "insertps")
  (e_insertq, "insertq")
  (e_insw, "insw")
  (e_int, "int")
  (e_int3, "int 3")
  (e_int1, "int1")
  (e_int80, "int 80")
  (e_into, "into")
  (e_invd, "invd")
  (e_invlpg, "invlpg")
  (e_iret, "iret")
  (e_jb, "jb")
  (e_jb_jnaej_j, "jb")
  (e_jbe, "jbe")
  (e_jcxz_jec, "jcxz")
  (e_jl, "jl")
  (e_jle, "jle")
  (e_jmp, "jmp")
  (e_jae, "jae")
  (e_jnb_jae_j, "jnb")
  (e_ja, "ja")
  (e_jge, "jge")
  (e_jg, "jg")
  (e_jno, "jno")
  (e_jnp, "jnp")
  (e_jns, "jns")
  (e_jne, "jne")
  (e_jo, "jo")
  (e_jp, "jp")
  (e_js, "js")
  (e_je, "je")
  (e_lahf, "lahf")
  (e_lar, "lar")
  (e_ldmxcsr, "ldmxcsr")
  (e_lds, "lds")
  (e_lddqu, "lddqu")
  (e_lea, "lea")
  (e_leave, "leave")
  (e_les, "les")
  (e_lfence, "lfence")
  (e_lfs, "lfs")
  (e_lgdt, "lgdt")
  (e_lgs, "lgs")
  (e_lidt, "lidt")
  (e_lldt, "lldt")
  (e_lmsw, "lmsw")
  (e_lodsb, "lodsb")
  (e_lodsd, "lodsd")
  (e_lodsw, "lodsw")
  (e_loop, "loop")
  (e_loope, "loope")
  (e_loopne, "loopne")
  (e_lsl, "lsl")
  (e_lss, "lss")
  (e_ltr, "ltr")
  (e_maskmovdqu, "maskmovdqu")
  (e_maskmovq, "maskmovq")
  (e_maxpd, "maxpd")
  (e_maxps, "maxps")
  (e_maxsd, "maxsd")
  (e_maxss, "maxss")
  (e_mfence, "mfence")
  (e_minpd, "minpd")
  (e_minps, "minps")
  (e_minsd, "minsd")
  (e_minss, "minss")
  (e_mov, "mov")
  (e_movapd, "movapd")
  (e_movaps, "movaps")
  (e_movbe, "movbe")
  (e_movd, "movd")
  (e_movddup, "movddup")
  (e_movdq2q, "movdq2q")
  (e_movdqa, "movdqa")
  (e_movdqu, "movdqu")
  (e_movhpd, "movhpd")
  (e_movhps, "movhps")
  (e_movhps_movlhps, "movhps/movlhps")
  (e_movlpd, "movlpd")
  (e_movlps, "movlps")
  (e_movlps_movhlps, "movlps/movhlps")
  (e_movmskpd, "movmskpd")
  (e_movmskps, "movmskps")
  (e_movntdq, "movntdq")
  (e_movntdqa, "movntdqa")
  (e_movnti, "movnti")
  (e_movntpd, "movntpd")
  (e_movntps, "movntps")
  (e_movntq, "movntq")
  (e_movq, "movq")
  (e_movq2dq, "movq2dq")
  (e_movsb, "movsb")
  (e_movsd, "movsd")
  (e_movsd_sse, "movsd")
  (e_movshdup, "movshdup")
  (e_movsldup, "movsldup")
  (e_movss, "movss")
  (e_movsw, "movsw")
  (e_movsx, "movsx")
  (e_movsxd, "movsxd")
  (e_movupd, "movupd")
  (e_movups, "movups")
  (e_movzx, "movzx")
  (e_mpsadbw, "mpsadbw")
  (e_mul, "mul")
  (e_mulpd, "mulpd")
  (e_mulps, "mulps")
  (e_mulsd, "mulsd")
  (e_mulss, "mulss")
  (e_neg, "neg")
  (e_nop, "nop")
  (e_not, "not")
  (e_or, "or")
  (e_orpd, "orpd")
  (e_orps, "orps")
  (e_out, "out")
  (e_outsb, "outsb")
  (e_outsd, "outsd")
  (e_outsw, "outsw")
  (e_pabsb, "pabsb")
  (e_pabsd, "pabsd")
  (e_pabsw, "pabsw")
  (e_packssdw, "packssdw")
  (e_packsswb, "packsswb")
  (e_packusdw, "packusdw")
  (e_packuswb, "packuswb")
  (e_paddb, "paddb")
  (e_paddd, "paddd")
  (e_paddq, "paddq")
  (e_paddsb, "paddsb")
  (e_paddsw, "paddsw")
  (e_paddusb, "paddusb")
  (e_paddusw, "paddusw")
  (e_paddw, "paddw")
  (e_palignr, "palignr")
  (e_pand, "pand")
  (e_pandn, "pandn")
  (e_pavgb, "pavgb")
  (e_pavgw, "pavgw")
  (e_pblendvb, "pblendvb")
  (e_pblendw, "pblendw")
  (e_pcmpeqb, "pcmpeqb")
  (e_pcmpeqd, "pcmpeqd")
  (e_pcmpeqq, "pcmpeqq")
  (e_pcmpeqw, "pcmpeqw")
  (e_pcmpestri, "pcmpestri")
  (e_pcmpestrm, "pcmpestrm")
  (e_pcmpgtd, "pcmpgtd")
  (e_pcmpgtb, "pcmpgtb")
  (e_pcmpgtq, "pcmpgtq")
  (e_pcmpgtw, "pcmpgtw")
  (e_pcmpistri, "pcmpistri")
  (e_pcmpistrm, "pcmpistrm")
  (e_pextrb, "pextrb")
  (e_pextrd_pextrq, "pextrd/pextrq")
  (e_pextrw, "pextrw")
  (e_phaddd, "phaddd")
  (e_phaddsw, "phaddsw")
  (e_phaddw, "phaddw")
  (e_phminposuw, "phminposuw")
  (e_phsubd, "phsubd")
  (e_phsubsw, "phsubsw")
  (e_phsubw, "phsubw")
  (e_pinsrb, "pinsrb")
  (e_pinsrd_pinsrq, "pinsrd/pinsrq")
  (e_pinsrw, "pinsrw")
  (e_pmaddubsw, "pmaddubsw")
  (e_pmaddwd, "pmaddwd")
  (e_pmaxsb, "pmaxsb")
  (e_pmaxsd, "pmaxsd")
  (e_pmaxsw, "pmaxsw")
  (e_pmaxub, "pmaxub")
  (e_pmaxud, "pmaxud")
  (e_pmaxuw, "pmaxuw")
  (e_pminsb, "pminsb")
  (e_pminsd, "pminsd")
  (e_pminsw, "pminsw")
  (e_pminub, "pminub")
  (e_pminud, "pminud")
  (e_pminuw, "pminuw")
  (e_pmovmskb, "pmovmskb")
  (e_pmovsxbd, "pmovsxbd")
  (e_pmovsxbq, "pmovsxbq")
  (e_pmovsxbw, "pmovsxbw")
  (e_pmovsxdq, "pmovsxdq")
  (e_pmovsxwd, "pmovsxwd")
  (e_pmovsxwq, "pmovsxwq")
  (e_pmovzxbd, "pmovzxbd")
  (e_pmovzxbq, "pmovzxbq")
  (e_pmovzxbw, "pmovzxbw")
  (e_pmovzxdq, "pmovzxdq")
  (e_pmovzxwd, "pmovzxwd")
  (e_pmovzxwq, "pmovzxwq")
  (e_pmuldq, "pmuldq")
  (e_pmulhrsw, "pmulhrsw")
  (e_pmulhuw, "pmulhuw")
  (e_pmulhw, "pmulhw")
  (e_pmullw, "pmullw")
  (e_pmulld, "pmulld")
  (e_pmuludq, "pmuludq")
  (e_pop, "pop")
  (e_popal, "popa")
  (e_popaw, "popad")
  (e_popcnt, "popcnt")
  (e_popf, "popf")
  (e_popfd, "popfd")
  (e_por, "por")
  (e_prefetch, "prefetch")
  (e_prefetchnta, "prefetchnta")
  (e_prefetcht0, "prefetcht0")
  (e_prefetcht1, "prefetchT1")
  (e_prefetcht2, "prefetchT2")
  (e_prefetch_w, "prefetchw")
  (e_prefetchw, "prefetchw")
  (e_prefetchwt1, "prefetchwt1")
  (e_psadbw, "psadbw")
  (e_pshufb, "pshufb")
  (e_pshufd, "pshufd")
  (e_pshufhw, "pshufhw")
  (e_pshuflw, "pshuflw")
  (e_pshufw, "pshufw")
  (e_psignb, "psignb")
  (e_psignd, "psignd")
  (e_psignw, "psignw")
  (e_pslld, "pslld")
  (e_pslldq, "pslldq")
  (e_psllq, "psllq")
  (e_psllw, "psllw")
  (e_psrad, "psrad")
  (e_psraw, "psraw")
  (e_psrld, "psrld")
  (e_psrldq, "psrldq")
  (e_psrlq, "psrlq")
  (e_psrlw, "psrlw")
  (e_psubb, "psubb")
  (e_psubd, "psubd")
  (e_psubsb, "psubsb")
  (e_psubsw, "psubsw")
  (e_psubusb, "psubusb")
  (e_psubusw, "psubusw")
  (e_psubw, "psubw")
  (e_ptest, "ptest")
  (e_punpckhbw, "punpckhbw")
  (e_punpckhdq, "punpckhdq")
  (e_punpckhqdq, "punpckhqdq")
  (e_punpckhwd, "punpckhwd")
  (e_punpcklbw, "punpcklbw")
  (e_punpckldq, "punpckldq")
  (e_punpcklqdq, "punpcklqdq")
  (e_punpcklwd, "punpcklwd")
  (e_push, "push")
  (e_pushal, "pusha")
  (e_pushf, "pushf")
  (e_pxor, "pxor")
  (e_rcl, "rcl")
  (e_rcpps, "rcpps")
  (e_rcpss, "rcpss")
  (e_rcr, "rcr")
  (e_rdmsr, "rdmsr")
  (e_rdpmc, "rdpmc")
  (e_rdtsc, "rdtsc")
  (e_rdrand, "rdrand")
  (e_ret_far, "ret far")
  (e_ret_near, "ret near")
  (e_rol, "rol")
  (e_ror, "ror")
  (e_roundpd, "roundpd")
  (e_roundps, "roundps")
  (e_roundsd, "roundsd")
  (e_roundss, "roundss")
  (e_rsm, "rsm")
  (e_rsqrtps, "rsqrtps")
  (e_rsqrtss, "rsqrtss")
  (e_sahf, "sahf")
  (e_salc, "salc")
  (e_sar, "sar")
  (e_sbb, "sbb")
  (e_scasb, "scasb")
  (e_scasd, "scasd")
  (e_scasw, "scasw")
  (e_setb, "setb")
  (e_setbe, "setbe")
  (e_setl, "setl")
  (e_setle, "setle")
  (e_setae, "setae")
  (e_seta, "seta")
  (e_setge, "setge")
  (e_setg, "setg")
  (e_setno, "setno")
  (e_setnp, "setnp")
  (e_setns, "setns")
  (e_setne, "setne")
  (e_seto, "seto")
  (e_setp, "setp")
  (e_sets, "sets")
  (e_sete, "sete")
  (e_sfence, "sfence")
  (e_sgdt, "sgdt")
  (e_shl, "shl")
  (e_shld, "shld")
  (e_shr, "shr")
  (e_shrd, "shrd")
  (e_shufpd, "shufpd")
  (e_shufps, "shufps")
  (e_sha1rnds4, "sha1rnds4")
  (e_sha1nexte, "sha1nexte")
  (e_sha1msg1, "sha1msg1")
  (e_sha1msg2, "sha1msg2")
  (e_sha256rnds2, "sha256rnds2")
  (e_sha256msg1, "sha256msg1")
  (e_sha256msg2, "sha256msg2")
  (e_sarx, "sarx")
  (e_clflushopt, "clflushopt")
  (e_clwb, "clwb")
  (e_sidt, "sidt")
  (e_sldt, "sldt")
  (e_smsw, "smsw")
  (e_sqrtpd, "sqrtpd")
  (e_sqrtps, "sqrtps")
  (e_sqrtsd, "sqrtsd")
  (e_sqrtss, "sqrtss")
  (e_stc, "stc")
  (e_std, "std")
  (e_sti, "sti")
  (e_stmxcsr, "stmxcsr")
  (e_stosb, "stosb")
  (e_stosd, "stosd")
  (e_stosw, "stosw")
  (e_str, "str")
  (e_sub, "sub")
  (e_subpd, "subpd")
  (e_subps, "subps")
  (e_subsd, "subsd")
  (e_subss, "subss")
  (e_syscall, "syscall")
  (e_sysenter, "sysenter")
  (e_sysexit, "sysexit")
  (e_sysret, "sysret")
  (e_test, "test")
  (e_ucomisd, "ucomisd")
  (e_ucomiss, "ucomiss")
  (e_ud2, "ud2")
  (e_ud2grp10, "ud2grp10")
  (e_unpckhpd, "unpckhpd")
  (e_unpckhps, "unpckhps")
  (e_unpcklpd, "unpcklpd")
  (e_unpcklps, "unpcklps")
  (e_verr, "verr")
  (e_verw, "verw")
  (e_wait, "wait")
  (e_wbinvd, "wbinvd")
  (e_wrmsr, "wrmsr")
  (e_xadd, "xadd")
  (e_xchg, "xchg")
  (e_xlatb, "xlatb")
  (e_xor, "xor")
  (e_xorpd, "xorpd")
  (e_xorps, "xorps")
  (e_vaesenc, "vaesenc")
  (e_vaesenclast, "vaesenclast")
  (e_vaesdec, "vaesdec")
  (e_vaesdeclast, "vaesdeclast")
  (e_vaeskeygenassist, "vaeskeygenassist")
  (e_vaesimc, "vaesimc")
  (e_vpclmulqdq, "vpclmulqdq")
  (e_vpperm, "vpperm")
  (e_vmpsadbw, "vmpsadbw") 
  (e_vmwrite, "vmwrite") 
  (e_vmread, "vmread") 
  (e_vphaddw, "vphaddw")
  (e_vphaddd, "vphaddd")
  (e_vphaddsw, "vphaddsw")
  (e_vphsubw, "vphsubw")
  (e_vphsubd, "vphsubd")
  (e_vpmovb2m, "vpmovb2m")
  (e_vpmacsdd, "vpmacsdd")
  (e_vpmaddubsw, "vpmaddubsw")
  (e_vpmaddwd, "vpmaddwd")
  (e_vpmovm2d, "vpmovm2d")
  (e_vpmovmskb, "vpmovmskb")
  (e_vpmovm2b, "vpmovm2b")
  (e_andn, "andn")
  (e_bextr, "bextr")
  (e_blsi, "blsi")
  (e_blsmsk, "blsmsk")
  (e_blsr, "blsr")
  (e_bzhi, "bzhi")
  (e_lzcnt, "lzcnt")
  (e_mulx, "mulx")
  (e_pdep, "pdep")
  (e_pext, "pext")
  (e_rorx, "rorx")
  (e_shlx, "shlx")
  (e_shrx, "shrx")
  (e_tzcnt, "tzcnt")
  (e_vaddpd, "vaddpd")
  (e_vaddps, "vaddps")
  (e_vaddsd, "vaddsd")
  (e_vaddss, "vaddss")
  (e_vandnpd, "vandnpd")
  (e_vandnps, "vandnps")
  (e_vandpd, "vandpd")
  (e_vandps, "vandps")
  (e_valignd, "valignd")
  (e_valignq, "valignq")
  (e_vbroadcastf128, "vbroadcastf128")
  (e_vbroadcasti128, "vbroadcasti128")
  (e_vbroadcastsd, "vbroadcastsd")
  (e_vbroadcastss, "vbroadcastss")
  (e_vblendmps, "vblendmps")
  (e_vblendmpd, "vblendmpd")
  (e_vblendps, "vblendps")
  (e_vblendvpd, "vblendvpd")
  (e_vblendvps, "vblendvps")
  (e_vpblendmb, "vpblendmb")
  (e_vpblendmw, "vpblendmw")
  (e_vpblendvb, "vpblendvb")
  (e_vcmppd, "vcmppd")
  (e_vcmpps, "vcmpps")
  (e_vcmpsd, "vcmpsd")
  (e_vcmpss, "vcmpss")
  (e_vcomisd, "vcomisd")
  (e_vcomiss, "vcomiss")
  (e_vcvtudq2pd, "vcvtudq2pd")
  (e_vcvtudq2ps, "vcvtudq2ps")
  (e_vcvtps2uqq, "vcvtps2uqq")
  (e_vcvtpd2qq, "vcvtpd2qq")
  (e_vcvtdq2pd, "vcvtdq2pd")
  (e_vcvtdq2ps, "vcvtdq2ps")
  (e_vcvtpd2dq, "vcvtpd2dq")
  (e_vcvtpd2ps, "vcvtpd2ps")
  (e_vcvtph2ps, "vcvtph2ps")
  (e_vcvtps2dq, "vcvtps2dq")
  (e_vcvtps2pd, "vcvtps2pd")
  (e_vcvtps2ph, "vcvtps2ph")
  (e_vcvtsd2si, "vcvtsd2si")
  (e_vcvtsd2ss, "vcvtsd2ss")
  (e_vcvtsi2sd, "vcvtsi2sd")
  (e_vcvtsi2ss, "vcvtsi2ss")
  (e_vcvtss2sd, "vcvtss2sd")
  (e_vcvtss2si, "vcvtss2si")
  (e_vcvttpd2udq, "vcvttpd2udq")
  (e_vcvttpd2uqq, "vcvttpd2uqq")
  (e_vcvttpd2qq, "vcvttpd2qq")
  (e_vcvttpd2dq, "vcvttpd2dq")
  (e_vcvttps2dq, "vcvttps2dq")
  (e_vcvttsd2si, "vcvttsd2si")
  (e_vcvttss2si, "vcvttss2si")
  (e_vcvtpd2udq, "vcvtpd2udq")
  (e_vcvtpd2uqq, "vcvtpd2uqq")
  (e_vdivpd, "vdivpd")
  (e_vdivps, "vdivps")
  (e_vdivsd, "vdivsd")
  (e_vdivss, "vdivss")
  (e_vexpandpd, "vexpandpd")
  (e_vexpandps, "vexpandps")
  (e_vextractf128, "vextractf128")
  (e_vextracti128, "vextracti128")
  (e_vextractf32x4, "vextractf32x4")
  (e_vextractf64x2, "vextractf64x2")
  (e_vextractf32x8, "vextractf32x8")
  (e_vextractf64x4, "vextractf64x4")
  (e_vextracti32x4, "vextracti32x4")
  (e_vextracti64x2, "vextracti64x2")
  (e_vextracti32x8, "vextracti32x8")
  (e_vextracti64x4, "vextracti64x4")
  (e_vextractps, "vextractps")
  (e_vexp2pd, "vexp2pd")
  (e_vexp2ps, "vexp2ps")
  (e_vroundpd, "vroundpd")
  (e_vroundps, "vroundps")
  (e_vroundsd, "vroundsd")
  (e_vroundss, "vroundss")
  (e_vrcp28pd, "vrcp28pd")
  (e_vrcp28sd, "vrcp28sd")
  (e_vrcp28ps, "vrcp28ps")
  (e_vrcp28ss, "vrcp28ss")
  (e_vrsqrt28pd, "vrsqrt28pd")
  (e_vrsqrt28sd, "vrsqrt28sd")
  (e_vrsqrt28ps, "vrsqrt28ps")
  (e_vrsqrt28ss, "vrsqrt28ss")
  (e_vfixupimmpd, "vfixupimmpd")
  (e_vfixupimmps, "vfixupimmps")
  (e_vfixupimmsd, "vfixupimmsd")
  (e_vfixupimmss, "vfixupimmss")
  (e_vfmaddpd, "vfmaddpd")
  (e_vfmaddps, "vfmaddps")
  (e_vfmaddsd, "vfmaddsd")
  (e_vfmaddss, "vfmaddss")
  (e_vfmadd132pd, "vfmadd132pd")
  (e_vfmadd132ps, "vfmadd132ps")
  (e_vfmadd132sd, "vfmadd132sd")
  (e_vfmadd132ss, "vfmadd132ss")
  (e_vfmadd213pd, "vfmadd213pd")
  (e_vfmadd213ps, "vfmadd213ps")
  (e_vfmadd213sd, "vfmadd213sd")
  (e_vfmadd213ss, "vfmadd213ss")
  (e_vfmadd231pd, "vfmadd231pd")
  (e_vfmadd231ps, "vfmadd231ps")
  (e_vfmadd231sd, "vfmadd231sd")
  (e_vfmadd231ss, "vfmadd231ss")
  (e_vfmsubpd, "vfmsubpd")
  (e_vfmsubsd, "vfmsubsd")
  (e_vfmaddsub132pd, "vfmaddsub132pd")
  (e_vfmaddsub132ps, "vfmaddsub132ps")
  (e_vfmaddsub213pd, "vfmaddsub213pd")
  (e_vfmaddsub213ps, "vfmaddsub213ps")
  (e_vfmaddsub231pd, "vfmaddsub231pd")
  (e_vfmaddsub231ps, "vfmaddsub231ps")
  (e_vfpclassps, "vfpclassps")
  (e_vfpclasspd, "vfpclasspd")
  (e_vfpclassss, "vfpclassss")
  (e_vfpclasssd, "vfpclasssd")
  (e_vfmsub132pd, "vfmsub132pd")
  (e_vfmsub132ps, "vfmsub132ps")
  (e_vfmsub132sd, "vfmsub132sd")
  (e_vfmsub132ss, "vfmsub132ss")
  (e_vfmsub213pd, "vfmsub213pd")
  (e_vfmsub213ps, "vfmsub213ps")
  (e_vfmsub213sd, "vfmsub213sd")
  (e_vfmsub213ss, "vfmsub213ss")
  (e_vfmsub231pd, "vfmsub231pd")
  (e_vfmsub231ps, "vfmsub231ps")
  (e_vfmsub231sd, "vfmsub231sd")
  (e_vfmsub231ss, "vfmsub231ss")
  (e_vfmsubadd132pd, "vfmsubadd132pd")
  (e_vfmsubadd132ps, "vfmsubadd132ps")
  (e_vfmsubadd213pd, "vfmsubadd213pd")
  (e_vfmsubadd213ps, "vfmsubadd213ps")
  (e_vfmsubadd231pd, "vfmsubadd231pd")
  (e_vfmsubadd231ps, "vfmsubadd231ps")
  (e_vfnmaddpd, "vfnmaddpd")
  (e_vfnmaddsd, "vfnmaddsd")
  (e_vfnmadd132pd, "vfnmadd132pd")
  (e_vfnmadd132ps, "vfnmadd132ps")
  (e_vfnmadd132sd, "vfnmadd132sd")
  (e_vfnmadd132ss, "vfnmadd132ss")
  (e_vfnmadd213pd, "vfnmadd213pd")
  (e_vfnmadd213ps, "vfnmadd213ps")
  (e_vfnmadd213sd, "vfnmadd213sd")
  (e_vfnmadd213ss, "vfnmadd213ss")
  (e_vfnmadd231pd, "vfnmadd231pd")
  (e_vfnmadd231ps, "vfnmadd231ps")
  (e_vfnmadd231sd, "vfnmadd231sd")
  (e_vfnmadd231ss, "vfnmadd231ss")
  (e_vfnmsub132pd, "vfnmsub132pd")
  (e_vfnmsub132ps, "vfnmsub132ps")
  (e_vfnmsub132sd, "vfnmsub132sd")
  (e_vfnmsub132ss, "vfnmsub132ss")
  (e_vfnmsub213pd, "vfnmsub213pd")
  (e_vfnmsub213ps, "vfnmsub213ps")
  (e_vfnmsub213sd, "vfnmsub213sd")
  (e_vfnmsub213ss, "vfnmsub213ss")
  (e_vfnmsub231pd, "vfnmsub231pd")
  (e_vfnmsub231ps, "vfnmsub231ps")
  (e_vfnmsub231sd, "vfnmsub231sd")
  (e_vfnmsub231ss, "vfnmsub231ss")
  (e_vgatherpf0dps, "vgatherpf0dps")
  (e_vgatherpf0dpd, "vgatherpf0dpd")
  (e_vgatherpf1qps ,"vgatherpf1qps")
  (e_vgatherpf1dpd ,"vgatherpf1dpd")
  (e_vgatherpf0qps ,"vgatherpf0qps")
  (e_vscatterpf0dps ,"vscatterpf0dps")
  (e_vscatterpf0qpd ,"vscatterpf0qpd")
  (e_vscatterpf1qps ,"vscatterpf1qps")
  (e_vscatterpf1qpd ,"vscatterpf1qpd")
  (e_vgatherdpd, "vgatherdpd")
  (e_vgatherdps, "vgatherdps")
  (e_vgatherqpd, "vgatherqpd")
  (e_vgatherqps, "vgatherqps")
  (e_vgetexpps, "vgetexpps")
  (e_vgetexppd, "vgetexppd")
  (e_vgetexpss, "vgetexpss")
  (e_vgetexpsd, "vgetexpsd")
  (e_vgetmantps, "vgetmantps")
  (e_vgetmantpd, "vgetmantpd")
  (e_vgetmantss, "vgetmantss")
  (e_vgetmantsd, "vgetmantsd")
  (e_vinsertf128, "vinsertf128")
  (e_vinserti128, "vinserti128")
  (e_vinsertps, "vinsertps")
  (e_vinsertf32x4, "vinsertf32x4")
  (e_vinsertf64x2, "vinsertf64x2")
  (e_vinsertf32x8, "vinsertf32x8")
  (e_vinsertf64x4, "vinsertf64x4")
  (e_vinserti32x4, "vinserti32x4")
  (e_vinserti64x2, "vinserti64x2")
  (e_vinserti32x8, "vinserti32x8")
  (e_vinserti64x4, "vinserti64x4")
  (e_vmaskmovpd, "vmaskmovpd")
  (e_vmaskmovps, "vmaskmovps")
  (e_vmaxpd, "vmaxpd")
  (e_vmaxps, "vmaxps")
  (e_vmaxsd, "vmaxsd")
  (e_vmaxss, "vmaxss")
  (e_vminpd, "vminpd")
  (e_vminps, "vminps")
  (e_vminsd, "vminsd")
  (e_vminss, "vminss")
  (e_vmovapd, "vmovapd")
  (e_vmovaps, "vmovaps")
  (e_vmovddup, "vmovddup")
  (e_vmovdqa, "vmovdqa")
  (e_vmovdqa32, "vmovdqa32")
  (e_vmovdqa64, "vmovdqa64")
  (e_vmovdqu32, "vmovdqu32")
  (e_vmovdqu64, "vmovdqu64")
  (e_vmovdqu, "vmovdqu")
  (e_vmovdqu8, "vmovdqu8")
  (e_vmovdqu16, "vmovdqu16")
  (e_vmovhlps, "vmovhlps")
  (e_vmovhpd, "vmovhpd")
  (e_vmovhps, "vmovhps")
  (e_vmovlhps, "vmovlhps")
  (e_vmovlpd, "vmovlpd")
  (e_vmovlps, "vmovlps")
  (e_vmovntps, "vmovntps")
  (e_vmovq, "vmovq")
  (e_vmovsd, "vmovsd")
  (e_vmovshdup, "vmovshdup")
  (e_vmovsldup, "vmovsldup")
  (e_vmovss, "vmovss")
  (e_vmovupd, "vmovupd")
  (e_vmovups, "vmovups")
  (e_vmulpd, "vmulpd")
  (e_vmulps, "vmulps")
  (e_vmulsd, "vmulsd")
  (e_vmulss, "vmulss")
  (e_vorpd, "vorpd")
  (e_vorps, "vorps")
  (e_vpabsb, "vpabsb")
  (e_vpabsd, "vpabsd")
  (e_vpabsw, "vpabsw")
  (e_vpackssdw, "vpackssdw")
  (e_vpacksswb, "vpacksswb")
  (e_vpackusdw, "vpackusdw")
  (e_vpackuswb, "vpackuswb")
  (e_vpaddb, "vpaddb")
  (e_vpaddd, "vpaddd")
  (e_vpaddq, "vpaddq")
  (e_vpaddsb, "vpaddsb")
  (e_vpaddsw, "vpaddsw")
  (e_vpaddusb, "vpaddusb")
  (e_vpaddusw, "vpaddusw")
  (e_vpaddw, "vpaddw")
  (e_vpalignr, "vpalignr")
  (e_vpand, "vpand")
  (e_vpandn, "vpandn")
  (e_vpandd, "vpandd")
  (e_vpandq, "vpandq")
  (e_vpandnd, "vpandnd")
  (e_vpandnq, "vpandnq")
  (e_vpavgb, "vpavgb")
  (e_vpavgw, "vpavgw")
  (e_vpblendd, "vpblendd")
  (e_vpbroadcastb, "vpbroadcastb")
  (e_vpbroadcastd, "vpbroadcastd")
  (e_vpbroadcastq, "vpbroadcastq")
  (e_vpbroadcastw, "vpbroadcastw")
  (e_vpcmov, "vpcmov")
  (e_vpcmpub, "vpcmpub")
  (e_vpcmpb, "vpcmpb")
  (e_vpcmpeqb, "vpcmpeqb")
  (e_vpcmpeqd, "vpcmpeqd")
  (e_vpcmpeqq, "vpcmpeqq")
  (e_vpcmpeqw, "vpcmpeqw")
  (e_vpcmpgtb, "vpcmpgtb")
  (e_vpcmpgtd, "vpcmpgtd")
  (e_vpcmpgtq, "vpcmpgtq")
  (e_vpcmpgtw, "vpcmpgtw")
  (e_vpcomd, "vpcomd")
  (e_vpcompressd, "vpcompressd")
  (e_vpcompressq, "vpcompressq")
  (e_vpconflictd, "vpconflictd")
  (e_vpconflictq, "vpconflictq")
  (e_vperm2f128, "vperm2f128")
  (e_vperm2i128, "vperm2i128")
  (e_vpermd, "vpermd")
  (e_vpermilpd, "vpermilpd")
  (e_vpermilps, "vpermilps")
  (e_vpermi2b, "vpermi2b")
  (e_vpermi2w, "vpermi2w")
  (e_vpermi2d, "vpermi2d")
  (e_vpermi2q, "vpermi2q")
  (e_vpermi2ps, "vpermi2ps")
  (e_vpermi2pd, "vpermi2pd")
  (e_vpermt2b, "vpermt2b")
  (e_vpermt2w, "vpermt2w")
  (e_vpermt2d, "vpermt2d")
  (e_vpermt2q, "vpermt2q")
  (e_vpermt2ps, "vpermt2ps")
  (e_vpermt2pd, "vpermt2pd")
  (e_vpermpd, "vpermpd")
  (e_vpermps, "vpermps")
  (e_vpermq, "vpermq")
  (e_vpermb, "vpermb")
  (e_vpermw, "vpermw")
  (e_vpextrb, "vpextrb")
  (e_vpextrd, "vpextrd")
  (e_vpextrq, "vpextrq")
  (e_vpextrw, "vpextrw")
  (e_vpexpandd, "vpexpandd")
  (e_vpexpandq, "vpexpandq")
  (e_vplzcntd, "vplzcntd")
  (e_vplzcntq, "vplzcntq")
  (e_vpgatherdd, "vpgatherdd")
  (e_vpgatherdq, "vpgatherdq")
  (e_vpgatherqd, "vpgatherqd")
  (e_vpgatherqq, "vpgatherqq")
  (e_vpinsrb, "vpinsrb")
  (e_vpinsrd, "vpinsrd")
  (e_vpinsrq, "vpinsrq")
  (e_vpinsrw, "vpinsrw")
  (e_vpmaskmovd, "vpmaskmovd")
  (e_vpmaskmovq, "vpmaskmovq")
  (e_vpmaxsq, "vpmaxsq")
  (e_vpmaxuq, "vpmaxuq")
  (e_vpmaxsb, "vpmaxsb")
  (e_vpmaxsd, "vpmaxsd")
  (e_vpmaxsw, "vpmaxsw")
  (e_vpmaxub, "vpmaxub")
  (e_vpmaxud, "vpmaxud")
  (e_vpmaxuw, "vpmaxuw")
  (e_vpminsq, "vpminsq")
  (e_vpminuq, "vpminuq")
  (e_vpminsb, "vpminsb")
  (e_vpminsd, "vpminsd")
  (e_vpminsw, "vpminsw")
  (e_vpminub, "vpminub")
  (e_vpminud, "vpminud")
  (e_vpminuw, "vpminuw")
  (e_vpmovsdb, "vpmovsdb")
  (e_vpmovsdw, "vpmovsdw")
  (e_vpmovsqb, "vpmovsqb")
  (e_vpmovsqd, "vpmovsqd")
  (e_vpmovsqw, "vpmovsqw")
  (e_vpmovswb, "vpmovswb")
  (e_vpmovsxbd, "vpmovsxbd")
  (e_vpmovsxbq, "vpmovsxbq")
  (e_vpmovsxbw, "vpmovsxbw")
  (e_vpmovsxdq, "vpmovsxdq")
  (e_vpmovsxwd, "vpmovsxwd")
  (e_vpmovsxwq, "vpmovsxwq")
  (e_vpmovzxbd, "vpmovzxbd")
  (e_vpmovzxbq, "vpmovzxbq")
  (e_vpmovzxbw, "vpmovzxbw")
  (e_vpmovzxdq, "vpmovzxdq")
  (e_vpmovzxwd, "vpmovzxwd")
  (e_vpmovzxwq, "vpmovzxwq")
  (e_vpmuldq, "vpmuldq")
  (e_vpmulhrsw, "vpmulhrsw")
  (e_vpmulhuw, "vpmulhuw")
  (e_vpmulhw, "vpmulhw")
  (e_vpmulld, "vpmulld")
  (e_vpmullw, "vpmullw")
  (e_vpmuludq, "vpmuludq")
  (e_vpor, "vpor")
  (e_vpord, "vpord")
  (e_vporq, "vporq")
  (e_vprolvd, "vprolvd")
  (e_vprolvq, "vprolvq")
  (e_vprold, "vprold")
  (e_vprolq, "vprolq")
  (e_vprorvd, "vprorvd")
  (e_vprorvq, "vprorvq")
  (e_vprord, "vprord")
  (e_vprorq, "vprorq")
  (e_vrsqrt14ps, "vrsqrt14ps")
  (e_vrsqrt14pd, "vrsqrt14pd")
  (e_vrsqrt14ss, "vrsqrt14ss")
  (e_vrsqrt14sd, "vrsqrt14sd")
  (e_vscatterdps, "vscatterdps")
  (e_vscatterdpd, "vscatterdpd")
  (e_vscatterqps, "vscatterqps")
  (e_vscatterqpd, "vscatterqpd")
  (e_vpscatterdd, "vpscatterdd")
  (e_vpscatterdq, "vpscatterdq")
  (e_vpscatterqd, "vpscatterqd")
  (e_vpscatterqq, "vpscatterqq")
  (e_vpsadbw, "vpsadbw")
  (e_vpshad, "vpshad")
  (e_vpshufb, "vpshufb")
  (e_vpshufd, "vpshufd")
  (e_vpshufhw, "vpshufhw")
  (e_vpshuflw, "vpshuflw")
  (e_vpslldq, "vpslldq")
  (e_vpslld, "vpslld")
  (e_vpsllq, "vpsllq")
  (e_vpsllvd, "vpsllvd")
  (e_vpsllvq, "vpsllvq")
  (e_vpsllw, "vpsllw")
  (e_vpsrad, "vpsrad")
  (e_vpsravd, "vpsravd")
  (e_vpsraw, "vpsraw")
  (e_vpsrldq, "vpsrldq")
  (e_vpsrld, "vpsrld")
  (e_vpsrlq, "vpsrlq")
  (e_vpsrlvd, "vpsrlvd")
  (e_vpsrlvq, "vpsrlvq")
  (e_vpsrlw, "vpsrlw")
  (e_vpsubb, "vpsubb")
  (e_vpsubd, "vpsubd")
  (e_vpsubq, "vpsubq")
  (e_vpsubsb, "vpsubsb")
  (e_vpsubsw, "vpsubsw")
  (e_vpsubusb, "vpsubusb")
  (e_vpsubusw, "vpsubusw")
  (e_vpsubw, "vpsubw")
  (e_vptestmd, "vptestmd")
  (e_vptestnmd, "vptestnmd")
  (e_vptestnmb, "vptestnmb")
  (e_vpternlogd, "vpternlogd")
  (e_vpternlogq, "vpternlogq")
  (e_vpunpckhbw, "vpunpckhbw")
  (e_vpunpckhdq, "vpunpckhdq")
  (e_vpunpckhqdq, "vpunpckhqdq")
  (e_vpunpckhwd, "vpunpckhwd")
  (e_vpunpcklbw, "vpunpcklbw")
  (e_vpunpckldq, "vpunpckldq")
  (e_vpunpcklqdq, "vpunpcklqdq")
  (e_vpunpcklwd, "vpunpcklwd")
  (e_vpxord, "vpxord")
  (e_vpxorq, "vpxorq")
  (e_vrangeps, "vrangeps")
  (e_vrangepd, "vrangepd")
  (e_vrangess, "vrangess")
  (e_vrangesd, "vrangesd")
  (e_vrcp14ps, "vrcp14ps")
  (e_vrcp14pd, "vrcp14pd")
  (e_vrcp14ss, "vrcp14ss")
  (e_vrcp14sd, "vrcp14sd")
  (e_vreduceps, "vreduceps")
  (e_vreducepd, "vreducepd")
  (e_vreducess, "vreducess")
  (e_vreducesd, "vreducesd")
  (e_vpxor, "vpxor")
  (e_vshufpd, "vshufpd")
  (e_vshufps, "vshufps")
  (e_vshuff32x4, "vshuff32x4")
  (e_vshuff64x2, "vshuff64x2")
  (e_vsqrtpd, "vsqrtpd")
  (e_vsqrtps, "vsqrtps")
  (e_vsqrtsd, "vsqrtsd")
  (e_vsqrtss, "vsqrtss")
  (e_vsubpd, "vsubpd")
  (e_vsubps, "vsubps")
  (e_vsubsd, "vsubsd")
  (e_vsubss, "vsubss")
  (e_vtestpd, "vtestpd")
  (e_vtestps, "vtestps")
  (e_vucomisd, "vucomisd")
  (e_vucomiss, "vucomiss")
  (e_vunpckhpd, "vunpckhpd")
  (e_vunpckhps, "vunpckhps")
  (e_vunpcklpd, "vunpcklpd")
  (e_vunpcklps, "vunpcklps")
  (e_vxorpd, "vxorpd")
  (e_vxorps, "vxorps")
  (e_vzeroall, "vzeroall")
  (e_vzeroupper, "vzeroupper")
  (e_kandb, "kandb")
  (e_kandd, "kandd")
  (e_kandw, "kandw")
  (e_kandq, "kandq")
  (e_kandnb, "kandnb")
  (e_kandnd, "kandnd")
  (e_kandnw, "kandnw")
  (e_kandnq, "kandnq")
  (e_knotb, "knotb")
  (e_knotd, "knotd")
  (e_knotw, "knotw")
  (e_knotq, "knotq")
  (e_korb, "korb")
  (e_kord, "kord")
  (e_korw, "korw")
  (e_korq, "korq")
  (e_kxnorb, "kxnorb")
  (e_kxnord, "kxnord")
  (e_kxnorw, "kxnorw")
  (e_kxnorq, "kxnorq")
  (e_kxorb, "kxorb")
  (e_kxord, "kxord")
  (e_kxorw, "kxorw")
  (e_kxorq, "kxorq")
  (e_kaddb, "kaddb")
  (e_kaddd, "kaddd")
  (e_kaddw, "kaddw")
  (e_kaddq, "kaddq")
  (e_kshiftlw, "kshiftlw")
  (e_kshiftlb, "kshiftlb")
  (e_kshiftlq, "kshiftlq")
  (e_kshiftld, "kshiftld")
  (e_kshiftrw, "kshiftrw")
  (e_kshiftrb, "kshiftrb")
  (e_kshiftrq, "kshiftrq")
  (e_kshiftrd, "kshiftrd")
  (e_kunpckbw, "kunpckbw")
  (e_kunpckwd, "kunpckwd")
  (e_kunpckdq, "kunpckdq")
  (e_kmovb, "kmovb")
  (e_kmovd, "kmovd")
  (e_kmovw, "kmovw")
  (e_kmovq, "kmovq")
  (e_kortestb, "kortestb")
  (e_kortestd, "kortestd")
  (e_kortestw, "kortestw")
  (e_kortestq, "kortestq")
  (e_ktestb, "ktestb")
  (e_ktestd, "ktestd")
  (e_ktestw, "ktestw")
  (e_ktestq, "ktestq")
  (e_vmovntpd, "vmovntpd")
  (e_vcvttsd2usi, "vcvttsd2usi")
  (e_vcvttss2usi, "vcvttss2usi")
  (e_vcvtsd2usi, "vcvtsd2usi")
  (e_vcvtss2usi, "vcvtss2usi")
  (e_vcvtusi2sd, "vcvtusi2sd")
  (e_vcvtusi2ss, "vcvtusi2ss")
  (e_vmovntdq, "vmovntdq")
  (e_vpsrlvw, "vpsrlvw")
  (e_vpmovuswb, "vpmovuswb")
  (e_vpsravw, "vpsravw")
  (e_vpsravq, "vpsravq")
  (e_vpmovusdb, "vpmovusdb")
  (e_vpsllvw, "vpsllvw")
  (e_vpmovusqb, "vpmovusqb")
  (e_vpmovusdw, "vpmovusdw")
  (e_vpmovusqw, "vpmovusqw")
  (e_vpmovusqd, "vpmovusqd")
  (e_vbroadcastf32x4, "vbroadcastf32x4")
  (e_vpabsq, "vpabsq")
  (e_vmovntdqa, "vmovntdqa")
  (e_vpbroadcastmb2q, "vpbroadcastmb2q")
  (e_vpmovwb, "vpmovwb")
  (e_vpmovdb, "vpmovdb")
  (e_vpmovqb, "vpmovqb")
  (e_vpmovdw, "vpmovdw")
  (e_vpmovqw, "vpmovqw")
  (e_vpmovqd, "vpmovqd")
  (e_vpmultishiftqb, "vpmultishiftqb")
  (e_vpmadd52luq, "vpmadd52luq")
  (e_vpmadd52huq, "vpmadd52huq")
  (e_vrndscaleps, "vrndscaleps")
  (e_vrndscalepd, "vrndscalepd")
  (e_vrndscaless, "vrndscaless")
  (e_vrndscalesd, "vrndscalesd")
  (e_vdbpsadbw, "vdbpsadbw")
  (e_vphsubsw, "vphsubsw")

 (e_fp_generic, "[FIXME: GENERIC FPU INSN]")
 (e_3dnow_generic, "[FIXME: GENERIC 3DNow INSN]")
 (e_No_Entry, "No_Entry")
        ;

dyn_hash_map<prefixEntryID, std::string> prefixEntryNames_IAPI = map_list_of
  (prefix_rep, "REP")
  (prefix_repnz, "REPNZ")
        ;

dyn_hash_map<entryID, flagInfo> ia32_instruction::flagTable;


COMMON_EXPORT dyn_hash_map<entryID, flagInfo> const& ia32_instruction::getFlagTable()
{
  static std::once_flag flagTableInit;
  std::call_once(flagTableInit, [&]() {
    initFlagTable(flagTable);
    ANNOTATE_HAPPENS_BEFORE(&flagTableInit);
  });
  ANNOTATE_HAPPENS_AFTER(&flagTableInit);
  return flagTable;
}
  
void ia32_instruction::initFlagTable(dyn_hash_map<entryID, flagInfo>& flagTable_)
{
  static const vector<Dyninst::MachRegister> standardFlags = list_of(x86::of)(x86::sf)(x86::zf)(x86::af)(x86::pf)(x86::cf);

  flagTable_[e_aaa] = flagInfo(list_of(x86::af), standardFlags);
  flagTable_[e_aad] = flagInfo(vector<Dyninst::MachRegister>(), standardFlags);
  flagTable_[e_aam] = flagInfo(vector<Dyninst::MachRegister>(), standardFlags);
  flagTable_[e_aas] = flagInfo(list_of(x86::af), standardFlags);
  flagTable_[e_adc] = flagInfo(list_of(x86::cf), standardFlags);
  flagTable_[e_add] = flagInfo(vector<Dyninst::MachRegister>(), standardFlags);
  flagTable_[e_and] = flagInfo(vector<Dyninst::MachRegister>(), standardFlags);
  flagTable_[e_arpl] = flagInfo(vector<Dyninst::MachRegister>(), list_of(x86::zf));
  flagTable_[e_bsf] = flagInfo(vector<Dyninst::MachRegister>(), standardFlags);
  flagTable_[e_bsr] = flagInfo(vector<Dyninst::MachRegister>(), standardFlags);
  flagTable_[e_bt] = flagInfo(vector<Dyninst::MachRegister>(), standardFlags);
  flagTable_[e_bts] = flagInfo(vector<Dyninst::MachRegister>(), standardFlags);
  flagTable_[e_btr] = flagInfo(vector<Dyninst::MachRegister>(), standardFlags);
  flagTable_[e_btc] = flagInfo(vector<Dyninst::MachRegister>(), standardFlags);
  flagTable_[e_clc] = flagInfo(vector<Dyninst::MachRegister>(), list_of(x86::cf));
  flagTable_[e_cld] = flagInfo(vector<Dyninst::MachRegister>(), list_of(x86::df));
  flagTable_[e_cli] = flagInfo(vector<Dyninst::MachRegister>(), list_of(x86::if_));
  flagTable_[e_cmc] = flagInfo(vector<Dyninst::MachRegister>(), list_of(x86::cf));
  flagTable_[e_cmovbe] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable_[e_cmove] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable_[e_cmovb] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable_[e_cmovae] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable_[e_cmova] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable_[e_cmovne] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable_[e_cmovle] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable_[e_cmovl] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable_[e_cmovge] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable_[e_cmovno] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable_[e_cmovns] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable_[e_cmovo] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable_[e_cmovp] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable_[e_cmovnp] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable_[e_cmovs] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable_[e_cmp] = flagInfo(vector<Dyninst::MachRegister>(), standardFlags);
  flagTable_[e_cmpsb] = flagInfo(vector<Dyninst::MachRegister>(), standardFlags);
  flagTable_[e_cmpsd] = flagInfo(vector<Dyninst::MachRegister>(), standardFlags);
  flagTable_[e_cmpss] = flagInfo(vector<Dyninst::MachRegister>(), standardFlags);
  flagTable_[e_cmpsw] = flagInfo(vector<Dyninst::MachRegister>(), standardFlags);
  flagTable_[e_cmpxchg] = flagInfo(vector<Dyninst::MachRegister>(), standardFlags);
  flagTable_[e_cmpxchg8b] = flagInfo(vector<Dyninst::MachRegister>(), list_of(x86::zf));
  flagTable_[e_comisd] = flagInfo(vector<Dyninst::MachRegister>(), standardFlags);
  flagTable_[e_comiss] = flagInfo(vector<Dyninst::MachRegister>(), standardFlags);
  flagTable_[e_daa] = flagInfo(list_of(x86::af)(x86::cf), standardFlags);
  flagTable_[e_das] = flagInfo(list_of(x86::af)(x86::cf), standardFlags);
  flagTable_[e_dec] = flagInfo(vector<Dyninst::MachRegister>(), list_of(x86::of)(x86::sf)(x86::zf)(x86::af)(x86::pf));
  flagTable_[e_div] = flagInfo(list_of(x86::af)(x86::cf), standardFlags);
  // TODO: FCMOVcc (not in our entry table) (reads zf/pf/cf)
  // TODO: FCOMI/FCOMIP/FUCOMI/FUCOMIP (writes/zf/pf/cf)
  flagTable_[e_idiv] = flagInfo(vector<Dyninst::MachRegister>(), standardFlags);
  flagTable_[e_imul] = flagInfo(vector<Dyninst::MachRegister>(), standardFlags);
  flagTable_[e_inc] = flagInfo(vector<Dyninst::MachRegister>(), list_of(x86::of)(x86::sf)(x86::zf)(x86::af)(x86::pf));
  flagTable_[e_insb] = flagInfo(list_of(x86::df), vector<Dyninst::MachRegister>());
  flagTable_[e_insw] = flagInfo(list_of(x86::df), vector<Dyninst::MachRegister>());
  flagTable_[e_insd] = flagInfo(list_of(x86::df), vector<Dyninst::MachRegister>());
  flagTable_[e_int] = flagInfo(vector<Dyninst::MachRegister>(), list_of(x86::tf)(x86::nt_));
  flagTable_[e_int3] = flagInfo(vector<Dyninst::MachRegister>(), list_of(x86::tf)(x86::nt_));
  flagTable_[e_int80] = flagInfo(vector<Dyninst::MachRegister>(), list_of(x86::tf)(x86::nt_));
  flagTable_[e_into] = flagInfo(list_of(x86::of), list_of(x86::tf)(x86::nt_));
  flagTable_[e_ucomisd] = flagInfo(vector<Dyninst::MachRegister>(), standardFlags);
  flagTable_[e_ucomiss] = flagInfo(vector<Dyninst::MachRegister>(), standardFlags);
  flagTable_[e_iret] = flagInfo(list_of(x86::nt_),
list_of(x86::of)(x86::sf)(x86::zf)(x86::af)(x86::pf)(x86::cf)(x86::tf)(x86::if_)(x86::df));
  flagTable_[e_jb] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable_[e_jb_jnaej_j] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable_[e_jbe] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable_[e_jl] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable_[e_jle] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable_[e_jae] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable_[e_jnb_jae_j] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable_[e_ja] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable_[e_jge] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable_[e_jg] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable_[e_jno] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable_[e_jnp] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable_[e_jns] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable_[e_jne] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable_[e_jo] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable_[e_jp] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable_[e_js] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable_[e_je] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable_[e_lar] = flagInfo(vector<Dyninst::MachRegister>(), list_of(x86::zf));
  flagTable_[e_lodsb] = flagInfo(list_of(x86::df), vector<Dyninst::MachRegister>());
  flagTable_[e_lodsd] = flagInfo(list_of(x86::df), vector<Dyninst::MachRegister>());
  flagTable_[e_lodsw] = flagInfo(list_of(x86::df), vector<Dyninst::MachRegister>());
  flagTable_[e_loope] = flagInfo(list_of(x86::zf), vector<Dyninst::MachRegister>());
  flagTable_[e_loopne] = flagInfo(list_of(x86::zf), vector<Dyninst::MachRegister>());
  flagTable_[e_lsl] = flagInfo(vector<Dyninst::MachRegister>(), list_of(x86::zf));
  // I'd expect that mov control/debug/test gets handled when we do operand analysis
  // If it doesn't, fix later
  flagTable_[e_mul] = flagInfo(vector<Dyninst::MachRegister>(), standardFlags);
  flagTable_[e_neg] = flagInfo(vector<Dyninst::MachRegister>(), standardFlags);
  flagTable_[e_or] = flagInfo(vector<Dyninst::MachRegister>(), standardFlags);
  flagTable_[e_outsb] = flagInfo(list_of(x86::df), vector<Dyninst::MachRegister>());
  flagTable_[e_outsw] = flagInfo(list_of(x86::df), vector<Dyninst::MachRegister>());
  flagTable_[e_outsd] = flagInfo(list_of(x86::df), vector<Dyninst::MachRegister>());
  flagTable_[e_popf] = flagInfo(vector<Dyninst::MachRegister>(),
list_of(x86::of)(x86::sf)(x86::zf)(x86::af)(x86::pf)(x86::cf)(x86::tf)(x86::if_)(x86::df)(x86::nt_));
  flagTable_[e_popfd] = flagInfo(vector<Dyninst::MachRegister>(),
list_of(x86::of)(x86::sf)(x86::zf)(x86::af)(x86::pf)(x86::cf)(x86::tf)(x86::if_)(x86::df)(x86::nt_));
  flagTable_[e_rcl] = flagInfo(list_of(x86::cf), list_of(x86::of)(x86::cf));
  flagTable_[e_rcr] = flagInfo(list_of(x86::cf), list_of(x86::of)(x86::cf));
  flagTable_[e_rol] = flagInfo(list_of(x86::cf), list_of(x86::of)(x86::cf));
  flagTable_[e_ror] = flagInfo(list_of(x86::cf), list_of(x86::of)(x86::cf));
  flagTable_[e_rsm] = flagInfo(vector<Dyninst::MachRegister>(),
list_of(x86::of)(x86::sf)(x86::zf)(x86::af)(x86::pf)(x86::cf)(x86::tf)(x86::if_)(x86::df)(x86::nt_)(x86::rf));
  flagTable_[e_sahf] = flagInfo(list_of(x86::sf)(x86::zf)(x86::af)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable_[e_sar] = flagInfo(vector<Dyninst::MachRegister>(), standardFlags);
  flagTable_[e_shr] = flagInfo(vector<Dyninst::MachRegister>(), standardFlags);
  flagTable_[e_salc] = flagInfo(list_of(x86::cf), vector<Dyninst::MachRegister>());
  flagTable_[e_sbb] = flagInfo(list_of(x86::cf), standardFlags);
  flagTable_[e_setb] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable_[e_setbe] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable_[e_setl] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable_[e_setle] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable_[e_setae] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable_[e_seta] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable_[e_setge] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable_[e_setg] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable_[e_setno] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable_[e_setnp] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable_[e_setns] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable_[e_setne] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable_[e_seto] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable_[e_setp] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable_[e_sets] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable_[e_sete] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::pf)(x86::cf), vector<Dyninst::MachRegister>());
  flagTable_[e_shld] = flagInfo(vector<Dyninst::MachRegister>(), standardFlags);
  flagTable_[e_shrd] = flagInfo(vector<Dyninst::MachRegister>(), standardFlags);
  flagTable_[e_shl] = flagInfo(vector<Dyninst::MachRegister>(), standardFlags);
  flagTable_[e_stc] = flagInfo(vector<Dyninst::MachRegister>(), list_of(x86::cf));
  flagTable_[e_std] = flagInfo(vector<Dyninst::MachRegister>(), list_of(x86::df));
  flagTable_[e_sti] = flagInfo(vector<Dyninst::MachRegister>(), list_of(x86::if_));
  flagTable_[e_stosb] = flagInfo(list_of(x86::df), vector<Dyninst::MachRegister>());
  flagTable_[e_stosd] = flagInfo(list_of(x86::df), vector<Dyninst::MachRegister>());
  flagTable_[e_stosw] = flagInfo(list_of(x86::df), vector<Dyninst::MachRegister>());
  flagTable_[e_sub] = flagInfo(vector<Dyninst::MachRegister>(), standardFlags);
  flagTable_[e_test] = flagInfo(vector<Dyninst::MachRegister>(), standardFlags);
  flagTable_[e_verr] = flagInfo(vector<Dyninst::MachRegister>(), list_of(x86::zf));
  flagTable_[e_verw] = flagInfo(vector<Dyninst::MachRegister>(), list_of(x86::zf));
  flagTable_[e_xadd] = flagInfo(vector<Dyninst::MachRegister>(), standardFlags);
  flagTable_[e_xor] = flagInfo(vector<Dyninst::MachRegister>(), standardFlags);
  flagTable_[e_scasb] = flagInfo(list_of(x86::df), standardFlags);
  flagTable_[e_scasw] = flagInfo(list_of(x86::df), standardFlags);
  flagTable_[e_scasd] = flagInfo(list_of(x86::df), standardFlags);
  flagTable_[e_pcmpestri] = flagInfo(vector<Dyninst::MachRegister>(), standardFlags);
  flagTable_[e_pcmpestrm] = flagInfo(vector<Dyninst::MachRegister>(), standardFlags);
  flagTable_[e_pcmpistri] = flagInfo(vector<Dyninst::MachRegister>(), standardFlags);
  flagTable_[e_pcmpistrm] = flagInfo(vector<Dyninst::MachRegister>(), standardFlags);
  flagTable_[e_popcnt] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::af)(x86::cf)(x86::pf), vector<Dyninst::MachRegister>());
  flagTable_[e_ptest] = flagInfo(vector<Dyninst::MachRegister>(), standardFlags);

//  flagTable_[e_ptest] = flagInfo(list_of(x86::of)(x86::sf)(x86::zf)(x86::af)(x86::cf)(x86::pf), vector<Dyninst::MachRegister>());
}

bool ia32_entry::flagsUsed(std::set<MachRegister>& flagsRead, std::set<MachRegister>& flagsWritten, ia32_locations* locs)
{
  dyn_hash_map<entryID, flagInfo>::const_iterator found = ia32_instruction::getFlagTable().find(getID(locs));
  if(found == ia32_instruction::getFlagTable().end())
  {
    return false;
  }
  // No entries for something that touches no flags, so always return true if we had an entry

  copy((found->second).readFlags.begin(), (found->second).readFlags.end(), inserter(flagsRead, flagsRead.begin()));
  copy((found->second).writtenFlags.begin(), (found->second).writtenFlags.end(), inserter(flagsWritten,flagsWritten.begin()));
  return true;
}



// Modded table entry for push/pop, daa, das, aaa, aas, insb/w/d, outsb/w/d, xchg, cbw
// les, lds, aam, aad, loop(z/nz), cmpxch, lss, mul, imul, div, idiv, cmpxch8, [ld/st]mxcsr
// clflush, prefetch*


/**
 * This is generally the first table in the decoding process. The row selected here
 * is just based on the current byte we are looking at in the instruction. This
 * table contains a lot of the really basic and most common x86 instructions.
 */
static ia32_entry oneByteMap[256] = {
  /* 00 */
  { e_add,  t_done, 0, true, { Eb, Gb, Zz }, 0, s1RW2R, 0 },
  { e_add,  t_done, 0, true, { Ev, Gv, Zz }, 0, s1RW2R, 0 },
  { e_add,  t_done, 0, true, { Gb, Eb, Zz }, 0, s1RW2R, 0 },
  { e_add,  t_done, 0, true, { Gv, Ev, Zz }, 0, s1RW2R, 0 },
  { e_add,  t_done, 0, false, { AL, Ib, Zz }, 0, s1RW2R, 0 },
  { e_add,  t_done, 0, false, { eAX, Iz, Zz }, 0, s1RW2R, 0 },
  { e_push, t_done, 0, false, { ES, eSP, Zz }, 0, s1R2RW, s2I }, // Semantics rewritten to ignore stack "operand"
  { e_pop,  t_done, 0, false, { ES, eSP, Zz }, 0, s1W2RW, s2I },
  /* 08 */
  { e_or,   t_done, 0, true, { Eb, Gb, Zz }, 0, s1RW2R, 0},
  { e_or,   t_done, 0, true, { Ev, Gv, Zz }, 0, s1RW2R, 0 },
  { e_or,   t_done, 0, true, { Gb, Eb, Zz }, 0, s1RW2R, 0 },
  { e_or,   t_done, 0, true, { Gv, Ev, Zz }, 0, s1RW2R, 0 },
  { e_or,   t_done, 0, false, { AL, Ib, Zz }, 0, s1RW2R, 0 },
  { e_or,   t_done, 0, false, { rAX, Iz, Zz }, 0, s1RW2R, 0 },
  { e_push, t_done, 0, false, { CS, eSP, Zz }, 0, s1R2RW, s2I },
  { e_No_Entry,      t_twoB, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  /* 10 */
  { e_adc,  t_done, 0, true, { Eb, Gb, Zz }, 0, s1RW2R, 0 },
  { e_adc,  t_done, 0, true, { Ev, Gv, Zz }, 0, s1RW2R, 0 },
  { e_adc,  t_done, 0, true, { Gb, Eb, Zz }, 0, s1RW2R, 0 },
  { e_adc,  t_done, 0, true, { Gv, Ev, Zz }, 0, s1RW2R, 0 },
  { e_adc,  t_done, 0, false, { AL, Ib, Zz }, 0, s1RW2R, 0 },
  { e_adc,  t_done, 0, false, { rAX, Iz, Zz }, 0, s1RW2R, 0 },
  { e_push, t_done, 0, false, { SS, eSP, Zz }, 0, s1R2RW, s2I },
  { e_pop,  t_done, 0, false, { SS, eSP, Zz }, 0, s1W2RW, s2I },
  /* 18 */
  { e_sbb,  t_done, 0, true, { Eb, Gb, Zz }, 0, s1RW2R, 0 },
  { e_sbb,  t_done, 0, true, { Ev, Gv, Zz }, 0, s1RW2R, 0 },
  { e_sbb,  t_done, 0, true, { Gb, Eb, Zz }, 0, s1RW2R, 0 },
  { e_sbb,  t_done, 0, true, { Gv, Ev, Zz }, 0, s1RW2R, 0 },
  { e_sbb,  t_done, 0, false, { AL, Ib, Zz }, 0, s1RW2R, 0 },
  { e_sbb,  t_done, 0, false, { rAX, Iz, Zz }, 0, s1RW2R, 0 },
  { e_push, t_done, 0, false, { DS, eSP, Zz }, 0, s1R2RW, s2I },
  { e_pop , t_done, 0, false, { DS, eSP, Zz }, 0, s1W2RW, s2I },
  /* 20 */
  { e_and, t_done, 0, true, { Eb, Gb, Zz }, 0, s1RW2R, 0 },
  { e_and, t_done, 0, true, { Ev, Gv, Zz }, 0, s1RW2R, 0 },
  { e_and, t_done, 0, true, { Gb, Eb, Zz }, 0, s1RW2R, 0 },
  { e_and, t_done, 0, true, { Gv, Ev, Zz }, 0, s1RW2R, 0 },
  { e_and, t_done, 0, false, { AL, Ib, Zz }, 0, s1RW2R, 0 },
  { e_and, t_done, 0, false, { rAX, Iz, Zz }, 0, s1RW2R, 0 },
  { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }, // PREFIX_SEG_OVR
  { e_daa, t_done, 0, false, { AL, Zz, Zz }, 0, s1RW, s1I },
  /* 28 */
  { e_sub, t_done, 0, true, { Eb, Gb, Zz }, 0, s1RW2R, 0 },
  { e_sub, t_done, 0, true, { Ev, Gv, Zz }, 0, s1RW2R, 0 },
  { e_sub, t_done, 0, true, { Gb, Eb, Zz }, 0, s1RW2R, 0 },
  { e_sub, t_done, 0, true, { Gv, Ev, Zz }, 0, s1RW2R, 0 },
  { e_sub, t_done, 0, false, { AL, Ib, Zz }, 0, s1RW2R, 0 },
  { e_sub, t_done, 0, false, { rAX, Iz, Zz }, 0, s1RW2R, 0 },
  { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }, // PREFIX_SEG_OVR
  { e_das , t_done, 0, false, { AL, Zz, Zz }, 0, s1RW, s1I },
  /* 30 */
  { e_xor, t_done, 0, true, { Eb, Gb, Zz }, 0, s1RW2R, 0 },
  { e_xor, t_done, 0, true, { Ev, Gv, Zz }, 0, s1RW2R, 0 },
  { e_xor, t_done, 0, true, { Gb, Eb, Zz }, 0, s1RW2R, 0 },
  { e_xor, t_done, 0, true, { Gv, Ev, Zz }, 0, s1RW2R, 0 },
  { e_xor, t_done, 0, false, { AL, Ib, Zz }, 0, s1RW2R, 0 },
  { e_xor, t_done, 0, false, { rAX, Iz, Zz }, 0, s1RW2R, 0 },
  { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }, // PREFIX_SEG_OVR
  { e_aaa, t_done, 0, false, { AX, Zz, Zz }, 0, s1RW, s1I },
  /* 38 */
  { e_cmp, t_done, 0, true, { Eb, Gb, Zz }, 0, s1R2R, 0 },
  { e_cmp, t_done, 0, true, { Ev, Gv, Zz }, 0, s1R2R, 0 },
  { e_cmp, t_done, 0, true, { Gb, Eb, Zz }, 0, s1R2R, 0 },
  { e_cmp, t_done, 0, true, { Gv, Ev, Zz }, 0, s1R2R, 0 },
  { e_cmp, t_done, 0, false, { AL, Ib, Zz }, 0, s1R2R, 0 },
  { e_cmp, t_done, 0, false, { rAX, Iz, Zz }, 0, s1R2R, 0 },
  { e_No_Entry,     t_ill,  0, false, { Zz, Zz, Zz }, 0, 0, 0 }, // PREFIX_SEG_OVR
  { e_aas, t_done, 0, false, { AX, Zz, Zz }, 0, s1RW, s1I },
  /* 40 */
  { e_inc, t_done, 0, false, { eAX, Zz, Zz }, 0, s1RW, 0 },
  { e_inc, t_done, 0, false, { eCX, Zz, Zz }, 0, s1RW, 0 },
  { e_inc, t_done, 0, false, { eDX, Zz, Zz }, 0, s1RW, 0 },
  { e_inc, t_done, 0, false, { eBX, Zz, Zz }, 0, s1RW, 0 },
  { e_inc, t_done, 0, false, { eSP, Zz, Zz }, 0, s1RW, 0 },
  { e_inc, t_done, 0, false, { eBP, Zz, Zz }, 0, s1RW, 0 },
  { e_inc, t_done, 0, false, { eSI, Zz, Zz }, 0, s1RW, 0 },
  { e_inc, t_done, 0, false, { eDI, Zz, Zz }, 0, s1RW, 0 },
  /* 48 */
  { e_dec, t_done, 0, false, { eAX, Zz, Zz }, 0, s1RW, 0 },
  { e_dec, t_done, 0, false, { eCX, Zz, Zz }, 0, s1RW, 0 },
  { e_dec, t_done, 0, false, { eDX, Zz, Zz }, 0, s1RW, 0 },
  { e_dec, t_done, 0, false, { eBX, Zz, Zz }, 0, s1RW, 0 },
  { e_dec, t_done, 0, false, { eSP, Zz, Zz }, 0, s1RW, 0 },
  { e_dec, t_done, 0, false, { eBP, Zz, Zz }, 0, s1RW, 0 },
  { e_dec, t_done, 0, false, { eSI, Zz, Zz }, 0, s1RW, 0 },
  { e_dec, t_done, 0, false, { eDI, Zz, Zz }, 0, s1RW, 0 },
  /* 50 */
  { e_push, t_done, 0, false, { rAX, eSP, Zz }, 0, s1R2RW, s2I },
  { e_push, t_done, 0, false, { rCX, eSP, Zz }, 0, s1R2RW, s2I },
  { e_push, t_done, 0, false, { rDX, eSP, Zz }, 0, s1R2RW, s2I },
  { e_push, t_done, 0, false, { rBX, eSP, Zz }, 0, s1R2RW, s2I },
  { e_push, t_done, 0, false, { rSP, eSP, Zz }, 0, s1R2RW, s2I },
  { e_push, t_done, 0, false, { rBP, eSP, Zz }, 0, s1R2RW, s2I },
  { e_push, t_done, 0, false, { rSI, eSP, Zz }, 0, s1R2RW, s2I },
  { e_push, t_done, 0, false, { rDI, eSP, Zz }, 0, s1R2RW, s2I },
  /* 58 */
  { e_pop, t_done, 0, false, { rAX, eSP, Zz }, 0, s1W2RW, s2I },
  { e_pop, t_done, 0, false, { rCX, eSP, Zz }, 0, s1W2RW, s2I },
  { e_pop, t_done, 0, false, { rDX, eSP, Zz }, 0, s1W2RW, s2I },
  { e_pop, t_done, 0, false, { rBX, eSP, Zz }, 0, s1W2RW, s2I },
  { e_pop, t_done, 0, false, { rSP, eSP, Zz }, 0, s1W2RW, s2I },
  { e_pop, t_done, 0, false, { rBP, eSP, Zz }, 0, s1W2RW, s2I },
  { e_pop, t_done, 0, false, { rSI, eSP, Zz }, 0, s1W2RW, s2I },
  { e_pop, t_done, 0, false, { rDI, eSP, Zz }, 0, s1W2RW, s2I },
  /* 60 */
  { e_pushal, t_done, 0, false, { GPRS, eSP, Zz }, 0, s1R2RW, s2I },
  { e_popaw,  t_done, 0, false, { GPRS, eSP, Zz }, 0, s1W2RW, s2I },
  { e_bound, t_done, 0, true, { Gv, Ma, Zz }, 0, s1R2R, 0 }, // or VEX
  { e_arpl, t_done, 0, true, { Ew, Gw, Zz }, 0, s1R2R, 0 }, /* No REX */
  { e_No_Entry,          t_ill,  0, false, { Zz, Zz, Zz }, 0, 0, 0 }, // PREFIX_SEG_OVR
  { e_No_Entry,          t_ill,  0, false, { Zz, Zz, Zz }, 0, 0, 0 }, // PREFIX_SEG_OVR
  { e_No_Entry,          t_ill,  2, false, { Zz, Zz, Zz }, 0, 0, 0 }, /* operand size prefix (PREFIX_OPR_SZ) (depricated: prefixedSSE)*/
  { e_No_Entry,          t_ill,  0, false, { Zz, Zz, Zz }, 0, 0, 0 }, /* address size prefix (PREFIX_ADDR_SZ)*/
  /* 68 */
  { e_push,    t_done, 0, false, { Iz, eSP, Zz }, 0, s1R2RW, s2I },
  { e_imul,    t_done, 0, true, { Gv, Ev, Iz }, 0, s1W2R3R, 0 },
  { e_push,    t_done, 0, false, { Ib, eSP, Zz }, 0, s1R2RW, s2I },
  { e_imul,    t_done, 0, true, { Gv, Ev, Ib }, 0, s1W2R3R, 0 },
  { e_insb,    t_done, 0, false, { Yb, DX, Zz }, 0, s1W2R | (fREP << FPOS), 0 }, // (e)SI/DI changed
  { e_insd,  t_done, 0, false, { Yv, DX, Zz }, 0, s1W2R | (fREP << FPOS), 0 },
  { e_outsb,   t_done, 0, false, { DX, Xb, Zz }, 0, s1W2R | (fREP << FPOS), 0 },
  { e_outsd, t_done, 0, false, { DX, Xv, Zz }, 0, s1W2R | (fREP << FPOS), 0 },
  /* 70 */
  { e_jo,         t_done, 0, false, { Jb, Zz, Zz }, (IS_JCC | REL_B), s1R, 0 },
  { e_jno,        t_done, 0, false, { Jb, Zz, Zz }, (IS_JCC | REL_B), s1R, 0 },
  { e_jb_jnaej_j, t_done, 0, false, { Jb, Zz, Zz }, (IS_JCC | REL_B), s1R, 0 },
  { e_jnb_jae_j,  t_done, 0, false, { Jb, Zz, Zz }, (IS_JCC | REL_B), s1R, 0 },
  { e_je,         t_done, 0, false, { Jb, Zz, Zz }, (IS_JCC | REL_B), s1R, 0 },
  { e_jne,        t_done, 0, false, { Jb, Zz, Zz }, (IS_JCC | REL_B), s1R, 0 },
  { e_jbe,        t_done, 0, false, { Jb, Zz, Zz }, (IS_JCC | REL_B), s1R, 0 },
  { e_ja,       t_done, 0, false, { Jb, Zz, Zz }, (IS_JCC | REL_B), s1R, 0 },
  /* 78 */
  { e_js,   t_done, 0, false, { Jb, Zz, Zz }, (IS_JCC | REL_B), s1R, 0 },
  { e_jns,  t_done, 0, false, { Jb, Zz, Zz }, (IS_JCC | REL_B), s1R, 0 },
  { e_jp,   t_done, 0, false, { Jb, Zz, Zz }, (IS_JCC | REL_B), s1R, 0 },
  { e_jnp,  t_done, 0, false, { Jb, Zz, Zz }, (IS_JCC | REL_B), s1R, 0 },
  { e_jl,   t_done, 0, false, { Jb, Zz, Zz }, (IS_JCC | REL_B), s1R, 0 },
  { e_jge,  t_done, 0, false, { Jb, Zz, Zz }, (IS_JCC | REL_B), s1R, 0 },
  { e_jle,  t_done, 0, false, { Jb, Zz, Zz }, (IS_JCC | REL_B), s1R, 0 },
  { e_jg, t_done, 0, false, { Jb, Zz, Zz }, (IS_JCC | REL_B), s1R, 0 },
  /* 80 */
  { e_No_Entry, t_grp, Grp1a, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_grp, Grp1b, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_grp, Grp1c, true, { Zz, Zz, Zz }, 0, 0, 0 }, // book says Grp1 however;sandpile.org agrees.
  { e_No_Entry, t_grp, Grp1d, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_test, t_done, 0, true, { Eb, Gb, Zz }, 0, s1R2R, 0 },
  { e_test, t_done, 0, true, { Ev, Gv, Zz }, 0, s1R2R, 0 },
  { e_xchg, t_done, 0, true, { Eb, Gb, Zz }, 0, s1RW2RW, 0 },
  { e_xchg, t_done, 0, true, { Ev, Gv, Zz }, 0, s1RW2RW, 0 },
  /* 88 */
  { e_mov, t_done, 0, true, { Eb, Gb, Zz }, 0, s1W2R, 0 },
  { e_mov, t_done, 0, true, { Ev, Gv, Zz }, 0, s1W2R, 0 },
  { e_mov, t_done, 0, true, { Gb, Eb, Zz }, 0, s1W2R, 0 },
  { e_mov, t_done, 0, true, { Gv, Ev, Zz }, 0, s1W2R, 0 },
  { e_mov, t_done, 0, true, { Ew, Sw, Zz }, 0, s1W2R, 0 },
  { e_lea, t_done, 0, true, { Gv, Mlea, Zz }, IS_NOP, s1W2R, 0 }, // this is just M in the book
                                                        // AFAICT the 2nd operand is not accessed
  { e_mov, t_done, 0, true, { Sw, Ew, Zz }, 0, s1W2R, 0 },
  { e_pop, t_done, 0, true, { Ev, eSP, Zz }, 0, s1W2RW, s2I }, // or VEX XOP
  /* 90 */
  { e_nop,  t_done, 0, false, { Zz, Zz, Zz }, IS_NOP, sNONE, 0 }, // actually xchg eax,eax
  { e_xchg, t_done, 0, false, { rCX, rAX, Zz }, 0, s1RW2RW, 0 },
  { e_xchg, t_done, 0, false, { rDX, rAX, Zz }, 0, s1RW2RW, 0 },
  { e_xchg, t_done, 0, false, { rBX, rAX, Zz }, 0, s1RW2RW, 0 },
  { e_xchg, t_done, 0, false, { rSP, rAX, Zz }, 0, s1RW2RW, 0 },
  { e_xchg, t_done, 0, false, { rBP, rAX, Zz }, 0, s1RW2RW, 0 },
  { e_xchg, t_done, 0, false, { rSI, rAX, Zz }, 0, s1RW2RW, 0 },
  { e_xchg, t_done, 0, false, { rDI, rAX, Zz }, 0, s1RW2RW, 0 },
  /* 98 */
  { e_cwde, t_done, 0, false, { eAX, Zz, Zz }, 0, s1RW, s1I },
  { e_cdq,  t_done, 0, false, { eDX, eAX, Zz }, 0, s1W2R, s1I },
  { e_call,     t_done, 0, false, { Ap, Zz, Zz }, IS_CALL | PTR_WX, s1R, 0 },
  { e_wait,     t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE, 0 },
  { e_pushf, t_done, 0, false, { Fv, rSP, Zz }, 0, s1R2RW, s2I },
  { e_popfd,  t_done, 0, false, { Fv, rSP, Zz }, 0, s1W2RW, s2I },
  { e_sahf,     t_done, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }, // FIXME Intel
  { e_lahf,     t_done, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }, // FIXME Intel
  /* A0 */
  { e_mov,   t_done, 0, false, { AL, Ob, Zz },  0, s1W2R, 0 },
  { e_mov,   t_done, 0, false, { rAX, Ov, Zz }, 0, s1W2R, 0 },
  { e_mov,   t_done, 0, false, { Ob, AL, Zz },  0, s1W2R, 0 },
  { e_mov,   t_done, 0, false, { Ov, rAX, Zz }, 0, s1W2R, 0 },
  // XXX: Xv is source, Yv is destination for movs, so they're swapped!
  { e_movsb, t_done, 0, false, { Yb, Xb, Zz },  0, s1W2R | (fREP << FPOS), 0 }, // (e)SI/DI changed
  { e_movsd, t_done, 0, false, { Yv, Xv, Zz }, 0, s1W2R | (fREP << FPOS), 0 },
  { e_cmpsb, t_done, 0, false, { Xb, Yb, Zz },  0, s1R2R | (fCMPS << FPOS), 0 },
  { e_cmpsw, t_done, 0, false, { Xv, Yv, Zz },  0, s1R2R | (fCMPS << FPOS), 0 },
  /* A8 */
  { e_test,     t_done, 0, false, { AL, Ib, Zz },  0, s1R2R, 0 },
  { e_test,     t_done, 0, false, { rAX, Iz, Zz }, 0, s1R2R, 0 },
  { e_stosb,    t_done, 0, false, { Yb, AL, Zz },  0, s1W2R | (fREP << FPOS), 0 },
  { e_stosd,  t_done, 0, false, { Yv, rAX, Zz }, 0, s1W2R | (fREP << FPOS), 0 },
  { e_lodsb,    t_done, 0, false, { AL, Xb, Zz },  0, s1W2R | (fREP << FPOS), 0 },
  { e_lodsd,    t_done, 0, false, { rAX, Xv, Zz }, 0, s1W2R | (fREP << FPOS), 0 },
  { e_scasb,    t_done, 0, false, { AL, Yb, Zz },  0, s1R2R | (fSCAS << FPOS), 0 },
  { e_scasd,  t_done, 0, false, { rAX, Yv, Zz }, 0, s1R2R | (fSCAS << FPOS), 0 },
  /* B0 */
  { e_mov, t_done, 0, false, { AL, Ib, Zz }, 0, s1W2R, 0 },
  { e_mov, t_done, 0, false, { CL, Ib, Zz }, 0, s1W2R, 0 },
  { e_mov, t_done, 0, false, { DL, Ib, Zz }, 0, s1W2R, 0 },
  { e_mov, t_done, 0, false, { BL, Ib, Zz }, 0, s1W2R, 0 },
  { e_mov, t_done, 0, false, { AH, Ib, Zz }, 0, s1W2R, 0 },
  { e_mov, t_done, 0, false, { CH, Ib, Zz }, 0, s1W2R, 0 },
  { e_mov, t_done, 0, false, { DH, Ib, Zz }, 0, s1W2R, 0 },
  { e_mov, t_done, 0, false, { BH, Ib, Zz }, 0, s1W2R, 0 },
  /* B8 */
  { e_mov, t_done, 0, false, { rAX, Iv, Zz }, 0, s1W2R, 0 },
  { e_mov, t_done, 0, false, { rCX, Iv, Zz }, 0, s1W2R, 0 },
  { e_mov, t_done, 0, false, { rDX, Iv, Zz }, 0, s1W2R, 0 },
  { e_mov, t_done, 0, false, { rBX, Iv, Zz }, 0, s1W2R, 0 },
  { e_mov, t_done, 0, false, { rSP, Iv, Zz }, 0, s1W2R, 0 },
  { e_mov, t_done, 0, false, { rBP, Iv, Zz }, 0, s1W2R, 0 },
  { e_mov, t_done, 0, false, { rSI, Iv, Zz }, 0, s1W2R, 0 },
  { e_mov, t_done, 0, false, { rDI, Iv, Zz }, 0, s1W2R, 0 },
  /* C0 */
  { e_No_Entry, t_grp, Grp2, true, { Eb, Ib, Zz }, 0, s1RW2R, 0 },
  { e_No_Entry, t_grp, Grp2, true, { Ev, Ib, Zz }, 0, s1RW2R, 0 },
  { e_ret_near, t_done, 0, false, { Iw, Zz, Zz }, (IS_RET | IS_RETC), s1R | (fNEARRET << FPOS), s1I },
  { e_ret_near, t_done, 0, false, { Zz, Zz, Zz }, (IS_RET), fNEARRET << FPOS, s1I },
  { e_les,      t_done, 0, true, { ES, Gv, Mp }, 0, s1W2W3R, 0 }, // or VEX
  { e_lds,      t_done, 0, true, { DS, Gv, Mp }, 0, s1W2W3R, 0 }, // or VEX
  { e_No_Entry, t_grp, Grp11, true, { Eb, Ib, Zz }, 0, s1W2R, 0 },
  { e_No_Entry, t_grp, Grp11, true, { Ev, Iz, Zz }, 0, s1W2R, 0 },
  /* C8 */
  { e_enter,   t_done, 0, false, { Iw, Ib, Zz }, 0, s1R2R | (fENTER << FPOS), 0 },
  { e_leave,   t_done, 0, false, { Zz, Zz, Zz }, 0, fLEAVE << FPOS, 0 },
  { e_ret_far, t_done, 0, false, { Iw, Zz, Zz }, (IS_RETF | IS_RETC), s1R | (fFARRET << FPOS), s1I },
  { e_ret_far, t_done, 0, false, { Zz, Zz, Zz }, (IS_RETF), fFARRET << FPOS, s1I },
  { e_int3,   t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE, 0 },
  { e_int,     t_done, 0, false, { Ib, Zz, Zz }, 0, s1R, 0 },
  { e_into,    t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE, 0 },
  { e_iret,    t_done, 0, false, { Zz, Zz, Zz }, (IS_RET), fIRET << FPOS, 0 },
  /* D0 */
  { e_No_Entry, t_grp, Grp2, true, { Eb, ImplImm, Zz }, 0, s1RW2R, 0 }, // const1
  { e_No_Entry, t_grp, Grp2, true, { Ev, ImplImm, Zz }, 0, s1RW2R, 0 }, // --"--
  { e_No_Entry, t_grp, Grp2, true, { Eb, CL, Zz }, 0, s1RW2R, 0 },
  { e_No_Entry, t_grp, Grp2, true, { Ev, CL, Zz }, 0, s1RW2R, 0 },
  { e_aam,  t_done, 0, false, { AX, Ib, Zz }, 0, s1RW2R, s1I | s2I },
  { e_aad,  t_done, 0, false, { AX, Ib, Zz }, 0, s1RW2R, s1I | s2I },
  { e_salc, t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE, 0 }, // undocumeted
  { e_xlatb, t_done, 0, false, { Zz, Zz, Zz }, 0, fXLAT << FPOS, 0 }, // scream
  /* D8 */
  { e_No_Entry, t_coprocEsc, GrpD8, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_coprocEsc, GrpD9, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_coprocEsc, GrpDA, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_coprocEsc, GrpDB, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_coprocEsc, GrpDC, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_coprocEsc, GrpDD, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_coprocEsc, GrpDE, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_coprocEsc, GrpDF, true, { Zz, Zz, Zz }, 0, 0, 0 },
  /* E0 */
  { e_loopne,    t_done, 0, false, { Jb, eCX, Zz }, (IS_JCC | REL_B), s1R2R, 0 },
  { e_loope,    t_done, 0, false, { Jb, eCX, Zz }, (IS_JCC | REL_B), s1R2R, 0 },
  { e_loop,     t_done, 0, false, { Jb, eCX, Zz }, (IS_JCC | REL_B), s1R2R, 0 },
  { e_jcxz_jec, t_done, 0, false, { Jb, eCX, Zz }, (IS_JCC | REL_B), s1R2R, 0 },
  { e_in,       t_done, 0, false, { AL, Ib, Zz }, 0, s1W2R | (fIO << FPOS), 0 },
  { e_in,       t_done, 0, false, { eAX, Ib, Zz }, 0, s1W2R | (fIO << FPOS), 0 },
  { e_out,      t_done, 0, false, { Ib, AL, Zz }, 0, s1W2R | (fIO << FPOS), 0 },
  { e_out,      t_done, 0, false, { Ib, eAX, Zz }, 0, s1W2R | (fIO << FPOS), 0 },
  /* E8 */
  { e_call, t_done, 0, false, { Jz, Zz, Zz }, (IS_CALL | REL_X), s1R | (fCALL << FPOS), 0 },
  { e_jmp,  t_done, 0, false, { Jz, Zz, Zz }, (IS_JUMP | REL_X), s1R, 0 },
  { e_jmp,  t_done, 0, false, { Ap, Zz, Zz }, (IS_JUMP | PTR_WX), s1R, 0 },
  { e_jmp,  t_done, 0, false, { Jb, Zz, Zz }, (IS_JUMP | REL_B), s1R, 0 },
  { e_in,   t_done, 0, false, { AL, DX, Zz }, 0, s1W2R | (fIO << FPOS), 0 },
  { e_in,   t_done, 0, false, { eAX, DX, Zz }, 0, s1W2R | (fIO << FPOS), 0 },
  { e_out,  t_done, 0, false, { DX, AL, Zz }, 0, s1W2R | (fIO << FPOS), 0 },
  { e_out,  t_done, 0, false, { DX, eAX, Zz }, 0, s1W2R | (fIO << FPOS), 0 },
  /* F0 */
  { e_No_Entry,      t_ill,  0, false, { Zz, Zz, Zz }, 0, 0, 0 }, // PREFIX_INSTR
  { e_int1, t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE, 0 }, // undocumented
  { e_No_Entry, t_ill, 3, false, { Zz, Zz, Zz }, 0, 0, 0 }, /* depricated: prefixedSSE */
  { e_No_Entry, t_ill, 1, false, { Zz, Zz, Zz }, 0, 0, 0 }, /* depricated: prefixedSSE */
  { e_hlt,  t_done, 0, false, { Zz, Zz, Zz }, PRVLGD, sNONE, 0 },
  { e_cmc,  t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE, 0 },
  { e_No_Entry, t_grp, Grp3a, true, { Zz, Zz, Zz }, 0, sNONE, 0 },
  { e_No_Entry, t_grp, Grp3b, true, { Zz, Zz, Zz }, 0, sNONE, 0 },
  /* F8 */
  { e_clc, t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE, 0 },
  { e_stc, t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE, 0 },
  { e_cli, t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE, 0 },
  { e_sti, t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE, 0 },
  { e_cld, t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE, 0 },
  { e_std, t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE, 0 },
  { e_No_Entry, t_grp, Grp4, true, { Zz, Zz, Zz }, 0, sNONE, 0 },
  { e_No_Entry, t_grp, Grp5, true, { Zz, Zz, Zz }, 0, sNONE, 0 }
};


/**
 * This table is for two byte instructions. The index for this table is
 * always the current byte. Decoding can continue into the group map, the
 * sse map or one of the three byte maps. It can also continue into the
 * sse/vex multiplexing table if there is an SSE and VEX version of the
 * same instruction.
 */
static ia32_entry twoByteMap[256] = {
  /* 00 */
  // Syscall/sysret are somewhat hacked
  // Syscall on amd64 will also read CS/SS for where to call in 32-bit mode
  { e_No_Entry, t_grp, Grp6, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_grp, Grp7, false, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_lar,        t_done, 0, true, { Gv, Ew, Zz }, 0, s1W2R | (fSEGDESC << FPOS), 0 },
  { e_lsl,        t_done, 0, true, { Gv, Ew, Zz }, 0, s1W2R | (fSEGDESC << FPOS), 0 },
  { e_No_Entry,    t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_syscall,    t_done, 0, false, { eCX, Zz, Zz }, 0, s1W, 0 }, // AMD: writes return address to eCX; for liveness, treat as hammering all
  { e_clts,       t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE, 0 },
  { e_sysret,     t_done, 0, false, { eCX, Zz, Zz }, 0, s1R, 0 }, // AMD; reads return address from eCX; unlikely to occur in Dyninst use cases but we'll be paranoid
  /* 08 */
  { e_invd,   t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE, 0 }, // only in priviledge 0, so ignored
  { e_wbinvd, t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE, 0 }, // idem
  { e_No_Entry,        t_ill,  0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_ud2,    t_done,  0, 0, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry,        t_ill,  0, 0, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_prefetch_w, t_grp, GrpAMD, true, { Zz, Zz, Zz }, 0, 0, 0 },    // AMD prefetch group
  { e_femms,       t_done,  0, false, { Zz, Zz, Zz }, 0, sNONE, 0 },  // AMD specific
  // semantic is bogus for the 1st operand - but correct for the 2nd,
  // which is the only one that can be a memory operand :)
  // fixing the 1st operand requires an extra table for the 3dnow instructions...
  { e_No_Entry,             t_3dnow, 0, true,  { Pq, Qq, Zz }, 0, s1RW2R, 0 }, // AMD 3DNow! suffixes
  /* 10 */
  { e_No_Entry, t_sse, SSE10, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSE11, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSE12, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSE13, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSE14, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSE15, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSE16, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSE17, true, { Zz, Zz, Zz }, 0, 0, 0 },
  /* 18 */
  { e_No_Entry, t_grp, Grp16, 0, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_nop, t_done, 0, true, { Ev, Zz, Zz }, IS_NOP, 0, 0 }, // 19-1F according to sandpile and AMD are NOPs with an Ev operand
  { e_nop, t_done, 0, true, { Ev, Zz, Zz }, IS_NOP, 0, 0 }, // Can we go out on a limb that the 'operand' of a NOP is never read?
  { e_nop, t_done, 0, true, { Ev, Zz, Zz }, IS_NOP, 0, 0 }, // I think we can...so nullary operand semantics, but consume the
  { e_nop, t_done, 0, true, { Ev, Zz, Zz }, IS_NOP, 0, 0 }, // mod/rm byte operand.
  { e_nop, t_done, 0, true, { Ev, Zz, Zz }, IS_NOP, 0, 0 }, // -- BW 1/08
  { e_nop, t_done, 0, true, { Ev, Zz, Zz }, IS_NOP, 0, 0 },
  { e_nop, t_done, 0, true, { Ev, Zz, Zz }, IS_NOP, 0, 0 },
  /* 20 */
  { e_mov, t_done, 0, true, { Rd, Cd, Zz }, 0, s1W2R, 0 },
  { e_mov, t_done, 0, true, { Rd, Dd, Zz }, 0, s1W2R, 0 },
  { e_mov, t_done, 0, true, { Cd, Rd, Zz }, 0, s1W2R, 0 },
  { e_mov, t_done, 0, true, { Dd, Rd, Zz }, 0, s1W2R, 0 },
  { e_mov, t_done, 0, true, { Rd, Td, Zz }, 0, s1W2R, 0 }, // actually a SSE5A
  { e_No_Entry,     t_ill,  0, 0, { Zz, Zz, Zz }, 0, 0, 0 }, // SSE5A
  { e_mov, t_done, 0, true, { Td, Rd, Zz }, 0, s1W2R, 0 },
  { e_No_Entry,     t_ill,  0, 0, { Zz, Zz, Zz }, 0, 0, 0 },
  /* 28 */
  { e_No_Entry, t_sse, SSE28, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSE29, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSE2A, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSE2B, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSE2C, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSE2D, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSE2E, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSE2F, true, { Zz, Zz, Zz }, 0, 0, 0 },
  /* 30 */
  { e_wrmsr, t_done, 0, false, { rAX, rDX, rCX }, 0, s1R2R3R, 0 },
  { e_rdtsc, t_done, 0, false, { rAX, rDX, Zz }, 0, s1W2W3R, s1I | s2I },
  { e_rdmsr, t_done, 0, false, { rAX, rDX, rCX }, 0, s1W2W3R, 0 },
  { e_rdpmc, t_done, 0, false, { rAX, rDX, rCX }, 0, s1W2W3R, 0 },
  { e_sysenter, t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE, 0 }, // XXX: fixme for kernel work
  { e_sysexit,  t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE, 0 }, // XXX: fixme for kernel work
  { e_No_Entry, t_ill, 0, 0, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_getsec, t_done, 0, false, { Zz, Zz, Zz }, 0 , sNONE, 0 },
  /* 38 */
  { e_No_Entry, t_threeB, 0, 0, { Zz, Zz, Zz }, 0, 0, 0 }, //3-Byte escape (Book Table A-4)
  { e_No_Entry, t_ill, 0, 0, { Zz, Zz, Zz }, 0 ,0, 0 },
  { e_No_Entry, t_threeB2, 0, 0, { Zz, Zz, Zz }, 0, 0, 0 }, //3-Byte escape (Book Table A-5)
  { e_No_Entry, t_ill, 0, 0, { Zz, Zz, Zz }, 0 ,0, 0 },
  { e_No_Entry, t_ill, 0, 0, { Zz, Zz, Zz }, 0 ,0, 0 },
  { e_No_Entry, t_ill, 0, 0, { Zz, Zz, Zz }, 0 ,0, 0 },
  { e_No_Entry, t_ill, 0, 0, { Zz, Zz, Zz }, 0 ,0, 0 },
  { e_No_Entry, t_ill, 0, 0, { Zz, Zz, Zz }, 0 ,0, 0 },
  /* 40 */
  { e_cmovo,   t_done, 0, true, { Gv, Ev, Zz }, 0, s1RW2R | (fCOND << FPOS), 0 },
  { e_No_Entry,  t_sse_vex_mult, SSEVEX41, false, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry,  t_sse_vex_mult, SSEVEX42, false, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_cmovae,  t_done, 0, true, { Gv, Ev, Zz }, 0, s1RW2R | (fCOND << FPOS), 0 },
  { e_No_Entry,  t_sse_vex_mult, SSEVEX44, false, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry,  t_sse_vex_mult, SSEVEX45, false, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry,  t_sse_vex_mult, SSEVEX46, false, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry,  t_sse_vex_mult, SSEVEX47, false, { Zz, Zz, Zz }, 0, 0, 0 },
  /* 48 */
  { e_cmovs,   t_done, 0, true, { Gv, Ev, Zz }, 0, s1RW2R | (fCOND << FPOS), 0 },
  { e_cmovns,  t_done, 0, true, { Gv, Ev, Zz }, 0, s1RW2R | (fCOND << FPOS), 0 },
  { e_No_Entry,  t_sse_vex_mult, SSEVEX4A, false, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry,  t_sse_vex_mult, SSEVEX4B, false, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_cmovl, t_done, 0, true, { Gv, Ev, Zz }, 0, s1RW2R | (fCOND << FPOS), 0 },
  { e_cmovge,  t_done, 0, true, { Gv, Ev, Zz }, 0, s1RW2R | (fCOND << FPOS), 0 },
  { e_cmovle,  t_done, 0, true, { Gv, Ev, Zz }, 0, s1RW2R | (fCOND << FPOS), 0 },
  { e_cmovge,  t_done, 0, true, { Gv, Ev, Zz }, 0, s1RW2R | (fCOND << FPOS), 0 },
  /* 50 */
  { e_No_Entry, t_sse, SSE50, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSE51, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSE52, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSE53, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSE54, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSE55, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSE56, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSE57, true, { Zz, Zz, Zz }, 0, 0, 0 },
  /* 58 */
  { e_No_Entry, t_sse, SSE58, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSE59, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSE5A, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSE5B, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSE5C, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSE5D, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSE5E, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSE5F, true, { Zz, Zz, Zz }, 0, 0, 0 },
  /* 60 */
  { e_No_Entry, t_sse, SSE60, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSE61, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSE62, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSE63, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSE64, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSE65, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSE66, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSE67, true, { Zz, Zz, Zz }, 0, 0, 0 },
  /* 68 */
  { e_No_Entry, t_sse, SSE68, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSE69, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSE6A, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSE6B, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSE6C, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSE6D, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSE6E, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSE6F, true, { Zz, Zz, Zz }, 0, 0, 0 },
  /* 70 */
  { e_No_Entry, t_sse, SSE70, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_grp, Grp12, false, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_grp, Grp13, false, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse_vex_mult, SSEVEX73, false, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSE74, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSE75, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSE76, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSE77, false, { Zz, Zz, Zz }, 0, 0, 0 },
  /* 78 */
  { e_No_Entry, t_sse, SSE78, 0, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSE79, 0, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSE7A, 0, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSE7B, 0, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSE7C, 0, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSE7D, 0, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSE7E, 0, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSE7F, 0, { Zz, Zz, Zz }, 0, 0, 0 },
  /* 80 */
  { e_jo,   t_done, 0, false, { Jz, Zz, Zz }, (IS_JCC | REL_X), s1R | (fCOND << FPOS), 0 },
  { e_jno,  t_done, 0, false, { Jz, Zz, Zz }, (IS_JCC | REL_X), s1R | (fCOND << FPOS), 0 },
  { e_jb,   t_done, 0, false, { Jz, Zz, Zz }, (IS_JCC | REL_X), s1R | (fCOND << FPOS), 0 },
  { e_jae,  t_done, 0, false, { Jz, Zz, Zz }, (IS_JCC | REL_X), s1R | (fCOND << FPOS), 0 },
  { e_je,   t_done, 0, false, { Jz, Zz, Zz }, (IS_JCC | REL_X), s1R | (fCOND << FPOS), 0 },
  { e_jne,  t_done, 0, false, { Jz, Zz, Zz }, (IS_JCC | REL_X), s1R | (fCOND << FPOS), 0 },
  { e_jbe,  t_done, 0, false, { Jz, Zz, Zz }, (IS_JCC | REL_X), s1R | (fCOND << FPOS), 0 },
  { e_ja, t_done, 0, false, { Jz, Zz, Zz }, (IS_JCC | REL_X), s1R | (fCOND << FPOS), 0 },
  /* 88 */
  { e_js,   t_done, 0, false, { Jz, Zz, Zz }, (IS_JCC | REL_X), s1R | (fCOND << FPOS), 0 },
  { e_jns,  t_done, 0, false, { Jz, Zz, Zz }, (IS_JCC | REL_X), s1R | (fCOND << FPOS), 0 },
  { e_jp,   t_done, 0, false, { Jz, Zz, Zz }, (IS_JCC | REL_X), s1R | (fCOND << FPOS), 0 },
  { e_jnp,  t_done, 0, false, { Jz, Zz, Zz }, (IS_JCC | REL_X), s1R | (fCOND << FPOS), 0 },
  { e_jl,   t_done, 0, false, { Jz, Zz, Zz }, (IS_JCC | REL_X), s1R | (fCOND << FPOS), 0 },
  { e_jge,  t_done, 0, false, { Jz, Zz, Zz }, (IS_JCC | REL_X), s1R | (fCOND << FPOS), 0 },
  { e_jle,  t_done, 0, false, { Jz, Zz, Zz }, (IS_JCC | REL_X), s1R | (fCOND << FPOS), 0 },
  { e_jg, t_done, 0, false, { Jz, Zz, Zz }, (IS_JCC | REL_X), s1R | (fCOND << FPOS), 0 },
  /* 90 */
  { e_No_Entry,  t_sse_vex_mult, SSEVEX90, false, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry,  t_sse_vex_mult, SSEVEX91, false, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry,  t_sse_vex_mult, SSEVEX92, false, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry,  t_sse_vex_mult, SSEVEX93, false, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_sete,   t_done, 0, true, { Eb, Zz, Zz }, 0, s1W | (fCOND << FPOS), 0 },
  { e_setne,  t_done, 0, true, { Eb, Zz, Zz }, 0, s1W | (fCOND << FPOS), 0 },
  { e_setbe,  t_done, 0, true, { Eb, Zz, Zz }, 0, s1W | (fCOND << FPOS), 0 },
  { e_seta, t_done, 0, true, { Eb, Zz, Zz }, 0, s1W | (fCOND << FPOS), 0 },
  /* 98 */
  { e_No_Entry,  t_sse_vex_mult, SSEVEX98, false, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry,  t_sse_vex_mult, SSEVEX99, false, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_setp,   t_done, 0, true, { Eb, Zz, Zz }, 0, s1W | (fCOND << FPOS), 0 },
  { e_setnp,  t_done, 0, true, { Eb, Zz, Zz }, 0, s1W | (fCOND << FPOS), 0 },
  { e_setl,   t_done, 0, true, { Eb, Zz, Zz }, 0, s1W | (fCOND << FPOS), 0 },
  { e_setge,  t_done, 0, true, { Eb, Zz, Zz }, 0, s1W | (fCOND << FPOS), 0 },
  { e_setle,  t_done, 0, true, { Eb, Zz, Zz }, 0, s1W | (fCOND << FPOS), 0 },
  { e_setg, t_done, 0, true, { Eb, Zz, Zz }, 0, s1W | (fCOND << FPOS), 0 },
  /* A0 */
  { e_push,   t_done, 0, false, { FS, eSP, Zz }, 0, s1R2RW, s2I },
  { e_pop,    t_done, 0, false, { FS, eSP, Zz }, 0, s1W2RW, s2I },
  { e_cpuid,  t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE, 0 },
  { e_bt,     t_done, 0, true, { Ev, Gv, Zz }, 0, s1R2R, 0 },
  { e_shld,   t_done, 0, true, { Ev, Gv, Ib }, 0, s1RW2R3R, 0 },
  { e_shld,   t_done, 0, true, { Ev, Gv, CL }, 0, s1RW2R3R, 0 },
  { e_No_Entry, t_ill, 0, 0, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_ill, 0, 0, { Zz, Zz, Zz }, 0, 0, 0 },
  /* A8 */
  { e_push, t_done, 0, false, { GS, eSP, Zz }, 0, s1R2RW, s2I },
  { e_pop,  t_done, 0, false, { GS, eSP, Zz }, 0, s1W2RW, s2I },
  { e_rsm,  t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE, 0 },
  { e_bts,  t_done, 0, true, { Ev, Gv, Zz }, 0, s1RW2R, 0 },
  { e_shrd, t_done, 0, true, { Ev, Gv, Ib }, 0, s1RW2R3R, 0 },
  { e_shrd, t_done, 0, true, { Ev, Gv, CL }, 0, s1RW2R3R, 0 },
  { e_No_Entry, t_grp, Grp15, 0, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_imul, t_done, 0, true, { Gv, Ev, Zz }, 0, s1RW2R, 0 },
  /* B0 */
  // Assuming this is used with LOCK prefix, the destination gets a write anyway
  // This is not the case without lock prefix, but I ignore that case
  // Also, given that the 3rd operand is a register I ignore that it may be written
  { e_cmpxchg, t_done, 0, true, { Eb, Gb, AL }, 0, s1RW2R3R | (fCMPXCH << FPOS), s2I },
  { e_cmpxchg, t_done, 0, true, { Ev, Gv, eAX }, 0, s1RW2R3R | (fCMPXCH << FPOS), s2I },
  { e_lss, t_done, 0, true, { SS, Gv, Mp }, 0, s1W2W3R, 0 },
  { e_btr, t_done, 0, true, { Ev, Gv, Zz }, 0, s1RW2R, 0 },
  { e_lfs, t_done, 0, true, { FS, Gv, Mp }, 0, s1W2W3R, 0 },
  { e_lgs, t_done, 0, true, { GS, Gv, Mp }, 0, s1W2W3R, 0 },
  { e_movzx, t_done, 0, true, { Gv, Eb, Zz }, 0, s1W2R, 0 },
  { e_movzx, t_done, 0, true, { Gv, Ew, Zz }, 0, s1W2R, 0 },
  /* B8 */
  { e_No_Entry, t_sse, SSEB8, 0, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_ud2grp10, t_ill, 0, 0, { Zz, Zz, Zz }, 0, sNONE, 0 },
  { e_No_Entry, t_grp, Grp8, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_btc, t_done, 0, true, { Ev, Gv, Zz }, 0, s1RW2R, 0 },
  { e_bsf, t_done, 0, true, { Gv, Ev, Zz }, 0, s1W2R, 0 },
  { e_No_Entry, t_sse, SSEBD, 0, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_movsx, t_done, 0, true, { Gv, Eb, Zz }, 0, s1W2R, 0 },
  { e_movsx, t_done, 0, true, { Gv, Ew, Zz }, 0, s1W2R, 0 },
  /* C0 */
  { e_xadd, t_done, 0, true, { Eb, Gb, Zz }, 0, s1RW2RW, 0 },
  { e_xadd, t_done, 0, true, { Ev, Gv, Zz }, 0, s1RW2RW, 0 },
  { e_No_Entry, t_sse, SSEC2, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_movnti , t_done, 0, true, { Ev, Gv, Zz }, 0, s1W2R | (fNT << FPOS), 0 },
  { e_No_Entry, t_sse, SSEC4, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSEC5, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSEC6, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_grp, Grp9,  true, { Zz, Zz, Zz }, 0, 0, 0 },
  /* C8 */
  { e_bswap, t_done, 0, false, { EAX, Zz, Zz }, 0, s1RW, 0 },
  { e_bswap, t_done, 0, false, { ECX, Zz, Zz }, 0, s1RW, 0 },
  { e_bswap, t_done, 0, false, { EDX, Zz, Zz }, 0, s1RW, 0 },
  { e_bswap, t_done, 0, false, { EBX, Zz, Zz }, 0, s1RW, 0 },
  { e_bswap, t_done, 0, false, { ESP, Zz, Zz }, 0, s1RW, 0 },
  { e_bswap, t_done, 0, false, { EBP, Zz, Zz }, 0, s1RW, 0 },
  { e_bswap, t_done, 0, false, { ESI, Zz, Zz }, 0, s1RW, 0 },
  { e_bswap, t_done, 0, false, { EDI, Zz, Zz }, 0, s1RW, 0 },
  /* D0 */
  { e_No_Entry, t_sse, SSED0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSED1, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSED2, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSED3, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSED4, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSED5, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSED6, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSED7, true, { Zz, Zz, Zz }, 0, 0, 0 },
  /* D8 */
  { e_No_Entry, t_sse, SSED8, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSED9, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSEDA, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSEDB, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSEDC, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSEDD, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSEDE, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSEDF, true, { Zz, Zz, Zz }, 0, 0, 0 },
  /* E0 */
  { e_No_Entry, t_sse, SSEE0, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSEE1, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSEE2, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSEE3, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSEE4, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSEE5, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSEE6, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSEE7, true, { Zz, Zz, Zz }, 0, 0, 0 },
  /* E8 */
  { e_No_Entry, t_sse, SSEE8, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSEE9, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSEEA, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSEEB, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSEEC, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSEED, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSEEE, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSEEF, true, { Zz, Zz, Zz }, 0, 0, 0 },
  /* F0 */
  { e_No_Entry, t_sse, SSEF0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSEF1, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSEF2, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSEF3, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSEF4, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSEF5, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSEF6, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSEF7, true, { Zz, Zz, Zz }, 0, 0, 0 },
  /* F8 */
  { e_No_Entry, t_sse, SSEF8, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSEF9, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSEFA, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSEFB, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSEFC, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSEFD, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSEFE, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_sse, SSEFF, false, { Zz, Zz, Zz }, 0, 0, 0 }
};

/**
 * This table is very similar to the twoByteMap. This table just holds
 * three byte instructions. Decoding can progress through this table
 * into sseMapBis if the current instruction is an SSE instruction.
 * If the current instruction also has an SSE and VEX version, decoding
 * can progress into the sseMapBisMult table.
 */
static ia32_entry threeByteMap[256] = {
		/* 00 */
		{ e_No_Entry, t_sse_bis, SSEB00, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB01, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB02, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB03, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB04, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB05, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB06, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB07, true, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 08*/
		{ e_No_Entry, t_sse_bis, SSEB08, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB09, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB0A, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB0B, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB0C, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB0D, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB0E, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB0F, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 10 */
		{ e_No_Entry, t_sse_bis, SSEB10, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB11, true, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis, SSEB12, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB13, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB14, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB15, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB16, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB17, true, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 18 */
		{ e_No_Entry, t_sse_bis, SSEB18, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB19, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB1A, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB1B, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB1C, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB1D, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB1E, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB1F, true, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 20 */
		{ e_No_Entry, t_sse_bis, SSEB20, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB21, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB22, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB23, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB24, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB25, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB26, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB27, true, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 28 */
		{ e_No_Entry, t_sse_bis, SSEB28, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB29, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB2A, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB2B, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB2C, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB2D, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB2E, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB2F, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 30 */
		{ e_No_Entry, t_sse_bis, SSEB30, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB31, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB32, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB33, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB34, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB35, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB36, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB37, true, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 38 */
		{ e_No_Entry, t_sse_bis, SSEB38, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB39, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB3A, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB3B, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB3C, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB3D, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB3E, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB3F, true, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 40 */
		{ e_No_Entry, t_sse_bis, SSEB40, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB41, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB42, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB43, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB44, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB45, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB46, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB47, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 48 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB4C, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB4D, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB4E, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB4F, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 50 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 58 */
		{ e_No_Entry, t_sse_bis, SSEB58, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB59, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB5A, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 65 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB65, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB66, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 68 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 70 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB75, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB76, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB77, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 78 */
		{ e_No_Entry, t_sse_bis, SSEB78, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB79, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB7C, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB7D, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB7E, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB7F, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 80 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB83, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 88 */
		{ e_No_Entry, t_sse_bis, SSEB88, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB89, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB8B, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB8C, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB8D, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB8E, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 90 */
		{ e_No_Entry, t_sse_bis, SSEB90, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB91, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB92, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB93, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB96, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB97, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 98 */
		{ e_No_Entry, t_sse_bis, SSEB98, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB99, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB9A, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB9B, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB9C, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB9D, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB9E, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEB9F, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* A0 */
		{ e_No_Entry, t_sse_bis, SSEBA0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEBA1, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEBA2, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEBA3, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEBA6, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEBA7, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* A8 */
		{ e_No_Entry, t_sse_bis, SSEBA8, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEBA9, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEBAA, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEBAB, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEBAC, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEBAD, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEBAE, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEBAF, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* B0 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEBB4, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEBB5, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEBB6, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEBB7, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* B8 */
		{ e_No_Entry, t_sse_bis, SSEBB8, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEBB9, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEBBA, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEBBB, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEBBC, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEBBD, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEBBE, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEBBF, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* C0 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEBC4, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEBC6, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEBC7, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* C8 */
		{ e_No_Entry, t_sse_bis, SSEBC8, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_sha1msg1, t_done, 0, true, { Wps, Vps, Zz }, 0, s1RW2R, 0 },
		{ e_No_Entry, t_sse_bis, SSEBCA, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEBCB, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEBCC, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEBCD, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* D0 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* D8 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEBDB, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEBDC, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEBDD, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEBDE, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEBDF, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* E0 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* E8 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* F0 */
		{ e_No_Entry, t_sse_bis, SSEBF0, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEBF1, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEBF2, true, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_grp, Grp17, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEBF5, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEBF6, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_bis, SSEBF7, true, { Zz, Zz, Zz }, 0, 0, 0 },
		/* F8 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
};

/**
 * This table is very similar to the twoByteMap. This table just holds
 * three byte instructions. Decoding can progress through this table
 * into sseMapBis if the current instruction is an SSE instruction.
 * If the current instruction also has an SSE and VEX version, decoding
 * can progress into the sseMapBisMult table.
 */

static ia32_entry threeByteMap2[256] = {
		/* 00 */
		{ e_No_Entry, t_sse_ter, SSET00, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_ter, SSET01, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_ter, SSET02, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_ter, SSET03, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_ter, SSET04, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_ter, SSET05, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_ter, SSET06, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 08 */
		{ e_No_Entry, t_sse_ter, SSET08, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_ter, SSET09, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_ter, SSET0A, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_ter, SSET0B, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_ter, SSET0C, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_ter, SSET0D, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_ter, SSET0E, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_ter, SSET0F, true, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 10 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_ter, SSET14, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_ter, SSET15, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_ter, SSET16, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_ter, SSET17, true, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 18 */
		{ e_No_Entry, t_sse_ter, SSET18, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_ter, SSET19, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_ter, SSET1A, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_ter, SSET1B, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_ter, SSET1D, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_ter, SSET1E, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_ter, SSET1F, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 20 */
		{ e_No_Entry, t_sse_ter, SSET20, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_ter, SSET21, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_ter, SSET22, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_ter, SSET23, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_ter, SSET25, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_ter, SSET26, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_ter, SSET27, true, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 28 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 30 */
		{ e_No_Entry, t_sse_ter, SSET30, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_ter, SSET31, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_ter, SSET32, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_ter, SSET33, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 38 */
		{ e_No_Entry, t_sse_ter, SSET38, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_ter, SSET39, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_ter, SSET3A, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_ter, SSET3B, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_ter, SSET3E, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_ter, SSET3F, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 40 */
		{ e_No_Entry, t_sse_ter, SSET40, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_ter, SSET41, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_ter, SSET42, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_ter, SSET44, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_ter, SSET46, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 48 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_ter, SSET4A, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_ter, SSET4B, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_ter, SSET4C, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 50 */
		{ e_No_Entry, t_sse_ter, SSET50, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_ter, SSET51, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_ter, SSET54, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_ter, SSET55, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_ter, SSET56, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_ter, SSET57, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 58 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 60 */
		{ e_No_Entry, t_sse_ter, SSET60, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_ter, SSET61, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_ter, SSET62, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_ter, SSET63, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_ter, SSET66, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_ter, SSET67, true, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 68 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_ter, SSET69, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_fma4, FMA46A, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_fma4, FMA46B, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_fma4, FMA46D, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_fma4, FMA46F, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 70 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 78 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_fma4, FMA479, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_fma4, FMA47B, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 80 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 88 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 90 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 98 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* A0 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* A8 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* B0 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* B8 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* C0 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* C8 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_sha1rnds4, t_done, 0, true, { Wps, Vps, Ib }, 0, s1RW2R3R, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* D0 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* D8 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_sse_ter, SSETDF, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* E0 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* E8 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* F0 */
		{ e_No_Entry, t_sse_ter, SSETF0, true, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* F8 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
};

static ia32_entry fpuMap[][2][8] = {
{
    { // D8
        { e_fadd,  t_done, 0, true, { ST0, Ef, Zz }, 0, s1RW2R, s1I },
        { e_fmul,  t_done, 0, true, { ST0, Ef, Zz }, 0, s1RW2R, s1I },
        { e_fcom,  t_done, 0, true, { ST0, Ef, Zz }, 0, s1RW2R, s1I },
        { e_fcomp, t_done, 0, true, { ST0, Ef, Zz }, 0, s1RW2R, s1I }, // stack pop
        { e_fsub,  t_done, 0, true, { ST0, Ef, Zz }, 0, s1RW2R, s1I },
        { e_fsubr, t_done, 0, true, { ST0, Ef, Zz }, 0, s1RW2R, s1I },
        { e_fdiv,  t_done, 0, true, { ST0, Ef, Zz }, 0, s1RW2R, s1I },
        { e_fdivr, t_done, 0, true, { ST0, Ef, Zz }, 0, s1RW2R, s1I }
    },
    { // D8 -- IMPORTANT NOTE: the C0-FF tables in the book are interleaved from how they
        // need to appear here (and for all FPU insns).  i.e. book rows 0, 4, 1, 5, 2, 6, 3, 7 are table rows
        // 0, 1, 2, 3, 4, 5, 6, 7.
        { e_fadd,  t_done, 0, true, { ST0, Ef, Zz }, 0, s1RW2R, s1I },
        { e_fmul,  t_done, 0, true, { ST0, Ef, Zz }, 0, s1RW2R, s1I },
        { e_fcom,  t_done, 0, true, { ST0, Ef, Zz }, 0, s1RW2R, s1I },
        { e_fcomp, t_done, 0, true, { ST0, Ef, Zz }, 0, s1RW2R, s1I }, // stack pop
        { e_fsub,  t_done, 0, true, { ST0, Ef, Zz }, 0, s1RW2R, s1I },
        { e_fsubr, t_done, 0, true, { ST0, Ef, Zz }, 0, s1RW2R, s1I },
        { e_fdiv,  t_done, 0, true, { ST0, Ef, Zz }, 0, s1RW2R, s1I },
        { e_fdivr, t_done, 0, true, { ST0, Ef, Zz }, 0, s1RW2R, s1I }
    },
},
{
    { // D9 
        { e_fld,    t_done, 0, true, { ST0, Ef, Zz }, 0, s1W2R, 0 }, // stack push
        { e_fnop,   t_done, 0, true, { Zz,  Zz, Zz }, 0, sNONE, 0 },
        { e_fst,    t_done, 0, true, { Ef, ST0, Zz }, 0, s1W2R, 0 },
        { e_fstp,   t_done, 0, true, { Ef, ST0, Zz }, 0, s1W2R, 0 }, // stack pop
        { e_fldenv, t_done, 0, true, { M14, Zz, Zz }, 0, s1R, 0 },
        { e_fldcw,  t_done, 0, true, { Ew,  Zz, Zz }, 0, s1R, 0 },
        { e_fnstenv, t_done, 0, true, { M14, Zz, Zz }, 0, s1W, 0 },
        { e_fnstcw,  t_done, 0, true, { Ew,  Zz, Zz }, 0, s1W, 0 }
    },
    { // D9 
        { e_fld,    t_done, 0, true, { ST0, Ef, Zz }, 0, s1W2R, 0 }, // stack push
        { e_fxch, t_done, 0, true, { ST0, Ef, Zz }, 0, s1RW2RW, 0 },
        { e_fnop,   t_done, 0, true, { Zz,  Zz, Zz }, 0, sNONE, 0 },
        { e_fstp,  t_done, 0, true, { Ef,  ST0, Zz }, 0, sNONE, 0 },
        { e_fchs,    t_done, 0, true, { ST0, Zz, Zz }, 0, s1RW, 0 }, // FIXME: using first of group as placeholder
        { e_fld1, t_done, 0, true, { ST0, Zz, Zz }, 0, s1RW, 0 }, // FIXME: using first of group
        { e_f2xm1,   t_done, 0, true, { ST0, Zz, Zz }, 0, s1RW, 0 }, // FIXME: using first of group as placeholder
        { e_fprem,  t_done, 0, true, { ST0, ST1, Zz }, 0, s1RW2R, 0 } // FIXME: using first of group
    },
},
{
    { // DA 
        { e_fiadd,  t_done, 0, true, { ST0, Ev, Zz }, 0, s1RW2R, s1I },
        { e_fimul,  t_done, 0, true, { ST0, Ev, Zz }, 0, s1RW2R, s1I },
        { e_ficom,  t_done, 0, true, { ST0, Ev, Zz }, 0, s1RW2R, s1I },
        { e_ficomp, t_done, 0, true, { ST0, Ev, Zz }, 0, s1RW2R, s1I }, // stack pop
        { e_fisub,  t_done, 0, true, { ST0, Ev, Zz }, 0, s1RW2R, s1I },
        { e_fisubr, t_done, 0, true, { ST0, Ev, Zz }, 0, s1RW2R, s1I },
        { e_fidiv,  t_done, 0, true, { ST0, Ev, Zz }, 0, s1RW2R, s1I },
        { e_fidivr, t_done, 0, true, { ST0, Ev, Zz }, 0, s1RW2R, s1I }
    },
    { // DA
        { e_fcmovb,  t_done, 0, true, { ST0, Ef, Zz }, 0, s1RW2R, s1I },
        { e_fcmove,  t_done, 0, true, { ST0, Ef, Zz }, 0, s1RW2R, s1I },
        { e_fcmovbe, t_done, 0, true, { ST0, Ef, Zz }, 0, s1RW2R, s1I },
        { e_fcmovu, t_done, 0, true,  { ST0, Ef, Zz }, 0, s1RW2R, s1I },
        { e_No_Entry,  t_done, 0, true, { Zz, Zz, Zz }, 0, sNONE, 0 },
        { e_fucompp,  t_done, 0, true, { ST0, ST1, Zz }, 0, s1RW2RW, s1I }, // double pop
        { e_No_Entry,  t_done, 0, true, { Zz, Zz, Zz }, 0, sNONE, 0 },
        { e_No_Entry,  t_done, 0, true, { Zz, Zz, Zz }, 0, sNONE, 0 }
    },
},
{
    { // DB 
      { e_fild,   t_done, 0, true, { ST0, Ev, Zz }, 0, s1W2R, 0 },
      { e_fisttp, t_done, 0, true, { Ev, ST0, Zz }, 0, s1W2R, 0 }, //stack pop
      { e_fist,   t_done, 0, true, { Ev, ST0, Zz }, 0, s1W2R, 0 },
      { e_fistp,  t_done, 0, true, { Ev, ST0, Zz }, 0, s1W2R, 0 }, // stack pop
      { e_No_Entry,  t_done, 0, true, { Zz, Zz, Zz }, 0, sNONE, 0 },
      { e_fld,    t_done, 0, true, { ST0, Ef, Zz }, 0, s1W2R, 0 },
      { e_No_Entry,  t_done, 0, true, { Zz, Zz, Zz }, 0, sNONE, 0 },
      { e_fstp,   t_done, 0, true, { Ef, ST0, Zz }, 0, s1W2R, 0 }
    },
    { // DB
        { e_fcmovnb,  t_done, 0, true, { ST0, Ef, Zz }, 0, s1RW2R, 0 },
        { e_fcmovne,  t_done, 0, true, { ST0, Ef, Zz }, 0, s1RW2R, 0 },
        { e_fcmovnbe, t_done, 0, true, { ST0, Ef, Zz }, 0, s1RW2R, 0 },
        { e_fcmovnu,  t_done, 0, true, { ST0, Ef, Zz }, 0, s1RW2R, 0 },
        { e_fp_generic,  t_done, 0, true, { Zz, Zz, Zz }, 0, sNONE, 0 }, // FIXME: needs FCLEX and FINIT in group
        { e_fucomi,  t_done, 0, true, { ST0, Ef, Zz }, 0, s1RW2R, 0 },
        { e_fcomi,  t_done, 0, true, { ST0, Ef, Zz }, 0, s1RW2R, 0 },
        { e_No_Entry,  t_done, 0, true, { Zz, Zz, Zz }, 0, sNONE, 0 },
    },
},
{
    { // DC
        { e_fadd,  t_done, 0, true, { ST0, Efd, Zz }, 0, s1RW2R, s1I },
        { e_fmul,  t_done, 0, true, { ST0, Efd, Zz }, 0, s1RW2R, s1I },
        { e_fcom,  t_done, 0, true, { ST0, Efd, Zz }, 0, s1RW2R, s1I },
        { e_fcomp, t_done, 0, true, { ST0, Efd, Zz }, 0, s1RW2R, s1I }, // stack pop
        { e_fsub,  t_done, 0, true, { ST0, Efd, Zz }, 0, s1RW2R, s1I },
        { e_fsubr, t_done, 0, true, { ST0, Efd, Zz }, 0, s1RW2R, s1I },
        { e_fdiv,  t_done, 0, true, { ST0, Efd, Zz }, 0, s1RW2R, s1I },
        { e_fdivr, t_done, 0, true, { ST0, Efd, Zz }, 0, s1RW2R, s1I }
    },
    { // DC
        { e_fadd,  t_done, 0, true, { Efd, ST0, Zz }, 0, s1RW2R, 0 },
        { e_fmul,  t_done, 0, true, { Efd, ST0, Zz }, 0, s1RW2R, 0 },
        { e_fcom,  t_done, 0, true, { Efd, ST0, Zz }, 0, s1RW2R, 0 },
        { e_fcomp,  t_done, 0, true, { ST0, Efd, Zz }, 0, s1RW2R, 0 },
        { e_fsubr,  t_done, 0, true, { Efd, ST0, Zz }, 0, s1RW2R, 0 },
        { e_fsub,  t_done, 0, true, { Efd, ST0, Zz }, 0, s1RW2R, 0 },
        { e_fdivr,  t_done, 0, true, { Efd, ST0, Zz }, 0, s1RW2R, 0 },
        { e_fdiv,  t_done, 0, true, { Efd, ST0, Zz }, 0, s1RW2R, 0 }
    },
},
{
    { // DD TODO semantics check
        { e_fld,    t_done, 0, true, { ST0, Efd, Zz }, 0, s1W2R, 0 },
        { e_fisttp, t_done, 0, true, { Mq, ST0, Zz }, 0, s1W2R, 0 },
        { e_fst,    t_done, 0, true, { Efd, ST0, Zz }, 0, s1W2R, 0 },
        { e_fstp,   t_done, 0, true, { Efd, ST0, Zz }, 0, s1W2R, 0 }, // stack pop
        { e_frstor, t_done, 0, true, { M512, Zz, Zz }, 0, s1R, 0 },
        { e_fucomp, t_done, 0, true, { ST0, Efd, Zz }, 0, s1R2R, 0 }, // stack pop
        { e_fnsave,  t_done, 0, true, { M512, Zz, Zz }, 0, s1W, 0 },
        { e_fnstsw,  t_done, 0, true, { Ew, Zz, Zz }, 0, s1W, 0 }
    },
    { // DD TODO semantics check
        { e_ffree,    t_done, 0, true, { Efd, Zz, Zz }, 0, s1W, 0 },
        { e_fxch,  t_done, 0, true, { ST0, Ef, Zz }, 0, s1RW2RW, 0 },
        { e_fst, t_done, 0, true, { Efd, ST0, Zz }, 0, s1W2R, 0 },
        { e_fstp, t_done, 0, true, { Efd, ST0, Zz }, 0, s1W2RW, 0 },
        { e_fucom,    t_done, 0, true, { ST0, Efd, Zz }, 0, s1R2R, 0 },
        { e_fucomp,    t_done, 0, true, { ST0, Efd, Zz }, 0, s1RW2R, 0 },
        { e_No_Entry,  t_done, 0, true, { Zz, Zz, Zz }, 0, sNONE, 0 },
        { e_No_Entry,  t_done, 0, true, { Zz, Zz, Zz }, 0, sNONE, 0 },
    },
},
{    
    { // DE 
        { e_fiadd,  t_done, 0, true, { ST0, Ev, Zz }, 0, s1RW2R, 0 },
        { e_fimul,  t_done, 0, true, { ST0, Ev, Zz }, 0, s1RW2R, 0 },
        { e_ficom,  t_done, 0, true, { ST0, Ev, Zz }, 0, s1RW2R, 0 },
        { e_ficomp, t_done, 0, true, { ST0, Ev, Zz }, 0, s1RW2R, 0 }, // stack pop
        { e_fisub,  t_done, 0, true, { ST0, Ev, Zz }, 0, s1RW2R, 0 },
        { e_fisubr, t_done, 0, true, { ST0, Ev, Zz }, 0, s1RW2R, 0 },
        { e_fidiv,  t_done, 0, true, { ST0, Ev, Zz }, 0, s1RW2R, 0 },
        { e_fidivr, t_done, 0, true, { ST0, Ev, Zz }, 0, s1RW2R, 0 }
    },
    { // DE
        { e_faddp,  t_done, 0, true, { Ef, ST0, Zz }, 0, s1RW2R, 0 },
        { e_fmulp,  t_done, 0, true, { Ef, ST0, Zz }, 0, s1RW2R, 0 },
        { e_fcomp,  t_done, 0, true, { ST0, Ef, Zz }, 0, s1RW2R, 0 },
        { e_fcompp, t_done, 0, true, { ST0, Ef, Zz }, 0, s1RW2R, 0 },
        { e_fsubrp,  t_done, 0, true, { Ef, ST0, Zz }, 0, s1RW2R, 0 },
        { e_fsubp,  t_done, 0, true, { Ef, ST0, Zz }, 0, s1RW2R, 0 },
        { e_fdivrp, t_done, 0, true, { Ef, ST0, Zz }, 0, s1RW2R, 0 }, // stack pop
        { e_fdivp, t_done, 0, true, { Ef, ST0, Zz }, 0, s1RW2R, 0 }
    },
},
{
    { // DF TODO semantics/operand sizes
        { e_fild,   t_done, 0, true, { ST0, Ew, Zz }, 0, s1W2R, 0 },
        { e_fisttp, t_done, 0, true, { Ew, ST0, Zz }, 0, s1W2R, 0 },
        { e_fist,   t_done, 0, true, { Ew, ST0, Zz }, 0, s1W2R, 0 },
        { e_fistp,  t_done, 0, true, { Ew, ST0, Zz }, 0, s1W2R, 0 }, // stack pop
        { e_fbld,   t_done, 0, true, { ST0, Mq, Zz }, 0, s1W2R, 0 }, // BCD 80 bit
        { e_fild,   t_done, 0, true, { ST0, Ev, Zz }, 0, s1W2R, 0 },
        { e_fbstp,  t_done, 0, true, { Mq, ST0, Zz }, 0, s1RW2R, 0 },// BCD 80 bit
        { e_fistp,  t_done, 0, true, { Ev, ST0, Zz }, 0, s1W2R, 0 }
    },
    { // DF TODO semantics/operand sizes
        { e_ffreep,  t_done, 0, true, { Ef, Zz, Zz }, 0, sNONE, 0 },
        { e_fxch,  t_done, 0, true, { ST0, Ef, Zz }, 0, s1RW2RW, 0 },
        { e_fstp,  t_done, 0, true, { Ef, ST0, Zz }, 0, sNONE, 0 },
        { e_fstp,  t_done, 0, true, { Ef, ST0, Zz }, 0, sNONE, 0 },
        { e_fnstsw,   t_done, 0, true, { AX, Zz, Zz }, 0, s1W, 0 },
        { e_fucompi,  t_done, 0, true, { ST0, Ef, Zz }, 0, s1RW2R, 0 }, // stack pop
        { e_fcompi,  t_done, 0, true, { ST0, Ef, Zz }, 0, s1RW2R, 0 }, // stack pop
        { e_No_Entry,  t_done, 0, true, { Zz, Zz, Zz }, 0, sNONE, 0 },
    }
}
};

/**
 * This is one of the more complicated tables. Each row in this table has
 * multiple entries. The row that is selected is based off of the previous
 * table the decoder was in. The current byte is broken down as a ModR/M byte.
 * Here is a quick overview of how a ModR/M byte is broken down:
 *
 * +-----+-------+-------+
 * | 8 7 | 6 5 4 | 3 2 1 |
 * +-----+-------+-------+
 * Mod   Reg     R/M
 *
 *
 * We only care about the `Reg` part of the ModR/M byte. This value is used as
 * the indexer for this table. For example, if you have a byte `0x3F`:
 *
 *
 * Value in Hex: 0x3F
 * Value in Bin: 0 0 1 1 1 1 1 1
 * Reg bits:         1 1 1
 * Reg value: 7
 *
 *
 * Therefore we can see that we should choose entry 7 in this row (for this example).
 *
 */
static ia32_entry groupMap[][8] = {
  { /* group 1a */
    { e_add, t_done, 0, true, { Eb, Ib, Zz }, 0, s1RW2R, 0 },
    { e_or,  t_done, 0, true, { Eb, Ib, Zz }, 0, s1RW2R, 0 },
    { e_adc, t_done, 0, true, { Eb, Ib, Zz }, 0, s1RW2R, 0 },
    { e_sbb, t_done, 0, true, { Eb, Ib, Zz }, 0, s1RW2R, 0 },
    { e_and, t_done, 0, true, { Eb, Ib, Zz }, 0, s1RW2R, 0 },
    { e_sub, t_done, 0, true, { Eb, Ib, Zz }, 0, s1RW2R, 0 },
    { e_xor, t_done, 0, true, { Eb, Ib, Zz }, 0, s1RW2R, 0 },
    { e_cmp, t_done, 0, true, { Eb, Ib, Zz }, 0, s1R2R, 0 },
  },
  { /* group 1b */
    { e_add, t_done, 0, true, { Ev, Iz, Zz }, 0, s1RW2R, 0 },
    { e_or,  t_done, 0, true, { Ev, Iz, Zz }, 0, s1RW2R, 0 },
    { e_adc, t_done, 0, true, { Ev, Iz, Zz }, 0, s1RW2R, 0 },
    { e_sbb, t_done, 0, true, { Ev, Iz, Zz }, 0, s1RW2R, 0 },
    { e_and, t_done, 0, true, { Ev, Iz, Zz }, 0, s1RW2R, 0 },
    { e_sub, t_done, 0, true, { Ev, Iz, Zz }, 0, s1RW2R, 0 },
    { e_xor, t_done, 0, true, { Ev, Iz, Zz }, 0, s1RW2R, 0 },
    { e_cmp, t_done, 0, true, { Ev, Iz, Zz }, 0, s1R2R, 0 },
  },
  { /* group 1c */
    { e_add, t_done, 0, true, { Eb, Ib, Zz }, 0, s1RW2R, 0 },
    { e_or,  t_done, 0, true, { Eb, Ib, Zz }, 0, s1RW2R, 0 },
    { e_adc, t_done, 0, true, { Eb, Ib, Zz }, 0, s1RW2R, 0 },
    { e_sbb, t_done, 0, true, { Eb, Ib, Zz }, 0, s1RW2R, 0 },
    { e_and, t_done, 0, true, { Eb, Ib, Zz }, 0, s1RW2R, 0 },
    { e_sub, t_done, 0, true, { Eb, Ib, Zz }, 0, s1RW2R, 0 },
    { e_xor, t_done, 0, true, { Eb, Ib, Zz }, 0, s1RW2R, 0 },
    { e_cmp, t_done, 0, true, { Eb, Ib, Zz }, 0, s1R2R, 0 },
  },
  { /* group 1d */
    { e_add, t_done, 0, true, { Ev, Ib, Zz }, 0, s1RW2R, 0 },
    { e_or,  t_done, 0, true, { Ev, Ib, Zz }, 0, s1RW2R, 0 },
    { e_adc, t_done, 0, true, { Ev, Ib, Zz }, 0, s1RW2R, 0 },
    { e_sbb, t_done, 0, true, { Ev, Ib, Zz }, 0, s1RW2R, 0 },
    { e_and, t_done, 0, true, { Ev, Ib, Zz }, 0, s1RW2R, 0 },
    { e_sub, t_done, 0, true, { Ev, Ib, Zz }, 0, s1RW2R, 0 },
    { e_xor, t_done, 0, true, { Ev, Ib, Zz }, 0, s1RW2R, 0 },
    { e_cmp, t_done, 0, true, { Ev, Ib, Zz }, 0, s1R2R, 0 },
  },


 {  /* group 2 - only opcode is defined here, 
       operands are defined in the one or two byte maps above */
  { e_rol, t_done, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_ror, t_done, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_rcl, t_done, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_rcr, t_done, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_shl, t_done, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_shr, t_done, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_shl, t_done, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_sar, t_done, 0, true, { Zz, Zz, Zz }, 0, 0, 0 }
 },

 { /* group 3a - operands are defined here */
  { e_test, t_done, 0, true, { Eb, Ib, Zz }, 0, s1R2R, 0 },
  { e_test, t_done, 0, true, { Eb, Ib, Zz }, 0, s1R2R, 0 }, // book swears this is illegal, sandpile claims it's an aliased TEST
  { e_not,  t_done, 0, true, { Eb, Zz, Zz }, 0, s1RW, 0 },
  { e_neg,  t_done, 0, true, { Eb, Zz, Zz }, 0, s1RW, 0 },
  { e_mul,  t_done, 0, true, { AX, AL, Eb }, 0, s1W2R3R, 0 },
  { e_imul, t_done, 0, true, { AX, AL, Eb }, 0, s1W2R3R, 0 },
  { e_div,  t_done, 0, true, { AX, AL, Eb }, 0, s1RW2R3R, s1I },
  { e_idiv, t_done, 0, true, { AX, AL, Eb }, 0, s1RW2R3R, s1I }
 },

 { /* group 3b - operands are defined here */
  { e_test, t_done, 0, true, { Ev, Iz, Zz }, 0, s1R2R, 0 },
  { e_test, t_done, 0, true, { Ev, Iz, Zz }, 0, s1R2R, 0 }, // book swears this is illegal, sandpile claims it's an aliased TEST
  { e_not,  t_done, 0, true, { Ev, Zz, Zz }, 0, s1RW, 0 },
  { e_neg,  t_done, 0, true, { Ev, Zz, Zz }, 0, s1RW, 0 },
  { e_mul,  t_done, 0, true, { eDX, eAX, Ev }, 0, s1W2RW3R, 0 },
  { e_imul, t_done, 0, true, { eDX, eAX, Ev }, 0, s1W2RW3R, 0 },
  { e_div,  t_done, 0, true, { eDX, eAX, Ev }, 0, s1RW2RW3R, s1I },
  { e_idiv, t_done, 0, true, { eDX, eAX, Ev }, 0, s1RW2RW3R, s1I }
 },

 { /* group 4 - operands are defined here */
  { e_inc, t_done, 0, true, { Eb, Zz, Zz }, 0, s1RW, 0 },
  { e_dec, t_done, 0, true, { Eb, Zz, Zz }, 0, s1RW, 0 },
  { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
 },

 { /* group 5 - operands are defined here */
  { e_inc,  t_done, 0, true, { Ev, Zz, Zz }, 0, s1RW, 0 },
  { e_dec,  t_done, 0, true, { Ev, Zz, Zz }, 0, s1RW, 0 },
  { e_call, t_done, 0, true, { Ev, Zz, Zz }, (IS_CALL | INDIR), s1R | (fINDIRCALL << FPOS), 0 },
  { e_call, t_done, 0, true, { Ep, Zz, Zz }, (IS_CALL | INDIR), s1R | (fINDIRCALL << FPOS), 0 },
  { e_jmp,  t_done, 0, true, { Ev, Zz, Zz }, (IS_JUMP | INDIR), s1R | (fINDIRJUMP << FPOS), 0 },
  { e_jmp,  t_done, 0, true, { Ep, Zz, Zz }, (IS_JUMP | INDIR), s1R | (fINDIRJUMP << FPOS), 0 },
  { e_push, t_done, 0, true, { Ev, eSP, Zz }, 0, s1R2RW, s2I },
  { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
 },

 { /* group 6 - operands are defined here */
   // these need to be fixed for kernel mode accesses
  { e_sldt, t_done, 0, true, { Ew, Zz, Zz }, 0, s1W, 0 },
  { e_str,  t_done, 0, true, { Ew, Zz, Zz }, 0, s1W, 0 },
  { e_lldt, t_done, 0, true, { Ew, Zz, Zz }, 0, s1R, 0 },
  { e_ltr,  t_done, 0, true, { Ew, Zz, Zz }, 0, s1R, 0 },
  { e_verr, t_done, 0, true, { Ew, Zz, Zz }, 0, s1R, 0 },
  { e_verw, t_done, 0, true, { Ew, Zz, Zz }, 0, s1R, 0 },
  { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
 },

 { /* group 7 - operands are defined here */
   // idem
  { e_sgdt, t_done, 0, true, { Ms, Zz, Zz }, 0, s1W, 0 },
  { e_sidt, t_done, 0, true, { Ms, Zz, Zz }, 0, s1W, 0 },
  { e_lgdt, t_done, 0, true, { Ms, Zz, Zz }, 0, s1R, 0 },
  { e_lidt, t_done, 0, true, { Ms, Zz, Zz }, 0, s1R, 0 },
  { e_smsw, t_done, 0, true, { Ew, Zz, Zz }, 0, s1W, 0 },
  { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_lmsw, t_done, 0, true, { Ew, Zz, Zz }, 0, s1R, 0 },
  { e_invlpg, t_done, 0, true, { Zz, Zz, Zz }, 0, sNONE, 0 }, // 64-bit: swapgs also uses this encoding (w/ mod=11)
 },

 { /* group 8 - only opcode is defined here, 
     operands are defined in the one or two byte maps below */
  { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_bt,  t_done, 0, true, { Ev, Ib, Zz }, 0, s1R2R, 0 },
  { e_bts, t_done, 0, true, { Ev, Ib, Zz }, 0, s1RW2R, 0 },
  { e_btr, t_done, 0, true, { Ev, Ib, Zz }, 0, s1RW2R, 0 },
  { e_btc, t_done, 0, true, { Ev, Ib, Zz }, 0, s1RW2R, 0 },
 },

 { /* group 9 - operands are defined here */
  { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
  // see comments for cmpxch
  { e_cmpxchg8b, t_done, 0, true, { EDXEAX, Mq, ECXEBX }, 0, s1RW2RW3R | (fCMPXCH8 << FPOS), s2I },
  { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_xrstors, t_done, 0, true, { Wps, Zz, Zz }, 0, 0, 0 },
  { e_xsavec, t_done, 0, true, { Md, Zz, Zz }, s1W, 0, 0 },
  { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
  { e_rdrand, t_done, 0, true, { Ev, Zz, Zz }, 0, s1W, 0 },
  { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 }
 },

 /* group 10 is all illegal */

 { /* group 11, opcodes defined in one byte map */
   { e_mov, t_done, 0, true, { Ev, Gv, Zz }, 0, s1W2R, 0 },
   { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
   { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
   { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
   { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
   { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
   { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
   { e_xbegin, t_done, 0, false, { Jz, Zz, Zz }, 0, s1R, 0 },
 }

};


/**
 * This table is very similar to `groupMap`. Each row in this table actually
 * has 2 sub rows which contain 8 entries. The row that is selected is based
 * off of the previous table the decoder was in. The current byte is broken
 * down as a ModR/M byte. Here is a quick overview of how a ModR/M byte is
 * broken down:
 *
 *
 * +-----+-------+-------+
 * | 8 7 | 6 5 4 | 3 2 1 |
 * +-----+-------+-------+
 *  Mod   Reg     R/M
 *
 *
 * We care about two things now: the `Mod` and the `Reg`. If the `Mod` is all
 * 1's, then we will use the 2nd sub row. Otherwise we will use the first sub
 * row (sub rows here contain 8 entries each). Then the entry is selected based
 * off of the `Reg` value, just like the `groupMap` table. Here is our example
 * again of using `0x3F` as our ModR/M
 *
 *
 *
 *  Value in Hex: 0x3F
 *  Value in Bin: 0 0 1 1 1 1 1 1
 *  Reg bits:         1 1 1
 *  Mod bits:     0 0
 *  Reg value: 7
 *  Mod value: 0
 *
 *
 * Therefore we will use the first sub row, and select the 7th entry in that sub
 * row. If `Mod` would have been all 1's, we would have selected the second sub
 * row. **Note: all values of mod should map to the first sub row except for
 * 3 (0b11)**
 *
 */
static ia32_entry groupMap2[][2][8] = {
  { /* group 12 */
    {
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
    },
    {
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
      { e_No_Entry, t_grpsse, G12SSE010B, true, { Zz, Zz, Zz }, 0, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
      { e_No_Entry, t_grpsse, G12SSE100B, true, { Zz, Zz, Zz }, 0, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
      { e_No_Entry, t_grpsse, G12SSE110B, true, { Zz, Zz, Zz }, 0, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
    }
  },
  { /* group 13 */
    {
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
    },
    {
      { e_No_Entry, t_sse, SSE72, true, { Zz, Zz, Zz }, 0, 0, 0 },
      { e_No_Entry, t_sse, SSE72, true, { Zz, Zz, Zz }, 0, 0, 0 },
      { e_No_Entry, t_grpsse, G13SSE010B, true, { Zz, Zz, Zz }, 0, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
      { e_No_Entry, t_grpsse, G13SSE100B, true, { Zz, Zz, Zz }, 0, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
      { e_No_Entry, t_grpsse, G13SSE110B, true, { Zz, Zz, Zz }, 0, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
    }
  },
  { /* group 14 */
    {
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
    },
    {
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
      { e_No_Entry, t_grpsse, G14SSE010B, true, { Zz, Zz, Zz }, 0, 0, 0 },
      { e_No_Entry, t_grpsse, G14SSE011B, true, { Zz, Zz, Zz }, 0, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
      { e_No_Entry, t_grpsse, G14SSE110B, true, { Zz, Zz, Zz }, 0, 0, 0 },
      { e_No_Entry, t_grpsse, G14SSE111B, true, { Zz, Zz, Zz }, 0, 0, 0 },
    }
  },
  { /* group 15 */
    {
      { e_fxsave,  t_done, 0, true, { M512, Zz, Zz }, 0, s1W | (fFXSAVE << FPOS), 0 },
      { e_fxrstor, t_done, 0, true, { M512, Zz, Zz }, 0, s1R | (fFXRSTOR << FPOS), 0 },
      { e_ldmxcsr, t_done, 0, true, { Md, Zz, Zz }, 0, s1R, 0 },
      { e_stmxcsr, t_done, 0, true, { Md, Zz, Zz }, 0, s1W, 0 },
      { e_xsave, t_done, 0, true, { Md, Zz, Zz }, 0, s1W, 0 },
	  { e_xrstor, t_done, 0, true, { Md, Zz, Zz }, 0, s1R, 0 },
      { e_clwb, t_done, 0, true, { Mb, Zz, Zz }, 0, s1W | (fCLFLUSH << FPOS), 0 },
      { e_clflush, t_done, 0, true, { Mb, Zz, Zz }, 0, s1W | (fCLFLUSH << FPOS), 0 },
    },
    {
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
      { e_lfence, t_done, 0, true, { Zz, Zz, Zz }, 0, sNONE, 0 },
      { e_mfence, t_done, 0, true, { Zz, Zz, Zz }, 0, sNONE, 0 },
      { e_sfence, t_done, 0, true, { Zz, Zz, Zz }, 0, sNONE, 0 },
    }
  },
  { /* group 16 */
    {
      { e_prefetchnta, t_done, 0, true, { Mb, Zz, Zz }, 0, s1R | (fPREFETCHNT << FPOS), 0 },
      { e_prefetcht0,  t_done, 0, true, { Mb, Zz, Zz }, 0, s1R | (fPREFETCHT0 << FPOS), 0 },
      { e_prefetcht1,  t_done, 0, true, { Mb, Zz, Zz }, 0, s1R | (fPREFETCHT1 << FPOS), 0 },
      { e_prefetcht2,  t_done, 0, true, { Mb, Zz, Zz }, 0, s1R | (fPREFETCHT1 << FPOS), 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
    },
    {
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
    }
  },
  { /* group 17 */
    {
      { e_extrq, t_done, 0, true, { Vdq, Ib, Ib }, 0, s1RW2R3R, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
      { e_No_Entry, t_sse_mult, SSE78_66, true, { Zz, Zz, Zz }, 0, 0, 0 },
    },
    {
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
      { e_blsr, t_done, 0, true, { Bv, Ev, Zz }, 0, s1W2R, 0 },
      { e_blsmsk, t_done, 0, true, { Bv, Ev, Zz }, 0, s1W2R, 0 },
      { e_blsi, t_done, 0, true, { Bv, Ev, Zz }, 0, s1W2R, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
    }
  },
  { /* AMD prefetch group */
    {
      { e_prefetch,   t_done, 0, true, { Mb, Zz, Zz }, 0, s1R | (fPREFETCHAMDE << FPOS), 0 },
      { e_prefetchw,  t_done, 0, true, { Mb, Zz, Zz }, 0, s1R | (fPREFETCHAMDW << FPOS), 0 },
      { e_prefetchwt1,  t_done, 0, true, { Mb, Zz, Zz }, 0, s1R | (fPREFETCHAMDW << FPOS), 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 }, // this is reserved, not illegal, ugh...
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 }, // this is reserved, not illegal, ugh...
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 }, // this is reserved, not illegal, ugh...
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 }, // this is reserved, not illegal, ugh...
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 }, // this is reserved, not illegal, ugh...
    },
    {
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
      { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
    }
  }
};

/**
 * The purpose of this table is to allow our decoder to differentiate between
 * looking at an SSE instruction or a VEX instruction. For certain instructions,
 * there are SSE and VEX versions of the same instruction. Having a VEX prefix
 * on the instruction instead of an SSE prefix means that we have to make a
 * different decoding decision.
 *
 * Each row in this table has multiple entries. The entry that is selected is
 * based on the prefix of the instruction. If the instruction has only an SSE
 * prefix, entry 0th is selected. If the instruction has a VEX2 prefix, then
 * the 1st entry is selected. If the instruction has a VEX3 prefix, then the
 * 2nd entry is selected. Finally, if the instruction has an EVEX prefix, the
 * 3rd entry is selected.
 */
/** START_DYNINST_TABLE_VERIFICATION(sse_vex_mult_table) */
static ia32_entry sseVexMult[][4] = {
    { /* SSEVEX41 */
        { e_cmovno,  t_done, 0, true, { Gv, Ev, Zz }, 0, s1RW2R | (fCOND << FPOS), 0 },
        { e_No_Entry, t_sse, SSE41, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse, SSE41, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse, SSE41, false, { Zz, Zz, Zz }, 0, 0, 0 },
    }, { /* SSEVEX42 */
        { e_cmovb, t_done, 0, true, { Gv, Ev, Zz }, 0, s1RW2R | (fCOND << FPOS), 0 },
        { e_No_Entry, t_sse, SSE42, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse, SSE42, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse, SSE42, false, { Zz, Zz, Zz }, 0, 0, 0 },
    }, { /* SSEVEX44 */
        { e_cmove,   t_done, 0, true, { Gv, Ev, Zz }, 0, s1RW2R | (fCOND << FPOS), 0 },
        { e_No_Entry, t_sse, SSE44, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse, SSE44, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse, SSE44, false, { Zz, Zz, Zz }, 0, 0, 0 },
    }, { /* SSEVEX45 */
        { e_cmovne,  t_done, 0, true, { Gv, Ev, Zz }, 0, s1RW2R | (fCOND << FPOS), 0 },
        { e_No_Entry, t_sse, SSE45, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse, SSE45, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse, SSE45, false, { Zz, Zz, Zz }, 0, 0, 0 },
    }, { /* SSEVEX46 */
        { e_cmovbe,  t_done, 0, true, { Gv, Ev, Zz }, 0, s1RW2R | (fCOND << FPOS), 0 },
        { e_No_Entry, t_sse, SSE46, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse, SSE46, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse, SSE46, false, { Zz, Zz, Zz }, 0, 0, 0 },
    }, { /* SSEVEX47 */
        { e_cmova, t_done, 0, true, { Gv, Ev, Zz }, 0, s1RW2R | (fCOND << FPOS), 0 },
        { e_No_Entry, t_sse, SSE47, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse, SSE47, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse, SSE47, false, { Zz, Zz, Zz }, 0, 0, 0 },
    }, { /* SSEVEX4A */
        { e_cmovp,  t_done, 0, true, { Gv, Ev, Zz }, 0, s1RW2R | (fCOND << FPOS), 0 },
        { e_No_Entry, t_sse, SSE4A, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse, SSE4A, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse, SSE4A, false, { Zz, Zz, Zz }, 0, 0, 0 },
    }, { /* SSEVEX4B */
        { e_cmovnp,  t_done, 0, true, { Gv, Ev, Zz }, 0, s1RW2R | (fCOND << FPOS), 0 },
        { e_No_Entry, t_sse, SSE4B, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse, SSE4B, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse, SSE4B, false, { Zz, Zz, Zz }, 0, 0, 0 },
    }, { /* SSEVEX73 */
        { e_No_Entry, t_grp, Grp14, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse, SSE73, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse, SSE73, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse, SSE73, false, { Zz, Zz, Zz }, 0, 0, 0 },
    }, { /* SSEVEX78 */
        { e_No_Entry, t_grp, Grp17, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_mult, SSE78_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_mult, SSE78_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_mult, SSE78_66, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEVEX90 */
        { e_seto,   t_done, 0, true, { Eb, Zz, Zz }, 0, s1W | (fCOND << FPOS), 0 },
        { e_No_Entry, t_sse, SSE90, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse, SSE90, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse, SSE90, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEVEX91 */
        { e_setno,  t_done, 0, true, { Eb, Zz, Zz }, 0, s1W | (fCOND << FPOS), 0 },
        { e_No_Entry, t_sse, SSE91, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse, SSE91, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse, SSE91, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEVEX92 */
        { e_setb,   t_done, 0, true, { Eb, Zz, Zz }, 0, s1W | (fCOND << FPOS), 0 },
        { e_No_Entry, t_sse, SSE92, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse, SSE92, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse, SSE92, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEVEX93 */
        { e_setae,  t_done, 0, true, { Eb, Zz, Zz }, 0, s1W | (fCOND << FPOS), 0 },
        { e_No_Entry, t_sse, SSE93, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse, SSE93, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse, SSE93, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEVEX98 */
        { e_sets,   t_done, 0, true, { Eb, Zz, Zz }, 0, s1W | (fCOND << FPOS), 0 },
        { e_No_Entry, t_sse, SSE98, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse, SSE98, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse, SSE98, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEVEX99 */
        { e_setns,  t_done, 0, true, { Eb, Zz, Zz }, 0, s1W | (fCOND << FPOS), 0 },
        { e_No_Entry, t_sse, SSE99, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse, SSE99, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse, SSE99, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }
};
/* END_DYNINST_TABLE_VERIFICATION */

/* rows are not, F3, 66, F2 prefixed in this order (see book) */
/** START_DYNINST_TABLE_VERIFICATION(sse_table) */
static ia32_entry sseMap[][4] = {
  { /* SSE10 */
    { e_movups, t_sse_mult, SSE10_NO, true, { Vps, Wps, Zz }, 0, s1W2R, 0 },
    { e_movss,  t_sse_mult, SSE10_F3, true, { Vss, Wss, Zz }, 0, s1W2R, 0 },
    { e_movupd, t_sse_mult, SSE10_66, true, { Vpd, Wpd, Zz }, 0, s1W2R, 0 },
    { e_movsd_sse, t_sse_mult, SSE10_F2, true, { Vsd, Wsd, Zz }, 0, s1W2R, 0 },
  },
  { /* SSE11 */
    { e_movups, t_done, 0, true, { Wps, Vps, Zz }, 0, s1W2R, 0 },
    { e_movss,  t_done, 0, true, { Wss, Vss, Zz }, 0, s1W2R, 0 },
    { e_movupd, t_done, 0, true, { Wpd, Vpd, Zz }, 0, s1W2R, 0 },
    { e_movsd_sse,  t_done, 0, true, { Wsd, Vsd, Zz }, 0, s1W2R, 0 }, // Book is wrong, this is a W/V
  },
  { /* SSE12 */
    { e_movlps_movhlps, t_sse_mult, SSE12_NO, true, { Wq, Vq, Zz }, 0, s1W2R, 0 }, // FIXME: wierd 1st op
    { e_movsldup, t_sse_mult, SSE12_F3, true, { Vdq, Wdq, Zz }, 0, s1W2R, 0 },
    { e_movlpd, t_done, 0, true, { Vq, Ws, Zz }, 0, s1W2R, 0 },
    { e_movddup, t_sse_mult, SSE12_F2, true, { Vdq, Wq, Zz }, 0, s1W2R, 0 },
  },
  { /* SSE13 */
    { e_movlps, t_sse_mult, SSE13_NO, true, { Vq, Wq, Zz }, 0, s1W2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_movlpd, t_sse_mult, SSE13_66, true, { Vq, Wq, Zz }, 0, s1W2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSE14 */
    { e_unpcklps, t_sse_mult, SSE14_NO, true, { Vps, Wq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_unpcklpd, t_sse_mult, SSE14_66, true, { Vpd, Wq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSE15 */
    { e_unpckhps, t_sse_mult, SSE15_NO, true, { Vps, Wq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_unpckhpd, t_sse_mult, SSE15_66, true, { Vpd, Wq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSE16 */
    { e_movhps_movlhps, t_sse_mult, SSE16_NO, true, { Vq, Wq, Zz }, 0, s1W2R, 0 }, // FIXME: wierd 2nd op
    { e_movshdup, t_sse_mult, SSE16_F3, true, { Vdq, Wdq, Zz }, 0, s1W2R, 0 },
    { e_movhpd, t_sse_mult, SSE16_66, true, { Vq, Wq, Zz }, 0, s1W2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSE17 */
    { e_movhps, t_done, 0, true, { Wq, Vq, Zz }, 0, s1W2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_movhpd, t_done, 0, true, { Wq, Vq, Zz }, 0, s1W2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSE28 */
    { e_movaps, t_sse_mult, SSE28_NO, true, { Vps, Wps, Zz }, 0, s1W2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_movapd, t_sse_mult, SSE28_66, true, { Vpd, Wpd, Zz }, 0, s1W2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSE29 */
    { e_movaps, t_done, 0, true, { Wps, Vps, Zz }, 0, s1W2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_movapd, t_done, 0, true, { Wpd, Vpd, Zz }, 0, s1W2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSE2A */
    { e_cvtpi2ps, t_done, 0, true, { Vps, Qq, Zz }, 0, s1W2R, 0 },
    { e_cvtsi2ss, t_sse_mult, SSE2A_F3, true, { Vss, Ev, Zz }, 0, s1W2R, 0 },
    { e_cvtpi2pd, t_done, 0, true, { Vpd, Qdq, Zz }, 0, s1W2R, 0 },
    { e_cvtsi2sd, t_sse_mult, SSE2A_F2, true, { Vsd, Ev, Zz }, 0, s1W2R, 0 },
  },
  { /* SSE2B */
    { e_movntps, t_sse_mult, SSE2B_NO, true, { Wps, Vps, Zz }, 0, s1W2R | (fNT << FPOS), 0 },
    { e_movntss, t_done, 0, true, { Md, Vd, Zz }, 0, s1W2R | (fNT << FPOS), 0 },
    { e_movntpd, t_sse_mult, SSE2B_66, true, { Wpd, Vpd, Zz }, 0, s1W2R | (fNT << FPOS), 0 }, // bug in book
    { e_movntsd, t_done, 0, true, { Wq, Vq, Zz }, 0, s1W2R | (fNT << FPOS), 0 },
  },
  { /* SSE2C */
    { e_cvttps2pi, t_done, 0, true, { Qq, Wps, Zz }, 0, s1W2R, 0 },
    { e_cvttss2si, t_sse_mult, SSE2C_F3, true, { Gv, Wss, Zz }, 0, s1W2R, 0 },
    { e_cvttpd2pi, t_done, 0, true, { Qdq, Wpd, Zz }, 0, s1W2R, 0 },
    { e_cvttsd2si, t_sse_mult, SSE2C_F2, true, { Gv, Wsd, Zz }, 0, s1W2R, 0 },
  },
  { /* SSE2D */
    { e_cvtps2pi, t_done, 0, true, { Qq, Wps, Zz }, 0, s1W2R, 0 },
    { e_cvtss2si, t_sse_mult, SSE2D_F3, true, { Gv, Wss, Zz }, 0, s1W2R, 0 },
    { e_cvtpd2pi, t_done, 0, true, { Qdq, Wpd, Zz }, 0, s1W2R, 0 },
    { e_cvtsd2si, t_sse_mult, SSE2D_F2, true, { Gv, Wsd, Zz }, 0, s1W2R, 0 },
  },
  { /* SSE2E */
    { e_ucomiss, t_sse_mult, SSE2E_NO, true, { Vss, Wss, Zz }, 0, s1R2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_ucomisd, t_sse_mult, SSE2E_66, true, { Vsd, Wsd, Zz }, 0, s1R2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSE2F */
    { e_comiss, t_sse_mult, SSE2F_NO, true, { Vps, Wps, Zz }, 0, s1R2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_comisd, t_sse_mult, SSE2F_66, true, { Vsd, Wsd, Zz }, 0, s1R2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSE41 */
    { e_No_Entry, t_sse_mult, SSE41_NO, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_sse_mult, SSE41_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSE42 */
    { e_No_Entry, t_sse_mult, SSE42_NO, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_sse_mult, SSE42_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSE44 */
    { e_No_Entry, t_sse_mult, SSE44_NO, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_sse_mult, SSE44_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSE45 */
    { e_No_Entry, t_sse_mult, SSE45_NO, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_sse_mult, SSE45_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSE46 */
    { e_No_Entry, t_sse_mult, SSE46_NO, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_sse_mult, SSE46_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSE47 */
    { e_No_Entry, t_sse_mult, SSE47_NO, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_sse_mult, SSE47_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSE4A */
    { e_No_Entry, t_sse_mult, SSE4A_NO, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_sse_mult, SSE4A_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSE4B */
    { e_No_Entry, t_sse_mult, SSE4B_NO, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_sse_mult, SSE4B_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSE50 */
    { e_movmskps, t_done, 0, true, { Ed, Vps, Zz }, 0, s1W2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_movmskpd, t_done, 0, true, { Ed, Vpd, Zz }, 0, s1W2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSE51 */
    { e_sqrtps, t_sse_mult, SSE51_NO, true, { Vps, Wps, Zz }, 0, s1W2R, 0 },
    { e_sqrtss, t_sse_mult, SSE51_F3, true, { Vss, Wss, Zz }, 0, s1W2R, 0 },
    { e_sqrtpd, t_sse_mult, SSE51_66, true, { Vpd, Wpd, Zz }, 0, s1W2R, 0 },
    { e_sqrtsd, t_sse_mult, SSE51_F2, true, { Vsd, Wsd, Zz }, 0, s1W2R, 0 },
  },
  { /* SSE52 */
    { e_rsqrtps, t_done, 0, true, { Vps, Wps, Zz }, 0, s1W2R, 0 },
    { e_rsqrtss, t_done, 0, true, { Vss, Wss, Zz }, 0, s1W2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSE53 */
    { e_rcpps, t_done, 0, true, { Vps, Wps, Zz }, 0, s1W2R, 0 },
    { e_rcpss, t_done, 0, true, { Vss, Wss, Zz }, 0, s1W2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSE54 */
    { e_andps, t_sse_mult, SSE54_NO, true, { Vps, Wps, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_andpd, t_sse_mult, SSE54_66, true, { Vpd, Wpd, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSE55 */
    { e_andnps, t_sse_mult, SSE55_NO, true, { Vps, Wps, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_andnpd, t_sse_mult, SSE55_66, true, { Vpd, Wpd, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSE56 */
    { e_orps, t_sse_mult, SSE56_NO, true, { Vps, Wps, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_orpd, t_sse_mult, SSE56_66, true, { Vpd, Wpd, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSE57 */
    { e_xorps, t_sse_mult, SSE57_NO, true, { Vps, Wps, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_xorpd, t_sse_mult, SSE57_66, true, { Vpd, Wpd, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSE58 */
    { e_addps, t_sse_mult, SSE58_NO, true, { Vps, Wps, Zz }, 0, s1RW2R, 0 },
    { e_addss, t_sse_mult, SSE58_F3, true, { Vss, Wss, Zz }, 0, s1RW2R, 0 },
    { e_addpd, t_sse_mult, SSE58_66, true, { Vpd, Wpd, Zz }, 0, s1RW2R, 0 },
    { e_addsd, t_sse_mult, SSE58_F2, true, { Vsd, Wsd, Zz }, 0, s1RW2R, 0 },
  },
  { /* SSE59 */
    { e_mulps, t_sse_mult, SSE59_NO, true, { Vps, Wps, Zz }, 0, s1RW2R, 0 },
    { e_mulss, t_sse_mult, SSE59_F3, true, { Vss, Wss, Zz }, 0, s1RW2R, 0 },
    { e_mulpd, t_sse_mult, SSE59_66, true, { Vpd, Wpd, Zz }, 0, s1RW2R, 0 },
    { e_mulsd, t_sse_mult, SSE59_F2, true, { Vsd, Wsd, Zz }, 0, s1RW2R, 0 },
  },
  { /* SSE5A */
    { e_cvtps2pd, t_sse_mult, SSE5A_NO, true, { Vpd, Wps, Zz }, 0, s1W2R, 0 },
    { e_cvtss2sd, t_sse_mult, SSE5A_F3, true, { Vss, Wss, Zz }, 0, s1W2R, 0 },
    { e_cvtpd2ps, t_sse_mult, SSE5A_66, true, { Vps, Wpd, Zz }, 0, s1W2R, 0 }, // FIXME: book bug ???
    { e_cvtsd2ss, t_sse_mult, SSE5A_F2, true, { Vsd, Wsd, Zz }, 0, s1W2R, 0 },
  },
  { /* SSE5B */
    { e_cvtdq2ps, t_sse_mult, SSE5B_NO, true, { Vps, Wdq, Zz }, 0, s1W2R, 0 },
    { e_cvttps2dq, t_sse_mult, SSE5B_F3, true, { Vdq, Wps, Zz }, 0, s1W2R, 0 }, // book has this/next swapped!!!
    { e_cvtps2dq, t_sse_mult, SSE5B_66, true, { Vdq, Wps, Zz }, 0, s1W2R, 0 },  // FIXME: book bug ???
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSE5C */
    { e_subps, t_sse_mult, SSE5C_NO, true, { Vps, Wps, Zz }, 0, s1RW2R, 0 },
    { e_subss, t_sse_mult, SSE5C_F3, true, { Vss, Wss, Zz }, 0, s1RW2R, 0 },
    { e_subpd, t_sse_mult, SSE5C_66, true, { Vpd, Wpd, Zz }, 0, s1RW2R, 0 },
    { e_subsd, t_sse_mult, SSE5C_F2, true, { Vsd, Wsd, Zz }, 0, s1RW2R, 0 },
  },
  { /* SSE5D */
    { e_minps, t_sse_mult, SSE5D_NO, true, { Vps, Wps, Zz }, 0, s1RW2R, 0 },
    { e_minss, t_sse_mult, SSE5D_F3, true, { Vss, Wss, Zz }, 0, s1RW2R, 0 },
    { e_minpd, t_sse_mult, SSE5D_66, true, { Vpd, Wpd, Zz }, 0, s1RW2R, 0 },
    { e_minsd, t_sse_mult, SSE5D_F2, true, { Vsd, Wsd, Zz }, 0, s1RW2R, 0 },
  },
  { /* SSE5E */
    { e_divps, t_sse_mult, SSE5E_NO, true, { Vps, Wps, Zz }, 0, s1RW2R, 0 },
    { e_divss, t_sse_mult, SSE5E_F3, true, { Vss, Wss, Zz }, 0, s1RW2R, 0 },
    { e_divpd, t_sse_mult, SSE5E_66, true, { Vpd, Wpd, Zz }, 0, s1RW2R, 0 },
    { e_divsd, t_sse_mult, SSE5E_F2, true, { Vsd, Wsd, Zz }, 0, s1RW2R, 0 },
  },
  { /* SSE5F */
    { e_maxps, t_sse_mult, SSE5F_NO, true, { Vps, Wps, Zz }, 0, s1RW2R, 0 },
    { e_maxss, t_sse_mult, SSE5F_F3, true, { Vss, Wss, Zz }, 0, s1RW2R, 0 },
    { e_maxpd, t_sse_mult, SSE5F_66, true, { Vpd, Wpd, Zz }, 0, s1RW2R, 0 },
    { e_maxsd, t_sse_mult, SSE5F_F2, true, { Vsd, Wsd, Zz }, 0, s1RW2R, 0 },
  },
  { /* SSE60 */
    { e_punpcklbw, t_done, 0, true, { Pq, Qd, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_punpcklbw, t_sse_mult, SSE60_66, true, { Vdq, Wdq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSE61 */
    { e_punpcklwd, t_done, 0, true, { Pq, Qd, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_punpcklwd, t_sse_mult, SSE61_66, true, { Vdq, Wdq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSE62 */
    { e_punpckldq, t_done, 0, true, { Pq, Qd, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_punpckldq, t_sse_mult, SSE62_66, true, { Vdq, Wdq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSE63 */
    { e_packsswb, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_packsswb, t_sse_mult, SSE63_66, true, { Vdq, Wdq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSE64 */
    { e_pcmpgtb, t_done, 0, true, { Pq, Qq, Zz }, 0, s1R2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_pcmpgtb, t_sse_mult, SSE64_66, true, { Vdq, Wdq, Zz }, 0, s1R2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSE65 */
    { e_pcmpgtw, t_done, 0, true, { Pq, Qq, Zz }, 0, s1R2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_pcmpgtw, t_sse_mult, SSE65_66, true, { Vdq, Wdq, Zz }, 0, s1R2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSE66 */
    { e_pcmpgtd, t_done, 0, true, { Pq, Qq, Zz }, 0, s1R2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_pcmpgtd, t_sse_mult, SSE66_66, true, { Vdq, Wdq, Zz }, 0, s1R2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSE67 */
    { e_packuswb, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_packuswb, t_sse_mult, SSE67_66, true, { Vdq, Wdq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSE68 */
    { e_punpckhbw, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_punpckhbw, t_sse_mult, SSE68_66, true, { Pdq, Qdq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSE69 */
    { e_punpckhwd, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_punpckhwd, t_sse_mult, SSE69_66, true, { Pdq, Qdq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSE6A */
    { e_punpckhdq, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_punpckhdq, t_sse_mult, SSE6A_66, true, { Pdq, Qdq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSE6B */
    { e_packssdw, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_packssdw, t_sse_mult, SSE6B_66, true, { Pdq, Qdq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSE6C */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_punpcklqdq, t_sse_mult, SSE6C_66, true, { Vdq, Wdq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSE6D */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_punpckhqdq, t_sse_mult, SSE6D_66, true, { Vdq, Wdq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSE6E */
    { e_movd, t_done, 0, true, { Pd, Ev, Zz }, 0, s1W2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_movd, t_done, 0, true, { Vdq, Ev, Zz }, 0, s1W2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSE6F */
    { e_movq, t_done, 0, true, { Pq, Qq, Zz }, 0, s1W2R, 0 },
    { e_movdqu, t_sse_mult, SSE6F_F3, true, { Vdq, Wdq, Zz }, 0, s1W2R, 0 }, // book has this/next swapped!!!
    { e_movdqa, t_sse_mult, SSE6F_66, true, { Vdq, Wdq, Zz }, 0, s1W2R, 0 },
    { e_No_Entry, t_sse_mult, SSE6F_F2, false, { Zz, Zz, Zz }, 0, 0, 0 }
  },
  { /* SSE70 */
    { e_pshufw, t_done, 0, true, { Pq, Qq, Ib }, 0, s1W2R3R, 0 },
    { e_pshufhw, t_sse_mult, SSE70_F3, true, { Vdq, Wdq, Ib }, 0, s1W2R3R, 0 }, // book has this/next swapped!!!
    { e_pshufd, t_sse_mult, SSE70_66, true, { Vdq, Wdq, Ib }, 0, s1W2R3R, 0 },
    { e_pshuflw, t_sse_mult, SSE70_F2, true, { Vdq, Wdq, Ib }, 0, s1W2R3R, 0 },
  },
  { /* SSE71 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_sse_mult, SSE71_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSE72 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_sse_mult, SSE72_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSE73 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_sse_mult, SSE73_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSE74 */
    { e_pcmpeqb, t_done, 0, true, { Pq, Qq, Zz }, 0, s1R2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_pcmpeqb, t_sse_mult, SSE74_66, true, { Vdq, Wdq, Zz }, 0, s1R2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSE75 */
    { e_pcmpeqw, t_done, 0, true, { Pq, Qq, Zz }, 0, s1R2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_pcmpeqw, t_sse_mult, SSE75_66, true, { Vdq, Wdq, Zz }, 0, s1R2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSE76 */
    { e_pcmpeqd, t_done, 0, true, { Pq, Qq, Zz }, 0, s1R2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_pcmpeqd, t_sse_mult, SSE76_66, true, { Vdq, Wdq, Zz }, 0, s1R2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSE77 */
    { e_emms, t_vexl, VEXL00, false, { Zz, Zz, Zz }, 0, 0, 0 }, /* vzeroall or vzeroupper */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
  },
  { /* SSE78 */
    { e_vmread, t_sse_mult, SSE78_NO, true, { Ed, Gd, Zz }, 0, s1W2R, 0 },
    { e_No_Entry, t_sse_mult, SSE78_F3, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_sse_vex_mult, SSEVEX78, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_insertq, t_sse_mult, SSE78_F2, true, {Vdq, Wdq, Ib}, 0, s1RW2R3R4R, 0 }
  },
  { /* SSE79 */
    { e_vmwrite, t_sse_mult, SSE79_NO, true, { Ed, Gd, Zz }, 0, s1W2R, 0 },
    { e_No_Entry, t_sse_mult, SSE79_F3, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_extrq, t_sse_mult, SSE79_66, true, { Vdq, Ib, Ib }, 0, s1RW2R, 0 },
    { e_insertq, t_sse_mult, SSE79_F2, true, { Vdq, Wdq, Ib }, 0, s1RW2R3R4R, 0 },
  },
  { /* SSE7A */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_sse_mult, SSE7A_F3, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_sse_mult, SSE7A_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_sse_mult, SSE7A_F2, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSE7B */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_sse_mult, SSE7B_F3, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_sse_mult, SSE7B_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_sse_mult, SSE7B_F2, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSE7C */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_haddpd, t_done, 0, true, { Vpd, Wpd, Zz }, 0, s1RW2R, 0 },
    { e_haddps, t_done, 0, true, { Vps, Wps, Zz }, 0, s1RW2R, 0 },
  },
  { /* SSE7D */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_hsubpd, t_done, 0, true, { Vpd, Wpd, Zz }, 0, s1RW2R, 0 },
    { e_hsubps, t_done, 0, true, { Vps, Wps, Zz }, 0, s1RW2R, 0 },
  },
  { /* SSE7E */
    { e_movd, t_done, 0, true, { Ev, Pd, Zz }, 0, s1W2R, 0 },
    { e_movq, t_sse_mult, SSE7E_F3, true, { Vq, Wq, Zz }, 0, s1W2R, 0 }, // book has this and next swapped!!!
    { e_movd, t_done, 0, true, { Ev, Vdq, Zz }, 0, s1W2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSE7F */
    { e_movq, t_done, 0, true, { Qq, Pq, Zz }, 0, s1W2R, 0 },
    { e_movdqu, t_done, 0, true, { Wdq, Vdq, Zz }, 0, s1W2R, 0 }, // book has this and next swapped!!!
    { e_movdqa, t_sse_mult, SSE7F_66, true, { Wdq, Vdq, Zz }, 0, s1W2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSE90 */
    { e_No_Entry, t_sse_mult, SSE90_NO, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_sse_mult, SSE90_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
  },
  { /* SSE91 */
    { e_No_Entry, t_sse_mult, SSE91_NO, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_sse_mult, SSE91_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
  },
  { /* SSE92 */
    { e_No_Entry, t_sse_mult, SSE92_NO, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
  },
  { /* SSE93 */
    { e_No_Entry, t_sse_mult, SSE93_NO, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_sse_mult, SSE93_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_sse_mult, SSE93_F2, false, { Zz, Zz, Zz }, 0, 0, 0 }
  },
  { /* SSE98 */
    { e_No_Entry, t_sse_mult, SSE98_NO, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_sse_mult, SSE98_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
  },
  { /* SSE99 */
    { e_No_Entry, t_sse_mult, SSE99_NO, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_sse_mult, SSE99_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
  },
  { /* SSEB8 */
    { e_No_Entry, t_done, 0, false, { Zz, Zz, Zz }, 0, s1R, 0 },
    { e_popcnt, t_done, 0, true, { Gv, Ev, Zz }, 0, s1W2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSEBD */
    { e_bsr, t_done, 0, true, { Gv, Ev, Zz }, 0, s1W2R, 0 },
    { e_lzcnt, t_done, 0, true, { Gv, Ev, Zz }, 0, s1W2R, 0 },
    { e_bsr, t_done, 0, true, { Gv, Ev, Zz }, 0, s1W2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSEC2 */
    { e_cmpps, t_sse_mult, SSEC2_NO, true, { Vps, Wps, Ib }, 0, s1RW2R3R, 0 }, // comparison writes to dest!
    { e_cmpss, t_sse_mult, SSEC2_F3, true, { Vss, Wss, Ib }, 0, s1RW2R3R, 0 },
    { e_cmppd, t_sse_mult, SSEC2_66, true, { Vpd, Wpd, Ib }, 0, s1RW2R3R, 0 },
    { e_cmpsd, t_sse_mult, SSEC2_F2, true, { Vsd, Wsd, Ib }, 0, s1RW2R3R, 0 },
  },
  { /* SSEC4 */
    { e_pinsrw, t_done, 0, true, { Pq, Ed, Ib }, 0, s1RW2R3R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_pinsrw, t_sse_mult, SSEC4_66, true, { Vdq, Ed, Ib }, 0, s1RW2R3R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSEC5 */
    { e_pextrw, t_done, 0, true, { Gd, Pq, Ib }, 0, s1W2R3R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_pextrw, t_sse_mult, SSEC5_66, true, { Gd, Vdq, Ib }, 0, s1W2R3R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSEC6 */
    { e_shufps, t_sse_mult, SSEC6_NO, true, { Vps, Wps, Ib }, 0, s1RW2R3R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_shufpd, t_sse_mult, SSEC6_66, true, { Vpd, Wpd, Ib }, 0, s1RW2R3R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSED0 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_addsubpd, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R, 0 },
    { e_addsubps, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R, 0 },
  },
  { /* SSED1 */
    { e_psrlw, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_psrlw, t_sse_mult, SSED1_66, true, { Vdq, Wdq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSED2 */
    { e_psrld, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_psrld, t_sse_mult, SSED2_66, true, { Vdq, Wdq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSED3 */
    { e_psrlq, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_psrlq, t_sse_mult, SSED3_66, true, { Vdq, Wdq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSED4 */
    { e_paddq, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_paddq, t_sse_mult, SSED4_66, true, { Vdq, Wdq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSED5 */
    { e_pmullw, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_pmullw, t_sse_mult, SSED5_66, true, { Vdq, Wdq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSED6 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_movq2dq, t_done, 0, true, { Vdq, Qq, Zz }, 0, s1W2R, 0 }, // lines jumbled in book
    { e_movq, t_done, 0, true, { Wq, Vq, Zz }, 0, s1W2R, 0 },
    { e_movdq2q, t_done, 0, true, { Pq, Wq, Zz }, 0, s1W2R, 0 },
  },
  { /* SSED7 */
    { e_pmovmskb, t_done, 0, true, { Gd, Pq, Zz }, 0, s1W2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_pmovmskb, t_done, 0, true, { Gd, Vdq, Zz }, 0, s1W2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSED8 */
    { e_psubusb, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_psubusb, t_sse_mult, SSED8_66, true, { Vdq, Wdq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSED9 */
    { e_psubusw, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_psubusw, t_sse_mult, SSED9_66, true, { Vdq, Wdq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSEDA */
    { e_pminub, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_pminub, t_sse_mult, SSEDA_66, true, { Vdq, Wdq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSEDB */
    { e_pand, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_pand, t_sse_mult, SSEDB_66, true, { Vdq, Wdq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSEDC */
    { e_paddusb, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_paddusb, t_sse_mult, SSEDC_66, true, { Vdq, Wdq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSEDD */
    { e_paddusw, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_paddusw, t_sse_mult, SSEDD_66, true, { Vdq, Wdq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSEDE */
    { e_pmaxub, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_pmaxub, t_sse_mult, SSEDE_66, true, { Vdq, Wdq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSEDF */
    { e_pandn, t_sse_mult, 0, true, { Pq, Qq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_pandn, t_sse_mult, SSEDF_66, true, { Vdq, Wdq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSEE0 */
    { e_pavgb, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_pavgb, t_sse_mult, SSEE0_66, true, { Vdq, Wdq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSEE1 */
    { e_psraw, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_psraw, t_sse_mult, SSEE1_66, true, { Vdq, Wdq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSEE2 */
    { e_psrad, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_psrad, t_sse_mult, SSEE2_66, true, { Vdq, Wdq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSEE3 */
    { e_pavgw, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_pavgw, t_sse_mult, SSEE3_66, true, { Vdq, Wdq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSEE4 */
    { e_pmulhuw, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_pmulhuw, t_sse_mult, SSEE4_66, true, { Vdq, Wdq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSEE5 */
    { e_pmulhw, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_pmulhw, t_sse_mult, SSEE5_66, true, { Vdq, Wdq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSEE6 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_cvtdq2pd, t_sse_mult, SSEE6_F3, true, { Vpd, Wdq, Zz }, 0, s1W2R, 0 }, // lines jumbled in book
    { e_cvttpd2dq, t_sse_mult, SSEE6_66, true, { Vdq, Wpd, Zz }, 0, s1W2R, 0 },
    { e_cvtpd2dq, t_sse_mult, SSEE6_F2, true, { Vdq, Wpd, Zz }, 0, s1W2R, 0 },
  },
  { /* SSEE7 */
    { e_movntq, t_done, 0, true, { Wq, Vq, Zz }, 0, s1W2R | (fNT << FPOS), 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_movntdq, t_sse_mult, SSEE7_66, true, { Wdq, Vdq, Zz }, 0, s1W2R | (fNT << FPOS), 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSEE8 */
    { e_psubsb, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_psubsb, t_sse_mult, SSEE8_66, true, { Vdq, Wdq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSEE9 */
    { e_psubsw, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_psubsw, t_sse_mult, SSEE9_66, true, { Vdq, Wdq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSEEA */
    { e_pminsw, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_pminsw, t_sse_mult, SSEEA_66, true, { Vdq, Wdq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSEEB */
    { e_por, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_por, t_sse_mult, SSEEB_66, true, { Vdq, Wdq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSEEC */
    { e_paddsb, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_paddsb, t_sse_mult, SSEEC_66, true, { Vdq, Wdq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSEED */
    { e_paddsw, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_paddsw, t_sse_mult, SSEED_66, true, { Vdq, Wdq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSEEE */
    { e_pmaxsw, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_pmaxsw, t_sse_mult, SSEEE_66, true, { Vdq, Wdq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSEEF */
    { e_pxor, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_pxor, t_sse_mult, SSEEF_66, true, { Vdq, Wdq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSEF0 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_lddqu, t_done, 0, true, { Vdq, Mdq, Zz }, 0, s1W2R, 0 },
  },
  { /* SSEF1 */
    { e_psllw, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_psllw, t_sse_mult, SSEF1_66, true, { Vdq, Wdq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSEF2 */
    { e_pslld, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_pslld, t_sse_mult, SSEF2_66, true, { Vdq, Wdq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSEF3 */
    { e_psllq, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_psllq, t_sse_mult, SSEF3_66, true, { Vdq, Wdq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSEF4 */
    { e_pmuludq, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_pmuludq, t_sse_mult, SSEF4_66, true, { Vdq, Wdq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSEF5 */
    { e_pmaddwd, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_pmaddwd, t_sse_mult, SSEF5_66, true, { Vdq, Wdq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSEF6 */
    { e_psadbw, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_psadbw, t_sse_mult, SSEF6_66, true, { Vdq, Wdq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSEF7 */
    { e_maskmovq, t_done, 0, true, { Ppi, Qpi, Zz }, 0, s1W2R | (fNT << FPOS), 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_maskmovdqu, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1W2R | (fNT << FPOS), 0 }, // bug in book
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSEF8 */
    { e_psubb, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_psubb, t_sse_mult, SSEF8_66, true, { Vdq, Wdq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSEF9 */
    { e_psubw, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_psubw, t_sse_mult, SSEF9_66, true, { Vdq, Wdq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSEFA */
    { e_psubd, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_psubd, t_sse_mult, SSEFA_66, true, { Vdq, Wdq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSEFB */ // FIXME: Same????
    { e_psubd, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_psubd, t_sse_mult, SSEFB_66, true, { Vdq, Wdq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSEFC */
    { e_paddb, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_paddb, t_sse_mult, SSEFC_66, true, { Vdq, Wdq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSEFD */
    { e_paddw, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_paddw, t_sse_mult, SSEFD_66, true, { Vdq, Wdq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSEFE */
    { e_paddd, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_paddd, t_sse_mult, SSEFE_66, true, { Vdq, Wdq, Zz }, 0, s1RW2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  },
  { /* SSEFF */
    { e_ud0, t_done, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_ud0, t_done, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  }
};
/** END_DYNINST_TABLE_VERIFICATION */

/* rows are not, F3, 66, F2, 66&F2 prefixed in this order (see book) */
/** START_DYNINST_TABLE_VERIFICATION(sse_bis_table) */
static ia32_entry sseMapBis[][5] = {
    { /* SSEB00 */
        { e_pshufb, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_pshufb, t_sse_bis_mult, SSEB00_66, true, { Vdq, Wdq, Zz }, 0, s1RW2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB01 */
        { e_phaddw, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_phaddw, t_sse_bis_mult, SSEB01_66, true, { Vdq, Wdq, Zz }, 0, s1RW2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB02 */
        { e_phaddd, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_phaddd, t_sse_bis_mult, SSEB02_66, true, { Vdq, Wdq, Zz }, 0, s1RW2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB03 */
        { e_phaddsw, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_phaddsw, t_sse_bis_mult, SSEB03_66, true, { Vdq, Wdq, Zz }, 0, s1RW2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB04 */
        { e_pmaddubsw, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_pmaddubsw, t_sse_bis_mult, SSEB04_66, true, { Vdq, Wdq, Zz }, 0, s1RW2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB05 */
        { e_phsubw, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_phsubw, t_sse_bis_mult, SSEB05_66, true, { Vdq, Wdq, Zz }, 0, s1RW2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB06 */
        { e_phsubd, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_phsubd, t_sse_bis_mult, SSEB06_66, true, { Vdq, Wdq, Zz }, 0, s1RW2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB07 */
        { e_phsubsw, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_phsubsw, t_sse_bis_mult, SSEB07_66, true, { Vdq, Wdq, Zz }, 0, s1RW2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB08 */
        { e_psignb, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_psignb, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB09 */
        { e_psignw, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_psignw, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB0A */
        { e_psignd, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_psignd, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1RW2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB0B */
        { e_pmulhrsw, t_done, 0, true, { Pq, Qq, Zz }, 0, s1RW2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_pmulhrsw, t_sse_bis_mult, SSEB0B_66, true, { Vdq, Wdq, Zz }, 0, s1RW2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB0C */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_vpermilps, t_sse_bis_mult, SSEB0C_66, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB0D */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_vpermilpd, t_sse_bis_mult, SSEB0D_66, true, { Vpd, Hpd, Wpd }, 0, s1W2R3R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB0E */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_vtestps, t_done, 0, true, { Vps, Wps, Zz }, 0, s1W2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB0F */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_vtestpd, t_done, 0, true, { Vpd, Wpd, Zz }, 0, s1W2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB10 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEB10_F3, true, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_pblendvb, t_sse_bis_mult, SSEB10_66, true, { Vdq, Wdq, Zz }, 0, s1RW2R3R, s3I },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB11 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEB11_F3, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEB11_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB12 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEB12_F3, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEB12_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB13 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEB13_F3, true, { Vps, Wdq, Zz }, 0, s1W2R, 0 },
        { e_vcvtph2ps, t_sse_bis_mult, SSEB13_66, true, { Vps, Wdq, Zz }, 0, s1W2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB14 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEB14_F3, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_blendvps, t_sse_bis_mult, SSEB14_66, true, { Vdq, Wdq, Zz }, 0, s1RW2R3R, s3I },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB15 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEB15_F3, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_blendvpd, t_sse_bis_mult, SSEB15_66, true, { Vdq, Wdq, Zz }, 0, s1RW2R3R, s3I },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB16 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_vpermps, t_sse_bis_mult, SSEB16_66, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB17 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_ptest, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1R2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB18 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_vbroadcastss, t_done, 0, true, { Vss, Wss, Zz }, 0, s1W2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB19 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_vbroadcastsd, t_done, 0, true, { Vsd, Wsd, Zz }, 0, s1W2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB1A */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_vbroadcastf128, t_sse_bis_mult, SSEB1A_66, true, { Vsd, Wq, Zz }, 0, s1W2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB1B */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB1C */
        { e_pabsb, t_done, 0, true, { Pq, Qq, Zz }, 0, s1W2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_pabsb, t_sse_bis_mult, SSEB1C_66, true, { Vdq, Wdq, Zz }, 0, s1W2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB1D */
        { e_pabsw, t_done, 0, true, { Pq, Qq, Zz }, 0, s1W2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_pabsw, t_sse_bis_mult, SSEB1D_66, true, { Vdq, Wdq, Zz }, 0, s1W2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB1E */
        { e_pabsd, t_done, 0, true, { Pq, Qq, Zz }, 0, s1W2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_pabsd, t_sse_bis_mult, SSEB1E_66, true, { Vdq, Wdq, Zz }, 0, s1W2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB1F */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEB1F_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB20 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_vpmovswb,  t_done, 0, true, { Wdq, Vdq, Zz }, 0, s1W2R, 0 },
        { e_pmovsxbw, t_sse_bis_mult, SSEB20_66, true, { Vdq, Wdq, Zz }, 0, s1W2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB21 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_vpmovsdb,  t_done, 0, true, { Wdq, Vdq, Zz }, 0, s1W2R, 0 },
        { e_pmovsxbd, t_sse_bis_mult, SSEB21_66, true, { Vdq, Wdq, Zz }, 0, s1W2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB22 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_vpmovsqb,  t_done, 0, true, { Wdq, Vdq, Zz }, 0, s1W2R, 0 },
        { e_pmovsxbq, t_sse_bis_mult, SSEB22_66, true, { Vdq, Wdq, Zz }, 0, s1W2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB23 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_vpmovsdw, t_done, 0, true, { Wdq, Vdq, Zz }, 0, s1W2R, 0 },
        { e_pmovsxwd, t_sse_bis_mult, SSEB23_66, true, { Vdq, Wdq, Zz }, 0, s1W2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB24 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_vpmovsqw, t_done, 0, true, { Wdq, Vdq, Zz }, 0, s1W2R, 0 },
        { e_pmovsxwq, t_sse_bis_mult, SSEB24_66, true, { Vdq, Wdq, Zz }, 0, s1W2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB25 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_vpmovsqd, t_done, 0, true, { Wdq, Vdq, Zz }, 0, s1W2R, 0 },
        { e_pmovsxdq, t_sse_bis_mult, SSEB25_66, true, { Vdq, Wdq, Zz }, 0, s1W2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB26 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEB26_F3, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB27 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEB27_F3, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEB27_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB28 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEB28_F3, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_pmuldq, t_sse_bis_mult, SSEB28_66, true, { Vdq, Wdq, Zz }, 0, s1RW2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB29 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEB29_F3, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_pcmpeqq, t_sse_bis_mult, SSEB29_66, true, { Vdq, Wdq, Zz }, 0, s1RW2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB2A */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEB2A_F3, true, { Mdq, Wdq, Zz }, 0, s1W2R, 0 },
        { e_movntdqa, t_sse_bis_mult, SSEB2A_66, true, { Mdq, Wdq, Zz }, 0, s1W2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB2B */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_packusdw, t_sse_bis_mult, SSEB2B_66, true, { Vdq, Wdq, Zz }, 0, s1RW2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB2C */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_vmaskmovps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB2D */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_vmaskmovpd, t_done, 0, true, { Vpd, Hpd, Wpd }, 0, s1W2R3R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB2E */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_vmaskmovps, t_done, 0, true, { Wps, Hps, Vps }, 0, s1W2R3R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB2F */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_vmaskmovpd, t_done, 0, true, { Wpd, Hpd, Vpd }, 0, s1W2R3R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB30 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEB30_F3, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_pmovzxbw, t_sse_bis_mult, SSEB30_66, true, { Vdq, Wdq, Zz }, 0, s1W2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB31 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEB31_F3, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_pmovzxbd, t_sse_bis_mult, SSEB31_66, true, { Vdq, Wdq, Zz }, 0, s1W2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB32 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEB32_F3, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_pmovzxbq, t_sse_bis_mult, SSEB32_66, true, { Vdq, Wdq, Zz }, 0, s1W2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB33 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEB33_F3, true, { Vdq, Wdq, Zz }, 0, s1W2R, 0 },
        { e_pmovzxwd, t_sse_bis_mult, SSEB33_66, true, { Vdq, Wdq, Zz }, 0, s1W2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB34 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_pmovzxwq, t_sse_bis_mult, SSEB34_F3, true, { Vdq, Wdq, Zz }, 0, s1W2R, 0 },
        { e_pmovzxwq, t_sse_bis_mult, SSEB34_66, true, { Vdq, Wdq, Zz }, 0, s1W2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB35 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEB35_F3, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_pmovzxdq, t_sse_bis_mult, SSEB35_66, true, { Vdq, Wdq, Zz }, 0, s1W2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB36 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_vpermd, t_sse_bis_mult, SSEB36_66, true, { Vdq, Hdq, Wdq }, 0, s1W2R3R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB37 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_pcmpgtq, t_sse_bis_mult, SSEB37_66, true, { Vdq, Wdq, Zz }, 0, s1RW2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB38 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEB38_F3, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_pminsb, t_sse_bis_mult, SSEB38_66, true, { Vdq, Wdq, Zz }, 0, s1RW2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB39 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEB39_F3, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_pminsd, t_sse_bis_mult, SSEB39_66, true, { Vdq, Wdq, Zz }, 0, s1RW2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB3A */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_pminuw, t_sse_bis_mult, SSEB3A_66, true, { Vdq, Wdq, Zz }, 0, s1RW2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB3B */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_pminud, t_sse_bis_mult, SSEB3B_66, true, { Vdq, Wdq, Zz }, 0, s1RW2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB3C */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_pmaxsb, t_sse_bis_mult, SSEB3C_66, true, { Vdq, Wdq, Zz }, 0, s1RW2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB3D */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_pmaxsd, t_sse_bis_mult, SSEB3D_66, true, { Vdq, Wdq, Zz }, 0, s1RW2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB3E */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_pmaxuw, t_sse_bis_mult, SSEB3E_66, true, { Vdq, Wdq, Zz }, 0, s1RW2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB3F */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_pmaxud, t_sse_bis_mult, SSEB3F_66, true, { Vdq, Wdq, Zz }, 0, s1RW2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB40 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_pmulld, t_sse_bis_mult, SSEB40_66, true, { Vdq, Wdq, Zz }, 0, s1RW2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB41 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_phminposuw, t_done, 0, true, { Vdq, Wdq, Zz }, 0, s1W2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB42 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEB42_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB43 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEB43_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB44 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEB44_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB45 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEB45_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB46 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_vpsravd, t_sse_bis_mult, SSEB46_66, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB47 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEB47_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB4C */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEB4C_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB4D */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEB4D_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB4E */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEB4E_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB4F */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEB4F_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB58 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_vpbroadcastd, t_done, 0, true, { Vps, Wd, Zz }, 0, s1W2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB59 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_vpbroadcastq, t_done, 0, true, { Vps, Wq, Zz }, 0, s1W2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB5A */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_vbroadcastf128, t_done, 0, true, { Vsd, Wq, Zz }, 0, s1W2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB65 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEB65_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB66 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEB66_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB75 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEB75_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB76 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEB76_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB77 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEB77_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB78 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_vpbroadcastb, t_done, 0, true, { Vps, Wb, Zz }, 0, s1W2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB79 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_vpbroadcastw, t_done, 0, true, { Vps, Ww, Zz }, 0, s1W2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB7C */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEB7C_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB7D */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEB7D_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB7E */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEB7E_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB7F */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEB7F_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB83 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEB83_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB88 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEB88_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB89 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEB89_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB8B */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEB8B_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB8C */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEB8C_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB8D */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEB8D_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB8E */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEB8E_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB90 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEB90_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB91 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEB91_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB92 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEB92_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB93 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEB93_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB96 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEB96_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB97 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEB97_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB98 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEB98_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB99 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEB99_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB9A */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEB9A_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB9B */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEB9B_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB9C */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEB9C_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB9D */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEB9D_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB9E */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEB9E_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEB9F */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEB9F_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEBA0 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEBA0_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEBA1 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEBA1_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEBA2 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEBA2_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEBA3 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEBA3_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEBA6 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEBA6_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEBA7 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEBA7_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEBA8 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEBA8_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEBA9 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEBA9_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEBAA */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEBAA_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEBAB */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEBAB_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEBAC */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEBAC_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEBAD */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEBAD_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEBAE */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEBAE_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEBAF */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEBAF_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEBB4 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEBB4_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEBB5 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEBB5_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEBB6 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEBB6_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEBB7 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEBB7_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEBB8 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEBB8_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEBB9 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEBB9_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEBBA */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEBBA_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEBBB */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEBBB_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEBBC */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEBBC_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEBBD */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEBBD_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEBBE */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEBBE_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEBBF */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEBBF_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEBC4 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEBC4_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEBC6 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEBC6_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEBC7 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEBC7_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEBC8 */
        { e_sha1nexte, t_done, 0, true, { Wps, Vps, Zz }, 0, s1RW2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEBC8_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEBCA */
        { e_sha1msg2, t_done, 0, true, { Wps, Vps, Zz }, 0, s1RW2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEBCA_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEBCB */
        { e_sha256rnds2, t_done, 0, true, { Wps, Vps, Zz }, 0, s1RW2R, s3I },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEBCB_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEBCC */
        { e_sha256msg1, t_done, 0, true, { Wps, Vps, Zz }, 0, s1RW2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEBCC_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEBCD */
        { e_sha256msg2, t_done, 0, true, { Wps, Vps, Zz }, 0, s1RW2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEBCD_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEBDB */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_aesimc, t_sse_bis_mult, SSEBDB_66, true, { Vps, Wps, Zz }, 0, s1W2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    }, { /* SSEBDC */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_aesenc, t_sse_bis_mult, SSEBDC_66, true, { Vps, Wps, Zz }, 0, s1RW2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    }, { /* SSEBDD */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_aesenclast, t_sse_bis_mult, SSEBDD_66, true, { Vps, Wps, Zz }, 0, s1RW2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    }, { /* SSEBDE */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_aesdec, t_sse_bis_mult, SSEBDE_66, true, { Vps, Wps, Zz }, 0, s1RW2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    }, { /* SSEBDF */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_aesdeclast, t_sse_bis_mult, SSEBDF_66, true, { Vps, Wps, Zz }, 0, s1RW2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    }, { /* SSEBF0 */
        { e_movbe, t_done, 0, true, {Gv, Mv}, 0, s1W2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_movbe, t_done, 0, true, {Gv, Mv}, 0, s1W2R, 0 },
        { e_crc32, t_done, 0, true, { Gv, Eb, Zz }, 0, s1RW2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEBF1 */
        { e_movbe, t_done, 0, true, {Mv, Gv}, 0, s1W2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_movbe, t_done, 0, true, {Mv, Gv}, 0, s1W2R, 0 },
        { e_crc32, t_done, 0, true, { Vps, Wps, Zz }, 0, s1RW2R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEBF2 */
        { e_No_Entry, t_sse_bis_mult, SSEBF2_NO, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEBF5 */
        { e_No_Entry, t_sse_bis_mult, SSEBF5_NO, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEBF5_F3, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEBF5_F2, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEBF6 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEBF6_F3, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEBF6_F2, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSEBF7 */
        { e_No_Entry, t_sse_bis_mult, SSEBF7_NO, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEBF7_F3, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEBF7_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_bis_mult, SSEBF7_F2, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }
};
/** END_DYNINST_TABLE_VERIFICATION */

/* rows are not, 66, F2 prefixed in this order (see book) */
/** START_DYNINST_TABLE_VERIFICATION(sse_ter_table) */
static ia32_entry sseMapTer[][3] = 
{
    { /* SSET00 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_vpermq, t_sse_ter_mult, SSET00_66, true, { Vdq, Wqq, Ib }, 0, s1W2R3R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSET01 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_vpermpd, t_sse_ter_mult, SSET01_66, true, { Vpd, Wpd, Ib }, 0, s1W2R3R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSET02 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_vpblendd, t_done, 0, true, { Vpd, Hpd, Wpd }, 0, s1W2R3R4R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSET03 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_ter_mult, SSET03_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSET04 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_vpermilps, t_sse_ter_mult, SSET04_66, true, { Vps, Wps, Ib }, 0, s1W2R3R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSET05 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_vpermilpd, t_sse_ter_mult, SSET05_66, true, { Vpd, Wpd, Ib }, 0, s1W2R3R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSET06 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_vperm2f128, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R4R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSET08 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_ter_mult, SSET08_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_roundps, t_done, 0, true, { Vdq, Wdq, Ib }, 0, s1W2R3R, 0 }
    }, { /* SSET09 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_roundpd, t_sse_ter_mult, SSET09_66, true, { Vdq, Wdq, Ib }, 0, s1W2R3R, 0 },
        { e_roundpd, t_done, 0, true, { Vdq, Wdq, Ib }, 0, s1W2R3R, 0 }
    }, { /* SSET0A */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_ter_mult, SSET0A_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_roundss, t_done, 0, true, { Vss, Wss, Ib }, 0, s1W2R3R, 0 },
    }, { /* SSET0B */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_roundsd, t_sse_ter_mult, SSET0B_66, true, { Vsd, Wsd, Ib }, 0, s1W2R3R, 0 },
        { e_roundsd, t_done, 0, true, { Vsd, Wsd, Ib }, 0, s1W2R3R, 0 },
    }, { /* SSET0C */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_ter_mult, SSET0C_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_blendps, t_done, 0, true, { Vdq, Wdq, Ib }, 0, s1RW2R3R, 0 },
    }, { /* SSET0D */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_vexw, VEXW94, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_vexw, VEXW94, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSET0E */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_pblendw, t_done, 0, true, { Vdq, Wdq, Ib }, 0, s1RW2R3R, 0 },
        { e_pblendw, t_done, 0, true, { Vdq, Wdq, Ib }, 0, s1RW2R3R, 0 }
    }, { /* SSET0F */
        { e_palignr, t_done, 0, true, { Pq, Qq, Ib }, 0, s1RW2R3R, 0 },
        { e_palignr, t_sse_ter_mult, SSET0F_66, true, { Pq, Qq, Ib }, 0, s1RW2R3R, 0 },
        { e_palignr, t_done, 0, true, { Vdq, Wdq, Ib }, 0, s1RW2R3R, 0 },
    }, { /* SSET14 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_ter_mult, SSET14_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_pextrb, t_done, 0, true, { RMb, Vdq, Ib }, 0, s1W2R3R, 0 }
    }, { /* SSET15 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_pextrw, t_done, 0, true, { RMw, Vdq, Ib }, 0, s1W2R3R, 0 },
    }, { /* SSET16 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_pextrd_pextrq, t_sse_ter_mult, SSET16_66, true, { Ey, Vdq, Ib }, 0, s1W2R3R, 0 },
        { e_pextrd_pextrq, t_done, 0, true, { Ey, Vdq, Ib }, 0, s1W2R3R, 0 },
    }, { /* SSET17 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_vextractps, t_sse_ter_mult, SSET17_66, true, { Ev, Vps, Ib }, 0, s1W2R3R, 0 },
        { e_extractps, t_done, 0, true, { Ed, Vdq, Ib }, 0, s1W2R3R, 0 },
    }, { /* SSET18 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_vinsertf128, t_sse_ter_mult, SSET18_66, true, { Vdq, Hps, Wps }, 0, s1W2R3R4R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    }, { /* SSET19 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_vextractf128, t_sse_ter_mult, SSET19_66, true, { Wps, Vps, Ib }, 0, s1W2R3R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    }, { /* SSET1A */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_ter_mult, SSET1A_66, true, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    }, { /* SSET1B */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_ter_mult, SSET1B_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    }, { /* SSET1D */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_vcvtps2ph, t_sse_ter_mult, SSET1D_66, true, { Wps, Vps, Ib }, 0, s1W2R3R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    }, { /* SSET1E */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_ter_mult, SSET1E_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    }, { /* SSET1F */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_ter_mult, SSET1F_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    }, { /* SSET20 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_pinsrb, t_sse_ter_mult, SSET20_66, true, { Vdq, RMb, Ib }, 0, s1W2R3R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    }, { /* SSET21 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_insertps, t_sse_ter_mult, SSET21_66, true, { Vdq, UMd, Ib }, 0, s1W2R3R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    }, { /* SSET22 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_pinsrd_pinsrq, t_sse_ter_mult, SSET22_66, true, { Vdq, Ey, Ib }, 0, s1W2R3R, 0 },
         { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    }, { /* SSET23 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_pinsrd_pinsrq, t_sse_ter_mult, SSET23_66, true, { Vdq, Ey, Ib }, 0, s1W2R3R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    }, { /* SSET25 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_ter_mult, SSET25_66, true, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    }, { /* SSET26 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_ter_mult, SSET26_66, true, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    }, { /* SSET27 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_ter_mult, SSET27_66, true, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    }, { /* SSET30 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_ter_mult, SSET30_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSET31 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_ter_mult, SSET31_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSET32 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_ter_mult, SSET32_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSET33 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_ter_mult, SSET33_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSET38 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_vinserti128, t_sse_ter_mult, SSET38_66, true, { Vps, Hps, Wps }, 0, s1W2R3R4R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    }, { /* SSET39 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_vextracti128, t_sse_ter_mult, SSET39_66, true, { Wps, Vps, Ib }, 0, s1RW2R3R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    }, { /* SSET3A */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_ter_mult, SSET3A_66, true, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    }, { /* SSET3B */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_ter_mult, SSET3B_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    }, { /* SSET3E */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_ter_mult, SSET3E_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    }, { /* SSET3F */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_ter_mult, SSET3F_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    }, { /* SSET40 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_dpps, t_done, 0, true, { Vdq, Wdq, Ib }, 0, s1RW2R3R, 0 },
    }, { /* SSET41 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_vdppd, t_done, 0, true, { Vdq, Wdq, Ib }, 0, s1RW2R3R, 0 },
        { e_dppd, t_done, 0, true, { Vdq, Wdq, Ib }, 0, s1RW2R3R, 0 },
    }, { /* SSET42 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_ter_mult, SSET42_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_mpsadbw, t_done, 0, true, { Vdq, Wdq, Ib }, 0, s1RW2R3R, 0 },
    }, { /* SSET44 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_pclmulqdq, t_sse_ter_mult, SSET44_66, true, { Vps, Wps, Ib }, 0, s1RW2R3R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    }, { /* SSET46 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_vperm2i128, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R4R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSET4A */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_ter_mult, SSET4A_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    }, { /* SSET4B */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_ter_mult, SSET4B_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    }, { /* SSET4C */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_ter_mult, SSET4C_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    }, { /* SSET50 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_ter_mult, SSET50_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSET51 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_ter_mult, SSET51_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSET54 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_ter_mult, SSET54_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSET55 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_ter_mult, SSET55_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSET56 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_ter_mult, SSET56_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSET57 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_ter_mult, SSET57_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSET60 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_pcmpestrm, t_done, 0, true, { Vdq, Wdq, Ib }, 0, s1R2R3R, 0 }
    }, { /* SSET61 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_pcmpestri, t_done, 0, true, { Vdq, Wdq, Ib }, 0, s1R2R3R, 0 },
        { e_pcmpestri, t_done, 0, true, { Vdq, Wdq, Ib }, 0, s1R2R3R, 0 }
    }, { /* SSET62 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_pcmpistrm, t_done, 0, true, { Vdq, Wdq, Ib }, 0, s1R2R3R, 0 },
        { e_pcmpistrm, t_done, 0, true, { Vdq, Wdq, Ib }, 0, s1R2R3R, 0 }
    }, { /* SSET63 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_pcmpistri, t_done, 0, true, { Vdq, Wdq, Ib }, 0, s1R2R3R, 0 },
        { e_pcmpistri, t_done, 0, true, { Vdq, Wdq, Ib }, 0, s1R2R3R, 0 },
    }, { /* SSET66 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_ter_mult, SSET66_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSET67 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_ter_mult, SSET67_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSET69 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_ter_mult, SSET69_66, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSETDF */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_aeskeygenassist, t_sse_ter_mult, SSETDF_66, true, { Vps, Wps, Ib }, 0, s1W2R3R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSETF0 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_sse_ter_mult, SSETF0_F2, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }
};
/** END_DYNINST_TABLE_VERIFICATION */

/**
 * SSE multiplexer tables:
 *
 * Some instructions share the same opcode and the only way
 * to tell how many operands there are and the addressing
 * mode is by looking at which vex prefix is used. This doesn't
 * affect all sse/vex instructions so some skip this table
 *
 * We are allowed to have a VEX instruction that has a decoding
 * path that passes through one of the SSE tables (sseMap,
 * sseMapBis, sseMapTer). However, a VEX instruction cannot end
 * it's decoding in one of the SSE tables. Therefore when the
 * decoder exits one of the SSE tables when it's decoding a VEX
 * instruction, it will enter one of the SSE multiplexer tables
 * in order to get the correct decoding for the VEX prefix that
 * is on the current instruction.
 *
 * This table has 3 entries per row. The 0th entry specifies the
 * entry that should be used for VEX2 instructions. The 1st entry
 * specifies the entry that should be used for VEX3 instructions.
 * Finallly, the last entry specifies the entry that should be used
 * for EVEX instructions.
 *
 */

/* Table orders for reference */
/* SSE Table order:  NO, F3, 66, F2 */
/* BSSE Table order: NO, F3, F2, 66F2 */
/* TSSE Table order: NO, 66, F2 */

/* rows are VEX2, VEX3, or EVEX prefixed in this order */
/** START_DYNINST_TABLE_VERIFICATION(sse_vex_table) */
ia32_entry sseMapMult[][3] = 
{
  { /* SSE10_66 */
    { e_vmovupd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vmovupd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vmovupd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 }
  }, { /* SSE10_F2 */
    { e_vmovsd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vmovsd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vmovsd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSE10_F3 */
    { e_vmovss, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vmovss, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vmovss, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSE10_NO */
    { e_vmovups, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vmovups, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vmovups, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSE12_F2 */
    { e_vmovddup, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vmovddup, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vmovddup, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSE12_F3 */
    { e_vmovsldup, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vmovsldup, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vmovsldup, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSE12_NO */
    { e_vmovhlps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vmovhlps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSE13_66 */
    { e_vmovlpd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vmovlpd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSE13_NO */
    { e_vmovlps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vmovlps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSE14_66 */
    { e_vunpcklpd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vunpcklpd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vunpcklpd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSE14_NO */
    { e_vunpcklps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vunpcklps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vunpcklps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSE15_66 */
    { e_vunpckhpd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vunpckhpd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vunpckhpd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSE15_NO */
    { e_vunpckhps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vunpckhps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vunpckhps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSE16_66 */
    { e_vmovhpd, t_done, 0, true, { Vpd, Wpd, Hpd }, 0, s1W2R3R, 0 },
    { e_vmovhpd, t_done, 0, true, { Vpd, Wpd, Hpd }, 0, s1W2R3R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSE16_F3 */
    { e_vmovshdup, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vmovshdup, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vmovshdup, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSE16_NO */
    { e_vmovhps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vmovhps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vmovhps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSE28_66 */
    { e_vmovapd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vmovapd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vmovapd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSE28_NO */
    { e_vmovaps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vmovaps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vmovaps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSE2A_F2 */
    { e_vcvtsi2sd, t_done, 0, true, { Vps, Wps, Zz }, 0, s1W2R, 0 },
    { e_vcvtsi2sd, t_done, 0, true, { Vps, Wps, Zz }, 0, s1W2R, 0 },
    { e_vcvtsi2sd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSE2A_F3 */
    { e_vcvtsi2ss, t_done, 0, true, { Vps, Wps, Zz }, 0, s1W2R, 0 },
    { e_vcvtsi2ss, t_done, 0, true, { Vps, Wps, Zz }, 0, s1W2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSE2B_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vmovntpd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 }
  }, { /* SSE2B_NO */
    { e_vmovntps, t_done, 0, true, { Wps, Vps, Zz }, 0, s1W2R3R, 0 },
    { e_vmovntps, t_done, 0, true, { Wps, Vps, Zz }, 0, s1W2R3R, 0 },
    { e_vmovntps, t_done, 0, true, { Wps, Vps, Zz }, 0, s1W2R3R, 0 },
  }, { /* SSE2C_F2 */
    { e_vcvttsd2si, t_done, 0, true, { Vps, Wps, Zz }, 0, s1W2R, 0 },
    { e_vcvttsd2si, t_done, 0, true, { Vps, Wps, Zz }, 0, s1W2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSE2C_F3 */
    { e_vcvttss2si, t_done, 0, true, { Vps, Wps, Zz }, 0, s1W2R, 0 },
    { e_vcvttss2si, t_done, 0, true, { Vps, Wps, Zz }, 0, s1W2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSE2D_F2 */
    { e_vcvtsd2si, t_done, 0, true, { Vps, Wps, Zz }, 0, s1W2R, 0 },
    { e_vcvtsd2si, t_done, 0, true, { Vps, Wps, Zz }, 0, s1W2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSE2D_F3 */
    { e_vcvtss2si, t_done, 0, true, { Vps, Wps, Zz }, 0, s1W2R, 0 },
    { e_vcvtss2si, t_done, 0, true, { Vps, Wps, Zz }, 0, s1W2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSE2E_66 */
    { e_vucomisd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vucomisd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSE2E_NO */
    { e_vucomiss, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vucomiss, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSE2F_66 */
    { e_vcomisd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vcomisd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSE2F_NO */
    { e_vcomiss, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vcomiss, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSE41_66 */
    { e_kandb, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_kandd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSE41_NO */
    { e_kandw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_kandq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSE42_66 */
    { e_kandnb, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_kandnd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSE42_NO */
    { e_kandnw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_kandnq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSE44_66 */
    { e_knotb, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_knotd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSE44_NO */
    { e_knotw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_knotq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSE45_66 */
    { e_korb, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_kord, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSE45_NO */
    { e_korw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_korq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSE46_66 */
    { e_kxnorb, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_kxnord, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSE46_NO */
    { e_kxnorw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_kxnorq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSE47_66 */
    { e_kxorb, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_kxord, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSE47_NO */
    { e_kxorw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_kxorq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSE4A_66 */
    { e_kaddb, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_kaddd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSE4A_NO */
    { e_kaddw, t_done, 0, true, { VK, HK, WK }, 0, s1W2R3R, 0 },
    { e_kaddq, t_done, 0, true, { VK, HK, WK }, 0, s1W2R3R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSE4B_66 */
    { e_kunpckbw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSE4B_NO */
    { e_kunpckwd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_kunpckdq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSE51_66 */
    { e_vsqrtpd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vsqrtpd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vsqrtpd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSE51_F2 */
    { e_vsqrtsd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vsqrtsd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vsqrtsd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSE51_F3 */
    { e_vsqrtss, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vsqrtss, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vsqrtss, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSE51_NO */
    { e_vsqrtps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vsqrtps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vsqrtps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSE54_66 */
    { e_vandpd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vandpd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vandpd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSE54_NO */
    { e_vandps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vandps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vandps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSE55_66 */
    { e_vandnpd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vandnpd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vandnpd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSE55_NO */
    { e_vandnps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vandnps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vandnps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSE56_66 */
    { e_vorpd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vorpd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vorpd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSE56_NO */
    { e_vorps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vorps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vorps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSE57_66 */
    { e_vxorpd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vxorpd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vxorpd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSE57_NO */
    { e_vxorps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vxorps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vxorps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSE58_66 */
    { e_vaddpd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vaddpd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vaddpd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSE58_F2 */
    { e_vaddsd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vaddsd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vaddsd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSE58_F3 */
    { e_vaddss, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vaddss, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vaddss, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSE58_NO */
    { e_vaddps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vaddps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vaddps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSE59_66 */
    { e_vmulpd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vmulpd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vmulpd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 }
  }, { /* SSE59_F2 */
    { e_vmulsd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vmulsd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vmulsd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSE59_F3 */
    { e_vmulss, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vmulss, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vmulss, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSE59_NO */
    { e_vmulps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vmulps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vmulps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSE5A_66 */
    { e_vcvtpd2ps, t_done, 0, true, { Vpd, Wps, Zz }, 0, s1W2R, 0 },
    { e_vcvtpd2ps, t_done, 0, true, { Vpd, Wps, Zz }, 0, s1W2R, 0 },
    { e_vcvtpd2ps, t_done, 0, true, { Vpd, Wps, Zz }, 0, s1W2R, 0 },
  }, { /* SSE5A_F2 */
    { e_vcvtsd2ss, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vcvtsd2ss, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vcvtsd2ss, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSE5A_F3 */
    { e_vcvtss2sd, t_done, 0, true, { Vps, Wps, Zz }, 0, s1W2R, 0 },
    { e_vcvtss2sd, t_done, 0, true, { Vps, Wps, Zz }, 0, s1W2R, 0 },
    { e_vcvtss2sd, t_done, 0, true, { Vps, Wps, Zz }, 0, s1W2R, 0 },
  }, { /* SSE5A_NO */
    { e_vcvtps2pd, t_done, 0, true, { Vps, Wps, Zz }, 0, s1W2R, 0 },
    { e_vcvtps2pd, t_done, 0, true, { Vps, Wps, Zz }, 0, s1W2R, 0 },
    { e_vcvtps2pd, t_done, 0, true, { Vps, Wps, Zz }, 0, s1W2R, 0 },
  }, { /* SSE5B_66 */
    { e_vcvtps2dq, t_done, 0, true, { Vps, Wps, Zz }, 0, s1W2R, 0 },
    { e_vcvtps2dq, t_done, 0, true, { Vps, Wps, Zz }, 0, s1W2R, 0 },
    { e_vcvtps2dq, t_done, 0, true, { Vps, Wps, Zz }, 0, s1W2R, 0 },
  }, { /* SSE5B_F3 */
    { e_vcvttps2dq, t_done, 0, true, { Vps, Wps, Zz }, 0, s1W2R, 0 },
    { e_vcvttps2dq, t_done, 0, true, { Vps, Wps, Zz }, 0, s1W2R, 0 },
    { e_vcvttps2dq, t_done, 0, true, { Vps, Wps, Zz }, 0, s1W2R, 0 },
  }, { /* SSE5B_NO */
    { e_vcvtdq2ps, t_done, 0, true, { Vps, Wps, Zz }, 0, s1W2R, 0 },
    { e_vcvtdq2ps, t_done, 0, true, { Vps, Wps, Zz }, 0, s1W2R, 0 },
    { e_vcvtdq2ps, t_done, 0, true, { Vps, Wps, Zz }, 0, s1W2R, 0 }, /* TODO: Generate collsion! */
  }, { /* SSE5C_66 */
    { e_vsubpd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vsubpd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vsubpd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSE5C_F2 */
    { e_vsubsd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vsubsd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vsubsd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSE5C_F3 */
    { e_vsubss, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vsubss, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vsubss, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSE5C_NO */
    { e_vsubps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vsubps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vsubps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSE5D_66 */
    { e_vminpd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vminpd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vminpd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSE5D_F2 */
    { e_vminsd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vminsd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vminsd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSE5D_F3 */
    { e_vminss, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vminss, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vminss, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSE5D_NO */
    { e_vminps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vminps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vminps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSE5E_66 */
    { e_vdivpd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vdivpd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vdivpd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSE5E_F2 */
    { e_vdivsd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vdivsd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vdivsd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSE5E_F3 */
    { e_vdivss, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vdivss, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vdivss, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSE5E_NO */
    { e_vdivps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vdivps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vdivps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSE5F_66 */
    { e_vmaxpd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vmaxpd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vmaxpd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSE5F_F2 */
    { e_vmaxsd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vmaxsd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vmaxsd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSE5F_F3 */
    { e_vmaxss, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vmaxss, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vmaxss, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSE5F_NO */
    { e_vmaxps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vmaxps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vmaxps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSE60_66 */
    { e_vpunpcklbw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpunpcklbw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpunpcklbw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSE61_66 */
    { e_vpunpcklwd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpunpcklwd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpunpcklwd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSE62_66 */
    { e_vpunpckldq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpunpckldq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpunpckldq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSE63_66 */
    { e_vpacksswb, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpacksswb, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpacksswb, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSE64_66 */
    { e_vpcmpgtb, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpcmpgtb, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpcmpgtb, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSE65_66 */
    { e_vpcmpgtw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpcmpgtw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpcmpgtw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSE66_66 */
    { e_vpcmpgtd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpcmpgtd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpcmpgtd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSE67_66 */
    { e_vpackuswb, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpackuswb, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpackuswb, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSE68_66 */
    { e_vpunpckhbw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpunpckhbw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpunpckhbw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSE69_66 */
    { e_vpunpckhwd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpunpckhwd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpunpckhwd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSE6A_66 */
    { e_vpunpckhdq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpunpckhdq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpunpckhdq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSE6B_66 */
    { e_vpackssdw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpackssdw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpackssdw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSE6C_66 */
    { e_vpunpcklqdq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpunpcklqdq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpunpcklqdq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSE6D_66 */
    { e_vpunpckhqdq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpunpckhqdq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpunpckhqdq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSE6F_66 */
    { e_vmovdqa, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vmovdqa, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_No_Entry, t_vexw, VEXW61, false, { Zz, Zz, Zz }, 0, 0, 0 },
  }, { /* SSE6F_F2 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW62, false, { Zz, Zz, Zz }, 0, 0, 0 },
  }, { /* SSE6F_F3 */
    { e_vmovdqu, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vmovdqu, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_No_Entry, t_vexw, VEXW63, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSE70_66 */
    { e_vpshufd, t_done, 0, true, { Vps, Wps, Ib }, 0, s1W2R3R, 0 },
    { e_vpshufd, t_done, 0, true, { Vps, Wps, Ib }, 0, s1W2R3R, 0 },
    { e_vpshufd, t_done, 0, true, { Vps, Wps, Ib }, 0, s1W2R3R, 0 },
  }, { /* SSE70_F2 */
    { e_vpshuflw, t_done, 0, true, { Wps, Vps, Ib }, 0, s1W2R3R, 0 },
    { e_vpshuflw, t_done, 0, true, { Wps, Vps, Ib }, 0, s1W2R3R, 0 },
    { e_vpshuflw, t_done, 0, true, { Wps, Vps, Ib }, 0, s1W2R3R, 0 },
  }, { /* SSE70_F3 */
    { e_vpshufhw, t_done, 0, true, { Wps, Vps, Ib }, 0, s1W2R3R, 0 },
    { e_vpshufhw, t_done, 0, true, { Wps, Vps, Ib }, 0, s1W2R3R, 0 },
    { e_vpshufhw, t_done, 0, true, { Wps, Vps, Ib }, 0, s1W2R3R, 0 },
  }, { /* SSE71_66 */
    /**/{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }, // COLLISION HERE
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    /**/{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }, // COLLISION HERE
  }, { /* SSE72_66 */
    /**/{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }, // COLLISION HERE
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW77, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSE73_66 */
    { e_vpsrlq, t_done, 0, true, { Vps, Wps, Ib }, 0, s1W2R3R, 0 },
    { e_No_Entry, t_vexw, VEXW7C, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW7C, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSE74_66 */
    { e_vpcmpeqb, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpcmpeqb, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpcmpeqb, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSE75_66 */
    { e_vpcmpeqw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpcmpeqw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpcmpeqw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSE76_66 */
    { e_vpcmpeqd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpcmpeqd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpcmpeqd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSE78_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vcvttpd2uqq, t_done, 0, true, { Vps, Wps, Zz }, 0, s1W2R, 0 }
  }, { /* SSE78_F2 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vcvttsd2usi, t_done, 0, true, { Vps, Wps, Zz }, 0, s1W2R, 0 }
  }, { /* SSE78_F3 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vcvttss2usi, t_done, 0, true, { Vps, Wps, Zz }, 0, s1W2R, 0 }
  }, { /* SSE78_NO */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vcvttpd2udq, t_done, 0, true, { Vps, Wps, Zz }, 0, s1W2R, 0 }
  }, { /* SSE79_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vcvtps2uqq, t_done, 0, true, { Vps, Wps, Zz }, 0, s1W2R, 0 }
  }, { /* SSE79_F2 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vcvtsd2usi, t_done, 0, true, { Vps, Wps, Zz }, 0, s1W2R, 0 }
  }, { /* SSE79_F3 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vcvtss2usi, t_done, 0, true, { Gv, Wps, Zz }, 0, s1W2R, 0 }
  }, { /* SSE79_NO */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vcvtpd2udq, t_done, 0, true, { Vpd, Wps, Zz }, 0, s1W2R, 0 }
  }, { /* SSE7A_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vcvttpd2qq, t_done, 0, true, { Vpd, Wpd, Zz }, 0, s1W2R, 0 },
  }, { /* SSE7A_F2 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vcvtudq2ps, t_done, 0, true, { Vpd, Wpd, Zz }, 0, s1W2R, 0 },
  }, { /* SSE7A_F3 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vcvtudq2pd, t_done, 0, true, { Vps, Wpd, Zz }, 0, 0, 0 }
  }, { /* SSE7B_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vcvtpd2qq, t_done, 0, true, { Vps, Wps, Zz }, 0, s1W2R, 0 }
  }, { /* SSE7B_F2 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vcvtusi2sd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 }
  }, { /* SSE7B_F3 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vcvtusi2ss, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 }
  }, { /* SSE7E_F3 */
    { e_vmovq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vmovq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSE7F_66 */
    { e_vmovdqa, t_done, 0, true, { Vps, Wps, Zz }, 0, s1W2R, 0 },
    { e_vmovdqa, t_done, 0, true, { Vps, Wps, Zz }, 0, s1W2R, 0 },
    { e_vmovdqa, t_done, 0, true, { Vps, Wps, Zz }, 0, s1W2R, 0 },
  }, { /* SSE90_66 */
    { e_kmovb, t_done, 0, true, { VK, HK, WK }, 0, s1W2R3R, 0 },
    { e_kmovd, t_done, 0, true, { VK, HK, WK }, 0, s1W2R3R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSE90_NO */
    { e_kmovw, t_done, 0, true, { VK, WK, Zz }, 0, s1W2R, 0 },
    { e_kmovq, t_done, 0, true, { VK, HK, WK }, 0, s1W2R3R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSE91_66 */
    { e_kmovb, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_kmovd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSE91_NO */
    { e_kmovw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_kmovq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSE92_NO */
    { e_kmovw, t_done, 0, true, { VK, Ev, Zz }, 0, s1W2R, 0 },
    { e_kmovq, t_done, 0, true, { VK, Ev, Zz }, 0, s1W2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSE93_66 */
    { e_kmovb, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSE93_F2 */
    { e_kmovd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_kmovq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSE93_NO */
    { e_kmovw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_kmovw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSE98_66 */
    { e_kortestb, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_kortestd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSE98_NO */
    { e_kortestw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_kortestq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSE99_66 */
    { e_ktestb, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_ktestd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSE99_NO */
    { e_ktestw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_ktestq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEC2_66 */
    { e_vcmppd, t_done, 0, true, { Vpd, Hpd, Wpd }, 0, s1W2R3R4R, 0 },
    { e_vcmppd, t_done, 0, true, { Vpd, Hpd, Wpd }, 0, s1W2R3R4R, 0 },
    { e_vcmppd, t_done, 0, true, { Vpd, Hpd, Wpd }, 0, s1W2R3R4R, 0 },
  }, { /* SSEC2_F2 */
    { e_vcmpsd, t_done, 0, true, { Vsd, Hsd, Wsd }, 0, s1W2R3R4R, 0 },
    { e_vcmpsd, t_done, 0, true, { Vsd, Hsd, Wsd }, 0, s1W2R3R4R, 0 },
    { e_vcmpsd, t_done, 0, true, { Vsd, Hsd, Wsd }, 0, s1W2R3R4R, 0 },
  }, { /* SSEC2_F3 */
    { e_vcmpss, t_done, 0, true, { Vss, Hss, Wss }, 0, s1W2R3R4R, 0 },
    { e_vcmpss, t_done, 0, true, { Vss, Hss, Wss }, 0, s1W2R3R4R, 0 },
    { e_vcmpss, t_done, 0, true, { Vss, Hss, Wss }, 0, s1W2R3R4R, 0 },
  }, { /* SSEC2_NO */
    { e_vcmpps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R4R, 0 },
    { e_vcmpps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R4R, 0 },
    { e_vcmpps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R4R, 0 },
  }, { /* SSEC4_66 */
    { e_vpinsrw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R4R, 0 },
    { e_vpinsrw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R4R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEC5_66 */
    { e_vpextrw, t_done, 0, true, { Wps, Vps, Ib }, 0, s1W2R3R, 0 },
    { e_vpextrw, t_done, 0, true, { Wps, Vps, Ib }, 0, s1W2R3R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEC6_66 */
    { e_vshufpd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R4R, 0 },
    { e_vshufpd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R4R, 0 },
    { e_vshufpd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R4R, 0 },
  }, { /* SSEC6_NO */
    { e_vshufps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R4R, 0 },
    { e_vshufps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R4R, 0 },
    { e_vshufps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R4R, 0 },
  }, { /* SSED1_66 */
    { e_vpsrlw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpsrlw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpsrlw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSED2_66 */
    { e_vpsrld, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpsrld, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpsrld, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSED3_66 */
    { e_vpsrlq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpsrlq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpsrlq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSED4_66 */
    { e_vpaddq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpaddq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpaddq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSED5_66 */
    { e_vpmullw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpmullw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpmullw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSED8_66 */
    { e_vpsubusb, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpsubusb, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpsubusb, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSED9_66 */
    { e_vpsubusw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpsubusw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpsubusw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSEDA_66 */
    { e_vpminub, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpminub, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpminub, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSEDB_66 */
    { e_vpand, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_No_Entry, t_vexw, VEXW91, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW64, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEDC_66 */
    { e_vpaddusb, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpaddusb, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpaddusb, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSEDD_66 */
    { e_vpaddusw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpaddusw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpaddusw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSEDE_66 */
    { e_vpmaxub, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpmaxub, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpmaxub, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSEDF_66 */
    { e_vpandn, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpandn, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_No_Entry, t_vexw, VEXW65, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEE0_66 */
    { e_vpavgb, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpavgb, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpavgb, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSEE1_66 */
    { e_vpsraw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpsraw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpsraw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSEE2_66 */
    { e_vpsrad, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpsrad, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpsrad, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 }
  }, { /* SSEE3_66 */
    { e_vpavgw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpavgw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpavgw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSEE4_66 */
    { e_vpmulhuw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpmulhuw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpmulhuw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSEE5_66 */
    { e_vpmulhw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpmulhw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpmulhw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSEE6_66 */
    { e_vcvttpd2dq, t_done, 0, true, { Vps, Wps, Zz }, 0, s1W2R, 0 },
    { e_vcvttpd2dq, t_done, 0, true, { Vps, Wps, Zz }, 0, s1W2R, 0 },
    { e_vcvttpd2dq, t_done, 0, true, { Vps, Wps, Zz }, 0, s1W2R, 0 },
  }, { /* SSEE6_F2 */
    { e_vcvtpd2dq, t_done, 0, true, { Vps, Wps, Zz }, 0, s1W2R, 0 },
    { e_vcvtpd2dq, t_done, 0, true, { Vps, Wps, Zz }, 0, s1W2R, 0 },
    { e_vcvtpd2dq, t_done, 0, true, { Vps, Wps, Zz }, 0, s1W2R, 0 },
  }, { /* SSEE6_F3 */
    { e_vcvtdq2pd, t_done, 0, true, { Vps, Wps, Zz }, 0, s1W2R, 0 },
    { e_vcvtdq2pd, t_done, 0, true, { Vps, Wps, Zz }, 0, s1W2R, 0 },
    { e_vcvtdq2pd, t_done, 0, true, { Vps, Wps, Zz }, 0, s1W2R, 0 },
  }, { /* SSEE7_66 */
    { e_vmovntdq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vmovntdq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vmovntdq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 }
  }, { /* SSEE8_66 */
    { e_vpsubsb, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpsubsb, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpsubsb, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSEE9_66 */
    { e_vpsubsw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpsubsw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpsubsw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSEEA_66 */
    { e_vpminsw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpminsw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpminsw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSEEB_66 */
    { e_vpor, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpor, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_No_Entry, t_vexw, VEXW75, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEEC_66 */
    { e_vpaddsb, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpaddsb, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpaddsb, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSEED_66 */
    { e_vpaddsw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpaddsw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpaddsw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSEEE_66 */
    { e_vpmaxsw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpmaxsw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpmaxsw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSEEF_66 */
    { e_vpxor, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpxor, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_No_Entry, t_vexw, VEXW81, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEF1_66 */
    { e_vpsllw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpsllw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpsllw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSEF2_66 */
    { e_vpslld, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpslld, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpslld, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSEF3_66 */
    { e_vpsllq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpsllq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpsllq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSEF4_66 */
    { e_vpmuludq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpmuludq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpmuludq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSEF5_66 */
    { e_vpmaddwd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpmaddwd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpmaddwd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSEF6_66 */
    { e_vpsadbw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpsadbw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpsadbw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSEF8_66 */
    { e_vpsubb, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpsubb, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpsubb, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSEF9_66 */
    { e_vpsubw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpsubw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpsubw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSEFA_66 */
    { e_vpsubd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpsubd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpsubd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSEFB_66 */
    { e_vpsubq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpsubq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEFC_66 */
    { e_vpaddb, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpaddb, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpaddb, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSEFD_66 */
    { e_vpaddw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpaddw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpaddw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSEFE_66 */
    { e_vpaddd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpaddd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpaddd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }
};
/** END_DYNINST_TABLE_VERIFICATION */

/* BSSE Table order: NO, F3, F2, 66F2 */
/* rows are none, VEX2 or VEX3, EVEX prefixed in this order */
/** START_DYNINST_TABLE_VERIFICATION(sse_bis_vex_table) */
ia32_entry sseMapBisMult[][3] = 
{
  { /* SSEB00_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vpshufb, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpshufb, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSEB01_66 */
    { e_vphaddw, t_done, 0, true, { Vpd, Hpd, Wpd }, 0, s1W2R3R, 0 },
    { e_vphaddw, t_done, 0, true, { Vpd, Hpd, Wpd }, 0, s1W2R3R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEB02_66 */
    { e_vphaddd, t_done, 0, true, { Vpd, Hpd, Wpd }, 0, s1W2R3R, 0 },
    { e_vphaddd, t_done, 0, true, { Vpd, Hpd, Wpd }, 0, s1W2R3R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEB03_66 */
    { e_vphaddsw, t_done, 0, true, { Vpd, Hpd, Wpd }, 0, s1W2R3R, 0 },
    { e_vphaddsw, t_done, 0, true, { Vpd, Hpd, Wpd }, 0, s1W2R3R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEB04_66 */
    { e_vpmaddubsw, t_done, 0, true, { Vpd, Hpd, Wpd }, 0, s1W2R3R, 0 },
    { e_vpmaddubsw, t_done, 0, true, { Vpd, Hpd, Wpd }, 0, s1W2R3R, 0 },
    { e_vpmaddubsw, t_done, 0, true, { Vpd, Hpd, Wpd }, 0, s1W2R3R, 0 },
  }, { /* SSEB05_66 */
    { e_vphsubw, t_done, 0, true, { Vpd, Hpd, Wpd }, 0, s1W2R3R, 0 },
    { e_vphsubw, t_done, 0, true, { Vpd, Hpd, Wpd }, 0, s1W2R3R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEB06_66 */
    { e_vphsubd, t_done, 0, true, { Vpd, Hpd, Wpd }, 0, s1W2R3R, 0 },
    { e_vphsubd, t_done, 0, true, { Vpd, Hpd, Wpd }, 0, s1W2R3R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEB07_66 */
    { e_vphsubsw, t_done, 0, true, { Vpd, Hpd, Wpd }, 0, s1W2R3R, 0 },
    { e_vphsubsw, t_done, 0, true, { Vpd, Hpd, Wpd }, 0, s1W2R3R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEB0B_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vpmulhrsw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpmulhrsw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSEB0C_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vpermilps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpermilps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSEB0D_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vpermilpd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpermilpd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSEB10_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vpsrlvw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 }
  }, { /* SSEB10_F3 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vpmovuswb, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 }
  }, { /* SSEB11_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vpsravw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 }
  }, { /* SSEB11_F3 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vpmovusdb, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 }
  }, { /* SSEB12_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vpsllvw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 }
  }, { /* SSEB12_F3 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vpmovusqb, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 }
  }, { /* SSEB13_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vcvtph2ps, t_done, 0, true, { Vps, Wps, Zz }, 0, s1W2R, 0 },
    { e_vcvtph2ps, t_done, 0, true, { Vps, Wps, Zz }, 0, s1W2R, 0 },
  }, { /* SSEB13_F3 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vpmovusdw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 }
  }, { /* SSEB14_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW78, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEB14_F3 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vpmovusqw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 }
  }, { /* SSEB15_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW76, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEB15_F3 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vpmovusqd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 }
  }, { /* SSEB16_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW96, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW96, false, { Zz, Zz, Zz }, 0, 0, 0 },
  }, { /* SSEB18_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vbroadcastss, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 }
  }, { /* SSEB19_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    /**/{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }, // COLLISION HERE
  }, { /* SSEB1A_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vbroadcastf32x4, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 }
  }, { /* SSEB1C_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vpabsb, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpabsb, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSEB1D_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vpabsw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpabsw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSEB1E_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vpabsd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpabsd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSEB1F_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vpabsq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 }
  }, { /* SSEB20_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vpmovsxbw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpmovsxbw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSEB20_F3 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vpmovswb, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 }
  }, { /* SSEB21_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vpmovsxbd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpmovsxbd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSEB21_F3 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vpmovsdb, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 }
  }, { /* SSEB22_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vpmovsxbq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpmovsxbq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSEB22_F3 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vpmovsqb, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 }
  }, { /* SSEB23_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vpmovsxwd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpmovsxwd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSEB23_F3 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vpmovsdw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 }
  }, { /* SSEB24_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vpmovsxwq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpmovsxwq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSEB24_F3 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vpmovsqw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 }
  }, { /* SSEB25_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vpmovsxdq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpmovsxdq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSEB25_F3 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vpmovsqd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 }
  }, { /* SSEB26_F3 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vptestnmb, t_done, 0, true, { VK, Hps, Wps }, 0, s1W2R3R, 0 }
  }, { /* SSEB27_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vptestmd, t_done, 0, true, { VK, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vptestmd, t_done, 0, true, { VK, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSEB27_F3 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vptestnmd, t_done, 0, true, { VK, Hps, Wps }, 0, s1W2R3R, 0 }
  }, { /* SSEB28_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vpmuldq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpmuldq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSEB28_F3 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vpmovm2b, t_done, 0, true, { VK, Wps, Zz }, 0, s1W2R, 0 },
  }, { /* SSEB29_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vpcmpeqq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpcmpeqq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSEB29_F3 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vpmovb2m, t_done, 0, true, { Vps, WK, Zz }, 0, 0, 0 }
  }, { /* SSEB2A_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vmovntdqa, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 }
  }, { /* SSEB2A_F3 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vpbroadcastmb2q, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 }
  }, { /* SSEB2B_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vpackusdw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpackusdw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSEB30_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vpmovzxbw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpmovzxbw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSEB30_F3 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vpmovwb, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 }
  }, { /* SSEB31_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vpmovzxbd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpmovzxbd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSEB31_F3 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vpmovdb, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 }
  }, { /* SSEB32_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vpmovzxbq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpmovzxbq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSEB32_F3 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vpmovqb, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 }
  }, { /* SSEB33_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vpmovzxwd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpmovzxwd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSEB33_F3 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vpmovdw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 }
  }, { /* SSEB34_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vpmovzxwq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpmovzxwq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSEB34_F3 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vpmovqw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 }
  }, { /* SSEB35_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vpmovzxdq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpmovzxdq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSEB35_F3 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vpmovqd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 }
  }, { /* SSEB36_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vpermd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpermd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSEB37_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vpcmpgtq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpcmpgtq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSEB38_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vpminsb, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpminsb, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSEB38_F3 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vpmovm2d, t_done, 0, true, { VK, Wps, Zz }, 0, s1W2R, 0 }
  }, { /* SSEB39_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vpminsd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_No_Entry, t_vexw, VEXW73, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEB39_F3 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW73, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEB3A_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vpminuw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpminuw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSEB3B_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vpminud, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_No_Entry, t_vexw, VEXW74, false, { Zz, Zz, Zz }, 0, 0, 0 },
  }, { /* SSEB3C_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vpmaxsb, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpmaxsb, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSEB3D_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vpmaxsd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_No_Entry, t_vexw, VEXW71, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEB3E_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vpmaxuw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpmaxuw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSEB3F_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vpmaxud, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_No_Entry, t_vexw, VEXW72, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEB40_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vpmulld, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vpmulld, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSEB42_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW59, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEB43_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW5A, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEB44_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW70, false, { Zz, Zz, Zz }, 0, 0, 0 },
  }, { /* SSEB45_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW20, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW70, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEB46_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vpsravd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_No_Entry, t_vexw, VEXW80, false, { Zz, Zz, Zz }, 0, 0, 0 },
  }, { /* SSEB47_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW21, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW7D, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEB4C_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW84, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEB4D_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW85, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEB4E_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW88, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEB4F_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW89, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEB65_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW2C, false, { Zz, Zz, Zz }, 0, 0, 0 },
  }, { /* SSEB66_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW14, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW2C, false, { Zz, Zz, Zz }, 0, 0, 0 },
  }, { /* SSEB75_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW68, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEB76_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW69, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEB77_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW6A, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEB7C_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW93, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW93, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEB7D_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW6B, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEB7E_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW6C, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEB7F_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW6D, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEB83_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vpmultishiftqb, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 }
  }, { /* SSEB88_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW2E, false, { Zz, Zz, Zz }, 0, 0, 0 },
  }, { /* SSEB89_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW6F, false, { Zz, Zz, Zz }, 0, 0, 0 },
  }, { /* SSEB8B_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW66, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEB8C_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    /**/{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }, // COLLISION HERE
  }, { /* SSEB8D_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW68, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEB8E_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    /**/{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }, // COLLISION HERE
  }, { /* SSEB90_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW22, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW55, false, { Zz, Zz, Zz }, 0, 0, 0 },
  }, { /* SSEB91_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW23, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW56, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEB92_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW92, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW57, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEB93_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW58, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW58, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEB96_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW00, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW35, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEB97_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW01, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW36, false, { Zz, Zz, Zz }, 0, 0, 0 },
  }, { /* SSEB98_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW02, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW37, false, { Zz, Zz, Zz }, 0, 0, 0 },
  }, { /* SSEB99_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW03, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW38, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEB9A_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW04, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW39, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEB9B_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW05, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW3A, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEB9C_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW06, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW3B, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEB9D_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW07, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW3C, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEB9E_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vfnmsub132ps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_vfnmsub132ps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
  }, { /* SSEB9F_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW09, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW3E, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEBA0_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW7A, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEBA1_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW7B, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEBA2_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW8A, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEBA3_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW8B, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEBA6_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW0A, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW3F, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEBA7_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW0B, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW40, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEBA8_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW0C, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW41, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEBA9_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW0D, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW42, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEBAA_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW0E, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW43, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEBAB_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW0F, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW44, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEBAC_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW10, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW45, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEBAD_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW11, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW46, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEBAE_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW09, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW47, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEBAF_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW13, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW48, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEBB4_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vpmadd52luq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 }
  }, { /* SSEBB5_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vpmadd52huq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 }
  }, { /* SSEBB6_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW14, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW49, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEBB7_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW15, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW4A, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEBB8_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW16, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW4B, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEBB9_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW17, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW4C, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEBBA_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW18, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW4D, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEBBB_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW19, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW4E, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEBBC_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW1A, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW4F, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEBBD_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW1B, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW52, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEBBE_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW13, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW51, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEBBF_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW1D, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW48, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEBC4_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW67, false, { Zz, Zz, Zz }, 0, 0, 0 },
  }, { /* SSEBC6_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vgatherpf0dps, t_done, 0, true, { Wps, Zz, Zz }, 0, s1W, 0 }
  }, { /* SSEBC7_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vgatherpf0qps, t_done, 0, true, { Wps, Zz, Zz }, 0, s1W, 0 }
  }, { /* SSEBC8_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW8C, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEBCA_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW8D, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEBCB_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW8E, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEBCC_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW8F, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEBCD_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_No_Entry, t_vexw, VEXW90, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEBDB_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vaesimc, t_done, 0, true, { Vps, Wps, Zz }, 0, s1W2R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  }, { /* SSEBDC_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vaesenc, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  }, { /* SSEBDD_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vaesenclast, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  }, { /* SSEBDE_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vaesdec, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  }, { /* SSEBDF_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_aesdeclast, t_done, 0, true, { Vps, Hps, Wps }, 0, s1R2R3R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  }, { /* SSEBF2_NO */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_andn, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEBF5_F2 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_pdep, t_done, 0, true, { Gv, Ev, Bv }, 0, s1W2R3R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  }, { /* SSEBF5_F3 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_pext, t_done, 0, true, { Gv, Ev, Bv }, 0, s1W2R3R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  }, { /* SSEBF5_NO */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_bzhi, t_done, 0, true, { Gv, Ev, Bv }, 0, s1W2R3R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  }, { /* SSEBF6_F2 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_mulx, t_done, 0, true, { Gv, Ev, Bv }, 0, s1W2R3R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEBF6_F3 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_mulx, t_done, 0, true, { Gv, Ev, Bv }, 0, s1W2R3R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
  }, { /* SSEBF7_66 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_shlx, t_done, 0, true, { Gv, Ev, Bv }, 0, s1W2R3R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEBF7_F2 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_shrx, t_done, 0, true, { Gv, Ev, Bv }, 0, s1W2R3R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEBF7_F3 */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_sarx, t_done, 0, true, { Gv, Ev, Bv }, 0, s1W2R3R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }, { /* SSEBF7_NO */
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_bextr, t_done, 0, true, { Gv, Ev, Bv }, 0, s1W2R3R, 0 },
    { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
  }
};
/** END_DYNINST_TABLE_VERIFICATION */
  
/* TSSE Table order: NO, 66, F2 */
/* rows are none, VEX2 or VEX3, EVEX prefixed in this order */
/** START_DYNINST_TABLE_VERIFICATION(sse_vex_ter_table) */
ia32_entry sseMapTerMult[][3] = 
{
    { /* SSET00_66 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_vpermq, t_done, 0, true, { Wps, Vps, Ib }, 0, s1W2R3R, 0 },
        { e_vpermq, t_done, 0, true, { Wps, Vps, Ib }, 0, s1W2R3R, 0 },
    }, { /* SSET01_66 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_vpermpd, t_done, 0, true, { Wpd, Vpd, Ib }, 0, s1W2R3R, 0 },
        { e_vpermpd, t_done, 0, true, { Wpd, Vpd, Ib }, 0, s1W2R3R, 0 },
    }, { /* SSET03_66 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_vexw, VEXW2B, false, { Zz, Zz, Zz }, 0, 0, 0 },
    }, { /* SSET04_66 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_vpermilps, t_done, 0, true, { Wps, Vps, Ib }, 0, s1W2R3R, 0 },
        { e_vpermilps, t_done, 0, true, { Wps, Vps, Ib }, 0, s1W2R3R, 0 },
    }, { /* SSET05_66 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_vpermilpd, t_done, 0, true, { Wpd, Vpd, Ib }, 0, s1W2R3R, 0 },
        { e_vpermilpd, t_done, 0, true, { Wpd, Vpd, Ib }, 0, s1W2R3R, 0 },
    }, { /* SSET08_66 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_vrndscaleps, t_done, 0, true, { Wps, Vps, Ib }, 0, s1W2R3R, 0 }
    }, { /* SSET09_66 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_vroundpd, t_done, 0, true, { Wps, Vps, Ib }, 0, s1W2R3R, 0 },
        { e_vrndscalepd, t_done, 0, true, { Wps, Vps, Ib }, 0, s1W2R3R, 0 }
    }, { /* SSET0A_66 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_vrndscaless, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R4R, 0 }
    }, { /* SSET0B_66 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_vroundsd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R4R, 0 },
        { e_vrndscalesd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R4R, 0 }
    }, { /* SSET0C_66 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_vblendps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R4R, 0 },
        { e_vblendps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R4R, 0 }
    }, { /* SSET0F_66 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_vpalignr, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R4R, 0 }, /** Intel manual is wrong -- 4 operands */
        { e_vpalignr, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R4R, 0 },
    }, { /* SSET14_66 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_vpextrb, t_done, 0, true, { Wps, Vps, Ib }, 0, s1W2R3R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSET16_66 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_vexw, VEXW26, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSET17_66 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_vextractps, t_done, 0, true, { Wps, Vps, Ib }, 0, s1W2R3R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSET18_66 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_vinsertf128, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R4R, 0 },
        { e_No_Entry, t_vexw, VEXW5D, false, { Zz, Zz, Zz }, 0, 0, 0 },
    }, { /* SSET19_66 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_vextractf128, t_done, 0, true, { Wps, Vps, Ib }, 0, s1W2R3R, 0 },
        { e_No_Entry, t_vexw, VEXW2F, false, { Zz, Zz, Zz }, 0, 0, 0 },
    }, { /* SSET1A_66 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_vexw, VEXW5E, false, { Zz, Zz, Zz }, 0, 0, 0 },
    }, { /* SSET1B_66 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_vexw, VEXW30, false, { Zz, Zz, Zz }, 0, 0, 0 },
    }, { /* SSET1D_66 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_vcvtps2ph, t_done, 0, true, { Wps, Vps, Ib }, 0, s1W2R3R, 0 },
        { e_vcvtps2ph, t_done, 0, true, { Wps, Vps, Ib }, 0, s1W2R3R, 0 },
    }, { /* SSET1E_66 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    }, { /* SSET1F_66 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_vpcmpeqd, t_done, 0, true, { Wpd, Vpd, IK }, 0, s1W2R3R, 0 }
    }, { /* SSET20_66 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_vpinsrb, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R4R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSET21_66 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_vinsertps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R4R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSET22_66 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_vexw, VEXW27, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSET23_66 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_vexw, VEXW7E, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSET25_66 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_vexw, VEXW7F, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSET26_66 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_vexw, VEXW5B, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSET27_66 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_vexw, VEXW5C, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSET30_66 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_kshiftrb, t_done, 0, true, { VK, HK, Ib }, 0, s1W2R3R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSET31_66 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_kshiftrq, t_done, 0, true, { VK, HK, Ib }, 0, s1W2R3R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSET32_66 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_kshiftlw, t_done, 0, true, { VK, HK, Ib }, 0, s1W2R3R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSET33_66 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_kshiftlq, t_done, 0, true, { VK, HK, Ib }, 0, s1W2R3R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSET38_66 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_vinserti128, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R4R, 0 },
        { e_No_Entry, t_vexw, VEXW5F, false, { Zz, Zz, Zz }, 0, 0, 0 },
    }, { /* SSET39_66 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_vextracti128, t_done, 0, true, { Wps, Vps, Ib }, 0, s1W2R3R, 0 },
        { e_No_Entry, t_vexw, VEXW31, false, { Zz, Zz, Zz }, 0, 0, 0 },
    }, { /* SSET3A_66 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_vexw, VEXW60, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSET3B_66 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_vexw, VEXW32, false, { Zz, Zz, Zz }, 0, 0, 0 },
    }, { /* SSET3E_66 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_vpcmpub, t_done, 0, true, { Vps, Wps, Ib }, 0, s1W2R3R, 0 }
    }, { /* SSET3F_66 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_vpcmpb, t_done, 0, true, { Vps, Wps, Ib }, 0, s1W2R3R, 0 }
    }, { /* SSET42_66 */
        { e_vmpsadbw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R4R, 0 },
        { e_vdbpsadbw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R4R, 0 },
        { e_vdbpsadbw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R4R, 0 }
    }, { /* SSET44_66 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_vpclmulqdq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R4R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    }, { /* SSET4A_66 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_vblendvps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R4R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSET4B_66 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_vblendvpd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R4R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSET4C_66 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_vpblendvb, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R4R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    }, { /* SSET50_66 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_vexw, VEXW82, false, { Zz, Zz, Zz }, 0, 0, 0 },
    }, { /* SSET51_66 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_vexw, VEXW83, false, { Zz, Zz, Zz }, 0, 0, 0 },
    }, { /* SSET54_66 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_vexw, VEXW33, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSET55_66 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_vexw, VEXW34, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSET56_66 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_vexw, VEXW86, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSET57_66 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_vexw, VEXW87, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSET66_66 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_vexw, VEXW53, false, { Zz, Zz, Zz }, 0, 0, 0 },
    }, { /* SSET67_66 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_vexw, VEXW54, false, { Zz, Zz, Zz }, 0, 0, 0 },
    }, { /* SSET69_66 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_vexw, VEXW95, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSETDF_66 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_vaeskeygenassist, t_done, 0, true, { Vps, Wps, Ib }, 0, s1W2R3R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 }
    }, { /* SSETF0_F2 */
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
        { e_rorx, t_done, 0, true, { Gv, Bv, Ib }, 0, s1W2R3R, 0 },
        { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
    }
};
/** END_DYNINST_TABLE_VERIFICATION */

/**
 * This is the table that typically follows after the Group Map
 * table. This table holds the SSE version of the instructions in
 * the Group Map. The format of the names is as follows:
 *
 *
 * +---+----+-----+-----+-----+
 * | G | XX | SSE | XXX | (X) |
 * +---+----+-----+-----+-----+
 *       |           |     +-> If this is a B, then the Mod value was 3
 *       |           +-------> This is the value of the Reg value in binary
 *       +-------------------> This is the group number.
 *
 */

/* rows are none or 66 prefixed in this order (see book) */
/** START_DYNINST_TABLE_VERIFICATION(sse_grp_map) */
static ia32_entry ssegrpMap[][2] = {
  /* G12SSE010B */
  {
    { e_psrlw, t_done, 0, true, { Pq, Ib, Zz }, 0, s1RW2R, 0 },
    { e_psrlw, t_done, 0, true, { Pdq, Ib, Zz }, 0, s1RW2R, 0 }
  },
  /* G12SSE100B */
  {
    { e_psraw, t_done, 0, true, { Pq, Ib, Zz }, 0, s1RW2R, 0 },
    { e_psraw, t_done, 0, true, { Pdq, Ib, Zz }, 0, s1RW2R, 0 }
  },
  /* G12SSE110B */
  {
    { e_psllw, t_done, 0, true, { Pq, Ib, Zz }, 0, s1RW2R, 0 },
    { e_psllw, t_done, 0, true, { Pdq, Ib, Zz }, 0, s1RW2R, 0 }
  },
  /* G13SSE010B */
  {
    { e_psrld, t_done, 0, true, { Pq, Ib, Zz }, 0, s1RW2R, 0 },
    { e_psrld, t_done, 0, true, { Wdq, Ib, Zz }, 0, s1RW2R, 0 }
  },
  /* G13SSE100B */
  {
    { e_psrad, t_done, 0, true, { Pq, Ib, Zz }, 0, s1RW2R, 0 },
    { e_psrad, t_done, 0, true, { Wdq, Ib, Zz }, 0, s1RW2R, 0 }
  },
  /* G13SSE110B */
  {
    { e_pslld, t_done, 0, true, { Pq, Ib, Zz }, 0, s1RW2R, 0 },
    { e_pslld, t_done, 0, true, { Wdq, Ib, Zz }, 0, s1RW2R, 0 }
  },
  /* G14SSE010B */
  {
    { e_psrlq, t_done, 0, true, { Pq, Ib, Zz }, 0, s1RW2R, 0 },
    { e_psrlq, t_done, 0, true, { Wdq, Ib, Zz }, 0, s1RW2R, 0 }
  },
  /* G14SSE011B */
  {
    { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_psrldq, t_done, 0, true, { Wdq, Ib, Zz }, 0, s1RW2R, 0 }
  },
  /* G14SSE110B */
  {
    { e_psllq, t_done, 0, true, { Pq, Ib, Zz }, 0, s1RW2R, 0 },
    { e_psllq, t_done, 0, true, { Wdq, Ib, Zz }, 0, s1RW2R, 0 }
  },
  /* G14SSE111B */
  {
    { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_pslldq, t_done, 0, true, { Wdq, Ib, Zz }, 0, s1RW2R, 0 }
  }
};
/** END_DYNINST_TABLE_VERIFICATION */

/** ssegrpMap instructions, except with VEX prefix. */
/** START_DYNINST_TABLE_VERIFICATION(sse_grp_map) */
static ia32_entry ssegrpMap_VEX[][2] = {
  /* G12SSE010B */
  {
    { e_vpsrlw, t_done, 0, true, { Pq, Ib, Zz }, 0, s1RW2R, 0 },
    { e_vpsrlw, t_done, 0, true, { Pdq, Ib, Zz }, 0, s1RW2R, 0 }
  },
  /* G12SSE100B */
  {
    { e_vpsraw, t_done, 0, true, { Pq, Ib, Zz }, 0, s1RW2R, 0 },
    { e_vpsraw, t_done, 0, true, { Pdq, Ib, Zz }, 0, s1RW2R, 0 }
  },
  /* G12SSE110B */
  {
    { e_vpsllw, t_done, 0, true, { Pq, Ib, Zz }, 0, s1RW2R, 0 },
    { e_vpsllw, t_done, 0, true, { Pdq, Ib, Zz }, 0, s1RW2R, 0 }
  },
  /* G13SSE010B */
  {
    { e_vpsrld, t_done, 0, true, { Pq, Ib, Zz }, 0, s1RW2R, 0 },
    { e_vpsrld, t_done, 0, true, { Wdq, Ib, Zz }, 0, s1RW2R, 0 }
  },
  /* G13SSE100B */
  {
    { e_vpsrad, t_done, 0, true, { Pq, Ib, Zz }, 0, s1RW2R, 0 },
    { e_vpsrad, t_done, 0, true, { Wdq, Ib, Zz }, 0, s1RW2R, 0 }
  },
  /* G13SSE110B */
  {
    { e_vpslld, t_done, 0, true, { Pq, Ib, Zz }, 0, s1RW2R, 0 },
    { e_vpslld, t_done, 0, true, { Wdq, Ib, Zz }, 0, s1RW2R, 0 }
  },
  /* G14SSE010B */
  {
    { e_vpsrlq, t_done, 0, true, { Pq, Ib, Zz }, 0, s1RW2R, 0 },
    { e_vpsrlq, t_done, 0, true, { Wdq, Ib, Zz }, 0, s1RW2R, 0 }
  },
  /* G14SSE011B */
  {
    { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vpsrldq, t_done, 0, true, { Wdq, Ib, Zz }, 0, s1RW2R, 0 }
  },
  /* G14SSE110B */
  {
    { e_vpsllq, t_done, 0, true, { Pq, Ib, Zz }, 0, s1RW2R, 0 },
    { e_vpsllq, t_done, 0, true, { Wdq, Ib, Zz }, 0, s1RW2R, 0 }
  },
  /* G14SSE111B */
  {
    { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 },
    { e_vpslldq, t_done, 0, true, { Wdq, Ib, Zz }, 0, s1RW2R, 0 }
  }
};
/** END_DYNINST_TABLE_VERIFICATION */


/**
 * VEX (2 byte) prefixed instructions
 *
 * Instruction lookup: [index][L]
 *    index: found by using opcode lookups in the oneByteMap.
 *    L: The l bit of the prefix. L=1 is YMM registers, L=0 is XMM registers
 */
/** START_DYNINST_TABLE_VERIFICATION(vexl_table) */
static struct ia32_entry vex2Map[][2] =
{
    { /* VEXL00 */
      { e_vzeroupper, t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE, 0 }, /* L = 0 */
      { e_vzeroall, t_done, 0, false, { Zz, Zz, Zz }, 0, sNONE, 0 }  /* L = 1 */
    }
};
/** END_DYNINST_TABLE_VERIFICATION */

/**
 * VEX (3 byte) prefixed instructions
 *
 * Instruction lookup (loop 1): [index][W]
 *    index: found by using opcode lookups in the oneByteMap.
 *    W: The w bit of the prefix. This can completely change the behavior
 *        of certain VEX3 prefixed instructions.
 */

#define VEX3_ILL \
  {{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 }, \
   { e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0 }}

/** START_DYNINST_TABLE_VERIFICATION(vex_w_table) */
static struct ia32_entry vexWMap[][2] =
{
    { /* VEXW00 */
      { e_vfmaddsub132ps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vfmaddsub132pd, t_done, 0, true, { Vpd, Hpd, Wpd }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW01 */
      { e_vfmsubadd132ps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vfmsubadd132pd, t_done, 0, true, { Vpd, Hpd, Wpd }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW02 */
      { e_vfmadd132ps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vfmadd132pd, t_done, 0, true, { Vpd, Hpd, Wpd }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW03 */
      { e_vfmadd132ss, t_done, 0, true, { Vss, Hss, Wss }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vfmadd132sd, t_done, 0, true, { Vsd, Hsd, Wsd }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW04 */
      { e_vfmsub132ps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vfmsub132pd, t_done, 0, true, { Vpd, Hpd, Wpd }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW05 */
      { e_vfmsub132ss, t_done, 0, true, { Vss, Hss, Wss }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vfmsub132sd, t_done, 0, true, { Vsd, Hsd, Wsd }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW06 */
      { e_vfnmadd132ps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vfnmadd132pd, t_done, 0, true, { Vpd, Hpd, Wpd }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW07 */
      { e_vfnmadd132ss, t_done, 0, true, { Vss, Hss, Wss }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vfnmadd132sd, t_done, 0, true, { Vsd, Hsd, Wsd }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW08 */
      { e_vfnmsub132ps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vfnmsub132pd, t_done, 0, true, { Vpd, Hpd, Wpd }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW09 */
      { e_vfnmsub132ss, t_done, 0, true, { Vss, Hss, Wss }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vfnmsub132sd, t_done, 0, true, { Vsd, Hsd, Wsd }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW0A */
      { e_vfmaddsub213ps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vfmaddsub213pd, t_done, 0, true, { Vpd, Hpd, Wpd }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW0B */
      { e_vfmsubadd213ps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vfmsubadd213pd, t_done, 0, true, { Vpd, Hpd, Wpd }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW0C */
      { e_vfmadd213ps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vfmadd213pd, t_done, 0, true, { Vpd, Hpd, Wpd }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW0D */
      { e_vfmadd213ss, t_done, 0, true, { Vss, Hss, Wss }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vfmadd213sd, t_done, 0, true, { Vsd, Hsd, Wsd }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW0E */
      { e_vfmsub213ps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vfmsub213pd, t_done, 0, true, { Vpd, Hpd, Wpd }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW0F */
      { e_vfmsub213ss, t_done, 0, true, { Vss, Hss, Wss }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vfmsub213sd, t_done, 0, true, { Vsd, Hsd, Wsd }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW10 */
      { e_vfnmadd213ps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vfnmadd213pd, t_done, 0, true, { Vpd, Hpd, Wpd }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW11 */
      { e_vfnmadd213ss, t_done, 0, true, { Vss, Hss, Wss }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vfnmadd213sd, t_done, 0, true, { Vsd, Hsd, Wsd }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW12 */
      { e_vfnmsub213ps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vfnmsub213pd, t_done, 0, true, { Vpd, Hpd, Wpd }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW13 */
      { e_vfnmsub213ss, t_done, 0, true, { Vss, Hss, Wss }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vfnmsub213sd, t_done, 0, true, { Vsd, Hsd, Wsd }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW14 */
      { e_vfmaddsub231ps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vfmaddsub231pd, t_done, 0, true, { Vpd, Hpd, Wpd }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW15 */
      { e_vfmsubadd231ps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vfmsubadd231pd, t_done, 0, true, { Vpd, Hpd, Wpd }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW16 */
      { e_vfmadd231ps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vfmadd231pd, t_done, 0, true, { Vpd, Hpd, Wpd }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW17 */
      { e_vfmadd231ss, t_done, 0, true, { Vss, Hss, Wss }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vfmadd231sd, t_done, 0, true, { Vsd, Hsd, Wsd }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW18 */
      { e_vfmsub231ps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vfmsub231pd, t_done, 0, true, { Vpd, Hpd, Wpd }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW19 */
      { e_vfmsub231ss, t_done, 0, true, { Vss, Hss, Wss }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vfmsub231sd, t_done, 0, true, { Vsd, Hsd, Wsd }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW1A */
      { e_vfnmadd231ps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vfnmadd231pd, t_done, 0, true, { Vpd, Hpd, Wpd }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW1B */
      { e_vfnmadd231ss, t_done, 0, true, { Vss, Hss, Wss }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vfnmadd231sd, t_done, 0, true, { Vsd, Hsd, Wsd }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW1C */
      { e_vfnmsub231ps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vfnmsub231pd, t_done, 0, true, { Vpd, Hpd, Wpd }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW1D */
      { e_vfnmsub231ss, t_done, 0, true, { Vss, Hss, Wss }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vfnmsub231sd, t_done, 0, true, { Vsd, Hsd, Wsd }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW1E */
      { e_vpmaskmovd, t_done, 0, true, { Wd, Hps, Vps }, 0, s1W2R3R, 0 }, /* W = 0 */
      { e_vpmaskmovq, t_done, 0, true, { Wq, Hps, Vps }, 0, s1W2R3R, 0 }  /* W = 1 */
    }, { /* VEXW1F*/
      { e_vpmaskmovd, t_done, 0, true, { Vps, Hps, Wd }, 0, s1W2R3R, 0 }, /* W = 0 */
      { e_vpmaskmovq, t_done, 0, true, { Vps, Hps, Wq }, 0, s1W2R3R, 0 }  /* W = 1 */
    }, { /* VEXW20 */
      { e_vpsrlvd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 }, /* W = 0 */
      { e_vpsrlvq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 }  /* W = 1 */
    }, { /* VEXW21 */
      { e_vpsllvd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 }, /* W = 0 */
      { e_vpsllvq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 }  /* W = 1 */
    }, { /* VEXW22 */
      { e_vpgatherdd, t_done, 0, true, { Vpd, Wpd, Hpd }, 0, s1RW2R3RW, 0 }, /* W = 0 */
      { e_vpgatherdq, t_done, 0, true, { Vdq, Wdq, Hdq }, 0, s1RW2R3RW, 0 }  /* W = 1 */
    }, { /* VEXW23 */
      { e_vpgatherqd, t_done, 0, true, { Vpd, Wpd, Hpd }, 0, s1RW2R3RW, 0 }, /* W = 0 */
      { e_vpgatherqq, t_done, 0, true, { Vdq, Wdq, Hdq }, 0, s1RW2R3RW, 0 }  /* W = 1 */
    }, { /* VEXW24 */
      { e_vgatherdps, t_done, 0, true, { Vps, Wps, Hps }, 0, s1RW2R3RW, 0 }, /* W = 0 */
      { e_vgatherdpd, t_done, 0, true, { Vpd, Wpd, Hpd }, 0, s1RW2R3RW, 0 }  /* W = 1 */
    }, { /* VEXW25 */
      { e_vgatherqps, t_done, 0, true, { Vps, Wps, Hps }, 0, s1RW2R3RW, 0 }, /* W = 0 */
      { e_vgatherqpd, t_done, 0, true, { Vpd, Wpd, Hpd }, 0, s1RW2R3RW, 0 }  /* W = 1 */
    }, { /* VEXW26 */
      { e_vpextrd, t_done, 0, true, { Vpd, Wpd, Ib }, 0, s1W2R3R, 0 }, /* W = 0 */
      { e_vpextrq, t_done, 0, true, { Vpd, Wpd, Ib }, 0, s1W2R3R, 0 }  /* W = 1 */
    }, { /* VEXW27 */
      { e_vpinsrd, t_done, 0, true, { Vpd, Hpd, Wpd }, 0, s1W2R3R4R, 0 }, /* W = 0 */
      { e_vpinsrq, t_done, 0, true, { Vpd, Hpd, Wpd }, 0, s1W2R3R4R, 0 }  /* W = 1 */
    }, { /* VEXW28 */
      { e_valignd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R4R, 0 }, /* W = 0 */ /* Intel manual wrong, 4 operands */
      { e_valignq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R4R, 0 }  /* W = 1 */
    }, { /* VEXW29 */
      { e_vblendmps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 }, /* W = 0 */
      { e_vblendmpd, t_done, 0, true, { Vpd, Hpd, Wpd }, 0, s1W2R3R, 0 }  /* W = 1 */
    }, { /* VEXW2A */
      { e_vpblendmb, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 }, /* W = 0 */
      { e_vpblendmw, t_done, 0, true, { Vpd, Hpd, Wpd }, 0, s1W2R3R, 0 }  /* W = 1 */
    }, { /* VEXW2B */ /* EVEX Version */
      { e_valignd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R4R, 0 }, /* W = 0 */ /* Intel manual wrong, 4 operands */
      { e_valignq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R4R, 0 }  /* W = 1 */
    }, { /* VEXW2C */
      { e_vblendmps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },  /* W = 0 */
      { e_vblendmpd, t_done, 0, true, { Vpd, Hpd, Wpd }, 0, s1W2R3R, 0 } /* W = 1 */
    }, { /* VEXW2D */ /* EVEX VERSION*/
      { e_vpblendmb, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 },  /* W = 0 */
      { e_vpblendmw, t_done, 0, true, { Vpd, Hpd, Wpd }, 0, s1W2R3R, 0 } /* W = 1 */
    }, { /* VEXW2E */ /* EVEX VERSION*/
      { e_vexpandps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 }, /* W = 0 */
      { e_vexpandpd, t_done, 0, true, { Vpd, Hpd, Wpd }, 0, s1W2R3R, 0 }  /* W = 1 */
    }, { /* VEXW2F */ /* EVEX VERSION*/
      { e_vextractf32x4, t_done, 0, true, { Wps, Vps, Ib }, 0, s1W2R3R, 0 }, /* W = 0 */
      { e_vextractf64x2, t_done, 0, true, { Wps, Vps, Ib }, 0, s1W2R3R, 0 }  /* W = 1 */
    }, { /* VEXW30 */ /* EVEX VERSION*/
      { e_vextractf32x8, t_done, 0, true, { Wps, Vps, Ib }, 0, s1W2R3R, 0 }, /* W = 0 */
      { e_vextractf64x4, t_done, 0, true, { Wps, Vps, Ib }, 0, s1W2R3R, 0 }  /* W = 1 */
    }, { /* VEXW31 */ /* EVEX VERSION*/
      { e_vextracti32x4, t_done, 0, true, { Wps, Vps, Ib }, 0, s1W2R3R, 0 }, /* W = 0 */
      { e_vextracti64x2, t_done, 0, true, { Wps, Vps, Ib }, 0, s1W2R3R, 0 }  /* W = 1 */
    }, { /* VEXW32 */ /* EVEX VERSION*/
      { e_vextracti32x8, t_done, 0, true, { Wps, Vps, Ib }, 0, s1W2R3R, 0 }, /* W = 0 */
      { e_vextracti64x4, t_done, 0, true, { Wps, Vps, Ib }, 0, s1W2R3R, 0 }  /* W = 1 */
    }, { /* VEXW33 */ /* EVEX VERSION*/
      { e_vfixupimmps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R4R, 0 }, /* W = 0 */
      { e_vfixupimmpd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R4R, 0 }  /* W = 1 */
    }, { /* VEXW34 */ /* EVEX VERSION*/
      { e_vfixupimmss, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R4R, 0 }, /* W = 0 */
      { e_vfixupimmsd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R4R, 0 }  /* W = 1 */
    }, { /* VEXW35 */ /* EVEX VERSION */
      { e_vfmaddsub132ps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vfmaddsub132pd, t_done, 0, true, { Vpd, Hpd, Wpd }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW36 *//* EVEX VERSION */
      { e_vfmsubadd132ps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vfmsubadd132pd, t_done, 0, true, { Vpd, Hpd, Wpd }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW37 *//* EVEX VERSION */
      { e_vfmadd132ps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vfmadd132pd, t_done, 0, true, { Vpd, Hpd, Wpd }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW38 *//* EVEX VERSION */
      { e_vfmadd132ss, t_done, 0, true, { Vss, Hss, Wss }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vfmadd132sd, t_done, 0, true, { Vsd, Hsd, Wsd }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW39 *//* EVEX VERSION */
      { e_vfmsub132ps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vfmsub132pd, t_done, 0, true, { Vpd, Hpd, Wpd }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW3A *//* EVEX VERSION */
      { e_vfmsub132ss, t_done, 0, true, { Vss, Hss, Wss }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vfmsub132sd, t_done, 0, true, { Vsd, Hsd, Wsd }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW3B *//* EVEX VERSION */
      { e_vfnmadd132ps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vfnmadd132pd, t_done, 0, true, { Vpd, Hpd, Wpd }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW3C *//* EVEX VERSION */
      { e_vfnmadd132ss, t_done, 0, true, { Vss, Hss, Wss }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vfnmadd132sd, t_done, 0, true, { Vsd, Hsd, Wsd }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW3D *//* EVEX VERSION */
      { e_vfnmsub132ps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vfnmsub132pd, t_done, 0, true, { Vpd, Hpd, Wpd }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW3E *//* EVEX VERSION */
      { e_vfnmsub132ss, t_done, 0, true, { Vss, Hss, Wss }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vfnmsub132sd, t_done, 0, true, { Vsd, Hsd, Wsd }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW3F *//* EVEX VERSION */
      { e_vfmaddsub213ps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vfmaddsub213pd, t_done, 0, true, { Vpd, Hpd, Wpd }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW40 *//* EVEX VERSION */
      { e_vfmsubadd213ps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vfmsubadd213pd, t_done, 0, true, { Vpd, Hpd, Wpd }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW41 *//* EVEX VERSION */
      { e_vfmadd213ps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vfmadd213pd, t_done, 0, true, { Vpd, Hpd, Wpd }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW42 *//* EVEX VERSION */
      { e_vfmadd213ss, t_done, 0, true, { Vss, Hss, Wss }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vfmadd213sd, t_done, 0, true, { Vsd, Hsd, Wsd }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW43 *//* EVEX VERSION */
      { e_vfmsub213ps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vfmsub213pd, t_done, 0, true, { Vpd, Hpd, Wpd }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW44 *//* EVEX VERSION */
      { e_vfmsub213ss, t_done, 0, true, { Vss, Hss, Wss }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vfmsub213sd, t_done, 0, true, { Vsd, Hsd, Wsd }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW45 *//* EVEX VERSION */
      { e_vfnmadd213ps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vfnmadd213pd, t_done, 0, true, { Vpd, Hpd, Wpd }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW46 *//* EVEX VERSION */
      { e_vfnmadd213ss, t_done, 0, true, { Vss, Hss, Wss }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vfnmadd213sd, t_done, 0, true, { Vsd, Hsd, Wsd }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW47 *//* EVEX VERSION */
      { e_vfnmsub213ps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vfnmsub213pd, t_done, 0, true, { Vpd, Hpd, Wpd }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW48 *//* EVEX VERSION */
      { e_vfnmsub213ss, t_done, 0, true, { Vss, Hss, Wss }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vfnmsub213sd, t_done, 0, true, { Vsd, Hsd, Wsd }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW49 *//* EVEX VERSION */
      { e_vfmaddsub231ps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vfmaddsub231pd, t_done, 0, true, { Vpd, Hpd, Wpd }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW4A *//* EVEX VERSION */
      { e_vfmsubadd231ps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vfmsubadd231pd, t_done, 0, true, { Vpd, Hpd, Wpd }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW4B *//* EVEX VERSION */
      { e_vfmadd231ps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vfmadd231pd, t_done, 0, true, { Vpd, Hpd, Wpd }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW4C *//* EVEX VERSION */
      { e_vfmadd231ss, t_done, 0, true, { Vss, Hss, Wss }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vfmadd231sd, t_done, 0, true, { Vsd, Hsd, Wsd }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW4D *//* EVEX VERSION */
      { e_vfmsub231ps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vfmsub231pd, t_done, 0, true, { Vpd, Hpd, Wpd }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW4E *//* EVEX VERSION */
      { e_vfmsub231ss, t_done, 0, true, { Vss, Hss, Wss }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vfmsub231sd, t_done, 0, true, { Vsd, Hsd, Wsd }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW4F *//* EVEX VERSION */
      { e_vfnmadd231ps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vfnmadd231pd, t_done, 0, true, { Vpd, Hpd, Wpd }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW50 *//* EVEX VERSION */
      { e_vfnmadd231ss, t_done, 0, true, { Vss, Hss, Wss }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vfnmadd231sd, t_done, 0, true, { Vsd, Hsd, Wsd }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW51 *//* EVEX VERSION */
      { e_vfnmsub231ps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vfnmsub231pd, t_done, 0, true, { Vpd, Hpd, Wpd }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW52 *//* EVEX VERSION */
      { e_vfnmsub231ss, t_done, 0, true, { Vss, Hss, Wss }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vfnmsub231sd, t_done, 0, true, { Vsd, Hsd, Wsd }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW53 *//* EVEX VERSION */
      { e_vfpclassps, t_done, 0, true, { Wps, Vps, Ib }, 0, s1RW2R, 0 }, /* W = 0 */
      { e_vfpclasspd, t_done, 0, true, { Wpd, Vpd, Ib }, 0, s1RW2R, 0 }  /* W = 1 */
    }, { /* VEXW54 *//* EVEX VERSION */
      { e_vfpclassss, t_done, 0, true, { Wss, Vss, Ib }, 0, s1RW2R, 0 }, /* W = 0 */
      { e_vfpclasssd, t_done, 0, true, { Wsd, Vsd, Ib }, 0, s1RW2R, 0 }  /* W = 1 */
    }, { /* VEXW55 *//* EVEX VERSION */
      { e_vpgatherdd, t_done, 0, true, { Wss, Vss, Zz }, 0, s1RW2R, 0 }, /* W = 0 */
      { e_vpgatherdq, t_done, 0, true, { Wsd, Vsd, Zz }, 0, s1RW2R, 0 }  /* W = 1 */
    }, { /* VEXW56 *//* EVEX VERSION */
      { e_vpgatherqd, t_done, 0, true, { Wss, Vss, Zz }, 0, s1RW2R, 0 }, /* W = 0 */
      { e_vpgatherqq, t_done, 0, true, { Wsd, Vsd, Zz }, 0, s1RW2R, 0 }  /* W = 1 */
    }, { /* VEXW57 *//* EVEX VERSION */
      { e_vgatherdps, t_done, 0, true, { Wss, Vss, Zz }, 0, s1RW2R, 0 }, /* W = 0 */
      { e_vgatherdpd, t_done, 0, true, { Wsd, Vsd, Zz }, 0, s1RW2R, 0 }  /* W = 1 */
    }, { /* VEXW58 *//* EVEX VERSION */
      { e_vgatherqps, t_done, 0, true, { Wss, Vss, Zz }, 0, s1RW2R, 0 }, /* W = 0 */
      { e_vgatherqpd, t_done, 0, true, { Wsd, Vsd, Zz }, 0, s1RW2R, 0 }  /* W = 1 */
    }, { /* VEXW59 *//* EVEX VERSION */
      { e_vgetexpps, t_done, 0, true, { Wps, Vps, Zz }, 0, s1RW2R, 0 }, /* W = 0 */
      { e_vgetexppd, t_done, 0, true, { Wpd, Vpd, Zz }, 0, s1RW2R, 0 }  /* W = 1 */
    }, { /* VEXW5A *//* EVEX VERSION */
      { e_vgetexpss, t_done, 0, true, { Vss, Hss, Wss }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vgetexpsd, t_done, 0, true, { Vsd, Hsd, Wsd }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW5B */
      { e_vgetmantps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R4R, 0 }, /* W = 0 */
      { e_vgetmantpd, t_done, 0, true, { Vpd, Hpd, Wpd }, 0, s1RW2R3R4R, 0 }  /* W = 1 */
    }, { /* VEXW5C */
      { e_vgetmantsd, t_done, 0, true, { Vss, Hss, Wss }, 0, s1RW2R3R4R, 0 }, /* W = 0 */
      { e_vgetmantss, t_done, 0, true, { Vsd, Hsd, Wsd }, 0, s1RW2R3R4R, 0 }  /* W = 1 */
    }, { /* VEXW5D */
      { e_vinsertf32x4, t_done, 0, true, { Vss, Hss, Wss }, 0, s1RW2R3R4R, 0 }, /* W = 0 */
      { e_vinsertf64x4, t_done, 0, true, { Vsd, Hsd, Wsd }, 0, s1RW2R3R4R, 0 }  /* W = 1 */
    }, { /* VEXW5E */
      { e_vinsertf32x8, t_done, 0, true, { Vss, Hss, Wss }, 0, s1RW2R3R4R, 0 }, /* W = 0 */
      { e_vinsertf64x4, t_done, 0, true, { Vsd, Hsd, Wsd }, 0, s1RW2R3R4R, 0 }  /* W = 1 */
    }, { /* VEXW5F */
      { e_vinserti32x4, t_done, 0, true, { Vss, Hss, Wss }, 0, s1RW2R3R4R, 0 }, /* W = 0 */
      { e_vinserti64x2, t_done, 0, true, { Vsd, Hsd, Wsd }, 0, s1RW2R3R4R, 0 }  /* W = 1 */
    }, { /* VEXW60 */
      { e_vinserti32x8, t_done, 0, true, { Vss, Hss, Wss }, 0, s1RW2R3R4R, 0 }, /* W = 0 */
      { e_vinserti64x4, t_done, 0, true, { Vsd, Hsd, Wsd }, 0, s1RW2R3R4R, 0 }  /* W = 1 */
    }, { /* VEXW61 */
      { e_vmovdqa32, t_done, 0, true, { Wps, Vps, Zz }, 0, s1RW2R, 0 }, /* W = 0 */
      { e_vmovdqa64, t_done, 0, true, { Wps, Vps, Zz }, 0, s1RW2R, 0 }  /* W = 1 */
    }, { /* VEXW62 */
      { e_vmovdqu8, t_done, 0, true, { Wps, Vps, Zz }, 0, s1RW2R, 0 }, /* W = 0 */
      { e_vmovdqu16, t_done, 0, true, { Wps, Vps, Zz }, 0, s1RW2R, 0 }  /* W = 1 */
    }, { /* VEXW63 */
      { e_vmovdqu32, t_done, 0, true, { Wps, Vps, Zz }, 0, s1RW2R, 0 }, /* W = 0 */
      { e_vmovdqu64, t_done, 0, true, { Wps, Vps, Zz }, 0, s1RW2R, 0 }  /* W = 1 */
    }, { /* VEXW64 */
      { e_vpandd, t_done, 0, true, { Wps, Vps, Zz }, 0, s1RW2R, 0 }, /* W = 0 */
      { e_vpandq, t_done, 0, true, { Wps, Vps, Zz }, 0, s1RW2R, 0 }  /* W = 1 */
    }, { /* VEXW65 */
      { e_vpandnd, t_done, 0, true, { Wps, Vps, Zz }, 0, s1RW2R, 0 }, /* W = 0 */
      { e_vpandnq, t_done, 0, true, { Wps, Vps, Zz }, 0, s1RW2R, 0 }  /* W = 1 */
    }, { /* VEXW66 */
      { e_vpcompressd, t_done, 0, true, { Wps, Vps, Zz }, 0, s1RW2R, 0 }, /* W = 0 */
      { e_vpcompressq, t_done, 0, true, { Wps, Vps, Zz }, 0, s1RW2R, 0 }  /* W = 1 */
    }, { /* VEXW67 */
      { e_vpconflictd, t_done, 0, true, { Wps, Vps, Zz }, 0, s1RW2R, 0 }, /* W = 0 */
      { e_vpconflictq, t_done, 0, true, { Wps, Vps, Zz }, 0, s1RW2R, 0 }  /* W = 1 */
    }, { /* VEXW68 */
      { e_vpermb, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vpermw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW69 */
      { e_vpermi2b, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vpermi2w, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW6A */
      { e_vpermi2d, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vpermi2q, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW6B */
      { e_vpermi2ps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vpermi2pd, t_done, 0, true, { Vpd, Hpd, Wpd }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW6C */
      { e_vpermt2b, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vpermt2w, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW6D */
      { e_vpermt2d, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vpermt2q, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW6E */
      { e_vpermt2ps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vpermt2pd, t_done, 0, true, { Vpd, Hpd, Wpd }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW6F */
      { e_vpexpandd, t_done, 0, true, { Wps, Vps, Zz }, 0, s1RW2R, 0 }, /* W = 0 */
      { e_vpexpandq, t_done, 0, true, { Wps, Vps, Zz }, 0, s1RW2R, 0 }  /* W = 1 */
    }, { /* VEXW70 */
      { e_vplzcntd, t_done, 0, true, { Wps, Vps, Zz }, 0, s1RW2R, 0 }, /* W = 0 */
      { e_vplzcntq, t_done, 0, true, { Wps, Vps, Zz }, 0, s1RW2R, 0 }  /* W = 1 */
    }, { /* VEXW71 */
      { e_vpmaxsd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vpmaxsq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW72 */
      { e_vpmaxud, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vpmaxuq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW73 */
      { e_vpminsd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vpminsq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW74 */
      { e_vpminud, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vpminuq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW75 */
      { e_vpord, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vporq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW76 */
      { e_vprolvd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vprolvq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW77 */
      { e_vprold, t_done, 0, true, { Vps, Wps, Ib }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vprolq, t_done, 0, true, { Vps, Wps, Ib }, 0, s1RW2R3R, 0 }, /* W = 1 */
    }, { /* VEXW78 */
      { e_vprorvd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vprorvq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW79 */
      { e_vprord, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vprorq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW7A */
      { e_vpscatterdd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vpscatterdq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW7B */
      { e_vpscatterqd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vpscatterqq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW7C */
      { e_vpsrldq, t_done, 0, true, { Wps, Vps, Ib }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vpsrlq, t_done, 0, true, { Wps, Vps, Ib }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW7D */
      { e_vpsllvd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vpsllvw, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW7E */
      { e_vshuff32x4, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R4R, 0 }, /* W = 0 */
      { e_vshuff64x2, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R4R, 0 }  /* W = 1 */
    }, { /* VEXW7F */
      { e_vpternlogd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R4R, 0 }, /* W = 0 */
      { e_vpternlogq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R4R, 0 }  /* W = 1 */
    }, { /* VEXW80 */
      { e_vpsravd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vpsravq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW81 */
      { e_vpxord, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vpxorq, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW82 */
      { e_vrangeps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R4R, 0 }, /* W = 0 */
      { e_vrangepd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R4R, 0 }  /* W = 1 */
    }, { /* VEXW83 */
      { e_vrangess, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R4R, 0 }, /* W = 0 */
      { e_vrangesd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R4R, 0 }  /* W = 1 */
    }, { /* VEXW84 */
      { e_vrcp14ps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vrcp14pd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW85 */
      { e_vrcp14ss, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vrcp14sd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW86 */
      { e_vreduceps, t_done, 0, true, { Wps, Vps, Ib }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vreducepd, t_done, 0, true, { Wps, Vps, Ib }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW87 */
      { e_vreducess, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R4R, 0 }, /* W = 0 */
      { e_vreducesd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R4R, 0 }  /* W = 1 */
    }, { /* VEXW88 */
      { e_vrsqrt14ps, t_done, 0, true, { Wps, Vps, Zz }, 0, s1RW2R, 0 }, /* W = 0 */
      { e_vrsqrt14pd, t_done, 0, true, { Wps, Vps, Zz }, 0, s1RW2R, 0 }  /* W = 1 */
    }, { /* VEXW89 */
      { e_vrsqrt14ss, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vrsqrt14sd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW8A */
      { e_vscatterdps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vscatterdpd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW8B */
      { e_vscatterqps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vscatterqpd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW8C */
      { e_vexp2ps, t_done, 0, true, { Wps, Vps, Zz }, 0, s1RW2R, 0 }, /* W = 0 */
      { e_vexp2pd, t_done, 0, true, { Wpd, Vpd, Zz }, 0, s1RW2R, 0 }  /* W = 1 */
    }, { /* VEXW8D */
      { e_vrcp28ps, t_done, 0, true, { Wps, Vps, Zz }, 0, s1W2R, 0 }, /* W = 0 */
      { e_vrcp28pd, t_done, 0, true, { Wpd, Vpd, Zz }, 0, s1W2R, 0 }  /* W = 1 */
    }, { /* VEXW8E */
      { e_vrcp28ss, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vrcp28sd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1RW2R3R, 0 }  /* W = 1 */
    }, { /* VEXW8F */
      { e_vrsqrt28ps, t_done, 0, true, { Vps, Wps, Zz }, 0, s1W2R, 0 }, /* W = 0 */
      { e_vrsqrt28pd, t_done, 0, true, { Vps, Wps, Zz }, 0, s1W2R, 0 }  /* W = 1 */
    }, { /* VEXW90 */
      { e_vrsqrt28ss, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 }, /* W = 0 */
      { e_vrsqrt28sd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 }  /* W = 1 */
    }, { /* VEXW91 */
      { e_vpandd, t_done, 0, true, { Wps, Vps, Zz }, 0, s1RW2R, 0 }, /* W = 0 */
      { e_vpandq, t_done, 0, true, { Wps, Vps, Zz }, 0, s1RW2R, 0 }  /* W = 1 */
    }, { /* VEXW92 */
      { e_vgatherdps, t_done, 0, true, { Wss, Vss, Zz }, 0, s1RW2R, 0 }, /* W = 0 */
      { e_vgatherdpd, t_done, 0, true, { Wsd, Vsd, Zz }, 0, s1RW2R, 0 }  /* W = 1 */
    }, { /* VEXW93 */
      { e_vpbroadcastd, t_done, 0, true, { Vps, Wq, Zz }, 0, s1W2R, 0 }, /* W = 0 */
      { e_vpbroadcastq, t_done, 0, true, { Vps, Wq, Zz }, 0, s1W2R, 0 }  /* W = 1 */
    }, { /* VEXW94 */
      { e_blendps, t_done, 0, true, { Vdq, Wdq, Ib }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_blendpd, t_done, 0, true, { Vdq, Wdq, Ib }, 0, s1RW2R3R, 0 }, /* W = 1 */
    }, { /* VEXW95 */
      { e_vfmaddps, t_done, 0, true, { Vdq, Wdq, Ib }, 0, s1RW2R3R, 0 }, /* W = 0 */
      { e_vfmaddpd, t_done, 0, true, { Vdq, Wdq, Ib }, 0, s1RW2R3R, 0 }, /* W = 1 */
    }, { /* VEXW96 */
      { e_vpermps, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 }, /* W = 0 */
      { e_vpermpd, t_done, 0, true, { Vps, Hps, Wps }, 0, s1W2R3R, 0 }  /* W = 1 */
    }
};
/** END_DYNINST_TABLE_VERIFICATION */
#undef VEX3_ILL

/* FMA4 deocoding table
 * W impacts the operand order */
static struct ia32_entry fma4Map[][2] =
{    
   { /* FMA46A */
       { e_vfmaddss, t_done, 0, true, {Vss, Hss, Wd, Lb}, 0, s1W2R3R4R, 0}, /* W = 0 */
       { e_vfmaddss, t_done, 0, true, {Vss, Hss, Wd, Lb}, 0, s1W2R3R4R, 0}, /* W = 1 */
   }, { /* FMA46B */
       { e_vfmaddsd, t_done, 0, true, {Vss, Hss, Wps, Lb}, 0, s1W2R3R4R, 0}, /* W = 0 */
       { e_vfmaddsd, t_done, 0, true, {Vss, Hss, Wps, Lb}, 0, s1W2R3R4R, 0}, /* W = 1 */
   }, { /* FMA46D */
       { e_vfmsubpd, t_done, 0, true, {Vss, Hss, Wps, Lb}, 0, s1W2R3R4R, 0}, /* W = 0 */
       { e_vfmsubpd, t_done, 0, true, {Vss, Hss, Wps, Lb}, 0, s1W2R3R4R, 0}, /* W = 1 */
   }, { /* FMA46F */
       { e_vfmsubsd, t_done, 0, true, {Vss, Hss, Wq, Lb}, 0, s1W2R3R4R, 0}, /* W = 0 */
       { e_vfmsubsd, t_done, 0, true, {Vss, Hss, Wq, Lb}, 0, s1W2R3R4R, 0}, /* W = 1 */
   }, { /* FMA479 */
       { e_vfnmaddpd, t_done, 0, true, {Vss, Hss, Wps, Lb}, 0, s1W2R3R4R, 0}, /* W = 0 */
       { e_vfnmaddpd, t_done, 0, true, {Vss, Hss, Wps, Lb}, 0, s1W2R3R4R, 0}, /* W = 1 */
   }, { /* FMA47B */
       { e_vfnmaddsd, t_done, 0, true, {Vss, Hss, Wq, Lb}, 0, s1W2R3R4R, 0}, /* W = 0 */
       { e_vfnmaddsd, t_done, 0, true, {Vss, Hss, Wq, Lb}, 0, s1W2R3R4R, 0}, /* W = 1 */
   }, 
};

static struct ia32_entry XOP8[256] = 
{
		/* 00 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 08 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 10 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 18 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 20 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 28 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 30 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 38 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 40 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 48 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 50 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 58 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 60 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 68 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 70 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 78 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 80 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 88 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 90 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 98 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_vpmacsdd, t_done, 0, true, { Vss, Hss, Wps, Lb }, 0, s1W2R3R4R, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* A0 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_xop_8_w, XOP8_A2, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_xop_8_w, XOP8_A3, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* A8 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* B0 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* B8 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* C0 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* C8 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_vpcomd, t_done, 0, true, { Vss, Hss, Wps, Ib }, 0, s1W2R3R4R, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* D0 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* D8 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* E0 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* E8 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* F0 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* F8 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
};


static struct ia32_entry XOP9[256] = 
{
		/* 00 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 08 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 10 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 18 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 20 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 28 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 30 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 38 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 40 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 48 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 50 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 58 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 60 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 68 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 70 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 78 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 80 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 88 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 90 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 98 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_xop_9_w, XOP9_9A, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* A0 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* A8 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* B0 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* B8 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* C0 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* C8 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* D0 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* D8 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* E0 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* E8 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* F0 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* F8 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
};

static struct ia32_entry XOPA[256] = 
{
		/* 00 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 08 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 10 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 18 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 20 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 28 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 30 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 38 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 40 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 48 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 50 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 58 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 60 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 68 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 70 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 78 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 80 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 88 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 90 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* 98 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* A0 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* A8 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* B0 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* B8 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* C0 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* C8 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* D0 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* D8 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* E0 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* E8 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* F0 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		/* F8 */
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
		{ e_No_Entry, t_ill, 0, false, { Zz, Zz, Zz }, 0, 0, 0 },
};

static struct ia32_entry XOP8_W[][2] = 
{
    { /* XOP8_A2 */
      { e_vpcmov, t_done, 0, true, { Vss, Hss, Wps, Lb }, 0, s1W2R3R4R, 0 }, /* W = 0 */
      { e_vpcmov, t_done, 0, true, { Vss, Hss, Wps, Lb }, 0, s1W2R3R4R, 0 }  /* W = 1 */
    }, { /* XOP8_A3 */
      { e_vpperm, t_done, 0, true, { Vss, Hss, Wps, Lb }, 0, s1W2R3R4R, 0 }, /* W = 0 */
      { e_vpperm, t_done, 0, true, { Vss, Hss, Wps, Lb }, 0, s1W2R3R4R, 0 }  /* W = 1 */
    }
};


static struct ia32_entry XOP9_W[][2] = 
{
    { /* XOP9_9A */
      { e_vpshad, t_done, 0, true, { Vss, Wps, Hss }, 0, s1W2R3R, 0 }, /* W = 0 */
      { e_vpshad, t_done, 0, true, { Vss, Hss, Wps }, 0, s1W2R3R, 0 }  /* W = 1 */
    }
};


ia32_entry movsxd = { e_movsxd, t_done, 0, true, { Gv, Ed, Zz }, 0, s1W2R, 0 };
ia32_entry invalid = { e_No_Entry, t_ill, 0, true, { Zz, Zz, Zz }, 0, 0, 0 };
ia32_entry seg_mov = { e_mov, t_done, 0, true, {Ev, Sw, Zz}, 0, s1W2R, 0 };
static void ia32_translate_for_64(ia32_entry** gotit_ptr)
{
    if (*gotit_ptr == &oneByteMap[0x63]) // APRL redefined to MOVSXD
	    *gotit_ptr = &movsxd;
    if (*gotit_ptr == &oneByteMap[0x06] || // Invalid instructions in 64-bit mode: push es
	        *gotit_ptr == &oneByteMap[0x07] || // pop es
	        *gotit_ptr == &oneByteMap[0x0E] || // push cs
	        *gotit_ptr == &oneByteMap[0x16] || // push ss
	        *gotit_ptr == &oneByteMap[0x17] || // pop ss
	        *gotit_ptr == &oneByteMap[0x1E] || // push ds
	        *gotit_ptr == &oneByteMap[0x1F] || // pop ds
	        *gotit_ptr == &oneByteMap[0x27] || // daa
	        *gotit_ptr == &oneByteMap[0x2F] || // das
	        *gotit_ptr == &oneByteMap[0x37] || // aaa
	        *gotit_ptr == &oneByteMap[0x3F] || // aas
	        *gotit_ptr == &oneByteMap[0x60] || // pusha
	        *gotit_ptr == &oneByteMap[0x61] || // popa
	        *gotit_ptr == &oneByteMap[0x62] || // bound gv, ma
	        *gotit_ptr == &oneByteMap[0x82] || // group 1 eb/ib
	        *gotit_ptr == &oneByteMap[0x9A] || // call ap
	        *gotit_ptr == &oneByteMap[0xC4] || // les gz, mp
	        *gotit_ptr == &oneByteMap[0xC5] || // lds gz, mp
	        *gotit_ptr == &oneByteMap[0xCE] || // into
	        *gotit_ptr == &oneByteMap[0xD4] || // aam ib
	        *gotit_ptr == &oneByteMap[0xD5] || // aad ib
	        *gotit_ptr == &oneByteMap[0xD6] || // salc
	        *gotit_ptr == &oneByteMap[0xEA]) // jump ap
    {
        *gotit_ptr = &invalid;
    }

    if(*gotit_ptr == &oneByteMap[0x8C]) {
	    *gotit_ptr = &seg_mov;
    }
    
}

/* full decoding version: supports memory access information */
static unsigned int ia32_decode_modrm(const unsigned int addrSzAttr, const unsigned char *addr, ia32_memacc *macadr,
                                      const ia32_prefixes *pref, ia32_locations *pos, bool mode_64);


void ia32_memacc::print()
{
    fprintf(stderr, "base: %d, index: %d, scale:%d, disp: %ld (%lx), size: %d, addr_size: %d\n",
	    regs[0], regs[1], scale, imm, (unsigned long)imm, size, addr_size);
}

int getOperSz(const ia32_prefixes &pref) 
{
    /* TODO: VEX prefixed instructions only touch XMM or YMM unless they are loading/storing to memory. */
    if(pref.vex_present)
    {
        switch(pref.vex_ll)
        {
            case 0:
                return 16;
            case 1:
                return 32;
            case 2:
                return 64;
            default: /* Shouldn't be valid */
                return 16;
        }
    }
    else if (pref.rexW()) return 4;
    else if (pref.getPrefix(2) == PREFIX_SZOPER) return 1;
    else return 2;
}


/**
 * This is the main Intel x86/x86_64 decoding function. This is used to determine
 * the instruction mnemonic, operands and the overall instruction length.
 *
 * @param capa A mask of capabilities that should be enabled for decoding.
 * @param addr A pointer to the start of the instruction that should be decoded.
 * @param instruc A reference to an ia32_instruction that should be setup with the decoded instruction.
 */
ia32_instruction &ia32_decode(unsigned int capa, const unsigned char *addr, ia32_instruction &instruct, bool mode_64)
{
    const unsigned char* addr_orig = addr; /* The original start to this instruction (addr will change) */
    ia32_prefixes& pref = instruct.prf; /* A reference to the prefix information for this instruction */
    ia32_entry *gotit = NULL; /* A pointer to the descriptor for the decoded instruction */

    /* If we are being assed to decode memory accesses, then instrut.mac must not be null. */
    if(capa & IA32_DECODE_MEMACCESS)
        assert(instruct.mac != NULL);

    /* First decode any prefixes for this instruction */
    if (!ia32_decode_prefixes(addr, instruct, mode_64))
    {
#ifdef VEX_DEBUG
        fprintf(stderr, "PREFIX DECODE FAILURE\n");
#endif
        instruct.size = 1;
	    instruct.entry = NULL;
        instruct.legacy_type = ILLEGAL;
        return instruct;
    }

    /* Skip the prefixes so that we don't decode them again */
    addr = addr_orig + instruct.size;

    /* Get the entry in the decoding tables */
    int opcode_decoding;

    if((opcode_decoding = ia32_decode_opcode(capa, addr, instruct, &gotit, mode_64)))
    {
        if(opcode_decoding > 0)
        {
            /* FPU decoding success. Return immediately */
            return instruct;
        }

        /* Opcode decoding failed */
        instruct.entry = NULL;
        instruct.legacy_type = ILLEGAL;
        return instruct;
    }

    if(!gotit)
        assert(!"Didn't find a valid instruction, however decode suceeded.");

    /* Skip the opcode */
    addr = addr_orig + instruct.size;

    /* Do the operand decoding */
    ia32_decode_operands(pref, *gotit, addr, instruct, instruct.mac, mode_64);
    /* Decode the memory accesses if requested */
    if(capa & IA32_DECODE_MEMACCESS) 
    {
        int sema = gotit->opsema & ((1<<FPOS)-1);
        int hack = gotit->opsema >> FPOS;

        switch(sema) 
        {
            case sNONE:
                break;
            case s1R:
                switch(hack) 
                {
                    case fPREFETCHNT:
                        instruct.mac[0].prefetch = true;
                        instruct.mac[0].prefetchlvl = 0;
                        break;
                    case fPREFETCHT0:
                        instruct.mac[0].prefetch = true;
                        instruct.mac[0].prefetchlvl = 1;
                        break;
                    case fPREFETCHT1:
                        instruct.mac[0].prefetch = true;
                        instruct.mac[0].prefetchlvl = 2;
                        break;
                    case fPREFETCHT2:
                        instruct.mac[0].prefetch = true;
                        instruct.mac[0].prefetchlvl = 3;
                        break;
                    case fPREFETCHAMDE:
                        instruct.mac[0].prefetch = true;
                        instruct.mac[0].prefetchstt = 0;
                        break;
                    case fPREFETCHAMDW:
                        instruct.mac[0].prefetch = true;
                        instruct.mac[0].prefetchstt = 1;
                        break;
                    default:
                        instruct.mac[0].read = true;
                }
                break;
            case s1W:
                instruct.mac[0].write = true;
                break;
            case s1RW:
                instruct.mac[0].read = true;
                instruct.mac[0].write = true;
                instruct.mac[0].nt = hack == fNT;
                break;
            case s1R2R:
                instruct.mac[0].read = true;
                instruct.mac[1].read = true;
                break;
            case s1W2R:
                instruct.mac[0].write = true;
                instruct.mac[0].nt = hack == fNT; // all NTs are s1W2R
                instruct.mac[1].read = true;
                break;
            case s1RW2R:
                instruct.mac[0].read = true;
                instruct.mac[0].write = true;
                instruct.mac[1].read = true;
                break;
            case s1RW2RW:
                instruct.mac[0].read = true;
                instruct.mac[0].write = true;
                instruct.mac[1].read = true;
                instruct.mac[1].write = true;
                break;
            case s1W2R3R:
                instruct.mac[0].write = true;
                instruct.mac[1].read = true;
                instruct.mac[2].read = true;
                break;
            case s1W2W3R:
                instruct.mac[0].write = true;
                instruct.mac[1].write = true;
                instruct.mac[2].read = true;
                break;
            case s1W2RW3R:
                instruct.mac[0].write = true;
                instruct.mac[1].read = true;
                instruct.mac[1].write = true;
                instruct.mac[2].read = true;
                break;
            case s1W2R3RW:
                instruct.mac[0].write = true;
                instruct.mac[1].read = true;
                instruct.mac[2].read = true;
                instruct.mac[2].write = true;
                break;
            case s1RW2R3R:
                instruct.mac[0].read = true;
                instruct.mac[0].write = true;
                instruct.mac[1].read = true;
                instruct.mac[2].read = true;
                break;
            case s1RW2RW3R:
                instruct.mac[0].write = true;
                instruct.mac[0].read = true;
                instruct.mac[1].read = true;
                instruct.mac[1].write = true;
                instruct.mac[2].read = true;
                break;
            case s1RW2R3R4R:
                instruct.mac[0].write = true;
                instruct.mac[0].read = true;
                instruct.mac[1].read = true;
                instruct.mac[2].read = true;
                break;
            case s1W2R3R4R:
                instruct.mac[0].write = true;
                instruct.mac[1].read = true;
                instruct.mac[2].read = true;
                break;
            case s1RW2R3RW:
                instruct.mac[0].read = true;
                instruct.mac[0].write = true;
                instruct.mac[1].read = true;
                instruct.mac[2].read = true;
                instruct.mac[2].write = true;
                break;
            default:
                // assert(!"Unknown addressing semantics!");
                break;
        }

        switch(pref.getPrefix(0)) 
        {
            case PREFIX_REPNZ:
                switch(hack) 
                {
                    case fSCAS:
                        instruct.mac[1].sizehack = shREPNESCAS;
                        break;
                    case fCMPS:
                        instruct.mac[0].sizehack = shREPNECMPS;
                        instruct.mac[1].sizehack = shREPNECMPS;
                        break;
                    default:
	                    break;
                }
                break;
            case PREFIX_REP:
                switch(hack) 
                {
                    case fSCAS:
                        instruct.mac[1].sizehack = shREPESCAS;
                        break;
                    case fCMPS:
                        instruct.mac[0].sizehack = shREPECMPS;
                        instruct.mac[1].sizehack = shREPECMPS;
                        break;
                    case fREP:
                        instruct.mac[0].sizehack = shREP;
                        instruct.mac[1].sizehack = shREP;
                        break;
                    default:
	                    break;
                }
                break;
            case 0:
            case PREFIX_LOCK:
            default:
                break;
        }

        // debug output for memory access decoding
#if 0
        for (int i = 0; i < 3; i++) 
        {
	        if (instruct.mac[i].is) 
            {
	            fprintf(stderr, "%d)", i);

	            if (instruct.mac[i].read)
		            fprintf(stderr, " read");
	            if (instruct.mac[i].write)
		            fprintf(stderr, " write");
	            if (instruct.mac[i].nt)
		            fprintf(stderr, " nt");
	            if (instruct.mac[i].sizehack)
		            fprintf(stderr, " sizehack");

	            fprintf(stderr, "\n");
	            instruct.mac[i].print();
	        }
        }
#endif

    }

    instruct.entry = gotit;
    return instruct;
}

/**
 * Get the instruction table descriptor for the given instructino.
 *
 * @param capa The capabilities that should be enabled for this instruction decoding.
 * @param addr The start of the opcode. WARNING: Prefixes must already be decoded!
 * @param instruct The instruction structure to fill out with the decoding information.
 * @param gotit_ret The ia32_entry that we stopped at. NULL if there was an issue.
 *
 * @return >= 0 on success. < 0 on failure. A result greater that zero usually means
 *          that it was an FPU instruction that has been fully decoded.
 */
int ia32_decode_opcode(unsigned int capa, const unsigned char *addr, ia32_instruction &instruct, ia32_entry **gotit_ret,
                       bool mode_64)
{
    ia32_prefixes& pref = instruct.prf;
    unsigned int table, nxtab, idx;
    int sseidx = 0;
    ia32_entry* gotit = NULL;
    int condbits = 0;
    bool vextab = false; /* Did we end in a VEX table? */
    if (pref.XOP) {
        /* Grab the opcode for the index */
        idx = addr[0];

        /* Move past the opcode for this instruction */
        instruct.size += 1;
        addr += 1;

        // We reuse vex3 data structures for XOP
        switch(pref.vex_m_mmmm)
        {
            case 8:
                gotit = &XOP8[idx];
                break;
            case 9:
                gotit = &XOP9[idx];
                break;
            case 10:
                gotit = &XOPA[idx];
                break;
            default:
                instruct.legacy_type = ILLEGAL;
                instruct.entry = NULL;
                return -1;
        }
        nxtab = gotit->otable;
    } else if(pref.vex_present)
    /* Is there a VEX prefix for this instruction? */
    {
#ifdef VEX_PEDANTIC
        printf("DECODING VEX\n");
#endif
        /* Grab the opcode for the index */
        idx = addr[0];

        /* Move past the opcode for this instruction */
        instruct.size += 1;
        addr += 1;

        switch(pref.vex_type)
        {
            case VEX_TYPE_VEX2:
                /* This is a VEX2 prefixed instruction -- start in the twoByteMap */
                gotit = &twoByteMap[idx];
                sseidx = vex3_simdop_convert[0][pref.vex_pp];
                break;

            case VEX_TYPE_VEX3:
            case VEX_TYPE_EVEX:
                /* Make sure we start in the proper table */
                switch(pref.vex_m_mmmm)
                {
                    case 1:
                        gotit = &twoByteMap[idx];
                        sseidx = vex3_simdop_convert[0][pref.vex_pp];
                        break;
                    case 2:
                        gotit = &threeByteMap[idx];
                        sseidx = vex3_simdop_convert[1][pref.vex_pp];
                        break;
                    case 3:
                        gotit = &threeByteMap2[idx];
                        sseidx = vex3_simdop_convert[2][pref.vex_pp];
                        break;
                    default:
                        instruct.legacy_type = ILLEGAL;
                        instruct.entry = NULL;
                        return -1;
                }

                if(sseidx < 0)
                {
                    /* This instruction can't be expressed with an sseidx */
                    instruct.legacy_type = ILLEGAL;
                    instruct.entry = NULL;
                    return -1;
                }
                break;
            default:
                assert(!"Invalid type of VEX instruction!\n");
        }

        nxtab = gotit->otable;
    } else {
        /* Non VEX instruction */
        table = t_oneB;

        /* Adjust the idx */
        idx = addr[0];
        instruct.size += 1;
        addr += 1;

        gotit = &oneByteMap[idx];
        nxtab = gotit->otable;
    }

    if(capa & IA32_DECODE_CONDITION)
    {
        assert(instruct.cond != NULL);
        condbits = idx & 0x0F;
    }

    /* Find the correct entry in the tables */
    while(nxtab != t_done)
    {
        vextab = false;
        table = nxtab;
        switch(table)
        {
            case t_twoB:
                idx = addr[0];
                gotit = &twoByteMap[idx];
                nxtab = gotit->otable;
                instruct.size += 1;
                addr += 1;
                if(capa & IA32_DECODE_CONDITION)
                    condbits = idx & 0x0F;
                break;
            case t_threeB:
                idx = addr[0];
                gotit = &threeByteMap[idx];
                nxtab = gotit->otable;
                instruct.size += 1;
                addr += 1;
                if(capa & IA32_DECODE_CONDITION)
                    condbits = idx & 0x0F;
                break;
            case t_threeB2:
                idx = addr[0];
                gotit = &threeByteMap2[idx];
                nxtab = gotit->otable;
                instruct.size += 1;
                addr += 1;
                if(capa & IA32_DECODE_CONDITION)
                    condbits = idx & 0x0F;
                break;
            case t_sse:
                /* Decode the sse prefix for this type */
                switch(pref.getOpcodePrefix())
                {
                    case 0x00:
                        sseidx = 0;
                        break;
                    case 0xf3:
                        sseidx = 1;
                        break;
                    case 0x66:
                        sseidx = 2;
                        break;
                    case 0xF2:
                        sseidx = 3;
                        break;
                }

                idx = gotit->tabidx;
                gotit = &sseMap[idx][sseidx];
                nxtab = gotit->otable;

#ifdef VEX_DEBUG
                fprintf(stderr, "SSE MAP   idx: %d 0x%x  sseidx: %d 0x%x\n", idx, idx, sseidx, sseidx);
                fprintf(stderr, "HAS VEX? %s\n", pref.vex_present ? "YES" : "NO");
                fprintf(stderr, "NEXT TAB == SSE_MULT? %s\n", nxtab == t_sse_mult ? "YES" : "NO");
                fprintf(stderr, "NEXT TAB == DONE? %s\n", nxtab == t_done ? "YES" : "NO");
#endif

                /* If there is no vex prefix, we're done */
                if(!pref.vex_present)
                    nxtab = t_done;
                break;
            case t_sse_mult:
                if(!pref.vex_present)
                    assert(!"Entered VEX SSE MULT table when no VEX present.");
                idx = gotit->tabidx;
                gotit = &sseMapMult[idx][pref.vex_sse_mult];
                nxtab = gotit->otable;
                vextab = true;

#ifdef VEX_DEBUG
                fprintf(stderr, "SSE MULT MAP   idx: %d  sseidx: %d sse_mult: %d\n", idx, sseidx, pref.vex_sse_mult);
                fprintf(stderr, "NEXT TAB == DONE? %s\n", nxtab == t_done ? "YES" : "NO");
                fprintf(stderr, "NEXT TAB == VEXW? %s\n", nxtab == t_vexw ? "YES" : "NO");
#endif

                break;
            case t_sse_bis:
                /* Decode the sse prefix for this type */
                switch(pref.getOpcodePrefix())
                {
                    case 0x00:
                        sseidx = 0;
                        break;
                    case 0xf3:
                        sseidx = 1;
                        break;
                    case 0x66:
                        sseidx = 2;
                        break;
                    case 0xF2:
                        sseidx = 3;
                        break;
                }

                idx = gotit->tabidx;
                gotit = &sseMapBis[idx][sseidx];
                nxtab = gotit->otable;

#ifdef VEX_DEBUG
                fprintf(stderr, "SSEB MAP  idx: %d  sseidx: %d\n", idx, sseidx);
#endif

                /* If there is no vex prefix, we're done */
                if(!pref.vex_present)
                    nxtab = t_done;
                break;
            case t_sse_bis_mult:
                if(!pref.vex_present)
                    assert(!"Entered VEX BIS MULT table when no VEX present.");
                idx = gotit->tabidx;
                gotit = &sseMapBisMult[idx][pref.vex_sse_mult];
                nxtab = gotit->otable;
                vextab = true;

#ifdef VEX_DEBUG
                fprintf(stderr, "SSEB MULT idx: %d  sseMul: %d\n", idx, pref.vex_sse_mult);
#endif
                break;
            case t_sse_ter:
                /* Decode the sse prefix for this type */
                switch(pref.getOpcodePrefix())
                {
                    case 0x00:
                        sseidx = 0;
                        break;
                    case 0x66:
                        sseidx = 1;
                        break;
                    case 0xF2:
                        sseidx = 2;
                        break;
                }

                idx = gotit->tabidx;
                gotit = &sseMapTer[idx][sseidx];
                nxtab = gotit->otable;

#ifdef VEX_DEBUG
                fprintf(stderr, "SSET MAP  idx: %d  sseidx: %d\n", idx, sseidx);
#endif

                /* If there is no vex prefix, we're done */
                if(!pref.vex_present)
                    nxtab = t_done;
                break;
            case t_sse_ter_mult:
                if(!pref.vex_present)
                    assert(!"Entered VEX TER MULT table when no VEX present.");

                idx = gotit->tabidx;
                gotit = &sseMapTerMult[idx][pref.vex_sse_mult];
                nxtab = gotit->otable;
                vextab = true;

#ifdef VEX_DEBUG
                fprintf(stderr, "SSET MULT idx: %d  sseMul: %d\n", idx, pref.vex_sse_mult);
#endif
                break;

            case t_grp:
                {
                    idx = gotit->tabidx;
                    unsigned int reg  = (addr[0] >> 3) & 7;
                    vextab = true;
                    if(idx < Grp12)
                        switch(idx)
                        {
                            case Grp2:
                            case Grp11:
                                /* leave table unchanged because operands are in not
                                   defined in group map, unless this is an invalid index
                                   into the group, in which case we need the instruction
                                   to reflect its illegal status */
                                if(groupMap[idx][reg].id == e_No_Entry)
                                    gotit = &groupMap[idx][reg];
                                nxtab = groupMap[idx][reg].otable;
                                assert(nxtab==t_done || nxtab==t_ill);
                                break;
                            default:
                                gotit = &groupMap[idx][reg];
                                nxtab = gotit->otable;
                        }
                    else {
                        unsigned int mod = addr[0] >> 6;
                        gotit = &groupMap2[idx-Grp12][mod==3][reg];
                        nxtab = gotit->otable;
                    }
                    break;
                }
            case t_grpsse:
                switch(pref.getOpcodePrefix())
                {
                    case 0x00:
                    case 0xF3:
                        sseidx = 0;
                        break;
                    case 0xF2:
                    case 0x66:
                        sseidx = 1;
                        break;
                    default:
                        assert(!"Unknown opcode prefixed used for t_grpsse table.\n");
                        break;
                }

                // sseidx >>= 1;
                idx = gotit->tabidx;
                vextab = true;
                if(pref.vex_present)
                    gotit = &ssegrpMap_VEX[idx][sseidx];
                else
                    gotit = &ssegrpMap[idx][sseidx];

                nxtab = gotit->otable;
                break;

            case t_coprocEsc:
                {
                    instruct.legacy_type = 0;
                    unsigned int reg  = (addr[0] >> 3) & 7;
                    unsigned int mod = addr[0] >> 6;
                    gotit = &fpuMap[gotit->tabidx][mod==3][reg];
                    ia32_decode_FP(idx, pref, addr, instruct, gotit, instruct.mac, mode_64);
                    if(gotit_ret)
                        *gotit_ret = gotit;
                    return 1; /* Decoding success */
                }

            case t_3dnow:
                // 3D now opcodes are given as suffix: ModRM [SIB] [displacement] opcode
                // Right now we don't care what the actual opcode is, so there's no table
                nxtab = t_done;
                break;
            case t_vexl:
                /* This can have a vex prefix */
                if(!pref.vex_present)
                {
                    /* If this instruction is valid without it, then it's fine */
                    nxtab = t_done;
                    break;
                }

                /* Whats the index into the vex2 table? */
                idx = gotit->tabidx;
                /* Set the current entry */
                gotit = &vex2Map[idx][pref.vex_ll];
                /* Set the next table - this is almost always t_done */
                nxtab = gotit->otable;
                vextab = true;
                break;
            case t_vexw:
                /* This MUST have a vex prefix and must NOT be VEX2 */
                if(!pref.vex_present || pref.vex_type == VEX_TYPE_VEX2)
                {
#ifdef VEX_PEDANTIC
                    assert(!"VEXW can only be used by vex prefixed instructions!\n");
#endif
                    instruct.legacy_type = ILLEGAL;
                    instruct.entry = gotit;
                    return -1;
                }

                /* Whats the index into the vexWMap table? */
                idx = gotit->tabidx;

                /* Sanity check: does this index make sense? */
                if(idx > VEXW_MAX)
                {
#ifdef VEX_PEDANTIC
                    assert(!"VEXW index out of bounds!\n");
#endif
                    instruct.legacy_type = ILLEGAL;
                    instruct.entry = gotit;
                    return -1;
                }

#ifdef VEX_DEBUG
                fprintf(stderr, "VEXW ENTRY:      VEXW%x\n", idx);
                fprintf(stderr, "VEXW MAX ENTRY:  VEXW%lx\n", 
                        (sizeof(vexWMap) / sizeof(vexWMap[0])) - 1);
#endif

                /* Set the current entry */
                gotit = &vexWMap[idx][pref.vex_w];
                /* Set the next table - this is almost always t_done */
                nxtab = gotit->otable;

                if(nxtab != t_done)
                {
#ifdef VEX_PEDANTIC
                    assert(!"VEXW should always be the final table.\n");
#endif

                    instruct.legacy_type = ILLEGAL;
                    instruct.entry = gotit;
                    return -1;
                }

                vextab = true;
                break;
            case t_sse_vex_mult:
                /* Get the SSE entry */
                idx = gotit->tabidx;

#ifdef VEX_DEBUG
                printf("SSE_VEX_MULT  index: %d\n", idx);
                printf("SSE_VEX_MULT  has vex? %s\n", pref.vex_present ? "YES" : "NO");
#endif

                /* Switch based on whether or not VEX is present */
                if(pref.vex_present)
                {
                    switch(pref.vex_type)
                    {
                        case VEX_TYPE_VEX2:
                        case VEX_TYPE_VEX3:
                        case VEX_TYPE_EVEX:
                            break;
                        case VEX_TYPE_NONE:
                            assert(!"pref.vex_present set but no vex prefix!\n");
                        default:
                            assert(!"Invalid VEX prefix for sseVexMult table.\n");
                    }
                    gotit = &sseVexMult[idx][(int)pref.vex_type];
                } else gotit = &sseVexMult[idx][0];

                nxtab = gotit->otable;
                vextab = true;
                break;
            case t_fma4:
                /* FMA4 instructions for AMD */
                idx = gotit->tabidx;
                gotit = &fma4Map[idx][pref.vex_w];
                nxtab = gotit->otable;
                vextab = true;
                break;
            case t_xop_8_w:
                /* XOP 9 instructions with W selector for AMD */
                idx = gotit->tabidx;
                gotit = &XOP8_W[idx][pref.vex_w];
                nxtab = gotit->otable;
                vextab = true;
                break;
            case t_xop_9_w:
                /* XOP 9 instructions with W selector for AMD */
                idx = gotit->tabidx;
                gotit = &XOP9_W[idx][pref.vex_w];
                nxtab = gotit->otable;
                vextab = true;
                break;
            case t_ill:
#ifdef VEX_DEBUG
                if(pref.vex_present)
                {
                    printf("MISSING INSTRUCTION IN TABLE: %s\n",
                            (pref.vex_m_mmmm == 1 ? "twoByteMap" :
                             (pref.vex_m_mmmm == 2 ? "threeByteMap" : "threeByteMap2")));
                    printf("                     SSE_IDX: %d, previous idx %x\n", sseidx, idx);
                }
#endif

                /* Illegal or unknown instruction */
                instruct.legacy_type = ILLEGAL;
                instruct.entry = gotit;
                return -1;

            default:
                assert(!"wrong table");
        }
    }

    /* We should have a valid decoding or we should have returned by now */
    assert(gotit != NULL);
    instruct.legacy_type = gotit->legacyType;

    if(pref.vex_present && !vextab && false)
    {
#ifdef VEX_DEBUG
        printf("ERROR: This instruction doesn't support VEX prefixes.\n");
#endif
        instruct.legacy_type = ILLEGAL;
        instruct.entry = gotit;
        return -1;
    }

    /* Addr points after the opcode, and the size has been adjusted accordingly */
    if(instruct.loc)
    {
        instruct.loc->opcode_size = instruct.size - pref.getCount();
        instruct.loc->opcode_position = pref.getCount();
    }

    /* make adjustments for instruction redefined in 64-bit mode */
    if(mode_64)
    {
        ia32_translate_for_64(&gotit);
    } 

    /* Set the condition bits if we need to */
    if(capa & IA32_DECODE_CONDITION)
    {
        int hack = gotit->opsema >> FPOS;
        if(hack == fCOND)
            instruct.cond->set(condbits);
    }

    if(gotit_ret)
        *gotit_ret = gotit;

    return 0;
}

ia32_instruction &
ia32_decode_FP(unsigned int opcode, const ia32_prefixes &pref, const unsigned char *addr, ia32_instruction &instruct,
               ia32_entry *entry, ia32_memacc *mac, bool mode_64)
{
    // addr points after the opcode, and the size has been adjusted accordingly
    if (instruct.loc) instruct.loc->opcode_size = instruct.size - pref.getCount();
    if (instruct.loc) instruct.loc->opcode_position = pref.getCount();

    unsigned int nib = byteSzB; // modRM
    unsigned int addrSzAttr = (pref.getPrefix(3) == PREFIX_SZADDR ? 1 : 2); // 32-bit mode implicit
    unsigned int operSzAttr = (pref.getPrefix(2) == PREFIX_SZOPER ? 1 : 2); // 32-bit mode implicit

    // There *will* be a mod r/m byte at least as far as locs are concerned, though the mod=3 case does not
    // consume extra bytes.  So pull this out of the conditional.
    if (instruct.loc) 
    {
        instruct.loc->modrm_position = instruct.loc->opcode_position +
            instruct.loc->opcode_size;
        instruct.loc->modrm_operand = 0;
    }
    nib += ia32_decode_modrm(addrSzAttr, addr, mac, &pref, instruct.loc, mode_64);
    // also need to check for AMD64 rip-relative data addressing
    // occurs when mod == 0 and r/m == 101
    if (mode_64)
    {
        if ((addr[0] & 0xc7) == 0x05)
        {
            instruct.rip_relative_data = true;
        }
    }

    if (addr[0] <= 0xBF) // modrm
    {
        // operand size has to be determined from opcode
        if(mac)
        {
            switch(opcode) {
                case 0xD8: // all single real
                    mac->size = 4;
                    mac->read = true;
                    break;
                case 0xD9: 
                    {
                        unsigned char modrm = addr[0];
                        unsigned char reg = (modrm >> 3) & 7;
                        switch(reg) 
                        {
                            case 0:
                                mac->size = 4;
                                mac->read = true;
                                break;
                            case 2:
                            case 3:
                                mac->size = 4;
                                mac->write = true;
                                break;
                            case 1:
                                instruct.legacy_type = ILLEGAL;
                                break;
                            case 4:
                                mac->size = 14 * operSzAttr;
                                mac->read = true;
                                break;
                            case 5:
                                mac->read = true;
                                mac->size = 2;
                                break;
                            case 6:
                                mac->size = 14 * operSzAttr;
                                mac->write = true;
                                break;
                            case 7:
                                mac->write = true;
                                mac->size = 2;
                                break;
                        }

                        break; 
                    }
                case 0xDA:  // all double real
                    mac->size = 8;
                    mac->read = true;
                    break;
                case 0xDB: 
                    {
                        unsigned char modrm = addr[0];
                        unsigned char reg = (modrm >> 3) & 7;
                        switch(reg) {
                            case 0:
                                mac->size = dwordSzB;
                                mac->read = true;
                                break;
                            case 2:
                            case 3:
                                mac->size = dwordSzB;
                                mac->write = true;
                                break;
                            case 1:
                            case 4:
                            case 6:
                                instruct.legacy_type = ILLEGAL;
                                break;
                            case 5:
                                mac->size = 10; // extended real
                                mac->read = true;
                                break;
                            case 7:
                                mac->size = 10; // extended real
                                mac->write = true;
                                break;
                        }
                        break; 
                    }
                case 0xDC:   // all double real
                    mac->size = 8;
                    mac->read = true;
                    break;
                case 0xDD: 
                    {
                        unsigned char modrm = addr[0];
                        unsigned char reg = (modrm >> 3) & 7;
                        switch(reg) 
                        {
                            case 0:
                                mac->size = 8;
                                mac->read = true;
                                break;
                            case 2:
                            case 3:
                                mac->size = 8;
                                mac->write = true;
                                break;
                            case 1:
                            case 5:
                                instruct.legacy_type = ILLEGAL;
                                break;
                            case 4:
                                mac->size = operSzAttr == 2 ? 108 : 98;
                                mac->read = true;
                                break;
                            case 6:
                                mac->size = operSzAttr == 2 ? 108 : 98;
                                mac->write = true;
                                break;
                            case 7:
                                mac->size = 2;
                                mac->write = true;
                                break;    
                        }
                        break; 
                    }
                case 0xDE: // all word integer
                    mac->size = wordSzB;
                    mac->write = true;
                    break;
                case 0xDF: 
                    {
                        unsigned char modrm = addr[0];
                        unsigned char reg = (modrm >> 3) & 7;
                        switch(reg) 
                        {
                            case 0:
                                mac->size = wordSzB;
                                mac->read = true;
                                break;
                            case 2:
                            case 3:
                                mac->size = wordSzB;
                                mac->write = true;
                                break;
                            case 1:
                                instruct.legacy_type = ILLEGAL;
                                break;
                            case 4:
                                mac->size = 10;
                                mac->read = true;
                                break;
                            case 5:
                                mac->size = 8;
                                mac->read = true;
                                break;
                            case 6:
                                mac->size = 10;
                                mac->write = true;
                                break;
                            case 7:
                                mac->size = 8;
                                mac->write = true; 
                                break;
                        }
                        break; 
                    }
                default: break;
            }
        }
    }

    instruct.size += nib;
    instruct.entry = entry;

    return instruct;
}


#define MODRM_MOD(x) (((x) >> 6) & 3)
#define MODRM_RM(x) ((x) & 7)
#define MODRM_REG(x) (((x) >> 3) & 7)
#define MODRM_SET_MOD(x, y) ((x) |= ((y) << 6))
#define MODRM_SET_RM(x, y) ((x) |= (y))
#define MODRM_SET_REG(x, y) ((x) |= ((y) << 3))

static unsigned int ia32_decode_modrm(const unsigned int addrSzAttr, const unsigned char *addr, ia32_memacc *macadr,
                                     const ia32_prefixes *pref, ia32_locations *loc, bool mode_64)
{
   /**
    * ModR/M Format:
    * 11  111  111
    *  |   |    +-> R/M bits
    *  |   +------> REG bits
    *  +----------> MOD bits
    */

   unsigned char modrm = addr[0];
   unsigned char mod = MODRM_MOD(modrm);
   unsigned char rm  = MODRM_RM(modrm);
   unsigned char reg = MODRM_REG(modrm);

   /* If locations are provided, configure them */
   if (loc)
   {
      loc->modrm_byte = modrm;
      loc->modrm_mod = mod;
      loc->modrm_rm = rm;
      loc->modrm_reg = reg;
      loc->address_size = addrSzAttr;
   }

   /* Move past the ModR/M byte */
   addr++;

   /* Get displacements we're going to use */
   auto dispAddr = addr;

   // addrSzAttr equals 1 means the address size override prefix is 
   // not present. On 64-bit platforms, this means the default address
   // size is 64-bit. On other platforms, this means 16-bit
   if(addrSzAttr == 1 && !mode_64)  // 16-bit, cannot have SIB
   {
      /* This mode can only occur when running in 32 bit mode. */
      switch(mod)
      {
         case 0:
            if(macadr)
            {
               switch (rm)
               {
                  case 0: // [BX+SI]
                     macadr->set16(mBX, mSI, 0);
                     break;
                  case 1:
                     macadr->set16(mBX, mDI, 0);
                     break;
                  case 2:
                     macadr->set16(mBP, mSI, 0);
                     break;
                  case 3:
                     macadr->set16(mBP, mDI, 0);
                     break;
                  case 4:
                     macadr->set16(mSI, -1, 0);
                     break;
                  case 5:
                     macadr->set16(mDI, -1, 0);
                     break;
                  case 6:
                     macadr->set16(-1, -1, Dyninst::read_memory_as<int16_t>(dispAddr));
                     if (loc)
                     {
                        loc->disp_position = loc->modrm_position + 1;
                        loc->disp_size = 2; /* 16 bit displacement */
                     }
                     break;
                  case 7:
                     macadr->set16(mBX, -1, 0);
                     break;
                  default:
                     assert(0);
                     return 0;
               }
            }

            return rm == 6 ? wordSzB : 0;
         case 1:
            if(macadr)
            {
               if (loc)
               {
                  loc->disp_position = loc->modrm_position + 1;
                  loc->disp_size = 1;
               }

               auto disp8 = read_memory_as<int8_t>(dispAddr);
               switch (rm)
               {
                  case 0:
                     macadr->set16(mBX, mSI, disp8);
                     break;
                  case 1:
                     macadr->set16(mBX, mDI, disp8);
                     break;
                  case 2:
                     macadr->set16(mBP, mSI, disp8);
                     break;
                  case 3:
                     macadr->set16(mBP, mDI, disp8);
                     break;
                  case 4:
                     macadr->set16(mSI, -1, disp8);
                     break;
                  case 5:
                     macadr->set16(mDI, -1, disp8);
                     break;
                  case 6:
                     macadr->set16(mBP, -1, disp8);
                     break;
                  case 7:
                     macadr->set16(mBX, -1, disp8);
                     break;
                  default:
                     assert(0);
                     return 0;
               }
            }

            return byteSzB;
         case 2:
            if(macadr)
            {
               if (loc)
               {
                  loc->disp_position = loc->modrm_position + 1;
                  loc->disp_size = 2;
               }

               auto disp16 = Dyninst::read_memory_as<int16_t>(dispAddr);
               switch (rm)
               {
                  case 0:
                     macadr->set16(mBX, mSI, disp16);
                     break;
                  case 1:
                     macadr->set16(mBX, mDI, disp16);
                     break;
                  case 2:
                     macadr->set16(mBP, mSI, disp16);
                     break;
                  case 3:
                     macadr->set16(mBP, mDI, disp16);
                     break;
                  case 4:
                     macadr->set16(mSI, -1, disp16);
                     break;
                  case 5:
                     macadr->set16(mDI, -1, disp16);
                     break;
                  case 6:
                     macadr->set16(mBP, -1, disp16);
                     break;
                  case 7:
                     macadr->set16(mBX, -1, disp16);
                     break;
                  default:
                     assert(0);
                     return 0;
               }
            }

            return wordSzB;
         case 3:
            /* This is a register */
            return 0;
         default:
            assert(0);
            return 0;
      }
   } else { // 32-bit or 64-bit, may have SIB

      /* When bits are 0b11, only a register can be provided */
      if(mod == 3)
         return 0;

      /* We have an additional sib if the RM bits are 0b100 */
      bool hassib = rm == 4;
      unsigned int nsib = 0;
      unsigned char sib;
      int base = 0, scale = -1, index = -1;  // prevent g++ from whining.
      /* Do we have an sib? */
      if(hassib)
      {
         /* Grab the sib */
         sib = addr[0];
         nsib = byteSzB;

         /* Update locations if needed */
         if (loc)
         {
            loc->sib_position = loc->modrm_position + 1;
            loc->sib_byte = sib;
         }

         /* We have consumed the sib bite */
         addr++;

         base = sib & 7;
         if(macadr)
         {
            scale = sib >> 6;
            index = (sib >> 3) & 7;

            // stack pointer can't be used as an index - supports base-only addressing w/ SIB
            if(index == 4 && !pref->rexX())
               index = -1;
         }
      }

      /* Update displacement pointers  */
      dispAddr = addr;

      /* this is tricky: there is a disp32 iff (1) rm == 5  or  (2) hassib && base == 5 */
      unsigned char check5 = hassib ? base : rm;

      switch(mod)
      {
         case 0:
            if(macadr)
            {
               switch (rm)
               {
                  case 0:
                     macadr->set(apply_rex_bit(mEAX, pref->rexB()), 0, addrSzAttr);
                     break;
                  case 1:
                     macadr->set(apply_rex_bit(mECX, pref->rexB()), 0, addrSzAttr);
                     break;
                  case 2:
                     macadr->set(apply_rex_bit(mEDX, pref->rexB()), 0, addrSzAttr);
                     break;
                  case 3:
                     macadr->set(apply_rex_bit(mEBX, pref->rexB()), 0, addrSzAttr);
                     break;
                  case 4: // SIB
                     if(base == 5)
                     {
                        // disp32[index<<scale]
                        if (loc)
                        {
                           loc->disp_position = loc->sib_position + 1;
                           loc->disp_size = 4;
                        }

                        auto disp32 = Dyninst::read_memory_as<int32_t>(dispAddr);
                        macadr->set_sib(-1, scale, apply_rex_bit(index, pref->rexX()),
                              disp32, addrSzAttr);
                     } else {
                        macadr->set_sib(apply_rex_bit(base, pref->rexB()), scale,
                              apply_rex_bit(index, pref->rexX()), 0, addrSzAttr);
                     }
                     break;
                  case 5:
                     {
                        // disp32 (or [RIP + disp32] for 64-bit mode)
                        if (loc)
                        {
                           loc->disp_position = loc->modrm_position + 1;
                           loc->disp_size = 4;
                        }

                        auto disp32 = Dyninst::read_memory_as<int32_t>(dispAddr);
                        if (mode_64)
                           macadr->set(mRIP, disp32, addrSzAttr);
                        else
                           macadr->set(-1, disp32, addrSzAttr);
                        break;
                     }
                  case 6:
                     macadr->set(apply_rex_bit(mESI, pref->rexB()), 0, addrSzAttr);
                     break;
                  case 7:
                     macadr->set(apply_rex_bit(mEDI, pref->rexB()), 0, addrSzAttr);
                     break;
                  default:
                     assert(0);
                     return 0;
               }
            }

            return nsib + ((check5 == 5) ? dwordSzB : 0);
         case 1:
            if(macadr)
            {
               if (loc)
               {
                  loc->disp_position = loc->modrm_position + 1;
                  loc->disp_size = 1;
               }

               auto disp8 = Dyninst::read_memory_as<int8_t>(dispAddr);
               switch (rm)
               {
                  case 0:
                     macadr->set(apply_rex_bit(mEAX, pref->rexB()), disp8, addrSzAttr);
                     break;
                  case 1:
                     macadr->set(apply_rex_bit(mECX, pref->rexB()), disp8, addrSzAttr);
                     break;
                  case 2:
                     macadr->set(apply_rex_bit(mEDX, pref->rexB()), disp8, addrSzAttr);
                     break;
                  case 3:
                     macadr->set(apply_rex_bit(mEBX, pref->rexB()), disp8, addrSzAttr);
                     break;
                  case 4:
                     // disp8[EBP + index<<scale] happens naturally here when base=5
                     if (loc) {
                        loc->disp_position = loc->sib_position + 1;
                        loc->disp_size = 1;
                     }
                     macadr->set_sib(apply_rex_bit(base, pref->rexB()), scale, apply_rex_bit(index, pref->rexX()),
                           disp8, addrSzAttr);
                     break;
                  case 5:
                     macadr->set(apply_rex_bit(mEBP, pref->rexB()), disp8, addrSzAttr);
                     break;
                  case 6:
                     macadr->set(apply_rex_bit(mESI, pref->rexB()), disp8, addrSzAttr);
                     break;
                  case 7:
                     macadr->set(apply_rex_bit(mEDI, pref->rexB()), disp8, addrSzAttr);
                     break;
               }
            }

            return nsib + byteSzB;
         case 2:
            if(macadr)
            {
               if (loc)
               {
                  loc->disp_position = loc->modrm_position + 1;
                  loc->disp_size = 4;
               }

               auto disp32 = Dyninst::read_memory_as<int32_t>(dispAddr);
               switch (rm)
               {
                  case 0:
                     macadr->set(apply_rex_bit(mEAX, pref->rexB()), disp32, addrSzAttr);
                     break;
                  case 1:
                     macadr->set(apply_rex_bit(mECX, pref->rexB()), disp32, addrSzAttr);
                     break;
                  case 2:
                     macadr->set(apply_rex_bit(mEDX, pref->rexB()), disp32, addrSzAttr);
                     break;
                  case 3:
                     macadr->set(apply_rex_bit(mEBX, pref->rexB()), disp32, addrSzAttr);
                     break;
                  case 4:
                     // disp32[EBP + index<<scale] happens naturally here when base=5
                     if (loc) {
                        loc->disp_position = loc->sib_position + 1;
                        loc->disp_size = 4;
                     }
                     macadr->set_sib(apply_rex_bit(base, pref->rexB()), scale, apply_rex_bit(index, pref->rexX()),
                           disp32, addrSzAttr);
                     break;
                  case 5:
                     macadr->set(apply_rex_bit(mEBP, pref->rexB()), disp32, addrSzAttr);
                     break;
                  case 6:
                     macadr->set(apply_rex_bit(mESI, pref->rexB()), disp32, addrSzAttr);
                     break;
                  case 7:
                     macadr->set(apply_rex_bit(mEDI, pref->rexB()), disp32, addrSzAttr);
                     break;
                  default:
                     assert(0);
                     return 0;
               }
            }

            return nsib + dwordSzB;
         default:
            assert(0);
            return 0;
      }

      /* We shouldn't get out of the above switch */
      assert(0);
   }

   return 0; // MS compiler from VS 6.0 wants this
}


static inline int type2size(unsigned int optype, unsigned int operSzAttr)
{
   switch(optype) {
      case op_a:
         return 2 * wordSzB * operSzAttr;
      case op_b:
         return byteSzB;
      case op_c:
         assert(!"Where is this used, Intel?");
         return byteSzB * operSzAttr;
      case op_d:
         return dwordSzB;
      case op_dq:
         return dqwordSzB;
      case op_p:
         return wordSzB + wordSzB * operSzAttr; // XXX book says operand size...
      case op_pd:  // Intel "forgot" to define this in book, but uses it
         return dqwordSzB;
      case op_pi:
         return qwordSzB;
      case op_ps:
         return dqwordSzB;
      case op_q:
         return qwordSzB;
      case op_s:
         return 6;
      case op_sd:  // another Intel amnesia case
         return qwordSzB;
      case op_ss:
         return dwordSzB;
      case op_si:
         assert(!"Where is this used, Intel?");
         return dwordSzB;
      case op_v:
         return wordSzB * operSzAttr;
      case op_w:
         return wordSzB;
      case op_y:
         return operSzAttr == 4 ? qwordSzB : dwordSzB;
      case op_z:
         return operSzAttr == 1 ? wordSzB : dwordSzB;
      case op_lea:
         //    assert(!"Should not be evaluated");
         // We might be called, if we don't know this is an lea ahead of time
         // It's okay to return 0 here, because we really don't load/store
         return 0;
      case op_allgprs:
         return 8 * wordSzB * operSzAttr;
      case op_512:
         return 512;
      default:
         assert(0);
         return 0;
         //    RegisterAST reg(optype);
         //    return reg.eval().size();
   }
}

unsigned int ia32_decode_operands (const ia32_prefixes& pref, 
                                   const ia32_entry& gotit, 
                                   const unsigned char* addr, 
                                   ia32_instruction& instruct,
                                   ia32_memacc *mac, bool mode_64)
{
   /* # of bytes in instruction */
   unsigned int nib = 0;
   ia32_locations *loc = instruct.loc;

   if(loc)
      loc->imm_cnt = 0;

   /* If the prefix is present, the address size changes to 16 bit */
   int addrSzAttr = (pref.getPrefix(3) == PREFIX_SZADDR ? 1 : 2);

   /* If the prefix is present in 64 bit mode, the address changes to 32 bit */
   if(mode_64)
      addrSzAttr *= 2;

   int operSzAttr = getOperSz(pref);

   if(gotit.hasModRM)
      nib += byteSzB;

   for(int i = 0; i < 3; i++)
   {
      const ia32_operand& op = gotit.operands[i];

      if(op.admet)
      {
         // At most two operands can be memory, the third is register or immediate
         //assert(i<2 || op.admet == am_reg || op.admet == am_I);
         switch(op.admet)
         {
            case am_A: /* address = segment + offset (word or dword or qword) */
               nib += wordSzB; // segment
               nib += wordSzB * addrSzAttr;  // + offset (1 or 2 words, depending on prefix)
               break;
            case am_O: /* operand offset */
               nib += wordSzB * addrSzAttr;
               if(mac)
               {
                  long offset = 0;
                  switch(addrSzAttr)
                  {
                     case 1: // 16-bit offset
                        offset = Dyninst::read_memory_as<int16_t>(addr);
                        break;
                     case 2: // 32-bit offset
                        offset = Dyninst::read_memory_as<int32_t>(addr);
                        break;
                     case 4: // 64-bit
                        offset = Dyninst::read_memory_as<int64_t>(addr);
                        break;
                     default:
                        assert(0);
                        break;
                  }

                  mac[i].set(-1, offset, addrSzAttr);
                  mac[i].size = type2size(op.optype, operSzAttr);
               }
               break;
            case am_tworeghack:
            case am_ImplImm:
            case am_B:   /* General register selected by VEX.vvvv*/
            case am_C:   /* control register */
            case am_D:   /* debug register */
            case am_F:   /* flags register */
            case am_G:   /* general purpose register, selecteb by reg field */
            case am_P:   /* MMX register */
            case am_R:   /* general purpose register, selected by r/m field */
            case am_S:   /* segment register */
            case am_T:   /* test register */
            case am_XV:  /* XMM register (From reg of ModR/M) */
            case am_XU:  /* XMM register (from R/M of ModR/M) */
            case am_XH:  /* XMM register (vvvv of prefix) */
            case am_YV:  /* XMM or YMM register (From reg of ModR/M) */
            case am_YU:  /* XMM or YMM register (from R/M of ModR/M) */
            case am_YH:  /* XMM or YMM register (vvvv of prefix) */
            case am_V:   /* XMM, YMM or ZMM register (From reg of ModR/M) */
            case am_U:   /* XMM, YMM or ZMM register (from R/M of ModR/M) */
            case am_H:   /* XMM, YMM or ZMM register (vvvv of prefix) */
            case am_HK:  /* K register (vvvv of prefix) */
            case am_VK:  /* K register (vvvv of prefix) */
            case am_reg: /* register implicitely encoded in opcode */
            case am_allgprs:
               break;
            case am_E:  /* register or memory location, so decoding needed */
            case am_M:  /* memory operand, decoding needed; size includes modRM byte */
            case am_Q:  /* MMX register or memory location */
            case am_RM: /* register or memory location, so decoding needed */
            case am_UM: /* XMM register or memory location */
            case am_XW: /* XMM register or memory location */
            case am_YW: /* XMM or YMM register or memory location */
            case am_W:  /* XMM, YMM or ZMM register or memory location */
            case am_WK:  /* K register (vvvv of prefix) */
               if (loc)
               {
                  loc->modrm_position = loc->opcode_size + loc->opcode_position;
                  loc->modrm_operand = i;
               }

               if(mac)
               {
                  nib += ia32_decode_modrm(addrSzAttr, addr, &mac[i], &pref, loc, mode_64);
                  mac[i].size = type2size(op.optype, operSzAttr);

                  if (loc)
                     loc->address_size = mac[i].size;
               } else {
                  nib += ia32_decode_modrm(addrSzAttr, addr, NULL, &pref, loc, mode_64);
               }

               // also need to check for AMD64 rip-relative data addressing
               // occurs when mod == 0 and r/m == 101
               if (mode_64 && (addr[0] & 0xc7) == 0x05)
                  instruct.rip_relative_data = true;

               break;
            case am_I: /* immediate data */
            case am_J: /* instruction pointer offset */
            case am_L:
               {
                  int imm_size = type2size(op.optype, operSzAttr);
                  if (loc)
                  {
                     // sanity
                     if(loc->imm_cnt > 1)
                     {
                        fprintf(stderr,"Oops, more than two immediate operands\n");
                     } else {
                        loc->imm_position[loc->imm_cnt] =
                           nib + loc->opcode_position + loc->opcode_size;
                        loc->imm_size[loc->imm_cnt] = imm_size;
                        ++loc->imm_cnt;
                     }
                  }
                  nib += imm_size;
                  break;
               }
               /* TODO: rep prefixes, deal with them here? */

            case am_X: /* memory at DS:(E)SI*/
               if(mac)
                  mac[i].setXY(mESI, type2size(op.optype, operSzAttr), addrSzAttr);
               break;
            case am_Y: /* memory at ES:(E)DI*/
               if(mac)
                  mac[i].setXY(mEDI, type2size(op.optype, operSzAttr), addrSzAttr);
               break;
            case am_stackH: /* stack push */
            case am_stackP: /* stack pop */
               assert(0 && "Wrong table!");
               break;
            default:
#ifdef VEX_DEBUG
               printf("mode: %d  %x\n", op.admet, op.admet);
#endif
               assert(0 && "Bad addressing mode!");
         }
      } else {
         break;
      }
   }

   /* Are there 4 operands? */
   if((gotit.opsema &  0xffff) >= s4OP)
   {
      /* This last one is always Ib */
      int imm_size = type2size(op_b, operSzAttr);

      if (loc)
      {
         if(loc->imm_cnt > 1)
         {
            fprintf(stderr,"Oops, more than two immediate operands\n");
         } else {
            loc->imm_position[loc->imm_cnt] = nib + loc->opcode_position + loc->opcode_size;
            loc->imm_size[loc->imm_cnt] = imm_size;
            ++loc->imm_cnt;
         }
      }

      nib += imm_size;
   }

   if((gotit.id == e_push) && mac)
   {
      // assuming 32-bit (64-bit for AMD64) stack segment
      // AMD64: push defaults to 64-bit operand size
      if (mode_64 && operSzAttr == 2)
         operSzAttr = 4;

      mac[1].set(mESP, -2 * operSzAttr, addrSzAttr);

      if(gotit.operands[0].admet == am_reg)
      {
         mac[1].size = type2size(op_v, operSzAttr);
      } else {
         mac[1].size = type2size(gotit.operands[0].optype, operSzAttr);
      }

      mac[1].write = true;
   }

   if((gotit.id == e_pop) && mac)
   {
      // assuming 32-bit (64-bit for AMD64) stack segment
      // AMD64: pop defaults to 64-bit operand size
      if (mode_64 && operSzAttr == 2)
         operSzAttr = 4;
      mac[1].set(mESP, 0, addrSzAttr);
      if(gotit.operands[0].admet == am_reg)
      {
         mac[1].size = type2size(op_v, operSzAttr);
      } else {
         mac[1].size = type2size(gotit.operands[0].optype, operSzAttr);
      }
      mac[1].read = true;
   }

   if((gotit.id == e_leave) && mac)
   {
      // assuming 32-bit (64-bit for AMD64) stack segment
      // AMD64: push defaults to 64-bit operand size
      if (mode_64 && operSzAttr == 2)
         operSzAttr = 4;
      mac[0].set(mESP, 0, addrSzAttr);
      mac[0].size = type2size(op_v, operSzAttr);
      mac[0].read = true;
   }

   if((gotit.id == e_ret_near || gotit.id == e_ret_far) && mac)
   {
      mac[0].set(mESP, 0, addrSzAttr);
      mac[0].size = type2size(op_v, addrSzAttr);
      mac[0].read = true;
   }

   if((gotit.id == e_call) && mac)
   {
      int index = 0;
      while((mac[index].regs[0] != -1) ||
            (mac[index].regs[1] != -1) ||
            (mac[index].scale != 0) ||
            (mac[index].imm != 0))
      {
         index++;
         assert(index < 3);
      }

      mac[index].set(mESP, -2 * addrSzAttr, addrSzAttr);
      mac[index].size = type2size(op_v, addrSzAttr);
      mac[index].write = true;
   }

   instruct.size += nib;
   return nib;
}


static const unsigned char sse_prefix[256] = {
   /*       0 1 2 3 4 5 6 7 8 9 A B C D E F  */
   /* 0x */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   /* 1x */ 1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,
   /* 2x */ 0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,
   /* 3x */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   /* 4x */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   /* 5x */ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
   /* 6x */ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
   /* 7x */ 1,1,1,1,1,1,1,0,1,1,0,0,1,1,1,1, // Grp12-14 are SSE groups
   /* 8x */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   /* 9x */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   /* Ax */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   /* Bx */ 0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,
   /* Cx */ 0,0,1,0,1,1,1,0,0,0,0,0,0,0,0,0,
   /* Dx */ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
   /* Ex */ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
   /* Fx */ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
};

static const unsigned char sse_prefix_bis[256] = {
   /*       0 1 2 3 4 5 6 7 8 9 A B C D E F  */
   /* 0x */ 1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,
   /* 1x */ 1,0,0,0,1,1,0,1,0,0,0,0,1,1,1,0,
   /* 2x */ 1,1,1,1,1,1,0,0,1,1,1,1,0,0,0,0,
   /* 3x */ 1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,
   /* 4x */ 1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   /* 5x */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   /* 6x */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   /* 7x */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   /* 8x */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   /* 9x */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   /* Ax */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   /* Bx */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   /* Cx */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   /* Dx */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   /* Ex */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   /* Fx */ 1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

static const unsigned char sse_prefix_ter[256] = {
   /*       0 1 2 3 4 5 6 7 8 9 A B C D E F  */
   /* 0x */ 0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,
   /* 1x */ 0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,
   /* 2x */ 1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,
   /* 3x */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   /* 4x */ 1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,
   /* 5x */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   /* 6x */ 1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,
   /* 7x */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   /* 8x */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   /* 9x */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   /* Ax */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   /* Bx */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   /* Cx */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   /* Dx */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   /* Ex */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   /* Fx */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

#define REX_ISREX(x) (((x) >> 4) == 4)

bool is_sse_opcode(unsigned char byte1, unsigned char byte2, unsigned char byte3) {
   if((byte1==0x0F && sse_prefix[byte2])
         || (byte1==0x0F && byte2==0x38 && sse_prefix_bis[byte3])
         || (byte1==0x0F && byte2==0x3A && sse_prefix_ter[byte3]))
      return true;

   return false;
}


/**
 * Decode's an instruction's prefixes. If there are no prefixes for this instruction,
 * then nothing is done and false is returned.
 *
 * @param addr A pointer to the start of the instruction.
 * @param instruct A reference to the instruction that we should setup the prefix information for.
 *
 * @return true if the prefixes were decoded successfully, false if there was a problem.
 */
    bool ia32_decode_prefixes(const unsigned char *addr, ia32_instruction &instruct, bool mode_64)
{
   ia32_prefixes& pref = instruct.prf;
   ia32_locations* loc = instruct.loc;

   /* Initilize the prefix */
   memset(pref.prfx, 0, 5);
   pref.count = 0;
   pref.opcode_prefix = 0;
   bool in_prefix = true;

   /* Clear all of the VEX information */
   pref.vex_present = false;
   pref.vex_type = VEX_TYPE_NONE;
   memset(pref.vex_prefix, 0, 5);
   pref.vex_sse_mult = -1;
   pref.vex_vvvv_reg = -1;
   pref.vex_ll = -1;
   pref.vex_pp = -1;
   pref.vex_m_mmmm = -1;
   pref.vex_w = -1;
   pref.vex_V = 0;
   pref.vex_r = 0;
   pref.vex_R = 0;
   pref.vex_x = 0;
   pref.vex_b = 0;
   pref.vex_aaa = 0;

   pref.XOP = false;
   bool err = false;

   while(in_prefix && !err)
   {
      /**
       * Switch based on the current byte. If the current byte
       * is a valid prefix, we will consume the byte and keep
       * trying to decode more prefixes. All of the prefix
       * constants for this switch are defined in arch-x86.h.
       *
       * NOTE: Some prefixes need to be in a certain order and
       * some prefixes cannot go together.
       */

      switch(addr[0])
      {
         case PREFIX_REPNZ:
         case PREFIX_REP:
            if(mode_64 && REX_ISREX(addr[1])
                  && is_sse_opcode(addr[2],addr[3],addr[4]))
            {
               ++pref.count;
               pref.opcode_prefix = addr[0];
            } else if(is_sse_opcode(addr[1],addr[2],addr[3]))
            {
               ++pref.count;
               pref.opcode_prefix = addr[0];
            } else {
               ++pref.count;
               pref.prfx[0] = addr[0];
            }
            break;

         case PREFIX_LOCK:
            ++pref.count;
            pref.prfx[0] = addr[0];
            break;

         case PREFIX_SEGCS:
         case PREFIX_SEGSS:
         case PREFIX_SEGDS:
         case PREFIX_SEGES:
         case PREFIX_SEGFS:
         case PREFIX_SEGGS:
            ++pref.count;
            pref.prfx[1] = addr[0];
            break;

         case PREFIX_SZOPER:
            pref.opcode_prefix = addr[0];
            ++pref.count;
            pref.prfx[2] = addr[0];
            break;

         case PREFIX_SZADDR:
            ++pref.count;
            pref.prfx[3] = addr[0];
            break;

         case PREFIX_EVEX:
            pref.vex_present = true;
            pref.vex_type = VEX_TYPE_EVEX;
            memmove(&pref.vex_prefix, addr + 1, 3);
            pref.vex_sse_mult = 2;
            pref.vex_vvvv_reg = EVEXGET_VVVV(pref.vex_prefix[1], pref.vex_prefix[2]);
            pref.vex_ll = EVEXGET_LL(pref.vex_prefix[2]);
            pref.vex_pp = EVEXGET_PP(pref.vex_prefix[1]);
            pref.vex_m_mmmm = EVEXGET_MM(pref.vex_prefix[0]);
            pref.vex_w = EVEXGET_W(pref.vex_prefix[1]);
            pref.vex_V = EVEXGET_V(pref.vex_prefix[2]);
            pref.vex_r = EVEXGET_r(pref.vex_prefix[0]);
            pref.vex_R = EVEXGET_R(pref.vex_prefix[0]);
            pref.vex_x = EVEXGET_x(pref.vex_prefix[0]);
            pref.vex_b = EVEXGET_b(pref.vex_prefix[0]);
            pref.vex_aaa = EVEXGET_AAA(pref.vex_prefix[2]);
            pref.count += 4;

            /* VEX_LL must be 0, 1, or 2 */
            if(pref.vex_ll >= 3 || pref.vex_ll < 0)
            {
               err = true;
               break;
            }

            switch(pref.vex_pp)
            {
               case 0:
                  pref.opcode_prefix = 0x00;
                  break;
               case 1:
                  pref.opcode_prefix = 0x66;
                  break;
               case 2:
                  pref.opcode_prefix = 0xF3;
                  break;
               case 3:
                  pref.opcode_prefix = 0xF2;
                  break;
               default:
                  assert(!"Can't happen: value & 0x03 not in 0...3");
            }

            /* There are a couple of constant bits for this prefix */
            if(((pref.vex_prefix[0] & (unsigned int)(0x03 << 2)) != 0)
                  || ((pref.vex_prefix[1] & (unsigned int)(1 << 2)) == 0))
            {
#ifdef VEX_DEBUG
               printf("EVEX PREFIX INVALID!\n");
#endif
               err = true;
               break;
            }

            /* VEX prefix excludes all others */
            in_prefix = false;
            break;

         case PREFIX_XOP:
         // XOP prefix has the same structure as VEX3
            pref.XOP = true;
	    DYNINST_FALLTHROUGH;
         case PREFIX_VEX3:
            pref.vex_present = true;
            pref.vex_type = VEX_TYPE_VEX3;
            memmove(&pref.vex_prefix, addr + 1, 2);
            pref.vex_sse_mult = 1;
            pref.vex_vvvv_reg = VEXGET_VVVV(pref.vex_prefix[1]);
            pref.vex_ll = VEXGET_L(pref.vex_prefix[1]);
            pref.vex_pp = VEXGET_PP(pref.vex_prefix[1]);
            pref.vex_m_mmmm = VEX3GET_M(pref.vex_prefix[0]);
            pref.vex_w = VEX3GET_W(pref.vex_prefix[1]);
            pref.vex_V = 0;
            pref.vex_r = VEXGET_R(pref.vex_prefix[0]);
            pref.vex_x = VEX3GET_X(pref.vex_prefix[0]);
            pref.vex_b = VEX3GET_B(pref.vex_prefix[0]);
            pref.count += 3;

            switch(pref.vex_pp)
            {
               case 0:
                  pref.opcode_prefix = 0x00;
                  break;
               case 1:
                  pref.opcode_prefix = 0x66;
                  break;
               case 2:
                  pref.opcode_prefix = 0xF3;
                  break;
               case 3:
                  pref.opcode_prefix = 0xF2;
                  break;
               default:
                  assert(!"Can't happen: value & 0x03 not in 0...3");
            }

            /* VEX prefix excludes all others */
            in_prefix = false;
            break;
         case PREFIX_VEX2:
            pref.vex_present = true;
            pref.vex_type = VEX_TYPE_VEX2;
            pref.vex_prefix[0] = addr[1]; /* Only 1 byte for VEX2 */
            pref.vex_sse_mult = 0;
            pref.vex_vvvv_reg = VEXGET_VVVV(pref.vex_prefix[0]);

            pref.vex_ll = VEXGET_L(pref.vex_prefix[0]);
            pref.vex_pp = VEXGET_PP(pref.vex_prefix[0]);
            pref.vex_m_mmmm = -1; /* No W bit for VEX2 */
            pref.vex_w = -1; /* No W bit for VEX2 */
            pref.vex_r = VEXGET_R(pref.vex_prefix[0]);
            pref.count += 2;

            switch(pref.vex_pp)
            {
               case 0:
                  pref.opcode_prefix = 0x00;
                  break;
               case 1:
                  pref.opcode_prefix = 0x66;
                  break;
               case 2:
                  pref.opcode_prefix = 0xF3;
                  break;
               case 3:
                  pref.opcode_prefix = 0xF2;
                  break;
               default:
                  assert(!"Can't happen: value & 0x03 not in 0...3");
            }

            /* VEX prefixes exclude all others */
            in_prefix = false;
            break;
         default:
            // If we hit a REX prefix, keep going and process other potential prefixes.
            // The only one *used* is one in the last position, but others are ignored,
            // not illegal.

            if(mode_64)
            {
               if(ia32_decode_rex(addr, pref, loc))
               {
                  in_prefix = false;
                  break;
               }

               if(!REX_ISREX(addr[0]))
               {
                  /* We probably hit the opcode now */
                  in_prefix = false;
               } else  {
                  /* We hit a REX prefix, but we are skipping it. */
                  ++pref.count;
               }
            } else {
               in_prefix = false;
            }
            break;
      }

      /* Move to the next byte */
      ++addr;
   }

   /* If there was no error, set prefix count and correct instruction length */
   if(!err)
   {
      if(loc)
         loc->num_prefixes = pref.count;
      instruct.size = pref.count;
   }

   /* Print debug information that is super helpful for VEX debugging. */

#ifdef VEX_DEBUG /* Print out prefix information (very verbose) */
   fprintf(stderr, "Prefix buffer: %x %x %x %x %x\n", pref.prfx[0], pref.prfx[1],
         pref.prfx[2], pref.prfx[3], pref.prfx[4]);
   fprintf(stderr, "opcode prefix: 0x%x\n", pref.opcode_prefix);
   fprintf(stderr, "REX  W: %s  R: %s  X: %s  B: %s\n",
         pref.rexW() ? "yes" : "no", pref.rexR() ? "yes" : "no",
         pref.rexX() ? "yes" : "no", pref.rexB() ? "yes" : "no");

   fprintf(stderr, "IS VEX PRESENT?  %s\n", pref.vex_present ? "YES" : "NO");
   if(pref.vex_present)
   {
      fprintf(stderr, "VEX IS PRESENT: %d\n",
            pref.vex_type);
      fprintf(stderr, "VEX BYTES:      %x %x %x %x %x\n",
            pref.vex_prefix[0], pref.vex_prefix[1], pref.vex_prefix[2],
            pref.vex_prefix[3], pref.vex_prefix[4]);
      fprintf(stderr, "VEX SSE MULT:   %d  0x%x\n",
            pref.vex_sse_mult, pref.vex_sse_mult);
      fprintf(stderr, "VEX_VVVV:       %d  0x%x\n",
            pref.vex_vvvv_reg, pref.vex_vvvv_reg);
      fprintf(stderr, "VEX_LL:         %d  0x%x\n",
            pref.vex_ll, pref.vex_ll);
      fprintf(stderr, "VEX_PP:         %d  0x%x\n",
            pref.vex_pp, pref.vex_pp);
      fprintf(stderr, "VEX_M-MMMM:     %d  0x%x\n",
            pref.vex_m_mmmm, pref.vex_m_mmmm);
      fprintf(stderr, "VEX_W:          %d  0x%x\n",
            pref.vex_w, pref.vex_w);
      fprintf(stderr, "VEX_r:          %d  0x%x\n",
            pref.vex_r, pref.vex_r);
      fprintf(stderr, "VEX_R:          %d  0x%x\n",
            pref.vex_R, pref.vex_R);
      fprintf(stderr, "VEX_x:          %d  0x%x\n",
            pref.vex_x, pref.vex_x);
      fprintf(stderr, "VEX_b:          %d  0x%x\n",
            pref.vex_b, pref.vex_b);
   }
#endif

   return !err;
}

#define REX_W(x) ((x) & 0x8)
#define REX_R(x) ((x) & 0x4)
#define REX_X(x) ((x) & 0x2)
#define REX_B(x) ((x) & 0x1)

#define REX_INIT(x) ((x) = 0x40)
#define REX_SET_W(x, v) ((x) |= ((v) ? 0x8 : 0))
#define REX_SET_R(x, v) ((x) |= ((v) ? 0x4 : 0))
#define REX_SET_X(x, v) ((x) |= ((v) ? 0x2 : 0))
#define REX_SET_B(x, v) ((x) |= ((v) ? 0x1 : 0))

bool ia32_decode_rex(const unsigned char* addr, ia32_prefixes& pref,
      ia32_locations *loc)
{
   if (REX_ISREX(addr[0]))
   {
      /**
       * it is an error to have legacy prefixes after a REX prefix
       * in particular, ia32_decode will get confused if a prefix
       * that could be used as an SSE opcode extension follows our
       * REX
       */

      switch(addr[1])
      {
         case PREFIX_SZOPER:
         case PREFIX_REPNZ:
         case PREFIX_REP:
         case PREFIX_SEGCS:
         case PREFIX_SEGSS:
         case PREFIX_SEGDS:
         case PREFIX_SEGES:
         case PREFIX_SEGFS:
         case PREFIX_SEGGS:
         case PREFIX_LOCK:
         case PREFIX_SZADDR:
            return false;
      }

      /* We also must ignore all but the last REX prefix. */
      if(REX_ISREX(addr[1]))
         return false;

      /* Set the locations for this REX prefix. */
      if (loc)
      {
         loc->rex_byte = addr[0];
         loc->rex_w = REX_W(addr[0]);
         loc->rex_r = REX_R(addr[0]);
         loc->rex_x = REX_X(addr[0]);
         loc->rex_b = REX_B(addr[0]);
         loc->rex_position = pref.count - 1;
      }

      /**
       * VEX Prefixes are valid after the REX prefix because VEX
       * prefixes must be the last prefix before the opcode.
       */
      switch(addr[1])
      {
         case PREFIX_VEX2:
         case PREFIX_VEX3:
         case PREFIX_EVEX:
            return false;
         default: break;
      }

      /* We should be done parsing all prefixes if we get here. */
      ++pref.count;
      pref.prfx[4] = addr[0];
   }

   return true;
}

unsigned int ia32_emulate_old_type(ia32_instruction &instruct, bool mode_64)
{
   const ia32_prefixes& pref = instruct.prf;
   unsigned int& insnType = instruct.legacy_type;

   unsigned int operSzAttr = getOperSz(pref);

   if (pref.getPrefix(0)) // no distinction between these
      insnType |= PREFIX_INST;
   if (pref.getPrefix(3) == PREFIX_SZADDR)
      insnType |= PREFIX_ADDR;
   if (pref.getPrefix(2) == PREFIX_SZOPER)
      insnType |= PREFIX_OPR;
   if (pref.getPrefix(1))
      insnType |= PREFIX_SEG; // no distinction between segments
   if (mode_64 && pref.getPrefix(4))
      insnType |= PREFIX_REX;
   if (pref.getOpcodePrefix())
      insnType |= PREFIX_OPCODE;

   if(pref.vex_present)
   {
      switch(pref.vex_type)
      {
         case VEX_TYPE_VEX2:
            insnType |= PREFIX_AVX;
            break;
         case VEX_TYPE_VEX3:
            insnType |= PREFIX_AVX2;
            break;
         case VEX_TYPE_EVEX:
            insnType |= PREFIX_AVX512;
            break;
         default:
            assert(!"VEX prefixed instruction with no VEX prefix!");
      }
   }

   // this still works for AMD64, since there is no "REL_Q"
   // actually, it will break if there is both a REX.W and operand
   // size prefix, since REX.W takes precedence (FIXME)
   if (insnType & REL_X) {
      if (operSzAttr == 1)
         insnType |= REL_W;
      else
         insnType |= REL_D;
   }
   else if (insnType & PTR_WX) {
      if (operSzAttr == 1)
         insnType |= PTR_WW;
      else
         insnType |= PTR_WD;
   }

   // AMD64 rip-relative data
   if (instruct.hasRipRelativeData())
      insnType |= REL_D_DATA;

   return insnType;
}

/* decode instruction at address addr, return size of instruction */
unsigned get_instruction(const unsigned char* addr, unsigned &insnType,
			 const unsigned char** op_ptr, bool mode_64)
{
   int r1;

  ia32_instruction i;
  ia32_decode(0, addr, i, mode_64);

   r1 = i.getSize();
   insnType = ia32_emulate_old_type(i, mode_64);
   if (op_ptr)
      *op_ptr = addr + i.getPrefixCount();

   return r1;
}

// find the target of a jump or call
Address get_target(const unsigned char *instr, unsigned type, unsigned size,
      Address addr) {
   int disp = displacement(instr, type);
   return (Address)(addr + size + disp);
}

// get the displacement of a jump or call
int displacement(const unsigned char *instr, unsigned type) {

   int disp = 0;
   //skip prefix
   instr = skip_headers( instr );

   if (type & REL_D_DATA) {
      // Some SIMD instructions have a mandatory 0xf2 prefix; the call to skip_headers
      // doesn't skip it and I don't feel confident in changing that - bernat, 22MAY06
      // 0xf3 as well...
      // Some 3-byte opcodes start with 0x66... skip
      if (*instr == 0x66) instr++;
      if (*instr == 0xf2) instr++;
      else if (*instr == 0xf3) instr++;
      // And the "0x0f is a 2-byte opcode" skip
      if (*instr == 0x0F) {
         instr++;
         // And the "0x38 or 0x3A is a 3-byte opcode" skip
         if(*instr == 0x38 || *instr == 0x3A)  instr++;
      }
      // Skip the instr opcode and the MOD/RM byte
      disp = Dyninst::read_memory_as<int32_t>(instr+2);
   } else if (type & IS_JUMP) {
      if (type & REL_B) {
         disp = Dyninst::read_memory_as<int8_t>(instr+1);
      } else if (type & REL_W) {
         disp = Dyninst::read_memory_as<int16_t>(instr+1); // skip opcode
      } else if (type & REL_D) {
         disp = Dyninst::read_memory_as<int32_t>(instr+1);
      }
   } else if (type & IS_JCC) {
      if (type & REL_B) {
         disp = Dyninst::read_memory_as<int8_t>(instr+1);
      } else if (type & REL_W) {
         disp = Dyninst::read_memory_as<int16_t>(instr+2); // skip two byte opcode
      } else if (type & REL_D) {
         disp = Dyninst::read_memory_as<int32_t>(instr+2);   // skip two byte opcode
      }
   } else if (type & IS_CALL) {
      if (type & REL_W) {
         disp = Dyninst::read_memory_as<int16_t>(instr+1); // skip opcode
      } else if (type & REL_D) {
         disp = Dyninst::read_memory_as<int32_t>(instr+1);
      }
   }

   return disp;
}

int count_prefixes(unsigned insnType) {
   int nPrefixes = 0;
   if (insnType & PREFIX_OPR)
      nPrefixes++;
   if (insnType & PREFIX_SEG)
      nPrefixes++;
   if (insnType & PREFIX_OPCODE)
      nPrefixes++;
   if (insnType & PREFIX_REX)
      nPrefixes++;
   if (insnType & PREFIX_INST)
      nPrefixes++;
   if (insnType & PREFIX_ADDR)
      nPrefixes++;

   if(insnType & PREFIX_AVX) /* VEX2 */
      nPrefixes += 2;
   else if(insnType & PREFIX_AVX2) /* VEX3 */
      nPrefixes += 3;
   else if(insnType & PREFIX_AVX512) /* EVEX */
      nPrefixes += 4;

   /* Make sure dyninst didn't decode more than one VEX prefix. */
   if(((insnType & PREFIX_AVX) && (insnType & PREFIX_AVX2))
         || ((insnType & PREFIX_AVX2) && (insnType & PREFIX_AVX512))
         || ((insnType & PREFIX_AVX512) && (insnType & PREFIX_AVX))
         || ((insnType & PREFIX_AVX) && (insnType & PREFIX_AVX2) && (insnType & PREFIX_AVX512)))
      assert(!"An instruction can only have one type of VEX prefix!\n");

   return nPrefixes;
}

unsigned copy_prefixes(const unsigned char *&origInsn, unsigned char *&newInsn, unsigned insnType) {
   unsigned nPrefixes = count_prefixes(insnType);

   for (unsigned u = 0; u < nPrefixes; u++)
      *newInsn++ = *origInsn++;

   return nPrefixes;
}

//Copy all prefixes but the Operand-Size and Address-Size prefixes (0x66 and 0x67)
unsigned copy_prefixes_nosize(const unsigned char *&origInsn, unsigned char *&newInsn, 
      unsigned insnType)
{
   unsigned nPrefixes = count_prefixes(insnType);

   for (unsigned u = 0; u < nPrefixes; u++) {
      if (*origInsn == 0x66 || *origInsn == 0x67)
      {
         origInsn++;
         continue;
      }
      *newInsn++ = *origInsn++;
   }
   return nPrefixes;
}

bool convert_to_rel8(const unsigned char*&origInsn, unsigned char *&newInsn) {
   if (*origInsn == 0x0f)
      origInsn++;

   // We think that an opcode in the 0x8? range can be mapped to an equivalent
   // opcode in the 0x7? range...

   if ((*origInsn >= 0x70) &&
         (*origInsn < 0x80)) {
      *newInsn++ = *origInsn++;
      return true;
   }

   if ((*origInsn >= 0x80) &&
         (*origInsn < 0x90)) {
      *newInsn++ = *origInsn++ - 0x10;
      return true;
   }

   if (*origInsn == 0xE3) {
      *newInsn++ = *origInsn++;
      return true;
   }

   // Oops...
   fprintf(stderr, "Unhandled jump conversion case: opcode is 0x%x\n", *origInsn);
   assert(0 && "Unhandled jump conversion case!");
   return false;
}

bool convert_to_rel32(const unsigned char*&origInsn, unsigned char *&newInsn) {
   if (*origInsn == 0x0f)
      origInsn++;
   *newInsn++ = 0x0f;

   // We think that an opcode in the 0x7? range can be mapped to an equivalent
   // opcode in the 0x8? range...

   if ((*origInsn >= 0x70) &&
         (*origInsn < 0x80)) {
      *newInsn++ = *origInsn++ + 0x10;
      return true;
   }

   if ((*origInsn >= 0x80) &&
         (*origInsn < 0x90)) {
      *newInsn++ = *origInsn++;
      return true;
   }

   // Oops...
   fprintf(stderr, "Unhandled jump conversion case: opcode is 0x%x\n", *origInsn);
   assert(0 && "Unhandled jump conversion case!");
   return false;
}



//Determine appropriate scale, index, and base given SIB byte.
void decode_SIB(unsigned sib, unsigned& scale, Dyninst::Register& index_reg, Dyninst::Register& base_reg){
   scale = sib >> 6;

   //scale = 2^scale
   if(scale == 0)
      scale = 1;
   else if(scale == 1)
      scale = 2;
   else if(scale == 2)
      scale = 4;
   else if(scale == 3)
      scale = 8;

   index_reg = (sib >> 3) & 0x07;
   base_reg = sib & 0x07;
}

   const unsigned char*
skip_headers(const unsigned char* addr, ia32_instruction* instruct)
{
   ia32_instruction tmp;
   if(!instruct)
      instruct = &tmp;
   ia32_decode_prefixes(addr, *instruct, false);
   return addr + instruct->getSize();
}

bool insn_hasSIB(unsigned ModRMbyte,unsigned& Mod,unsigned& Reg,unsigned& RM){
   Mod = (ModRMbyte >> 6) & 0x03;
   Reg = (ModRMbyte >> 3) & 0x07;
   RM = ModRMbyte & 0x07;
   return ((Mod != 3) && (RM == 4));
}

bool insn_hasDisp8(unsigned ModRMbyte){
   unsigned Mod = (ModRMbyte >> 6) & 0x03;
   return (Mod == 1);
}

bool insn_hasDisp32(unsigned ModRMbyte){
   unsigned Mod = (ModRMbyte >> 6) & 0x03;
   /* unsigned Reg = (ModRMbyte >> 3) & 0x07; */
   unsigned RM = ModRMbyte & 0x07;
   return (Mod == 0 && RM == 5) || (Mod == 2);
}

const char* ia32_entry::name(ia32_locations* loc)
{
   dyn_hash_map<entryID, string>::const_iterator found = entryNames_IAPI.find(id);
   if(found != entryNames_IAPI.end())
   {
      return found->second.c_str();
   }
   if(loc)
   {
      if(otable == t_grp && (tabidx == Grp2 || tabidx == Grp11))
      {
         int reg = loc->modrm_reg;
         found = entryNames_IAPI.find(groupMap[tabidx][reg].id);
         if(found != entryNames_IAPI.end())
         {
            return found->second.c_str();
         }
      }
   }
   return NULL;
}

entryID ia32_entry::getID(ia32_locations* l) const
{
   if((id != e_No_Entry) || (tabidx == t_done) || (l == NULL))
   {
      return id;
   }
   int reg = l->modrm_reg;
   switch(otable)
   {
      case t_grp:
         switch(tabidx)
         {
            case Grp2:
            case Grp11:
               return groupMap[tabidx][reg].id;
               break;
            default:
               break;
         }
	 break;
      case t_coprocEsc:
         return e_fp_generic;
      case t_3dnow:
         return e_3dnow_generic;
      default:
         break;
   }
   return id;
}

bool isStackFramePrecheck_gcc( const unsigned char *buffer )
{
   //Currently enabled entry bytes for gaps:
   //  0x55 - push %ebp
   static char gap_initial_bytes[] =
   { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
   return (gap_initial_bytes[*buffer] != 0);
}  

bool isStackFramePrecheck_msvs( const unsigned char *buffer )
{
   //Currently enabled entry bytes for gaps:
   //  0x55 - push %ebp
   static char gap_initial_bytes[] =
   { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
   return (gap_initial_bytes[*buffer] != 0);
}  

/*
   bool isStackFramePreamble( instruction& insn1 )
   {
   instruction insn2, insn3;
   insn2.setInstruction( insn1.ptr() + insn1.size() );
   insn3.setInstruction( insn2.ptr() + insn2.size() );

   const unsigned char* p = insn1.op_ptr();
   const unsigned char* q = insn2.op_ptr();
   const unsigned char* r = insn3.op_ptr();

   unsigned Mod1_1 =  ( q[ 1 ] >> 3 ) & 0x07;
   unsigned Mod1_2 =  q[ 1 ] & 0x07;
   unsigned Mod2_1 =  ( r[ 1 ] >> 3 ) & 0x07;
   unsigned Mod2_2 =  r[ 1 ] & 0x07;

   if( insn1.size() != 1 )
   {
   return false;  //shouldn't need this, but you never know
   }

   if( p[ 0 ] == PUSHEBP  )
   {
// Looking for mov %esp -> %ebp in one of the two
// following instructions.  There are two ways to encode
// mov %esp -> %ebp: as '0x8b 0xec' or as '0x89 0xe5'.
if( insn2.isMoveRegMemToRegMem() &&
((Mod1_1 == 0x05 && Mod1_2 == 0x04) ||
(Mod1_1 == 0x04 && Mod1_2 == 0x05)))
return true;

if( insn3.isMoveRegMemToRegMem() &&
((Mod2_1 == 0x05 && Mod2_2 == 0x04) ||
(Mod2_1 == 0x04 && Mod2_2 == 0x05)))
return true;
}

return false;
}
*/

instruction *instruction::copy() const {
   // Or should we copy? I guess it depends on who allocated
   // the memory...
   return new instruction(*this);
}

unsigned instruction::spaceToRelocate() const {
   // List of instructions that might be painful:
   // jumps (displacement will change)
   // call (displacement will change)
   // PC-relative ops

   // TODO: pc-relative ops

   // longJumpSize == size of code needed to get
   // anywhere in memory space.
#if defined(arch_x86_64)
   const int longJumpSize = JUMP_ABS64_SZ;
#else
   const int longJumpSize = JUMP_ABS32_SZ;
#endif


   // Assumption: rewriting calls into immediates will
   // not be larger than rewriting a call into a call...

   if (!((type() & REL_B) ||
            (type() & REL_W) ||
            (type() & REL_D) ||
            (type() & REL_D_DATA))) {
      return size();
   }

   // Now that the easy case is out of the way...

   if (type() & IS_JUMP) {
      // Worst-case: prefixes + max-displacement branch
      return count_prefixes(type()) + longJumpSize;
   }
   if (type() & IS_JCC) {
      // Jump conditional; jump past jump; long jump
      return count_prefixes(type()) + 2 + 2 + longJumpSize;
   }
   if (type() & IS_CALL) {
      // Worst case is approximated by two long jumps (AMD64) or a REL32 (x86)
      unsigned size;
#if defined(arch_x86_64)
      size = 2*JUMP_ABS64_SZ+count_prefixes(type());
#else
      size = JUMP_SZ+count_prefixes(type());
#endif
      size = (size > CALL_RELOC_THUNK) ? size : CALL_RELOC_THUNK;
      return size;
   }
#if defined(arch_x86_64)
   if (type() & REL_D_DATA) {
      // Worst-case: replace RIP with push of IP, use, and cleanup
      // 8: unknown; previous constant
      return count_prefixes(type()) + size() + 8;
   }
#endif

   assert(0);
   return 0;
}

#if defined(arch_x86_64)
unsigned instruction::jumpSize(long disp, unsigned addr_width) 
{
   if (addr_width == 8 && !is_disp32(disp))
      return JUMP_ABS64_SZ;
   return JUMP_SZ;
}
#else
unsigned instruction::jumpSize(long /*disp*/, unsigned /*addr_width*/)
{
   return JUMP_SZ;
}
#endif

unsigned instruction::jumpSize(Address from, Address to, unsigned addr_width) 
{
   long disp = to - (from + JUMP_SZ);
   return jumpSize(disp, addr_width);
}

#if defined(arch_x86_64)
unsigned instruction::maxJumpSize(unsigned addr_width) 
{
   if (addr_width == 8)
      return JUMP_ABS64_SZ;
   return JUMP_SZ;
}
#else
unsigned instruction::maxJumpSize(unsigned /*addr_width*/)
{
   return JUMP_SZ;
}
#endif


#define SIB_SET_REG(x, y) ((x) |= ((y) & 7))
#define SIB_SET_INDEX(x, y) ((x) |= (((y) & 7) << 3))
#define SIB_SET_SS(x, y) ((x) | (((y) & 3) << 6))


} // namespace arch_x86
