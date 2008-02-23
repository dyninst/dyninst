/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

// $Id: ast.h,v 1.104 2008/02/23 02:09:05 jaw Exp $

#ifndef AST_HDR
#define AST_HDR

//
// Define a AST class for use in generating primitive and pred calls
//

#include <stdio.h>
#include <string>
#include "common/h/Vector.h"
#include "common/h/Dictionary.h"
#include "common/h/Types.h"

// The great experiment: boost shared_ptr libraries
#include "boost/shared_ptr.hpp"

class process;
class AddressSpace;
class instPoint;
class int_function;
class codeGen;
class codeRange;
class instruction;
class BPatch_instruction; // Memory, etc. are at BPatch. Might want to move 'em.
class BPatch_type;

// a register number, e.g. [0,31]
// typedef int reg; // see new Register type in "common/h/Types.h"

#include "opcode.h"

// Register retention mechanism...
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
class AstMiniTrampNode;
typedef boost::shared_ptr<AstMiniTrampNode> AstMiniTrampNodePtr;

class registerSpace;

class regTracker_t {
public:
	class commonExpressionTracker {
	public:
		Register keptRegister;	
		int keptLevel;
		commonExpressionTracker() : keptRegister(REG_NULL), keptLevel(-1) {};
	};

	int condLevel;
	
	static unsigned astHash(AstNode * const &ast);

	regTracker_t() : condLevel(0), tracker(astHash) {};

	dictionary_hash<AstNode *, commonExpressionTracker> tracker;

	void addKeptRegister(codeGen &gen, AstNode *n, Register reg);
	void removeKeptRegister(codeGen &gen, AstNode *n);
	Register hasKeptRegister(AstNode *n);
	bool stealKeptRegister(Register reg); 

	void reset();

	void increaseConditionalLevel();
	void decreaseAndClean(codeGen &gen);
	void cleanKeptRegisters(int level);

	void debugPrint();
};

class dataReqNode;
class AstNode {
    public:
        enum nodeType { sequenceNode_t, opCodeNode_t, operandNode_t, callNode_t };
        enum operandType { Constant, 
                           ConstantString,
                           DataReg,
                           DataIndir,
			   Param, 
                           ReturnVal, 
                           DataAddr,  // ?
                           FrameAddr, // Calculate FP 
                           RegOffset, // Calculate *reg + offset; oValue is reg, loperand->oValue is offset. 
                           PreviousStackFrameDataReg,
						   RegValue, // A possibly spilled, possibly saved register.
                           undefOperandType };

        enum memoryType {
            EffectiveAddr,
            BytesAccessed };

        AstNode(); // mdl.C

        // Factory methods....
        static AstNodePtr nullNode();

        static AstNodePtr labelNode(std::string &label);

        static AstNodePtr operandNode(operandType ot, void *arg);
        static AstNodePtr operandNode(operandType ot, AstNodePtr ast);

        static AstNodePtr memoryNode(memoryType ot, int which);

        static AstNodePtr sequenceNode(pdvector<AstNodePtr > &sequence);

        static AstNodePtr operatorNode(opCode ot, 
                                       AstNodePtr l = AstNodePtr(), 
                                       AstNodePtr r = AstNodePtr(), 
                                       AstNodePtr e = AstNodePtr());

        static AstNodePtr funcCallNode(const std::string &func, pdvector<AstNodePtr > &args, AddressSpace *addrSpace = NULL);
        static AstNodePtr funcCallNode(int_function *func, pdvector<AstNodePtr > &args);
        static AstNodePtr funcCallNode(Address addr, pdvector<AstNodePtr > &args); // For when you absolutely need
        // to jump somewhere.

        static AstNodePtr funcReplacementNode(int_function *func);

        static AstNodePtr insnNode(BPatch_instruction *insn);

        // Acquire the thread index value - a 0...n labelling of threads.
        static AstNodePtr threadIndexNode();

        // TODO...
        // Needs some way of marking what to save and restore... should be a registerSpace, really

#if 0
        static AstNodePtr saveStateNode();
        static AstNodePtr restoreStateNode();
        static AstNodePtr trampGuardNode();
#endif

        static AstNodePtr miniTrampNode(AstNodePtr tramp);

        static AstNodePtr originalAddrNode();
        static AstNodePtr actualAddrNode();

        AstNode(AstNodePtr src);
        //virtual AstNode &operator=(const AstNode &src);
        
