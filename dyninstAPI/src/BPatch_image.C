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

// $Id: BPatch_image.C,v 1.109 2008/02/15 17:27:40 giri Exp $

#define BPATCH_FILE

#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "process.h"
#include "instPoint.h"
#include "instP.h"
#include "function.h"

#include "mapped_module.h"
#include "mapped_object.h"

#include "BPatch.h"
#include "BPatch_image.h"
#include "BPatch_type.h"
#include "BPatch_collections.h"
#include "BPatch_libInfo.h"
#include "BPatch_statement.h"
#include "symtabAPI/h/LineInformation.h"
#include "BPatch_function.h" 

#include "addressSpace.h"

#include "ast.h"
using namespace Dyninst;


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

BPatch_image::BPatch_image(BPatch_addressSpace *_addSpace) :
   defaultNamespacePrefix(NULL), addSpace(_addSpace)
{
   AddrToVarExpr = new AddrToVarExprHash();
   _srcType = BPatch_sourceProgram;
}


/*
 * BPatch_image::BPatch_image
 *
 * Construct a BPatch_image.
 */

BPatch_image::BPatch_image() :
   defaultNamespacePrefix(NULL),
   addSpace(NULL)
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
   for (unsigned int i = 0; i < modlist.size(); i++) {
      delete modlist[i];
   }

   for (unsigned j = 0; j < removed_list.size(); j++) {
      delete removed_list[j];
   }

   delete AddrToVarExpr;
}


/*
 * getThr - Return the BPatch_thread
 *
 */

BPatch_thread *BPatch_image::getThrInt()
{
   assert(addSpace->getType() == TRADITIONAL_PROCESS);
   BPatch_process * bpTemp = dynamic_cast<BPatch_process *>(addSpace);

   assert(bpTemp->threads.size() > 0);
   return bpTemp->threads[0];
}

BPatch_process *BPatch_image::getProcessInt()
{
   if (addSpace->getType() == TRADITIONAL_PROCESS)
      return dynamic_cast<BPatch_process *>(addSpace);
   else
      return NULL;   
}

BPatch_addressSpace *BPatch_image::getAddressSpaceInt()
{
   return addSpace;
}

/* 
 * getSourceObj - Return the children (modules)
 *
 */
bool BPatch_image::getSourceObjInt(BPatch_Vector<BPatch_sourceObj *> &vect)
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

BPatch_sourceObj *BPatch_image::getObjParentInt()
{
   return NULL;
}

/*
 * BPatch_image::getProcedures
 *
 * Returns a list of all procedures in the image upon success, and NULL
 * upon failure.
 */

BPatch_Vector<BPatch_function *> *BPatch_image::getProceduresInt(bool incUninstrumentable)
{
   BPatch_Vector<BPatch_function *> *proclist =
      new BPatch_Vector<BPatch_function *>;

   if (proclist == NULL) return NULL;

   // XXX Also, what should we do about getting rid of this?  Should
   //     the BPatch_functions already be made and kept around as long
   //     as the process is, so the user doesn't have to delete them?
   BPatch_Vector<BPatch_module *> *mods = getModules();

   for (unsigned int i = 0; i < (unsigned) mods->size(); i++) {
      BPatch_Vector<BPatch_function *> *funcs = (*mods)[i]->getProcedures(incUninstrumentable);
      for (unsigned int j=0; j < (unsigned) funcs->size(); j++) {
         proclist->push_back((*funcs)[j]);
      }
   }

   return proclist;
}


BPatch_Vector<BPatch_parRegion *> *BPatch_image::getParRegionsInt(bool incUninstrumentable)
{
   BPatch_Vector<BPatch_parRegion *> *parRegionList = 
      new BPatch_Vector<BPatch_parRegion *>;

   if (parRegionList == NULL) return NULL;

   BPatch_Vector<BPatch_function *> *procList = getProcedures(incUninstrumentable);

   for (unsigned int i=0; i < (unsigned) procList->size(); i++) {
      int_function * intF = (*procList)[i]->lowlevel_func();
      const pdvector<int_parRegion *> intPR = intF->parRegions();

      for (unsigned int j=0; j < (unsigned) intPR.size(); j++)
      {
         BPatch_parRegion * pR = new BPatch_parRegion(intPR[j], (*procList)[i]);
         parRegionList->push_back(pR);
      }
   }

   return parRegionList;
}

