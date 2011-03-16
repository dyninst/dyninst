/*
 * Copyright (c) 1996-2011 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
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

#ifndef _BPatch_function_h_
#define _BPatch_function_h_

#include "Annotatable.h"
#include "BPatch_dll.h"
#include "BPatch_Vector.h"
#include "BPatch_point.h"
#include "BPatch_type.h"
#include "BPatch_module.h"
#include "BPatch_flowGraph.h"
#include "BPatch_eventLock.h"
#include "BPatch_memoryAccess_NP.h"
//#include "BPatch_dependenceGraphNode.h"
// class BPatch_dependenceGraphNode;

class int_function;
class process;
class InstrucIter;

class BPatch_localVarCollection;
class BPatch_function;
class BPatch_point;
class BPatch_flowGraph;

class BPatchTranslatorBase;
class ParameterType;
class ReturnParameterType;

namespace Dyninst {
  namespace ParseAPI {
    class Function;
  };
};

#ifdef DYNINST_CLASS_NAME
#undef DYNINST_CLASS_NAME
#endif
#define DYNINST_CLASS_NAME BPatch_function

class BPATCH_DLL_EXPORT BPatch_function : 
   public BPatch_sourceObj, 
   public BPatch_eventLock,
   public Dyninst::AnnotatableSparse
{
    friend class BPatch_flowGraph;
    friend class InstrucIter;
    friend class BPatch_basicBlock;
    friend class BPatch_asyncEventHandler;
    friend class BPatch_image;
    friend class BPatch_thread;
    friend class BPatch_process;
    friend class BPatch_binaryEdit;
    friend class BPatch_addressSpace;
    friend class BPatch_point;
    friend BPatch_Vector<BPatch_point*> *findPoint(
                     const BPatch_Set<BPatch_opCode>& ops,
						   InstrucIter &ii, 
						   BPatch_process *proc,
						   BPatch_function *bpf);

    //BPatch_process *proc;
    BPatch_addressSpace *addSpace;
    AddressSpace *lladdSpace;
    BPatch_type *retType;
    BPatch_Vector<BPatch_localVar *> params;
    std::map<BPatch_localVar *, BPatch_variableExpr *> local_vars;
    BPatch_module *mod;
    BPatch_flowGraph* cfg;
    bool cfgCreated;
    bool liveInit;

    BPatch_point* createMemInstPoint(void *addr, BPatch_memoryAccess* ma);

    int_function *func;
    bool varsAndParamsValid;

private:
   void constructVarsAndParams();

   void identifyParamDependencies(BPatch_function* callee, void* calleeAddress);

  public:
   //dynC internal use only
   bool hasParamDebugInfo();
public:
    virtual	~BPatch_function();

    // The following are for internal use by the library only:
    int_function *lowlevel_func() const { return func; }
    BPatch_process *getProc() const;
    BPatch_addressSpace *getAddSpace() const { return addSpace; }

    BPatch_function(BPatch_addressSpace *_addSpace, int_function *_func, 
                    BPatch_module *mod = NULL);
    BPatch_function(BPatch_addressSpace *_addSpace, int_function *_func,
                    BPatch_type * _retType, 
                    BPatch_module *);
    bool getSourceObj(BPatch_Vector<BPatch_sourceObj *> &);
    BPatch_sourceObj *getObjParent();
    BPatch_localVarCollection * localVariables;
    BPatch_localVarCollection * funcParameters;
    void setReturnType(BPatch_type * _retType){retType = _retType;}
    void setModule(BPatch_module *module) { if (this->mod == NULL) this->mod = module;}
    void removeCFG() { cfg = NULL; }
    void getUnresolvedControlTransfers(BPatch_Vector<BPatch_point *> &unresolvedCF);
    void getAbruptEndPoints(BPatch_Vector<BPatch_point *> &abruptEnds);
    void getCallerPoints(std::vector<BPatch_point*>& callerPoints);
    void getAllPoints(std::vector<BPatch_point*>& allPoints);
    bool setHandlerFaultAddrAddr(Dyninst::Address addr, bool set);
    void fixHandlerReturnAddr(Dyninst::Address addr);
    bool removeInstrumentation();
    bool parseNewEdge(Dyninst::Address source, Dyninst::Address target);

    void addParam(Dyninst::SymtabAPI::localVar *lvar);

//    void addParam(const char * _name, BPatch_type *_type, int _linenum,
//                  long _frameOffset, int _reg = -1,
//                  BPatch_storageClass _sc = BPatch_storageFrameOffset);
    void fixupUnknown(BPatch_module *);
    

        // This isn't so much for internal use only, but it *should*
        // remain undocumented for now.
    bool containsSharedBlocks();



    // End of functions for internal use only
    

    // For users of the library:

    API_EXPORT(Str, (), std::string, getName, ());

    //  BPatch_function::getName
    //  Returns <demangled> name of function
    API_EXPORT(Buffer, (s, len),     

    char *,getName,(char *s, int len));


    //  BPatch_function::getMangledName
    //  Returns mangled name of function, same as getName for non-c++ mutatees
    API_EXPORT(Int, (s, len),

    char *,getMangledName,(char *s, int len));

    //  BPatch_function::getTypedName
    //  Returns demanged name of function (with type string), may be empty
    API_EXPORT(Int, (s, len),

    char *,getTypedName,(char *s, int len));

    // BPatch_function::getNames
    // Adds all names of the function (inc. weak symbols) to the
    // provided vector. Names are represented as const char *s,
    // and do not require cleanup by the user.

    API_EXPORT(Int, (names),
    bool, getNames, (BPatch_Vector<const char *> &names));

    // BPatch_function::getMangledNames
    // Adds all mangled names of the function (inc. weak symbols) to
    // the provided vector. Names are represented as const char *s,
    // and do not require cleanup by the user.

    API_EXPORT(Int, (names),
    bool, getMangledNames, (BPatch_Vector<const char *> &names));

    //  BPatch_function::getBaseAddr
    //  Returns base address of function
    API_EXPORT(Int, (),

    void *,getBaseAddr,(void));

    //  BPatch_function::getSize
    //  Returns the size of the function in bytes (end of last block - start of first block)
    API_EXPORT(Int, (),

    unsigned int,getSize,());

    //  BPatch_function::getSize
    //  Returns the number of contiguous bytes a function takes up following its entry point
    //   This may be different from getSize if the function is disjoint
    API_EXPORT(Int, (),
    unsigned int,getContiguousSize,());

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

    API_EXPORT(Int, (name, vars),
    bool, findVariable,(const char *name, BPatch_Vector<BPatch_variableExpr*> &vars));

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

    API_EXPORT(Int, (name, isPrimary, isMangled),
    const char *, addName, (const char *name, bool isPrimary = true, bool isMangled = false));           

    //  Return native pointer to the function. 
    //  Allocates and returns a special type of BPatch_variableExpr.
    API_EXPORT( Int, (), BPatch_variableExpr *, getFunctionRef, () );

    // Get all functions that share a block (or any code, but it will
    // always be a block) with this function.
    API_EXPORT( Int, (funcs), bool, findOverlapping, (BPatch_Vector<BPatch_function *> &funcs));

    //  Get the underlying ParseAPI Function
    API_EXPORT( Int, (), Dyninst::ParseAPI::Function *, getParseAPIFunc, () );

#ifdef IBM_BPATCH_COMPAT
    API_EXPORT(Int, (start, end),

    bool,getLineNumbers,(unsigned int &start, unsigned int &end));

    API_EXPORT(Int, (),

    void *,getAddress,());

    API_EXPORT(Int, (start, end),

    bool,getAddressRange,(void * &start, void * &end));

    API_EXPORT(Int, (),

    BPatch_type *,returnType,());

    API_EXPORT_V(Int, (vect),

    void,getIncPoints,(BPatch_Vector<BPatch_point *> &vect));

    API_EXPORT(Int, (),

    int,getMangledNameLen,());

    API_EXPORT_V(Int, (points),

    void,getExcPoints,(BPatch_Vector<BPatch_point*> &points));


    API_EXPORT(DPCL, (),

    const char *,getName,());


#endif
};

#endif /* _BPatch_function_h_ */
