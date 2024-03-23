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

func_instance *getFunction(instPoint *point);

#define FUNC_ENTRY      0x1
#define FUNC_EXIT       0x2
#define FUNC_CALL       0x4
#define FUNC_ARG  	0x8

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

      instMapping(const instMapping *parMapping, AddressSpace *child);

  ~instMapping() {
  }

public:
  void dontUseTrampGuard() { useTrampGuard = false; }
  void markAs_MTonly() { mt_only = true; }
  void canUseTrap(bool t) { allow_trap = t; }
  bool is_MTonly() { return mt_only; }

  std::string func;
  std::string inst;
  std::string lib;
  int where;
  callWhen when;
  callOrder order;
  std::vector<AstNodePtr> args;
  bool useTrampGuard;
  bool mt_only;
  bool allow_trap;
  std::vector<Dyninst::PatchAPI::InstancePtr> instances;
};


void initPrimitiveCost();
void initDefaultPointFrequencyTable();

unsigned getPrimitiveCost(const std::string &name);

typedef enum gnenum {
   rc_before_jump,
   rc_after_jump,
   rc_no_control
} RegControl;

codeBufIndex_t emitA(opCode op, Dyninst::Register src1, Dyninst::Register src2, long dst,
                     codeGen &gen, RegControl rc, bool noCost);

Dyninst::Register emitR(opCode op, Dyninst::Register src1, Dyninst::Register src2, Dyninst::Register dst,
               codeGen &gen, bool noCost, 
               const instPoint *location, bool for_multithreaded);

void     emitV(opCode op, Dyninst::Register src1, Dyninst::Register src2, Dyninst::Register dst,
               codeGen &gen, bool noCost, 
               registerSpace *rs = NULL, int size = 4,
               const instPoint * location = NULL, AddressSpace * proc = NULL, bool s = true);

void     emitVload(opCode op, Dyninst::Address src1, Dyninst::Register src2, Dyninst::Register dst,
                   codeGen &gen, bool noCost, 
                   registerSpace *rs = NULL, int size = 4, 
                   const instPoint * location = NULL, AddressSpace * proc = NULL);

void     emitVstore(opCode op, Dyninst::Register src1, Dyninst::Register src2, Dyninst::Address dst,
                    codeGen &gen, bool noCost, 
                    registerSpace *rs = NULL, int size = 4, 
                    const instPoint * location = NULL, AddressSpace * proc = NULL);

void     emitVload(opCode op, const image_variable* src1, Dyninst::Register src2, Dyninst::Register dst,
                   codeGen &gen, bool noCost, 
                   registerSpace *rs = NULL, int size = 4, 
                   const instPoint * location = NULL, AddressSpace * proc = NULL);

void     emitVstore(opCode op, Dyninst::Register src1, Dyninst::Register src2, const image_variable* dst,
                    codeGen &gen, bool noCost, 
                    registerSpace *rs = NULL, int size = 4, 
                    const instPoint * location = NULL, AddressSpace * proc = NULL);

void     emitImm(opCode op, Dyninst::Register src, Dyninst::RegValue src2imm, Dyninst::Register dst,
                 codeGen &gen, bool noCost,
                 registerSpace *rs = NULL, bool s = true);


//#include "dyninstAPI/h/BPatch_memoryAccess_NP.h"
class BPatch_addrSpec_NP;
typedef BPatch_addrSpec_NP BPatch_countSpec_NP;

void emitJmpMC(int condition, int offset, codeGen &gen);

void emitASload(const BPatch_addrSpec_NP *as, Dyninst::Register dest, int stackShift, codeGen &gen, bool noCost);

void emitCSload(const BPatch_countSpec_NP *as, Dyninst::Register dest, codeGen &gen, bool noCost);

Dyninst::Register emitFuncCall(opCode op, codeGen &gen,
                      std::vector<AstNodePtr> &operands,
					  bool noCost, 
                      func_instance *func);

Dyninst::Register emitFuncCall(opCode op, codeGen &gen,
                      std::vector<AstNodePtr> &operands, 
		      		  bool noCost, 
                      Dyninst::Address callee_addr_);

int getInsnCost(opCode t);

Dyninst::Register getParameter(Dyninst::Register dest, int param);

extern std::string getProcessStatus(const AddressSpace *p);

extern unsigned findTags(const std::string funcName);

extern Dyninst::Address getMaxBranch();

// extern std::unordered_map<std::string, unsigned> tagDict;
extern std::map<std::string, unsigned> primitiveCosts; 

bool writeFunctionPtr(AddressSpace *p, Dyninst::Address addr, func_instance *f);


bool emitStoreConst(Dyninst::Address addr, int imm, codeGen &gen, bool noCost);
bool emitAddSignedImm(Dyninst::Address addr, long int imm, codeGen &gen, bool noCost);
bool emitSubSignedImm(Dyninst::Address addr, long int imm, codeGen &gen, bool noCost);

#endif