BPatch_variableExpr *BPatch_image::createVarExprByName(BPatch_module *mod, const char *name)
{
   Symbol syminfo;
   BPatch_type *type;

   type = mod->getModuleTypes()->globalVarsByName[name];

   if (!type) {
      switch (syminfo.getSize()) {
         case 1:
            type = findType("char");
            break;
         case 2:
            type = findType("short");
            break;
         case 8:
            type = findType("integer*8");
            break;
         case 4:
         default:
            type = findType("int");
            break;
      }
   }

   if (!type) return NULL;

   if (!addSpace->getAS()->getSymbolInfo(name, syminfo))
      return NULL;


   // Error case. 
   if (syminfo.getAddr() == 0)
      return NULL;

   BPatch_variableExpr *var = AddrToVarExpr->hash[syminfo.getAddr()];
   if (!var) {
      var = new BPatch_variableExpr( const_cast<char *>(name), addSpace,
            (void *)syminfo.getAddr(), type);
      AddrToVarExpr->hash[syminfo.getAddr()] = var;
   }
   return var;
}


/*
 * BPatch_image::getProcedures
 *
 * Returns a list of all procedures in the image upon success, and NULL
 * upon failure.
 */

BPatch_Vector<BPatch_variableExpr *> *BPatch_image::getGlobalVariablesInt()
{
   BPatch_variableExpr *var;
   BPatch_Vector<BPatch_variableExpr *> *varlist =
      new BPatch_Vector<BPatch_variableExpr *>;

   if (varlist == NULL) return NULL;

   // XXX - should this stuff really be by image ??? jkh 3/19/99
   BPatch_Vector<BPatch_module *> *mods = getModules();

   for (unsigned int m = 0; m < mods->size(); m++) {
      BPatch_module *module = (*mods)[m];
      char name[255];
      module->getName(name, sizeof(name));
      pdvector<pdstring> keys = module->getModuleTypes()->globalVarsByName.keys();

      int limit = keys.size();
      for (int j = 0; j < limit; j++) {
         pdstring name = keys[j];
         var = createVarExprByName(module, name.c_str());

         if (var != NULL) {
            varlist->push_back(var);
         }
      }
   }

   return varlist;
}

bool BPatch_image::getVariablesInt(BPatch_Vector<BPatch_variableExpr *> &vars)
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

bool BPatch_image::setFuncModulesCallback(BPatch_function *bpf, void *data)
{
   BPatch_image *img = (BPatch_image *) data;
   if (bpf->getModule() == NULL && bpf->func->mod() != NULL) {
      bpf->mod = img->findModule(bpf->func->mod()->fileName().c_str());
   }

   if ( bpf->getModule() == NULL ) {
      char name[256];
      fprintf(stderr, "Warning: bpf '%s' unclaimed, setting to DEFAULT_MODULE\n",
            bpf->getName( name, 255 ) );
      bpf->setModule( img->defaultModule );
   }   
   return true;
}

BPatch_Vector<BPatch_module *> *BPatch_image::getModulesInt() 
{
   pdvector<mapped_module *> modules;

   addSpace->getAS()->getAllModules(modules);

   if (modules.size() == modlist.size())
      return &modlist;

   // We may have created a singleton module already -- check to see that we 
   // don't double-create
   for (unsigned i = 0; i < modules.size(); i++ ) {
      mapped_module *map_mod = modules[i];
      findOrCreateModule(map_mod);
   }

   return &modlist;
} 

/*
 * BPatch_image::findModule
 *
 * Returns module with <name>, NULL if not found
 */

