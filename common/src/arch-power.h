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

// $Id: arch-power.h,v 1.45 2008/03/25 19:24:23 bernat Exp $

#ifndef _ARCH_POWER_H
#define _ARCH_POWER_H

// Code generation

#include "registers/ppc32_regs.h"
#include "registers/ppc64_regs.h"
#include <vector>
class AddressSpace;

namespace NS_power {

/*
 * Define power instruction information.
 *
 */

//struct genericform {
//  unsigned op : 6;
//  unsigned XX : 26;
//};
#define GENERIC_OP(x) ((unsigned int) (((x).asInt() & 0xfc000000) >> 26 ))
#define GENERIC_XX(x) ((unsigned int) (((x).asInt() & 0x03ffffff)       ))

#define GENERIC_OP_SET(x, y) ((x).setBits(26,  6, (y)))
#define GENERIC_XX_SET(x, y) ((x).setBits( 0, 26, (y)))

//struct iform {            // unconditional branch + 
//  unsigned op : 6;
//  signed   li : 24;
//  unsigned aa : 1;
//  unsigned lk : 1;
//};
#define IFORM_OP(x) ((unsigned int) (((x).asInt() & 0xfc000000) >> 26 ))
#define IFORM_LI(x) (instruction::signExtend( \
                                    (((x).asInt() & 0x03fffffc) >>  2 ), 24))
#define IFORM_AA(x) ((unsigned int) (((x).asInt() & 0x00000002) >>  1 ))
#define IFORM_LK(x) ((unsigned int) (((x).asInt() & 0x00000001)       ))

#define IFORM_OP_SET(x, y) ((x).setBits(26,  6, (y)))
#define IFORM_LI_SET(x, y) ((x).setBits( 2, 24, (y)))
#define IFORM_AA_SET(x, y) ((x).setBits( 1,  1, (y)))
#define IFORM_LK_SET(x, y) ((x).setBits( 0,  1, (y)))

//struct bform {            // conditional branch +
//  unsigned op : 6;
//  unsigned bo : 5;
//  unsigned bi : 5;
//  signed   bd : 14;
//  unsigned aa : 1;
//  unsigned lk : 1;
//};
#define BFORM_OP(x) ((unsigned int) (((x).asInt() & 0xfc000000) >> 26 ))
#define BFORM_BO(x) ((unsigned int) (((x).asInt() & 0x03e00000) >> 21 ))
#define BFORM_BI(x) ((unsigned int) (((x).asInt() & 0x001f0000) >> 16 ))
#define BFORM_BD(x) (instruction::signExtend( \
                                    (((x).asInt() & 0x0000fffc) >>  2 ), 14))
#define BFORM_AA(x) ((unsigned int) (((x).asInt() & 0x00000002) >>  1 ))
#define BFORM_LK(x) ((unsigned int) (((x).asInt() & 0x00000001)       ))

#define BFORM_OP_SET(x, y) ((x).setBits(26,  6, (y)))
#define BFORM_BO_SET(x, y) ((x).setBits(21,  5, (y)))
#define BFORM_BI_SET(x, y) ((x).setBits(16,  5, (y)))
#define BFORM_BD_SET(x, y) ((x).setBits( 2, 14, (y)))
#define BFORM_AA_SET(x, y) ((x).setBits( 1,  1, (y)))
#define BFORM_LK_SET(x, y) ((x).setBits( 0,  1, (y)))

//struct dform {
//    unsigned op : 6;
//    unsigned rt : 5;        // rt, rs, frt, frs, to, bf_l
//    unsigned ra : 5;
//    signed   d_or_si : 16;  // d, si, ui
//};
#define DFORM_OP(x) ((unsigned int) (((x).asInt() & 0xfc000000) >> 26 ))
#define DFORM_RT(x) ((unsigned int) (((x).asInt() & 0x03e00000) >> 21 ))
#define DFORM_RA(x) ((unsigned int) (((x).asInt() & 0x001f0000) >> 16 ))
#define DFORM_D(x)  (instruction::signExtend( \
                                    (((x).asInt() & 0x0000ffff)       ), 16))
#define DFORM_SI(x) (instruction::signExtend( \
                                    (((x).asInt() & 0x0000ffff)       ), 16))

#define DFORM_OP_SET(x, y) ((x).setBits(26,  6, (y)))
#define DFORM_RT_SET(x, y) ((x).setBits(21,  5, (y)))
#define DFORM_RA_SET(x, y) ((x).setBits(16,  5, (y)))
#define DFORM_D_SET(x, y)  ((x).setBits( 0, 16, (y)))
#define DFORM_SI_SET(x, y) ((x).setBits( 0, 16, (y)))

