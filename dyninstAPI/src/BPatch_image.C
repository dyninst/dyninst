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

// $Id: BPatch_image.C,v 1.26 2001/08/20 19:59:04 bernat Exp $

#define BPATCH_FILE

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
#include "LineInformation.h"

//
// We made this a seperate class to allow us to only expose a pointer to
//    it in the public header files of the dyninst API.  This keeps 
//    dictionary_hash and other internal dyninst things hidden.  The
//    things we do decouple interface from implementation! - jkh 8/28/99
//
class AddrToVarExprHash {
    public:
	AddrToVarExprHash(): hash(addrHash) { }
	dictionary_hash <Address, BPatch_variableExpr*> hash;
};

/*
 * BPatch_image::BPatch_image
 *
 * Construct a BPatch_image for the given process.
 */

BPatch_image::BPatch_image(process *_proc) : proc(_proc)
{
    modlist = NULL;
    AddrToVarExpr = new AddrToVarExprHash();

    _srcType = BPatch_sourceProgram;
}

/*
 * BPatch_image::BPatch_image
 *
 * Construct a BPatch_image.
 */
BPatch_image::BPatch_image() : proc(NULL), modlist(NULL) 
{
    AddrToVarExpr = new AddrToVarExprHash();

    _srcType = BPatch_sourceProgram;
}

/* 
 * Cleanup the image's memory usage when done.
 *
 */
BPatch_image::~BPatch_image()
{
    // modules are shared by multuple programs, don't delete them
    // for (unsigned int i = 0; i < modlist->size(); i++) {
	 // delete (*modlist)[i];
    // }

    delete AddrToVarExpr;
}

/* 
 * getSourceObj - Return the children (modules)
 *
 */
BPatch_Vector<BPatch_sourceObj *> *BPatch_image::getSourceObj()
{
    return (BPatch_Vector<BPatch_sourceObj *> *) getModules();
}

/* 
 * getObjParent - Return the parent (this is the top level so its null)
 *
 */
