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

// $Id: BPatch_function.C,v 1.109 2008/11/03 15:19:24 jaw Exp $

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

#include "common/h/Types.h"
#if !defined(cap_instruction_api)
#include "InstrucIter-Function.h"
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

BPatch_function::BPatch_function(BPatch_addressSpace *_addSpace, 
                                 int_function *_func,
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
};

/*
 * BPatch_function::BPatch_function
 *
 * Constructor that creates the BPatch_function with return type.
 *
 */
BPatch_function::BPatch_function(BPatch_addressSpace *_addSpace, int_function *_func,
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
};


BPatch_function::~BPatch_function()
{
   if (localVariables) delete localVariables;
   if (funcParameters) delete funcParameters;
   
   if (cfg) delete cfg;
   
   // Remove us from the proc map...
   int num_erased = mod->func_map.erase(lowlevel_func());
   assert(num_erased == 1);
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

struct compareByEntryAddr
{
    bool operator()(const BPatch_basicBlock* lhs, const BPatch_basicBlock* rhs)
    {
        return lhs->getStartAddress() < rhs->getStartAddress();
    }
};
 
BPatch_Vector<BPatch_point*> *BPatch_function::findPointByOp(
        const BPatch_Set<BPatch_opCode>& ops)
{
  // function does not exist!
  if (func == NULL) return NULL;

    if (!mod->isValid()) return NULL;
#if defined(cap_instruction_api)
  // function is generally uninstrumentable (with current technology)
  if (func->funcEntries().size() == 0) return NULL;

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
      for(unsigned int i = 0; i < tmp->size(); i++)
      {
          ret->push_back((*tmp)[i]);
      } 
      
  }
  return ret;
#else  
  // Use an instruction iterator
  InstrucIterFunction ii(func);
    
  return BPatch_point::getPoints(ops, ii, this);
#endif
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

void BPatch_function::constructVarsAndParams()
{
    if (varsAndParamsValid)
       return;

    if (mod) 
	{
        mod->parseTypesIfNecessary();
    }

    //Check flag to see if vars & params are already constructed
    std::vector<localVar *>vars;

    if (lowlevel_func()->ifunc()->getSymtabFunction()->getLocalVariables(vars)) 
	{
        for (unsigned i = 0; i< vars.size(); i++) 
	    {
            if (mod) 
			{
                vars[i]->fixupUnknown(mod->lowlevel_mod()->pmod()->mod());
            }

	        localVariables->addLocalVar(new BPatch_localVar(vars[i]));
	    }    
    }

    std::vector<localVar *>parameters;

    if (lowlevel_func()->ifunc()->getSymtabFunction()->getParams(parameters)) 
	{
        for (unsigned i = 0; i< parameters.size(); i++) 
		{
            if (mod) 
			{
                parameters[i]->fixupUnknown(mod->lowlevel_mod()->pmod()->mod());
            }

	        BPatch_localVar *lparam = new BPatch_localVar(parameters[i]);
    	    funcParameters->addLocalVar(lparam);
	        params.push_back(lparam);
    	}    
    }

    if (!lowlevel_func()->ifunc()->getSymtabFunction()->getReturnType()) 
	{
        varsAndParamsValid = true;
        return;
    }
        
	SymtabAPI::Type *ret_type = lowlevel_func()->ifunc()->getSymtabFunction()->getReturnType();
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

BPatch_Vector<BPatch_localVar *> *BPatch_function::getVarsInt() 
{
    if (!mod->isValid()) return NULL;
    constructVarsAndParams();
    return localVariables->getAllVars(); 
}

bool BPatch_function::findVariableInt(const char *name, 
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


BPatch_Vector<BPatch_variableExpr *> *BPatch_function::findVariableInt(const char *name)
{
   BPatch_Vector<BPatch_variableExpr *> *vars = new BPatch_Vector<BPatch_variableExpr*>;
   bool result = findVariableInt(name, *vars);
   if (!result) {
      delete vars;
      return NULL;
   }
   return vars;
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
  
  
  AstNodePtr wrapper(AstNode::operandNode(AstNode::Constant, (void *) remoteAddress));
  // variableExpr owns the AST
  return new BPatch_variableExpr(fname, proc, lladdSpace, wrapper, 
                                 type, (void *) remoteAddress); 
  //return (BPatch_function::voidVoidFunctionPointer)remoteAddress;
  
#else
  //  For other platforms, the baseAddr of the function should be sufficient.
  
  //  In truth it would make more sense for this to be a BPatch_constExpr,
  //  But since we are adding this as part of the DPCL compatibility process
  //  we use the IBM API, to eliminate one API difference.
  
  AstNodePtr ast(AstNode::operandNode(AstNode::Constant, (void *) remoteAddress));
  
  // the variableExpr owns the ast now.
  return new BPatch_variableExpr(fname, addSpace, lladdSpace, ast, 
                                 type, (void *) remoteAddress);
  
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

