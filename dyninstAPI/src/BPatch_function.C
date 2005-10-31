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

// $Id: BPatch_function.C,v 1.65 2005/10/31 22:42:53 rutar Exp $

#define BPATCH_FILE

#include <string.h>
#include "symtab.h"
#include "process.h"
#include "instPoint.h"

#include "BPatch.h"
#include "BPatch_function.h"
#include "BPatch_type.h"
#include "BPatch_collections.h"
#include "BPatch_Vector.h"
#include "BPatch_flowGraph.h"
#include "BPatch_libInfo.h"
#include "BPatch_memoryAccess_NP.h"
#include "BPatch_basicBlock.h"

#include "LineInformation.h"
#include "common/h/Types.h"
#include "InstrucIter.h"


/**************************************************************************
 * BPatch_function
 *************************************************************************/
/*
 * BPatch_function::BPatch_function
 *
 * Constructor that creates a BPatch_function.
 *
 */
BPatch_function::BPatch_function(BPatch_process *_proc, int_function *_func,
	BPatch_module *_mod) :
	proc(_proc), mod(_mod), cfg(NULL), cfgCreated(false), func(_func)
{
  // there should be at most one BPatch_func for each int_function per process
  assert( proc && !proc->func_map->defines(func) );
  
  _srcType = BPatch_sourceFunction;

  localVariables = new BPatch_localVarCollection;
  funcParameters = new BPatch_localVarCollection;
  retType = NULL;

  proc->func_map->add(_func, this);
  if (mod) {
      // Track for deletion
      mod->all_funcs.push_back(this);
  }
};

/*
 * BPatch_function::BPatch_function
 *
 * Constructor that creates the BPatch_function with return type.
 *
 */
BPatch_function::BPatch_function(BPatch_process *_proc, int_function *_func,
				 BPatch_type * _retType, BPatch_module *_mod) :
	proc(_proc), mod(_mod), cfg(NULL), cfgCreated(false), func(_func)
{
  assert(proc && !proc->func_map->defines(_func));

  _srcType = BPatch_sourceFunction;

  localVariables = new BPatch_localVarCollection;
  funcParameters = new BPatch_localVarCollection;
  retType = _retType;

  proc->func_map->add(_func, this);
  if (mod) {
      // Track for deletion
      mod->all_funcs.push_back(this);
  }
};


