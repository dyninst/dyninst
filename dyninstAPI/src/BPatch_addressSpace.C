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

#define BPATCH_FILE

#include "binaryEdit.h"
#include "inst.h"
#include "instP.h"
#include "function.h" // Dyninst::PatchAPI::PatchFunction
#include "codeRange.h"
#include "addressSpace.h"
#include "dynProcess.h"
#include "debug.h"

#include "mapped_module.h"

#include "BPatch_libInfo.h"
#include "BPatch.h"
#include "BPatch_thread.h"
#include "BPatch_function.h"
#include "BPatch_point.h"

#include "BPatch_private.h"

#include "ast.h"

#include "BPatch_addressSpace.h"

#include "BPatch_instruction.h"

#include "mapped_object.h"

#include <sstream>
#include "Parsing.h"

#include "Command.h"
#include "Relocation/DynInstrumenter.h"

#include "PatchMgr.h"

using Dyninst::PatchAPI::Patcher;
using Dyninst::PatchAPI::DynInsertSnipCommand;
using Dyninst::PatchAPI::DynReplaceFuncCommand;
using Dyninst::PatchAPI::DynModifyCallCommand;
using Dyninst::PatchAPI::DynRemoveCallCommand;

BPatch_addressSpace::BPatch_addressSpace() :
   pendingInsertions(NULL), image(NULL)
{
}

BPatch_addressSpace::~BPatch_addressSpace()
{}


BPatch_function *BPatch_addressSpace::findOrCreateBPFunc(Dyninst::PatchAPI::PatchFunction* ifunc,
                                                         BPatch_module *bpmod)
{
   func_instance *fi = static_cast<func_instance *>(ifunc);
   if (!bpmod)
      bpmod = image->findOrCreateModule(fi->mod());
   assert(bpmod);
   if (bpmod->func_map.count(ifunc)) {
      BPatch_function *bpf = bpmod->func_map[ifunc];
      assert(bpf);
      assert(bpf->func == ifunc);
      return bpf;
   }

   // check to see if the func_instance refers to a different
   // module, and that module contains a bpatch_func
   BPatch_module* containing = NULL;
   if (fi->mod() != NULL) {
      containing = getImage()->findModule(fi->mod()->fileName().c_str());
   }

   // findModule has a tendency to make new function objects... so
   // check the map again
   if (containing->func_map.count(ifunc)) {
      BPatch_function *bpf = containing->func_map[ifunc];
      assert(bpf);
      assert(bpf->func == ifunc);
      return bpf;
   }

   BPatch_function *ret = new BPatch_function(this, fi, bpmod);
   assert( ret != NULL );
   assert(ret->func == ifunc);
   return ret;
}




BPatch_point *BPatch_addressSpace::findOrCreateBPPoint(BPatch_function *bpfunc,
                                                       Dyninst::PatchAPI::Point *p,
                                                       BPatch_procedureLocation pointType)
{
   instPoint *ip = static_cast<instPoint *>(p);
   assert(ip);
   func_instance *fi = static_cast<func_instance *>(ip->func());
   if (!fi) return NULL; // PatchAPI can create function-less points, but we can't represent them as BPatch_points

   BPatch_module *mod = image->findOrCreateModule(fi->mod());
   assert(mod);

   if (mod->instp_map.count(ip))
      return mod->instp_map[ip];

   if (pointType == BPatch_locUnknownLocation) {
      cerr << "Error: point type not specified!" << endl;
      assert(0);
      return NULL;
   }

   AddressSpace *lladdrSpace = fi->proc();
   if (!bpfunc)
      bpfunc = findOrCreateBPFunc(fi, mod);

   assert(bpfunc->func == ip->func());
   std::pair<instPoint *, instPoint *> pointsToUse = instPoint::getInstpointPair(ip);

   BPatch_point *pt = new BPatch_point(this, bpfunc,
                                       pointsToUse.first, pointsToUse.second,
                                       pointType, lladdrSpace);
   mod->instp_map[ip] = pt;

   return pt;
}

