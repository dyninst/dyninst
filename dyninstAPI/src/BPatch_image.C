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

// $Id: BPatch_image.C,v 1.65 2005/01/18 00:51:53 eli Exp $

#define BPATCH_FILE

#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "process.h"
#include "symtab.h"
#include "instPoint.h"
#include "instP.h"

#include "BPatch.h"
#include "BPatch_image.h"
#include "BPatch_type.h"
#include "BPatch_collections.h"
#include "LineInformation.h"
#include "ast.h"

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

BPatch_image::BPatch_image(BPatch_thread *_thr) :
   proc(_thr->proc), appThread(_thr), defaultNamespacePrefix(NULL)
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
BPatch_image::BPatch_image() :
        proc(NULL), 
        defaultNamespacePrefix(NULL),
        modlist(NULL)
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
bool BPatch_image::getSourceObj(BPatch_Vector<BPatch_sourceObj *> &vect)
{
    BPatch_Vector<BPatch_module *> *temp = getModules();
    if (temp) {
       vect = * (BPatch_Vector<BPatch_sourceObj *> *) temp;
       return (true);
    } else {
	return (false);
    }
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


BPatch_variableExpr *BPatch_image::createVarExprByName(BPatch_module *mod, const char *name)
{
    Symbol syminfo;
    BPatch_type *type;
    
    type = mod->moduleTypes->globalVarsByName[name];
    assert(type);
    if (!proc->getSymbolInfo(name, syminfo)) {
	bperr("unable to find variable %s\n", name);
    }
    BPatch_variableExpr *var = AddrToVarExpr->hash[syminfo.addr()];
    if (!var) {
	var = new BPatch_variableExpr( const_cast<char *>(name), appThread,
	    (void *)syminfo.addr(), (const BPatch_type *) type);
	AddrToVarExpr->hash[syminfo.addr()] = var;
    }
    return var;
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
    //BPatch_type *type;
    for (unsigned int m = 0; m < mods->size(); m++) {
	BPatch_module *module = (*mods)[m];
	char name[255];
	module->getName(name, sizeof(name));
	pdvector<pdstring> keys = module->moduleTypes->globalVarsByName.keys();
	int limit = keys.size();
	for (int j = 0; j < limit; j++) {
	    pdstring name = keys[j];
	    var = createVarExprByName(module, name.c_str());
	    varlist->push_back(var);
	}
    }


    return varlist;
}

bool BPatch_image::getVariables(BPatch_Vector<BPatch_variableExpr *> &vars)
{
    BPatch_Vector<BPatch_variableExpr *> *temp = BPatch_image::getGlobalVariables();

    if (temp) {
	vars = *temp;
	return true;
    } else {
	vars = BPatch_Vector<BPatch_variableExpr *>();
	return false;
    }
}


/*
 * BPatch_image::getModules
 *
 * Returns a list of all procedures in the image upon success, and NULL
 * upon failure.
 */
BPatch_Vector<BPatch_module *> *BPatch_image::getModules() {
	if( modlist ) { return modlist; }
	
	modlist = new BPatch_Vector< BPatch_module *>;
	if( modlist == NULL ) { return NULL; }
  
	pdvector< module * > * pdModules = proc->getAllModules();
	/* Generate the BPatch_functions for every module before
	   constructing the BPatch_modules.  This allows us to
	   parse the debug information once per image.*/
	unsigned int i; 
	for( i = 0; i < pdModules->size(); i++ ) {
		pdmodule * currentModule = (pdmodule *) ((* pdModules)[i]);
		pdvector< function_base * > * currentFunctions = currentModule->getFunctions();
		for( unsigned int j = 0; j < currentFunctions->size(); j++ ) {
			/* The constructor will try to register the bpf with proc's map,
			   so check first.  This happens when, for instance, when we parse libunwind
			   during BPatch_thread creation. */
			if( ! proc->PDFuncToBPFuncMap.defines( (* currentFunctions)[j] ) ) {
				new BPatch_function( proc, (* currentFunctions)[j], NULL );
				}
			} /* end iteration over functions in modules */
		} /* end initial iteration over modules */
	
	/* With all the BPatch_functions created, generate the modules.
	   The BPatch_module constructor will set its bpfs to point to itself,
	   and the parser will cache per-image type collections. */
	char moduleName[255];   
	BPatch_module * defaultModule = NULL;
	for( i = 0; i < pdModules->size(); i++ ) {
		pdmodule * currentModule = (pdmodule *) ((* pdModules)[i]);
		BPatch_module * bpm = new BPatch_module( proc, currentModule, this );
		modlist->push_back( bpm );
		if( strcmp( bpm->getName( moduleName, 255 ), "DEFAULT_MODULE" ) ) { defaultModule = bpm; }
		} /* end of second iteration over modules */		
	assert( defaultModule != NULL ) ;

	/* DEBUG: verify that all known bpfs have non-NULL bpm pointers. */
	dictionary_hash< function_base *, BPatch_function *>::const_iterator iter = proc->PDFuncToBPFuncMap.begin();
	dictionary_hash< function_base *, BPatch_function *>::const_iterator end = proc->PDFuncToBPFuncMap.end();
	for( ; iter != end; ++iter ) {
		//char name[255];
		BPatch_function * bpf = * iter;
		if( bpf->getModule() == NULL ) {
			// /* DEBUG */ fprintf( stderr, "Warning: bpf '%s' unclaimed by any module, setting to DEFAULT_MODULE.\n", bpf->getName( name, 255 ) );
			bpf->setModule( defaultModule );
			}
		} /* end iteration over function map */
		
	return modlist;
	} /* end getModules() */



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
	return createInstPointAtAddr(address,NULL);
}

