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
 * Revision 1.30  1998/08/26 20:57:20  zhichen
 * Fixed dag code generation, added one parameter to AstNode::CodeGeneration.
 *
 * Revision 1.29  1998/08/25 19:35:04  buck
 * Initial commit of DEC Alpha port.
 *
 * Revision 1.5  1997/07/08 20:48:21  buck
 * Merge changes from Wisconsin into Maryland repository.
 *
 * Revision 1.28  1997/06/23 19:15:50  buck
 * Added features to the dyninst API library, including an optional "else"
 * in a BPatch_ifExpr; the BPatch_setMutationsActive call to temporarily
 * disable all snippets; and the replaceFunctionCall and removeFunctionCall
 * member functions of BPatch_thread to retarget or NOOP out a function
 * call.
 *
 * Revision 1.27  1997/06/23 17:06:09  tamches
 * opCode moved into this file
 *
 * Revision 1.26  1997/05/07 19:02:55  naim
 * Getting rid of old support for threads and turning it off until the new
 * version is finished. Additionally, new superTable, baseTable and superVector
 * classes for future support of multiple threads. The fastInferiorHeap class has
 * also changed - naim
 *
 * Revision 1.25  1997/04/29 16:58:51  buck
 * Added features to dyninstAPI library, including the ability to delete
 * inserted snippets and the start of type checking.
 *
 * Revision 1.2  1997/04/09 17:20:14  buck
 * Added getThreads calls to BPatch, support for deleting snippets,
 * and the start of support for the type system.
 *
 * Revision 1.1.1.1  1997/04/01 20:25:00  buck
 * Update Maryland repository with latest from Wisconsin.
 *
 * Revision 1.24  1997/03/18 19:44:08  buck
 * first commit of dyninst library.  Also includes:
 * 	moving templates from paradynd to dyninstAPI
 * 	converting showError into a function (in showerror.C)
 * 	many ifdefs for BPATCH_LIBRARY in dyinstAPI/src.
 *
 * Revision 1.23  1997/03/14 15:58:13  lzheng
 * Dealing with complier optimization related to the return value
 *
 * Revision 1.22  1997/02/26 23:42:50  mjrg
 * First part on WindowsNT port: changes for compiling with Visual C++;
 * moved unix specific code to unix.C
 *
 * Revision 1.21  1997/02/21 20:13:17  naim
 * Moving files from paradynd to dyninstAPI + moving references to dataReqNode
 * out of the ast class. The is the first pre-dyninstAPI commit! - naim
 *
 * Revision 1.20  1997/01/27 19:40:38  naim
 * Part of the base instrumentation for supporting multithreaded applications
 * (vectors of counter/timers) implemented for all current platforms +
 * different bug fixes - naim
 *
 */

//
// Define a AST class for use in generating primitive and pred calls
//
//

#include <stdio.h>
#include "util/h/Vector.h"
#include "util/h/Dictionary.h"
#include "util/h/String.h"
#include "util/h/Types.h"

class process;
class instPoint;

class BPatch_type;

// a register.
typedef int reg;

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
               storeOp,
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
	       loadIndirOp,
	       storeIndirOp,
	       saveRegOp,
	       updateCostOp,
	       branchOp} opCode;

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
	reg allocateRegister(char *insn, Address &base, bool noCost);
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
        enum operandType { Constant, ConstantPtr, ConstantString,
			   DataValue, DataPtr, 
                           DataId, DataIndir, DataReg,
			   Param, ReturnVal, DataAddr,
			   SharedData};

        AstNode(); // mdl.C
	AstNode(const string &func, AstNode *l, AstNode *r);
        AstNode(const string &func, AstNode *l); // needed by inst.C
	AstNode(operandType ot, void *arg);
	AstNode(AstNode *l, AstNode *r);

        AstNode(opCode, AstNode *left); 
        // assumes "NULL" for right child ptr
        // needed by inst.C and stuff in ast.C

        AstNode(operandType ot, AstNode *l);
	AstNode(opCode ot, AstNode *l, AstNode *r, AstNode *e = NULL);
        AstNode(const string &func, vector<AstNode *> &ast_args);

        AstNode(AstNode *src);
        AstNode &operator=(const AstNode &src);

       ~AstNode();

	int generateTramp(process *proc, char *i, Address &base,
			  int &trampCost, bool noCost);
	reg generateCode(process *proc, registerSpace *rs, char *i, 
			 Address &base, bool noCost, bool root);
	reg generateCode_phase2(process *proc, registerSpace *rs, char *i, 
			        Address &base, bool noCost);

	int cost() const;	// return the # of instruction times in the ast.
	void print() const;
        int referenceCount;     // Reference count for freeing memory
        int useCount;           // Reference count for generating code
        void setUseCount(void); // Set values for useCount
        void cleanUseCount(void);
        bool checkUseCount(registerSpace*, bool&);
        void printUseCount(void);
        reg kept_register;      // Use when generating code for shared nodes
        void updateOperandsRC(bool flag); // Update operand's referenceCount
                                          // if "flag" is true, increments the
                                          // counter, otherwise it decrements 
                                          // the counter.
        void printRC(void);
	bool findFuncInAst(string func) ;
	void replaceFuncInAst(string func1, string func2);
	void replaceFuncInAst(string func1, string func2, vector<AstNode *> &ast_args, int index=0);

#if defined(sparc_sun_sunos4_1_3) || defined(sparc_sun_solaris2_4)  
	bool astFlag;  
	void sysFlag(instPoint *location);    

	void optRetVal(AstNode *opt);
#endif

    private:
        AstNode(opCode); // like AstNode(opCode, const AstNode &, 
                         //              const AstNode &)
                         // but assumes "NULL" for both child ptrs
	nodeType type;
	opCode op;		    // only for opCode nodes
	string callee;		    // only for call nodes
	vector<AstNode *> operands; // only for call nodes
	operandType oType;	    // for operand nodes
	void *oValue;	            // operand value for operand nodes
	const BPatch_type *bptype;  // type of corresponding BPatch_snippet
	bool doTypeCheck;	    // should operands be type checked

        // These 2 vrbles must be pointers; otherwise, we'd have a recursive
        // data structure with an infinite size.
        // The only other option is to go with references, which would have
        // to be initialized in the constructor and can't use NULL as a
        // sentinel value...
	AstNode *loperand;
	AstNode *roperand;
	AstNode *eoperand;

	int firstInsn;
	int lastInsn;

    public:
	// Functions for getting and setting type decoration used by the
	// dyninst API library
#ifdef BPATCH_LIBRARY
	const BPatch_type *getType() { return bptype; };
	void		  setType(const BPatch_type *t) { bptype = t; }
	void		  setTypeChecking(bool x) { doTypeCheck = x; }
	BPatch_type	  *checkType();
#endif
};

AstNode *assignAst(AstNode *src);
void removeAst(AstNode *&ast);
void terminateAst(AstNode *&ast);
AstNode *createIf(AstNode *expression, AstNode *action);
#if defined(SHM_SAMPLING) && defined(MT_THREAD)
AstNode *createCounter(const string &func, void *, void *, AstNode *arg);
AstNode *createTimer(const string &func, void *, void *, 
                     vector<AstNode *> &arg_args);
AstNode *computeAddress(void *level, void *index, int type);
#else
AstNode *createCounter(const string &func, void *, AstNode *arg);
AstNode *createTimer(const string &func, void *, 
                     vector<AstNode *> &arg_args);
#endif
Address emitFuncCall(opCode op, registerSpace *rs, char *i,Address &base, 
		      const vector<AstNode *> &operands, const string &func,
		      process *proc, bool noCost);
#endif



