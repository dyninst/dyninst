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

// $Id: BPatch_function.C,v 1.109 2008/11/03 15:19:24 jaw Exp $

#define BPATCH_FILE

#include <string.h>
#include <string>
#include "function.h"
#include "instPoint.h"
#include "ast.h"

#include "BPatch.h"
#include "BPatch_function.h"
#include "BPatch_point.h"
#include "BPatch_type.h"
#include "BPatch_collections.h"
#include "BPatch_Vector.h"
#include "BPatch_flowGraph.h"
#include "BPatch_libInfo.h"
#include "BPatch_memoryAccess_NP.h"
#include "BPatch_basicBlock.h"
#include "BPatch_statement.h"
#include "mapped_module.h"
#include "mapped_object.h"
#include "debug.h"
#include "hybridAnalysis.h"
#include "addressSpace.h"

#if defined(cap_stack_mods)
#include "StackMod/StackModChecker.h"
#endif


#include "Point.h"
#include "PatchMgr.h"

using Dyninst::PatchAPI::PatchMgrPtr;
using Dyninst::PatchAPI::Point;



/**************************************************************************
 * BPatch_function
 *************************************************************************/
/*
 * BPatch_function::BPatch_function
 *
 * Constructor that creates a BPatch_function.
 *
 */

int bpatch_function_count = 0;

BPatch_function::BPatch_function(BPatch_addressSpace *_addSpace, 
                                 func_instance *_func,
                                 BPatch_module *_mod) :
	addSpace(_addSpace), 
   lladdSpace(_func->proc()),
   mod(_mod),
   cfg(NULL), 
   cfgCreated(false), 
   liveInit(false),
   func(_func), 
	varsAndParamsValid(false)
{
   _srcType = BPatch_sourceFunction;

   localVariables = new BPatch_localVarCollection;
   funcParameters = new BPatch_localVarCollection;
   retType = NULL;
   
   assert(mod && !mod->func_map.count(func));
   mod->func_map[func] = this;
}

/*
 * BPatch_function::BPatch_function
 *
 * Constructor that creates the BPatch_function with return type.
 *
 */
BPatch_function::BPatch_function(BPatch_addressSpace *_addSpace, func_instance *_func,
				 BPatch_type * _retType, BPatch_module *_mod) :
	addSpace(_addSpace),
   lladdSpace(_func->proc()),
   mod(_mod),
   cfg(NULL),
   cfgCreated(false),
   liveInit(false), 
   func(_func),
	varsAndParamsValid(false)
{
  _srcType = BPatch_sourceFunction;

  localVariables = new BPatch_localVarCollection;
  funcParameters = new BPatch_localVarCollection;
  retType = _retType;

  assert(mod);
  mod->func_map[func] = this;
}


BPatch_function::~BPatch_function()
{
   if (localVariables) delete localVariables;
   if (funcParameters) delete funcParameters;
   
   if (cfg) delete cfg;
   
   // Remove us from the proc map...
   int num_erased = mod->func_map.erase(lowlevel_func());
   assert(num_erased == 1);
}


//dynC internal:

bool BPatch_function::hasParamDebugInfo(){
   std::vector<Dyninst::SymtabAPI::localVar *>SymTabParams;
   return lowlevel_func()->ifunc()->getSymtabFunction()->getParams(SymTabParams);
}

/* 
 * BPatch_function::getSourceObj()
 *
 * Return the contained source objects (e.g. statements).
 *    This is not currently supported.
 *
 */
bool BPatch_function::getSourceObj(BPatch_Vector<BPatch_sourceObj *> &children)
{
    // init and empty vector
    BPatch_Vector<BPatch_sourceObj *> dummy;

    children = dummy;
    return false;
}


BPatch_process * BPatch_function::getProc() const
{
  assert(addSpace->getType() == TRADITIONAL_PROCESS); 
  return dynamic_cast<BPatch_process *>(addSpace); 
}


/*
 * BPatch_function::getObjParent()
 *
 * Return the parent of the function (i.e. the module)
 *
 */
BPatch_sourceObj *BPatch_function::getObjParent()
{
    return (BPatch_sourceObj *) mod;
}

std::string BPatch_function::getName() {
   return getDemangledName();
}

