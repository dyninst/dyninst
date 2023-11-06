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

#include <string>
#include <map>
#include <vector>
#include <unordered_map>
#include "opcode.h" // enum opCode now defined here.
#include "dyn_register.h"
#include "codegen.h" // codeBufIndex_t 
#include "dyninstAPI/src/ast.h" // astNodePtr

namespace Dyninst {
   namespace PatchAPI {
      class Instance;
      typedef boost::shared_ptr<Instance> InstancePtr;
   }
}

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

typedef enum { callNoArgs, callRecordType, callFullArgs } callOptions;
typedef enum { callPreInsn, callPostInsn, callBranchTargetInsn, callUnset } callWhen;
typedef enum { orderFirstAtPoint, orderLastAtPoint } callOrder;

class AstNode;

/* Utility functions */


/* return the function asociated with a point. */
func_instance *getFunction(instPoint *point);

/*
 * struct to define a list of inst requests 
 *
 */
#define FUNC_ENTRY      0x1             /* entry to the function */
#define FUNC_EXIT       0x2             /* exit from function */
#define FUNC_CALL       0x4             /* subroutines called from func */
#define FUNC_ARG  	0x8             /* use arg as argument */

// Container class for "instrument this point with this function". 
// What I want to know is who is allergic to multi-letter arguments? Yeesh.
class instMapping {

   public:

      instMapping(const std::string f, const std::string i, const int w, 
            callWhen wn, callOrder o, AstNodePtr a = AstNodePtr(), std::string l = "")
         : func(f), inst(i), lib(l),
         where(w), when(wn), order(o), useTrampGuard(true),
         mt_only(false), allow_trap(false) {
            if (a != AstNodePtr()) args.push_back(a);
         }

      instMapping(const std::string f, const std::string i, const int w, 
            AstNodePtr a = AstNodePtr(), std::string l = "")
         : func(f), inst(i), lib(l),
         where(w), when(callPreInsn), order(orderLastAtPoint),
         useTrampGuard(true), mt_only(false), allow_trap(false) {
            if (a != AstNodePtr()) args.push_back(a);
         }

      instMapping(const std::string f, const std::string i, const int w, 
            std::vector<AstNodePtr> &aList, std::string l = "") :
         func(f), inst(i), lib(l),
         where(w), when(callPreInsn), order(orderLastAtPoint),
         useTrampGuard(true), mt_only(false), allow_trap(false) {
            for(unsigned u=0; u < aList.size(); u++) {
               if (aList[u] != AstNodePtr()) args.push_back(aList[u]);
            }
         }

      // Fork
      instMapping(const instMapping *parMapping, AddressSpace *child);

  ~instMapping() {
  }

public:
  void dontUseTrampGuard() { useTrampGuard = false; }
  void markAs_MTonly() { mt_only = true; }
  void canUseTrap(bool t) { allow_trap = t; }
  bool is_MTonly() { return mt_only; }

  std::string func;                 /* function to instrument */
  std::string inst;                 /* inst. function to place at func */
  std::string lib;                  /* library name */
  int where;                   /* FUNC_ENTRY, FUNC_EXIT, FUNC_CALL */
  callWhen when;               /* callPreInsn, callPostInsn */
  callOrder order;             /* orderFirstAtPoint, orderLastAtPoint */
  std::vector<AstNodePtr> args;      /* what to pass as arg0 ... n */
  bool useTrampGuard;
  bool mt_only;
  bool allow_trap;
  std::vector<Dyninst::PatchAPI::InstancePtr> instances;
};


/*
 * get information about the cost of primitives.
 *
 */
void initPrimitiveCost();
void initDefaultPointFrequencyTable();

//
// Return the expected runtime of the passed function in instruction times.
//
unsigned getPrimitiveCost(const std::string &name);

/*
 * Generate an instruction.
 * Previously this was handled by the polymorphic "emit" function, which
 * took a variety of argument types and variously returned either an
 * Dyninst::Address or a Dyninst::Register or nothing of value.  The following family of
 * functions replace "emit" with more strongly typed versions.
 */

typedef enum gnenum {
   rc_before_jump,
   rc_after_jump,
   rc_no_control
} RegControl;

// The return value is a magic "hand this in when we update" black box;
// emitA handles emission of things like ifs that need to be updated later.
codeBufIndex_t emitA(opCode op, Dyninst::Register src1, Dyninst::Register src2, long dst,
                     codeGen &gen, RegControl rc, bool noCost);

