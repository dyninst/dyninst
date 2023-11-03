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

#ifndef _BPatch_function_h_
#define _BPatch_function_h_

#include <map>
#include <set>
#include <string>
#include <vector>
#include "Annotatable.h"
#include "BPatch_dll.h"
#include "BPatch_Vector.h"
#include "BPatch_Set.h"
#include "BPatch_enums.h"
#include "BPatch_type.h"
#include "BPatch_module.h"
#include "BPatch_memoryAccess_NP.h"
#include "StackMod.h"
#include "dyntypes.h"

class func_instance;

class BPatch_localVarCollection;
class BPatch_function;
class BPatch_point;
class BPatch_flowGraph;

class BPatchTranslatorBase;
class ParameterType;
class ReturnParameterType;
class BPatch_function;

namespace Dyninst {
  namespace ParseAPI {
    class Function;
     BPATCH_DLL_EXPORT Function *convert(const BPatch_function *);
  }
  namespace PatchAPI {
     class PatchFunction;
     BPATCH_DLL_EXPORT PatchFunction *convert(const BPatch_function *);
  }
}




class BPATCH_DLL_EXPORT BPatch_function : 
   public BPatch_sourceObj, 
   public Dyninst::AnnotatableSparse
{
    friend class BPatch_flowGraph;
    friend class BPatch_basicBlock;
    friend class BPatch_asyncEventHandler;
    friend class BPatch_image;
    friend class BPatch_thread;
    friend class BPatch_process;
    friend class BPatch_binaryEdit;
    friend class BPatch_addressSpace;
    friend class BPatch_point;
    friend Dyninst::ParseAPI::Function *Dyninst::ParseAPI::convert(const BPatch_function *);
    friend Dyninst::PatchAPI::PatchFunction *Dyninst::PatchAPI::convert(const BPatch_function *);

    //BPatch_process *proc;
    BPatch_addressSpace *addSpace;
    AddressSpace *lladdSpace;
    BPatch_type *retType;
    BPatch_module *mod;
    BPatch_flowGraph* cfg;
    bool cfgCreated;
    bool liveInit;

    BPatch_point* createMemInstPoint(void *addr, BPatch_memoryAccess* ma);

    func_instance *func;
    bool varsAndParamsValid;

private:
   void constructVarsAndParams();

   void identifyParamDependencies(BPatch_function* callee, void* calleeAddress);

   std::map<BPatch_localVar *, BPatch_variableExpr *> local_vars;
   BPatch_Vector<BPatch_localVar *> params;

  public:
   //dynC internal use only
   bool hasParamDebugInfo();

public:
    virtual	~BPatch_function();

    // The following are for internal use by the library only:
    func_instance *lowlevel_func() const { return func; }
    BPatch_process *getProc() const;
    BPatch_addressSpace *getAddSpace() const { return addSpace; }

    BPatch_function(BPatch_addressSpace *_addSpace, func_instance *_func, 
                    BPatch_module *mod = NULL);
    BPatch_function(BPatch_addressSpace *_addSpace, func_instance *_func,
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
    void getCallerPoints(BPatch_Vector<BPatch_point*>& callerPoints);
    void getAllPoints(BPatch_Vector<BPatch_point*>& allPoints);

    void getEntryPoints(BPatch_Vector<BPatch_point *> &entryPoints);
    void getExitPoints(BPatch_Vector<BPatch_point *> &entryPoints);
    void getCallPoints(BPatch_Vector<BPatch_point *> &entryPoints);

    bool setHandlerFaultAddrAddr(Dyninst::Address addr, bool set);
    bool removeInstrumentation(bool useInsertionSet);
    bool parseNewEdge(Dyninst::Address source, Dyninst::Address target);
    void relocateFunction();
    bool getSharedFuncs(std::set<BPatch_function*> &funcs);

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


    //  BPatch_function::getName
    //  Returns <demangled> name of function

    char * getName(char *s, int len);

    // String interface to mangled name
    std::string getName();
    std::string getMangledName();
    std::string getDemangledName();
    std::string getTypedName();
    bool getNames(std::vector<std::string> &names);
    bool getDemangledNames(std::vector<std::string> &names);
    bool getMangledNames(std::vector<std::string> &names);
    bool getTypedNames(std::vector<std::string> &names);
    
    //  BPatch_function::getMangledName
    //  Returns mangled name of function, same as getName for non-c++ mutatees

    char * getMangledName(char *s, int len);

    //  BPatch_function::getTypedName
    //  Returns demanged name of function (with type string), may be empty

    char * getTypedName(char *s, int len);

    // BPatch_function::getNames
    // Adds all names of the function (inc. weak symbols) to the
    // provided vector. Names are represented as const char *s,
    // and do not require cleanup by the user.

    bool  getNames(BPatch_Vector<const char *> &names);

    // BPatch_function::getMangledNames
    // Adds all mangled names of the function (inc. weak symbols) to
    // the provided vector. Names are represented as const char *s,
    // and do not require cleanup by the user.

    bool  getMangledNames(BPatch_Vector<const char *> &names);

    //  BPatch_function::getBaseAddr
    //  Returns base address of function

    void * getBaseAddr(void);

    //  BPatch_function::getReturnType
    //  Returns the <BPatch_type> return type of this function


    BPatch_type * getReturnType();

    //  BPatch_function::getModule
    //  Returns the BPatch_module to which this function belongs


    BPatch_module * getModule();
    
    //  BPatch_function::getParams
    //  Returns a vector of BPatch_localVar, representing this function's parameters


    BPatch_Vector<BPatch_localVar *> * getParams();

    //  BPatch_function::getVars
    //  Returns a vector of local variables in this functions


    BPatch_Vector<BPatch_localVar *> * getVars();

    //  BPatch_function::findPoint
    //  Returns a vector of inst points, corresponding to the given BPatch_procedureLocation


    BPatch_Vector<BPatch_point *> * findPoint(const BPatch_procedureLocation loc);

    //  BPatch_function::findPoint
    //  Returns a vector of inst points, corresponding to the given set of op codes

    BPatch_Vector<BPatch_point *> * findPoint(const BPatch_Set<BPatch_opCode>& ops);
    BPatch_Vector<BPatch_point *> * findPoint(const std::set<BPatch_opCode>& ops);

    //  BPatch_function::findPoint
    //
    //  Returns a BPatch_point that corresponds with the provided address. Returns NULL
    //  if the address does not correspond with an instruction. 
    BPatch_point *  findPoint(Dyninst::Address addr);


    //  BPatch_function::findLocalVar
    //  Returns a BPatch_localVar, if a match for <name> is found


    BPatch_localVar * findLocalVar(const char * name);

    //  BPatch_function::findLocalParam
    //  Returns a BPatch_localVar, if a match for <name> is found


    BPatch_localVar * findLocalParam(const char * name);

    //  BPatch_function::findVariable
    //  Returns a set of variables matching <name> at the scope of this function
    //  -- or global scope, if nothing found in this scope 

    BPatch_Vector<BPatch_variableExpr *> * findVariable(const char *name);

    bool  findVariable(const char *name, BPatch_Vector<BPatch_variableExpr*> &vars);

    //  BPatch_function::getVariables
    //  This returns false, and should probably not exist.  See getVars.
    //  is this defined, what variables should be returned??
    //  FIXME (delete me)


    bool getVariables(BPatch_Vector<BPatch_variableExpr *> &vect);

    //  BPatch_function::getModuleName
    //  Returns name of module this function belongs to


    char * getModuleName(char *name, int maxLen);

    //  BPatch_function::isInstrumentable
    //  
    // Returns true if the function is instrumentable.



    bool isInstrumentable();

    //  BPatch_function::isSharedLib
    //  Returns true if this function lives in a shared library


    bool isSharedLib();

    //  BPatch_function::getCFG
    //  
    //  method to create the control flow graph for the function


    BPatch_flowGraph* getCFG();

    const char *  addName(const char *name, bool isPrimary = true, bool isMangled = false);           

    //  Return native pointer to the function. 
    //  Allocates and returns a special type of BPatch_variableExpr.

    // Get all functions that share a block (or any code, but it will
    // always be a block) with this function.

    //  Get the underlying ParseAPI Function
    operator Dyninst::ParseAPI::Function *() const;

    // Get the underlying PatchAPI Function
    operator Dyninst::PatchAPI::PatchFunction *() const;

    bool getAddressRange(void * &start, void * &end);

    bool getAddressRange(Dyninst::Address &start, Dyninst::Address &end);

    unsigned int getFootprint();
    BPatch_variableExpr *getFunctionRef();
    bool findOverlapping(BPatch_Vector<BPatch_function *> &funcs);

    // Add stack modifications
    bool addMods(std::set<StackMod*>);
};

#endif /* _BPatch_function_h_ */