BPatch_function::~BPatch_function()
{
    // if (ast != NULL)
        // removeAst(ast);
#if 0
    // TODO!!!
    // There appears to be some illegal sharing going on here that needs to
    // be tracked down. Instead of leaking the entire function, we'll only
    // leak the variable collections...
    if (localVariables) delete localVariables;
    if (funcParameters) delete funcParameters;
#endif
    if (cfg) delete cfg;

    // Remove us from the proc map...
    if (proc && proc->func_map)
        proc->func_map->undefine(lowlevel_func());
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
char *BPatch_function::getNameBuffer(char *s, int len)
{
    assert(func);
    pdstring name = func->prettyName();
    strncpy(s, name.c_str(), len);

    return s;
}

#ifdef IBM_BPATCH_COMPAT
const char *BPatch_function::getNameDPCL()
{
  char n[1024];
  getName(n, 1023)[1023]='\0';
  return n;
}
#endif
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
char *BPatch_function::getMangledNameInt(char *s, int len)
{
  assert(func);
  pdstring mangledname = func->symTabName();
  strncpy(s, mangledname.c_str(), len);
  return s;
}

/*
 * BPatch_function::getBaseAddr
 *
 * Returns the starting address of the function.
 */
void *BPatch_function::getBaseAddrInt()
{
  return (void *)func->getAddress();
}

/*
 * BPatch_function::getSize
 *
 * Returns the size of the function in bytes.
 */
unsigned int BPatch_function::getSizeInt() 
{
    return func->getSize_NP();
}

/*
 * BPatch_function::getReturnType
 *
 * Returns the return type of the function.
 */
BPatch_type *BPatch_function::getReturnTypeInt()
{
    mod->parseTypesIfNecessary();
    return retType;
}

/*
 * BPatch_function::getModule
 *
 * Returns the BPatch_module to which this function belongs.
 */
BPatch_module *BPatch_function::getModuleInt()
{
  return mod;
}

//  BPatch_function::getParams
//  Returns a vector of BPatch_localVar, representing this function's parameters

BPatch_Vector<BPatch_localVar *> * BPatch_function::getParamsInt()
{
    mod->parseTypesIfNecessary();
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
BPatch_Vector<BPatch_point*> *BPatch_function::findPointInt(
        const BPatch_procedureLocation loc)
{
    // function does not exist!
    if (func == NULL) return NULL;

    // if the function is not instrumentable, we won't find the point
    if (!isInstrumentable())
       return NULL;

    // function is generally uninstrumentable (with current technology)
    if (func->funcEntries().size() == 0) return NULL;

    BPatch_Vector<BPatch_point*> *result = new BPatch_Vector<BPatch_point *>;

    if (loc == BPatch_entry || loc == BPatch_allLocations) {
        const pdvector<instPoint *> entries = func->funcEntries();
        for (unsigned foo = 0; foo < entries.size(); foo++)
            result->push_back(proc->findOrCreateBPPoint(this, 
                                                        entries[foo], 
                                                        BPatch_entry));
    }
    switch (loc) {
      case BPatch_entry: // already done
          break;
      case BPatch_allLocations:
        {
          const pdvector<instPoint *> &Rpoints = func->funcExits();
          const pdvector<instPoint *> &Cpoints = func->funcCalls();
          unsigned int c=0, r=0;
          Address cAddr, rAddr;
          while (c < Cpoints.size() || r < Rpoints.size()) {
              if (c < Cpoints.size()) cAddr = Cpoints[c]->addr();
              else                    cAddr = (Address)(-1);
              if (r < Rpoints.size()) rAddr = Rpoints[r]->addr();
              else                    rAddr = (Address)(-1);
              if (cAddr <= rAddr) {
                  result->push_back(proc->findOrCreateBPPoint(
                                                              this, Cpoints[c], BPatch_subroutine));
	      c++;
	    } else {
	      result->push_back(proc->findOrCreateBPPoint(
							  this, Rpoints[r], BPatch_exit));
	      r++;
	    }
          }
          break;
        }
      case BPatch_exit:
        {
          const pdvector<instPoint *> &points = func->funcExits();
          for (unsigned i = 0; i < points.size(); i++) {
             result->push_back(proc->findOrCreateBPPoint(
                                             this, points[i], BPatch_exit));
          }
          break;
        }
      case BPatch_subroutine:
        {
          const pdvector<instPoint *> &points = func->funcCalls();
          for (unsigned i = 0; i < points.size(); i++) {
             result->push_back(proc->findOrCreateBPPoint(
                                          this, points[i], BPatch_subroutine));
          }
          break;
        }
      case BPatch_longJump:
        /* XXX Not yet implemented */
      default:
        assert( 0 );
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
BPatch_Vector<BPatch_point*> *BPatch_function::findPointByOp(
        const BPatch_Set<BPatch_opCode>& ops)
{
  // function does not exist!
  if (func == NULL) return NULL;

  // function is generally uninstrumentable (with current technology)
  if (func->funcEntries().size() == 0) return NULL;
  
  // Use an instruction iterator
  InstrucIter ii(func);
    
  return BPatch_point::getPoints(ops, ii, this);
}

/*
 * BPatch_function::addParam()
 *
 * This function adds a function parameter to the BPatch_function parameter
 * vector.
 */
void BPatch_function::addParam(const char * _name, BPatch_type *_type,
			       int _linenum, long _frameOffset, int _reg,
			       BPatch_storageClass _sc)
{
  BPatch_localVar * param = new BPatch_localVar(_name, _type, _linenum,
						_frameOffset, _reg, _sc);

  // Add parameter to list of parameters
  params.push_back(param);
}

/*
 * BPatch_function::findLocalVarInt()
 *
 * This function searchs for a local variable in the BPatch_function's
 * local variable collection.
 */
BPatch_localVar * BPatch_function::findLocalVarInt(const char * name)
{
    mod->parseTypesIfNecessary();
    BPatch_localVar * var = localVariables->findLocalVar(name);
    return (var);
}

/*
 * BPatch_function::findLocalParam()
 *
 * This function searchs for a function parameter in the BPatch_function's
 * parameter collection.
 */
BPatch_localVar * BPatch_function::findLocalParamInt(const char * name)
{
    mod->parseTypesIfNecessary();
    BPatch_localVar * var = funcParameters->findLocalVar(name);
    return (var);
}

BPatch_flowGraph* BPatch_function::getCFGInt()
{
    if (cfg)
        return cfg;
    bool valid = false;
    cfg = new BPatch_flowGraph(this, valid);
    if (!valid) {
        delete cfg;
        cfg = NULL;
        fprintf(stderr, "CFG is NULL in %s\n", lowlevel_func()->symTabName().c_str());
        return NULL;
    }
    return cfg;
}


BPatch_Vector<BPatch_localVar *> *BPatch_function::getVarsInt() 
{
    mod->parseTypesIfNecessary();
    return localVariables->getAllVars(); 
}

BPatch_Vector<BPatch_variableExpr *> *BPatch_function::findVariableInt(
        const char *name)
{
   getModule()->parseTypesIfNecessary();
   BPatch_Vector<BPatch_variableExpr *> *ret;
   BPatch_localVar *lv = findLocalVar(name);
   if (!lv) {
      // look for it in the parameter scope now
      lv = findLocalParam(name);
   }
   if (lv) {
      // create a local expr with the correct frame offset or absolute
      //   address if that is what is needed
      ret = new BPatch_Vector<BPatch_variableExpr *>;
      BPatch_Vector<BPatch_point*> *points = findPoint(BPatch_entry);
      assert(points->size() == 1);
      BPatch_image *imgPtr = (BPatch_image *) mod->getObjParent();
      ret->push_back(new BPatch_variableExpr(imgPtr->getProcess(), 
                                             (void *) lv->getFrameOffset(), 
                                             lv->getRegister(), lv->getType(), 
                                             lv->getStorageClass(), 
                                             (*points)[0]));
      return ret;
   } else {
      // finally check the global scope.
      BPatch_image *imgPtr = (BPatch_image *) mod->getObjParent();
      
      if (!imgPtr) return NULL;
      
      BPatch_variableExpr *vars = imgPtr->findVariable(name);
      if (!vars) return NULL;
      
      ret = new BPatch_Vector<BPatch_variableExpr *>;
      ret->push_back(vars);
      return ret;
   }
}

bool BPatch_function::getVariablesInt(BPatch_Vector<BPatch_variableExpr *> &/*vect*/)
{
    	return false;
}

char *BPatch_function::getModuleNameInt(char *name, int maxLen) {
    return getModule()->getName(name, maxLen);
}

#ifdef IBM_BPATCH_COMPAT

bool BPatch_function::getLineNumbersInt(unsigned int &start, unsigned int &end) {
  char name[256];
  unsigned int length = 255;
  return getLineAndFile(start, end, name, length);
}

void *BPatch_function::getAddressInt() { return getBaseAddr(); }
    
bool BPatch_function::getAddressRangeInt(void * &start, void * &end) {
	start = getBaseAddr();
	unsigned long temp = (unsigned long) start;
	end = (void *) (temp + getSize());

	return true;
}

//BPatch_type *BPatch_function::returnType() { return retType; }
void BPatch_function::getIncPointsInt(BPatch_Vector<BPatch_point *> &vect) 
{
    BPatch_Vector<BPatch_point *> *v1 = findPoint(BPatch_allLocations);
    if (v1) {
	for (unsigned int i=0; i < v1->size(); i++) {
	    vect.push_back((*v1)[i]);
	}
    }
}

int	BPatch_function::getMangledNameLenInt() { return 1024; }

void BPatch_function::getExcPointsInt(BPatch_Vector<BPatch_point*> &points) {
  points.clear();
  abort();
  return;
};

BPatch_function::voidVoidFunctionPointer BPatch_function::getFunctionRefInt() {
#if defined( arch_ia64 )
	/* IA-64 function pointers actually point to structures.  We insert such
	   a structure in the mutatee so that instrumentation can use it. */
	Address entryPoint = (Address)getBaseAddr();
	Address gp = proc->getTOCoffsetInfo( entryPoint );

	Address remoteAddress = proc->inferiorMalloc( sizeof( Address ) * 2 );
	assert( remoteAddress != (Address)NULL );

	proc->writeDataSpace( (void *)remoteAddress, sizeof( Address ), & entryPoint );
	proc->writeDataSpace( (void *)(remoteAddress + sizeof( Address )), sizeof( Address ), & gp );

	return (BPatch_function::voidVoidFunctionPointer)remoteAddress;
#else
	/* This will probably work on all other platforms. */
	return (BPatch_function::voidVoidFunctionPointer) getBaseAddr();
#endif
	} /* end getFunctionRef() */

#endif

/*
 * BPatch_function::isInstrumentable
 *
 * Returns true if the function is instrumentable, false otherwise.
 */
bool BPatch_function::isInstrumentableInt()
{
     return ((int_function *)func)->isInstrumentable();
}

// Return TRUE if the function resides in a shared lib, FALSE otherwise

bool BPatch_function::isSharedLibInt(){
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
   }
}

void BPatch_function::calc_liveness(BPatch_point *point) {
#if defined(os_aix) || defined(arch_x86_64)
    assert(point);
    instPoint *iP = point->getPoint();
    assert(iP);
    Address pA = iP->addr();
    int *liveRegisters = iP->liveRegisters;
    int *liveFPRegisters = iP->liveFPRegisters;
    int *liveSPRegisters = iP->liveSPRegisters;
    
    // BEGIN LIVENESS ANALYSIS STUFF
    // Need to narrow it down to specific basic block at this point so we can 
    // recover liveness information
    
    // Need the CFG to do liveness analysis 
    BPatch_flowGraph * cfg = getCFG();
    // No CFG, no liveness.
    if (!cfg) return;
    BPatch_Set<BPatch_basicBlock*> allBlocks;
    cfg->getAllBasicBlocks(allBlocks);
    BPatch_basicBlock** elements = new BPatch_basicBlock*[allBlocks.size()];
    allBlocks.elements(elements);
    
    for (int i = 0; i < allBlocks.size(); i++) {
        
      BPatch_basicBlock *bb = elements[i];
      int_basicBlock *ibb = bb->lowlevel_block();

      if (iP->block() == ibb) {
	/* When we have the actual basic block belonging to the 
	   inst address, we put the live Registers in for that inst point*/
            
	ibb->liveRegistersIntoSet(iP->liveRegisters, iP->liveFPRegisters, pA );
	
	/* Function for handling special purpose registers on platforms,
	   for Power it figures out MX register usage (big performance hit) ... 
	   may be extended later for other special purpose registers */
	ibb->liveSPRegistersIntoSet(iP->liveSPRegisters, pA);
	  
	  //bb->printAll();
      }
    }
    delete [] elements;
    // END LIVENESS ANALYSIS STUFF
#endif
}