BPatch_variableExpr *BPatch_addressSpace::findOrCreateVariable(int_variable *v,
                                                               BPatch_type *type)
{
   BPatch_module *mod = image->findOrCreateModule(v->mod());
   assert(mod);
   if (mod->var_map.count(v))
      return mod->var_map[v];

   if (!type) {
      auto stype = v->ivar()->svar()->getType(Dyninst::SymtabAPI::Type::share);

      if (stype){
         type = BPatch_type::findOrCreateType(stype);
      }else{
         type = BPatch::bpatch->type_Untyped;
      }
   }

   BPatch_variableExpr *var = BPatch_variableExpr::makeVariableExpr(this, v, type);
   mod->var_map[v] = var;
   return var;
}



BPatch_function *BPatch_addressSpace::createBPFuncCB(AddressSpace *a, Dyninst::PatchAPI::PatchFunction *f)
{
   BPatch_addressSpace *aS = (BPatch_addressSpace *)a->up_ptr();
   assert(aS);
   return aS->findOrCreateBPFunc(f, NULL);
}

BPatch_point *BPatch_addressSpace::createBPPointCB(AddressSpace *a,
                                                   Dyninst::PatchAPI::PatchFunction *pf,
                                                   Dyninst::PatchAPI::Point *p, int type)
{
   func_instance *fi = static_cast<func_instance *>(pf);
   instPoint *ip  = static_cast<instPoint *>(p);
   BPatch_addressSpace *aS = (BPatch_addressSpace *)a->up_ptr();
   assert(aS);

   BPatch_module *bpmod = aS->getImage()->findOrCreateModule(fi->mod());
   assert(bpmod);

   BPatch_function *func = aS->findOrCreateBPFunc(fi, bpmod);
   assert(func);

   return aS->findOrCreateBPPoint(func, ip, (BPatch_procedureLocation) type);
}



/***************************************************************************
 * Bpatch_snippetHandle
 ***************************************************************************/

/*
 * BPatchSnippetHandle::BPatchSnippetHandle
 *
 * Constructor for BPatchSnippetHandle.  Delete the snippet instance(s)
 * associated with the BPatchSnippetHandle.
 */
BPatchSnippetHandle::BPatchSnippetHandle(BPatch_addressSpace * addSpace) :
   addSpace_(addSpace)
{
}

/*
 * BPatchSnippetHandle::~BPatchSnippetHandle
 *
 * Destructor for BPatchSnippetHandle.  Delete the snippet instance(s)
 * associated with the BPatchSnippetHandle.
 */
BPatchSnippetHandle::~BPatchSnippetHandle()
{
   // don't delete inst instances since they are might have been copied
}

BPatch_addressSpace *BPatchSnippetHandle::getAddressSpace()
{
   return addSpace_;
}

BPatch_process *BPatchSnippetHandle::getProcess()
{
   return dynamic_cast<BPatch_process *>(addSpace_);
}

BPatch_Vector<BPatch_thread *> &BPatchSnippetHandle::getCatchupThreads()
{
   return catchup_threads;
}

BPatchSnippetHandle::thread_iter BPatchSnippetHandle::getCatchupThreads_begin()
{
    return catchup_threads.begin();
}
BPatchSnippetHandle::thread_iter BPatchSnippetHandle::getCatchupThreads_end()
{
    return catchup_threads.end();
}

// Return true if any sub-minitramp uses a trap? Other option
// is "if all"...
bool BPatchSnippetHandle::usesTrap() {
    return false;
}

BPatch_function * BPatchSnippetHandle::getFunc()
{
    if (!instances_.empty()) {
       Dyninst::PatchAPI::PatchFunction *func = instances_.back()->point()->func();
       BPatch_function *bpfunc = addSpace_->findOrCreateBPFunc(func,NULL);
       return bpfunc;
    }
    return NULL;
}


BPatch_image * BPatch_addressSpace::getImage()
{
   return image;
}


/*
 * BPatch_addressSpace::deleteSnippet
 *
 * Deletes an instance of a snippet.
 *
 * handle       The handle returned by insertSnippet when the instance to
 *              deleted was created.
 */

