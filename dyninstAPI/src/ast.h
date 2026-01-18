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

// $Id: ast.h,v 1.107 2008/05/12 22:12:47 giri Exp $

#ifndef AST_HDR
#define AST_HDR

//
// Define a AST class for use in generating primitive and pred calls
//

#include "AstNode.h"

#include <assert.h>
#include <utility>
#include <vector>
#include <stdio.h>
#include <string>
#include <unordered_map>

#include "dyn_register.h"
#include "Point.h"

#include "BPatch_snippet.h"

// The great experiment: boost shared_ptr libraries
#include "BPatch_type.h"

class AddressSpace;
class instPoint;
class func_instance;
class int_variable;
class codeGen;
class image_variable;





#include "opcode.h"

// Dyninst::Register retention mechanism...
// If we've already calculated a result, then we want to reuse it if it's
// still available. This means it was calculated along a path that reaches the
// current point (not inside a conditional) and the register hasn't been
// reused. We handle this so:
//
// 1) Iterate over the AST tree and see if any node is reached more than once;
// if so, mark it as potentially being worth keeping around. We can do this
// because we use pointers; a better approach would be a comparison operator.
// 2) Start generation at "level 0". 
// 3) When a conditional AST is reached, generate each child at level+1.
// 4) When the AST is reached during code generation, and doesn't have a register:
// 4a) Allocate a register for it;
// 4b) Enter that register, the AST node, and the current level in the global table.
// 5) If it does have a register, reuse it. 
// 6) When the conditionally executed branch is finished, clean all entries in
// the table with that level value (undoing all kept registers along that
// path).
// 7) If we need a register, the register allocator (registerSpace) can forcibly
// undo this optimization and grab a register. Grab the register from the AstNode
// with the lowest usage count.

class AstNullNode : public AstNode {
 public:

    AstNullNode() : AstNode() {}

   virtual std::string format(std::string indent);
    
    bool canBeKept() const { return true; }
 private:
    virtual bool generateCode_phase2(codeGen &gen,
                                     bool noCost,
                                     Dyninst::Address &retAddr,
                                     Dyninst::Register &retReg);
};

/* Stack Frame Modification */
class AstStackInsertNode : public AstNode {
    public:
        AstStackInsertNode(int s, MSpecialType t) : AstNode(),
        size(s),
        type(t) {}

        virtual std::string format(std::string indent);

        bool canBeKept() const { return true; }

    private:
    virtual bool generateCode_phase2(codeGen &gen,
                                     bool noCost,
                                     Dyninst::Address &retAddr,
                                     Dyninst::Register &retReg);

    int size;
    MSpecialType type;
};

class AstStackRemoveNode : public AstNode {
    public:
        AstStackRemoveNode(int s, MSpecialType t = GENERIC_AST) : AstNode(),
        size(s),
        type(t) {}

        AstStackRemoveNode(int s, MSpecialType t, func_instance* func, bool canaryAfterPrologue, long canaryHeight) :
            AstNode(),
            size(s),
            type(t),
            func_(func),
            canaryAfterPrologue_(canaryAfterPrologue),
            canaryHeight_(canaryHeight)
    {}

        virtual std::string format(std::string indent);

        bool canBeKept() const { return true; }

    private:
    virtual bool generateCode_phase2(codeGen &gen,
                                     bool noCost,
                                     Dyninst::Address &retAddr,
                                     Dyninst::Register &retReg);

    int size;
    MSpecialType type;

    func_instance* func_{};
    bool canaryAfterPrologue_{};
    long canaryHeight_{};
};

class AstStackGenericNode : public AstNode {
    public: AstStackGenericNode() : AstNode() {}
            virtual std::string format(std::string indent);

            bool canBeKept() const { return true; }
    private:
            virtual bool generateCode_phase2(codeGen &gen,
                    bool noCost,
                    Dyninst::Address &retAddr,
                    Dyninst::Register &retReg);
};

class AstOperatorNode : public AstNode {
 public:

    AstOperatorNode(opCode opC, AstNodePtr l, AstNodePtr r = AstNodePtr(), AstNodePtr e = AstNodePtr());

   virtual std::string format(std::string indent);

    virtual BPatch_type	  *checkType(BPatch_function* func = NULL);

    virtual bool canBeKept() const;
 

    // We override initRegisters in the case of writing to an original register.
    virtual bool initRegisters(codeGen &gen);

 private:

    virtual bool generateCode_phase2(codeGen &gen,
                                     bool noCost,
                                     Dyninst::Address &retAddr,
                                     Dyninst::Register &retReg);

    bool generateOptimizedAssignment(codeGen &gen, int size, bool noCost);

    opCode op{};
    AstNodePtr loperand;
    AstNodePtr roperand;
    AstNodePtr eoperand;
};


class AstOperandNode : public AstNode {
    friend class AstOperatorNode; // ARGH
 public:

    // Direct operand
    AstOperandNode(operandType ot, void *arg);

