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

// $Id: BPatch_function.C,v 1.104 2008/02/23 02:09:04 jaw Exp $

#define BPATCH_FILE

#include <string.h>
#include <string>
#include "symtab.h"
#include "process.h"
#include "instPoint.h"
#include "ast.h"

#include "BPatch.h"
#include "BPatch_function.h"
#include "BPatch_type.h"
#include "BPatch_collections.h"
#include "BPatch_Vector.h"
#include "BPatch_flowGraph.h"
#include "BPatch_libInfo.h"
#include "BPatch_memoryAccess_NP.h"
#include "BPatch_basicBlock.h"
#include "BPatch_statement.h"
#include "mapped_module.h"

#include "symtabAPI/h/LineInformation.h"
#include "common/h/Types.h"
#include "InstrucIter-Function.h"

#if defined(cap_slicing)
// #include "BPatch_dependenceGraphNode.h"
#include "BPatch_dependenceGraphEdge.h"
#include "dynutil/h/Annotatable.h"
#endif

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

BPatch_function::BPatch_function(BPatch_addressSpace *_addSpace, int_function *_func,
	BPatch_module *_mod) :
	addSpace(_addSpace), mod(_mod), cfg(NULL), cfgCreated(false), liveInit(false), func(_func), 
	varsAndParamsValid(false)
{
#if defined(ROUGH_MEMORY_PROFILE)
    bpatch_function_count++;
    if ((bpatch_function_count % 10) == 0)
        fprintf(stderr, "bpatch_function_count: %d (%d)\n",
                bpatch_function_count, bpatch_function_count*sizeof(BPatch_function));
#endif

  // there should be at most one BPatch_func for each int_function per process
    //  assert( proc && !proc->func_map->defines(func) );
    assert( addSpace && !addSpace->func_map->defines(func) );
  _srcType = BPatch_sourceFunction;

  localVariables = new BPatch_localVarCollection;
  funcParameters = new BPatch_localVarCollection;
  retType = NULL;

  addSpace->func_map->add(_func, this);
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
BPatch_function::BPatch_function(BPatch_addressSpace *_addSpace, int_function *_func,
				 BPatch_type * _retType, BPatch_module *_mod) :
	addSpace(_addSpace), mod(_mod), cfg(NULL), cfgCreated(false), liveInit(false), func(_func),
	varsAndParamsValid(false)
{
  //assert(proc && !proc->func_map->defines(_func));
  assert(addSpace && !addSpace->func_map->defines(_func));

  _srcType = BPatch_sourceFunction;

  localVariables = new BPatch_localVarCollection;
  funcParameters = new BPatch_localVarCollection;
  retType = _retType;

  //proc->func_map->add(_func, this);
  addSpace->func_map->add(_func,this);
  if (mod) {
      // Track for deletion
      mod->all_funcs.push_back(this);
  }
};


BPatch_function::~BPatch_function()
{
    if (localVariables) delete localVariables;
    if (funcParameters) delete funcParameters;

    if (cfg) delete cfg;

    // Remove us from the proc map...
    //if (proc && proc->func_map)
    //  proc->func_map->undefine(lowlevel_func());

    if (addSpace && addSpace->func_map)
        addSpace->func_map->undefine(lowlevel_func());
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
    string name = func->prettyName();
    strncpy(s, name.c_str(), len);
    return s;
}

#ifdef IBM_BPATCH_COMPAT
const char *BPatch_function::getNameDPCL()
{
    assert(func);
    return func->prettyName().c_str();
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
char *BPatch_function::getTypedNameInt(char *s, int len)
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

bool BPatch_function::getNamesInt(BPatch_Vector<const char *> &names)
{
    assert(func);
    unsigned pre_size = names.size();

    for (unsigned i = 0; i < func->prettyNameVector().size(); i++) {
        names.push_back(func->prettyNameVector()[i].c_str());
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

bool BPatch_function::getMangledNamesInt(BPatch_Vector<const char *> &names)
{
    assert(func);
    unsigned pre_size = names.size();

    for (unsigned i = 0; i < func->symTabNameVector().size(); i++) {
        names.push_back(func->symTabNameVector()[i].c_str());
    }

    return names.size() > pre_size;
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
    constructVarsAndParams();
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
    if (!mod->isValid()) return NULL;
    constructVarsAndParams();
    return funcParameters->getAllVars();
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

    if (!mod->isValid()) return NULL;

    // if the function is not instrumentable, we won't find the point
    if (!isInstrumentable())
       return NULL;

    // function is generally uninstrumentable (with current technology)
    if (func->funcEntries().size() == 0) return NULL;

    BPatch_Vector<BPatch_point*> *result = new BPatch_Vector<BPatch_point *>;

    if (loc == BPatch_entry || loc == BPatch_allLocations) {
        const pdvector<instPoint *> entries = func->funcEntries();
        for (unsigned foo = 0; foo < entries.size(); foo++)
            result->push_back(addSpace->findOrCreateBPPoint(this, 
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
                  result->push_back(addSpace->findOrCreateBPPoint(
                                                              this, Cpoints[c], BPatch_subroutine));
	      c++;
	    } else {
                 result->push_back(addSpace->findOrCreateBPPoint(
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
             result->push_back(addSpace->findOrCreateBPPoint(
                                             this, points[i], BPatch_exit));
          }
          break;
        }
      case BPatch_subroutine:
        {
          const pdvector<instPoint *> &points = func->funcCalls();
          for (unsigned i = 0; i < points.size(); i++) {
             result->push_back(addSpace->findOrCreateBPPoint(
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

    if (!mod->isValid()) return NULL;

  // function is generally uninstrumentable (with current technology)
  if (func->funcEntries().size() == 0) return NULL;
  
  // Use an instruction iterator
  InstrucIterFunction ii(func);
    
  return BPatch_point::getPoints(ops, ii, this);
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
 * BPatch_function::findLocalVarInt()
 *
 * This function searchs for a local variable in the BPatch_function's
 * local variable collection.
 */
BPatch_localVar * BPatch_function::findLocalVarInt(const char * name)
{
    if (!mod->isValid()) return NULL;
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
BPatch_localVar * BPatch_function::findLocalParamInt(const char * name)
{
    if (!mod->isValid()) return NULL;
    constructVarsAndParams();
    BPatch_localVar * var = funcParameters->findLocalVar(name);
    return (var);
}

BPatch_flowGraph* BPatch_function::getCFGInt()
{
    assert(mod);
    if (!mod->isValid()) return NULL;
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

void BPatch_function::constructVarsAndParams(){
    if(varsAndParamsValid)
       return;
    mod->parseTypesIfNecessary();

    //Check flag to see if vars & params are already constructed
    std::vector<localVar *>vars;
    if(lowlevel_func()->ifunc()->symbol()->getLocalVariables(vars)) {
        for(unsigned i = 0; i< vars.size(); i++) 
	    {
	        vars[i]->fixupUnknown(mod->lowlevel_mod()->pmod()->mod());
	        localVariables->addLocalVar(new BPatch_localVar(vars[i]));
	    }    
    }
    std::vector<localVar *>parameters;
    if(lowlevel_func()->ifunc()->symbol()->getParams(parameters)) {
        for(unsigned i = 0; i< parameters.size(); i++) {
	        parameters[i]->fixupUnknown(mod->lowlevel_mod()->pmod()->mod());
	        BPatch_localVar *lparam = new BPatch_localVar(parameters[i]);
    	    funcParameters->addLocalVar(lparam);
	        params.push_back(lparam);
    	}    
    }
    if(!lowlevel_func()->ifunc()->symbol()->getReturnType()) {
        varsAndParamsValid = true;
        return;
    }
        
    if(!lowlevel_func()->ifunc()->symbol()->getReturnType()->getUpPtr())
        retType = new BPatch_type(lowlevel_func()->ifunc()->symbol()->getReturnType());
    else
        retType = (BPatch_type *)lowlevel_func()->ifunc()->symbol()->getReturnType()->getUpPtr();
    varsAndParamsValid = true;
}

BPatch_Vector<BPatch_localVar *> *BPatch_function::getVarsInt() 
{
    if (!mod->isValid()) return NULL;
    constructVarsAndParams();
    return localVariables->getAllVars(); 
}

BPatch_Vector<BPatch_variableExpr *> *BPatch_function::findVariableInt(
        const char *name)
{
    if (!mod->isValid()) return NULL;
   constructVarsAndParams();
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

      ret->push_back(new BPatch_variableExpr(imgPtr->getAddressSpace(),
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

BPatch_variableExpr *BPatch_function::getFunctionRefInt() 
{
  Address remoteAddress = (Address)getBaseAddrInt();
   char *fname = const_cast<char *>(func->prettyName().c_str());

   //  Need to figure out the type for this effective function pointer,
   //  of the form <return type> (*)(<arg1 type>, ... , <argn type>)

   //  Note:  getParamsInt allocates the vector
   assert(retType);
   char typestr[1024];
   sprintf(typestr, "%s (*)(", retType->getName());

   BPatch_Vector<BPatch_localVar *> *params = getParamsInt();
   assert(params);

   for (unsigned int i = 0; i < params->size(); ++i) {
     if (i >= (params->size() -1)) {
        //  no comma after last parameter
        sprintf(typestr, "%s %s", typestr, (*params)[i]->getName());
     } else 
        sprintf(typestr, "%s %s,", typestr, (*params)[i]->getName());
   }
   sprintf(typestr, "%s)", typestr);

   BPatch_type *type = addSpace->image->findType(typestr);
   if (!type) {
     fprintf(stderr, "%s[%d]:  cannot find type '%s'\n", FILE__, __LINE__, typestr);
   }
   assert(type);

   //  only the vector was newly allocated, not the parameters themselves
   delete [] params;

#if defined( arch_ia64 )
   // IA-64 function pointers actually point to structures.  We insert such
   // a structure in the mutatee so that instrumentation can use it. */
   Address entryPoint = (Address)getBaseAddr();
   
   //RUTAR, need to change this over when we start to support IA64
   assert(addSpace->getType() == TRADITIONAL_PROCESS);
   BPatch_process * proc = dynamic_cast<BPatch_process *>(addSpace);
   Address gp = proc->llproc->getTOCoffsetInfo( entryPoint );

   remoteAddress = proc->llproc->inferiorMalloc( sizeof( Address ) * 2 );
   assert( remoteAddress != (Address)NULL );

   if (!proc->llproc->writeDataSpace( (void *)remoteAddress, sizeof( Address ), & entryPoint ))
          fprintf(stderr, "%s[%d]:  writeDataSpace failed\n", FILE__, __LINE__);
   if (!proc->llproc->writeDataSpace( (void *)(remoteAddress + sizeof( Address )), 
                                           sizeof( Address ), & gp ))
   fprintf(stderr, "%s[%d]:  writeDataSpace failed\n", FILE__, __LINE__);

   
   AstNodePtr *wrapper = new AstNodePtr(AstNode::operandNode(AstNode::Constant, (void *) remoteAddress));
   // variableExpr owns the AST
   return new BPatch_variableExpr(fname, proc, wrapper, type, (void *) remoteAddress);
	//return (BPatch_function::voidVoidFunctionPointer)remoteAddress;

#else
   //  For other platforms, the baseAddr of the function should be sufficient.

   //  In truth it would make more sense for this to be a BPatch_constExpr,
   //  But since we are adding this as part of the DPCL compatibility process
   //  we use the IBM API, to eliminate one API difference.

   AstNodePtr *ast = new AstNodePtr(AstNode::operandNode(AstNode::Constant, (void *) remoteAddress));
   
   // the variableExpr owns the ast now.
   return new BPatch_variableExpr(fname, addSpace, ast, type, (void *) remoteAddress);
   
#endif

} /* end getFunctionRef() */

#ifdef IBM_BPATCH_COMPAT

bool BPatch_function::getLineNumbersInt(unsigned int &start, unsigned int &end) {
  char name[256];
  unsigned int length = 255;
  return getLineAndFileInt(start, end, name, length);
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
      delete vars;
   }
}

bool BPatch_function::containsSharedBlocks() {
    return func->containsSharedBlocks();
}

// isPrimary: function will now use this name as a primary output name
// isMangled: this is the "mangled" name rather than demangled (pretty)
const char *BPatch_function::addNameInt(const char *name,
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

/* This function should be deprecated. */
bool BPatch_function::getLineAndFileInt( unsigned int & start, 
                                         unsigned int & end, 
                                         char * filename, 
                                         unsigned int max ) 
{
   Address startAddress = func->getAddress();
   Address endAddress = startAddress + func->getSize_NP();
	
   BPatch_Vector<BPatch_statement> startLines;
   if ( ! mod->getSourceLines( startAddress, startLines ) ) { return false; }
   if ( startLines.size() == 0 ) { return false; }
   start = startLines[0].lineNumber();
	
   /* Arbitrarily... */
   strncpy( filename, startLines[0].fileName(), max );
	
   BPatch_Vector<BPatch_statement> endLines;
   if ( ! mod->getSourceLines( endAddress, endLines ) ) { return false; }
   if ( endLines.size() == 0 ) { return false; }
   end = endLines[0].lineNumber();

return true;
} /* end getLineAndFile() */

/* This function should be deprecated. */
bool BPatch_function::getLineToAddrInt( unsigned short lineNo, BPatch_Vector< unsigned long > & buffer, bool /* exactMatch */ ) {
	std::vector< std::pair< unsigned long, unsigned long > > ranges;
	if( ! mod->getAddressRanges( NULL, lineNo, ranges ) ) { return false; }
	
	for( unsigned int i = 0; i < ranges.size(); ++i ) {
		buffer.push_back( ranges[i].first );
		}
	
	return true;
	} /* end getLineToAddr() */

unsigned int BPatch_function::getContiguousSizeInt() {
   Address start, end;
   start = func->getAddress();
   end = start + func->getSize_NP();
    
    bblInstance* block = func->findBlockInstanceByAddr(start);
    while (block != NULL) {
       end = block->firstInsnAddr() + block->getSize();
       block = func->findBlockInstanceByAddr(end);
    }
    return end - start;
}

bool BPatch_function::findOverlappingInt(BPatch_Vector<BPatch_function *> &funcs) {
    assert(func);
    assert(addSpace);

    pdvector<int_function *> overlappingIntFuncs;
    if (!func->getOverlappingFuncs(overlappingIntFuncs)) {
        // No overlapping functions
        return false;
    }

    // We now need to map from int_functions to BPatch_functions
    for (unsigned i = 0; i < overlappingIntFuncs.size(); i++) {
        funcs.push_back(addSpace->findOrCreateBPFunc(overlappingIntFuncs[i],
                                                 mod));
    }

    return true;
}

#if defined(cap_slicing)
/* The following structures are used in the creation of Program/Data/Control Dependence Graphs */
/* used to store keep track of the node where a register with number 'reg' is updated */ 
typedef struct RegisterUpdate{
  int reg;
  BPatch_dependenceGraphNode* node;
} regUpdate;

/* used to store which instruction last modified a given memory location */
typedef struct MemoryUpdate{
  long immediate;
  int reg0;
  int reg1;
  int scale;
  BPatch_dependenceGraphNode* node;
} memUpdate;

#if defined(interprocedural)

/* should probably included from another file*/
#if defined(arch_x86)
#define EAX 0
#define ESP 4
#define EBP 5
#endif

/* The following type defs are useful for code reuse in interprocedural slicing*/
#if defined(sparc_sun_solaris2_4)
typedef regUpdate ParameterType;
typedef regUpdate ReturnParameterType;
#elif defined(arch_x86)
typedef memUpdate ParameterType;
typedef regUpdate ReturnParameterType;
#endif

#endif



/* as you traverse instructions in a basic block, keep which registers/mem locations are last modified by which instruction */
typedef struct VectorEntry{
  BPatch_basicBlock * basicBlock;
  BPatch_Vector<regUpdate*>* registers;
  BPatch_Vector<memUpdate*>* memLocs;
#if defined(interprocedural)
  BPatch_Vector<BPatch_dependenceGraphNode*>* prevCalls;
#endif
} entry;

/* stores a list of basic blocks which are predecessors of this 'basicBlock'*/
typedef struct CDGVectorEntry{
  BPatch_basicBlock * basicBlock;
  BPatch_Vector<BPatch_basicBlock*> depends;
} CDGentry;

////////////////////////////////////////////    INTERPROCEDURAL      ///////////////////////////
#if defined(interprocedural)
typedef struct ParameterWithFuncAddress{
  void* address;
  union {
    BPatch_Vector<ParameterType*>* parameters;
    BPatch_Vector<ReturnParameterType*>*returnVals;
  } v; // v for vector
} annotation;

/* returns true if a given actual parameter p1 matches a given formal parameter p2  */
bool parameterMatch(ParameterType* p1, ParameterType* p2) 
{
#if defined(sparc_sun_solaris2_4)
  return p1->reg+16 == p2->reg? true: false;
#elif defined(arch_x86)
  if(p1->reg0 == ESP && p2->reg0 == EBP &&
     p1->scale == p2->scale && p1->immediate+8 == p2->immediate )
    // The difference between immediate values is 8, since prev base pointer and return address are also pushed onto stack after the parameters
    return true;
  return false;
#endif
}

/* returns true if a given actual return value p1 matches a given formal return value p2 */
bool returnMatch(ReturnParameterType* p1, ReturnParameterType* p2) 
{
#if defined(sparc_sun_solaris2_4)
  return p1->reg+16 == p2->reg? true: false;
#elif defined(arch_x86)
  return (p1->reg == p2->reg && p1->reg == EAX)? true: false;
#endif
}


/*
 * Locates the dependencies between a set of formal parameters and a set of actual parameters
 */
void BPatch_function::identifyParamDependencies(BPatch_function* callee, 
                                                void* calleeAddress) 
{
  BPatch_Vector<ParameterType*>* parameters = (BPatch_Vector<ParameterType*>*)callee->getAnnotation("formalParams");
  
  BPatch_Vector<ParameterType*>* outParams = NULL;
  unsigned ctr;
  for(ctr=0; true; ctr++) {
    annotation* ann = (annotation*)this->getAnnotation("actualParams",ctr);
    if(ann == NULL) break;
    if(ann->address == calleeAddress) {
        outParams = ann->v.parameters;
        unsigned i,j;
        ParameterType* p1;
        fprintf(stderr,"1. Size: %d\t2. Size %d\n",outParams->size(),parameters->size());
        for(i=0; i<outParams->size(); i++) {
            p1 = (*outParams)[i];
            for(j=0; j<parameters->size(); j++) {
                ParameterType* p2 = (*parameters)[j];
                if (parameterMatch(p1,p2)) {
                    fprintf(stderr,"\n\nDepends\n\n");
                    // TODO: check this line
                    p2->node->inter->push_back(new BPatch_dependenceGraphEdge(p2->node,p1->node));
                }
            }
        }   
    }
  }
  if(outParams == NULL) {
    fprintf(stderr,"No such Annotation!!\n");
    return;
  }
  
}

/*
 * Locates the dependencies between a set of formal returns and a set of actual returns
 */
void identifyReturnDependencies(BPatch_function* thisFunc, BPatch_function* callee, void* calleeAddress) {
  BPatch_Vector<ReturnParameterType*>* returnVals = (BPatch_Vector<ReturnParameterType*>*)callee->getAnnotation("returnVals");

  BPatch_Vector<ReturnParameterType*>* returned = NULL;
  unsigned ctr;
  for(ctr=0; true; ctr++) {
    annotation* ann = (annotation*)thisFunc->getAnnotation("returned",ctr);
    if(ann == NULL) break;
    if(ann->address == calleeAddress) {
      returned = ann->v.returnVals;
      //      returnedNodes = ann->nodes;
      //      break;
      unsigned i,j;
      ReturnParameterType* r1;
      fprintf(stderr,"Ret - 1. Size: %d\t2. Size %d\n",returned->size(),returnVals->size());
      for(i=0; i<returned->size(); i++) {
	r1 = (*returned)[i];
	for(j=0; j<returnVals->size(); j++) {
	  ReturnParameterType* r2 = (*returnVals)[j];
	  if(returnMatch(r1,r2)) { // r1+16 == (*returnVals)[j]->reg) {
	    fprintf(stderr,"\nReturn Depends\n\n");
	    r1->node->inter->push_back(new BPatch_dependenceGraphEdge(r1->node,r2->node));
	  }
	}
      }
    }
  }
  if(returned == NULL) {
    fprintf(stderr,"No such Annotation!!\n");
    return;
  }
}

/*
 * Locates dependencies between two functions (this and callee) that are results of parameter passing
 */
void BPatch_function::identifyDependencies(BPatch_function* callee, void* calleeAddress) 
{
  identifyParamDependencies(this, callee,  calleeAddress);
  identifyReturnDependencies(this, callee, calleeAddress);
}

/* Used to copy from one function call vector to another */
void copyFuncCallVector(BPatch_Vector<BPatch_dependenceGraphNode*>* original,BPatch_Vector<BPatch_dependenceGraphNode*>* copy) {
  unsigned int i;
  for(i=0; i<original->size(); i++) {
    copy->push_back((*original)[i]);
  }
}
#else /*!cap_slicing*/
void BPatch_function::identifyParamDependencies(BPatch_function*, 
                                                void*)
{
}

#endif

/* If we come across a basic block more than once, check if we need to re-visit it
   Returns true if register lists or memory lists do not have the same contents */ // TODO: add prevCalls
bool needsVisiting(BPatch_Vector<regUpdate*>* firstRegs,BPatch_Vector<memUpdate*>* firstMems, BPatch_Vector<regUpdate*>* secondRegs, BPatch_Vector<memUpdate*>* secondMems) {
  // is there a better method than O(m*n)? Do we know the limits of reg id's?
  unsigned int i, j, numMatching;
  numMatching=0;

  for(i=0; i<firstRegs->size(); i++) {
    bool match = false;
    for(j=0; j<secondRegs->size(); j++) {
      // if this reg has a matching reg in secondRegs
      regUpdate* first = (*firstRegs)[i];
      regUpdate* second = (*secondRegs)[j];
      if(first->reg == second->reg && first->node->getBPInstruction()->getAddress() == second->node->getBPInstruction()->getAddress() ) {
	numMatching++;
	match=true;
	break;
      }
    }
    // if there isn't any matching reg in secondRegs, we need to visit this basic block
    if(match == false) {
      return true;
    }
  }
  if(numMatching != secondRegs->size()) {
    return true;
  }

  numMatching=0;
  for(i=0; i<firstMems->size(); i++) {
    bool match = false;
    for(j=0; j<secondMems->size(); j++) {
      // if this reg has a matching reg in secondRegs
      memUpdate* first = (*firstMems)[i];
      memUpdate* second = (*secondMems)[j];
      if(first->reg0 == second->reg0 && first->reg1 == second->reg1 && first->scale == second->scale && first->immediate == second->immediate && 
      first->node->getBPInstruction()->getAddress() == second->node->getBPInstruction()->getAddress() ) {
	numMatching++;
	match=true;
	break;
      }
    }
    // if there isn't any matching reg in secondRegs, we need to visit this basic block
    if(match == false) {
      return true;
    }
  }
  if(numMatching != secondMems->size()) {
    return true;
  }

  return false;
}

/* Used to copy from one regUpdate vector to another */
void copyRegisterVector(BPatch_Vector<regUpdate*>* original,BPatch_Vector<regUpdate*>* copy) {
  unsigned int i;
  for(i=0; i<original->size(); i++) {
    regUpdate* temp = (*original)[i];
    regUpdate * entry = (regUpdate*)malloc(sizeof(regUpdate));
    entry->reg = temp->reg;
    entry->node = temp->node;
    copy->push_back(entry);
  }
}

/* Used to copy from one memUpdate vector to another */
void copyMemLocVector(BPatch_Vector<memUpdate*>* original,BPatch_Vector<memUpdate*>* copy) {
  unsigned int i;
  for(i=0; i<original->size(); i++) {
    fflush(stdout);
    memUpdate* temp = (*original)[i];
    memUpdate * entry = (memUpdate*)malloc(sizeof(memUpdate));
    entry->reg0 = temp->reg0;
    entry->reg1 = temp->reg1;
    entry->scale = temp->scale;
    entry->immediate = temp->immediate;
    entry->node = temp->node;
    copy->push_back(entry);
  }
}

int* initialize_array(int* array, int size,int num) {
  int i;
  for(i=0;i<size;i++)
    array[i] = num;
  return array;
}

/*
 * print a given graph (Control dependence graph, program dependence graph, etc) in tabular format
 */
void outputGraph(BPatch_Vector<BPatch_dependenceGraphNode*>* graph) {
  unsigned i,j;
  printf("Output the results. Total Node Num: %d\n", graph->size());
  fflush(stdout);
  for(i=0; i<graph->size(); i++) {
    printf("\nAddress: %ld\t Dominates: ",((*graph)[i]->getBPInstruction())==NULL?0:(long)((*graph)[i]->getBPInstruction()->getAddress()));
    BPatch_Vector<BPatch_dependenceGraphEdge*>* list = new BPatch_Vector<BPatch_dependenceGraphEdge*>();
    (*graph)[i]->getIncomingEdges(*list);
    for(j=0; j<list->size(); j++) {
      printf("%ld ",((*list)[j]->source->getBPInstruction())==NULL?0:(long)((*list)[j]->source->getBPInstruction()->getAddress()) );
    }
  }
}

/* merge two graphs
 * for each BPatch_dependenceGraphNode in graph2, insert its predecessors to the predecessor list of the corresponding BPatch_dependenceGraphNode in graph1
 * Assumption: each instance of BPatch_dependenceGraphNode in graph 2 should have a matching BPatch_dependenceGraphNode object in graph1
 */
bool mergeGraphs(BPatch_Vector<BPatch_dependenceGraphNode*>* graph1, BPatch_Vector<BPatch_dependenceGraphNode*>* graph2) {
  unsigned int i,j,k;
  int ins_ind=0;
  for(i=0; i<graph2->size(); i++) {
    BPatch_Vector<BPatch_dependenceGraphEdge*>* preds1 = new BPatch_Vector<BPatch_dependenceGraphEdge*>();
    BPatch_Vector<BPatch_dependenceGraphEdge*>* preds2 = new BPatch_Vector<BPatch_dependenceGraphEdge*>();
    (*graph2)[i]->getIncomingEdges(*preds2);
    // general case. ith element of both graphs should be related to the same instruction.
    if(i < graph1->size() && (*graph1)[i]->getBPInstruction() == (*graph2)[i]->getBPInstruction() ) { // should always evaluate to true in current situation
      (*graph1)[i]->getIncomingEdges(*preds1);
    }
    else { // search for the instruction in case previous if clause fails for some reason.
      bool found = false;
      for(j=0; j<graph1->size(); j++) {
	if((*graph1)[j]->getBPInstruction()!= NULL && (*graph2)[i]->getBPInstruction()!= NULL && 
	   (*graph1)[j]->getBPInstruction()->getAddress() == (*graph2)[i]->getBPInstruction()->getAddress()) {
	  found = true;
	  (*graph1)[j]->getIncomingEdges(*preds1);
	  ins_ind = j;
	  break;
	}
      }
      // if not found in the first list, there is a mismatch between instructions! Not tolerable at this time.
      if(!found)
	return false;
    }
    // iterate through both lists and update first list if there are some objects in list 2 which are not matched in list 1.
    for(j=0; j<preds2->size(); j++) {
      bool found=false;
      for(k=0; k<preds1->size(); k++) {
	if((*preds2)[j]->source->getBPInstruction()->getAddress() == (*preds1)[k]->source->getBPInstruction()->getAddress()) {
	  found=true;
	  break;
	}
      }
      if(!found) {
	(*graph1)[ins_ind]->addToIncoming((*preds2)[j]->source);
      }
    }
    delete preds1;
    delete preds2;
  }

  return true;
}


/* Used to copy the dependence graph of any kind.
 * Before calling mergeGraphs, I copy the first one since it will get modified by the mergeGraphs method.
 */
bool copyDependenceGraph(BPatch_Vector<BPatch_dependenceGraphNode*> &target, BPatch_Vector<BPatch_dependenceGraphNode *> &src)
{
  if (!src.size()) {
    fprintf(stderr,"%s[%d]: Graph is empty (NULL)\n", FILE__, __LINE__);
    return false;
  }

  unsigned i,j,k;
  BPatch_Vector<BPatch_dependenceGraphEdge*>* preds; //successors;

  for (i=0; i<src.size(); i++) {
     BPatch_dependenceGraphNode* newInst = new BPatch_dependenceGraphNode(src[i]->getBPInstruction(),
           new BPatch_Vector<BPatch_dependenceGraphEdge*>(),
           new BPatch_Vector<BPatch_dependenceGraphEdge*>());
     target.push_back(newInst);
  }

  for (i=0; i<src.size(); i++) {
     preds = new BPatch_Vector<BPatch_dependenceGraphEdge*>();
     src[i]->getIncomingEdges(*preds);
     for (j=0; j<preds->size(); j++) {
        bool found=false;
        for (k=0; k<target.size(); k++) {
           if ( ( !target[k]->getBPInstruction() && !(*preds)[j]->source->getBPInstruction()) 
                 || (target[k]->getBPInstruction() != NULL
                    && (*preds)[j]->source->getBPInstruction()!=NULL 
                    && (target[k]->getBPInstruction()->getAddress() 
                       == (*preds)[j]->source->getBPInstruction()->getAddress() ) ) ) {
              // add to predecessor list of i'th obj.
              target[i]->addToIncoming(target[k]);
              // add to successor list of k'th object
              target[k]->addToOutgoing(target[i]);
              found=true;
              break;
           }
      }
      assert(found);
    }
  }
  return true;
}

/*
 * Given an instruction inst, find the backward slice of the program from that instruction.
 * This assumes that the variable/register that is in the slicing criteria is modified at this instruction
 * This function builds the extended program dependence graph if not built before, then searched for the given instruction in that graph 
 */
BPatch_dependenceGraphNode* BPatch_function::getSliceInt(BPatch_instruction* inst) 
{
   if (inst == NULL)
      return NULL;

   Annotatable<BPatch_dependenceGraphNode *, extended_prog_dep_graph_a> &edg = *this;
   createExtendedProgramDependenceGraph();

   for (unsigned int i = 0; i < edg.size(); ++i)
      if ( (NULL != edg[i]->getBPInstruction())
            && (inst->getAddress() == edg[i]->getBPInstruction()->getAddress()) )
         return edg[i];

#if 0
   BPatch_Vector<BPatch_dependenceGraphNode*>* extendedProgramDependenceGraph = (BPatch_Vector<BPatch_dependenceGraphNode*>*)getAnnotation("ExtendedProgramDependenceGraph");
   for (unsigned int i=0; i<extendedProgramDependenceGraph->size(); i++) {
      if ((*extendedProgramDependenceGraph)[i]->getBPInstruction() != NULL && (*extendedProgramDependenceGraph)[i]->getBPInstruction()->getAddress() == inst->getAddress()) {
         return (*extendedProgramDependenceGraph)[i];
      }
   }
#endif
   return NULL;
}

/*
 * Creates extended program dependence graph (program dependence graph + another graph which I call helper graph for now)
 */
void BPatch_function::createExtendedProgramDependenceGraph() 
{
   Annotatable<BPatch_dependenceGraphNode *, prog_dep_graph_a> &dg = *this;
   Annotatable<BPatch_dependenceGraphNode *, extended_prog_dep_graph_a> &edg = *this;
   Annotatable<BPatch_dependenceGraphNode *, dep_helper_graph_a> &helpergraph = *this;

   if (!edg.size()) {
      if (!dg.size()) {
         createProgramDependenceGraph();
      }
      if (!copyDependenceGraph(edg.getDataStructure(), dg.getDataStructure())) {
         fprintf(stderr, "%s[%d]:  failed to copy dependence graph\n", FILE__, __LINE__);
         return;
      }
      mergeGraphs(&edg.getDataStructure(), &helpergraph.getDataStructure());
   }

#if 0
   if(getAnnotation("ExtendedProgramDependenceGraph") == NULL) {
      if(getAnnotation("ProgramDependenceGraph") == NULL)
         createProgramDependenceGraph();
      BPatch_Vector<BPatch_dependenceGraphNode*>* extendedProgramDependenceGraph = copyDependenceGraph((BPatch_Vector<BPatch_dependenceGraphNode*>*)getAnnotation("ProgramDependenceGraph"));
      // now add the unconditional jump and return instructions.
      mergeGraphs(extendedProgramDependenceGraph,(BPatch_Vector<BPatch_dependenceGraphNode*>*)getAnnotation("HelperGraph"));
    setAnnotation(getAnnotationType("ExtendedProgramDependenceGraph"), new Annotation(extendedProgramDependenceGraph));
  }
#endif
}

/*
 * Creates program dependence graph (Control dependence Graph + data dependence graph)
 */
void BPatch_function::createProgramDependenceGraph(/*BPatch_flowGraph* cfg*/) 
{
   Annotatable<BPatch_dependenceGraphNode *, prog_dep_graph_a> &pdg = *this;
   Annotatable<BPatch_dependenceGraphNode *, control_dep_graph_a> &cdg = *this;
   Annotatable<BPatch_dependenceGraphNode *, data_dep_graph_a> &ddg = *this;

   if (!pdg.size()) {
      if (!cdg.size()) {
         createControlDependenceGraph();
      }
      if (!ddg.size()) {
         createDataDependenceGraph();
      }

      if (!copyDependenceGraph(pdg.getDataStructure(), cdg.getDataStructure())) {
         fprintf(stderr, "%s[%d]:  failed to copy control dep. graph\n", FILE__, __LINE__);
         return;
      }
      mergeGraphs(&pdg.getDataStructure(), &ddg.getDataStructure());
   }

#ifdef print_graphs
   if (pdg.size()) {
      printf("Progam Dependence Graph. Total Node Num: %d\n", pdg.size());
      outputGraph(allNodes);
   }
   else {
      printf("Program Dependence Graph. No Nodes\n");
   }
#endif

#if 0
      if(getAnnotation("ProgramDependenceGraph") == NULL) {
         if(getAnnotation("ControlDependenceGraph") == NULL) {
            // updates the global controlDependenceGraph variable
            createControlDependenceGraph();
         }
         if(getAnnotation("DataDependenceGraph") == NULL) {
            // updates the global dataDependenceGraph variable
            createDataDependenceGraph();
         }
         BPatch_Vector<BPatch_dependenceGraphNode*>* programDependenceGraph = copyDependenceGraph((BPatch_Vector<BPatch_dependenceGraphNode*>*)getAnnotation("ControlDependenceGraph"));
         mergeGraphs(programDependenceGraph,(BPatch_Vector<BPatch_dependenceGraphNode*>*)getAnnotation("DataDependenceGraph")); // updates programDependenceGraph
         setAnnotation(getAnnotationType("ProgramDependenceGraph"), new Annotation(programDependenceGraph));
      } 

#ifdef print_graphs
      BPatch_Vector<BPatch_dependenceGraphNode*> * allNodes = (BPatch_Vector<BPatch_dependenceGraphNode*>*)getAnnotation("ProgramDependenceGraph");
      printf("Progam Dependence Graph. Total Node Num: %d\n", allNodes->size());
      outputGraph(allNodes);
#endif
#endif
}

   /*
    * BPatch_function::getProgramDependenceGraph
 *
 * given a BPatch_instruction object, returns the corresponding BPatch_dependenceGraphNode object
 * which enables accessing the predecessors and successors of this instruction
 */
BPatch_dependenceGraphNode* BPatch_function::getProgramDependenceGraphInt(BPatch_instruction* inst) 
{
   Annotatable<BPatch_dependenceGraphNode *, prog_dep_graph_a> &pdg = *this;

   if (!pdg.size()) {
      createProgramDependenceGraph();
   }

   for (unsigned int i =0; i<pdg.size(); i++) {
      if (pdg[i]->getBPInstruction() != NULL
            && pdg[i]->getBPInstruction()->getAddress() == inst->getAddress()) {
         return pdg[i];
      }
   }

#if 0
   unsigned int i;
   if(getAnnotation("ProgramDependenceGraph") == NULL) {
      createProgramDependenceGraph();
   }
   BPatch_Vector<BPatch_dependenceGraphNode*>* programDependenceGraph = (BPatch_Vector<BPatch_dependenceGraphNode*>*)getAnnotation("ProgramDependenceGraph");
   for(i=0; i<programDependenceGraph->size(); i++) {
      if((*programDependenceGraph)[i]->getBPInstruction() != NULL && (*programDependenceGraph)[i]->getBPInstruction()->getAddress() == inst->getAddress()) {
         return (*programDependenceGraph)[i];
      }
   }
#endif
   return NULL;
}

/*
 * BPatch_function::getControlDependenceGraph
 * 
 * given a BPatch_instruction object, returns the corresponding BPatch_dependenceGraphNode object
 * that has information about which instructions dominate/are dominated by this instruction in the
 * control dependence graph
 */
BPatch_dependenceGraphNode* BPatch_function::getControlDependenceGraphInt(BPatch_instruction* inst) 
{
   Annotatable<BPatch_dependenceGraphNode *, control_dep_graph_a> &cdg = *this;
   if (!cdg.size()) {
      createControlDependenceGraph();
   }

   for (unsigned int i = 0; i < cdg.size(); ++i) {
      if (cdg[i]->getBPInstruction() != NULL
            && cdg[i]->getBPInstruction()->getAddress() == inst->getAddress()) {
         return cdg[i];
      }
   }

#if 0
   unsigned int i;
   if(getAnnotation("ControlDependenceGraph") == NULL) {
      createControlDependenceGraph();
   }
   BPatch_Vector<BPatch_dependenceGraphNode*>* controlDependenceGraph = (BPatch_Vector<BPatch_dependenceGraphNode*>*)getAnnotation("ControlDependenceGraph");
   for(i=0; i<controlDependenceGraph->size(); i++) {
      if((*controlDependenceGraph)[i]->getBPInstruction() != NULL && (*controlDependenceGraph)[i]->getBPInstruction()->getAddress() == inst->getAddress()) {
         return (*controlDependenceGraph)[i];
      }
   }
#endif
   return NULL;
}

/*
 * BPatch_function::getDataDependenceGraph
 * 
 * given a BPatch_instruction object, returns the corresponding BPatch_dependenceGraphNode object
 * that has information about which instructions dominate/are dominated by this instruction in the
 * data dependence graph
 */
BPatch_dependenceGraphNode* BPatch_function::getDataDependenceGraphInt(BPatch_instruction* inst) 
{
   Annotatable<BPatch_dependenceGraphNode *, data_dep_graph_a> &ddg = *this;
   if (!ddg.size()) {
      createDataDependenceGraph();
   }

   for (unsigned int i = 0; i < ddg.size(); ++i) {
      if (ddg[i]->getBPInstruction()->getAddress() == inst->getAddress()) {
         return ddg[i];
      }
   }

#if 0
   unsigned int i;
  if(getAnnotation("DataDependenceGraph") == NULL) {
    createDataDependenceGraph();
  }
  BPatch_Vector<BPatch_dependenceGraphNode*>* dataDependenceGraph = (BPatch_Vector<BPatch_dependenceGraphNode*>*)getAnnotation("DataDependenceGraph");
  for(i=0; i<dataDependenceGraph->size(); i++) {
    if((*dataDependenceGraph)[i]->getBPInstruction()->getAddress() == inst->getAddress()) {
      return (*dataDependenceGraph)[i];
    }
  }
#endif
  return NULL;
}

/*
 * Using post-dominator information, marks the control dependencies between basic blocks
 */
BPatch_Vector<BPatch_basicBlock*>** createBlockDependency(int num_blocks, 
      BPatch_basicBlock** blocks) 
{
   unsigned j;
   int i;
   BPatch_Vector<BPatch_basicBlock*>** dependencies = (BPatch_Vector<BPatch_basicBlock*>**)malloc(sizeof(BPatch_Vector<BPatch_basicBlock*>*)*num_blocks);//new BPatch_Vector<BPatch_basicBlock*>[num_blocks];
   for(i=0; i<num_blocks; i++) {
      dependencies[i] = new BPatch_Vector<BPatch_basicBlock*>();
   }
   BPatch_Vector<BPatch_basicBlock*>* out;
   int* parent_check = initialize_array(new int[num_blocks],num_blocks,-1);
   for(i=0; i<num_blocks; i++) {
      out = new BPatch_Vector<BPatch_basicBlock*>();
      (blocks[i])->getTargets(*out);
      for(j=0; j<out->size(); j++) {
         if( ! ((*out)[j]->postdominates(blocks[i])) ) {
            // Work on this edge right now;
            // mark all parents of blocks[i]
            BPatch_basicBlock* temp = blocks[i];
            // According to Ferrante et al. page 325, marking only this node and its parent is enough
            parent_check[ temp->getBlockNumber() ] = 1;
            temp = temp->getImmediatePostDominator();
            if(temp!=NULL) {
               parent_check[ temp->getBlockNumber() ] = 1;
            }
            // traverse from out[j] to one of the parents marked in the previous step
            temp = (*out)[j];
            while(temp!=NULL && parent_check[ temp->getBlockNumber() ] != 1) {
               // mark them as control dependent to blocks[i]
               dependencies[ temp->getBlockNumber() ]->push_back(blocks[i]);
               temp = temp->getImmediatePostDominator();
            }
         }
      }
      delete out;
   }
   free(parent_check);
   return dependencies;
}

void determineReturnBranchDependencies(BPatch_Vector<BPatch_dependenceGraphNode*>* controlDependenceGraph, BPatch_Vector<BPatch_dependenceGraphNode*>* helperGraph, BPatch_Vector<BPatch_basicBlock*>** dependencies,
      BPatch_Vector<BPatch_basicBlock*>* blockNumbers, BPatch_basicBlock** blocks, int num_blocks,BPatch_dependenceGraphNode** last_inst_in_block,BPatch_dependenceGraphNode** last_inst_helper) {
   unsigned i,j;
   for(i=0; i<(unsigned)num_blocks; i++) {
      BPatch_basicBlock * blck = blocks[i];
      InstrucIter* iter;
      for(iter = new InstrucIter(blck); iter->hasMore(); (*iter)++) {
         BPatch_dependenceGraphNode* bpdom = new BPatch_dependenceGraphNode(iter->getBPInstruction(),new BPatch_Vector<BPatch_dependenceGraphEdge*>(),new BPatch_Vector<BPatch_dependenceGraphEdge*>());
         // printf("in block %d\n",blck->getBlockNumber()/*bpdom->getBPInstruction()->getParent()->getBlockNumber()*/); fflush(stdout);
         controlDependenceGraph->push_back(bpdom);
         helperGraph->push_back(new BPatch_dependenceGraphNode(iter->getBPInstruction(),
                  new BPatch_Vector<BPatch_dependenceGraphEdge*>(),new BPatch_Vector<BPatch_dependenceGraphEdge*>()));
         blockNumbers->push_back(blck);
      }
    last_inst_in_block[blck->getBlockNumber()] = (*controlDependenceGraph)[ controlDependenceGraph->size()-1 ];
    (*iter)--;
    if(iter->hasMore() && (iter->isAReturnInstruction() || iter->isAJumpInstruction()) )
      last_inst_helper[blck->getBlockNumber()] = (*helperGraph)[ helperGraph->size()-1 ];
    else
      last_inst_helper[blck->getBlockNumber()] = NULL;
  }

  for(i=0; i<controlDependenceGraph->size(); i++) {
    int blockNum = (*blockNumbers)[i]->getBlockNumber();//(*controlDependenceGraph)[i]->getBPInstruction()->getParent()->getBlockNumber();
    BPatch_dependenceGraphNode* instDom = (*controlDependenceGraph)[i];
    for(j=0; j< dependencies[blockNum]->size(); j++) {
      instDom->addToIncoming(last_inst_in_block[(*dependencies[blockNum])[j]->getBlockNumber() ]);
      last_inst_in_block[(*dependencies[blockNum])[j]->getBlockNumber() ]->addToOutgoing(instDom);
    }
    if(last_inst_helper[blockNum] != NULL) {
      (*helperGraph)[i]->addToIncoming(last_inst_helper[blockNum]);
      last_inst_helper[blockNum]->addToOutgoing((*helperGraph)[i]);
    }
  }
}

/*
 * Used to create the control dependence graph from the binary.
 * Called only once for each function
 */
void BPatch_function::createControlDependenceGraph() 
{
   Annotatable<BPatch_dependenceGraphNode *, control_dep_graph_a> &cdg = *this;
   Annotatable<BPatch_dependenceGraphNode *, dep_helper_graph_a> &helper_graph = *this;

   if (cdg.size()) {
      //  must've already generated it, return without doing anything
      return;
   }

   unsigned num_blocks;
   BPatch_flowGraph* cfg = getCFG();

#if 0
   // if we already calculated graph, return immediately
   if(getAnnotation("ControlDependenceGraph") != NULL)
      return;
   // if we are here, annotation type for helper graph must be missing. So, create it:
   createAnnotationType("HelperGraph");
#endif

   // get the dominators
   // already there! No need to do anything real

   // calculate dependency relation in terms of basic blocks

   BPatch_Set<BPatch_basicBlock*> all_b_b;

   cfg->getAllBasicBlocks(all_b_b); //getBasicBlocks(cfg, all_b_b);
   num_blocks = all_b_b.size();

   BPatch_basicBlock** blocks = new BPatch_basicBlock*[num_blocks];
   all_b_b.elements(blocks);

   // create the dependencies between blocks.
   BPatch_Vector<BPatch_basicBlock*>** dependencies = createBlockDependency(num_blocks, blocks);

   BPatch_Vector<BPatch_basicBlock*>* blockNumbers = new BPatch_Vector<BPatch_basicBlock*>();
#if 0
   BPatch_Vector<BPatch_dependenceGraphNode*>* controlDependenceGraph = new BPatch_Vector<BPatch_dependenceGraphNode*>();
   BPatch_Vector<BPatch_dependenceGraphNode*>* helperGraph = new BPatch_Vector<BPatch_dependenceGraphNode*>();
#endif

   // use dependency relation to figure out which instruction depends on which instruction
   BPatch_dependenceGraphNode** last_inst_in_block = 
      (BPatch_dependenceGraphNode**)malloc(sizeof(BPatch_dependenceGraphNode*)*num_blocks);
   BPatch_dependenceGraphNode** last_inst_helper = 
      (BPatch_dependenceGraphNode**)malloc(sizeof(BPatch_dependenceGraphNode*)*num_blocks);
   determineReturnBranchDependencies(&cdg.getDataStructure(), 
         &helper_graph.getDataStructure(), dependencies, blockNumbers, blocks, 
         num_blocks,last_inst_in_block,last_inst_helper);

#ifdef print_graphs
  printf("\nControlDependenceGraph\n");
  outputGraph(controlDependenceGraph);

  printf("\nHelperGraph\n");
  outputGraph(helperGraph);
#endif
#if 0
  setAnnotation(getAnnotationType("ControlDependenceGraph"), new Annotation(controlDependenceGraph));
  setAnnotation(getAnnotationType("HelperGraph"), new Annotation(helperGraph));
#endif
} // end of createControlDependenceGraph


/* find out which instruction last modified the register specified with reg_num 
 * and add that instruction to predecessor list of this node,
 * and add this node to successor list of that instruction
*/
void register_check(int reg_num, BPatch_Vector<regUpdate*>* registers, BPatch_dependenceGraphNode* node) {
  unsigned k, l;
  bool found = false;
  //if(reg_num != -1) {
  for(k=0; k<registers->size(); k++) {
    if(reg_num == (*(registers))[k]->reg) {
      found = true;
      BPatch_Vector<BPatch_dependenceGraphEdge*>* predPtr = new BPatch_Vector<BPatch_dependenceGraphEdge*>();
      node->getIncomingEdges(*predPtr);
      bool alreadyInList = false;
      void* successor_addr = (*(registers))[k]->node->getBPInstruction()->getAddress();
      for(l=0; l<predPtr->size(); l++) {
	if((*predPtr)[l]->source->getBPInstruction()->getAddress() == successor_addr) {
	  alreadyInList = true;
	  break;
	}
      }
      if(alreadyInList == false) {
	node->addToIncoming((*(registers))[k]->node);
	(*(registers))[k]->node->addToOutgoing(node);
      }
    }
  }
  // if not found in the registers list, uninitialized!! Do whatever you want.. Report?
  if(found == false) {
    // printf("\nUninitialized %d!!",reg_num);  
  }
}

/*
 * Find the entry blocks for this function, and put them into the workload after wrapping them with some data structures
 */
void pushEntryBlocks(BPatch_Vector<entry*>& blocks, BPatch_flowGraph* cfg) {
  // get a handle to entry block(s)
  unsigned i;
  BPatch_Vector<BPatch_basicBlock*>* entryBlocks =new BPatch_Vector<BPatch_basicBlock*>();
  cfg->getEntryBasicBlock(*entryBlocks);

  // start with entry blocks, so copy them to the front
  for(i=0; i<entryBlocks->size(); i++) {
    entry* entryBlEntry = (entry *)malloc(sizeof(entry));
    entryBlEntry->registers =new BPatch_Vector<regUpdate*>();
    entryBlEntry->memLocs =new BPatch_Vector<memUpdate*>();
    entryBlEntry->basicBlock = (*entryBlocks)[i];
#if defined(interprocedural)
    entryBlEntry->prevCalls = new BPatch_Vector<BPatch_dependenceGraphNode*>();
#endif
    blocks.push_back(entryBlEntry);
  }
  delete entryBlocks;
}

/*
 * find the node of a graph that contains the given BPatch_instruction
 */
BPatch_dependenceGraphNode* getNode(BPatch_Vector<BPatch_dependenceGraphNode*>* allNodes, BPatch_instruction* bpins) {
  unsigned j;
  BPatch_dependenceGraphNode * node;
  bool nodeAvailable = false;
  for(j=0; j<allNodes->size(); j++) {
    if((*allNodes)[j]->getBPInstruction()->getAddress() == bpins->getAddress()) {
      nodeAvailable=true;
      break;
    }
  }
  // if available, it is at j'th position
  if(nodeAvailable == true)
    node = (*allNodes)[j];
  else {
    node = new BPatch_dependenceGraphNode(bpins,new BPatch_Vector<BPatch_dependenceGraphEdge*>(),new BPatch_Vector<BPatch_dependenceGraphEdge*>());
    allNodes->push_back(node);
  }
  return node;
}

void handleCondBranch(BPatch_dependenceGraphNode* node, InstrucIter* iter, BPatch_Vector<BPatch_dependenceGraphNode*>* allNodes) {
  unsigned i;
  if(iter->hasPrev()) {
    void* pred_addr = (void*)iter->peekPrev();
    BPatch_dependenceGraphNode* prevNode = NULL;
    // the previous instruction should be in the previous index. Check anyway...
    if(allNodes->size() >= 2 && (*allNodes)[allNodes->size()-2]->getBPInstruction()->getAddress() == pred_addr) {
      prevNode = (*allNodes)[allNodes->size()-2];
    }
    else {
      for(i=0; i<allNodes->size(); i++) {
	if((*allNodes)[i]->getBPInstruction()->getAddress() == pred_addr) {
	  prevNode = (*allNodes)[i];
	  break;
	}
      }
      if(prevNode == NULL) {
	fprintf(stderr,"Control should have never reached here. (No previous instruction). Exiting...");
	exit(1);
      }
    }
    
    BPatch_Vector<BPatch_dependenceGraphEdge*>* predPtr = new BPatch_Vector<BPatch_dependenceGraphEdge*>();
    node->getIncomingEdges(*predPtr);
    bool alreadyInList = false;
    for(i=0; i<predPtr->size(); i++) {
      if((*predPtr)[i]->source->getBPInstruction()->getAddress() == pred_addr ) {
	alreadyInList = true;
	break;
      }
    }
    if(alreadyInList == false) {
      node->addToIncoming(prevNode);
      prevNode->addToOutgoing(node);
    }
  }
}

/*
 * If an instruction accesses a memory location, find out which registers/memory locations are affected
 */
void handleMemoryAccess(BPatch_dependenceGraphNode* node, BPatch_instruction* bpins, BPatch_Vector<regUpdate*>* copy_regs, 
			BPatch_Vector<memUpdate*>* copy_mems, const BPatch_addrSpec_NP* addrSpec
#if defined(arch_x86)
#else
			, InstrucIter* iter, int reg_offset
#endif
			) {
  unsigned k, l;
  if(addrSpec->getReg(0) == 0 && addrSpec->getReg(1) == 0 && addrSpec->getImm() == 0 && addrSpec->getScale() == 0 ) {
    return;
  }
  
  int reg0 = addrSpec->getReg(0);
  int reg1 = addrSpec->getReg(1);
#if defined(arch_x86)
#else
  if(reg0 != -1)
    reg0 = iter->adjustRegNumbers(reg0,reg_offset);
  if(reg1 != -1)
    reg1 = iter->adjustRegNumbers(reg1,reg_offset);
#endif
  
  // first, check whether the registers that are used in address computation are already in the copy_regs list!!
  if(reg0 != -1)
    register_check(reg0,copy_regs,node);
  if(reg1 != -1)
    register_check(reg1,copy_regs,node);
  
  bool found = false;
  for(k=0; k<copy_mems->size(); k++) {
    // if this is the one we're looking for
    if(reg0 == (*(copy_mems))[k]->reg0 && reg1 == (*(copy_mems))[k]->reg1 &&
       addrSpec->getScale() == (*(copy_mems))[k]->scale && addrSpec->getImm() == (*(copy_mems))[k]->immediate ) {
      
      if(bpins->isALoad()) { // reading from memory
	// Update dominates list of the node where this reg was last written
	BPatch_Vector<BPatch_dependenceGraphEdge*>* predPtr = new BPatch_Vector<BPatch_dependenceGraphEdge*>();
	node->getIncomingEdges(*predPtr);//(*(copy_mems))[k]->node->getSuccessors();
	bool alreadyInList = false;
	void* predecessor_addr = (*(copy_mems))[k]->node->getBPInstruction()->getAddress();
	for(l=0; l<predPtr->size(); l++) {
	  if((*predPtr)[l]->source->getBPInstruction()->getAddress() == predecessor_addr ) {
	    alreadyInList = true;
	    break;
	  }
	}
	if(alreadyInList == false) {
	  node->addToIncoming((*(copy_mems))[k]->node);
	  (*(copy_mems))[k]->node->addToOutgoing(node);
	}
      }
      if(bpins->isAStore()) {
	// update the memory entry so that it stores this node as the last location this reg was written
	(*(copy_mems))[k]->node = node;
      }
      found = true;
      break;
    }
  }
  // if(found == false && bpins->isALoad()) // do nothing
  // if a register is written which is not in the out list, we add this register and node to the list
  if(found == false && bpins->isAStore()) {
    memUpdate* cell = (memUpdate*)malloc(sizeof(memUpdate));
    cell->reg0 = reg0;
    cell->reg1 = reg1;
    cell->scale = addrSpec->getScale();
    cell->immediate = addrSpec->getImm();
    cell->node = node;
    // add this to the list...
    copy_mems->push_back(cell);
  }
  else if(found == false) { // && bpins->isALoad() => trivially true
    // Uninitialized
  }
}

/*
 * add new blocks to the workload after one is processed. The blocks that are directly reachable from block blck are added
 */
void addBlocksToWorkload(BPatch_Vector<entry*>& blocks, BPatch_basicBlock* blck, int* visitedBlocks,
			 BPatch_Vector<regUpdate*>* copy_regs, BPatch_Vector<memUpdate*>* copy_mems
#if defined(interprocedural)
			 , BPatch_Vector<BPatch_dependenceGraphNode*>* funcCalls
#endif
			 ) {
  unsigned j;
  // get the basic blocks which has en edge towards this basic block
  //BPatch_Vector<BPatch_edge*>* outgoing = new BPatch_Vector<BPatch_edge*>();
  BPatch_Vector<BPatch_basicBlock*> outgoing;
  //      BPatch_Vector<BPatch_edge*> incoming = *incomingPtr;
  //      blck->getOutgoingEdges(*outgoing);
  blck->getTargets(outgoing);
  
  for(j=0; j<outgoing.size(); j++) {
    int blockLocation;
    BPatch_basicBlock* outBlck = outgoing[j];//->target;
    blockLocation = visitedBlocks[outBlck->getBlockNumber()];
    if(blockLocation == -1 || needsVisiting(copy_regs, copy_mems, blocks[blockLocation]->registers, blocks[blockLocation]->memLocs)) {
      
      entry * ptr = (entry*)malloc(sizeof(entry));
      ptr->basicBlock = outBlck;
      
      BPatch_Vector<regUpdate*>* newCopy = new BPatch_Vector<regUpdate*>();
      copyRegisterVector(copy_regs, newCopy);
      
      BPatch_Vector<memUpdate*>* new_mems = new BPatch_Vector<memUpdate*>();
      copyMemLocVector(copy_mems, new_mems);
      
#if defined(interprocedural)
      BPatch_Vector<BPatch_dependenceGraphNode*>* newCalls = new BPatch_Vector<BPatch_dependenceGraphNode*>();
      copyFuncCallVector(funcCalls, newCalls);
      ptr->prevCalls = newCalls;
#endif
      
      ptr->registers = newCopy;
      ptr->memLocs = new_mems;

      blocks.push_back(ptr);	  
    }
  }
}

/* if a register is written by an instruction, store this info since future reads will depend on this write */
void handleWrittenRegister(BPatch_dependenceGraphNode* node, int rn, BPatch_Vector<regUpdate*>* copy_regs) {
  bool found = false;
  unsigned k;
  for(k=0; k<copy_regs->size(); k++) {
    // if a reg in out list is written by this instruction
    if(rn == (*(copy_regs))[k]->reg) {
      // update the register entry so that it stores this node as the last location this reg was written     
      (*(copy_regs))[k]->node = node;
      found = true;
    }
  }
  // if a register is written which is not in the out list, we add this register and node to the list
  if(found == false) {
    regUpdate* reg = (regUpdate*)malloc(sizeof(regUpdate));
    reg->reg = rn;
    reg->node = node;
    // add this to the list...
    copy_regs->push_back(reg);
  }
}

#if defined(interprocedural)
 bool isWrittenBefore(int reg_num, BPatch_Vector<regUpdate*>* registers) {
   unsigned i;
   for(i=0;i<registers->size();i++) {
     if(reg_num == (*(registers))[i]->reg) {
       return true;
     }
   }
   return false;
 }
 
 bool isReturnedValue(int r) {
#if defined(sparc_sun_solaris2_4)
   if(r >= 8 && r < 14) {
     return true;
   }
#elif defined(arch_x86)
   if(r == EAX)
     return true;
#endif
   return false;
 }

 bool isReturnValue(int r) {
#if defined(sparc_sun_solaris2_4)
   if(r >= 24 && r < 30) {
     return true;
   }
#elif defined(arch_x86)
   if(r == EAX)
     return true;
#endif
   return false;
 }
#endif

/*
 * Used to create the data dependence graph from the binary.
 * Called only once for each function
 */
void BPatch_function::createDataDependenceGraph() 
{
   Annotatable<BPatch_dependenceGraphNode *, data_dep_graph_a> &ddg = *this;
   Annotatable<ParameterType *, formal_param_set_a> &formal_params = *this;

  unsigned i,j;
  BPatch_flowGraph* cfg = getCFG();

#if defined(arch_x86)
  unsigned num_regs=3;
#else
  unsigned num_regs=8;
  int reg_offset = 0;
#endif
  int * writeArr = (int *) malloc(sizeof(int)*num_regs);
  int * readArr = (int *) malloc(sizeof(int)*num_regs);
  // ******************** Initializations and collecting required objects **************************
#if 0
  if(getAnnotation("DataDependenceGraph") != NULL)
    return;
#endif
  if (ddg.size())
     return;

#if defined(interprocedural)
  BPatch_Vector<ParameterType*>* parameters = new BPatch_Vector<ParameterType*>();
  setAnnotation(createAnnotationType("formalParams"), new Annotation(parameters));  

  BPatch_Vector<ParameterType*>* actualParams = new BPatch_Vector<ParameterType*>();
  BPatch_Vector<ReturnParameterType*>* returned = new BPatch_Vector<ReturnParameterType*>();

  /*
  BPatch_Vector<void*>* production = new BPatch_Vector<void*>();
  setAnnotation(createAnnotationType("production"), new Annotation(production));  
  */
#endif


#if 0
  BPatch_Vector<BPatch_dependenceGraphNode*>* dataDependenceGraph = new BPatch_Vector<BPatch_dependenceGraphNode*>();
#endif
  BPatch_Vector<entry *> blocks;// = *blocksPtr;

  BPatch_Set<BPatch_basicBlock*> all_b_b;
  cfg->getAllBasicBlocks(all_b_b); // getBasicBlocks(cfg, all_b_b);

  // an array to store whether a basic block is visited, parallel to the blocks
  int* visitedBlocks = initialize_array(new int[all_b_b.size()], all_b_b.size(), -1);

  /// ****************** start with entry blocks ************************************************
  pushEntryBlocks(blocks, cfg);
// ***************************** Iterate through all blocks ***********************************
#if defined(logging)
  FILE* logFile = fopen("log.txt","w");
#endif
  // iterate as long as there are blocks in the list
  for(i=0; i<blocks.size(); i++) {
    BPatch_basicBlock * blck = blocks[i]->basicBlock;
    int blockNum = blck->getBlockNumber(); // the block number of this basic block
    int blockLoc = visitedBlocks[blockNum]; // the location of this block in the blocks list
#if defined(logging)
    fprintf(logFile,"Block Num: %d\n",blockNum);
#endif
    //    printf("Block Num %d\n",blockNum);
    // in case a block is visited after being checked, double-check if it is visited
    // May not be enough when there is a loop in CFG, so also check before pushing into the list

    // if not visited, visit now
    // perhaps the register list is different, check
    if(blockLoc == -1 || needsVisiting(blocks[i]->registers, blocks[i]->memLocs, blocks[blockLoc]->registers, blocks[blockLoc]->memLocs)) {
      visitedBlocks[blockNum] = i; // update the block location

#if defined(logging)
    fprintf(logFile,"Block Num: %d Being visited\n",blockNum);
#endif
      // create an iterator
      InstrucIter* iter;// = new InstrucIter(blck);
      InstrucIter* prevIter;
      // create a copy of the register vector
      BPatch_Vector<regUpdate*>* copy_regs = new BPatch_Vector<regUpdate*>();
      copyRegisterVector(blocks[i]->registers, copy_regs);

      BPatch_Vector<memUpdate*>* copy_mems = new BPatch_Vector<memUpdate*>();
      copyMemLocVector(blocks[i]->memLocs, copy_mems);

#if defined(interprocedural)
      BPatch_Vector<BPatch_dependenceGraphNode*>* copy_calls = new BPatch_Vector<BPatch_dependenceGraphNode*>();
      copyFuncCallVector(blocks[i]->prevCalls, copy_calls);

      //      BPatch_Vector<BPatch_instruction *> * insts = blck->getInstructions();
#endif
      // prevIter initally does not point to previous instruction, but it is OK.
      for(iter = new InstrucIter(blck), prevIter = new InstrucIter(blck); iter->hasMore(); (*iter)++) {
         instruction ins = iter->getInstruction();
         BPatch_instruction* bpins = iter->getBPInstruction();
         // check whether a memory operation is performed
         BPatch_memoryAccess * memAcc = iter->isLoadOrStore();
         // create a node for this instruction in case we did not create yet
         BPatch_dependenceGraphNode * node = getNode(&ddg.getDataStructure(), bpins);
#if defined(arch_x86)
         // if this instruction is a conditional branch instruction, it depends on the previous compare instruction
         if(iter->isACondBranchInstruction()) {
            handleCondBranch(node, iter, &ddg.getDataStructure());
         } // end checking whether this is conditional instruction
#endif
         // initialize read/write arrays
         for (j = 0; j < num_regs; j++) {
            writeArr[j] = readArr[j] = -1;
         }
         iter->readWriteRegisters(readArr,writeArr);
#if defined(logging)
         // ins.print();
         fprintf(logFile,"Read Registers ");
         int rg;
         for(rg=0; rg<num_regs; rg++)
            fprintf(logFile,"%d ", readArr[rg]);
         fprintf(logFile,"Write Registers ");
         for(rg=0; rg<num_regs; rg++)
            fprintf(logFile,"%d ", writeArr[rg]);
         if(memAcc != BPatch_memoryAccess::none) {
            const BPatch_addrSpec_NP* spec = memAcc->getStartAddr(0);
            fprintf(logFile,"Mem: %d %d %d %d",spec->getReg(0),spec->getReg(1),spec->getScale(),spec->getImm());
         }
         fprintf(logFile,"\n");
#endif

#if defined(interprocedural)

#if defined(arch_x86)
	if(iter->isACallInstruction()) {
	  int k = 0;
	  for(; k<num_regs && writeArr[k] != -1; k++);
	  if(k<num_regs)
	    writeArr[k] = EAX;
	  // TODO: if k>= reg_num
	  // TODO: if void func!
	}

	// handle memory operations
	if(bpins->isALoad() && memAcc != BPatch_memoryAccess::none) {
	  // Iterate list for the memory locations that are read at this line
	  for(j=0; j<BPatch_instruction::nmaxacc_NP; j++) {
	    const BPatch_addrSpec_NP* addrSpec = memAcc->getStartAddr(j);
	    if((addrSpec->getReg(0) == EBP || addrSpec->getReg(1) == EBP) && addrSpec->getImm() >=0 ) {
	      bool found = false;
	      unsigned k;
	      for(k=0; k<copy_mems->size(); k++) {
		// if this location is already written, this is not a parameter
		if(addrSpec->getReg(0) == (*(copy_mems))[k]->reg0 && addrSpec->getReg(1) == (*(copy_mems))[k]->reg1 &&
		   addrSpec->getScale() == (*(copy_mems))[k]->scale && addrSpec->getImm() == (*(copy_mems))[k]->immediate ) {
		  found = true;
		}
	      }
	      if(! found) {
		memUpdate* cell = (memUpdate*)malloc(sizeof(memUpdate));
		cell->reg0 = addrSpec->getReg(0);
		cell->reg1 = addrSpec->getReg(1);
		cell->scale = addrSpec->getScale();
		cell->immediate = addrSpec->getImm();
		cell->node = node;
		// add this to the list...
		parameters->push_back(cell);
	      }
	    }
	  }
	}
#endif

	for(j=0; j<num_regs && readArr[j] != -1; j++) {
#if defined(sparc_sun_solaris2_4)
	  //  if(readArr[j] < 16 && readArr[j] >= 8)
	  if(readArr[j] >= 24 && readArr[j] < 30) {

	      // if not overwritten by this function before
	      if(! isWrittenBefore(readArr[j], copy_regs)) {
		ParameterType* param = (ParameterType*)malloc(sizeof(ParameterType));
		param->reg = readArr[j];
		param->node = node;
		parameters->push_back(param);
	      }
	  }
#endif
	  if(isReturnedValue(readArr[j])) { //readArr[j] >= 8 && readArr[j] < 14) {
#if defined(arch_x86)
	    bool found = false;
	    int k;
	    for(k=0; k< copy_regs->size(); k++) {
	      regUpdate* r = (*copy_regs)[k];
	      if(r->reg == EAX && r->node->getBPInstruction()->insn()->isCall())
		found = true;
	    }
	    if(! found)
	      continue;
#endif
	    //	      if(! isWrittenBefore(readArr[j], copy_regs)) {
	    ReturnParameterType* rt = (ReturnParameterType*)malloc(sizeof(ReturnParameterType));
	    rt->reg = readArr[j];
	    rt->node = node;
	    returned->push_back(rt);
	    //	      }
	  }
	}

	bool retInst = iter->isAReturnInstruction();
	bool callInst = iter->isACallInstruction();
	if(retInst || callInst){
	  unsigned count;
	  for(count=0; count<copy_calls->size(); count++) {
	    // for the return value of the previous call instruction
	    annotation* ann = (annotation*)malloc(sizeof(annotation));
	    ann->v.returnVals = returned;
	    ann->address = (*copy_calls)[count]->getBPInstruction()->getAddress();
	    setAnnotation(createAnnotationType("returned"), new Annotation(ann));
	    returned = new BPatch_Vector<ReturnParameterType*>();
	  }
	  if(callInst) {
	    unsigned mynum;
#if defined(sparc_sun_solaris2_4)
	    //	    fprintf(stderr,"marking regs among %d regs that are written:",copy_regs->size());
	    for(mynum=0; mynum < copy_regs->size(); mynum++) {
	      int r = (*copy_regs)[mynum]->reg - WIN_SIZE*(MAX_SETS - reg_offset);
	      // fprintf(stderr,"%d-%d ",r,(*copy_regs)[mynum]->reg);
	      if(r >= 8 && r < 14) {
		ParameterType* pt = (ParameterType*)malloc(sizeof(ParameterType));
		pt->reg = r;
		pt->node = (*copy_regs)[mynum]->node;
		actualParams->push_back(pt);
	      }
	    }
#else
	    for(mynum=0; mynum < copy_mems->size(); mynum++) {
	      memUpdate* m = (*copy_mems)[mynum];
	      // fprintf(stderr,"%d-%d ",r,(*copy_regs)[mynum]->reg);
	      if(m->immediate >= 0 && (m->reg0 == ESP || m->reg1 == ESP)) {
		/*
		  ParameterType* pt = (ParameterType*)malloc(sizeof(ParameterType));
		pt->reg = r;
		pt->node = (*copy_regs)[mynum]->node;
		*/
		actualParams->push_back(m);
	      }
	    }
#endif
	    annotation* an = (annotation*)malloc(sizeof(annotation));
	    
	    void* addr = bpins->getAddress();
	    an->address = addr;

	    // TODO
	    // production->push_back(addr);
	    
	    an->v.parameters = actualParams;
	    setAnnotation(createAnnotationType("actualParams"), new Annotation(an));

	    actualParams = new BPatch_Vector<ParameterType*>();

	    copy_calls->clear();
	    copy_calls->push_back(node);
	  }
	  else if(retInst) {
	    BPatch_Vector<ReturnParameterType*>* returnVals = new BPatch_Vector<ReturnParameterType*>();
	    unsigned mynum;
	    //fprintf(stderr,"Return: marking regs among %d regs that are written:",copy_regs->size());
	    for(mynum=0; mynum < copy_regs->size(); mynum++) {
#if defined(sparc_sun_solaris2_4)
	      int r = (*copy_regs)[mynum]->reg - WIN_SIZE*(MAX_SETS - reg_offset);
#elif defined(arch_x86)
	      int r = (*copy_regs)[mynum]->reg;
#endif	      //fprintf(stderr,"%d-%d ",r,(*copy_regs)[mynum]->reg);
	      if(isReturnValue(r)) {
		ReturnParameterType* rt = (ReturnParameterType*)malloc(sizeof(ReturnParameterType));
		rt->reg = r;
		rt->node = (*copy_regs)[mynum]->node;
		returnVals->push_back(rt);
	      }
	    }
	    //fprintf(stderr," Marked %d\n",returnVals->size());
	    setAnnotation(createAnnotationType("returnVals"), new Annotation(returnVals));
	  }
	}
#endif
#if defined(arch_x86)
#else
	iter->adjustRegNumbers(readArr,writeArr,reg_offset);
	if(iter->isASaveInstruction())
	  reg_offset++;
	else if(iter->isARestoreInstruction())
	  reg_offset--;
#endif

	// printf("Read\t");
	// Iterate list for the registers that are read at this line
	for(j=0; j<num_regs && readArr[j] != -1; j++) { // are we sure that the registers are gonna fill the start of array
	  // printf("Reg %d\t",readArr[j]);
	  register_check(readArr[j],copy_regs,node);
	}
	// printf("\n\n");
	
	// handle memory operations
	if(memAcc != BPatch_memoryAccess::none) {
	  // Iterate list for the memory locations that are read at this line
	  for(j=0; j<BPatch_instruction::nmaxacc_NP; j++) {
	    const BPatch_addrSpec_NP* addrSpec = memAcc->getStartAddr(j);
#if defined(arch_x86)
	    handleMemoryAccess(node, bpins, copy_regs, copy_mems, addrSpec);
#else
	    handleMemoryAccess(node, bpins, copy_regs, copy_mems, addrSpec, iter, reg_offset);
#endif
	  }
	}
	delete memAcc;
	//	printf("Write\t");
	// Reading the registers that are written at this instruction
	for(j=0; j<num_regs && writeArr[j] != -1; j++) { // are we sure that the registers are gonna fill the start of array
	  // printf("Reg %d\t",writeArr[j]);
	  handleWrittenRegister(node, writeArr[j], copy_regs);
	}
	//	printf("\n\n");
	// Finally, update prevIter...
	prevIter = iter;


      } // end of loop that scans instructions in this block

      delete iter;
      addBlocksToWorkload(blocks, blck, visitedBlocks, copy_regs, copy_mems
#if defined(interprocedural)
			  , copy_calls
#endif
			  );
    }
  }

  free(writeArr);
  free(readArr);
  delete visitedBlocks;
  //  delete blocksPtr;

#ifdef print_graphs
  printf("\nAll\n");
  outputGraph(dataDependenceGraph);
#endif

#if defined(logging)
  fclose(logFile);
#endif
  
#if 0
  setAnnotation(getAnnotationType("DataDependenceGraph"), new Annotation(dataDependenceGraph));  
#endif
}
#else
BPatch_dependenceGraphNode* BPatch_function::getSliceInt(BPatch_instruction*)
{
   return NULL;
}

BPatch_dependenceGraphNode* BPatch_function::getProgramDependenceGraphInt(BPatch_instruction*)
{
   return NULL;
}

BPatch_dependenceGraphNode* BPatch_function::getControlDependenceGraphInt(BPatch_instruction*)
{
   return NULL;
}

BPatch_dependenceGraphNode* BPatch_function::getDataDependenceGraphInt(BPatch_instruction *)
{
   return NULL;
}
#endif