std::string BPatch_function::getMangledName() {
   return func->symTabName();
}

std::string BPatch_function::getDemangledName() {
   return func->prettyName();
}

std::string BPatch_function::getTypedName() {
   return func->typedName();
}

bool BPatch_function::getNames(std::vector<std::string> &names) {
	return getDemangledNames(names);
}

bool BPatch_function::getDemangledNames(std::vector<std::string> &names) {
  std::copy(func->pretty_names_begin(),
	    func->pretty_names_end(),
	    std::back_inserter(names));
  return func->pretty_names_begin() != func->pretty_names_end();
}
bool BPatch_function::getMangledNames(std::vector<std::string> &names) {
  std::copy(func->symtab_names_begin(),
	    func->symtab_names_end(),
	    std::back_inserter(names));
  return func->symtab_names_begin() != func->symtab_names_end();
}
bool BPatch_function::getTypedNames(std::vector<std::string> &names) {
  std::copy(func->typed_names_begin(),
	    func->typed_names_end(),
	    std::back_inserter(names));
  return func->typed_names_begin() != func->typed_names_end();
}


/*
 * BPatch_function::getName
 *
 * Copies the name of the function into a buffer, up to a given maximum
 * length.  Returns a pointer to the beginning of the buffer that was
 * passed in.
 *
 * s            The buffer into which the name will be copied.
 * len          The size of the buffer.
 */
char *BPatch_function::getName(char *s, int len)
{
    assert(func);
    string name = func->prettyName();
    strncpy(s, name.c_str(), len);
    return s;
}

/*
 * BPatch_function::getMangledName
 *
 * Copies the mangled name of the function into a buffer, up to a given maximum
 * length.  Returns a pointer to the beginning of the buffer that was
 * passed in.
 *
 * s            The buffer into which the name will be copied.
 * len          The size of the buffer.
 */
char *BPatch_function::getMangledName(char *s, int len)
{
  assert(func);
  string mangledname = func->symTabName();
  strncpy(s, mangledname.c_str(), len);
  return s;
}

/*
 * BPatch_function::getTypedName
 *
 * Copies the mangled name of the function into a buffer, up to a given maximum
 * length.  Returns a pointer to the beginning of the buffer that was
 * passed in.
 *
 * s            The buffer into which the name will be copied.
 * len          The size of the buffer.
 */
char *BPatch_function::getTypedName(char *s, int len)
{
  assert(func);
  string typedname = func->typedName();
  strncpy(s, typedname.c_str(), len);
  return s;
}


/*
 * BPatch_function::getNames
 *
 * Copies all names of the function into the provided BPatch_Vector.
 * Names are represented as const char *s and are
 * allocated/deallocated by Dyninst.
 *
 * names           BPatch_Vector reference
 */

bool BPatch_function::getNames(BPatch_Vector<const char *> &names)
{
    assert(func);
    unsigned pre_size = names.size();
    for (auto i = func->pretty_names_begin(); i != func->pretty_names_end(); i++) {
        names.push_back(i->c_str());
    }

    return names.size() > pre_size;
}

/*
 * BPatch_function::getMangledNames
 *
 * Copies all mangled names of the function into the provided
 * BPatch_Vector.  Names are represented as const char *s and are
 * allocated/deallocated by Dyninst.
 *
 * names           BPatch_Vector reference
 */

bool BPatch_function::getMangledNames(BPatch_Vector<const char *> &names)
{
    assert(func);
    unsigned pre_size = names.size();

    for (auto i = func->symtab_names_begin(); i != func->symtab_names_end(); i++) {
        names.push_back(i->c_str());
    }

    return names.size() > pre_size;
}



/*
 * BPatch_function::getBaseAddr
 *
 * Returns the starting address of the function.
 */
void *BPatch_function::getBaseAddr()
{
  return (void *)func->addr();
}

/*
 * BPatch_function::getReturnType
 *
 * Returns the return type of the function.
 */
BPatch_type *BPatch_function::getReturnType()
{
    constructVarsAndParams();
    return retType;
}

