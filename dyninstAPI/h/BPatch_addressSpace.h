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

#ifndef _BPatch_addressSpace_h_
#define _BPatch_addressSpace_h_

#include "boost/shared_ptr.hpp"
#include "BPatch_dll.h"
#include "BPatch_Vector.h"
#include "BPatch_enums.h"
#include "BPatch_instruction.h" // for register type
#include "BPatch_callbacks.h"
#include "dyntypes.h"
#include <string>
#include <utility>
#include <vector>
#include <set>
#include <stdio.h>
#include <signal.h>
#include <boost/iterator/transform_iterator.hpp>
#include "dyntypes.h"
// PatchAPI stuffs
//#include "Command.h"

class BPatch_addressSpace;

namespace Dyninst {
  namespace PatchAPI { 
    class PatchMgr;
    class DynAddrSpace;
    class Patcher;
    class Instance;
    class PatchFunction;
    class Point;
    typedef boost::shared_ptr<PatchMgr> PatchMgrPtr;
    typedef boost::shared_ptr<DynAddrSpace> DynAddrSpacePtr;
    typedef boost::shared_ptr<Instance> InstancePtr;
    BPATCH_DLL_EXPORT PatchMgrPtr convert(const BPatch_addressSpace *);
  }
  namespace SymtabAPI {
    class Symbol;
    struct AddressRange;
  }
}


class BPatch_statement;
class BPatch_snippet;
class BPatch_point;
class BPatch_variableExpr;
class BPatch_type;
class AddressSpace;
class miniTrampHandle;
class BPatch;
class BPatch_image;
class func_instance;
struct batchInsertionRecord;
class instPoint;
class int_variable;

typedef enum{
  TRADITIONAL_PROCESS, STATIC_EDITOR
} processType;


class BPATCH_DLL_EXPORT BPatchSnippetHandle {
  friend class BPatch_point;
  friend class BPatch_image;
  friend class BPatch_process;
  friend class BPatch_binaryEdit;
  friend class BPatch_addressSpace;
  friend class BPatch_thread;

 private:    
  BPatch_addressSpace *addSpace_;

  std::vector<Dyninst::PatchAPI::InstancePtr> instances_;

  BPatch_Vector<BPatch_thread *> catchup_threads;
    
  BPatchSnippetHandle(BPatch_addressSpace * addSpace);

  void addInstance(Dyninst::PatchAPI::InstancePtr p) { instances_.push_back(p); }
    
 public:
 
  ~BPatchSnippetHandle();

  bool usesTrap();
  bool isEmpty() 
  {
    return instances_.empty();
  }
  

  BPatch_function * getFunc ();

  BPatch_addressSpace * getAddressSpace();

  BPatch_process * getProcess();
  typedef BPatch_Vector<BPatch_thread*>::iterator thread_iter;
  thread_iter getCatchupThreads_begin();
  thread_iter getCatchupThreads_end();

  BPatch_Vector<BPatch_thread *> & getCatchupThreads();
  
};

class BPATCH_DLL_EXPORT BPatch_addressSpace {
  friend class BPatch;
  friend class BPatch_image;
  friend class BPatch_function;
  friend class BPatch_frame;
  friend class BPatch_module;
  friend class BPatch_basicBlock;
  friend class BPatch_flowGraph;
  friend class BPatch_loopTreeNode;
  friend class BPatch_point;
  friend class BPatch_funcCallExpr;
  friend class BPatch_eventMailbox;
  friend class BPatch_instruction;
  friend Dyninst::PatchAPI::PatchMgrPtr Dyninst::PatchAPI::convert(const BPatch_addressSpace *);
  
 public:
    
  BPatch_function *findOrCreateBPFunc(Dyninst::PatchAPI::PatchFunction *ifunc, 
                                      BPatch_module *bpmod);

  BPatch_point *findOrCreateBPPoint(BPatch_function *bpfunc, 
                                    Dyninst::PatchAPI::Point *ip,
                                    BPatch_procedureLocation pointType);

  BPatch_variableExpr *findOrCreateVariable(int_variable *v,
                                            BPatch_type *type = NULL);

 protected:
  
  
  static BPatch_function *createBPFuncCB(AddressSpace *p,
                                         Dyninst::PatchAPI::PatchFunction *f);
  static BPatch_point *createBPPointCB(AddressSpace *p,
                                       Dyninst::PatchAPI::PatchFunction *f,
				       Dyninst::PatchAPI::Point *ip, 
                                       int type);