// for operations requiring a Dyninst::Register to be returned
// (e.g., getRetValOp, getRetAddrOp, getParamOp, getSysRetValOp, getSysParamOp)
Dyninst::Register emitR(opCode op, Dyninst::Register src1, Dyninst::Register src2, Dyninst::Register dst,
               codeGen &gen, bool noCost, 
               const instPoint *location, bool for_multithreaded);

// for general arithmetic and logic operations which return nothing
void     emitV(opCode op, Dyninst::Register src1, Dyninst::Register src2, Dyninst::Register dst,
               codeGen &gen, bool noCost, 
               registerSpace *rs = NULL, int size = 4,
               const instPoint * location = NULL, AddressSpace * proc = NULL, bool s = true);

// for loadOp and loadConstOp (reading from an Dyninst::Address)
void     emitVload(opCode op, Dyninst::Address src1, Dyninst::Register src2, Dyninst::Register dst,
                   codeGen &gen, bool noCost, 
                   registerSpace *rs = NULL, int size = 4, 
                   const instPoint * location = NULL, AddressSpace * proc = NULL);

// for storeOp (writing to an Dyninst::Address)
void     emitVstore(opCode op, Dyninst::Register src1, Dyninst::Register src2, Dyninst::Address dst,
                    codeGen &gen, bool noCost, 
                    registerSpace *rs = NULL, int size = 4, 
                    const instPoint * location = NULL, AddressSpace * proc = NULL);

// for loadOp and loadConstOp (reading from an Dyninst::Address)
void     emitVload(opCode op, const image_variable* src1, Dyninst::Register src2, Dyninst::Register dst,
                   codeGen &gen, bool noCost, 
                   registerSpace *rs = NULL, int size = 4, 
                   const instPoint * location = NULL, AddressSpace * proc = NULL);

// for storeOp (writing to an Dyninst::Address)
void     emitVstore(opCode op, Dyninst::Register src1, Dyninst::Register src2, const image_variable* dst,
                    codeGen &gen, bool noCost, 
                    registerSpace *rs = NULL, int size = 4, 
                    const instPoint * location = NULL, AddressSpace * proc = NULL);

// and the retyped original emitImm companion
void     emitImm(opCode op, Dyninst::Register src, Dyninst::RegValue src2imm, Dyninst::Register dst,
                 codeGen &gen, bool noCost,
                 registerSpace *rs = NULL, bool s = true);


//#include "dyninstAPI/h/BPatch_memoryAccess_NP.h"
class BPatch_addrSpec_NP;
typedef BPatch_addrSpec_NP BPatch_countSpec_NP;

// Don't need the above: countSpec is typedefed to addrSpec

void emitJmpMC(int condition, int offset, codeGen &gen);

void emitASload(const BPatch_addrSpec_NP *as, Dyninst::Register dest, int stackShift, codeGen &gen, bool noCost);

void emitCSload(const BPatch_countSpec_NP *as, Dyninst::Register dest, codeGen &gen, bool noCost);

// VG(11/06/01): moved here and added location
Dyninst::Register emitFuncCall(opCode op, codeGen &gen,
                      std::vector<AstNodePtr> &operands,
					  bool noCost, 
                      func_instance *func);

// Obsolete version that uses an address. DON'T USE THIS or expect it to survive.
Dyninst::Register emitFuncCall(opCode op, codeGen &gen,
                      std::vector<AstNodePtr> &operands, 
		      		  bool noCost, 
                      Dyninst::Address callee_addr_);

int getInsnCost(opCode t);

/*
 * get the requested parameter into a register.
 *
 */
Dyninst::Register getParameter(Dyninst::Register dest, int param);

extern std::string getProcessStatus(const AddressSpace *p);

// TODO - what about mangled names ?
// expects the symbol name advanced past the underscore
extern unsigned findTags(const std::string funcName);

extern Dyninst::Address getMaxBranch();

// find these internal functions before finding any other functions
// extern std::unordered_map<std::string, unsigned> tagDict;
extern std::map<std::string, unsigned> primitiveCosts; 

bool writeFunctionPtr(AddressSpace *p, Dyninst::Address addr, func_instance *f);

/**
 * A set of optimized emiters for common idioms.  Return 
 * false if the platform can't perform any optimizations.
 **/
//Store constant in memory at address
bool emitStoreConst(Dyninst::Address addr, int imm, codeGen &gen, bool noCost);
//Add constant to memory at address
bool emitAddSignedImm(Dyninst::Address addr, long int imm, codeGen &gen, bool noCost);
//Subtract constant from memory at address
bool emitSubSignedImm(Dyninst::Address addr, long int imm, codeGen &gen, bool noCost);

#endif