//struct dsform {
//    unsigned op : 6;
//    unsigned rt : 5;        // rt, rs
//    unsigned ra : 5;
//    signed   ds : 14;
//    unsigned xo : 2;
//};
#define DSFORM_OP(x) ((unsigned int) (((x).asInt() & 0xfc000000) >> 26 ))
#define DSFORM_RT(x) ((unsigned int) (((x).asInt() & 0x03e00000) >> 21 ))
#define DSFORM_RA(x) ((unsigned int) (((x).asInt() & 0x001f0000) >> 16 ))
#define DSFORM_DS(x) (instruction::signExtend( \
                                     (((x).asInt() & 0x0000fffc) >>  2 ), 14))
#define DSFORM_XO(x) ((unsigned int) (((x).asInt() & 0x00000003)       ))

#define DSFORM_OP_SET(x, y) ((x).setBits(26,  6, (y)))
#define DSFORM_RT_SET(x, y) ((x).setBits(21,  5, (y)))
#define DSFORM_RA_SET(x, y) ((x).setBits(16,  5, (y)))
#define DSFORM_DS_SET(x, y) ((x).setBits( 2, 14, (y)))
#define DSFORM_XO_SET(x, y) ((x).setBits( 0,  2, (y)))

//struct xform {
//    unsigned op : 6;
//    unsigned rt : 5;   // rt, frt, bf_l, rs, frs, to, bt
//    unsigned ra : 5;   // ra, fra, bfa_, sr, spr
//    unsigned rb : 5;   // rb, frb, sh, nb, u_
//    unsigned xo : 10;  // xo, eo
//    unsigned rc : 1;
//};
#define XFORM_OP(x) ((unsigned int) (((x).asInt() & 0xfc000000) >> 26 ))
#define XFORM_RT(x) ((unsigned int) (((x).asInt() & 0x03e00000) >> 21 ))
#define XFORM_RA(x) ((unsigned int) (((x).asInt() & 0x001f0000) >> 16 ))
#define XFORM_RB(x) ((unsigned int) (((x).asInt() & 0x0000f800) >> 11 ))
#define XFORM_XO(x) ((unsigned int) (((x).asInt() & 0x000007fe) >>  1 ))
#define XFORM_RC(x) ((unsigned int) (((x).asInt() & 0x00000001)       ))

#define XFORM_OP_SET(x, y) ((x).setBits(26,  6, (y)))
#define XFORM_RT_SET(x, y) ((x).setBits(21,  5, (y)))
#define XFORM_RA_SET(x, y) ((x).setBits(16,  5, (y)))
#define XFORM_RB_SET(x, y) ((x).setBits(11,  5, (y)))
#define XFORM_XO_SET(x, y) ((x).setBits( 1, 10, (y)))
#define XFORM_RC_SET(x, y) ((x).setBits( 0,  1, (y)))

//struct xlform {
//  unsigned op : 6;
//  unsigned bt : 5;   // rt, bo, bf_
//  unsigned ba : 5;   // ba, bi, bfa_
//  unsigned bb : 5; 
//  unsigned xo : 10;  // xo, eo
//  unsigned lk : 1;
//};
#define XLFORM_OP(x) ((unsigned int) (((x).asInt() & 0xfc000000) >> 26 ))
#define XLFORM_BT(x) ((unsigned int) (((x).asInt() & 0x03e00000) >> 21 ))
#define XLFORM_BA(x) ((unsigned int) (((x).asInt() & 0x001f0000) >> 16 ))
#define XLFORM_BB(x) ((unsigned int) (((x).asInt() & 0x0000f800) >> 11 ))
#define XLFORM_XO(x) ((unsigned int) (((x).asInt() & 0x000007fe) >>  1 ))
#define XLFORM_LK(x) ((unsigned int) (((x).asInt() & 0x00000001)       ))

#define XLFORM_OP_SET(x, y) ((x).setBits(26,  6, (y)))
#define XLFORM_BT_SET(x, y) ((x).setBits(21,  5, (y)))
#define XLFORM_BA_SET(x, y) ((x).setBits(16,  5, (y)))
#define XLFORM_BB_SET(x, y) ((x).setBits(11,  5, (y)))
#define XLFORM_XO_SET(x, y) ((x).setBits( 1, 10, (y)))
#define XLFORM_LK_SET(x, y) ((x).setBits( 0,  1, (y)))

/* struct xx1form {
	unsigned op : 6;
	unsigned t  : 5;
	unsigned RA : 5;
	unsigned RB : 5; 
	unsigned xo : 10;
	unsigned TX/SX : 1;
};
*/




//struct xfxform {
//  unsigned op : 6;
//  unsigned rt : 5;   // rs
//  unsigned spr: 10;  // spr, tbr, fxm
//  unsigned xo : 10;
//  unsigned rc : 1;
//};
#define XFXFORM_OP(x)  ((unsigned int) (((x).asInt() & 0xfc000000) >> 26 ))
#define XFXFORM_RT(x)  ((unsigned int) (((x).asInt() & 0x03e00000) >> 21 ))
#define XFXFORM_SPR(x) ((unsigned int) (((x).asInt() & 0x001ff800) >> 11 ))
#define XFXFORM_XO(x)  ((unsigned int) (((x).asInt() & 0x000007fe) >>  1 ))
#define XFXFORM_RC(x)  ((unsigned int) (((x).asInt() & 0x00000001)       ))

