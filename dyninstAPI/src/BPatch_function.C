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

// $Id: BPatch_function.C,v 1.8 2000/10/17 17:42:14 schendel Exp $

#include <string.h>
#include "symtab.h"
#include "process.h"
#include "instPoint.h"

#include "BPatch.h"
#include "BPatch_type.h"
#include "BPatch_collections.h"
#include "BPatch_Vector.h"
#include "BPatch_flowGraph.h"
#include "LineInformation.h"

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
  _srcType = BPatch_sourceFunction;
  localVariables = new BPatch_localVarCollection;
  funcParameters = new BPatch_localVarCollection;
  retType = _retType;
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
BPatch_Vector<BPatch_sourceObj *> *BPatch_function::getSourceObj()
{
    return NULL;
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
#ifdef DEBUG_LINE
		cerr << "BPatch_function::getLineToAddr : ";
		cerr << "Line information is not available\n";
#endif
		return false;
	}

	//get the object which contains the function being asked
	FileLineInformation* fLineInformation = 
			lineInformation->getFunctionLineInformation(func->prettyName());
	if(!fLineInformation){
#ifdef DEBUG_LINE
		cerr << "BPatch_function::getLineToAddr : ";
		cerr << func->prettyName() << " is not found in its module\n";
#endif
		return false;
	}

	//retrieve the addresses
	BPatch_Set<Address> addresses;
	if(!fLineInformation->getAddrFromLine(func->prettyName(),addresses,
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
bool BPatch_function::getLineAndFile(unsigned short &start,
				     unsigned short &end,
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
	lineInformation->getFunctionLineInformation(func->prettyName());

    if (!fLineInformation) {
	logLine("BPatch_function::getLineToAddr: Line info is not available");
	return false;
    }

    //retrieve the addresses
    FunctionInfo *funcLineInfo;

    funcLineInfo = fLineInformation->findFunctionInfo(func->prettyName());

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

#if defined(sparc_sun_solaris2_4) || defined(mips_sgi_irix6_4)
	cfg = new BPatch_flowGraph((BPatch_function*)this);
#else
	cerr << "WARNING : BPatch_function::getCFG is not implemented";
	cerr << " for this platform\n";
#endif

	return cfg;
}


BPatch_Vector<BPatch_localVar *> *BPatch_function::getVars() {
      return localVariables->getAllVars(); 
}
