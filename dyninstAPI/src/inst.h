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

// $Id: inst.h,v 1.104 2008/09/15 18:37:49 jaw Exp $

#ifndef INST_HDR
#define INST_HDR

#include "codeGenAST.h" // codeGenASTPtr
#include <map>
#include <vector>
#include <unordered_map>
#include "dyn_register.h"
#include "codegen.h" // codeBufIndex_t 
#include "codegen/RegControl.h"

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

class instPoint;
class baseTramp;
class func_instance;
class metricFocusNode;
class codeGen;
class registerSpace;
class AddressSpace;
class image_variable;

/*
 * Generate an instruction.
 * Previously this was handled by the polymorphic "emit" function, which
 * took a variety of argument types and variously returned either an
 * Dyninst::Address or a Dyninst::Register or nothing of value.  The following family of
 * functions replace "emit" with more strongly typed versions.
 */

// The return value is a magic "hand this in when we update" black box;
// emitA handles emission of things like ifs that need to be updated later.
codeBufIndex_t emitA(opCode op, Dyninst::Register src1, long dst,
                     codeGen &gen, Dyninst::DyninstAPI::RegControl rc);

// for operations requiring a Dyninst::Register to be returned
// (e.g., getRetValOp, getRetAddrOp, getParamOp)
Dyninst::Register emitR(opCode op, Dyninst::Register src1, Dyninst::Register src2, Dyninst::Register dst,
               codeGen &gen,
               const instPoint *location);

// for general arithmetic and logic operations which return nothing
void     emitV(opCode op, Dyninst::Register src1, Dyninst::Register src2, Dyninst::Register dst,
               codeGen &gen,
               int size = 4,
               AddressSpace * proc = NULL, bool s = true);

// for loadOp and loadConstOp (reading from an Dyninst::Address)
void     emitVload(opCode op, Dyninst::Address src1, Dyninst::Register src2, Dyninst::Register dst,
                   codeGen &gen,
                   int size = 4,
                   AddressSpace * proc = NULL);

// for storeOp (writing to an Dyninst::Address)
void     emitVstore(opCode op, Dyninst::Register src1, Dyninst::Register src2, Dyninst::Address dst,
                    codeGen &gen,
                    int size = 4,
                    AddressSpace * proc = NULL);

// and the retyped original emitImm companion
void     emitImm(opCode op, Dyninst::Register src, Dyninst::RegValue src2imm, Dyninst::Register dst,
                 codeGen &gen,
                 bool s = true);


//#include "dyninstAPI/h/BPatch_memoryAccess_NP.h"
class BPatch_addrSpec_NP;
typedef BPatch_addrSpec_NP BPatch_countSpec_NP;

// Don't need the above: countSpec is typedefed to addrSpec

void emitJmpMC(int condition, int offset, codeGen &gen);

void emitASload(const BPatch_addrSpec_NP *as, Dyninst::Register dest, int stackShift, codeGen &gen);

void emitCSload(const BPatch_countSpec_NP *as, Dyninst::Register dest, codeGen &gen);

// VG(11/06/01): moved here and added location
Dyninst::Register emitFuncCall(opCode op, codeGen &gen,
                      std::vector<Dyninst::DyninstAPI::codeGenASTPtr> &operands,
                      func_instance *func);

// find these internal functions before finding any other functions
// extern std::unordered_map<std::string, unsigned> tagDict;

bool writeFunctionPtr(AddressSpace *p, Dyninst::Address addr, func_instance *f);

/**
 * A set of optimized emiters for common idioms.  Return 
 * false if the platform can't perform any optimizations.
 **/
//Store constant in memory at address
bool emitStoreConst(Dyninst::Address addr, int imm, codeGen &gen);
//Add constant to memory at address
bool emitAddSignedImm(Dyninst::Address addr, long int imm, codeGen &gen);
//Subtract constant from memory at address
bool emitSubSignedImm(Dyninst::Address addr, long int imm, codeGen &gen);

inline bool isPowerOf2(int value, int &result) {
  if(value <= 0) {
    return (false);
  }
  if(value == 1) {
    result = 0;
    return (true);
  }
  if((value % 2) != 0) {
    return (false);
  }
  if(isPowerOf2(value / 2, result)) {
    result++;
    return (true);
  } else {
    return (false);
  }
}

#endif