        virtual ~AstNode();
        
        virtual bool generateCode(codeGen &gen, 
                                  bool noCost, 
                                  Address &retAddr,
                                  Register &retReg);

        // Can't use default references....
        virtual bool generateCode(codeGen &gen, 
                                  bool noCost);

        // Can't use default references....
        virtual bool generateCode(codeGen &gen, 
                                  bool noCost, 
                                  Register &retReg) { 
            Address unused = ADDR_NULL;
            return generateCode(gen, noCost,  unused, retReg);
        }

        // I don't know if there is an overload between address and register...
        // so we'll toss in two different return types.
        virtual bool generateCode_phase2(codeGen &gen,
                                         bool noCost,
                                         Address &retAddr,
                                         Register &retReg);

        // Perform whatever pre-processing steps are necessary.
        virtual bool initRegisters(codeGen &gen);

       unsigned getTreeSize();


        bool previousComputationValid(Register &reg,
                                      codeGen &gen);
		// Remove any kept register at a greater level than
		// that provided (AKA that had been calculated within
		// a conditional statement)
        void cleanRegTracker(regTracker_t *tracker, int level);

        virtual AstNodePtr operand() const { return AstNodePtr(); }

        virtual bool containsFuncCall() const;
	
	enum CostStyleType { Min, Avg, Max };
	int minCost() const {  return costHelper(Min);  }
	int avgCost() const {  return costHelper(Avg);  }
	int maxCost() const {  return costHelper(Max);  }

	// return the # of instruction times in the ast.
	virtual int costHelper(enum CostStyleType) const { return 0; };	

        int referenceCount;     // Reference count for freeing memory
        int useCount;           // Reference count for generating code
        void setUseCount(); // Set values for useCount
        int getSize() { return size; };
        void cleanUseCount(void);
        bool checkUseCount(registerSpace*, bool&);
        void printUseCount(void);
	
	void debugPrint(unsigned level = 0);
	// Occasionally, we do not call .generateCode_phase2 for the
	// referenced node, but generate code by hand. This routine decrements
	// its use count properly
	void decUseCount(codeGen &gen);

	// Our children may have incorrect useCounts (most likely they 
	// assume that we will not bother them again, which is wrong)
	void fixChildrenCounts();

	// Check if the node can be kept at all. Some nodes (e.g., storeOp)
	// can not be cached
	virtual bool canBeKept() const = 0;

	// Allocate a register and make it available for sharing if our
    // node is shared
	Register allocateAndKeep(codeGen &gen, bool noCost);

    // If someone needs to take this guy away.
    bool stealRegister(Register reg);

	// Check to see if path1 is a subpath of path2
	bool subpath(const pdvector<AstNode*> &path1, 
		     const pdvector<AstNode*> &path2) const;

	// Return all children of this node ([lre]operand, ..., operands[])
	virtual void getChildren(pdvector<AstNodePtr> &); 

        void printRC(void);
	virtual bool accessesParam(void);

	virtual void setOValue(void *) { assert(0); }
	virtual const void *getOValue() const { assert(0); return NULL; }
	// only function that's defined in metric.C (only used in metri.C)
	bool condMatch(AstNode* a,
		       pdvector<dataReqNode*> &data_tuple1,
		       pdvector<dataReqNode*> &data_tuple2,
		       pdvector<dataReqNode*> datareqs1,
		       pdvector<dataReqNode*> datareqs2);


	// DEBUG
        virtual operandType getoType() const { return undefOperandType; }

        virtual void setConstFunc(bool) {};

 protected:
	BPatch_type *bptype;  // type of corresponding BPatch_snippet
	bool doTypeCheck;	    // should operands be type checked
	int size;		    // size of the operations (in bytes)


    public:
	// Functions for getting and setting type decoration used by the
	// dyninst API library
	//AstNode(operandType ot, int which); // for memory access
	BPatch_type *getType();
	void		  setType(BPatch_type *t);
	void		  setTypeChecking(bool x) { doTypeCheck = x; }
	virtual BPatch_type	  *checkType();

 private:
        static AstNodePtr originalAddrNode_;
        static AstNodePtr actualAddrNode_;

};


class AstNullNode : public AstNode {
 public:
    AstNullNode() : AstNode() {};

    bool canBeKept() const { return true; }
 private:
    virtual bool generateCode_phase2(codeGen &gen,
                                     bool noCost,
                                     Address &retAddr,
                                     Register &retReg);
};