/*
 * BPatch_image::createInstPointAtAddr
 *
 * Returns a pointer to a BPatch_point object representing an
 * instrumentation point at the given address. If the BPatch_function
 * argument is given it has to be the function that address belongs to or NULL.
 * The function is used to bypass the function that the address belongs to
 * The alternative argument is used to retrieve the point if the new point
 * intersects with another already existing one.
 *
 * Returns the pointer to the BPatch_point on success, or NULL upon
 * failure.
 *
 * address	The address that the instrumenation point should refer to.
 */
BPatch_point *BPatch_image::createInstPointAtAddr(void *address,
                                                  BPatch_point** alternative,
                                                  BPatch_function* bpf)
{
    unsigned i;

    /* First look in the list of non-standard instPoints. */
    if (proc->instPointMap.defines((Address)address)) {
        return proc->instPointMap[(Address)address];
    }

    /* Look in the regular instPoints of the enclosing function. */
    pd_Function *func;
    if(bpf)
        func = (pd_Function *) bpf->func;
    else {
        codeRange *range = proc->findCodeRangeByAddress((Address) address);
        if (!range) {
            return NULL;
        }
        
        pd_Function *function_ptr = range->is_pd_Function();
        if (!function_ptr)
            // Address doesn't correspond to a function
            return NULL;
        func = function_ptr;
    }
    
    // If it's in an uninstrumentable function, just return an error.
    if ( !func || !((pd_Function*)func)->isInstrumentable()){ 
        return NULL;
    }

    Address pointImageBase = 0;
    if(!func || !func->file())
	return NULL;
    image* pointImage = func->file()->exec();
    proc->getBaseAddress((const image*)pointImage,pointImageBase);

    if (func != NULL) {
        instPoint *entry = const_cast<instPoint *>(func->funcEntry(NULL));
        assert(entry);
        if ((entry->pointAddr() == (Address)address) ||
            (pointImageBase && 
             ((entry->pointAddr() + pointImageBase) == (Address)address))) 
        {
            return proc->findOrCreateBPPoint(NULL, entry, BPatch_entry);
        }
        
        const pdvector<instPoint*> &exits = func->funcExits(NULL);
        for (i = 0; i < exits.size(); i++) {
            assert(exits[i]);
            if ((exits[i]->pointAddr() == (Address)address) ||
                (pointImageBase && 
                 ((exits[i]->pointAddr() + pointImageBase) == (Address)address))) 
            {
                return proc->findOrCreateBPPoint(NULL, exits[i], BPatch_exit);
            }
        }
        
        const pdvector<instPoint*> &calls = func->funcCalls(NULL);
        for (i = 0; i < calls.size(); i++) {
            assert(calls[i]);
            if ((calls[i]->pointAddr() == (Address)address) ||
                (pointImageBase && 
                 ((calls[i]->pointAddr() + pointImageBase) == (Address)address))) 
            {
                return proc->findOrCreateBPPoint(NULL, calls[i],
                                                 BPatch_subroutine);
            }
        }
    }
    
    if(alternative)
	*alternative = NULL;

    /* We don't have an instPoint for this address, so make one. */
    return createInstructionInstPoint(proc, address, alternative,bpf);
}


/*
 * BPatch_image::findFunctionInImage
 *
 * Searches a single image (class image object) for all functions of
 * the given name.  Results are returned in the vector that is passed in.
 * Note that the vector is not cleared first, so anything in the vector
 * when it is passed in remains there.
 *
 * name		The name of function to look up.
 * img		The image to search.
 * funcs	The vector in which to return the results.
 *
 * This function would make a lot more sense as a method of the image class,
 * but it's here since it deals with BPatch_functions and not pd_Functions.
 */