bool BPatch_addressSpace::deleteSnippet(BPatchSnippetHandle *handle)
{
   if (getTerminated()) return true;

   if (handle == NULL) {
       bperr("Request to delete NULL snippet handle, returning false\n");
       return false;
   }

   if (handle->addSpace_ != this) {
     bperr("Error: wrong address space in deleteSnippet\n");
     return false;
   }

   mal_printf("deleting snippet handle from func at %lx, point at %lx of type %d\n",
              (Address)handle->getFunc()->getBaseAddr(), 
              handle->instances_.empty() ? 0 : handle->instances_[0]->point()->addr(),
              handle->instances_.empty() ? -1 : static_cast<int>(handle->instances_[0]->point()->type()));

   // if this is a process, check to see if the instrumentation is
   // executing on the call stack
   instPoint *ip = static_cast<instPoint *>(handle->instances_[0]->point());

   if ( handle->getProcess() && handle->instances_.size() > 0 &&
       BPatch_normalMode !=
        ip->func()->obj()->hybridMode())
   {
       if (handle->instances_.size() > 1) {
           mal_printf("ERROR: Removing snippet that is installed in "
                          "multiple miniTramps %s[%d]\n",FILE__,__LINE__);
     }
   }

   // uninstrument and remove snippet handle from point datastructures
   for (unsigned int i=0; i < handle->instances_.size(); i++)
     {
        uninstrument(handle->instances_[i]);
        Dyninst::PatchAPI::Point *iPoint = handle->instances_[i]->point();
        BPatch_point *bPoint = findOrCreateBPPoint(NULL, iPoint,
                                                   BPatch_point::convertInstPointType_t(iPoint->type()));
        assert(bPoint);
        bPoint->removeSnippet(handle);
     }

   handle->instances_.clear();

   if (pendingInsertions == NULL) {
     // Trigger it now
     bool tmp;
     finalizeInsertionSet(false, &tmp);
   }

   //delete handle;
   return true;
}

/*
 * BPatch_addressSpace::replaceCode
 *
 * Replace a given instruction with a BPatch_snippet.
 *
 * point       Represents the instruction to be replaced
 * snippet     The replacing snippet
 */

bool BPatch_addressSpace::replaceCode(BPatch_point * /*point*/,
                                         BPatch_snippet * /*snippet*/)
{
   // Need to reevaluate how this code works. I don't think it should be
   // point-based, though.

   assert(0);
   return false;
}

/*
 * BPatch_addressSpace::replaceFunctionCall
 *
 * Replace a function call with a call to a different function.  Returns true
 * upon success, false upon failure.
 *
 * point        The call site that is to be changed.
 * newFunc      The function that the call site will now call.
 */
bool BPatch_addressSpace::replaceFunctionCall(BPatch_point &point,
      BPatch_function &newFunc)
{
   char name[1024];
   newFunc.getName(name, 1024);

   // Can't make changes to code when mutations are not active.
   if (!getMutationsActive())
      return false;

   assert(point.point && newFunc.lowlevel_func());

  /* PatchAPI stuffs */
   AddressSpace* addr_space = point.getAS();
   DynModifyCallCommand* rep_call = DynModifyCallCommand::create(addr_space,
      point.point->block(), newFunc.lowlevel_func(), point.point->func());
  addr_space->patcher()->add(rep_call);
  /* End of PatchAPI */

   if (pendingInsertions == NULL) {
     // Trigger it now
     bool tmp;
     finalizeInsertionSet(false, &tmp);
   }
   return true;
}

/*
 * BPatch_addressSpace::removeFunctionCall
 *
 * Replace a function call with a NOOP.  Returns true upon success, false upon
 * failure.
 *
 * point        The call site that is to be NOOPed out.
 */
bool BPatch_addressSpace::removeFunctionCall(BPatch_point &point)
{
   // Can't make changes to code when mutations are not active.
   if (!getMutationsActive())
      return false;

   assert(point.point);

  /* PatchAPI stuffs */
   AddressSpace* addr_space = point.getAS();
   DynRemoveCallCommand* remove_call = DynRemoveCallCommand::create(addr_space,
      point.point->block(), point.point->func());
  addr_space->patcher()->add(remove_call);
  /* End of PatchAPI */

   if (pendingInsertions == NULL) {
     // Trigger it now
     bool tmp;
     finalizeInsertionSet(false, &tmp);
   }

   return true;
}


/*
 * BPatch_addressSpace::replaceFunction
 *
 * Replace all calls to function OLDFUNC with calls to NEWFUNC.
 * Returns true upon success, false upon failure.
 *
 * oldFunc      The function to replace
 * newFunc      The replacement function
 */
