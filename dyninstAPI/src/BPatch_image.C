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

// $Id: BPatch_image.C,v 1.14 1999/08/17 21:50:05 hollings Exp $

#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "process.h"
#include "symtab.h"
#include "instPoint.h"

#include "BPatch.h"
#include "BPatch_image.h"
#include "BPatch_type.h"
#include "BPatch_collections.h"


/*
 * BPatch_image::BPatch_image
 *
 * Construct a BPatch_image for the given process.
 */

BPatch_image::BPatch_image(process *_proc) : proc(_proc)
{
    modlist = NULL;
}


/*
 * BPatch_image::getProcedures
 *
 * Returns a list of all procedures in the image upon success, and NULL
 * upon failure.
 */
BPatch_Vector<BPatch_function *> *BPatch_image::getProcedures()
{
    BPatch_Vector<BPatch_function *> *proclist =
	new BPatch_Vector<BPatch_function *>;

    if (proclist == NULL) return NULL;

    // XXX Also, what should we do about getting rid of this?  Should
    //     the BPatch_functions already be made and kept around as long
    //     as the process is, so the user doesn't have to delete them?
    BPatch_Vector<BPatch_module *> *mods = getModules();

    for (unsigned int i = 0; i < mods->size(); i++) {
	BPatch_Vector<BPatch_function *> *funcs = (*mods)[i]->getProcedures();
	for (unsigned int j=0; j < funcs->size(); j++) {
	    proclist->push_back((*funcs)[j]);
	}
    }

    return proclist;
}


/*
 * BPatch_image::getProcedures
 *
 * Returns a list of all procedures in the image upon success, and NULL
 * upon failure.
 */
BPatch_Vector<BPatch_variableExpr *> *BPatch_image::getGlobalVariables()
{
    BPatch_variableExpr *var;
    BPatch_Vector<BPatch_variableExpr *> *varlist =
	new BPatch_Vector<BPatch_variableExpr *>;

    if (varlist == NULL) return NULL;

    // XXX - should this stuff really be by image ??? jkh 3/19/99
    BPatch_Vector<BPatch_module *> *mods = getModules();
    BPatch_type *type;
    for (int m = 0; m < mods->size(); m++) {
	BPatch_module *module = (*mods)[m];
	char name[255];
	module->getName(name, sizeof(name));
	vector<string> keys = module->moduleTypes->globalVarsByName.keys();
	int limit = keys.size();
	for (int j = 0; j < limit; j++) {
	    Symbol syminfo;
	    Address baseAddr;
	    string name = keys[j];
	    type = module->moduleTypes->globalVarsByName[name];
	    assert(type);
	    if (!proc->getSymbolInfo(name, syminfo, baseAddr)) {
		printf("unable to find variable %s\n", name.string_of());
	    }
	    var = new BPatch_variableExpr((char *) name.string_of(), proc, 
		(void *)syminfo.addr(), (const BPatch_type *) type);
	    varlist->push_back(var);
	}
    }


    return varlist;
}

/*
 * BPatch_image::getModules
 *
 * Returns a list of all procedures in the image upon success, and NULL
 * upon failure.
 */
BPatch_Vector<BPatch_module *> *BPatch_image::getModules()
{
  if (modlist) {
    return modlist;
  }
  
  modlist = new BPatch_Vector<BPatch_module *>;
  if (modlist == NULL) return NULL;
  
  // XXX Also, what should we do about getting rid of this?  Should
  //     the BPatch_modules already be made and kept around as long
  //     as the process is, so the user doesn't have to delete them?
  vector<module *> *mods = proc->getAllModules();
  
  for (unsigned int m = 0; m < mods->size(); m++) {
    pdmodule *curr = (pdmodule *) (*mods)[m];
    BPatch_module *bpmod = new BPatch_module(proc, curr);
    modlist->push_back(bpmod);
  }
  
  // BPatch_procedures are only built on demand, and we need to make sure
  //    they get built.
  BPatch_Vector<BPatch_function *> *procedures = getProcedures();

  return modlist;
}

/*
 * BPatch_image::findProcedurePoint
 *
 * Returns a vector of the instrumentation points from a procedure that is
 * identified by the parameters, or returns NULL upon failure.
 *
 * name		The name of the procedure in which to look up the points.
 * loc		The points within the procedure to return.  The following
 *		values are valid for this parameter:
 * 		  BPatch_entry         The function's entry point.
 * 		  BPatch_exit          The function's exit point(s).
 * 		  BPatch_subroutine    The points at which the procedure calls
 * 		                       other procedures.
 * 		  BPatch_longJump      The points at which the procedure make
 * 		                       long jump calls.
 *		  BPatch_allLocations  All of the points described above.
 */
