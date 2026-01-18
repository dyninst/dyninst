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

#ifndef DYNINST_DYNINSTAPI_ASTNODE_H
#define DYNINST_DYNINSTAPI_ASTNODE_H

#include "BPatch_type.h"
#include "dyn_register.h"
#include "dyntypes.h"
#include "opcode.h"
#include "Snippet.h"

#include <boost/shared_ptr.hpp>
#include <string>
#include <vector>

class AddressSpace;
class instPoint;
class func_instance;
class int_variable;
class codeGen;
class image_variable;

class AstNode;
typedef boost::shared_ptr<AstNode> AstNodePtr;

class registerSpace;

class AstNode : public Dyninst::PatchAPI::Snippet {
 public:
   enum class operandType { Constant,
                      ConstantString,
                      DataReg,
                      DataIndir,
                      Param,
                      ParamAtCall,
                      ParamAtEntry,
                      ReturnVal,
                      ReturnAddr, // address of a return instruction
                      DataAddr,  // Used to represent a variable in memory
                      FrameAddr, // Calculate FP
                      RegOffset, // Calculate *reg + offset; oValue is reg, loperand->oValue is offset.
                      //PreviousStackFrameDataReg,
                      //RegValue, // A possibly spilled, possibly saved register.
                      // Both the above are now: origRegister
                      origRegister,
                      variableAddr,
                      variableValue,
                      undefOperandType,
                      // Specific to AMDGPU. This represents an address in the form of (PlaceholderReg + offset).
                      // Codegen may assing the same or a different register in different contexts.
                      // Offset must be a constant.
                      AddressAsPlaceholderRegAndOffset
                      };



   enum memoryType {
      EffectiveAddr,
      BytesAccessed };

   enum MSpecialType{
       GENERIC_AST,
       CANARY_AST
   };

  public:
   virtual std::string format(std::string indent);
   std::string convert(operandType type);
   std::string convert(opCode op);

   AstNode() = default;

   virtual ~AstNode() = default;

   // Factory methods....
   static AstNodePtr nullNode();

   static AstNodePtr stackInsertNode(int size, MSpecialType type = GENERIC_AST);
   static AstNodePtr stackRemoveNode(int size, MSpecialType type);
   static AstNodePtr stackRemoveNode(int size, MSpecialType type, func_instance* func, bool canaryAfterPrologue, long canaryHeight);
   static AstNodePtr stackGenericNode();
   bool allocateCanaryRegister(codeGen& gen, bool noCost, Dyninst::Register& reg, bool& needSaveAndRestore);

   static AstNodePtr operandNode(operandType ot, void *arg);
   static AstNodePtr operandNode(operandType ot, AstNodePtr ast);
   static AstNodePtr operandNode(operandType ot, const image_variable* iv);

   static AstNodePtr memoryNode(memoryType ot, int which, int size = 8);

   static AstNodePtr sequenceNode(std::vector<AstNodePtr > &sequence);

   static AstNodePtr variableNode(std::vector<AstNodePtr>&ast_wrappers_, std::vector<std::pair<Dyninst::Offset, Dyninst::Offset> > *ranges = NULL);

   static AstNodePtr operatorNode(opCode ot,
                                  AstNodePtr l = AstNodePtr(),
                                  AstNodePtr r = AstNodePtr(),
                                  AstNodePtr e = AstNodePtr());

   static AstNodePtr atomicOperationStmtNode(opCode astOpcode, AstNodePtr variable,
                                             AstNodePtr constant);

   static AstNodePtr funcCallNode(const std::string &func, std::vector<AstNodePtr > &args, AddressSpace *addrSpace = NULL);
   static AstNodePtr funcCallNode(func_instance *func, std::vector<AstNodePtr > &args);
   static AstNodePtr funcCallNode(func_instance *func); // Special case for function call replacement.
   static AstNodePtr funcCallNode(Dyninst::Address addr, std::vector<AstNodePtr > &args); // For when you absolutely need
   // to jump somewhere.

    // Acquire the thread index value - a 0...n labelling of threads.
   static AstNodePtr threadIndexNode();

   static AstNodePtr scrambleRegistersNode();

   // TODO...
   // Needs some way of marking what to save and restore... should be a registerSpace, really

   static AstNodePtr originalAddrNode();
   static AstNodePtr actualAddrNode();
   static AstNodePtr dynamicTargetNode();

