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

// $Id: opcode.h,v 1.2 2006/11/09 17:16:22 bernat Exp $

#ifndef OPCODE_H
#define OPCODE_H

typedef enum { 
   invalidOp,
   plusOp,
   minusOp,
   timesOp,
   divOp,
   lessOp,
   leOp,
   greaterOp,
   geOp,
   eqOp,
   neOp,
   loadOp,           
   loadConstOp,
   loadFrameRelativeOp,
   loadFrameAddr,
   loadRegRelativeOp,	// More general form of loadFrameRelativeOp
   loadRegRelativeAddr,	// More general form of loadFrameAddr
   storeOp,
   storeFrameRelativeOp,
   ifOp,
   whileOp,  // Simple control structures will be useful
   doOp,     // Zhichen
   callOp,
   trampPreamble,
   noOp,
   orOp,
   andOp,
   getRetValOp,
   getRetAddrOp,
   getSysRetValOp,
   getParamOp,
   getParamAtCallOp,
   getParamAtEntryOp,
   getSysParamOp,	   
   getAddrOp,	// return the address of the operand
   loadIndirOp,
   storeIndirOp,
   saveRegOp,
   loadRegOp,
   saveStateOp,  // For saving of non-register state (flags reg, condition reg)
   loadStateOp,  // And the corresponding load
   updateCostOp,
   funcJumpOp,        // Jump to function without linkage
   funcCallOp,        // Call to function with linkage
   branchOp,
   ifMCOp,
   breakOp,
   xorOp,
   undefOp
} opCode;



#endif