/* Update code bytes if necessary (defensive mode), 
 * Parse new edge, 
 * Correct missing elements in BPatch-level datastructures
*/
bool BPatch_function::parseNewEdge(Dyninst::Address source, 
                                   Dyninst::Address target)
{
    // mark code bytes as needing an update
    if (BPatch_defensiveMode == func->obj()->hybridMode()) {
        func->obj()->setCodeBytesUpdated(false);
    }

    // set up arguments to lower level parseNewEdges and call it
    block_instance *sblock = func->obj()->findBlockByEntry(source);
    assert(sblock);
    vector<edgeStub> stubs;
    stubs.push_back(edgeStub(sblock, target, ParseAPI::NOEDGE, true));
    func->obj()->parseNewEdges(stubs);

    // Correct missing elements in BPatch-level datastructures
    // i.e., wipe out the BPatch_flowGraph CFG, we'll re-generate it
    if ( cfg ) {
        cfg->invalidate();
    }

    return true;
}

// Removes all instrumentation and relocation from the function and 
// restores the original version
// Also flushes the runtime library address cache, if present
bool BPatch_function::removeInstrumentation(bool useInsertionSet)
{
    bool removedAll = true;
    if (useInsertionSet) {
        addSpace->beginInsertionSet();
    }

    // remove instrumentation, point by point
    std::vector<BPatch_point*> points;
    getAllPoints(points);
    for (unsigned pidx=0; pidx < points.size(); pidx++) {
        vector<BPatchSnippetHandle*> allSnippets = 
            points[pidx]->getCurrentSnippets();
        for (unsigned all = 0; all < allSnippets.size(); all++) 
        {
            if (dynamic_cast<BPatch_process*>(addSpace)->getHybridAnalysis()->hybridOW()->
                hasLoopInstrumentation(true,*this))
            {
                mal_printf("ERROR: Trying to remove active looop instrumentation\n");
                removedAll = false;
                assert(0);
            }
            else if ( ! addSpace->deleteSnippet(allSnippets[all]) ) {
                removedAll = false;
            }
        } 
    }

    if (useInsertionSet) {
        bool dontcare=false;
        if (!addSpace->finalizeInsertionSet(false,&dontcare)) {
            removedAll = false;
        }
    }
    mal_printf("removed instrumentation from func %p\n", getBaseAddr());
    return removedAll;
}

/* Gets unresolved instPoints from func_instance and converts them to
   BPatch_points, puts them in unresolvedCF */
void BPatch_function::getUnresolvedControlTransfers
(BPatch_Vector<BPatch_point *> &unresolvedCF/*output*/)
{
   const func_instance::BlockSet &blocks = func->unresolvedCF();
   for (func_instance::BlockSet::const_iterator iter = blocks.begin(); iter != blocks.end(); ++iter) {
      // We want to instrument before the last instruction, since we can pull out
      // the target at that point. Technically, we want to instrument the sink edge;
      // but we can't do that yet. 
      instPoint *point = NULL;
      if ((*iter)->containsCall()) {
          point = instPoint::preCall(func, *iter);
      } else {
          point = instPoint::preInsn(func, *iter, (*iter)->last());
      }
      BPatch_procedureLocation ptType = 
         BPatch_point::convertInstPointType_t(point->type());
      if (ptType == BPatch_locInstruction) {
         // since this must be a control transfer, it's either an indirect
         // jump or a direct jump
         mal_printf("WARNING: ambiguous point type translation for "
                    "insn at %lx, setting to locLongJump %s[%d]\n",
                    point->block()->start(), FILE__,__LINE__);
         ptType = BPatch_locLongJump;
      }
      BPatch_point *curPoint = addSpace->findOrCreateBPPoint
         (this, point, ptType);
      unresolvedCF.push_back(curPoint);
   }
}

/* Gets abrupt end instPoints from func_instance and converts them to
   BPatch_points, puts them in abruptEnds */
void BPatch_function::getAbruptEndPoints
(BPatch_Vector<BPatch_point *> &abruptEnds/*output*/)
{
   const func_instance::BlockSet &blocks = func->abruptEnds();
   for (func_instance::BlockSet::const_iterator iter = blocks.begin(); iter != blocks.end(); ++iter) {
      // We just want to know if this code is executed, so use a "start of block" point.
      instPoint *point = instPoint::blockEntry(func, *iter);
      BPatch_point *curPoint = addSpace->findOrCreateBPPoint
         (this, point, BPatch_locInstruction);
      abruptEnds.push_back(curPoint);
   }
}