#define XFXFORM_OP_SET(x, y)  ((x).setBits(26,  6, (y)))
#define XFXFORM_RT_SET(x, y)  ((x).setBits(21,  5, (y)))
#define XFXFORM_SPR_SET(x, y) ((x).setBits(11, 10, (y)))
#define XFXFORM_XO_SET(x, y)  ((x).setBits( 1, 10, (y)))
#define XFXFORM_RC_SET(x, y)  ((x).setBits( 0,  1, (y)))

//struct xflform {
//  unsigned op : 6;
//  unsigned u1 : 1;
//  unsigned flm: 8;
//  unsigned u2 : 1;
//  unsigned frb: 5;
//  unsigned xo : 10;
//  unsigned rc : 1;
//};
#define XFLFORM_OP(x)  ((unsigned int) (((x).asInt() & 0xfc000000) >> 26 ))
#define XFLFORM_U1(x)  ((unsigned int) (((x).asInt() & 0x02000000) >> 25 ))
#define XFLFORM_FLM(x) ((unsigned int) (((x).asInt() & 0x01fe0000) >> 17 ))
#define XFLFORM_U2(x)  ((unsigned int) (((x).asInt() & 0x00010000) >> 16 ))
#define XFLFORM_FRB(x) ((unsigned int) (((x).asInt() & 0x0000f800) >> 11 ))
#define XFLFORM_XO(x)  ((unsigned int) (((x).asInt() & 0x000007fe) >>  1 ))
#define XFLFORM_RC(x)  ((unsigned int) (((x).asInt() & 0x00000001)       ))

#define XFLFORM_OP_SET(x, y)  ((x).setBits(26,  6, (y)))
#define XFLFORM_U1_SET(x, y)  ((x).setBits(25,  1, (y)))
#define XFLFORM_FLM_SET(x, y) ((x).setBits(17,  8, (y)))
#define XFLFORM_U2_SET(x, y)  ((x).setBits(16,  1, (y)))
#define XFLFORM_FRB_SET(x, y) ((x).setBits(11,  5, (y)))
#define XFLFORM_XO_SET(x, y)  ((x).setBits( 1, 10, (y)))
#define XFLFORM_RC_SET(x, y)  ((x).setBits( 0,  1, (y)))

//struct xoform {
//    unsigned op : 6;
//    unsigned rt : 5;
//    unsigned ra : 5;
//    unsigned rb : 5;
//    unsigned oe : 1;
//    unsigned xo : 9; // xo, eo'
//    unsigned rc : 1;
//};
#define XOFORM_OP(x) ((unsigned int) (((x).asInt() & 0xfc000000) >> 26 ))
#define XOFORM_RT(x) ((unsigned int) (((x).asInt() & 0x03e00000) >> 21 ))
#define XOFORM_RA(x) ((unsigned int) (((x).asInt() & 0x001f0000) >> 16 ))
#define XOFORM_RB(x) ((unsigned int) (((x).asInt() & 0x0000f800) >> 11 ))
#define XOFORM_OE(x) ((unsigned int) (((x).asInt() & 0x00000400) >> 10 ))
#define XOFORM_XO(x) ((unsigned int) (((x).asInt() & 0x000003fe) >>  1 ))
#define XOFORM_RC(x) ((unsigned int) (((x).asInt() & 0x00000001)       ))

#define XOFORM_OP_SET(x, y) ((x).setBits(26,  6, (y)))
#define XOFORM_RT_SET(x, y) ((x).setBits(21,  5, (y)))
#define XOFORM_RA_SET(x, y) ((x).setBits(16,  5, (y)))
#define XOFORM_RB_SET(x, y) ((x).setBits(11,  5, (y)))
#define XOFORM_OE_SET(x, y) ((x).setBits(10,  1, (y)))
#define XOFORM_XO_SET(x, y) ((x).setBits( 1,  9, (y)))
#define XOFORM_RC_SET(x, y) ((x).setBits( 0,  1, (y)))

//struct mform {
//    unsigned op : 6;
//    unsigned rs : 5;
//    unsigned ra : 5;
//    unsigned sh : 5;
//    unsigned mb : 5; // mb, sh
//    unsigned me : 5;
//    unsigned rc : 1;
//};
#define MFORM_OP(x) ((unsigned int) (((x).asInt() & 0xfc000000) >> 26 ))
#define MFORM_RS(x) ((unsigned int) (((x).asInt() & 0x03e00000) >> 21 ))
#define MFORM_RA(x) ((unsigned int) (((x).asInt() & 0x001f0000) >> 16 ))
#define MFORM_SH(x) ((unsigned int) (((x).asInt() & 0x0000f800) >> 11 ))
#define MFORM_MB(x) ((unsigned int) (((x).asInt() & 0x000007c0) >>  6 ))
#define MFORM_ME(x) ((unsigned int) (((x).asInt() & 0x0000003e) >>  1 ))
#define MFORM_RC(x) ((unsigned int) (((x).asInt() & 0x00000001)       ))