void BPatch_image::findFunctionInImage(
	const char *name, image *img, BPatch_Vector<BPatch_function*> *funcs)
{
   pd_Function *pdf;
   pdvector<pd_Function*> *pdfv;

   if ((pdfv = img->findFuncVectorByPretty(name)) != NULL) {
      assert(pdfv->size() > 0);

      for (unsigned int i = 0; i < pdfv->size(); i++) {
         BPatch_function * foo = proc->findOrCreateBPFunc((*pdfv)[i]);
         funcs->push_back( foo );
      }

	
   } else {

      if ((pdf = img->findFuncByMangled(name)) != NULL)
         funcs->push_back(proc->findOrCreateBPFunc(pdf));
   }

   // Note that we can only return one non instrumentable function right now.
   if ((pdf = img->findNonInstruFunc(name)) != NULL)
      funcs->push_back(proc->findOrCreateBPFunc(pdf));

}

/*
 * BPatch_image::findFunctionPatternInImage
 *
 * Searches a single image (class image object) for all functions that match
 * the given regex pattern.  Results are returned in the vector that is passed in.
 * Note that the vector is not cleared first, so anything in the vector
 * when it is passed in remains there.
 *
 * pattern	The compiled regex pattern of function to look up.
 * img		The image to search.
 * funcs	The vector in which to return the results.
 *
 * This function would make a lot more sense as a method of the image class,
 * but it's here since it deals with BPatch_functions and not pd_Functions.
 */
#if !defined(i386_unknown_nt4_0) && !defined(mips_unknown_ce2_11) // no regex for M$
void BPatch_image::findFunctionPatternInImage(regex_t *comp_pat, image *img, 
					      BPatch_Vector<BPatch_function*> *funcs)
{
  pdvector<pd_Function*> pdfv;
  
  img->findFuncVectorByPrettyRegex(&pdfv, comp_pat);

  for (unsigned int i = 0; i < pdfv.size(); i++)
    funcs->push_back(proc->findOrCreateBPFunc((pdfv)[i]));
  
  if (!pdfv.size()) { // didn't find any pretty matches, try mangled    
    img->findFuncVectorByMangledRegex(&pdfv, comp_pat);
    for (unsigned int j = 0; j < pdfv.size(); ++j) 
      funcs->push_back(proc->findOrCreateBPFunc(pdfv[j]));
  }
}
#endif

/*
 * BPatch_image::findFunction
 *
 * Fills a vector with BPatch_function pointers representing all functions in
 * the image with the given name.  Returns a pointer to the vector that was
 * passed in on success, and NULL on error.
 *
 * name		The name of function to look up.
 * funcs	The vector in which to place the results.
 */


BPatch_Vector<BPatch_function*> *BPatch_image::findFunction(
	const char *name, BPatch_Vector<BPatch_function*> &funcs, bool showError,
	bool regex_case_sensitive)
{

   if (NULL == strpbrk(name, REGEX_CHARSET)) {
      //  usual case, no regex
      findFunctionInImage(name, proc->getImage(), &funcs);
      
      if (proc->dynamiclinking && proc->shared_objects) {
         for(unsigned int j = 0; j < proc->shared_objects->size(); j++){
            const image *obj_image = ((*proc->shared_objects)[j])->getImage();
            if (obj_image) {
               findFunctionInImage(name, const_cast<image*>(obj_image), &funcs);
            }
         }
      }

      if (funcs.size() > 0) {
         return &funcs;
      } else {
         
         if (showError) {
            pdstring msg = pdstring("Image: Unable to find function: ") + 
               pdstring(name);
            BPatch_reportError(BPatchSerious, 100, msg.c_str());
         }
         return NULL;
      }
   }

#if !defined(i386_unknown_nt4_0) && !defined(mips_unknown_ce2_11) // no regex for M$
   // REGEX falls through:
   regex_t comp_pat;
   int err, cflags = REG_NOSUB | REG_EXTENDED;
   
   if( !regex_case_sensitive )
      cflags |= REG_ICASE;
  
   //cerr << "compiling regex: " <<name<<endl;

   if (0 != (err = regcomp( &comp_pat, name, cflags ))) {
      char errbuf[80];
      regerror( err, &comp_pat, errbuf, 80 );
      if (showError) {
         cerr << __FILE__ << ":" << __LINE__ << ":  REGEXEC ERROR: "<< errbuf << endl;
         pdstring msg = pdstring("Image: Unable to find function pattern: ") 
            + pdstring(name) + ": regex error --" + pdstring(errbuf);
         BPatch_reportError(BPatchSerious, 100, msg.c_str());
      }
      // remove this line
      cerr << __FILE__ << ":" << __LINE__ << ":  REGEXEC ERROR: "<< errbuf << endl;
      return NULL;
   }
   
   findFunctionPatternInImage(&comp_pat, proc->getImage(), &funcs);
   //cerr << "matched regex: " <<name<<"in symbols, results: "<<funcs.size()<<endl;

   if (proc->dynamiclinking && proc->shared_objects) {
      for(unsigned int j = 0; j < proc->shared_objects->size(); j++){
         const image *obj_image = ((*proc->shared_objects)[j])->getImage();
         if (obj_image) {
            findFunctionPatternInImage(&comp_pat, const_cast<image*>(obj_image), &funcs);
            //cerr << "matched regex: " <<name<<"in so, results: "<<funcs.size()<<endl;
         }
      }
   }
  
   regfree(&comp_pat);

   if (funcs.size() > 0) {
      return &funcs;
   } 
   
   if (showError) {
      pdstring msg = pdstring("Unable to find pattern: ") + pdstring(name);
      BPatch_reportError(BPatchSerious, 100, msg.c_str());
   }
#endif
   return NULL;
}