BPatch_Vector<BPatch_point*> *BPatch_image::findProcedurePoint(
	const char *name, const BPatch_procedureLocation loc)
{
    /* XXX Right now this assumes that there's only one function with
     * the given name.
     */

    BPatch_function *func = findBPFunction(name);
    
    if (func == NULL) return NULL;

    return func->findPoint(loc);
}


/*
 * BPatch_image::createInstPointAtAddr
 *
 * Returns a pointer to a BPatch_point object representing an
 * instrumentation point at the given address.
 *
 * Returns the pointer to the BPatch_point on success, or NULL upon
 * failure.
 *
 * address	The address that the instrumenation point should refer to.
 */
BPatch_point *BPatch_image::createInstPointAtAddr(void *address)
{
    unsigned i;

    /* First look in the list of non-standard instPoints. */
    if (proc->instPointMap.defines((Address)address)) {
	instPoint *ip = proc->instPointMap[(Address)address];
	return new BPatch_point(proc, NULL, ip, BPatch_allLocations);
    }

    /* Look in the regular instPoints of the enclosing function. */
    function_base *func = proc->findFunctionIn((Address)address);

    if (func != NULL) {
	instPoint *entry = const_cast<instPoint *>(func->funcEntry(proc));
	assert(entry);
	if (entry->iPgetAddress() == (Address)address) {
	    return new BPatch_point(proc, NULL, entry, BPatch_entry);
	}

	const vector<instPoint*> &exits = func->funcExits(proc);
	for (i = 0; i < exits.size(); i++) {
	    assert(exits[i]);
	    if (exits[i]->iPgetAddress() == (Address)address) {
		return new BPatch_point(proc, NULL, exits[i], BPatch_exit);
	    }
	}

	const vector<instPoint*> &calls = func->funcCalls(proc);
	for (i = 0; i < calls.size(); i++) {
	    assert(calls[i]);
	    if (calls[i]->iPgetAddress() == (Address)address) {
		return new BPatch_point(proc, NULL, calls[i], BPatch_subroutine);
	    }
	}
    }

    /* We don't have an instPoint for this address, so make one. */
#if defined(rs6000_ibm_aix4_1) || defined(alpha_dec_osf4_0)
    /*
     * XXX This is machine dependent and should be moved to somewhere else.
     */
    if (!isAligned((Address)address))
	return NULL;

    instruction instr;
    proc->readTextSpace(address, sizeof(instruction), &instr.raw);

#if defined(rs6000_ibm_aix4_1)
    instPoint *newpt = new instPoint((pd_Function *)func,
				     instr,
				     NULL, // image *owner - this is ignored
				     (Address)address,
				     false, // bool delayOk - this is ignored
				     ipOther);
#elif defined(alpha_dec_osf4_0)
    instPoint *newpt = new instPoint((pd_Function *)func,
				    (const instructUnion &) instr,
				    (const image *) NULL, // image * - ignored
				    (Address &)address,
				    false, // bool delayOk - ignored
				    otherPoint);
#endif


    proc->instPointMap[(Address)address] = newpt; // Save this instPoint

    return new BPatch_point(proc, NULL, newpt, BPatch_address);
#else
    /* Not implemented on this platform (yet). */
    assert(false);
    return NULL;
#endif
}


/*
 * BPatch_image::findFunction
 *
 * Returns a NEW BPatch_function* representing the named function upon success,
 * and NULL upon failure.
 *
 * name		The name of function to look up.
 */
BPatch_function *BPatch_image::findFunction(const char *name)
{
    extern dictionary_hash <function_base*, BPatch_function*> PDFuncToBPFunc;

    function_base *func = proc->findOneFunction(name);

    if (func == NULL) {
	string fullname = string("_") + string(name);
	func = proc->findOneFunction(fullname);
    }

    if (func == NULL) {
	string msg = string("Unable to find function: ") + string(name);
	showErrorCallback(100, msg);
	return NULL;
    }

    BPatch_function *bpfunc = PDFuncToBPFunc[func];
    if (!bpfunc) {
	bpfunc = new BPatch_function(proc, func, NULL);
	printf("created BPatch_function wo module\n");
    }
	
    return bpfunc;
}


/*
 * BPatch_image::findVariable
 *
 * Returns a BPatch_variableExpr* representing the given variable in the
 * application image.  If no such variable exists, returns NULL.
 *
 * name		The name of the variable to look up.
 *
 * First look for the name with an `_' prepended to it, and if that is not
 *   found try the original name.
 */
