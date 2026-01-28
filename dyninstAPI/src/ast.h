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

#ifndef DYNINST_DYNINSTAPI_AST_H
#define DYNINST_DYNINSTAPI_AST_H

#include "dyn_register.h"
#include "opcode.h"
#include "OperandType.h"
#include "Point.h"

#include <cassert>
#include <utility>
#include <vector>
#include <string>

class AddressSpace;
class BPatch_function;
class BPatch_snippet;
class BPatch_type;
class codeGen;
class instPoint;
class func_instance;
class image_variable;
class int_variable;


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

class AstNode;
typedef boost::shared_ptr<AstNode> AstNodePtr;

class registerSpace;

class AstNode : public Dyninst::PatchAPI::Snippet {
 public:
   enum memoryType {
      EffectiveAddr,
      BytesAccessed };

   enum MSpecialType{
       GENERIC_AST,
       CANARY_AST
   };

  public:
   virtual std::string format(std::string indent);
   
   AstNode() = default;

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
        
   virtual ~AstNode() = default;
        
   virtual bool generateCode(codeGen &gen, 
                             bool noCost, 
                             Dyninst::Address &retAddr,
                             Dyninst::Register &retReg);

   virtual bool generateCode(codeGen &gen, 
                             bool noCost);

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

   virtual operandType getoType() const { return operandType::undefOperandType; }

   virtual void setConstFunc(bool) {}

 protected:
	BPatch_type *bptype{};  // type of corresponding BPatch_snippet
	bool doTypeCheck{true};	    // should operands be type checked
	int size{4};		    // size of the operations (in bytes)
	std::vector<AstNodePtr> children{};


 public:
	// Functions for getting and setting type decoration used by the
	// dyninst API library
	BPatch_type *getType() { return bptype; }
	void		  setType(BPatch_type *t);
	void		  setTypeChecking(bool x) { doTypeCheck = x; }
	virtual BPatch_type	  *checkType(BPatch_function* func = NULL);
	

        // PatchAPI compatibility
        virtual bool generate(Dyninst::PatchAPI::Point *, 
                              Dyninst::Buffer &);

 private:
   static AstNodePtr originalAddrNode_;
   static AstNodePtr actualAddrNode_;
   static AstNodePtr dynamicTargetNode_;
};


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
    friend class AstOperatorNode;
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




#endif /* AST_HDR */