// This one is interesting - get call points for everywhere that _calls_ us. 
// That we know about. 
void BPatch_function::getCallerPoints(std::vector<BPatch_point*>& callerPoints)
{
   std::vector<block_instance *> callerBlocks;
   func->getCallerBlocks(std::back_inserter(callerBlocks));
   for (std::vector<block_instance *>::iterator iter = callerBlocks.begin(); 
        iter != callerBlocks.end(); ++iter) 
   {
      vector<func_instance*> callerFuncs;
      (*iter)->getFuncs(std::back_inserter(callerFuncs));
      for (vector<func_instance*>::iterator fit = callerFuncs.begin();
           fit != callerFuncs.end();
           fit++)
      {
          instPoint *point = instPoint::preCall(*fit, *iter);
          BPatch_function *callerFunc = addSpace->findOrCreateBPFunc
             (*fit, NULL);
          BPatch_point *curPoint = addSpace->findOrCreateBPPoint
             (callerFunc, point, BPatch_locSubroutine);
          callerPoints.push_back(curPoint);
      }
   }
}

void BPatch_function::getCallPoints(BPatch_Vector<BPatch_point *> &callPoints) {
   const PatchFunction::Blockset &blocks = func->callBlocks();
  for (PatchFunction::Blockset::const_iterator iter = blocks.begin();
       iter != blocks.end(); ++iter) {
    block_instance* iblk = SCAST_BI(*iter);
    instPoint *point = instPoint::preCall(func, iblk);
    BPatch_point *curPoint = addSpace->findOrCreateBPPoint(this, point,
                                                           BPatch_locSubroutine);
    callPoints.push_back(curPoint);
  }
}

void BPatch_function::getEntryPoints(BPatch_Vector<BPatch_point *> &entryPoints/*output*/) {
   BPatch_point *curPoint = addSpace->findOrCreateBPPoint(this,
                                                          instPoint::funcEntry(func),
                                                          BPatch_locEntry);
   entryPoints.push_back(curPoint);
}

void BPatch_function::getExitPoints(BPatch_Vector<BPatch_point *> &exitPoints) {
  func_instance::Points pts;
  func->funcExitPoints(&pts);
  for (func_instance::Points::iterator i = pts.begin(); i != pts.end(); i++) {
    instPoint *point = *i;
    BPatch_point *curPoint = addSpace->findOrCreateBPPoint(this, point, BPatch_locExit);
    exitPoints.push_back(curPoint);
  }
}


void BPatch_function::getAllPoints(std::vector<BPatch_point*>& bpPoints)
{
   getEntryPoints(bpPoints);
   getExitPoints(bpPoints);
   getCallPoints(bpPoints);
   getUnresolvedControlTransfers(bpPoints);
   getAbruptEndPoints(bpPoints);
   // Not running with arbitrary for now...
}

// Sets the address in the structure at which the fault instruction's
// address is stored if "set" is true.  Accesses the fault address and 
// translates it back to an original address if it corresponds to 
// relocated code in the Dyninst heap 
bool BPatch_function::setHandlerFaultAddrAddr
        (Dyninst::Address addr, bool set)
{
    if (lowlevel_func()->getHandlerFaultAddr()) {
        lowlevel_func()->setHandlerFaultAddrAddr(addr,set);
        return true;
    }
    return false;
}

/*
 * BPatch_function::getModule
 *
 * Returns the BPatch_module to which this function belongs.
 */
BPatch_module *BPatch_function::getModule()
{
  return mod;
}

//  BPatch_function::getParams
//  Returns a vector of BPatch_localVar, representing this function's parameters

BPatch_Vector<BPatch_localVar *> * BPatch_function::getParams()
{
    if (!mod->isValid()) return NULL;
    constructVarsAndParams();
    // Do *not* hand back the alphabetical locals collection; hand back the params in the order they're
    // presented
    return &params;
}

