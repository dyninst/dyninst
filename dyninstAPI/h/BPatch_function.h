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

#ifndef _BPatch_function_h_
#define _BPatch_function_h_

#include "BPatch_dll.h"
#include "BPatch_Vector.h"
#include "BPatch_point.h"
#include "BPatch_type.h"
#include "BPatch_module.h"
#include "BPatch_flowGraph.h"
#include "BPatch_memoryAccess_NP.h"

class function_base;
class process;

class BPatch_localVarCollection;
class BPatch_function;
class BPatch_point;
class BPatch_flowGraph;

class BPATCH_DLL_EXPORT BPatch_function: public BPatch_sourceObj {
    friend class BPatch_flowGraph;

    process *proc;
    BPatch_type * retType;
    BPatch_Vector<BPatch_localVar *> params;
    BPatch_module *mod;
    BPatch_flowGraph* cfg;

    void         *getBaseAddrRelative();
    BPatch_point* createMemInstPoint(void *addr, BPatch_memoryAccess* ma);

public:
    virtual	~BPatch_function();

// The following are for  internal use by the library only:
    function_base *func;
    process *getProc() const { return proc; }

// No longer inline but defined in .C file
    BPatch_function(process *_proc, function_base *_func, BPatch_module *mod = NULL);
    BPatch_function(process *_proc, function_base *_func,
		    BPatch_type * _retType, BPatch_module *);

    bool getSourceObj(BPatch_Vector<BPatch_sourceObj *> &);
    BPatch_sourceObj *getObjParent();
    BPatch_localVarCollection * localVariables;
    BPatch_localVarCollection * funcParameters;
    void setReturnType( BPatch_type * _retType){
      retType = _retType;}
    
// For users of the library:
    char	 *getName(char *s, int len) const;
    char	 *getMangledName(char *s, int len) const;
    void	 *getBaseAddr();
    unsigned int getSize() const;
    BPatch_type * getReturnType(){ return retType; }
    BPatch_module *getModule()	{ return mod; }
    void addParam(char * _name, BPatch_type *_type, int _linenum,
		  int _frameOffset, int _sc = 5 /* scAbs */ );
    
    BPatch_Vector<BPatch_localVar *> *getParams() { 
      return &params; }
    BPatch_Vector<BPatch_localVar *> *getVars();
    BPatch_Vector<BPatch_point *>
	*findPoint(const BPatch_procedureLocation loc);
    BPatch_Vector<BPatch_point *>
        *findPoint(const BPatch_Set<BPatch_opCode>& ops);
    BPatch_localVar * findLocalVar( const char * name);
    BPatch_localVar * findLocalParam(const char * name);
    BPatch_Vector<BPatch_variableExpr *> *findVariable(const char *);

    //method to retrieve addresses corresponding a line in the function
    bool getLineToAddr (unsigned short lineNo,
			BPatch_Vector<unsigned long>& buffer,
			bool exactMatch = true);

    // method to return file name, and first and last lines of the func
    bool getLineAndFile(unsigned int &start, 
			unsigned int &end, 
			char *name,
			unsigned &length);

    // is this defined, what variables should be returned??
    bool getVariables(BPatch_Vector<BPatch_variableExpr *> &vect);

    char *getModuleName(char *name, int maxLen);

    // Returns true if the function is instrumentable.
    bool isInstrumentable();

#ifdef IBM_BPATCH_COMPAT
    bool getLineNumbers(unsigned int &start, unsigned int &end);

    void *getAddress();
    
    bool getAddressRange(void * &start, void * &end);

    BPatch_type *returnType();

    void getIncPoints(BPatch_Vector<BPatch_point *> &vect);


    int	getMangledNameLen();

    void getExcPoints(BPatch_Vector<BPatch_point*> &points);

    // return a function that can be passed as a paramter
    BPatch_variableExpr *getFunctionRef();

 
#endif
		

//method to create the control flow graph for the function
    BPatch_flowGraph* getCFG();
};

#endif /* _BPatch_function_h_ */