bool BPatch_addressSpace::replaceFunction(BPatch_function &oldFunc,
      BPatch_function &newFunc)
{
  assert(oldFunc.lowlevel_func() && newFunc.lowlevel_func());
  if (!getMutationsActive())
    return false;

  // Self replacement is a nop
  // We should just test direct equivalence here...
  if (oldFunc.lowlevel_func() == newFunc.lowlevel_func()) {
    return true;
  }

  /* PatchAPI stuffs */
  AddressSpace* addr_space = oldFunc.lowlevel_func()->proc();
  DynReplaceFuncCommand* rep_func = DynReplaceFuncCommand::create(addr_space,
     oldFunc.lowlevel_func(), newFunc.lowlevel_func());
  addr_space->patcher()->add(rep_func);
  /* End of PatchAPI */

  if (pendingInsertions == NULL) {
    // Trigger it now
    bool tmp;
    finalizeInsertionSet(false, &tmp);
  }
  return true;
}

/*
 * BPatch_addressSpace::revertReplaceFunction
 *
 * Undoes a replaceFunction operation
 */
bool BPatch_addressSpace::revertReplaceFunction(BPatch_function &oldFunc)
{
  assert(oldFunc.lowlevel_func());
  if (!getMutationsActive())
    return false;

  func_instance *func = oldFunc.lowlevel_func();

  func->proc()->revertReplacedFunction(func);

  if (pendingInsertions == NULL) {
    // Trigger it now
    bool tmp;
    finalizeInsertionSet(false, &tmp);
  }
  return true;
}

bool BPatch_addressSpace::wrapFunction(BPatch_function *original,
                                          BPatch_function *wrapper,
                                          Dyninst::SymtabAPI::Symbol *clone)
{
   assert(original->lowlevel_func() && wrapper->lowlevel_func());
   if (!getMutationsActive())
      return false;

   // Self replacement is a nop
   // We should just test direct equivalence here...
   if (original->lowlevel_func() == wrapper->lowlevel_func()) {
      return true;
   }

   if (!original->lowlevel_func()->proc()->wrapFunction(original->lowlevel_func(), 
                                                      wrapper->lowlevel_func(),
                                                      clone))
      return false;

   if (pendingInsertions == NULL) {
      // Trigger it now
      bool tmp;
      finalizeInsertionSet(false, &tmp);
   }
   return true;
}

bool BPatch_addressSpace::revertWrapFunction(BPatch_function *original)
{
   assert(original->lowlevel_func());

   func_instance *func = original->lowlevel_func();
   assert(func);

   func->proc()->revertWrapFunction(func);
   
   if (pendingInsertions == NULL) {
      // Trigger it now
      bool tmp;
      finalizeInsertionSet(false, &tmp);
   }
   return true;
}


bool BPatch_addressSpace::getAddressRanges( const char * fileName,
      unsigned int lineNo,
      std::vector< SymtabAPI::AddressRange > & ranges )
{
   unsigned int originalSize = ranges.size();
   image->getAddressRanges(fileName, lineNo, ranges);

   //   BPatch_Vector< BPatch_module * > * modules = image->getModules();

   /* Iteratate over the modules, looking for addr in each. */
   //for ( unsigned int i = 0; i < modules->size(); i++ ) {
   //   BPatch_module *m = (*modules)[i];
   //   m->getAddressRanges(fileName, lineNo, ranges);
   //}
   if ( ranges.size() != originalSize ) { return true; }

   return false;
} /* end getAddressRanges() */


bool BPatch_addressSpace::getSourceLines( unsigned long addr,
      BPatch_Vector< BPatch_statement > & lines )
{
   return image->getSourceLines(addr, lines);
} /* end getLineAndFile() */


/*
 * BPatch_process::malloc
 *
 * Allocate memory in the thread's address space.
 *
 * n    The number of bytes to allocate.
 *
 * Returns:
 *      A pointer to a BPatch_variableExpr representing the memory.
 *
 * If otherwise unspecified when binary rewriting, then the allocation
 * happens in the original object.
 */

BPatch_variableExpr *BPatch_addressSpace::malloc(int n, std::string name)
{
   std::vector<AddressSpace *> as;
   assert(BPatch::bpatch != NULL);
   getAS(as);
   assert(as.size());
   void *ptr = (void *) as[0]->inferiorMalloc(n, dataHeap);
   if (!ptr) return NULL;
   if(name.empty()){
      std::stringstream namestr;
      namestr << "dyn_malloc_0x" << std::hex << ptr << "_" << n << "_bytes";
      name = namestr.str();
   }
   BPatch_type *type = BPatch::bpatch->createScalar(name.c_str(), n);

   return BPatch_variableExpr::makeVariableExpr(this, as[0], name, ptr,
                                                type);
}