#define MFORM_OP_SET(x, y) ((x).setBits(26,  6, (y)))
#define MFORM_RS_SET(x, y) ((x).setBits(21,  5, (y)))
#define MFORM_RA_SET(x, y) ((x).setBits(16,  5, (y)))
#define MFORM_SH_SET(x, y) ((x).setBits(11,  5, (y)))
#define MFORM_MB_SET(x, y) ((x).setBits( 6,  5, (y)))
#define MFORM_ME_SET(x, y) ((x).setBits( 1,  5, (y)))
#define MFORM_RC_SET(x, y) ((x).setBits( 0,  1, (y)))

//struct mdform {
//    unsigned op : 6;
//    unsigned rs : 5;
//    unsigned ra : 5;
//    unsigned sh : 5;
//    unsigned mb : 5; // me
//    unsigned mb2 : 1;
//    unsigned xo : 3;
//    unsigned sh2 : 1;
//    unsigned rc : 1;
//};
#define MDFORM_OP(x)  ((unsigned int) (((x).asInt() & 0xfc000000) >> 26 ))
#define MDFORM_RS(x)  ((unsigned int) (((x).asInt() & 0x03e00000) >> 21 ))
#define MDFORM_RA(x)  ((unsigned int) (((x).asInt() & 0x001f0000) >> 16 ))
#define MDFORM_SH(x)  ((unsigned int) (((x).asInt() & 0x0000f800) >> 11 ))
#define MDFORM_MB(x)  ((unsigned int) (((x).asInt() & 0x000007c0) >>  6 ))
#define MDFORM_MB2(x) ((unsigned int) (((x).asInt() & 0x00000020) >>  5 ))
#define MDFORM_XO(x)  ((unsigned int) (((x).asInt() & 0x0000001c) >>  2 ))
#define MDFORM_SH2(x) ((unsigned int) (((x).asInt() & 0x00000002) >>  1 ))
#define MDFORM_RC(x)  ((unsigned int) (((x).asInt() & 0x00000001)       ))

#define MDFORM_OP_SET(x, y)  ((x).setBits(26,  6, (y)))
#define MDFORM_RS_SET(x, y)  ((x).setBits(21,  5, (y)))
#define MDFORM_RA_SET(x, y)  ((x).setBits(16,  5, (y)))
#define MDFORM_SH_SET(x, y)  ((x).setBits(11,  5, (y)))
#define MDFORM_MB_SET(x, y)  ((x).setBits( 6,  5, (y)))
#define MDFORM_MB2_SET(x, y) ((x).setBits( 5,  1, (y)))
#define MDFORM_XO_SET(x, y)  ((x).setBits( 2,  3, (y)))
#define MDFORM_SH2_SET(x, y) ((x).setBits( 1,  1, (y)))
#define MDFORM_RC_SET(x, y)  ((x).setBits( 0,  1, (y)))

//struct aform {
//  unsigned op: 6;
//  unsigned frt: 5;
//  unsigned fra: 5;
//  unsigned frb: 5;
//  unsigned frc: 5;
//  unsigned xo:  5;
//  unsigned rc:  1;
//};
#define AFORM_OP(x)  ((unsigned int) (((x).asInt() & 0xfc000000) >> 26 ))
#define AFORM_FRT(x) ((unsigned int) (((x).asInt() & 0x03e00000) >> 21 ))
#define AFORM_FRA(x) ((unsigned int) (((x).asInt() & 0x001f0000) >> 16 ))
#define AFORM_FRB(x) ((unsigned int) (((x).asInt() & 0x0000f800) >> 11 ))
#define AFORM_FRC(x) ((unsigned int) (((x).asInt() & 0x000007c0) >>  6 ))
#define AFORM_XO(x)  ((unsigned int) (((x).asInt() & 0x0000003e) >>  1 ))
#define AFORM_RC(x)  ((unsigned int) (((x).asInt() & 0x00000001)       ))

#define AFORM_OP_SET(x, y)  ((x).setBits(26,  6, (y)))
#define AFORM_FRT_SET(x, y) ((x).setBits(21,  5, (y)))
#define AFORM_FRA_SET(x, y) ((x).setBits(16,  5, (y)))
#define AFORM_FRB_SET(x, y) ((x).setBits(11,  5, (y)))
#define AFORM_FRC_SET(x, y) ((x).setBits( 6,  5, (y)))
#define AFORM_XO_SET(x, y)  ((x).setBits( 1,  5, (y)))
#define AFORM_RC_SET(x, y)  ((x).setBits( 0,  1, (y)))

