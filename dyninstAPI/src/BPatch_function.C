/*
 * Copyright (c) 1996 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
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

// $Id: BPatch_function.C,v 1.20 2001/12/14 17:57:00 gaburici Exp $

#define BPATCH_FILE

#include <string.h>
#include "symtab.h"
#include "process.h"
#include "instPoint.h"

#include "BPatch.h"
#include "BPatch_type.h"
#include "BPatch_collections.h"
#include "BPatch_Vector.h"
#include "BPatch_flowGraph.h"
#include "BPatch_memoryAccess_NP.h"

#include "LineInformation.h"
#include "common/h/Types.h"
#include "AddressHandle.h"

/* XXX Should be in a dyninst API include file (right now in perfStream.h) */


/**************************************************************************
 * BPatch_function
 *************************************************************************/
/*
 * BPatch_function::BPatch_function
 *
 * Constructor that creates a BPatch_function.
 *
 */
BPatch_function::BPatch_function(process *_proc, function_base *_func,
	BPatch_module *_mod) :
	proc(_proc), mod(_mod), cfg(NULL), func(_func)
{
  // there should be at most one BPatch_func for each function_base per process
  assert(proc->thread && !proc->PDFuncToBPFuncMap[func]);

  _srcType = BPatch_sourceFunction;

  localVariables = new BPatch_localVarCollection;
  funcParameters = new BPatch_localVarCollection;
  retType = NULL;

  proc->PDFuncToBPFuncMap[_func] = this;
};

/*
 * BPatch_function::BPatch_function
 *
 * Constructor that creates the BPatch_function with return type.
 *
 */
BPatch_function::BPatch_function(process *_proc, function_base *_func,
				 BPatch_type * _retType, BPatch_module *_mod) :
	proc(_proc), mod(_mod), cfg(NULL), func(_func)
{
  assert(!proc->PDFuncToBPFuncMap[_func]);

  _srcType = BPatch_sourceFunction;
  localVariables = new BPatch_localVarCollection;
  funcParameters = new BPatch_localVarCollection;
  retType = _retType;

  proc->PDFuncToBPFuncMap[_func] = this;
};