  BPatch_Vector<batchInsertionRecord *> *pendingInsertions;

  BPatch_image *image;

  //  AddressSpace * as;
  
  std::vector<BPatch_register> registers_;

 protected:
  virtual void getAS(std::vector<AddressSpace *> &as) = 0;
  
 public:

  BPatch_addressSpace();


  virtual ~BPatch_addressSpace();

  virtual processType getType() = 0;

  virtual bool getTerminated() = 0;
  virtual bool getMutationsActive() = 0;

  BPatch_module *findModuleByAddr(Dyninst::Address addr);
  bool findFuncsByRange(Dyninst::Address startAddr,
                        Dyninst::Address endAddr,
                        std::set<BPatch_function*> &funcs);

  virtual BPatchSnippetHandle * insertSnippet(const BPatch_snippet &expr, 
					      BPatch_point &point,
					      BPatch_snippetOrder order = BPatch_firstSnippet);
  
  virtual BPatchSnippetHandle* insertSnippet(const BPatch_snippet &expr, 
					     BPatch_point &point,
					     BPatch_callWhen when,
					     BPatch_snippetOrder order = BPatch_firstSnippet);
    
  virtual BPatchSnippetHandle * insertSnippet(const BPatch_snippet &expr,
					      const BPatch_Vector<BPatch_point *> &points,
					      BPatch_snippetOrder order = BPatch_firstSnippet);
    
  virtual BPatchSnippetHandle * insertSnippet(const BPatch_snippet &expr,
					      const BPatch_Vector<BPatch_point *> &points,
					      BPatch_callWhen when,
					      BPatch_snippetOrder order = BPatch_firstSnippet);



  
  virtual void beginInsertionSet() = 0;

  virtual bool finalizeInsertionSet(bool atomic, bool *modified = NULL) = 0;
 

  bool deleteSnippet(BPatchSnippetHandle *handle);

  bool  replaceCode(BPatch_point *point, BPatch_snippet *snippet);

  bool replaceFunctionCall(BPatch_point &point, BPatch_function &newFunc);

  bool removeFunctionCall(BPatch_point &point);

  bool replaceFunction(BPatch_function &oldFunc, BPatch_function &newFunc);

  bool  revertReplaceFunction(BPatch_function &oldFunc);

  bool wrapFunction(BPatch_function *oldFunc, BPatch_function *newFunc, Dyninst::SymtabAPI::Symbol *clone);

  bool revertWrapFunction(BPatch_function *wrappedFunc);

  bool getSourceLines(unsigned long addr, BPatch_Vector< BPatch_statement > & lines );
 
  typedef BPatch_Vector<BPatch_statement>::const_iterator statement_iter;
  statement_iter getSourceLines_begin(unsigned long addr);
  statement_iter getSourceLines_end(unsigned long addr);

 
  bool getAddressRanges(const char * fileName, unsigned int lineNo, std::vector< Dyninst::SymtabAPI::AddressRange> & ranges );

  typedef std::vector<std::pair<unsigned long, unsigned long> >::const_iterator arange_iter;
  statement_iter getAddressRanges_begin(const char* file, unsigned long line);
  statement_iter getAddressRanges_end(const char* file, unsigned long line);

  BPatch_function * findFunctionByEntry(Dyninst::Address entry);

  bool  findFunctionsByAddr(Dyninst::Address addr, 
			    std::vector<BPatch_function*> &funcs);


  BPatch_image * getImage();


  BPatch_variableExpr * malloc(int n, std::string name = std::string(""));

  BPatch_variableExpr * malloc(const BPatch_type &type, std::string name = std::string(""));
  
  BPatch_variableExpr * createVariable(Dyninst::Address at_addr, 
				       BPatch_type *type,
				       std::string var_name = std::string(""),
				       BPatch_module *in_module = NULL);

  bool free(BPatch_variableExpr &ptr);

  BPatch_variableExpr * createVariable(std::string name, 
				       Dyninst::Address addr, 
				       BPatch_type *type = NULL);
  bool  getRegisters(std::vector<BPatch_register> &regs);
  typedef std::vector<BPatch_register>::iterator register_iter;
  register_iter getRegisters_begin();
  register_iter getRegisters_end();
 private:
  void init_registers();
 public:
  bool  createRegister_NP(std::string regName, BPatch_register &reg); 

  void allowTraps(bool allowtraps);

  virtual BPatch_object * loadLibrary(const char *libname, bool reload = false) = 0;

  bool  isStaticExecutable();
};


#endif 