typedef union {
//  struct iform  iform;  // branch;
//  struct bform  bform;  // cbranch;
//  struct dform  dform;
//  struct dsform dsform;
//  struct xform  xform;
//  struct xoform xoform;
//  struct xlform xlform;
//  struct xfxform xfxform;
//  struct xflform xflform;
//  struct mform  mform;
//  struct mdform  mdform;
//  struct aform  aform;
    unsigned char byte[4];
    unsigned int  raw;
} instructUnion;

// instruction is now a class for platform-indep.

// Mmmm alignment
typedef instructUnion codeBuf_t;
typedef unsigned codeBufIndex_t;

#define SPR_XER	1
#define SPR_LR	8
#define SPR_CTR	9
#define SPR_TAR 815
#define SPR_MQ 0

/*
 * Register saving constants
 */
#define maxFPR 32           /* Save FPRs 0-13 */
#define maxGPR 32           /* More space than is needed */
#define FPRspaceUsed (8*16) /* Aligned space for FPRs */
#define GPRoffset(reg) (-1* (FPRspaceUsed + 4*(maxGPR-reg)))
#define GPRspaceUsed (20*4) /* Aligned space for GPRs */

#define stackFrameSize (FPRspaceUsed+GPRspaceUsed+128)

/*
 * Define the operation codes
 */

#define X_EXTENDEDop     31
#define XO_EXTENDEDop    31
#define X_FP_EXTENDEDop  63
#define A_FP_EXTENDEDop1  59   
#define A_FP_EXTENDEDop2  63

// ------------- Op Codes, instruction form I  ------------------
#define Bop		18	/* (unconditional) branch */

// ------------- Op Codes, instruction form D  ------------------
#define TIop             3      /* trap immediate */
#define MULIop           7      /* multiply immediate */
#define SFIop            8      /* subtract from immediate */
#define DOZIop           9
#define CMPLIop         10
#define CMPIop		11	/* compare immediate */
#define SIop            12      /* subtract immediate */
#define AIDOTop         13
#define CALop		14	/* compute address lower -- ADDIop */
#define CAUop		15	/* compute address upper -- ADDISop*/
#define ORILop		24	/* (logical) or immediate lower -- ORIop*/
#define ORIUop          25
#define XORILop         26
#define XORIUop         27 
#define ANDILop         28      /* and immediate lower -- ANDIop*/
#define ANDIUop         29
#define RLDop		30	/* RLD* family -- rotate left doubleword */
#define Lop		32	/* load (word) (aka lwz op in PowerPC) */
#define LUop		33
#define LBZop		34
#define LBZUop		35
#define STop		36	/* store (word) -- STWop */
#define STUop		37	/* store (word) with update -- STWUop */
#define STBop		38
#define STBUop		39
#define LHZop		40
#define LHZUop		41
#define LHAop		42
#define LHAUop		43
#define STHop		44
#define STHUop		45
#define LMop		46
#define STMop		47
#define LFSop		48
#define LFSUop		49
#define LFDop           50      /* load floating-point double */
#define LFDUop		51
#define STFSop		52
#define STFSUop		53
#define STFDop          54      /* store floating-point double */
#define STFDUop		55

// ------------- Op Codes, instruction form DS  ------------------
#define LDop		58	// LWA and LDU have the same op, xop differs
#define LDxop		0
#define LDUxop		1
#define LWAxop		2
#define STDop		62	// ditto
#define STDxop		0
#define STDUxop		1

// ------------- Op Codes, instruction form B  ------------------
#define BCop		16	/* branch conditional */

// ------------- Op Codes, instruction form MD ------------------
#define RLDop          30      /* RLD* family -- rotate left doubleword */
#define ICLxop          0      // Immediate and Clear Left
#define ICRxop          1      // Immediate and Clear Right

// ------------- Op Codes, instruction form X  ------------------
/* #define XFPop        31      -- extendened fixed point ops */
// -- X-from Loads
#define LXop		31	// VG: all X-from loads have op 31, only xop differs
#define LWARXxop	20
#define LDXxop		21
#define LXxop		23
#define LDUXxop		53
#define LUXxop		55
#define LDARXxop	84
#define LBZXxop		87
#define LBZUXxop	119
#define LHZXxop		279
#define LHZUXxop	311
#define MFSPRop         31
#define MFSPRxop	339
#define LHAXxop		343
#define LWAXxop		341
#define LWAUXxop	373
#define LHAUXxop	375
#define LSXxop		533
#define LWBRXxop	534
#define LFSXxop		535
#define LFSUXxop	567	// I guess it must be so ;)
#define LSIxop		597
#define LFDXxop		599
#define LFDUXxop	631
#define LHBRXxop	790
// -- X-from stores
#define STXop		31	// VG: X-form stores, same story
#define STDXxop		149
#define STWCXxop	150
#define STXxop		151
#define STDUXxop	181
#define STUXxop		183
#define STDCXxop	214
#define STBXxop		215
#define STBUXxop	247
#define STHXxop		407
#define STHUXxop	439
#define STSXxop		661
#define STBRXxop	662
#define STFSXxop	663
#define STFSUXxop	695
#define STSIxop		725
#define STFDXxop	727
#define STFDUXxop	759
#define STHBRXxop	918
#define STFIWXxop	983
// -- other X-forms
#define CMPop           31      /* compare -- XFPop*/
#define CMPxop		0       /* compare */
#define CMPLxop         32      /* unsigned compare */
#define ANDop           31      /* and */
#define ANDxop          28      /* and */
#define ORop            31      /* or */
#define ORxop           444     /* or */
#define XORop           31
#define XORxop          316