BPatch_function::~BPatch_function()
{
    // if (ast != NULL)
        // removeAst(ast);
    if (localVariables) delete localVariables;
    if (funcParameters) delete funcParameters;
    if (cfg) delete cfg;
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
char *BPatch_function::getName(char *s, int len)
{
    assert(func);
    string name = func->prettyName();
    strncpy(s, name.string_of(), len);

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
    string name = func->symTabName();
    strncpy(s, name.string_of(), len);

    return s;
}

/*
 * BPatch_function::getBaseAddr
 *
 * Returns the starting address of the function.
 */
void *BPatch_function::getBaseAddr()
{
     return (void *)func->getEffectiveAddress(proc);
}

/*
* BPatch_function::getBaseAddrRelative
*
* Returns the starting address of the function in the module, relative
* to the module.
*/
void *BPatch_function::getBaseAddrRelative()
{
	return (void *)func->addr();
}


/*
 * BPatch_function::getSize
 *
 * Returns the size of the function in bytes.
 */
unsigned int BPatch_function::getSize()
{
  return func->size();
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

    // if the function is not instrumentable, we won't find the point
    if (!isInstrumentable())
	return NULL;

    // function is generally uninstrumentable (with current technology)
    if (func->funcEntry(proc) == NULL) return NULL;

    BPatch_Vector<BPatch_point*> *result = new BPatch_Vector<BPatch_point *>;

    if (loc == BPatch_entry || loc == BPatch_allLocations) {
        result->push_back(proc->findOrCreateBPPoint(
	    this,const_cast<instPoint *>(func->funcEntry(proc)),BPatch_entry));
    }
    switch (loc) {
      case BPatch_entry: // already done
          break;
      case BPatch_allLocations:
        {
          const vector<instPoint *> &Rpoints = func->funcExits(proc);
          const vector<instPoint *> &Cpoints = func->funcCalls(proc);
          unsigned int c=0, r=0;
          Address cAddr, rAddr;
          while (c < Cpoints.size() || r < Rpoints.size()) {
              if (c < Cpoints.size()) cAddr = Cpoints[c]->iPgetAddress();
              else                    cAddr = (Address)(-1);
              if (r < Rpoints.size()) rAddr = Rpoints[r]->iPgetAddress();
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
          const vector<instPoint *> &points = func->funcExits(proc);
          for (unsigned i = 0; i < points.size(); i++) {
	      result->push_back(proc->findOrCreateBPPoint(
		  this, points[i], BPatch_exit));
          }
          break;
        }
      case BPatch_subroutine:
        {
          const vector<instPoint *> &points = func->funcCalls(proc);
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

// VG(09/17/01): created this 'cause didn't want to add more 
// platform independent code to inst-XXX.C
BPatch_point* createInstPointForMemAccess(process *proc,
					  void *addr,
					  BPatch_memoryAccess* ma,
					  BPatch_point** alternative = NULL)
{
  // VG(09/17/01): This seems the right fuction to update all data structures
  // Trouble is that updating these should be platfrom independent, while this
  // function also does platform dependent stuff...
  BPatch_point *p = createInstructionInstPoint(proc, (void*) addr, alternative);
  if(p)
    p->memacc = ma;

  return p;
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
BPatch_Vector<BPatch_point*> *BPatch_function::findPoint(
        const BPatch_Set<BPatch_opCode>& ops)
{
  // function does not exist!
  if (func == NULL) return NULL;

  // function is generally uninstrumentable (with current technology)
  if (func->funcEntry(proc) == NULL) return NULL;
  
  BPatch_Vector<BPatch_point*> *result = new BPatch_Vector<BPatch_point *>;

  int osize = ops.size();
  BPatch_opCode* opa = new BPatch_opCode[osize];
  ops.elements(opa);

  bool findLoads = false, findStores = false, findPrefetch = false;

  for(int i=0; i<osize; ++i) {
    switch(opa[i]) {
    case BPatch_opLoad: findLoads = true; break;
    case BPatch_opStore: findStores = true; break;	
    case BPatch_opPrefetch: findPrefetch = true; break;	
    }
  }

  Address relativeAddress = (Address)getBaseAddrRelative();
  
  // Use an address handle as an iterator through instructions
  AddressHandle ah(proc, mod->mod->exec(),
		   relativeAddress, getSize());
  
  instruction inst;
  //int xx = -1;

  while(ah.hasMore()) {

    inst = ah.getInstruction();
    Address addr = *ah;     // XXX this gives the address *stored* by ah...

    //fprintf(stderr, "?????: %x\n", addr);
    ah++;

    BPatch_memoryAccess ma = isLoadOrStore(inst);

    //BPatch_addrSpec_NP start = ma.getStartAddr();
    //BPatch_countSpec_NP count = ma.getByteCount();
    //int imm = start.getImm();
    //int ra  = start.getReg(0);
    //int rb  = start.getReg(1);
    //int cnt = count.getImm();
    //short int fcn = ma.prefetchType();
    bool skip = false;

    if(findLoads && ma.isALoad()) {
      //fprintf(stderr, "LD[%d]: [%x -> %x], %d(%d)(%d) #%d\n",
      //      ++xx, addr, inst, imm, ra, rb, cnt);
      // XXX this leaks...
      BPatch_point* p = createInstPointForMemAccess(proc, (void*) addr,
						    new BPatch_memoryAccess(ma));
      if(p)
        result->push_back(p);
      skip = true;
    }

    if(findStores && !skip && ma.isAStore()) {
      //fprintf(stderr, "ST[%d]: [%x -> %x], %d(%d)(%d) #%d\n",
      //      ++xx, addr, inst, imm, ra, rb, cnt);
      // XXX this leaks...
      BPatch_point* p = createInstPointForMemAccess(proc, (void*) addr,
						    new BPatch_memoryAccess(ma));
      if(p)
        result->push_back(p);
      skip = true;
    }

    if(findPrefetch && !skip && ma.isAPrefetch()) {
      //fprintf(stderr, "PF[%d]: [%x -> %x], %d(%d)(%d) #%d %%%d\n",
      //      ++xx, addr, inst, imm, ra, rb, cnt, fcn);
      // XXX this leaks...
      BPatch_point* p = createInstPointForMemAccess(proc, (void*) addr,
						    new BPatch_memoryAccess(ma));
      if(p)
        result->push_back(p);
      skip = true;
    }
  }

  return result;
}

/*
 * BPatch_function::addParam()
 *
 * This function adds a function parameter to the BPatch_function parameter
 * vector.
 */
void BPatch_function::addParam(char * _name, BPatch_type *_type, int _linenum,
			       int _frameOffset, int _sc)
{
  BPatch_localVar * param = new BPatch_localVar(_name, _type, _linenum,
						_frameOffset, _sc);

  // Add parameter to list of parameters
  params.push_back(param);
}

/*
 * BPatch_function::findLocalVar()
 *
 * This function searchs for a local variable in the BPatch_function's
 * local variable collection.
 */
BPatch_localVar * BPatch_function::findLocalVar(const char * name)
{

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

  BPatch_localVar * var = funcParameters->findLocalVar(name);
  return (var);

}

/** method to retrieve addresses for a given line in the function
  * if the line number is not valid, or if the line info is not available
  * or if the module does not contain entry for the function then it returns
  * false. If exact match is not set then the line which is the next
  * greater or equal will be used.
  */
bool BPatch_function::getLineToAddr(unsigned short lineNo,
                   BPatch_Vector<unsigned long>& buffer,
                   bool exactMatch)
{

	//get the line info object and check whether it is available
	LineInformation* lineInformation = mod->lineInformation;
	if(!lineInformation){
		return false;
	}

	//get the object which contains the function being asked
	FileLineInformation* fLineInformation = 
			lineInformation->getFunctionLineInformation(func->symTabName());
	if(!fLineInformation){
		return false;
	}

	//retrieve the addresses
	BPatch_Set<Address> addresses;
	if(!fLineInformation->getAddrFromLine(func->symTabName(),addresses,
					      lineNo,false,exactMatch))
		return false;

	//then insert the elements to the vector given
	Address* elements = new Address[addresses.size()];
	addresses.elements(elements);
	for(int i=0;i<addresses.size();i++)
		buffer.push_back(elements[i]);
	delete[] elements;
	
	return true;
}

//
// Return the module name, first and last line numbers of the function.
//
bool BPatch_function::getLineAndFile(unsigned int &start,
				     unsigned int &end,
                                     char *fileName, unsigned &length)
{
    start = end = 0;

    //get the line info object and check whether it is available
    LineInformation* lineInformation = mod->lineInformation;

    if (!lineInformation) {
        logLine("BPatch_function::getLineToAddr : Line info is not available");
	return false;
    }

    //get the object which contains the function being asked
    FileLineInformation* fLineInformation = 
	lineInformation->getFunctionLineInformation(func->symTabName());

    if (!fLineInformation) {
	logLine("BPatch_function::getLineToAddr: Line info is not available");
	return false;
    }

    //retrieve the addresses
    FunctionInfo *funcLineInfo;

    funcLineInfo = fLineInformation->findFunctionInfo(func->symTabName());

    if (!funcLineInfo) return false;

    if (funcLineInfo->startLinePtr)
	start = funcLineInfo->startLinePtr->lineNo;

    if (funcLineInfo->endLinePtr)
	end = funcLineInfo->endLinePtr->lineNo;

    strncpy(fileName, fLineInformation->getFileName().string_of(), length);
    if (strlen(fLineInformation->getFileName().string_of()) < length) {
	length = strlen(fLineInformation->getFileName().string_of());
    }

    return true;
}

BPatch_flowGraph* BPatch_function::getCFG(){
	if(cfg)
		return cfg;

	cfg = new BPatch_flowGraph((BPatch_function*)this);

	return cfg;
}


BPatch_Vector<BPatch_localVar *> *BPatch_function::getVars() {
      return localVariables->getAllVars(); 
}

BPatch_Vector<BPatch_variableExpr *> *BPatch_function::findVariable(const char *name)
{
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
	ret->push_back(new BPatch_variableExpr(proc, (void *) lv->getFrameOffset(), 
	    lv->getType(), lv->getFrameRelative(), (*points)[0]));
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

bool BPatch_function::getVariables(BPatch_Vector<BPatch_variableExpr *> &/*vect*/)
{
    	return false;
}

char *BPatch_function::getModuleName(char *name, int maxLen) {
    return getModule()->getName(name, maxLen);
}

#ifdef IBM_BPATCH_COMPAT

bool BPatch_function::getLineNumbers(unsigned int &start, unsigned int &end) {
    unsigned int length = 0;
    return getLineAndFile(start, end, NULL, length);
}

void *BPatch_function::getAddress() { return getBaseAddr(); }
    
bool BPatch_function::getAddressRange(void * &start, void * &end) {
	start = getBaseAddr();
	unsigned long temp = (unsigned long) start;
	end = (void *) (temp + getSize());

	return true;
}

BPatch_type *BPatch_function::returnType() { return retType; }

void BPatch_function::getIncPoints(BPatch_Vector<BPatch_point *> &vect) 
{
    BPatch_Vector<BPatch_point *> *v1 = findPoint(BPatch_allLocations);
    if (v1) {
	for (int i=0; i < v1->size(); i++) {
	    vect.push_back((*v1)[i]);
	}
    }
}

int	BPatch_function::getMangledNameLen() { return 1024; }

void BPatch_function::getExcPoints(BPatch_Vector<BPatch_point*> &points) {
    abort();
    return;
};

// return a function that can be passed as a paramter
BPatch_variableExpr *BPatch_function::getFunctionRef() { abort(); return NULL; }

#endif

/*
 * BPatch_function::isInstrumentable
 *
 * Returns true if the function is instrumentable, false otherwise.
 */
bool BPatch_function::isInstrumentable()
{
     return ((pd_Function *)func)->isInstrumentable();
}