/*
 * BPatch_function::findPoint
 *
 * Returns a vector of the instrumentation points from a procedure that is
 * identified by the parameters, or returns NULL upon failure.
 * (Points are sorted by address in the vector returned.)
 *
 * loc          The points within the procedure to return.  The following
 *              values are valid for this parameter:
 *                BPatch_entry         The function's entry point.
 *                BPatch_exit          The function's exit point(s).
 *                BPatch_subroutine    The points at which the procedure calls
 *                                     other procedures.
 *                BPatch_longJump      The points at which the procedure make
 *                                     long jump calls.
 *                BPatch_allLocations  All of the points described above.
 */
BPatch_Vector<BPatch_point*> *BPatch_function::findPoint(
        const BPatch_procedureLocation loc)
{
    // function does not exist!
    if (func == NULL) return NULL;

    if (!mod->isValid()) return NULL;

    BPatch_Vector<BPatch_point*> *result = new BPatch_Vector<BPatch_point *>;
    
    switch(loc) {
       case BPatch_entry:
          getEntryPoints(*result);
          break;
       case BPatch_exit:
          getExitPoints(*result);
          break;
       case BPatch_subroutine:
          getCallPoints(*result);
          break;
       case BPatch_allLocations:
          // Seems to be "entry, exit, call" rather than a true "all"
          getEntryPoints(*result);
          getExitPoints(*result);
          getCallPoints(*result);
          break;
       default:
          // Not implemented
          delete result;
          return NULL;
    }

    return result;
}

/*
 * BPatch_function::findPoint (VG 09/05/01)
 *
 * Returns a vector of the instrumentation points from a procedure that is
 * identified by the parameters, or returns NULL upon failure.
 * (Points are sorted by address in the vector returned.)
 *
 * ops          The points within the procedure to return. A set of op codes
 *              defined in BPatch_opCode (BPatch_point.h)
 */

struct compareByEntryAddr
{
    bool operator()(const BPatch_basicBlock* lhs, const BPatch_basicBlock* rhs) const
    {
        return const_cast<BPatch_basicBlock*>(lhs)->getStartAddress() < 
			const_cast<BPatch_basicBlock*>(rhs)->getStartAddress();
    }
};
 
BPatch_Vector<BPatch_point*> *BPatch_function::findPoint(const std::set<BPatch_opCode>& ops)
{
   // function does not exist!
   if (func == NULL) return NULL;
   
   if (!mod->isValid()) return NULL;
   // function is generally uninstrumentable (with current technology)
   if (!isInstrumentable()) return NULL;
   
   std::set<BPatch_basicBlock*, compareByEntryAddr> blocks;
   std::set<BPatch_basicBlock*> unsorted_blocks;
   getCFG()->getAllBasicBlocks(unsorted_blocks);
   std::copy(unsorted_blocks.begin(), unsorted_blocks.end(), std::inserter(blocks, blocks.begin()));
   BPatch_Vector<BPatch_point*>* ret = new BPatch_Vector<BPatch_point*>;
   for(std::set<BPatch_basicBlock*, compareByEntryAddr>::iterator curBlk = blocks.begin();
       curBlk != blocks.end();
       curBlk++)
   {
      BPatch_Vector<BPatch_point*>* tmp = (*curBlk)->findPoint(ops);
      for (unsigned int i = 0; i < tmp->size(); i++)
      {
         ret->push_back((*tmp)[i]);
      } 
      
   }
   return ret;
}

BPatch_Vector<BPatch_point*> *BPatch_function::findPoint(const BPatch_Set<BPatch_opCode>& ops) {
   std::set<BPatch_opCode> tmp;
   std::copy(ops.int_set.begin(), ops.int_set.end(), std::inserter(tmp, tmp.end()));
   return findPoint(tmp);
}

/*
 * BPatch_function::findPoint
 *
 * Create a BPatch_point corresponding with the provided address.
 */

BPatch_point *BPatch_function::findPoint(Dyninst::Address addr) {
   // Find the matching block and feed this into

   block_instance *llb = lowlevel_func()->getBlock(addr);
   if (!llb) return NULL;
   
   instPoint *p = instPoint::preInsn(lowlevel_func(), llb, addr);
   if (!p) return NULL;
   return getAddSpace()->findOrCreateBPPoint(this,
                                             p,
                                             BPatch_locInstruction);
}