// -- Other extended op codes for X, XFX, & XO when op is 31
#define EXTop           31
#define Txop             4
#define Axop            10
#define MULHWUxop       11
#define MFCRxop         19
#define SLxop           24
#define CNTLZxop        26
#define SLDxop          27
#define MASKGxop        29
#define CMPLxop         32
#define SUBFxop         40
#define DCBSxop         54
#define ANDCxop         60
#define MULHWxop        75
#define MFMSRxop        83
#define DCBFxop         86
#define LBZXxop         87
#define NEGxop         104
#define MULxop         107
#define CLFxop         118
#define NORxop         124
#define SFExop         136
#define AExop          138
#define MTCRFxop       144
#define MTMSRxop       146
#define SLQxop         152
#define SLExop         153
#define SLIQxop        184
#define SFZExop        200
#define AZExop         202
#define MTSRxop        210
#define SLLQxop        216
#define SLEQxop        217
#define SFMExop        232
#define AMExop         234
#define MTSRIxop       242
#define DCBTSTxop      246
#define SLLIQxop       248
#define DOZxop         264
#define LSCBXxop       277
#define DCBTxop        278
#define EQVxop         284
#define TLBIxop        306
#define DIVxop         331
#define ABSxop         360
#define ORCxop         412
#define DIVWUxop       459
#define MTSPRop         31
#define MTSPRxop       467
#define DCBIxop        470
#define NANDxop        476
#define NABSxop        488
#define DIVWxop        491
#define CLIxop         502
#define CLCSxop        531
#define SRxop          536
#define RRIBxop        537
#define SRDxop         539
#define MASKIRxop      541
#define LFSUXxop       567
#define MFSRxop        595
#define MFSRIxop       627
#define DCLSTxop       630
#define MFSRINxop      659
#define SRQxop         664
#define SRExop         665
#define SRIQxop        696
#define SRLQxop        728
#define SREQxop        729
#define SRLIQxop       760
#define SRAxop         792
#define RACxop         818
#define SRAIxop        824
#define EIEIOxop       854
#define SRAQxop        920
#define SREAxop        921
#define EXTSxop        922
#define SRAIQxop       952
#define EXTSBxop       954
#define ICBIxop        982
#define DCLZxop       1014

// Vector Op Codes (XX1 Form, identical to XFORM)
#define LXVD2Xop 	    31
#define LXVD2Xxo	   844
#define STXVD2Xop 		31
#define STXVD2Xxo      972

// ------------- Op Codes, instruction form XL  -----------------
#define BCLRop		19	/* branch conditional link register */
#define BCLRxop		16	/* branch conditional link register */
#define BCCTRop         19      /* branch conditional count register */
#define BCCTRxop        528     /* branch conditional count register */
#define BCTARop			19 /* Branch conditional to TAR register */
#define BCTARxop        560 /* Branch conditional to TAR register */ 

// ------------- Op Codes, instruction form XO  -----------------
/* #define XFPop        31      -- extendened fixed point ops */
#define SFop		31      /* subtract from -- XFPop */
#define SFxop		8       /* subtract from -- SUBFxop */
#define MULSop          31      /* multiply short -- XFPop */
#define MULSxop         235     /* multiply short -- MULLWxop */
#define MULLxop         233     /* multiply long -- 64-bit integer multiplication */
#define CAXop		31      /* compute address -- XFPop */
#define CAXxop		266     /* compute address -- ADDxop */
#define DIVSop          31      /* divide short -- XFPop */
#define DIVSxop         363     /* divide short -- replacing DIVWxop */
#define DIVLSxop        489     /* divide signed long  -- 64-bit integer divison */
#define DIVLUxop        457     /* divide unsigned long  -- 64-bit integer divison */


// ------------- Extended Floating PointOp Codes, instruction form A ---
// Op code - 59
#define FDIVSxop        18
#define FSUBSxop        20
#define FADDSxop        21
#define FMULSxop        25
#define FMSUBSxop       28
#define FMADDSxop       29
#define FNMSUBSxop      30
#define FNMADDSxop      31

