/*
 * Copyright (c) 1996-2009 Barton P. Miller
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

// $Id: inst-sparc.h,v 1.70 2008/06/19 22:13:42 jaw Exp $

#if !defined(sparc_sun_sunos4_1_3) && !defined(sparc_sun_solaris2_4)
#error "invalid architecture-os inclusion"
#endif

#ifndef INST_SPARC_H
#define INST_SPARC_H

#include "common/h/headers.h"
#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/inst.h"

#include "common/h/arch.h"
#include "dyninstAPI/src/util.h"
#include "common/h/stats.h"
#include "dyninstAPI/src/os.h"
#include "dyninstAPI/src/debug.h"
#include "dyninstAPI/src/as-sparc.h"
#include "dyninstAPI/src/instP.h"

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

#define INSN_SIZE ( sizeof( instruction ) )

#define REG_MT_POS           REG_G(6)   /* Register which contains the current POS
					   value (for caching) */
#define NUM_INSN_MT_PREAMBLE 27   /* number of instructions required for   */
                                  /* the MT preamble.                      */ 

#define RECURSIVE_GUARD_ON_CODE_SIZE   7
#define RECURSIVE_GUARD_OFF_CODE_SIZE  3

// NOTE: LOW() and HIGH() can return ugly values if x is negative, because in
// that case, 2's complement has really changed the bitwise representation!
// Perhaps an assert should be done!
#define LOW10(x) ((x) & 0x3ff)
#define LOW13(x) ((x) & 0x1fff)
#define HIGH22(x) ((x) >> 10)

//#define MAX_BRANCH	(0x1<<23)

#define MAX_IMM13       (4095)
#define MIN_IMM13       (-4096)

#define REG_SPTR          14
#define REG_FPTR          30

extern Register deadList[];
class AstNode;

bool processOptimaRet(instPoint *location, AstNode *&ast);

extern bool isPowerOf2(int value, int &result);
extern void generateNoOp(process *proc, Address addr);
extern void changeBranch(process *proc, Address fromAddr, Address newAddr,
		  instruction originalBranch);

extern void generateBranch(process *proc, Address fromAddr, Address newAddr);
extern void generateCall(process *proc, Address fromAddr, Address newAddr);
extern void generateBranchOrCallNoSaveRestore(process *proc,Address fromAddr, Address toAddr);
extern void genImm(process *proc, Address fromAddr, int op, Register rs1, 
		   int immd, Register rd);

extern int getInsnCost(opCode op);
extern bool isReturnInsn(const image *owner, Address adr, bool &lastOne, 
			 std::string name); 
extern bool isReturnInsn(instruction i, Address adr, std::string name);
extern bool isBranchInsn(instruction i);
extern bool branchInsideRange(instruction i, Address branchAddress, 
      Address firstAddress, Address lastAddress); 
extern bool trueCallInsideRange(instruction instr, Address callAddress, 
      Address firstAddress, Address lastAddress);

#endif