    // And an indirect (say, a load)
    AstOperandNode(operandType ot, AstNodePtr l);

    AstOperandNode(operandType ot, const image_variable* iv);
    
    ~AstOperandNode() {
        if (oType == operandType::ConstantString) free((char *)oValue);
    }
        
    // Arguably, the previous should be an operation...
    // however, they're kinda endemic.

   virtual std::string format(std::string indent);

    virtual operandType getoType() const { return oType; }

    virtual void setOValue(void *o) { oValue = o; }
    virtual const void *getOValue() const { return oValue; }
    virtual const image_variable* getOVar() const 
    {
      return oVar;
    }
    

    virtual AstNodePtr operand() const { return operand_; }

    virtual BPatch_type	  *checkType(BPatch_function* func = NULL);

    virtual bool canBeKept() const;

    virtual bool usesAppRegister() const;
 
    virtual void emitVariableStore(opCode op, Dyninst::Register src1, Dyninst::Register src2, codeGen& gen,
			   bool noCost, registerSpace* rs, 
			   int size, const instPoint* point, AddressSpace* as);
    virtual void emitVariableLoad(opCode op, Dyninst::Register src2, Dyninst::Register dest, codeGen& gen,
			  bool noCost, registerSpace* rs, 
			  int size, const instPoint* point, AddressSpace* as);

    virtual bool initRegisters(codeGen &gen);
#if defined(DYNINST_CODEGEN_ARCH_AMDGPU_GFX908)
   static int lastOffset; // Last ofsfet in our GPU memory buffer.
   static std::map<std::string, int> allocTable;
   static void addToTable(const std::string &variableName, int size) {
      // We shouldn't allocate more than once
      assert(allocTable.find(variableName) == allocTable.end() && "Can't allocate variable twice");
      assert(size >0);
      allocTable[variableName] = lastOffset;
      std::cerr << "inserted " << variableName << " of " << size << " bytes at " << lastOffset << "\n";
      lastOffset += size;
   }

   static int getOffset(const std::string &variableName) {
      assert(allocTable.find(variableName) != allocTable.end() && "Variable must be allocated");
      return allocTable[variableName];
   }
#endif
   
 private:
    virtual bool generateCode_phase2(codeGen &gen,
                                     bool noCost,
                                     Dyninst::Address &retAddr,
                                     Dyninst::Register &retReg);
    int_variable* lookUpVar(AddressSpace* as);
    
    AstOperandNode(): oType(operandType::undefOperandType), oValue(NULL), oVar(NULL) {}

    operandType oType;
    void *oValue;
    const image_variable* oVar;
    AstNodePtr operand_;
};


class AstCallNode : public AstNode {
 public:

    AstCallNode(func_instance *func, std::vector<AstNodePtr>&args);
    AstCallNode(const std::string &str, std::vector<AstNodePtr>&args);
    AstCallNode(Dyninst::Address addr, std::vector<AstNodePtr> &args);
    AstCallNode(func_instance *func);
    
    ~AstCallNode() {}

   virtual std::string format(std::string indent);
        
    virtual BPatch_type	  *checkType(BPatch_function* func = NULL);
    virtual bool canBeKept() const;
    virtual bool containsFuncCall() const { return true; }
 
    void setConstFunc(bool val) { constFunc_ = val; }

    virtual bool initRegisters(codeGen &gen);

 private:
    virtual bool generateCode_phase2(codeGen &gen,
                                     bool noCost,
                                     Dyninst::Address &retAddr,
                                     Dyninst::Register &retReg);

    AstCallNode(): func_addr_(0), func_(NULL), callReplace_(false), constFunc_(false) {}
    // Sometimes we just don't have enough information...
    const std::string func_name_;
    Dyninst::Address func_addr_;
    
    func_instance *func_;

    bool callReplace_; // Node is intended for function call replacement
    bool constFunc_;  // True if the output depends solely on 
    // input parameters, or can otherwise be guaranteed to not change
    // if executed multiple times in the same sequence - AKA 
    // "can be kept".
};


class AstSequenceNode : public AstNode {
 public:
    AstSequenceNode(std::vector<AstNodePtr> &sequence);

    ~AstSequenceNode() {}

   virtual std::string format(std::string indent);

    virtual BPatch_type	  *checkType(BPatch_function* func = NULL);

    virtual bool canBeKept() const {
      // Theoretically we could keep the entire thing, but... not sure
      // that's a terrific idea. For now, don't keep a sequence node around.
        return false;
    }
    
 

 private:
    virtual bool generateCode_phase2(codeGen &gen,
                                     bool noCost,
                                     Dyninst::Address &retAddr,
                                     Dyninst::Register &retReg);

    AstSequenceNode() {}
};

class AstVariableNode : public AstNode {
  public:
    AstVariableNode(std::vector<AstNodePtr>&ast_wrappers, std::vector<std::pair<Dyninst::Offset, Dyninst::Offset> >*ranges);

    ~AstVariableNode() {}

    virtual std::string format(std::string indent);