   static AstNodePtr snippetNode(Dyninst::PatchAPI::SnippetPtr snip);

   virtual bool generateCode(codeGen &gen,
                             bool noCost,
                             Dyninst::Address &retAddr,
                             Dyninst::Register &retReg);

   // Can't use default references....
   virtual bool generateCode(codeGen &gen,
                             bool noCost);

   // Can't use default references....
   virtual bool generateCode(codeGen &gen,
                             bool noCost,
                             Dyninst::Register &retReg) {
      Dyninst::Address unused = Dyninst::ADDR_NULL;
      return generateCode(gen, noCost,  unused, retReg);
   }

   // I don't know if there is an overload between address and register...
   // so we'll toss in two different return types.
   virtual bool generateCode_phase2(codeGen &gen,
                                    bool noCost,
                                    Dyninst::Address &retAddr,
                                    Dyninst::Register &retReg) = 0;

   // Perform whatever pre-processing steps are necessary.
   virtual bool initRegisters(codeGen &gen);

   // Select the appropriate Variable AST as part of pre-processing
   // steps before code generation.
   virtual void setVariableAST(codeGen &g) {
     for(auto &&c : children) {
       c->setVariableAST(g);
     }
   }

   bool decRefCount();

   bool previousComputationValid(Dyninst::Register &reg,
                                 codeGen &gen);

   virtual AstNodePtr operand() const { return AstNodePtr(); }



   virtual bool containsFuncCall() const {
     for(auto &&c : children) {
       if(c->containsFuncCall()) {
         return true;
       }
     }
     return false;
   }

   virtual bool usesAppRegister() const {
     for(auto &&c : children) {
       if(c->usesAppRegister()) {
         return true;
       }
     }
     return false;
   }

   int referenceCount{};     // Reference count for freeing memory
   int useCount{};           // Reference count for generating code
   void setUseCount(); // Set values for useCount
   int getSize() { return size; }
   void cleanUseCount();

  // Occasionally, we do not call .generateCode_phase2 for the
  // referenced node, but generate code by hand. This routine decrements
  // its use count properly
  void decUseCount(codeGen &gen);

  // Our children may have incorrect useCounts (most likely they
  // assume that we will not bother them again, which is wrong)
  void fixChildrenCounts();

  // Check if the node can be kept at all. Some nodes (e.g., storeOp)
  // can not be cached
  virtual bool canBeKept() const { return false; }

  // Allocate a register and make it available for sharing if our
   // node is shared
  Dyninst::Register allocateAndKeep(codeGen &gen, bool noCost);

  // Return all children of this node ([lre]operand, ..., operands[])
  std::vector<AstNodePtr> const& getChildren() const { return children; }

  virtual void setOValue(void *) { assert(0); }
  virtual const void *getOValue() const { assert(0); return NULL; }
  virtual const image_variable* getOVar() const {
      return NULL;
  }

  virtual void emitVariableStore(opCode, Dyninst::Register, Dyninst::Register, codeGen&,
                                  bool, registerSpace*,
                                  int, const instPoint*, AddressSpace*)
  {
      assert(!"Never call this on anything but an operand");
  }
  virtual void emitVariableLoad(opCode, Dyninst::Register, Dyninst::Register, codeGen&,
                                 bool, registerSpace*,
                                 int, const instPoint*, AddressSpace*)
  {
      assert(!"Never call this on anything but an operand");
  }

  // DEBUG
   virtual operandType getoType() const { return operandType::undefOperandType; }

   virtual void setConstFunc(bool) {}

 protected:
  BPatch_type *bptype{};  // type of corresponding BPatch_snippet
  bool doTypeCheck{true};     // should operands be type checked
  int size{};       // size of the operations (in bytes)
  std::vector<AstNodePtr> children;


 public:
  // Functions for getting and setting type decoration used by the
  // dyninst API library
  BPatch_type *getType();
  void      setType(BPatch_type *t);
  void      setTypeChecking(bool x) { doTypeCheck = x; }
  virtual BPatch_type   *checkType(BPatch_function* func = NULL);


        // PatchAPI compatibility
        virtual bool generate(Dyninst::PatchAPI::Point *,
                              Dyninst::Buffer &);

 private:
   static AstNodePtr originalAddrNode_;
   static AstNodePtr actualAddrNode_;
   static AstNodePtr dynamicTargetNode_;
};







#endif