// Op code - 63
#define FDxop           18
#define FSxop           20
#define FAxop           21
#define FSQRTxop        22
#define FMxop           25
#define FMSxop          28
#define FMAxop          29
#define FNMSxop         30
#define FNMAxop         31

// ------------- Extended Floating Point Op Codes, instruction form X---
// Op Code - 63
#define FCMPUxop         0
#define FRSPxop         12
#define FCIRxop         14
#define FCIRZxop        15
#define FCMPOxop        32
#define FNEGxop         40
#define FMRxop          72
#define FNABSxop       136
#define FABSxop        264
#define MFFSxop        583
#define MTFSFxop       711

// ------------- Op Codes, instruction form SC  -----------------
#define SVCop		17	/* supervisor call -- used to be SCop */

// ------------- Op Codes, instruction form M  -----------------

#define RLIMIop         20      /* rotate left immediate then mask insert */
#define RLINMxop        21      /* rotate left immediate then AND with mask
                                 * -- RLWINMxop */
#define RLMIop          22
#define RLNMop          23

#define RLDICLop        30      /* Rotate Left Doubleword Imm and Clear Left */

// -------------------------- Raw instructions ------------------
/* a few full instructions that are in forms we don't break down by field */
// xlform
#define MTLR0raw       0x7c0803a6      /* move from link reg -- mtlw r0 */
#define MFLR0raw       0x7c0802a6      /* move from link reg -- mflr r0 */
#define MTLR2raw       0x7c4803a6      /* move from link reg -- mtlr r2 */
#define MFLR2raw       0x7c4802a6      /* move from link reg -- mflr r2 */
#define MR12CTR        0x7d8903a6      /* move from r12 to CTR */
#define BCTRraw        0x4e800420      /* bctr instrunction */
#define BCTRLraw       0x4e800421      /* bctrl instrunction */
#define BRraw          0x4e800020      /* br instruction bclr */
#define BRLraw         0x4e800021      /* branch and link to link reg bclrl */
#define NOOPraw        0x60000000      /* noop, d form ORIL 0, 0, 0 */

// -------------------------- Branch fields ------------------------------
// BO field of branch conditional
#define Bcondmask		0x1e
#define BALWAYSmask		0x14

#define BPREDICTbit		1	// Set means reverse branch prediction
#define BifCTRzerobit		2	// Set means branch if CTR = 0
#define BnoDecCTRbit		4	// Set means decrement CTR and branch
#define BifTRUEbit		8	// Set means branch if condition true
#define BnoCondbit		16	// Set means ignore condition (use CTR)

#define BFALSEcond              4
#define BTRUEcond               12
#define BALWAYScond		20

/* BI field for branch conditional (really bits of the CR Field to use) */
#define LTcond			0		/* negative */
#define GTcond			1		/* positive */
#define EQcond			2		/* zero */
#define SOcond			3		/* summary overflow */

// -------------------------- Load/Store fields ---------------------------
#define RTmask		0x03e00000		// bits  6-10
#define RAmask		0x001f0000		// bits 11-15
#define RBmask		0x0000f800		// bits 16-20
#define DinDmask	0x0000ffff		// bits 16-31
#define DinDSmask	0x0000fffc		// bits 16-29

#define getRT(x) (((x) & RTmask) >> 21)
#define getRAByMask(x) (((x) & RAmask) >> 16)
#define getRB(x) (((x) & RBmask) >> 11)
#define getDinD(x) ((x) & DinDmask)
#define getDinDS(x) ((x) & DinDSmask)

// -------------------------------------------------------------------

/* mask bits for various parts of the more common instruction format */
#define OPmask		0xfc000000              /* most sig. 6 bits */
#define AAmask		0x00000002		/* absolutate address */
#define LLmask		0x00000001		/* set linkage register */
#define AALKmask	AAmask|LLmask		/* AA | LK bits */
#define FULLmask	0xffffffff

#define Bmask		OPmask | AAmask
#define Bmatch		0x48000000 /* pc relative unconditional branch */
#define BCmatch		0x40000000 /* pc relative conditional branch */
#define BAAmatch		0x48000002 /* pc relative unconditional branch */
#define BCAAmatch		0x40000002 /* pc relative conditional branch */



#define BREAK_POINT_INSN 0x7d821008  /* trap */

/* high and low half words.  Useful to load addresses as two parts */
#define LOW(x)  ((x) & 0xffff)
#define HIGH(x) (((x) >> 16) & 0xffff)
// HA: adjusted hi value compensating for LOW(x) being sign extended
#define HA(x)   ((((x) >> 16) + (((x) & 0x8000) ? 1 : 0)) & 0xffff)

/* high and low half words for top and bottom words.  Useful to load
 * addresses in four parts.
 */
#define TOP_HI(x) (((x) >> 48))          // Can't think of a way to do this on
#define TOP_LO(x) (((x) >> 32) & 0xFFFF) // 32-bit compilers without warnings.
#define BOT_HI(x) (((x) >> 16) & 0xFFFF)
#define BOT_LO(x) (((x)      ) & 0xFFFF)

