/*
 * Copyright (c) 1996 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
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

// $Id: ast.h,v 1.65 2003/08/01 22:55:30 jodom Exp $

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
#if defined(BPATCH_LIBRARY)
#include "dyninstAPI/h/BPatch_type.h"
#endif

class process;
class instPoint;
class function_base;


// a register number, e.g. [0,31]
// typedef int reg; // see new Register type in "common/h/Types.h"

typedef enum { plusOp,
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
               storeOp,
	       storeFrameRelativeOp,
               ifOp,
	       whileOp,  // Simple control structures will be useful
	       doOp,     // Zhichen
	       callOp,
	       trampPreamble,
	       trampTrailer,
	       noOp,
	       orOp,
	       andOp,
	       getRetValOp,
	       getSysRetValOp,
	       getParamOp,
	       getSysParamOp,	   
	       getAddrOp,	// return the address of the operand
	       loadIndirOp,
	       storeIndirOp,
	       saveRegOp,
	       updateCostOp,
	       funcJumpOp,        // Jump to function without linkage
	       branchOp,
               ifMCOp
} opCode;

class registerSlot {
 public:
    Register number;    // what register is it
    int refCount;      	// == 0 if free
    bool needsSaving;	// been used since last rest
    bool mustRestore;   // need to restore it before we are done.		
    bool startsLive;	// starts life as a live register.
};

class registerSpace {
 public:
	registerSpace(const unsigned int dCount, Register *deads,
                 const unsigned int lCount, Register *lives,
                 bool multithreaded = false);
	Register allocateRegister(char *insn, Address &base, bool noCost);
	// Free the specified register (decrement its refCount)
	void freeRegister(Register k);
	// Free the register even if its refCount is greater that 1
	void forceFreeRegister(Register k);
	void resetSpace();

	// Check to see if the register is free
	bool isFreeRegister(Register k);

	// Check to see if the register is use in multiple places
	// (if refCount > 1)
	bool isSharedRegister(Register k);

	// Manually set the reference count of the specified register
	// we need to do so when reusing an already-allocated register
	void fixRefCount(Register k, int iRefCount);
	
	// Bump up the reference count. Occasionally, we underestimate it
	// and call this routine to correct this.
	void incRefCount(Register k);

	u_int getRegisterCount() { return numRegisters; }
	registerSlot *getRegSlot(Register k) { return (&registers[k]); }
	bool readOnlyRegister(Register k);
	// Make sure that no registers remain allocated, except "to_exclude"
	// Used for assertion checking.
	void checkLeaks(Register to_exclude);
   bool for_multithreaded() { return is_multithreaded; }
 private:
	u_int numRegisters;
	Register highWaterRegister;
	registerSlot *registers;
   bool is_multithreaded;
#if defined(ia64_unknown_linux2_4)

public:
	int originalLocals;
	int originalOutputs;
	int originalRotates;
#endif

};

class dataReqNode;
class AstNode {
    public:
        enum nodeType { sequenceNode, opCodeNode, operandNode, callNode };
        enum operandType { Constant, ConstantPtr, ConstantString,
			   OffsetConstant,      // add a OffsetConstant type for offset
			                        // generated for level or index:
			                        //   it is  MAX#THREADS * level * tSize  for level
			                        //     or                 index * tSize  for index
			   DataValue, DataPtr,  // restore AstNode::DataValue and AstNode::DataPtr
                           DataId, DataIndir, DataReg,
			   Param, ReturnVal, DataAddr, FrameAddr,
			   SharedData, PreviousStackFrameDataReg,
			   EffectiveAddr, BytesAccessed };
        AstNode(); // mdl.C
	AstNode(const pdstring &func, AstNode *l, AstNode *r);
        AstNode(const pdstring &func, AstNode *l); // needed by inst.C
	AstNode(operandType ot, void *arg);
	AstNode(AstNode *l, AstNode *r);

        AstNode(opCode, AstNode *left); 
        // assumes "NULL" for right child ptr
        // needed by inst.C and stuff in ast.C

        AstNode(operandType ot, AstNode *l);
	AstNode(opCode ot, AstNode *l, AstNode *r, AstNode *e = NULL);
        AstNode(const pdstring &func, pdvector<AstNode *> &ast_args);
        AstNode(function_base *func, pdvector<AstNode *> &ast_args);
	AstNode(function_base *func); // FuncJump (for replaceFunction)

        AstNode(AstNode *src);
        AstNode &operator=(const AstNode &src);

       ~AstNode();

        Address generateTramp(process *proc, const instPoint *location, char *i,
			      Address &base, int *trampCost, bool noCost);
	Address generateCode(process *proc, registerSpace *rs, char *i, 
			     Address &base, bool noCost, bool root,
			     const instPoint *location = NULL);
	Address generateCode_phase2(process *proc, registerSpace *rs, char *i, 
				    Address &base, bool noCost,
				    const pdvector<AstNode*> &ifForks,
				    const instPoint *location = NULL);

	enum CostStyleType { Min, Avg, Max };
	int minCost() const {  return costHelper(Min);  }
	int avgCost() const {  return costHelper(Avg);  }
	int maxCost() const {  return costHelper(Max);  }

	// return the # of instruction times in the ast.
	int costHelper(enum CostStyleType costStyle) const;	
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
	bool canBeKept() const;

	// Allocate a register and make it available for sharing if our
        // node is shared
	Register allocateAndKeep(registerSpace *rs, 
				 const pdvector<AstNode*> &ifForks,
				 char *insn, Address &base, bool noCost);

        // Sometimes we can reuse one of the source registers to store
        // the result. We must make sure that the source is
        // not shared between tree nodes.
	Register shareOrAllocate(Register left, Register right,
				 registerSpace *rs, 
				 const pdvector<AstNode*> &ifForks,
				 char *insn, Address &base, bool noCost);
	
	// Check to see if path1 is a subpath of path2
	bool subpath(const pdvector<AstNode*> &path1, 
		     const pdvector<AstNode*> &path2) const;

	// Return all children of this node ([lre]operand, ..., operands[])
	void getChildren(pdvector<AstNode*> *children);

        void updateOperandsRC(bool flag); // Update operand's referenceCount
                                          // if "flag" is true, increments the
                                          // counter, otherwise it decrements 
                                          // the counter.
        void printRC(void);
	bool findFuncInAst(pdstring func) ;
	void replaceFuncInAst(function_base *func1, function_base *func2);
	void replaceFuncInAst(function_base *func1, function_base *func2, pdvector<AstNode *> &ast_args, int index=0);
	bool accessesParam(void);         // Does this AST access "Param"

#if defined(sparc_sun_sunos4_1_3) || defined(sparc_sun_solaris2_4)  
	bool astFlag;  
	void sysFlag(instPoint *location);    

	void optRetVal(AstNode *opt);
#endif
	void setOValue(void *arg) { oValue = (void *) arg; }
	const void *getOValue() { return oValue; }
	// only function that's defined in metric.C (only used in metri.C)
	bool condMatch(AstNode* a,
		       pdvector<dataReqNode*> &data_tuple1,
		       pdvector<dataReqNode*> &data_tuple2,
		       pdvector<dataReqNode*> datareqs1,
		       pdvector<dataReqNode*> datareqs2);


	// DEBUG
	operandType getoType() const { return oType; };
    private:
        AstNode(opCode); // like AstNode(opCode, const AstNode &, 
                         //              const AstNode &)
                         // but assumes "NULL" for both child ptrs
	nodeType type;
	opCode op;		    // only for opCode nodes
	pdstring callee;		    // only for call nodes
	function_base *calleefunc;  // only for call nodes
	pdvector<AstNode *> operands; // only for call nodes
	operandType oType;	    // for operand nodes
	void *oValue;	            // operand value for operand nodes
        unsigned int whichMA;       // only for memory access nodes
#ifdef BPATCH_LIBRARY
	const BPatch_type *bptype;  // type of corresponding BPatch_snippet
	bool doTypeCheck;	    // should operands be type checked
#endif
	int size;		    // size of the operations (in bytes)

        // These 2 vrbles must be pointers; otherwise, we'd have a recursive
        // data structure with an infinite size.
        // The only other option is to go with references, which would have
        // to be initialized in the constructor and can't use NULL as a
        // sentinel value...
	AstNode *loperand;
	AstNode *roperand;
	AstNode *eoperand;

    public:
	// Functions for getting and setting type decoration used by the
	// dyninst API library
#ifdef BPATCH_LIBRARY
	AstNode(operandType ot, int which); // for memory access
	const BPatch_type *getType() { return bptype; };
	void		  setType(const BPatch_type *t) { 
				bptype = t; 
				size = t->getSize(); }
	void		  setTypeChecking(bool x) { doTypeCheck = x; }
	BPatch_type	  *checkType();
#endif
};

AstNode *assignAst(AstNode *src);
void removeAst(AstNode *&ast);
void terminateAst(AstNode *&ast);
AstNode *createIf(AstNode *expression, AstNode *action, process *proc=NULL);
AstNode *getTimerAddress(void *base, unsigned struct_size);
AstNode *getCounterAddress(void *base, unsigned struct_size);
AstNode *createCounter(const pdstring &func, void *, AstNode *arg);
AstNode *createHwTimer(const pdstring &func, void *, 
                     pdvector<AstNode *> &arg_args, int hwCntrIndex);

AstNode *createTimer(const pdstring &func, void *, 
                     pdvector<AstNode *> &arg_args);
// VG(11/06/01): This should be in inst.h I suppose; moved there...
/*
Register emitFuncCall(opCode op, registerSpace *rs, char *i, Address &base, 
		      const pdvector<AstNode *> &operands, const pdstring &func,
		      process *proc, bool noCost, const function_base *funcbase,
		      const instPoint *location = NULL);
*/
void emitLoadPreviousStackFrameRegister(Address register_num,
					Register dest,
					char *insn,
					Address &base,
					int size,
					bool noCost);
void emitFuncJump(opCode op, char *i, Address &base,
		  const function_base *func, process *proc,
		  const instPoint *loc, bool noCost);

#endif /* AST_HDR */