BPatch_module *BPatch_image::findModuleInt(const char *name, bool substring_match) 
{
   char buf[512];

   if (!name) {
      bperr("%s[%d]:  findModule:  no module name provided\n",
            __FILE__, __LINE__);
      return NULL;
   }

   BPatch_module *target = NULL;

   for (unsigned int i = 0; i < modlist.size(); ++i) {
      BPatch_module *mod = modlist[i];
      assert(mod);
      mod->getName(buf, 512); 
      if (substring_match) { 
         if (strstr(buf, name)) {
            target = mod;
            break;
         }
      }
      else  {//exact match required
         if (!strcmp(name, buf)) {
            target = mod;
            break;
         }
      }
   }

   if (target) 
      return target;

   // process::findModule does a wildcard match, not a substring match 

   char *tmp = (char *) malloc(strlen(name) + 3);
   if (substring_match)
      sprintf(tmp, "*%s*", name); 
   else 
      sprintf(tmp, "%s", name);


   mapped_module *mod = addSpace->getAS()->findModule(tmp,substring_match);

   free(tmp);
   if (!mod) return false;

   target = findOrCreateModule(mod);

   return target;
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

BPatch_point *BPatch_image::createInstPointAtAddrInt(void *address)
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

BPatch_point *BPatch_image::createInstPointAtAddrWithAlt(void *address,
      BPatch_point** alternative,
      BPatch_function* bpf)
{
   Address address_int = (Address) address;

   unsigned i;

   AddressSpace *llAS = addSpace->getAS();

   int_function *func = NULL;

   if (bpf) {
      func = bpf->func;
   }
   else {
      func = llAS->findFuncByAddr(address_int);
   }

   if (func == NULL) return NULL;

   /* See if there is an instPoint at this address */
   instPoint *p = NULL;

   if ((p = func->findInstPByAddr(address_int))) {
      return addSpace->findOrCreateBPPoint(NULL, p, BPatch_locInstruction);
   }

   /* Look in the regular instPoints of the enclosing function. */
   /* This has an interesting side effect: "instrument the first
      instruction" may return with "entry instrumentation", which can
      have different semantics. */

   // If it's in an uninstrumentable function, just return an error.
   if (!func->isInstrumentable()) { 
      return NULL;
   }

   const pdvector<instPoint *> entries = func->funcEntries();
   for (unsigned t = 0; t < entries.size(); t++) {
      assert(entries[t]);
      if (entries[t]->match(address_int)) {
         return addSpace->findOrCreateBPPoint(NULL, entries[t], BPatch_entry);
      }
   }

   const pdvector<instPoint*> &exits = func->funcExits();
   for (i = 0; i < exits.size(); i++) {
      assert(exits[i]);
      if (exits[i]->match(address_int)) {
         return addSpace->findOrCreateBPPoint(NULL, exits[i], BPatch_exit);
      }
   }

   const pdvector<instPoint*> &calls = func->funcCalls();
   for (i = 0; i < calls.size(); i++) {
      assert(calls[i]);
      if (calls[i]->match(address_int))  {
         return addSpace->findOrCreateBPPoint(NULL, calls[i], BPatch_subroutine);
      }
   }

   if (alternative)
      *alternative = NULL;

   /* We don't have an instPoint for this address, so make one. */
   instPoint *newInstP = instPoint::createArbitraryInstPoint(address_int, llAS, func);
   if (!newInstP) return NULL;

   return addSpace->findOrCreateBPPoint(NULL, newInstP, BPatch_locInstruction);
}

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

BPatch_Vector<BPatch_function*> *BPatch_image::findFunctionInt(const char *name, 
      BPatch_Vector<BPatch_function*> &funcs, 
      bool showError,
      bool regex_case_sensitive,
      bool incUninstrumentable)
{
   AddressSpace *as = addSpace->getAS();

   if (NULL == strpbrk(name, REGEX_CHARSET)) {
      //  usual case, no regex
      pdvector<int_function *> foundIntFuncs;
      if (!as->findFuncsByAll(pdstring(name), 
               foundIntFuncs)) {
         // Error callback...
         if (showError) {
            pdstring msg = pdstring("Image: Unable to find function: ") + 
               pdstring(name);
            BPatch_reportError(BPatchSerious, 100, msg.c_str());
         }
         return NULL;
      }
      // We have a list; if we don't want to include uninstrumentable,
      // scan and check
      for (unsigned int fi = 0; fi < foundIntFuncs.size(); fi++) {
         if (foundIntFuncs[fi]->isInstrumentable() || incUninstrumentable) {
            BPatch_function *foo = addSpace->findOrCreateBPFunc(foundIntFuncs[fi], NULL);
            //	BPatch_function *foo = proc->findOrCreateBPFunc(foundIntFuncs[fi], NULL);
            funcs.push_back(foo);
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

   if ( !regex_case_sensitive )
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

   // Regular expression search. This used to be handled at the image
   // class level, but was moved up here to simplify semantics. We
   // have to iterate over every function known to the process at some
   // point, so it might as well be top-level. This is also an
   // excellent candidate for a "value-added" library.

   pdvector<int_function *> all_funcs;
   //llproc->getAllFunctions(all_funcs);
   as->getAllFunctions(all_funcs);

   for (unsigned ai = 0; ai < all_funcs.size(); ai++) {
      int_function *func = all_funcs[ai];
      // If it matches, push onto the vector
      // Check all pretty names (and then all mangled names if there is no match)
      bool found_match = false;
      for (unsigned piter = 0; piter < func->prettyNameVector().size(); piter++) {
         const string &pName = func->prettyNameVector()[piter];
         int err;

         if (0 == (err = regexec(&comp_pat, pName.c_str(), 1, NULL, 0 ))){
            if (func->isInstrumentable() || incUninstrumentable) {
               BPatch_function *foo = addSpace->findOrCreateBPFunc(func,NULL);
               //BPatch_function *foo = proc->findOrCreateBPFunc(func, NULL);
               funcs.push_back(foo);
            }
            found_match = true;
            break;
         }
      }
      if (found_match) continue; // Don't check mangled names

      for (unsigned miter = 0; miter < func->symTabNameVector().size(); miter++) {
         const string &mName = func->symTabNameVector()[miter];
         int err;

         if (0 == (err = regexec(&comp_pat, mName.c_str(), 1, NULL, 0 ))){
            if (func->isInstrumentable() || incUninstrumentable) {
               BPatch_function *foo = addSpace->findOrCreateBPFunc(func,NULL);
               //	   BPatch_function *foo = proc->findOrCreateBPFunc(func, NULL);
               funcs.push_back(foo);
            }
            found_match = true;
            break;
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
BPatch_Vector<BPatch_function*> *BPatch_image::findFunctionPtr(
      const char *name, BPatch_Vector<BPatch_function*> *funcs, bool showError,
      bool regex_case_sensitive, bool incUninstrumentable)
{
   return findFunction(name, *funcs, showError, regex_case_sensitive, incUninstrumentable);
}
#endif

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
BPatch_image::findFunctionWithSieve(BPatch_Vector<BPatch_function *> &funcs, 
      BPatchFunctionNameSieve bpsieve,
      void *user_data, int showError, 
      bool incUninstrumentable)
{
   pdvector<int_function *> all_funcs;

   addSpace->getAS()->getAllFunctions(all_funcs);
   //    proc->llproc->getAllFunctions(all_funcs);

   for (unsigned ai = 0; ai < all_funcs.size(); ai++) {
      int_function *func = all_funcs[ai];
      // If it matches, push onto the vector
      // Check all pretty names (and then all mangled names if there is no match)
      bool found_match = false;

      for (unsigned piter = 0; piter < func->prettyNameVector().size(); piter++) {
         const string &pName = func->prettyNameVector()[piter];

         if ((*bpsieve)(pName.c_str(), user_data)) {
            if (func->isInstrumentable() || incUninstrumentable) {
               BPatch_function *foo = addSpace->findOrCreateBPFunc(func,NULL);
               //BPatch_function *foo = proc->findOrCreateBPFunc(func, NULL);
               funcs.push_back(foo);
            }
            found_match = true;
            break;
         }
      }

      if (found_match) continue; // Don't check mangled names

#if 0
      // Apparently don't check mangled at all

      for (unsigned miter = 0; miter < func->symTabNameVector().size(); miter++) {
         const pdstring &mName = func->symTabNameVector()[miter];
         int err;

         if (0 == (err = regexec(&comp_pat, mName.c_str(), 1, NULL, 0 ))){
            if (func->isInstrumentable() || incUninstrumentable) {
               BPatch_function *foo = proc->findOrCreateBPFunc(func, NULL);
               funcs.push_back(foo);
            }
            found_match = true;
            break;
         }
      }
#endif
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
 * BPatch_image::findFunctionP
 *
 * Finds a function based on an address in the mutatee
 */

BPatch_function *BPatch_image::findFunctionInt(unsigned long addr)
{
   int_function *ifunc = addSpace->getAS()->findFuncByAddr(addr);
   if (!ifunc)
      return NULL;

   return addSpace->findOrCreateBPFunc(ifunc,NULL);
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

BPatch_variableExpr *BPatch_image::findVariableInt(const char *name, bool showError)
{
    pdvector<int_variable *> vars;
    AddressSpace *as = addSpace->getAS();

    if (!as->findVarsByAll(name,vars)) {
        // _name?
        pdstring under_name = pdstring("_") + pdstring(name);
        if (!as->findVarsByAll(under_name,vars)) {
            // "default Namespace prefix?
            string defaultNamespacePref = as->getAOut()->parse_img()->getObject()->getDefaultNamespacePrefix();
            if (defaultNamespacePref != "") {
                pdstring prefix_name = pdstring(defaultNamespacePref.c_str()) + pdstring(".") + pdstring(name);
                if (!as->findVarsByAll(prefix_name, vars) ) {
                    if (showError) {
                        pdstring msg = pdstring("Unable to find variable: ") + pdstring(prefix_name);
                        showErrorCallback(100, msg);
                    }
                    return NULL;
                }
            } else {
                if (showError) {
                    pdstring msg = pdstring("Unable to find variable: ") + pdstring(name);
                    showErrorCallback(100, msg);
                }
                return NULL;
            }
        } else {
            if (showError) {
                pdstring msg = pdstring("Unable to find variable: ") + pdstring(name);
                showErrorCallback(100, msg);
            }
            return NULL;
        }
    }

   assert(vars.size());

   if (vars.size() > 1) {
      cerr << "Warning: found multiple matches for var " << name << endl;
   }

   int_variable *var = vars[0];

   BPatch_variableExpr *bpvar = AddrToVarExpr->hash[var->getAddress()];
   if (bpvar) {
      return bpvar;
   }

   // XXX - should this stuff really be by image ??? jkh 3/19/99
   BPatch_Vector<BPatch_module *> *mods = getModules();
   BPatch_type *type = NULL;

   // XXX look up the type off of the int_variable's module
   BPatch_module *module = NULL;
   for (unsigned int m = 0; m < mods->size(); m++) {
      if ( (*mods)[m]->lowlevel_mod() == var->mod() ) {
         module = (*mods)[m];
         break;
      }
   }
   if (module) {
      type = module->getModuleTypes()->findVariableType(name);
   }
   else {
      bperr("findVariable: failed look up module %s\n",
            var->mod()->fileName().c_str()); 
   }
   if (!type) {
      //  if we can't find the type in the module, check the other modules
      //  (fixes prob on alpha) --  actually seems like most missing types 
      //  end up in DEFAULT_MODULE
      for (unsigned int m = 0; m < mods->size(); m++) {
         BPatch_module *tm = (*mods)[m];
         type = tm->getModuleTypes()->findVariableType(name); 
         if (type) {
            break;
         }

      }

      if (!type) {
         char buf[128];
         sprintf(buf, "%s[%d]:  cannot find type for var %s\n", FILE__, __LINE__, name);
         BPatch_reportError(BPatchWarning, 0, buf);
         type = BPatch::bpatch->type_Untyped;
      }
   }

   char *nameCopy = strdup(name);
   assert(nameCopy);
   BPatch_variableExpr *ret = new BPatch_variableExpr((char *) nameCopy, 
         addSpace, (void *)var->getAddress(), 
         type);
   AddrToVarExpr->hash[var->getAddress()] = ret;
   return ret;
}

//
// findVariable
//	scp	- a BPatch_point that defines the scope of the current search
//	name	- name of the variable to find.
//

BPatch_variableExpr *BPatch_image::findVariableInScope(BPatch_point &scp,
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
      return new BPatch_variableExpr(addSpace, (void *) lv->getFrameOffset(), 
            lv->getRegister(), lv->getType(), lv->getStorageClass(), &scp);
   }

   // finally check the global scope.
   // return findVariable(name);

   /* If we have something else to try, don't report errors on this failure. */
   bool reportErrors = true;
   char mangledName[100];
   func->getName( mangledName, 100 );
   char * lastScoping = NULL;      
   if ( strrchr( mangledName, ':' ) != NULL ) { reportErrors = false; }
   BPatch_variableExpr * gsVar = findVariable( name, reportErrors );

   if ( gsVar == NULL ) {
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

BPatch_type *BPatch_image::findTypeInt(const char *name)
{
    BPatch_type *type;

    assert(BPatch::bpatch != NULL);

    // XXX - should this stuff really be by image ??? jkh 3/19/99
    BPatch_Vector<BPatch_module *> *mods = getModules();
    for (int m = mods->size() -1; m >= 0; m--) {
	BPatch_module *module = (*mods)[m];
	type = module->getModuleTypes()->findType(name);
	if (type) {
           return type;
        }
    }

    // check the default base types
    type = BPatch::bpatch->stdTypes->findType(name);
    if (type)  {
      return type;
    }
    // check the API types of last resort
    type = BPatch::bpatch->APITypes->findType(name);

    return type;

}

bool BPatch_image::getSourceLinesInt(unsigned long addr, 
                                     BPatch_Vector<BPatch_statement> &lines )
{
   unsigned int originalSize = lines.size();
   BPatch_Vector< BPatch_module * > * modules = getModules();

   /* Iteratate over the modules, looking for addr in each. */

   for (unsigned int i = 0; i < modules->size(); i++ ) {
      BPatch_module *m = (*modules)[i];
      //  address-to-offset conversion done in module routine 
      m->getSourceLinesInt(addr,lines);
   }

   if ( lines.size() != originalSize ) { return true; }	

   fprintf(stderr, "%s[%d]:  getSourceLines failing\n", FILE__, __LINE__);
   return false;
} /* eng getSourceLines() */

bool BPatch_image::getAddressRangesInt( const char * lineSource, 
      unsigned int lineNo, 
      std::vector< LineInformation::AddressRange > & ranges ) 
{
   unsigned int originalSize = ranges.size();
   BPatch_Vector< BPatch_module * > * modules = getModules();

   // Iteratate over the modules, looking for ranges in each. 

   for ( unsigned int i = 0; i < modules->size(); i++ ) {
      BPatch_module *m = (*modules)[i];
      m->getAddressRanges(lineSource, lineNo, ranges);
   }

   if ( ranges.size() != originalSize ) { 
      return true; 
   }

   fprintf(stderr, "%s[%d]:  getAddressRanges failing\n", FILE__, __LINE__);
   return false;
} 

char *BPatch_image::getProgramNameInt(char *name, unsigned int len) 
{
   if (!addSpace->getAS()->mappedObjects().size()) {
      // No program defined yet
      strncpy(name, "<no program defined>", len);
   }

   const char *imname =  addSpace->getAS()->getAOut()->fullName().c_str();
   if (NULL == imname) imname = "<unnamed image>";

   strncpy(name, imname, len);
   return name;
}

char *BPatch_image::getProgramFileNameInt(char *name, unsigned int len)
{
   if (!addSpace->getAS()->mappedObjects().size()) {
      // No program defined yet
      strncpy(name, "<no program defined>", len);
   }

   const char *imname =  addSpace->getAS()->getAOut()->fileName().c_str();
   if (NULL == imname) imname = "<unnamed image file>";

   strncpy(name, imname, len);
   return name;
}

#ifdef IBM_BPATCH_COMPAT
char *BPatch_image::programNameInt(char *name, unsigned int len) 
{
   return getProgramName(name, len);
}

int  BPatch_image::lpTypeInt() 
{
   switch(addSpace->getAS()->getAddressWidth()) {
      case 4: return LP32; break;
      case 8: return LP64; break;
      default: return UNKNOWN_LP; break;
   }
};
#endif

BPatch_module *BPatch_image::findModule(mapped_module *base) 
{
   // As below, but don't create if none already exists

   BPatch_module *bpm = NULL;
   for (unsigned j = 0; j < modlist.size(); j++) {
      if (modlist[j]->lowlevel_mod() == base) {
         bpm = modlist[j];
         break;
      }
   }
   return bpm;
}

BPatch_module *BPatch_image::findOrCreateModule(mapped_module *base) 
{
   BPatch_module *bpm = findModule(base);

   if (bpm == NULL) {
      bpm = new BPatch_module( addSpace, base, this );
      modlist.push_back( bpm );
   }
   assert(bpm != NULL);
   return bpm;
}


/* BPatch_image::parseNewRegion
 *
 * This function parses a region in memory owned by a BPatch_process
 *
 * addrStart and addrEnd: these parameters are taken as the bounds for
 * the mapped object that we create.  We check that they do not
 * overlap existing modules.  These values should be set to 0 if they
 * are unknown.  
 * 
 * funcEntryAddrs: this is a vector of function start addresses that
 * seed the control-flow-traversal parsing.  We check to ensure that
 * none of these addresses lie in existing modules.  They must all
 * lie in a contiguous valid region of memory.
 *
 * If both addrStart and addrEnd are 0, we attempt to find a valid 
 * memory region that encloses all of the entries in the funcEntryAddrs
 * vector.  funcEntryAddrs cannot be empty if addrStart and addrEnd are
 * both 0.  
 *
 */
   BPatch_module *BPatch_image::parseNewRegionInt
(unsigned long addrStart, unsigned long addrEnd, 
 BPatch_Vector<unsigned long> *funcEntryAddrs, bool parseGaps)
{
   assert(addSpace->getType() == TRADITIONAL_PROCESS);// KEVINTODO: this should not be an assert

   // ensure that start/end addrs are set or that func entry addrs are given
   if (addrStart == 0 && addrEnd == 0 
         && (funcEntryAddrs == NULL || funcEntryAddrs->size() == 0)) {
      fprintf(stderr,"%s[%d] parseNewRegion requires a list of function entry "
            "addresses to be provided, or the start and end addresses "
            "of a valid memory region to be provided, but got neither\n", 
            __FILE__, __LINE__);
      return NULL;
   }

   BPatch_Vector<BPatch_module *> *allmods = getModules();

   // check that the function entry addrs are not in existing modules
   // and that addrStart and addrEnd do not overlap existing modules
   for (unsigned int i=0; i < allmods->size(); i++) {
      Address curStart, curEnd;
      image *curImg = (*allmods)[i]->lowlevel_mod()->pmod()->imExec();
      curStart = (curImg->imageOffset() < curImg->dataOffset()) ? 
         curImg->imageOffset() : curImg->dataOffset();
      curEnd = (curImg->imageOffset()+curImg->imageLength() 
            < curImg->imageOffset()+curImg->dataLength()) ? 
         curImg->imageOffset()+curImg->imageLength() :
            curImg->dataOffset()+curImg->dataLength();
      // function entry address check
      for (unsigned int j=0; funcEntryAddrs && j < funcEntryAddrs->size(); j++) {
         if ((*funcEntryAddrs)[j] >= curStart && (*funcEntryAddrs)[j] < curEnd ) {
            fprintf(stderr, "%s[%d] Call to parseNewRegion included function "
                  "entry point %lx that overlaps existing module %s, which "
                  "occupies memory range [0x%lX 0x%lX]\n",__FILE__,__LINE__,
                  (*funcEntryAddrs)[j], curImg->file().c_str(), 
                  (long)curStart, (long)curEnd);
            return NULL;
         }
      }

      // overlap check
      if (addrStart <= (curEnd + curImg->desc().loadAddr()) 
            && addrEnd >= (curStart + curImg->desc().loadAddr())) {
         fprintf(stderr, "%s[%d] addMemModule received a request for a new "
               "region that overlaps existing module %s, which occupies "
               "memory range [0x%lX 0x%lX]", __FILE__,__LINE__,
               curImg->file().c_str(), (long)curStart, (long)curEnd);
         return NULL;
      }
   }

   // if addrStart & addrEnd are not set, find the module that
   // contains all funcEntryAddrs and set addrStart and addrEnd

   if (addrStart == 0 && addrEnd == 0 && funcEntryAddrs != NULL) { // KEVINTODO: could make this more general
      bool foundSection = false;
#if defined(os_windows) 
      for (unsigned int i=0; !foundSection && i < allmods->size(); i++) {
         image *curImg = (*allmods)[i]->lowlevel_mod()->pmod()->imExec();
         // check the PE header for uninitialized data sections
         std::vector<Section*> peSections;
         if (curImg->getObject()->getAllSections(peSections)) {
            Address baseAddress = curImg->desc().loadAddr();
            printf("sections list in image %s\n", curImg->name().c_str());
            for (unsigned int j=0; j < peSections.size(); j++) {
               Section *cursec = peSections[j];
               printf("section #%d: \"%s\"isLoadable=%d [0x%x 0x%x] "
                     "rawDataPtr=0x%x\n", //KEVINTODO: remove or hide this 
                     cursec->getSecNumber(), cursec->getSecName().c_str(), 
                     cursec->isLoadable(), 
                     cursec->getSecAddr() +baseAddress,
                     cursec->getSecAddr() +cursec->getSecSize() +baseAddress,
                     cursec->getPtrToRawData());
               bool inSection = true;
               for (unsigned int k=0; inSection && k < funcEntryAddrs->size(); k++) {
                  if (cursec->getPtrToRawData() 
                        || (*funcEntryAddrs)[k] < cursec->getSecAddr() + baseAddress
                        || (*funcEntryAddrs)[k] >= (cursec->getSecAddr() 
                           + cursec->getSecSize() 
                           + baseAddress)) {
                     inSection = false;
                  }
               }
               if (inSection) {
                  foundSection = true;
                  addrStart = cursec->getSecAddr() + baseAddress;
                  addrEnd = cursec->getSecAddr() + cursec->getSecSize() + baseAddress;
               }
            }
         }
      }
      // KEVINTODO: search list of loaded Windows dlls for dlls not listed in allmods
      // KEVINTODO: should I be searching the list of segments listed in the LDT?
#else       // check in procfs or linkmaps for a region that might contain this
      //KEVINTODO: implement procfs and linkmaps searching for empty regions
#endif
      if (!foundSection) {
         fprintf(stderr, "%s[%d] parseNewRegion tried to find a region to "
               "parse based on the addresses that were sent to it, but "
               "failed to find a valid memory region that contained all "
               "addresses\n", __FILE__, __LINE__);
         return NULL;
      }
   }

   // Copy the region to a temp file: /tmp/MemRegion_START_END
   BPatch_process * proc = dynamic_cast<BPatch_process *>(addSpace);
   void *regionBuf = malloc(addrEnd - addrStart);
   if (!addSpace->getAS()->readDataSpace((void*)addrStart, addrEnd-addrStart, 
            regionBuf, false)) {
      fprintf(stderr, "%s[%d]: Failed to read from region [%X %X]\n",
            __FILE__, __LINE__,(int)addrStart, 
            (int)addrEnd);
   }

   char regName[64];
   sprintf(regName, "/tmp/MemRegion_%lX_%lX", addrStart, addrEnd);
   FILE *regionFD = fopen(regName, "w");
   if (regionFD == NULL) { // try to write to /tmp
      fprintf(stderr,"%s[%d]Was unable to open file %s for writing, will "
            "attempt to write to current working directory\n",
            FILE__,__LINE__,regName);
      sprintf(regName, "MemRegion_%lX_%lX", addrStart, addrEnd);
      FILE *regionFD = fopen(regName, "w");
      if (regionFD == NULL) { // try writing to working directory
         fprintf(stderr,"%s[%d]Secondary attempt to open %s also failed\n",
               FILE__,__LINE__,regName);
         free(regionBuf);
         return NULL;
      }
   }
   fwrite(regionBuf, 1, addrEnd-addrStart, regionFD);
   fclose(regionFD);
   free(regionBuf);

   // create a fileDescriptor and mapped_object and add region as shared object
   sprintf(regName, "/tmp/MemRegion_%lX_%lX", addrStart, addrEnd);
   fileDescriptor fdesc = fileDescriptor(regName,0,addrEnd - addrStart);
   mapped_object *obj = mapped_object::createMappedObject
      (fdesc, addSpace->getAS(), parseGaps);

   // add function for the entry point we found
   for (unsigned int i=0; funcEntryAddrs && i < funcEntryAddrs->size(); i++) {
      obj->parse_img()->addFunctionStub((*funcEntryAddrs)[i] - addrStart);
   }

   // force module to be parsed and return BPatch_module
   obj->parse_img()->analyzeIfNeeded(); 
   proc->llproc->addASharedObject(obj);
   pdvector<mapped_module*> mods = obj->getModules();
   if (mods.size() == 0) {
      fprintf(stderr,"%s[%d] Failed to create module for region [%lx %lx]\n",
            FILE__, __LINE__, addrStart, addrEnd);
      return NULL;
   }

   return new BPatch_module(addSpace, mods[0], this);
}

// Merge unresolved control flow vectors of all mapped objects
// return the merged vector of all unresolved control flow

BPatch_Vector<BPatch_point *> *BPatch_image::getUnresolvedControlFlowInt()
{
   unresolvedCF.clear();
   for (unsigned int i=0; i < modlist.size(); i++) {
      BPatch_Vector<BPatch_point*> *modCalls = 
         modlist[i]->getUnresolvedControlFlow();
      for (unsigned int j=0; j < modCalls->size(); j++) {
         unresolvedCF.push_back((*modCalls)[j]);
      }
   }
   return &unresolvedCF;
}

void BPatch_image::removeAllModules()
{
   BPatch_Vector<BPatch_module *>::iterator i;
   for (i = modlist.begin(); i != modlist.end(); i++)
   {
      (*i)->handleUnload();
   }
   modlist.clear();
}

void BPatch_image::removeModule(BPatch_module *mod) 
{
#if !defined(USE_DEPRECATED_BPATCH_VECTOR)
   modlist.erase(std::find(modlist.begin(),
            modlist.end(),
            mod));
#else
   for (unsigned j = 0; j < modlist.size(); j++) {
      if (modlist[j] == mod) {
         modlist.erase(j);
      }
   }
#endif
   mod->handleUnload();
}

