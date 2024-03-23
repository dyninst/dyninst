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
   bool hasParamDebugInfo();

public:
    virtual	~BPatch_function();

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
    

    bool containsSharedBlocks();



    char * getName(char *s, int len);

    std::string getName();
    std::string getMangledName();
    std::string getDemangledName();
    std::string getTypedName();
    bool getNames(std::vector<std::string> &names);
    bool getDemangledNames(std::vector<std::string> &names);
    bool getMangledNames(std::vector<std::string> &names);
    bool getTypedNames(std::vector<std::string> &names);
    
    char * getMangledName(char *s, int len);

    char * getTypedName(char *s, int len);

    bool  getNames(BPatch_Vector<const char *> &names);

    bool  getMangledNames(BPatch_Vector<const char *> &names);

    void * getBaseAddr(void);

    BPatch_type * getReturnType();

    BPatch_module * getModule();
    
    BPatch_Vector<BPatch_localVar *> * getParams();

    BPatch_Vector<BPatch_localVar *> * getVars();

    BPatch_Vector<BPatch_point *> * findPoint(const BPatch_procedureLocation loc);

    BPatch_Vector<BPatch_point *> * findPoint(const BPatch_Set<BPatch_opCode>& ops);
    BPatch_Vector<BPatch_point *> * findPoint(const std::set<BPatch_opCode>& ops);

    BPatch_point *  findPoint(Dyninst::Address addr);

    BPatch_localVar * findLocalVar(const char * name);

    BPatch_localVar * findLocalParam(const char * name);

    BPatch_Vector<BPatch_variableExpr *> * findVariable(const char *name);

    bool  findVariable(const char *name, BPatch_Vector<BPatch_variableExpr*> &vars);

    bool getVariables(BPatch_Vector<BPatch_variableExpr *> &vect);

    char * getModuleName(char *name, int maxLen);

    bool isInstrumentable();

    bool isSharedLib();

    BPatch_flowGraph* getCFG();

    const char *  addName(const char *name, bool isPrimary = true, bool isMangled = false);           

    operator Dyninst::ParseAPI::Function *() const;

    operator Dyninst::PatchAPI::PatchFunction *() const;

    bool getAddressRange(void * &start, void * &end);

    bool getAddressRange(Dyninst::Address &start, Dyninst::Address &end);

    unsigned int getFootprint();
    BPatch_variableExpr *getFunctionRef();
    bool findOverlapping(BPatch_Vector<BPatch_function *> &funcs);

    bool addMods(std::set<StackMod*>);
};

#endif /* _BPatch_function_h_ */