/*
 * BPatch_process::malloc
 *
 * Allocate memory in the thread's address space for a variable of the given
 * type.
 *
 * type         The type of variable for which to allocate space.
 *
 * Returns:
 *      A pointer to a BPatch_variableExpr representing the memory.
 *
 * XXX Should return NULL on failure, but the function which it calls,
 *     inferiorMalloc, calls exit rather than returning an error, so this
 *     is not currently possible.
 */

BPatch_variableExpr *BPatch_addressSpace::malloc(const BPatch_type &type, std::string name)
{
   std::vector<AddressSpace *> as;
   assert(BPatch::bpatch != NULL);
   getAS(as);
   assert(as.size());
   BPatch_type &t = const_cast<BPatch_type &>(type);
   void *mem = (void *) as[0]->inferiorMalloc(t.getSize(), dataHeap);
   if (!mem) return NULL;
   if(name.empty()){
      std::stringstream namestr;
      namestr << "dyn_malloc_0x" << std::hex << mem << "_" << type.getName();
      name = namestr.str();
   }
   BPatch_variableExpr *varExpr = BPatch_variableExpr::makeVariableExpr(this, as[0], name, mem, &t);
   return varExpr;
}

/*
 * BPatch_process::free
 *
 * Free memory that was allocated with BPatch_process::malloc.
 *
 * ptr          A BPatch_variableExpr representing the memory to free.
 */

bool BPatch_addressSpace::free(BPatch_variableExpr &ptr)
{
  if(ptr.intvar)
  {
    // kill the symbols

  }

   ptr.getAS()->inferiorFree((Address)ptr.getBaseAddr());
   return true;
}

BPatch_variableExpr *BPatch_addressSpace::createVariable(std::string name,
                                                            Dyninst::Address addr,
                                                            BPatch_type *type) {
    assert(BPatch::bpatch != NULL);
    std::vector<AddressSpace *> as;
    getAS(as);
    assert(as.size());

//dynC added feature
    if(strstr(name.c_str(), "dynC") == name.c_str()){
       void *mem = (void *) as[0]->inferiorMalloc(type->getSize(), dataHeap);
       if (!mem) return NULL;
       BPatch_variableExpr *varExpr = BPatch_variableExpr::makeVariableExpr(this, as[0], name, mem, type);
       BPatch_module *mod = image->findOrCreateModule(varExpr->intvar->mod());
       assert(mod);
       mod->var_map[varExpr->intvar] = varExpr;
       return varExpr;
    }

    BPatch_variableExpr *varExpr = BPatch_variableExpr::makeVariableExpr(this,
                                                 as[0],
                                                 name,
                                                 (void *)addr,
                                                 type);

    return varExpr;
}

/*
 *  BPatch_addressSpace::findFunctionByEntry
 *
 *  Returns the function starting at the given address, or NULL if the
 *  address is not within a function.
 *
 *  entry       The address to use for the lookup.
 */
BPatch_function *BPatch_addressSpace::findFunctionByEntry(Address entry)
{
    vector<BPatch_function*> funcs;
    findFunctionsByAddr(entry, funcs);
    vector<BPatch_function*>::iterator fit;
    for (fit = funcs.begin(); fit != funcs.end(); fit++) {
        if (entry == (Address)(*fit)->getBaseAddr()) {
            return *fit;
        }
    }
    return NULL;
}

bool BPatch_addressSpace::findFuncsByRange(Address startAddr,
                                           Address endAddr,
                                           std::set<BPatch_function*> &bpFuncs)
{
    std::vector<AddressSpace *> as;
    getAS(as);
    assert(as.size());

    // find the first code range in the region
    mapped_object* mobj = as[0]->findObject(startAddr);
    assert(mobj);
    set<func_instance*> intFuncs;
    mobj->findFuncsByRange(startAddr,endAddr,intFuncs);
    set<func_instance*>::iterator fIter = intFuncs.begin();
    for (; fIter != intFuncs.end(); fIter++) {
        BPatch_function * bpfunc = findOrCreateBPFunc(*fIter,NULL);
        bpFuncs.insert(bpfunc);
    }
    return 0 != bpFuncs.size();
}


