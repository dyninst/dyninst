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

// $Id: ast.h,v 1.95 2006/11/22 04:03:08 bernat Exp $

#ifndef AST_HDR
#define AST_HDR

//
// Define a AST class for use in generating primitive and pred calls
//

#include <stdio.h>
#include "common/h/Vector.h"
#include "common/h/Dictionary.h"
#include "common/h/String.h"
#include "common/h/Types.h"
#include "dyninstAPI/h/BPatch_type.h"

#include "registerSpace.h"

class process;
class instPoint;
class int_function;
class codeGen;
class codeRange;
class instruction;
class BPatch_instruction; // Memory, etc. are at BPatch. Might want to move 'em.

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

class regTracker_t {
public:
	class commonExpressionTracker {
	public:
		Register keptRegister;	
		int keptLevel;
		commonExpressionTracker() : keptRegister(REG_NULL), keptLevel(-1) {};
	};

	int condLevel;
	
	static unsigned astHash(AstNode* const &ast);

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
        static AstNode *nullNode();

        static AstNode *labelNode(pdstring &label);

        static AstNode *operandNode(operandType ot, void *arg);
        static AstNode *operandNode(operandType ot, AstNode *ast);

        static AstNode *memoryNode(memoryType ot, int which);

        static AstNode *sequenceNode(pdvector<AstNode *> &sequence);

        static AstNode *operatorNode(opCode ot, AstNode *l=NULL, AstNode *r=NULL, AstNode *e=NULL);

        static AstNode *funcCallNode(const pdstring &func, pdvector<AstNode *> &args, process *proc = NULL);
        static AstNode *funcCallNode(int_function *func, pdvector<AstNode *> &args);
        static AstNode *funcCallNode(Address addr, pdvector<AstNode *> &args); // For when you absolutely need
        // to jump somewhere.

        static AstNode *funcReplacementNode(int_function *func);

        static AstNode *insnNode(BPatch_instruction *insn);

        // TODO...
        // Needs some way of marking what to save and restore... should be a registerSpace, really
#if 0
        static AstNode *saveStateNode();
        static AstNode *restoreStateNode();
        static AstNode *threadIndexNode();
        static AstNode *trampGuardNode();
#endif

        static AstNode *miniTrampNode(AstNode *tramp);


        AstNode(AstNode *src);
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
       unsigned getTreeSize();


        bool previousComputationValid(Register &reg,
                                      codeGen &gen);
		// Remove any kept register at a greater level than
		// that provided (AKA that had been calculated within
		// a conditional statement)
        void cleanRegTracker(regTracker_t *tracker, int level);

        virtual AstNode *operand() const { return NULL; }
	
		virtual bool containsFuncCall() const { return false; }

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
	virtual void getChildren(pdvector<AstNode*> &); 

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
	BPatch_type *getType() { return bptype; };
	void		  setType(BPatch_type *t) { 
				bptype = t; 
				if( t != NULL ) { size = t->getSize(); } }
	void		  setTypeChecking(bool x) { doTypeCheck = x; }
	virtual BPatch_type	  *checkType();
};


AstNode *assignAst(AstNode *src);
void removeAst(AstNode *&ast);
void terminateAst(AstNode *&ast);


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
    AstLabelNode(pdstring &label) : AstNode(), label_(label), generatedAddr_(0) {};

	bool canBeKept() const { return true; }
 private:
    virtual bool generateCode_phase2(codeGen &gen,
                                     bool noCost,
                                     Address &retAddr,
                                     Register &retReg);
    pdstring label_;
    Address generatedAddr_;
};

class AstOperatorNode : public AstNode {
 public:
    AstOperatorNode(opCode opC, AstNode *l, AstNode *r = NULL, AstNode *e = NULL);
    
    ~AstOperatorNode() {
        if (loperand) removeAst(loperand);
        if (roperand) removeAst(roperand);
        if (eoperand) removeAst(eoperand);
    }

    virtual int costHelper(enum CostStyleType costStyle) const;	

    virtual BPatch_type	  *checkType();
    virtual bool accessesParam(void);         // Does this AST access "Param"

    virtual bool canBeKept() const;

    virtual void getChildren(pdvector<AstNode*> &children);

	virtual bool containsFuncCall() const;

 private:
    virtual bool generateCode_phase2(codeGen &gen,
                                     bool noCost,
                                     Address &retAddr,
                                     Register &retReg);

    AstOperatorNode() {};
    opCode op;
    AstNode *loperand;
    AstNode *roperand;
    AstNode *eoperand;
};


class AstOperandNode : public AstNode {
    friend class AstOperatorNode; // ARGH
 public:
    // Direct operand
    AstOperandNode(operandType ot, void *arg);

    // And an indirect (say, a load)
    AstOperandNode(operandType ot, AstNode *l);

    ~AstOperandNode() {
        if (oType == ConstantString) free((char *)oValue);
        if (operand_) removeAst(operand_);
    }
        
    // Arguably, the previous should be an operation...
    // however, they're kinda endemic.

    virtual operandType getoType() const { return oType; };

    virtual void setOValue(void *o) { oValue = o; }
    virtual const void *getOValue() const { return oValue; };

