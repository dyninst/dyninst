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

#ifndef _BPatch_function_h_
#define _BPatch_function_h_

#include "BPatch_dll.h"
#include "BPatch_Vector.h"
#include "BPatch_point.h"
#include "BPatch_type.h"
#include "BPatch_module.h"
#include "BPatch_flowGraph.h"
#include "BPatch_eventLock.h"
#include "BPatch_memoryAccess_NP.h"

class int_function;
class process;
class InstrucIter;

class BPatch_localVarCollection;
class BPatch_function;
class BPatch_point;
class BPatch_flowGraph;

#ifdef DYNINST_CLASS_NAME
#undef DYNINST_CLASS_NAME
#endif
#define DYNINST_CLASS_NAME BPatch_function

class BPATCH_DLL_EXPORT BPatch_function: public BPatch_sourceObj, public BPatch_eventLock{
    friend class BPatch_flowGraph;
    friend class InstrucIter;
    friend class BPatch_basicBlock;
    friend class BPatch_asyncEventHandler;
    friend BPatch_Vector<BPatch_point*> *findPoint(const BPatch_Set<BPatch_opCode>& ops,
						   InstrucIter &ii, 
						   process *proc,
						   BPatch_function *bpf);

    process *proc;
    BPatch_type * retType;
    BPatch_Vector<BPatch_localVar *> params;
    BPatch_module *mod;
    BPatch_flowGraph* cfg;
    bool cfgCreated;

    void *getBaseAddrRelative() const;
    BPatch_point* createMemInstPoint(void *addr, BPatch_memoryAccess* ma);

public:
    virtual	~BPatch_function();

    // The following are for internal use by the library only:
    int_function *func;
    int_function *PDSEP_pdf() { return func;}
    process *getProc() const { return proc; }

    // No longer inline but defined in .C file
    BPatch_function(process *_proc, int_function *_func, BPatch_module *mod = NULL);
    BPatch_function(process *_proc, int_function *_func,
		    BPatch_type * _retType, BPatch_module *);
    bool getSourceObj(BPatch_Vector<BPatch_sourceObj *> &);
    BPatch_sourceObj *getObjParent();
    BPatch_localVarCollection * localVariables;
    BPatch_localVarCollection * funcParameters;
    void setReturnType(BPatch_type * _retType){retType = _retType;}
    void setModule(BPatch_module *module) { if (this->mod == NULL) this->mod = module;}
    void addParam(const char * _name, BPatch_type *_type, int _linenum,
                  long _frameOffset, int _reg = -1,
                  BPatch_storageClass _sc = BPatch_storageFrameOffset);
    void fixupUnknown(BPatch_module *);
    // End of functions for internal use only
    

    // For users of the library:


    //  BPatch_function::getName
    //  Returns <demangled> name of function
    API_EXPORT(Int, (s, len),     

    char *,getName,(char *s, int len));

    //  BPatch_function::getMangledName
    //  Returns mangled name of function, same as getName for non-c++ mutatees
    API_EXPORT(Int, (s, len),

    char *,getMangledName,(char *s, int len));

    //  BPatch_function::getBaseAddr
    //  Returns base address of function
    API_EXPORT(Int, (),

    void *,getBaseAddr,(void));

    //  BPatch_function::getSize
    //  Returns the size of the function in bytes
    API_EXPORT(Int, (),

    unsigned int,getSize,());

    //  BPatch_function::getReturnType
    //  Returns the <BPatch_type> return type of this function

    API_EXPORT(Int, (),

    BPatch_type *,getReturnType,());

    //  BPatch_function::getModule
    //  Returns the BPatch_module to which this function belongs

    API_EXPORT(Int, (),

    BPatch_module *,getModule,());
    
    //  BPatch_function::getParams
    //  Returns a vector of BPatch_localVar, representing this function's parameters

    API_EXPORT(Int, (),

    BPatch_Vector<BPatch_localVar *> *,getParams,());

    //  BPatch_function::getVars
    //  Returns a vector of local variables in this functions

