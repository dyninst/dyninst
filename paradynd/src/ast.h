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

#ifndef AST_HDR
#define AST_HDR

/*
 * $Log: ast.h,v $
 * Revision 1.19  1996/11/14 14:26:58  naim
 * Changing AstNodes back to pointers to improve performance - naim
 *
 * Revision 1.18  1996/11/11 01:44:56  lzheng
 * Moved the instructions which is used to caculate the observed cost
 * from the miniTramps to baseTramp
 *
 * Revision 1.17  1996/10/31 08:36:11  tamches
 * the shm-sampling commit; added noCost param to several fns
 *
 * Revision 1.16  1996/09/13 21:41:56  mjrg
 * Implemented opcode ReturnVal for ast's to get the return value of functions.
 * Added missing calls to free registers in Ast.generateCode and emitFuncCall.
 * Removed architecture dependencies from inst.C.
 * Changed code to allow base tramps of variable size.
 *
 * Revision 1.15  1996/08/20 19:06:29  lzheng
 * Implementation of moving multiple instructions sequence
 * and splitting the instrumentation into two phases
 *
 * Revision 1.14  1996/08/16 21:18:16  tamches
 * updated copyright for release 1.1
 *
 * Revision 1.13  1996/04/26 20:04:32  lzheng
 * Moved the defination of emitFuncCall from inst.h to here.
 *
 * Revision 1.12  1996/03/25 20:20:06  tamches
 * the reduce-mem-leaks-in-paradynd commit
 *
 * Revision 1.11  1996/03/20 17:02:42  mjrg
 * Added multiple arguments to calls.
 * Instrument pvm_send instead of pvm_recv to get tags.
 *
 * Revision 1.10  1995/08/24 15:03:45  hollings
 * AIX/SP-2 port (including option for split instruction/data heaps)
 * Tracing of rexec (correctly spawns a paradynd if needed)
 * Added rtinst function to read getrusage stats (can now be used in metrics)
 * Critical Path
 * Improved Error reporting in MDL sematic checks
 * Fixed MDL Function call statement
 * Fixed bugs in TK usage (strings passed where UID expected)
 *
 * Revision 1.9  1995/05/18  10:29:58  markc
 * Added new opcode DataAddr
 *
 * Revision 1.8  1995/03/10  19:29:13  hollings
 * Added code to include base tramp cost in first mini-tramp.
 *
 */

//
// Define a AST class for use in generating primitive and pred calls
//
//

#include <stdio.h>
#include <strstream.h>
#include "paradynd/src/inst.h"

class dataReqNode;

// a register.
typedef int reg;

class registerSlot {
 public:
    int number;         // what register is it
    bool inUse;      	// free or in use.
    bool needsSaving;	// been used since last rest
    bool mustRestore;   // need to restore it before we are done.		
    bool startsLive;	// starts life as a live register.
};

class registerSpace {
    public:
	registerSpace(int dCount, int *deads, int lCount, int *lives);
	reg allocateRegister(char *insn, unsigned &base, bool noCost);
	void freeRegister(int reg);
	void resetSpace();
	bool isFreeRegister(reg reg_number);
	int getRegisterCount() { return numRegisters; }
	registerSlot *getRegSlot(int i) { return (&registers[i]); }
	bool readOnlyRegister(reg reg_number);
        void keep_register(reg k);
        void unkeep_register(reg k);
        bool is_keep_register(reg k);
    private:
	int numRegisters;
	int highWaterRegister;
	registerSlot *registers;
        vector<reg> keep_list;
};

class AstNode {
    public:
        enum nodeType { sequenceNode, opCodeNode, operandNode, callNode };
        enum operandType { Constant, ConstantPtr, DataValue, DataPtr, 
                           DataId, DataIndir, DataReg,
			   Param, ReturnVal, DataAddr };

        AstNode(); // mdl.C
	AstNode(const string &func, AstNode *l, AstNode *r);
        AstNode(const string &func, AstNode *l); // needed by inst.C
	AstNode(operandType ot, void *arg);
	AstNode(AstNode *l, AstNode *r);

#if defined(sparc_sun_sunos4_1_3) || defined(sparc_sun_solaris2_4)  
    public:	
	bool astFlag;  
	void sysFlag(instPoint *location);    
#endif

    private:
        AstNode(opCode); // like AstNode(opCode, const AstNode &, 
                         //              const AstNode &)
                         // but assumes "NULL" for both child ptrs
    public:
        AstNode(opCode, AstNode *left); 
        // assumes "NULL" for right child ptr
        // needed by inst.C and stuff in ast.C

    public:
        AstNode(operandType ot, AstNode *l);
	AstNode(opCode ot, AstNode *l, AstNode *r);
        AstNode(const string &func, vector<AstNode *> &ast_args);

        AstNode(AstNode *src);
        AstNode &operator=(const AstNode &src);

       ~AstNode();

	int generateTramp(process *proc, char *i, unsigned &base,
			  int &trampCost, bool noCost);
	reg generateCode(process *proc, registerSpace *rs, char *i, 
			 unsigned &base, bool noCost);
	reg generateCode_phase2(process *proc, registerSpace *rs, char *i, 
			        unsigned &base, bool noCost);

	int cost() const;	// return the # of instruction times in the ast.
	void print() const;
        int referenceCount;     // Reference count for freeing memory
        int useCount;           // Reference count for generating code
        void setUseCount(void); // Set values for useCount
        void cleanUseCount(void);
        void printUseCount(void);
        reg kept_register;      // Use when generating code for shared nodes
        void updateOperandsRC(bool flag); // Update operand's referenceCount
                                          // if "flag" is true, increments the
                                          // counter, otherwise it decrements 
                                          // the counter.
        void printRC(void);
    private:
	nodeType type;
	opCode op;		    // only for opCode nodes
	string callee;		    // only for call nodes
	vector<AstNode *> operands; // only for call nodes
	operandType oType;	    // for operand nodes
	dataReqNode *dValue;	    // for operand nodes with type DataPtr 
                                    // or DataValue
	void *oValue;		    // for operand nodes with other type

        // These 2 vrbles must be pointers; otherwise, we'd have a recursive
        // data structure with an infinite size.
        // The only other option is to go with references, which would have
        // to be initialized in the constructor and can't use NULL as a
        // sentinel value...
	AstNode *loperand;
	AstNode *roperand;

	int firstInsn;
	int lastInsn;
};

AstNode *assignAst(AstNode *src);
void removeAst(AstNode *&ast);
void terminateAst(AstNode *&ast);
AstNode *createPrimitiveCall(const string &func, dataReqNode *dataPtr, 
                             int param2);
AstNode *createIf(AstNode *expression, AstNode *action);
AstNode *createCounter(const string &func, dataReqNode *, AstNode *arg);

unsigned emitFuncCall(opCode op, registerSpace *rs, char *i,unsigned &base, 
		      const vector<AstNode *> &operands, const string &func,
		      process *proc, bool noCost);

#endif
