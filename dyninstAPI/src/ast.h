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

// $Id: ast.h,v 1.92 2006/10/16 20:17:24 bernat Exp $

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
        
        virtual Address generateCode(codeGen &gen, 
                                     bool noCost, 
                                     bool root);

        virtual Address generateCode_phase2(codeGen &gen,
                                            bool noCost,
                                            const pdvector<AstNode*> &ifForks);
       
        bool previousComputationValid(Register &reg,
                                      const pdvector<AstNode *> &ifForks,
                                      registerSpace *rs);
        
        virtual AstNode *operand() const { return NULL; }
	

	enum CostStyleType { Min, Avg, Max };
	int minCost() const {  return costHelper(Min);  }
	int avgCost() const {  return costHelper(Avg);  }
	int maxCost() const {  return costHelper(Max);  }

	// return the # of instruction times in the ast.
	virtual int costHelper(enum CostStyleType) const { return 0; };	
	void print() const;
        int referenceCount;     // Reference count for freeing memory
        int useCount;           // Reference count for generating code
        void setUseCount(registerSpace *rs); // Set values for useCount
        int getSize() { return size; };
        void cleanUseCount(void);
        bool checkUseCount(registerSpace*, bool&);
        void printUseCount(void);
	
	// Occasionally, we do not call .generateCode_phase2 for the
	// referenced node, but generate code by hand. This routine decrements
	// its use count properly
	void decUseCount(registerSpace *rs);

        Register kept_register; // Use when generating code for shared nodes

	// Path from the root to this node which resulted in computing the
	// kept_register. It contains only the nodes where the control flow
	// forks (e.g., "then" or "else" clauses of an if statement)
	pdvector<AstNode*> kept_path;

	// Record the register to share as well as the path that lead
	// to its computation
	void keepRegister(Register r, pdvector<AstNode*> path);

	// Do not keep the register anymore
	void unkeepRegister();

	// Do not keep the register and force-free it
	void forceUnkeepAndFree(registerSpace *rs);

	// Our children may have incorrect useCounts (most likely they 
	// assume that we will not bother them again, which is wrong)
	void fixChildrenCounts(registerSpace *rs);

	// Check to see if the value had been computed earlier
	bool hasKeptRegister() const;

	// Check if the node can be kept at all. Some nodes (e.g., storeOp)
	// can not be cached
	virtual bool canBeKept() const { return true; }

	// Allocate a register and make it available for sharing if our
        // node is shared
	Register allocateAndKeep(registerSpace *rs, 
				 const pdvector<AstNode*> &ifForks,
				 codeGen &gen, bool noCost);

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

 private:
    virtual Address generateCode_phase2(codeGen &gen,
                                        bool noCost,
                                        const pdvector<AstNode*> &ifForks);
};

class AstLabelNode : public AstNode {
 public:
    AstLabelNode(pdstring &label) : AstNode(), label_(label), generatedAddr_(0) {};

 private:
    virtual Address generateCode_phase2(codeGen &gen,
                                        bool noCost,
                                        const pdvector<AstNode*> &ifForks);
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

 private:
    virtual Address generateCode_phase2(codeGen &gen,
                                        bool noCost,
                                        const pdvector<AstNode*> &ifForks);
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
        
 private:
        virtual Address generateCode_phase2(codeGen &gen,
                                            bool noCost,
                                            const pdvector<AstNode*> &ifForks);

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

 private:
        virtual Address generateCode_phase2(codeGen &gen,
                                            bool noCost,
                                            const pdvector<AstNode*> &ifForks);

    AstCallNode() {};
    // Sometimes we just don't have enough information...
    const pdstring func_name_;
    Address func_addr_; // Sigh... some 
    
    int_function *func_;
    pdvector<AstNode *> args_;
};

class AstReplacementNode : public AstNode {
 public:
    AstReplacementNode(int_function *rep) :
        AstNode(),
        replacement(rep) {};

    virtual bool canBeKept() const;

 private:
        virtual Address generateCode_phase2(codeGen &gen,
                                            bool noCost,
                                            const pdvector<AstNode*> &ifForks);

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

 private:
        virtual Address generateCode_phase2(codeGen &gen,
                                            bool noCost,
                                            const pdvector<AstNode*> &ifForks);

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


 protected:
        virtual Address generateCode_phase2(codeGen &gen,
                                            bool noCost,
                                            const pdvector<AstNode*> &ifForks);

    AstInsnNode() {};
    instruction *insn_;
    Address origAddr_; // The instruction class should wrap an address, but _wow_
    // reengineering
};

class AstInsnBranchNode : public AstInsnNode {
 public:
    AstInsnBranchNode(instruction *insn, Address addr) : AstInsnNode(insn, addr), target_(NULL) {};

    virtual bool overrideBranchTarget(AstNode *t) { target_ = t; return true; }
    
 protected:
    virtual Address generateCode_phase2(codeGen &gen,
                                        bool noCost,
                                        const pdvector<AstNode*> &ifForks);
    
    AstNode *target_;
};

class AstInsnMemoryNode : public AstInsnNode {
 public:
    AstInsnMemoryNode(instruction *insn, Address addr) : AstInsnNode(insn, addr), load_(NULL), store_(NULL) {};
    
    virtual bool overrideLoadAddr(AstNode *l) { load_ = l; return true; }
    virtual bool overrideStoreAddr(AstNode *s) { store_ = s; return true; }
    

 protected:
    virtual Address generateCode_phase2(codeGen &gen,
                                        bool noCost,
                                        const pdvector<AstNode*> &ifForks);
    
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

 private:
    AstMiniTrampNode() {};

    bool inline_;
    AstNode *ast_;
};

class AstMemoryNode : public AstNode {
 public:
    AstMemoryNode(memoryType mem, unsigned which);

 private:
        virtual Address generateCode_phase2(codeGen &gen,
                                            bool noCost,
                                            const pdvector<AstNode*> &ifForks);
    
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