    virtual BPatch_type	  *checkType(BPatch_function* = NULL) { return getType(); }

    virtual bool canBeKept() const {
      return children[index]->canBeKept();
    }

    virtual operandType getoType() const { return children[index]->getoType(); }
    virtual AstNodePtr operand() const { return children[index]->operand(); }
    virtual const void *getOValue() const { return children[index]->getOValue(); }

    virtual void setVariableAST(codeGen &gen);
    
    virtual bool containsFuncCall() const {
      return children[index]->containsFuncCall();
    }
    virtual bool usesAppRegister() const {
      return children[index]->usesAppRegister();
    }
 

 private:
    virtual bool generateCode_phase2(codeGen &gen,
                                     bool noCost,
                                     Dyninst::Address &retAddr,
                                     Dyninst::Register &retReg);

    AstVariableNode(): ranges_(NULL), index(0) {}
    std::vector<std::pair<Dyninst::Offset, Dyninst::Offset> > *ranges_;
    unsigned index;

};

class AstMemoryNode : public AstNode {
 public:
    AstMemoryNode(memoryType mem, unsigned which, int size);
    bool canBeKept() const {
      // Despite our memory loads, we can be kept;
      // we're loading off process state, which is defined
      // to be invariant during the instrumentation phase.
      return true;
    }

   virtual std::string format(std::string indent);
   virtual bool usesAppRegister() const { return true; }
 

 private:
    virtual bool generateCode_phase2(codeGen &gen,
                                     bool noCost,
                                     Dyninst::Address &retAddr,
                                     Dyninst::Register &retReg);
    
    AstMemoryNode() {}
    memoryType mem_{};
    unsigned which_{};
};

class AstOriginalAddrNode : public AstNode {
 public:
    AstOriginalAddrNode() {}

    virtual ~AstOriginalAddrNode() {}


    virtual BPatch_type *checkType(BPatch_function*  = NULL) { return getType(); }
    virtual bool canBeKept() const { return true; }
 

 private:
    virtual bool generateCode_phase2(codeGen &gen,
                                     bool noCost,
                                     Dyninst::Address &retAddr,
                                     Dyninst::Register &retReg);
};

class AstActualAddrNode : public AstNode {
 public:
    AstActualAddrNode() {}

    virtual ~AstActualAddrNode() {}


    virtual BPatch_type *checkType(BPatch_function*  = NULL) { return getType(); }
 
 private:
    virtual bool generateCode_phase2(codeGen &gen,
                                     bool noCost,
                                     Dyninst::Address &retAddr,
                                     Dyninst::Register &retReg);
};

class AstDynamicTargetNode : public AstNode {
 public:
    AstDynamicTargetNode() {}

    virtual ~AstDynamicTargetNode() {}


    virtual BPatch_type *checkType(BPatch_function*  = NULL) { return getType(); }
 
 private:
    virtual bool generateCode_phase2(codeGen &gen,
                                     bool noCost,
                                     Dyninst::Address &retAddr,
                                     Dyninst::Register &retReg);
};
class AstScrambleRegistersNode : public AstNode {
 public:
    AstScrambleRegistersNode() {}

    virtual ~AstScrambleRegistersNode() {}

    virtual bool usesAppRegister() const { return true; }
 
 private:
    virtual bool generateCode_phase2(codeGen &gen,
                                     bool noCost,
                                     Dyninst::Address &retAddr,
                                     Dyninst::Register &retReg);
};

class AstSnippetNode : public AstNode {
   // This is a little odd, since an AstNode _is_
   // a Snippet. It's a compatibility interface to 
   // allow generic PatchAPI snippets to play nice
   // in our world. 
  public:
  AstSnippetNode(Dyninst::PatchAPI::SnippetPtr snip) : snip_(snip) {}
   
  private:
    virtual bool generateCode_phase2(codeGen &gen,
                                     bool noCost,
                                     Dyninst::Address &retAddr,
                                     Dyninst::Register &retReg);
    Dyninst::PatchAPI::SnippetPtr snip_;
};

class AstAtomicOperationStmtNode : public AstNode {
    // This corresponds to a single statement, and not an expression that can be nested among other
    // expressions.
  public:
    AstAtomicOperationStmtNode(opCode astOpcode, AstNodePtr variableNode, AstNodePtr constantNode);

    virtual std::string format(std::string indent);

    virtual bool canBeKept() const { return true; }

  private:
    virtual bool generateCode_phase2(codeGen &gen, bool noCost, Dyninst::Address &retAddr,
                                     Dyninst::Register &retReg);
    opCode opcode;
    AstNodePtr variable;
    AstNodePtr constant;
};

void emitLoadPreviousStackFrameRegister(Dyninst::Address register_num,
					Dyninst::Register dest,
                                        codeGen &gen,
					int size,
					bool noCost);

#define SCAST_AST(ast) boost::static_pointer_cast<AstNode>(ast)
#define DCAST_AST(ast) boost::dynamic_pointer_cast<AstNode>(ast)


#endif /* AST_HDR */