#ifdef IBM_BPATCH_COMPAT
//  a wrapper for IBM compatibility.  The function vector is assumed to be
//  allocated before calling.
BPatch_Vector<BPatch_function*> *BPatch_image::findFunction(
        const char *name, BPatch_Vector<BPatch_function*> *funcs, bool showError,
        bool regex_case_sensitive)
{
  return findFunction(name, *funcs, showError, regex_case_sensitive);
}
#endif

void BPatch_image::sieveFunctionsInImage(image *img, BPatch_Vector<BPatch_function *> *funcs,
					BPatchFunctionNameSieve bpsieve, void *user_data) 
{
  pdvector<pd_Function*> pdfv;
  
  if (NULL != img->findFuncVectorByPretty(bpsieve, user_data,&pdfv)) {
    assert(pdfv.size() > 0);
    
    for (unsigned int i = 0; i < pdfv.size(); i++)
      funcs->push_back(proc->findOrCreateBPFunc(pdfv[i]));
  } else {
    
    if (NULL != img->findFuncVectorByMangled(bpsieve, user_data, &pdfv))
      for (unsigned int i = 0; i < pdfv.size(); i++)
	funcs->push_back(proc->findOrCreateBPFunc(pdfv[i]));
  }
}

/*
 * BPatch_image::findFunction 2
 *
 * Fills a vector with BPatch_function pointers representing all functions in
 * the image according to the user defined sieving function.  
 * Returns a pointer to the vector that was passed in on success, and NULL on error.
 * 
 *
 * bpsieve      User-provided boolean function used to determine inclusion in the
 *              filtered set.
 * user_data    a pointer to a user-defined data space for use by bpsieve
 * funcs	The vector in which to place the results.
 */