    virtual AstNode *operand() const { return operand_; }

    virtual int costHelper(enum CostStyleType costStyle) const;	
        
    virtual BPatch_type	  *checkType();

    virtual bool accessesParam(void) { return (oType == Param); };
    virtual bool canBeKept() const;
        
    virtual void getChildren(pdvector<AstNode*> &children);

	virtual bool containsFuncCall() const;
        
 private:
    virtual bool generateCode_phase2(codeGen &gen,
                                     bool noCost,
                                     Address &retAddr,
                                     Register &retReg);

    AstOperandNode() {};
    
    operandType oType;
    void *oValue;
    AstNode *operand_;
    
};


class AstCallNode : public AstNode {
 public:
    AstCallNode(int_function *func, pdvector<AstNode *>&args);
    AstCallNode(const pdstring &str, pdvector<AstNode *>&args);
    AstCallNode(Address addr, pdvector<AstNode *> &args);
    
    ~AstCallNode() {
        for (unsigned i = 0; i < args_.size(); i++) {
            removeAst(args_[i]);
        }
    }

    virtual int costHelper(enum CostStyleType costStyle) const;	
        
    virtual BPatch_type	  *checkType();
    virtual bool accessesParam(); 
    virtual bool canBeKept() const;

    virtual void getChildren(pdvector<AstNode*> &children);

    virtual bool containsFuncCall() const { return true; }

    void setConstFunc(bool val) { constFunc_ = val; }

 private:
    virtual bool generateCode_phase2(codeGen &gen,
                                     bool noCost,
                                     Address &retAddr,
                                     Register &retReg);

    AstCallNode() {};
    // Sometimes we just don't have enough information...
    const pdstring func_name_;
    Address func_addr_;
    
    int_function *func_;
    pdvector<AstNode *> args_;

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

    virtual bool containsFuncCall() const { return true; };

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
    AstSequenceNode(pdvector<AstNode *> &sequence);

    ~AstSequenceNode() {
        for (unsigned i = 0; i < sequence_.size(); i++) {
            removeAst(sequence_[i]);
        }
    }

    virtual int costHelper(enum CostStyleType costStyle) const;	

    virtual BPatch_type	  *checkType();
    virtual bool accessesParam();
    virtual bool canBeKept() const;

    virtual void getChildren(pdvector<AstNode*> &children);

	virtual bool containsFuncCall() const;

 private:
    virtual bool generateCode_phase2(codeGen &gen,
                                     bool noCost,
                                     Address &retAddr,
                                     Register &retReg);

    AstSequenceNode() {};
    pdvector<AstNode *> sequence_;
};

class instruction;

class AstInsnNode : public AstNode {
 public: 
    AstInsnNode(instruction *insn, Address addr);

    // Template methods...
    virtual bool overrideBranchTarget(AstNode *) { return false; }
    virtual bool overrideLoadAddr(AstNode *) { return false; }
    virtual bool overrideStoreAddr(AstNode *) { return false; }

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
    AstInsnBranchNode(instruction *insn, Address addr) : AstInsnNode(insn, addr), target_(NULL) {};

    virtual bool overrideBranchTarget(AstNode *t) { target_ = t; return true; }

	virtual bool containsFuncCall() const;
    
 protected:
    virtual bool generateCode_phase2(codeGen &gen,
                                     bool noCost,
                                     Address &retAddr,
                                     Register &retReg);
    
    AstNode *target_;
};

class AstInsnMemoryNode : public AstInsnNode {
 public:
    AstInsnMemoryNode(instruction *insn, Address addr) : AstInsnNode(insn, addr), load_(NULL), store_(NULL) {};
    
    virtual bool overrideLoadAddr(AstNode *l) { load_ = l; return true; }
    virtual bool overrideStoreAddr(AstNode *s) { store_ = s; return true; }
    
	virtual bool containsFuncCall() const;

 protected:
    virtual bool generateCode_phase2(codeGen &gen,
                                     bool noCost,
                                     Address &retAddr,
                                     Register &retReg);
    
    AstNode *load_;
    AstNode *store_;
};


class AstMiniTrampNode : public AstNode {
 public:
    AstMiniTrampNode(AstNode *ast) {
        ast_ = assignAst(ast);
    }

    Address generateTramp(codeGen &gen, 
                          int &trampCost, 
                          bool noCost, bool merged);
            
    virtual ~AstMiniTrampNode() {
        if (ast_) removeAst(ast_);
    }    

    virtual bool accessesParam(void) { return ast_->accessesParam(); } 

    virtual void getChildren(pdvector<AstNode*> &children);

    virtual bool containsFuncCall() const;
    bool canBeKept() const;

    AstNode *getAST() { return ast_; }

 private:
    AstMiniTrampNode() {};

    bool inline_;
    AstNode *ast_;
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


void emitLoadPreviousStackFrameRegister(Address register_num,
					Register dest,
                                        codeGen &gen,
					int size,
					bool noCost);
void emitFuncJump(opCode op, codeGen &gen,
		  const int_function *func, process *proc,
		  const instPoint *loc, bool noCost);


#endif /* AST_HDR */