/*
 * BPatch_function::addParam()
 *
 * This function adds a function parameter to the BPatch_function parameter
 * vector.
 */
void BPatch_function::addParam(Dyninst::SymtabAPI::localVar *lvar)
{
  BPatch_localVar * param = new BPatch_localVar(lvar);
  
  // Add parameter to list of parameters
  params.push_back(param);
}

#if 0
void BPatch_function::addParam(const char * _name, BPatch_type *_type,
			       int _linenum, long _frameOffset, int _reg,
			       BPatch_storageClass _sc)
{
  BPatch_localVar * param = new BPatch_localVar(_name, _type, _linenum,
						_frameOffset, _reg, _sc);

  // Add parameter to list of parameters
  params.push_back(param);
}
#endif

/*
 * BPatch_function::findLocalVar()
 *
 * This function searchs for a local variable in the BPatch_function's
 * local variable collection.
 */
BPatch_localVar * BPatch_function::findLocalVar(const char * name)
{
    if (!mod->isValid()) 
		return NULL;
    constructVarsAndParams();
    BPatch_localVar * var = localVariables->findLocalVar(name);
    return (var);
}

/*
 * BPatch_function::findLocalParam()
 *
 * This function searchs for a function parameter in the BPatch_function's
 * parameter collection.
 */
BPatch_localVar * BPatch_function::findLocalParam(const char * name)
{
    if (!mod->isValid()) return NULL;
    constructVarsAndParams();
    BPatch_localVar * var = funcParameters->findLocalVar(name);
    return (var);
}

BPatch_flowGraph* BPatch_function::getCFG()
{
    assert(mod);
    if (!mod->isValid()) return NULL;
    if (cfg && cfg->isValid() && 
        ( ! mod->isExploratoryModeOn() || 
          ! lowlevel_func()->obj()->parse_img()->hasNewBlocks()))
        return cfg;
    bool valid = false;
    cfg = new BPatch_flowGraph(this, valid);
    if (!valid) {
        delete cfg;
        cfg = NULL;
        fprintf(stderr, "CFG is NULL for func %s at %lx %s[%d]\n", 
                lowlevel_func()->symTabName().c_str(),
                func->addr(),FILE__,__LINE__);
        return NULL;
    }
    return cfg;
}

void BPatch_function::constructVarsAndParams()
{
    if (varsAndParamsValid)
       return;

    if (mod) {
        mod->parseTypesIfNecessary();
    }

    //Check flag to see if vars & params are already constructed
    std::vector<SymtabAPI::localVar *>vars;

    if (lowlevel_func()->ifunc()->getSymtabFunction()->getLocalVariables(vars)) 
	{
        for (unsigned i = 0; i< vars.size(); i++) 
	    {
            if (mod) {
                vars[i]->fixupUnknown(mod->lowlevel_mod()->pmod()->mod());
            }

	        localVariables->addLocalVar(new BPatch_localVar(vars[i]));
	    }    
    }

    std::vector<SymtabAPI::localVar *>parameters;
    if (lowlevel_func()->ifunc()->getSymtabFunction()->getParams(parameters)) 
	{
        for (unsigned i = 0; i< parameters.size(); i++) 
		{
            if (mod) {
                parameters[i]->fixupUnknown(mod->lowlevel_mod()->pmod()->mod());
            }

	        BPatch_localVar *lparam = new BPatch_localVar(parameters[i]);
    	    funcParameters->addLocalVar(lparam);
	        params.push_back(lparam);
    	}    
    }

    if (!lowlevel_func()->ifunc()->getSymtabFunction()->getReturnType(Dyninst::SymtabAPI::Type::share)) 
	{
        varsAndParamsValid = true;
        return;
    }
        
	auto ret_type = lowlevel_func()->ifunc()->getSymtabFunction()->getReturnType(Dyninst::SymtabAPI::Type::share);
	assert(ret_type);

	extern AnnotationClass<BPatch_type> TypeUpPtrAnno;

	if (!ret_type->getAnnotation(retType, TypeUpPtrAnno))
	{
		//  BPatch_type ctor adds the annotation to the lowlevel symtab type, so 
		//  no need to do it here.
		retType = new BPatch_type(ret_type);
	}
	else
		assert(retType);

    varsAndParamsValid = true;
}