BPatch_variableExpr *BPatch_image::findVariable(const char *name)
{
    string full_name = string("_") + string(name);

    Symbol syminfo;
    Address baseAddr;
    if (!proc->getSymbolInfo(full_name, syminfo, baseAddr)) {
	string short_name(name);
	if (!proc->getSymbolInfo(short_name, syminfo, baseAddr)) {
	    string msg = string("Unable to find variable: ") + string(name);
	    showErrorCallback(100, msg);
	    return NULL;
	}
    }
    if( syminfo.type() == Symbol::PDST_FUNCTION)
      return NULL;
    
    // XXX - should this stuff really be by image ??? jkh 3/19/99
    BPatch_Vector<BPatch_module *> *mods = getModules();
    BPatch_type *type;
    for (int m = 0; m < mods->size(); m++) {
	BPatch_module *module = (*mods)[m];
	//printf("The moduleType address is : %x\n", &(module->moduleTypes));
	type = module->moduleTypes->findVariableType(name);
	if (type) break;
    }
    if (!type) {
        type = BPatch::bpatch->type_Untyped;
    }

    return new BPatch_variableExpr((char *) name, proc, (void *)syminfo.addr(), 
	 (const BPatch_type *) type);
}

//
// findVariable
//	scp	- a BPatch_point that defines the scope of the current search
//	name	- name of the variable to find.
//
BPatch_variableExpr *BPatch_image::findVariable(BPatch_point &scp,
						const char *name)
{
    // Get the function to search for it's local variables.
    // XXX - should really use more detailed scoping info here - jkh 6/30/99
    BPatch_function *func = (BPatch_function *) scp.getFunction();
    if (!func) {
	string msg = string("point passed to findVariable lacks a function\n address point type passed?");
	showErrorCallback(100, msg);
	return NULL;
    }

    BPatch_localVar *lv = func->findLocalVar(name);

    if (!lv) {
	// look for it in the parameter scope now
	lv = func->findLocalParam(name);
    }
    if (lv) {
	// create a local expr with the correct frame offset 
	return new BPatch_variableExpr(proc, (void *) lv->getFrameOffset(), 
	    lv->getType(), true, &scp);
    }

    // finally check the global scope.
    return findVariable(name);
}

/*
 * BPatch_image::findType
 *
 * Returns a BPatch_type* representing the named type.  If no such type
 * exists, returns NULL.
 *
 * name		The name of type to look up.
 */
BPatch_type *BPatch_image::findType(const char *name)
{
    BPatch_type *type;

    assert(BPatch::bpatch != NULL);

    // XXX - should this stuff really be by image ??? jkh 3/19/99
    BPatch_Vector<BPatch_module *> *mods = getModules();
    for (int m = 0; m < mods->size(); m++) {
	BPatch_module *module = (*mods)[m];
	type = module->moduleTypes->findType(name);
	if (type) return type;
    }

    // check the default base types of last resort
    return BPatch::bpatch->stdTypes->findType(name);
}

/*
 * BPatch_image::findBPFunnction
 *
 * Returns a BPatch_function* representing the named function or if no func
 * exists, returns NULL.
 *
 * name		The name of function to look up.
 */
BPatch_function  *BPatch_image::findBPFunction(const char *name)
{
    BPatch_function *func;
    BPatch_Vector<BPatch_function *> * funclist =
      new BPatch_Vector<BPatch_function *>;
      
    assert(BPatch::bpatch != NULL);

    // XXX - should this stuff really be by image ??? jkh 3/19/99
    BPatch_Vector<BPatch_module *> *mods = getModules();
    //printf(" Number of Modules %d\n",mods->size());
    for (int m = 0; m < mods->size(); m++) {
	BPatch_module *module = (*mods)[m];
	func = module->findFunction(name);
	if (func){
	  funclist->push_back(func);
	 }
    }
    if( funclist->size()){
      //printf("Function list has %d functions\n", funclist->size());
      if( funclist->size() == 2)
	return (*funclist)[1];
      else 
	return (*funclist)[0];
    }
    // check the default base types of last resort
    else
      return NULL;
}


/*
 * BPatch_image::addModule
 *
 * Adds a new module to the BPatch_module vector
 * if modlist exists. 
 *
 * bpmod is a pointer to the BPatch_module to add to the vector
 */
void BPatch_image::addModuleIfExist(BPatch_module *bpmod){

  if( modlist )
    modlist->push_back(bpmod);
  
}

/*
 * BPatch_image::ModuleListExist
 *
 * Checks to see if modlist has been created.
 */
bool BPatch_image::ModuleListExist(){

  if ( modlist )
    return true;
  else
    return false;
}
  