BPatch_Vector<BPatch_function *> *
BPatch_image::findFunction(BPatch_Vector<BPatch_function *> &funcs, 
			   BPatchFunctionNameSieve bpsieve,
			   void *user_data, int showError)
{

  sieveFunctionsInImage(proc->getImage(), &funcs, bpsieve, user_data);
  
  if (proc->dynamiclinking && proc->shared_objects) {
    for(unsigned int j = 0; j < proc->shared_objects->size(); j++){
      const image *obj_image = ((*proc->shared_objects)[j])->getImage();
      if (obj_image) {
	sieveFunctionsInImage(const_cast<image *>(obj_image), &funcs, bpsieve, user_data);
      }
    }
  }

  if (funcs.size() > 0) {
    return &funcs;
  } 
    
  if (showError) {
    const char *msg = "No function matches for sieve provided";
    BPatch_reportError(BPatchSerious, 100, msg);
  }

  return NULL;
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
    pdstring full_name = pdstring("_") + pdstring(name);

    Symbol syminfo;
    if (!proc->getSymbolInfo(full_name, syminfo)) {
       pdstring short_name(name);
       if (!proc->getSymbolInfo(short_name, syminfo) && showError) {
          if (defaultNamespacePrefix) {
             full_name = pdstring(defaultNamespacePrefix) + "." + name;
             if (!proc->getSymbolInfo(full_name, syminfo) && showError) {
                pdstring msg = pdstring("Unable to find variable: ") + full_name;
                showErrorCallback(100, msg);
                return NULL;
             }
          } else if (showError) {
             pdstring msg = pdstring("Unable to find variable: ") + pdstring(name);
             showErrorCallback(100, msg);
             return NULL;
          }
       }
    }

    /* FIXME: this will probably cause DWARF to explode. */
    if( syminfo.type() == Symbol::PDST_FUNCTION)
      return NULL;
    
    BPatch_variableExpr *bpvar = AddrToVarExpr->hash[syminfo.addr()];
    if (bpvar) return bpvar;

    // XXX - should this stuff really be by image ??? jkh 3/19/99
    BPatch_Vector<BPatch_module *> *mods = getModules();
    BPatch_type *type = NULL;
    for (unsigned int m = 0; m < mods->size(); m++) {
	BPatch_module *module = (*mods)[m];
	//bperr("The moduleType address is : %x\n", &(module->moduleTypes));
	type = module->moduleTypes->findVariableType(name);
	if (type) break;
    }
    if (!type) {
        type = BPatch::bpatch->type_Untyped;
    }

    char *nameCopy = strdup(name);
    assert(nameCopy);
    BPatch_variableExpr *ret = new BPatch_variableExpr((char *) nameCopy, 
	appThread, (void *)syminfo.addr(), (const BPatch_type *) type);
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
    BPatch_function *func = const_cast<BPatch_function *> (scp.getFunction());
    if (!func) {
       pdstring msg = pdstring("point passed to findVariable lacks a function\n address point type passed?");
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
	return new BPatch_variableExpr(appThread, (void *) lv->getFrameOffset(), 
	    lv->getRegister(), lv->getType(), lv->getStorageClass(), &scp);
    }

    // finally check the global scope.
    // return findVariable(name);

    /* If we have something else to try, don't report errors on this failure. */
    bool reportErrors = true;
    char mangledName[100];
    func->getName( mangledName, 100 );
    char * lastScoping = NULL;      
    if( strrchr( mangledName, ':' ) != NULL ) { reportErrors = false; }
    BPatch_variableExpr * gsVar = findVariable( name, reportErrors );

    if( gsVar == NULL ) {
      /* Try finding it with the function's scope prefixed. */

      if( (lastScoping = strrchr( mangledName, ':' )) != NULL ) {
        * (lastScoping + sizeof(char)) = '\0';
	char scopedName[200];
	memmove( scopedName, mangledName, strlen( mangledName ) );
	memmove( scopedName + strlen( mangledName ), name, strlen( name ) );
	scopedName[ strlen( mangledName ) + strlen( name ) ] = '\0';
	bperr( "Searching for scoped name '%s'\n", scopedName );
	gsVar = findVariable( scopedName ); 
        }
      }
    return gsVar;
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
    for (unsigned int m = 0; m < mods->size(); m++) {
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
	pdstring fName(fileName);

	//first get all modules
	BPatch_Vector<BPatch_module*>* appModules =  getModules();

	LineInformation* lineInformation;
	FileLineInformation* fLineInformation = NULL;
		
	//in each module try to find the file
	for(unsigned int i=0;i<appModules->size();i++){
		lineInformation = (*appModules)[i]->getLineInformation();
		if(!lineInformation) {
		  cerr << __FILE__ << __LINE__ <<":  no Line Information avail!!!" << endl;
			continue;
		}
		fLineInformation = lineInformation->getFileLineInformation(fName);		
		if(fLineInformation)
			break;
	}
	
	//if there is no entry for the file is being found then give warning and return
	if(!fLineInformation){
		return false;
	}

	//get the addresses for the line number
	BPatch_Set<Address> addresses;
	if(!fLineInformation->getAddrFromLine(fName,addresses,lineNo,true,exactMatch))
		return false;

	//then insert the elements to the vector given
	Address* elements = new Address[addresses.size()];
	addresses.elements(elements);
	for(unsigned j=0;j<addresses.size();j++)
		buffer.push_back(elements[j]);
	delete[] elements;

	return true;
}


char *BPatch_image::getProgramName(char *name, unsigned int len) 
{
  const char *imname =  proc->getImage()->name().c_str();
  if (NULL == imname) imname = "<unnamed image>";

  strncpy(name, imname, len);
  return name;
}

  
char *BPatch_image::getProgramFileName(char *name, unsigned int len)
{
  const char *imname =  proc->getImage()->file().c_str();
  if (NULL == imname) imname = "<unnamed image file>";

  strncpy(name, imname, len);
  return name;
}

#ifdef IBM_BPATCH_COMPAT
char *BPatch_image::programName(char *name, unsigned int len) {
    return getProgramName(name, len);
}



int  BPatch_image::lpType() 
{
    return 0;
};
#endif