/*
 * BPatch_addressSpace::findFunctionsByAddr
 *
 * Returns the functions that contain the specified address, or NULL if the
 * address is not within a function. (there could be multiple functions
 * because of the possibility of shared code)
 *
 * addr         The address to use for the lookup.
 * returns false if there were no functions that matched the address
 */
bool BPatch_addressSpace::findFunctionsByAddr(Address addr, std::vector<BPatch_function*> &funcs)
{
    std::vector<AddressSpace *> as;
    getAS(as);
    assert(as.size());

    // grab the funcs, return false if there aren't any
    std::set<func_instance*> intfuncs;
    if (!as[0]->findFuncsByAddr( addr, intfuncs )) {
        return false;
    }
    // convert to BPatch_functions
    for (std::set<func_instance *>::iterator fiter=intfuncs.begin();
         fiter != intfuncs.end(); fiter++)
    {
        funcs.push_back(findOrCreateBPFunc(*fiter, NULL));
    }
    return 0 < funcs.size();
}


/*
 * BPatch_addressSpace::findModuleByAddr
 *
 * Returns the module that contains the specified address, or NULL if the
 * address is not within a module.  Does NOT trigger parsing
 *
 * addr         The address to use for the lookup.
 */
BPatch_module *BPatch_addressSpace::findModuleByAddr(Address addr)
{
   std::vector<AddressSpace *> as;
   getAS(as);
   assert(as.size());

   mapped_object *obj = as[0]->findObject(addr);
   if ( ! obj )
       return NULL;

   const std::vector<mapped_module*> mods = obj->getModules();
   if (mods.size()) {
       return getImage()->findOrCreateModule(mods[0]);
   }
   return NULL;
}


/*
 * BPatch_addressSpace::insertSnippet
 *
 * Insert a code snippet at a given instrumentation point.  Upon success,
 * returns a handle to the created instance of the snippet, which can be used
 * to delete it.  Otherwise returns NULL.
 *
 * expr         The snippet to insert.
 * point        The point at which to insert it.
 */

BPatchSnippetHandle *BPatch_addressSpace::insertSnippet(const BPatch_snippet &expr,
      BPatch_point &point,
      BPatch_snippetOrder order)
{
   BPatch_callWhen when;
   if (point.getPointType() == BPatch_exit)
      when = BPatch_callAfter;
   else
      when = BPatch_callBefore;

   return insertSnippet(expr, point, when, order);
}

/*
 * BPatch_addressSpace::insertSnippet
 *
 * Insert a code snippet at a given instrumentation point.  Upon succes,
 * returns a handle to the created instance of the snippet, which can be used
 * to delete it.  Otherwise returns NULL.
 *
 * expr         The snippet to insert.
 * point        The point at which to insert it.
 */

// This handles conversion without requiring inst.h in a header file...
extern bool BPatchToInternalArgs(BPatch_point *point,
      BPatch_callWhen when,
      BPatch_snippetOrder order,
      callWhen &ipWhen,
      callOrder &ipOrder);


BPatchSnippetHandle *BPatch_addressSpace::insertSnippet(const BPatch_snippet &expr,
      BPatch_point &point,
      BPatch_callWhen when,
      BPatch_snippetOrder order)
{
   BPatch_Vector<BPatch_point *> points;
   points.push_back(&point);
   return insertSnippet(expr,
         points,
         when,
         order);
}

extern int dyn_debug_ast;

/*
 * BPatch_addressSpace::insertSnippet
 *
 * Insert a code snippet at each of a list of instrumentation points.  Upon
 * success, Returns a handle to the created instances of the snippet, which
 * can be used to delete them (as a unit).  Otherwise returns NULL.
 *
 * expr         The snippet to insert.
 * points       The list of points at which to insert it.
 */