BPatch_Vector<BPatch_localVar *> *BPatch_function::getVars() 
{
    if (!mod->isValid()) return NULL;
    constructVarsAndParams();
    return localVariables->getAllVars(); 
}

bool BPatch_function::findVariable(const char *name, 
                                      BPatch_Vector<BPatch_variableExpr *> &vars) 
{
   if (!mod->isValid()) 
	   return false;

   constructVarsAndParams();

   BPatch_localVar *lv = findLocalVar(name);

   if (!lv) 
   {
      // look for it in the parameter scope now
      lv = findLocalParam(name);
   }

   if (lv) 
   {
      // create a local expr with the correct frame offset or absolute
      //   address if that is what is needed
      std::map<BPatch_localVar *, BPatch_variableExpr*>::iterator i;
      i = local_vars.find(lv);

      if (i != local_vars.end()) 
	  {
         vars.push_back((*i).second);
         return true;
      }

      BPatch_Vector<BPatch_point*> *points = findPoint(BPatch_entry);
      assert(points->size() == 1);
      BPatch_image *imgPtr = (BPatch_image *) mod->getObjParent();

      BPatch_variableExpr *nv = new BPatch_variableExpr(imgPtr->getAddressSpace(),
                                                        func->proc(),
                                                        lv, lv->getType(), (*points)[0]);
      vars.push_back(nv);
      return true;
   }

   // finally check the global scope.
   BPatch_image *imgPtr = (BPatch_image *) mod->getObjParent();
   
   if (!imgPtr) return false;
   
   BPatch_variableExpr *var = imgPtr->findVariable(name);
   if (!var) return false;
   
   vars.push_back(var);
   return true;   
}


BPatch_Vector<BPatch_variableExpr *> *BPatch_function::findVariable(const char *name)
{
   BPatch_Vector<BPatch_variableExpr *> *vars = new BPatch_Vector<BPatch_variableExpr*>;
   bool result = findVariable(name, *vars);
   if (!result) {
      delete vars;
      return NULL;
   }
   return vars;
}

bool BPatch_function::getVariables(BPatch_Vector<BPatch_variableExpr *> &/*vect*/)
{
    	return false;
}

char *BPatch_function::getModuleName(char *name, int maxLen) {
    return getModule()->getName(name, maxLen);
}

BPatch_variableExpr *BPatch_function::getFunctionRef() 
{
  Address remoteAddress = (Address)getBaseAddr();
  string fname = func->prettyName();

  //  Need to figure out the type for this effective function pointer,
  //  of the form <return type> (*)(<arg1 type>, ... , <argn type>)
  
  //  Note:  getParams allocates the vector
  string typestr;
  if(retType) {
      typestr += retType->getName();
  } else {
      typestr += "void";
  }
  typestr += " (*function)(";
  
  BPatch_Vector<BPatch_localVar *> *params_ = getParams();
  assert(params_);
  
  for (unsigned int i = 0; i < params_->size(); ++i) {
        typestr += (*params_)[i]->getName();
        //  no comma after last parameter
     if (i <= (params_->size() - 1)) {
        typestr +=  ",";
     }
  }
  if(params_->size()==0) {
      typestr += "void";
  }
  typestr += ")";
  
  BPatch_type *type = addSpace->image->findType(typestr.c_str());
  // Fallback to general pointer type.
  if (!type) {
    type = addSpace->image->findType("void *");
  }
  if (!type) {
     fprintf(stderr, "%s[%d]:  cannot find type '%s'\n", FILE__, __LINE__, typestr.c_str());
  }
  assert(type);
  
  //  In truth it would make more sense for this to be a BPatch_constExpr,
  //  But since we are adding this as part of the DPCL compatibility process
  //  we use the IBM API, to eliminate one API difference.
  
  AstNodePtr ast(AstNode::operandNode(AstNode::operandType::Constant, (void *) remoteAddress));
  
  // the variableExpr owns the ast now.
  return new BPatch_variableExpr(fname.c_str(), addSpace, lladdSpace, ast, 
                                 type, (void *) remoteAddress);
  

} /* end getFunctionRef() */

bool BPatch_function::getAddressRange(void * &start, void * &end) {
   Address s, e;
   bool ret = getAddressRange(s, e);
   start = (void *)s;
   end = (void *)e;
   return ret;
}