class AstLabelNode : public AstNode {
 public:
    AstLabelNode(std::string &label) : AstNode(), label_(label), generatedAddr_(0) {};

	bool canBeKept() const { return true; }
 private:
    virtual bool generateCode_phase2(codeGen &gen,
                                     bool noCost,
                                     Address &retAddr,
                                     Register &retReg);
    std::string label_;
    Address generatedAddr_;
};

class AstOperatorNode : public AstNode {
 public:
    AstOperatorNode(opCode opC, AstNodePtr l, AstNodePtr r = AstNodePtr(), AstNodePtr e = AstNodePtr());
    
    virtual int costHelper(enum CostStyleType costStyle) const;	

    virtual BPatch_type	  *checkType();
    virtual bool accessesParam(void);         // Does this AST access "Param"

    virtual bool canBeKept() const;

    virtual void getChildren(pdvector<AstNodePtr> &children);
    virtual bool containsFuncCall() const;

 private:

    virtual bool generateCode_phase2(codeGen &gen,
                                     bool noCost,
                                     Address &retAddr,
                                     Register &retReg);

    bool generateOptimizedAssignment(codeGen &gen, bool noCost);

    AstOperatorNode() {};
    opCode op;
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

    ~AstOperandNode() {
        if (oType == ConstantString) free((char *)oValue);
    }
        
    // Arguably, the previous should be an operation...
    // however, they're kinda endemic.

    virtual operandType getoType() const { return oType; };

    virtual void setOValue(void *o) { oValue = o; }
    virtual const void *getOValue() const { return oValue; };

    virtual AstNodePtr operand() const { return operand_; }

    virtual int costHelper(enum CostStyleType costStyle) const;	
        
    virtual BPatch_type	  *checkType();

    virtual bool accessesParam(void) { return (oType == Param); };
    virtual bool canBeKept() const;
        
    virtual void getChildren(pdvector<AstNodePtr> &children);

    virtual bool containsFuncCall() const;
        
 private:
    virtual bool generateCode_phase2(codeGen &gen,
                                     bool noCost,
                                     Address &retAddr,
                                     Register &retReg);

    AstOperandNode() {};

    operandType oType;
    void *oValue;
    AstNodePtr operand_;
    
};


class AstCallNode : public AstNode {
 public:
    AstCallNode(int_function *func, pdvector<AstNodePtr>&args);
    AstCallNode(const std::string &str, pdvector<AstNodePtr>&args);
    AstCallNode(Address addr, pdvector<AstNodePtr> &args);
    
    ~AstCallNode() {}

    virtual int costHelper(enum CostStyleType costStyle) const;	
        
    virtual BPatch_type	  *checkType();
    virtual bool accessesParam(); 
    virtual bool canBeKept() const;

    virtual void getChildren(pdvector<AstNodePtr> &children);
    virtual bool containsFuncCall() const;

    void setConstFunc(bool val) { constFunc_ = val; }

    virtual bool initRegisters(codeGen &gen);

 private:
    virtual bool generateCode_phase2(codeGen &gen,
                                     bool noCost,
                                     Address &retAddr,
                                     Register &retReg);

    AstCallNode() {};
    // Sometimes we just don't have enough information...
    const std::string func_name_;
    Address func_addr_;
    
    int_function *func_;
    pdvector<AstNodePtr> args_;

    bool constFunc_;  // True if the output depends solely on 
    // input parameters, or can otherwise be guaranteed to not change
    // if executed multiple times in the same sequence - AKA 
    // "can be kept".
};

class AstReplacementNode : public AstNode {
 public:
    AstReplacementNode(int_function *rep) :
        AstNode(),
        replacement(rep) {};

    virtual bool canBeKept() const;
    virtual bool containsFuncCall() const;

 private:
    virtual bool generateCode_phase2(codeGen &gen,
                                     bool noCost,
                                     Address &retAddr,
                                     Register &retReg);

    int_function *replacement;
    AstReplacementNode() {};
};


class AstSequenceNode : public AstNode {
 public:
    AstSequenceNode(pdvector<AstNodePtr> &sequence);

    ~AstSequenceNode() {}

    virtual int costHelper(enum CostStyleType costStyle) const;	

    virtual BPatch_type	  *checkType();
    virtual bool accessesParam();
    virtual bool canBeKept() const;

    virtual void getChildren(pdvector<AstNodePtr> &children);
    virtual bool containsFuncCall() const;