#define ABS(x)		((x) > 0 ? x : -x)
//#define MAX_BRANCH	0x1<<23
#define MAX_BRANCH      0x01fffffc
#define MAX_CBRANCH	0x1<<13

#define MAX_IMM		0x1<<15		/* 15 plus sign == 16 bits */

// Delcared some other functions in inst-power.C
// bool isCallInsn(const instruction);
// bool isReturnInsn(const image *, Address, bool&);

// Define bounds for immediate offsets.
// Use strange definitions to avoid compiler warnings.
#define MAX_IMM16      32767
#define MIN_IMM16      -32768

#define MAX_IMM32      2147483647
#define MIN_IMM32      (-2147483647 - 1)    // In C90, there are no negative
                                            // constants. Only negated positive
                                            // constants. Hopefully, compiler
                                            // will optimize this away.

#define MAX_IMM48      ((long)(-1 >> 17))   // To avoid warnings on 32-bit
#define MIN_IMM48      ((long)(~MAX_IMM48)) // compilers.

// Helps to mitigate host/target endian mismatches
COMMON_EXPORT unsigned int swapBytesIfNeeded(unsigned int i);

///////////////////////////////////////////////////////
// Bum bum bum.....
///////////////////////////////////////////////////////

class COMMON_EXPORT instruction {
 private:
    instructUnion insn_;

 public:
    instruction() { insn_.raw = 0; }
    instruction(unsigned int raw) {
        // Don't flip bits here.  Input is already in host byte order.
        insn_.raw = raw;
    }
    // Pointer creation method
    instruction(const void *ptr) {
      insn_ = *((const instructUnion *)ptr);
    }
    instruction(const void *ptr, bool) {
      insn_ = *((const instructUnion *)ptr);
    }

    instruction *copy() const;

    void clear() { insn_.raw = 0; }
    void setInstruction(codeBuf_t *ptr, Dyninst::Address = 0);
    void setBits(unsigned int pos, unsigned int len, unsigned int value) {
        unsigned int mask;

        mask = ~(~0U << len);
        value = value & mask;

        mask = ~(mask << pos);
        value = value << pos;

        insn_.raw = insn_.raw & mask;
        insn_.raw = insn_.raw | value;
    }
    unsigned int asInt() const { return insn_.raw; }
    void setInstruction(unsigned char *ptr, Dyninst::Address = 0);
    

    // To solve host/target endian mismatches
    static int signExtend(unsigned int i, unsigned int pos);
    static instructUnion &swapBytes(instructUnion &i);

    // We need instruction::size() all _over_ the place.
    static unsigned size() { return sizeof(instructUnion); } 

    Dyninst::Address getBranchOffset() const;
    void setBranchOffset(Dyninst::Address newOffset);

    // And tell us how much space we'll need...
    // Returns -1 if we can't do a branch due to architecture limitations
    static unsigned jumpSize(Dyninst::Address from, Dyninst::Address to, unsigned addr_width);
    static unsigned jumpSize(Dyninst::Address disp, unsigned addr_width);
    static unsigned maxJumpSize(unsigned addr_width);

    static unsigned maxInterFunctionJumpSize(unsigned addr_width);

    // return the type of the instruction
    unsigned type() const;
    
    // return a pointer to the instruction
    const unsigned char *ptr() const { return (const unsigned char *)&insn_; }

    // For external modification
    // Don't allow external modification anymore.  Host byte order may differ
    // from target byte order.
    //instructUnion &operator* () { return insn_; }
    //const instructUnion &operator* () const { return insn_; }
    //const unsigned int &raw() const { return insn_.raw; }

    unsigned opcode() const;
    
    // Local version
    bool isInsnType(const unsigned mask, const unsigned match) const { 
        return ((insn_.raw & mask) == match);
    }
    
    Dyninst::Address getTarget(Dyninst::Address insnAddr) const;
    
    unsigned spaceToRelocate() const;
    bool getUsedRegs(std::vector<int> &regs);
    
    
    /* -- CHECK !!!!!
     * catch small ints that are invalid instructions
     * opcode 0 is really reserved, not illegal (with the exception of all 0's).
     *
     * opcodes 1, 4-6, 56-57, 60-61 are illegal.
     *
     * opcodes 2, 30, 58, 62 are illegal in 32 bit implementations, but are
     *    defined in 64 bit implementations.
     *
     * opcodes 19, 30, 31, 59, 62, 63 contain extended op codes that are unused.
     */
    
    bool valid() const { return IFORM_OP(*this) > 0; }
    
    bool isCall() const;
    
    static bool isAligned(Dyninst::Address addr) {
        return !(addr & 0x3);
    }
    
    bool isCondBranch() const;
    bool isUncondBranch() const;
    bool isThunk() const;


  bool isCleaningRet() const {return false; }

};

} // arch_power namespace
#endif