bool BPatch_function::getAddressRange(Dyninst::Address &start, Dyninst::Address &end) {
   start = func->addr();
   
   // end is a little tougher
   end = func->addr();
   for (PatchFunction::Blockset::const_iterator iter = func->blocks().begin();
        iter != func->blocks().end(); ++iter) {
      end = (end < (*iter)->end()) ? (*iter)->end() : end;
   }

   return true;
}

/*
 * BPatch_function::isInstrumentable
 *
 * Returns true if the function is instrumentable, false otherwise.
 */
bool BPatch_function::isInstrumentable()
{
     return ((func_instance *)func)->isInstrumentable();
}

// Return TRUE if the function resides in a shared lib, FALSE otherwise

bool BPatch_function::isSharedLib(){
  return mod->isSharedLib();
} 

void BPatch_function::fixupUnknown(BPatch_module *module) {
   if (retType != NULL && retType->getDataClass() == BPatch_dataUnknownType) 
      retType = module->getModuleTypes()->findType(retType->getID());

   for (unsigned int i = 0; i < params.size(); i++)
      params[i]->fixupUnknown(module);
   if (localVariables != NULL) {
      BPatch_Vector<BPatch_localVar *> *vars = localVariables->getAllVars();
      for (unsigned int i = 0; i < vars->size(); i++)
         (*vars)[i]->fixupUnknown(module);
      delete vars;
   }
}

bool BPatch_function::containsSharedBlocks() {
    return func->containsSharedBlocks();
}

// isPrimary: function will now use this name as a primary output name
// isMangled: this is the "mangled" name rather than demangled (pretty)
const char *BPatch_function::addName(const char *name,
                                        bool isPrimary, /* = true */
                                        bool isMangled) { /* = false */
    // Add to the internal function object
    //    Add to the container mapped_object name mappings
    //    Add to the proc-independent function object
    //       Add to the container image class

    if (isMangled) {
        func->addSymTabName(std::string(name),
                            isPrimary);
    }
    else {
        func->addPrettyName(std::string(name),
                              isPrimary);
    }
    return name;
}

bool BPatch_function::findOverlapping(BPatch_Vector<BPatch_function *> &funcs) {
    assert(func);
    assert(addSpace);

    std::set<func_instance *> overlappingIntFuncs;
    if (!func->getSharingFuncs(overlappingIntFuncs)) {
        // No overlapping functions
        return false;
    }

    // We now need to map from func_instances to BPatch_functions
    for (std::set<func_instance *>::iterator iter = overlappingIntFuncs.begin();
         iter != overlappingIntFuncs.end(); ++iter) {
       funcs.push_back(addSpace->findOrCreateBPFunc(*iter,
                                                     mod));
    }

    return true;
}

Dyninst::ParseAPI::Function *Dyninst::ParseAPI::convert(const BPatch_function *f) {
   return f->func->ifunc();
}

Dyninst::PatchAPI::PatchFunction *Dyninst::PatchAPI::convert(const BPatch_function *f) {
   return f->func;
}

void BPatch_function::relocateFunction()
{
     lowlevel_func()->proc()->addModifiedFunction(lowlevel_func());
     if (getAddSpace()->pendingInsertions == NULL) {
        // Trigger it now
        bool tmp;
        getAddSpace()->finalizeInsertionSet(false, &tmp);
     }
}

bool BPatch_function::getSharedFuncs(set<BPatch_function*> &sharedFuncs)
{
   std::set<func_instance *> ishared;
   if (!func->getSharingFuncs(ishared)) return false;
   for (std::set<func_instance *>::iterator iter = ishared.begin();
        iter != ishared.end(); ++iter) {
      sharedFuncs.insert(getAddSpace()->findOrCreateBPFunc(*iter,
                                                           getModule()));
   }
   return true;
}

unsigned int BPatch_function::getFootprint()
{
    return func->footprint();
}

bool BPatch_function::addMods(std::set<StackMod*> mods)
{
#if defined(cap_stack_mods)
    StackModChecker checker(this, func);
    return checker.addModsInternal(mods);
#else
    (void)mods;
    return false;
#endif
}