    API_EXPORT(Int, (),

    BPatch_Vector<BPatch_localVar *> *,getVars,());

    //  BPatch_function::findPoint
    //  Returns a vector of inst points, corresponding to the given BPatch_procedureLocation

    API_EXPORT(Int, (loc),

    BPatch_Vector<BPatch_point *> *,findPoint,(CONST_EXPORT BPatch_procedureLocation loc));

    //  BPatch_function::findPoint
    //  Returns a vector of inst points, corresponding to the given set of op codes

    API_EXPORT(ByOp, (ops),

    BPatch_Vector<BPatch_point *> *,findPoint,(const BPatch_Set<BPatch_opCode>& ops));

    //  BPatch_function::findLocalVar
    //  Returns a BPatch_localVar, if a match for <name> is found

    API_EXPORT(Int, (name),

    BPatch_localVar *,findLocalVar,(const char * name));

    //  BPatch_function::findLocalParam
    //  Returns a BPatch_localVar, if a match for <name> is found

    API_EXPORT(Int, (name),

    BPatch_localVar *,findLocalParam,(const char * name));

    //  BPatch_function::findVariable
    //  Returns a set of variables matching <name> at the scope of this function
    //  -- or global scope, if nothing found in this scope 

    API_EXPORT(Int, (name),

    BPatch_Vector<BPatch_variableExpr *> *,findVariable,(const char *name));

    //  BPatch_function::getLineToAddr
    //  
    //  method to retrieve addresses corresponding a line in the function

    API_EXPORT(Int, (lineNo, buffer, exactMatch),

    bool,getLineToAddr,(unsigned short lineNo, BPatch_Vector<unsigned long> &buffer,
                        bool exactMatch = true));

    //  BPatch_function::getLineAndFile
    //  
    //  method to return file name, and first and last lines of the func

    API_EXPORT(Int, (start, end, name, length),

    bool,getLineAndFile,(unsigned int &start, unsigned int &end, char *name,
                         unsigned &length));

    //  BPatch_function::getVariables
    //  This returns false, and should probably not exist.  See getVars.
    //  is this defined, what variables should be returned??
    //  FIXME (delete me)

    API_EXPORT(Int, (vect),

    bool,getVariables,(BPatch_Vector<BPatch_variableExpr *> &vect));

    //  BPatch_function::getModuleName
    //  Returns name of module this function belongs to

    API_EXPORT(Int, (name, maxLen),

    char *,getModuleName,(char *name, int maxLen));

    //  BPatch_function::isInstrumentable
    //  
    // Returns true if the function is instrumentable.


    API_EXPORT(Int, (),

    bool,isInstrumentable,());

    //  BPatch_function::isSharedLib
    //  Returns true if this function lives in a shared library

    API_EXPORT(Int, (),

    bool,isSharedLib,());

    //  BPatch_function::getCFG
    //  
    //  method to create the control flow graph for the function

    API_EXPORT(Int, (),

    BPatch_flowGraph*,getCFG,());

#ifdef IBM_BPATCH_COMPAT
    API_EXPORT(Int, (start, end),

    virtual bool,getLineNumbers,(unsigned int &start, unsigned int &end));

    API_EXPORT(Int, (),

    void *,getAddress,());

    API_EXPORT(Int, (start, end),

    bool,getAddressRange,(void * &start, void * &end));

    API_EXPORT(Int, (),

    BPatch_type *,returnType,());

    API_EXPORT(Int, (vect),

    void,getIncPoints,(BPatch_Vector<BPatch_point *> &vect));

    API_EXPORT(Int, (),

    int,getMangledNameLen,());

    API_EXPORT(Int, (points),

    void,getExcPoints,(BPatch_Vector<BPatch_point*> &points));

    // return a function that can be passed as a paramter
    API_EXPORT(Int, (),

    BPatch_variableExpr *,getFunctionRef,());

    API_EXPORT(Int, (),

    const char *,getName,());

#endif
};

#endif /* _BPatch_function_h_ */