// A lot duplicated from the single-point version. This is unfortunate.
BPatchSnippetHandle *BPatch_addressSpace::insertSnippet(const BPatch_snippet &expr,
                                                                    const BPatch_Vector<BPatch_point *> &points,
                                                                    BPatch_callWhen when,
                                                                    BPatch_snippetOrder order)
{
  BPatchSnippetHandle *retHandle = new BPatchSnippetHandle(this);

  if (dyn_debug_inst) {
      BPatch_function *f;
      for (unsigned i=0; i<points.size(); i++) {
         f = points[i]->getFunction();
         const string sname = f->func->prettyName();
         inst_printf("[%s:%d] - %u. Insert instrumentation at function %s, "
               "address %p, when %d, order %d\n",
               FILE__, __LINE__, i,
               sname.c_str(), points[i]->getAddress(), (int) when, (int) order);

      }
  }

  if (BPatch::bpatch->isTypeChecked()) {
      if (expr.ast_wrapper->checkType() == BPatch::bpatch->type_Error) {
	fprintf(stderr, "[%s:%d] - Type error inserting instrumentation\n",
		FILE__, __LINE__);
        //expr.ast_wrapper->debugPrint();
         return NULL;
      }
   }

   if (!points.size()) {
      inst_printf("%s[%d]:  request to insert snippet at zero points!\n", FILE__, __LINE__);
      return NULL;
   }

   for (unsigned i = 0; i < points.size(); i++) {
      BPatch_point *bppoint = points[i];

      if (bppoint->addSpace == NULL) {
         fprintf(stderr, "Error: attempt to use point with no process info\n");
         continue;
      }

      if (dynamic_cast<BPatch_addressSpace *>(bppoint->addSpace) != this) {
         fprintf(stderr, "Error: attempt to use point specific to a different process\n");
         continue;
      }

      callWhen ipWhen;
      callOrder ipOrder;

      if (!BPatchToInternalArgs(bppoint, when, order, ipWhen, ipOrder)) {
        fprintf(stderr, "[%s:%d] - BPatchToInternalArgs failed for point %u\n",
               FILE__, __LINE__, i);
         return retHandle;
      }
      if(!expr.checkTypesAtPoint(bppoint)) 
      {
	continue;
      }

      /* PatchAPI stuffs */
      instPoint *ipoint = static_cast<instPoint *>(bppoint->getPoint(when));
      Dyninst::PatchAPI::InstancePtr instance = (ipOrder == orderFirstAtPoint) ?
         ipoint->pushFront(expr.ast_wrapper) :
         ipoint->pushBack(expr.ast_wrapper);
      /* End of PatchAPI stuffs */
      if (instance) {
         if (BPatch::bpatch->isTrampRecursive()) {
            instance->disableRecursiveGuard();
         }
         retHandle->addInstance(instance);
         bppoint->recordSnippet(when, order, retHandle);
      }
   }
   if (pendingInsertions == NULL) {
     // There's no insertion set, instrument now
     bool tmp;
     if (!finalizeInsertionSet(false, &tmp)) {
        return NULL;
     }
   }   
   // If we inserted nothing successfully, NULL
   if(retHandle->isEmpty()) return NULL;
   
   return retHandle;
}


/*
 * BPatch_addressSpace::insertSnippet
 *
 * Insert a code snippet at each of a list of instrumentation points.  Upon
 * success, Returns a handle to the created instances of the snippet, which
 * can be used to delete them (as a unit).  Otherwise returns NULL.
 *
 * expr         The snippet to insert.
 * points       The list of points at which to insert it.
 */

BPatchSnippetHandle *BPatch_addressSpace::insertSnippet(
      const BPatch_snippet &expr,
      const BPatch_Vector<BPatch_point *> &points,
      BPatch_snippetOrder order)
{
   return insertSnippet(expr,
         points,
         BPatch_callUnset,
         order);
}

/*
 * BPatch_addressSpace::isStaticExecutable
 *
 * Returns true if the underlying image represents a statically-linked executable, false otherwise.
 */
bool BPatch_addressSpace::isStaticExecutable() {
   std::vector<AddressSpace *> as;
   getAS(as);

   if( !as.size() ) return false;

   AddressSpace *aout = as[0];
   return aout->getAOut()->isStaticExec();
}

#include "registerSpace.h"