 private:
    virtual bool generateCode_phase2(codeGen &gen,
                                     bool noCost,
                                     Address &retAddr,
                                     Register &retReg);

    AstSequenceNode() {};
    pdvector<AstNodePtr> sequence_;
};

class instruction;

class AstInsnNode : public AstNode {
 public: 
    AstInsnNode(instruction *insn, Address addr);

    // Template methods...
    virtual bool overrideBranchTarget(AstNodePtr) { return false; }
    virtual bool overrideLoadAddr(AstNodePtr) { return false; }
    virtual bool overrideStoreAddr(AstNodePtr) { return false; }

	bool canBeKept() const { return false; }
 protected:
    virtual bool generateCode_phase2(codeGen &gen,
                                     bool noCost,
                                     Address &retAddr,
                                     Register &retReg);

    AstInsnNode() {};
    instruction *insn_;
    Address origAddr_; // The instruction class should wrap an address, but _wow_
    // reengineering
};

class AstInsnBranchNode : public AstInsnNode {
 public:
    AstInsnBranchNode(instruction *insn, Address addr) : AstInsnNode(insn, addr), target_() {};

    virtual bool overrideBranchTarget(AstNodePtr t) { target_ = t; return true; }
    virtual bool containsFuncCall() const;
    
 protected:
    virtual bool generateCode_phase2(codeGen &gen,
                                     bool noCost,
                                     Address &retAddr,
                                     Register &retReg);
    
    AstNodePtr target_;
};

class AstInsnMemoryNode : public AstInsnNode {
 public:
    AstInsnMemoryNode(instruction *insn, Address addr) : AstInsnNode(insn, addr), load_(), store_() {};
    
    virtual bool overrideLoadAddr(AstNodePtr l) { load_ = l; return true; }
    virtual bool overrideStoreAddr(AstNodePtr s) { store_ = s; return true; }
    virtual bool containsFuncCall() const;

 protected:
    virtual bool generateCode_phase2(codeGen &gen,
                                     bool noCost,
                                     Address &retAddr,
                                     Register &retReg);
    
    AstNodePtr load_;
    AstNodePtr store_;
};


class AstMiniTrampNode : public AstNode {
 public:
    AstMiniTrampNode(AstNodePtr ast) {
        ast_ = ast;
    }

    Address generateTramp(codeGen &gen, 
                          int &trampCost, 
                          bool noCost, bool merged);
            
    virtual ~AstMiniTrampNode() {}    

    virtual bool accessesParam(void) { return ast_->accessesParam(); } 

    virtual void getChildren(pdvector<AstNodePtr> &children);

    virtual bool containsFuncCall() const;
    bool canBeKept() const;

    AstNodePtr getAST() { return ast_; }
 private:
    AstMiniTrampNode() {};

    bool inline_;
    AstNodePtr ast_;
};

class AstMemoryNode : public AstNode {
 public:
    AstMemoryNode(memoryType mem, unsigned which);
	bool canBeKept() const;
 private:
    virtual bool generateCode_phase2(codeGen &gen,
                                     bool noCost,
                                     Address &retAddr,
                                     Register &retReg);
    
    AstMemoryNode() {};
    memoryType mem_;
    unsigned which_;
};

class AstOriginalAddrNode : public AstNode {
 public:
    AstOriginalAddrNode() {};

    virtual ~AstOriginalAddrNode() {};

    virtual BPatch_type *checkType() { return getType(); };
    virtual bool canBeKept() const { return true; }

 private:
    virtual bool generateCode_phase2(codeGen &gen,
                                     bool noCost,
                                     Address &retAddr,
                                     Register &retReg);

};

class AstActualAddrNode : public AstNode {
 public:
    AstActualAddrNode() {};

    virtual ~AstActualAddrNode() {};

    virtual BPatch_type *checkType() { return getType(); };
    virtual bool canBeKept() const { return false; }

 private:
    virtual bool generateCode_phase2(codeGen &gen,
                                     bool noCost,
                                     Address &retAddr,
                                     Register &retReg);
};




void emitLoadPreviousStackFrameRegister(Address register_num,
					Register dest,
                                        codeGen &gen,
					int size,
					bool noCost);
void emitFuncJump(opCode op, codeGen &gen,
		  const int_function *func, AddressSpace *addrSpace,
		  const instPoint *loc, bool noCost);


#endif /* AST_HDR */