BPatch_sourceObj *BPatch_image::getObjParent()
{
    return NULL;
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

    for (unsigned int i = 0; i < (unsigned) mods->size(); i++) {
	BPatch_Vector<BPatch_function *> *funcs = (*mods)[i]->getProcedures();
	for (unsigned int j=0; j < (unsigned) funcs->size(); j++) {
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
	    string name = keys[j];
	    type = module->moduleTypes->globalVarsByName[name];
	    assert(type);
	    if (!proc->getSymbolInfo(name, syminfo)) {
		printf("unable to find variable %s\n", name.string_of());
	    }
	    var = AddrToVarExpr->hash[syminfo.addr()];
	    if (!var) {
		var = new BPatch_variableExpr((char *) name.string_of(), proc, 
		    (void *)syminfo.addr(), (const BPatch_type *) type);
		AddrToVarExpr->hash[syminfo.addr()] = var;
	    }
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
    BPatch_module *bpmod = new BPatch_module(proc, curr, this);
    modlist->push_back(bpmod);
  }
  
  // BPatch_procedures are only built on demand, and we need to make sure
  //    they get built.
  (void) getProcedures();

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
	return proc->instPointMap[(Address)address];
    }

    /* Look in the regular instPoints of the enclosing function. */
    function_base *func = proc->findFuncByAddr((Address)address);

    pd_Function* pointFunction = (pd_Function*)func;
    Address pointImageBase = 0;
    if(!pointFunction || !pointFunction->file())
	return NULL;
    image* pointImage = pointFunction->file()->exec();
    proc->getBaseAddress((const image*)pointImage,pointImageBase);

    if (func != NULL) {
	instPoint *entry = const_cast<instPoint *>(func->funcEntry(NULL));
	assert(entry);
	if ((entry->iPgetAddress() == (Address)address) ||
	    (pointImageBase && 
	     ((entry->iPgetAddress() + pointImageBase) == (Address)address))) 
	{
	    return proc->findOrCreateBPPoint(NULL, entry, BPatch_entry);
	}

	const vector<instPoint*> &exits = func->funcExits(NULL);
	for (i = 0; i < exits.size(); i++) {
	    assert(exits[i]);
	    if ((exits[i]->iPgetAddress() == (Address)address) ||
	        (pointImageBase && 
	         ((exits[i]->iPgetAddress() + pointImageBase) == (Address)address))) 
	    {
		return proc->findOrCreateBPPoint(NULL, exits[i], BPatch_exit);
	    }
	}

	const vector<instPoint*> &calls = func->funcCalls(NULL);
	for (i = 0; i < calls.size(); i++) {
	    assert(calls[i]);
	    if ((calls[i]->iPgetAddress() == (Address)address) ||
	        (pointImageBase && 
	         ((calls[i]->iPgetAddress() + pointImageBase) == (Address)address))) 
	    {
		return proc->findOrCreateBPPoint(NULL, calls[i],
						 BPatch_subroutine);
	    }
	}
    }

    /* We don't have an instPoint for this address, so make one. */
    return createInstructionInstPoint(proc, address);
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

    BPatch_function *bpfunc = proc->PDFuncToBPFuncMap[func];
    if (!bpfunc) {
	bpfunc = new BPatch_function(proc, func, NULL);
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
BPatch_variableExpr *BPatch_image::findVariable(const char *name, bool showError)
{
    string full_name = string("_") + string(name);

    Symbol syminfo;
    if (!proc->getSymbolInfo(full_name, syminfo)) {
	string short_name(name);
	if (!proc->getSymbolInfo(short_name, syminfo) && showError) {
	    string msg = string("Unable to find variable: ") + string(name);
	    showErrorCallback(100, msg);
	    return NULL;
	}
    }
    if( syminfo.type() == Symbol::PDST_FUNCTION)
      return NULL;
    
    BPatch_variableExpr *bpvar = AddrToVarExpr->hash[syminfo.addr()];
    if (bpvar) return bpvar;

    // XXX - should this stuff really be by image ??? jkh 3/19/99
    BPatch_Vector<BPatch_module *> *mods = getModules();
    BPatch_type *type = NULL;
    for (int m = 0; m < mods->size(); m++) {
	BPatch_module *module = (*mods)[m];
	//printf("The moduleType address is : %x\n", &(module->moduleTypes));
	type = module->moduleTypes->findVariableType(name);
	if (type) break;
    }
    if (!type) {
        type = BPatch::bpatch->type_Untyped;
    }

    BPatch_variableExpr *ret = new BPatch_variableExpr((char *) name, 
	proc, (void *)syminfo.addr(), (const BPatch_type *) type);
    AddrToVarExpr->hash[syminfo.addr()] = ret;
    return ret;
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
	// create a local expr with the correct frame offset or absolute
	//   address if that is what is needed
	return new BPatch_variableExpr(proc, (void *) lv->getFrameOffset(), 
	    lv->getType(), lv->getFrameRelative(), &scp);
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

    // check the default base types
    type = BPatch::bpatch->stdTypes->findType(name);
    if(type) return type;

    // check the API types of last resort
    return BPatch::bpatch->APITypes->findType(name);

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
	if (func) {
	    if (func->getProc() != proc) {
		printf("got func in the wrong proc\n");
	    }
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

/** method that retrieves the addresses corresponding to a line number
  * in a file.It returns true in success. If the file is not in the image
  * or line number is not found it retuns false. In case of exact match is not
  * asked then the next line number which is greater or equal to the given one
  * is used
  */
//method to get the addresses corresponding to a line number given
//in case of success it returns true and inserts the addresses in to
//the vector given. If the file name is not found or the line information
//is not valid or if the exact match is not found it retuns false.
//If exact match is not asked then the line number is taken to be the
//first one greater or equal to the given one.
bool BPatch_image::getLineToAddr(const char* fileName,unsigned short lineNo,
				 BPatch_Vector<unsigned long>& buffer,
				 bool exactMatch)
{
	string fName(fileName);

	//first get all modules
	BPatch_Vector<BPatch_module*>* appModules =  getModules();

	LineInformation* lineInformation;
	FileLineInformation* fLineInformation = NULL;
		
	//in each module try to find the file
	for(int i=0;i<appModules->size();i++){
		lineInformation = (*appModules)[i]->lineInformation;
		if(!lineInformation)
			continue;
		fLineInformation = lineInformation->getFileLineInformation(fName);		
		if(fLineInformation)
			break;
	}
	
	//if there is no entry for the file is being found then give warning and return
	if(!fLineInformation){
#ifdef DEBUG_LINE
		cerr << "BPatch_image::getLineToAddr : ";
		cerr << fileName << "/line information  is not found/available in the image\n";
#endif
		return false;
	}

	//get the addresses for the line number
	BPatch_Set<Address> addresses;
	if(!fLineInformation->getAddrFromLine(fName,addresses,lineNo,true,exactMatch))
		return false;

	//then insert the elements to the vector given
	Address* elements = new Address[addresses.size()];
	addresses.elements(elements);
	for(int j=0;j<addresses.size();j++)
		buffer.push_back(elements[j]);
	delete[] elements;

	return true;
}