#if defined(cap_registers)
void BPatch_addressSpace::init_registers()
{
    if(registers_.size()) return;
    std::vector<AddressSpace *> as;
    
    getAS(as);
    assert(as.size());
    
    registerSpace *rs = registerSpace::getRegisterSpace(as[0]);
    
    for (unsigned i = 0; i < rs->realRegs().size(); i++) {
	// Let's do just GPRs for now
	registerSlot *regslot = rs->realRegs()[i];
	registers_.push_back(BPatch_register(regslot->name, regslot->number));
    }
    
    // Temporary override: also return EFLAGS though it's certainly not a 
#if defined(arch_x86) || defined(arch_x86_64)
    for (unsigned i = 0; i < rs->SPRs().size(); ++i) {
	if (rs->SPRs()[i]->name == "eflags") {
	    registers_.push_back(BPatch_register(rs->SPRs()[i]->name, 
						 rs->SPRs()[i]->number));
	}
    }
#endif
}

bool BPatch_addressSpace::getRegisters(std::vector<BPatch_register> &regs) {
   init_registers();
   regs = registers_;
   return true;
}
BPatch_addressSpace::register_iter BPatch_addressSpace::getRegisters_begin()
{
    init_registers();
    return registers_.begin();
}
BPatch_addressSpace::register_iter BPatch_addressSpace::getRegisters_end()
{
    init_registers();
    return registers_.end();
}
#else
void BPatch_addressSpace::init_registers() {}
bool BPatch_addressSpace::getRegisters(std::vector<BPatch_register> &) {
    // Empty vector since we're not supporting register objects on
    // these platforms (yet)
   return false;
}
BPatch_addressSpace::register_iter BPatch_addressSpace::getRegisters_begin()
{
    init_registers();
    return registers_.begin();
}
BPatch_addressSpace::register_iter BPatch_addressSpace::getRegisters_end()
{
    init_registers();
    return registers_.end();
}
#endif

#if defined(cap_registers)
bool BPatch_addressSpace::createRegister_NP(std::string regName,
                                               BPatch_register &reg) {
    // Build the register list.
    std::vector<BPatch_register> dontcare;
    getRegisters(dontcare);

    for (unsigned i = 0; i < registers_.size(); i++) {
        if (registers_[i].name() == regName) {
            reg = registers_[i];
            return true;
        }
    }
    return false;
}
#else
bool BPatch_addressSpace::createRegister_NP(std::string,
                                               BPatch_register &)
{
   return false;
}
#endif

void BPatch_addressSpace::allowTraps(bool allowtraps)
{
   std::vector<AddressSpace *> as;
   getAS(as);

   for (std::vector<AddressSpace *>::iterator i = as.begin(); i != as.end(); i++)
   {
      (*i)->setUseTraps(allowtraps);
   }
}

BPatch_variableExpr *BPatch_addressSpace::createVariable(
                        Dyninst::Address at_addr,
                        BPatch_type *type, std::string var_name,
                        BPatch_module *in_module)
{
   BPatch_binaryEdit *binEdit = dynamic_cast<BPatch_binaryEdit *>(this);
   if (binEdit && !in_module) {
      //Address alone isn't unique when binary rewriting
      return NULL;
   }
   if (!type) {
      //Required for size information.
      return NULL;
   }
   AddressSpace *ll_addressSpace = NULL;

   std::vector<AddressSpace *> as;
   getAS(as);
   if (binEdit) {
      std::vector<AddressSpace *>::iterator as_i;
      for (as_i = as.begin(); as_i != as.end(); as_i++)
      {
         BinaryEdit *b = dynamic_cast<BinaryEdit *>(*as_i);
         assert(b);
         if (in_module->mod->obj() == b->getMappedObject()) {
            ll_addressSpace = *as_i;
            break;
         }
      }
   }
   else {
      assert(as.size() == 1);
      ll_addressSpace = as[0];
   }

   if (!ll_addressSpace) {
      //in_module doesn't belong to 'this'
      return NULL;
   }

   if (!var_name.size()) {
      std::stringstream namestream;
      namestream << "dyninst_var_" << std::hex << at_addr;
      var_name = namestream.str();
   }

   return BPatch_variableExpr::makeVariableExpr(this, ll_addressSpace, var_name,
                                                (void *) at_addr, type);
}

Dyninst::PatchAPI::PatchMgrPtr Dyninst::PatchAPI::convert(const BPatch_addressSpace *a) {
   const BPatch_binaryEdit *edit = dynamic_cast<const BPatch_binaryEdit *>(a);
   if (edit) {
      return edit->lowlevel_edit()->mgr();
   }
   else {
      const BPatch_process *proc = dynamic_cast<const BPatch_process *>(a);
      return proc->lowlevel_process()->mgr();
   }
}

