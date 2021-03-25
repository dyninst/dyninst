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

// $Id: BPatch_image.C,v 1.120 2008/06/20 22:00:02 legendre Exp $

#define BPATCH_FILE

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <string>

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
#include "BPatch_function.h" 
#include "debug.h"
#include "BPatch_point.h"
#include "BPatch_object.h"

#include "addressSpace.h"

#include "dynProcess.h"

#include "debug.h"

#include "parseAPI/h/CodeSource.h"

#include "ast.h"
using namespace Dyninst;
using namespace std;

/*
 * BPatch_image::BPatch_image
 *
 * Construct a BPatch_image for the given process.
 */

BPatch_image::BPatch_image(BPatch_addressSpace *_addSpace) :
   addSpace(_addSpace), defaultModule(NULL)
{
   _srcType = BPatch_sourceProgram;
}

/*
 * BPatch_image::BPatch_image
 *
 * Construct a BPatch_image.
 */
BPatch_image::BPatch_image() :
   addSpace(NULL), defaultModule(NULL)
{
   _srcType = BPatch_sourceProgram;
}

/* 
 * Cleanup the image's memory usage when done.
 *
 */
BPatch_image::~BPatch_image()
{
   for (ModMap::iterator iter = modmap.begin(); iter != modmap.end(); ++iter) {
      delete iter->second;
   }
   for (ObjMap::iterator iter = objmap.begin(); iter != objmap.end(); ++iter) {
      delete iter->second;
   }

   for (unsigned j = 0; j < removed_list.size(); j++) {
      delete removed_list[j];
   }
}


/*
 * getThr - Return the BPatch_thread
 *
 */
BPatch_thread *BPatch_image::getThr()
{
   assert(addSpace->getType() == TRADITIONAL_PROCESS);
   BPatch_process * bpTemp = dynamic_cast<BPatch_process *>(addSpace);

   assert(bpTemp->threads.size() > 0);
   return bpTemp->threads[0];
}

BPatch_process *BPatch_image::getProcess()
{
   if (addSpace->getType() == TRADITIONAL_PROCESS)
      return dynamic_cast<BPatch_process *>(addSpace);
   else
      return NULL;   
}

BPatch_addressSpace *BPatch_image::getAddressSpace()
{
   return addSpace;
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

BPatch_Vector<BPatch_function *> *BPatch_image::getProcedures(bool incUninstrumentable)
{
   BPatch_Vector<BPatch_function *> *proclist = new BPatch_Vector<BPatch_function *>;
   bool some_succeeded = false;
   if (proclist == NULL) return NULL;

   BPatch_Vector<BPatch_module *> *mods = getModules();

   for (unsigned int i = 0; i < (unsigned) mods->size(); i++) {
      bool result = (*mods)[i]->getProcedures(*proclist, incUninstrumentable);
      if (result)
         some_succeeded = true;
   }
   if (!some_succeeded) {
      delete proclist;
      return NULL;
   }
   
   return proclist;
}

bool BPatch_image::getProcedures(BPatch_Vector<BPatch_function*> &procs, bool incUninstrumentable)
{
   bool some_succeeded = false;
   BPatch_Vector<BPatch_module *> *mods = getModules();

   for (unsigned int i = 0; i < (unsigned) mods->size(); i++) {
      bool result = (*mods)[i]->getProcedures(procs, incUninstrumentable);
      if (result)
         some_succeeded = true;
   }
   return some_succeeded;
}


BPatch_Vector<BPatch_parRegion *> *BPatch_image::getParRegions(bool incUninstrumentable)
{
   BPatch_Vector<BPatch_function *> procList;
   bool result = getProcedures(procList, incUninstrumentable);
   if (!result)
      return NULL;

   BPatch_Vector<BPatch_parRegion *> *parRegionList = new BPatch_Vector<BPatch_parRegion *>;
   if (parRegionList == NULL) return NULL;


   for (unsigned int i=0; i < (unsigned) procList.size(); i++) {
      func_instance * intF = procList[i]->lowlevel_func();
      const std::vector<int_parRegion *> intPR = intF->parRegions();

      for (unsigned int j=0; j < (unsigned) intPR.size(); j++)
      {
         BPatch_parRegion * pR = new BPatch_parRegion(intPR[j], procList[i]);
         parRegionList->push_back(pR);
      }
   }

   return parRegionList;
}

/*
 * BPatch_image::getGlobalVariables
 *
 * Returns a list of all variables in the image upon success, and NULL
 * upon failure.
 */
BPatch_Vector<BPatch_variableExpr *> *BPatch_image::getGlobalVariables()
{
   if (!addSpace)
      return NULL;

   BPatch_Vector<BPatch_variableExpr *> *varlist = new BPatch_Vector<BPatch_variableExpr *>;
   if (!varlist) return NULL;

   bool result = getVariables(*varlist);
   if (!result) {
      delete varlist;
      return NULL;
   }
   return varlist;
}

bool BPatch_image::getVariables(BPatch_Vector<BPatch_variableExpr *> &vars)
{
   bool some_succeeded = false;

   if (!addSpace)
      return false;

   BPatch_Vector<BPatch_module *> mods;
   getModules(mods);
   
   for (unsigned int m = 0; m < mods.size(); m++) {
      BPatch_module *module = mods[m];
      
      bool result = module->getVariables(vars);
      if (result)
         some_succeeded = true;
   }

   return some_succeeded;
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

bool BPatch_image::getModules(BPatch_Vector<BPatch_module *> &mods)
{
   BPatch_Vector<BPatch_module *> *ret = getModules();
   if (!ret) {
      return false;
   }
   mods = *ret;
   return true;
}

BPatch_Vector<BPatch_module *> *BPatch_image::getModules() 
{
   std::vector<mapped_module *> modules;
   std::vector<AddressSpace *> as;
   addSpace->getAS(as);
   
   for (unsigned i=0; i<as.size(); i++) {
      as[i]->getAllModules(modules);

      // We may have created a singleton module already -- check to see that we 
      // don't double-create
      for (unsigned j = 0; j < modules.size(); j++ ) {
         mapped_module *map_mod = modules[j];
         findOrCreateModule(map_mod);
      }
   }

   return &modlist;
} 

void BPatch_image::getObjects(std::vector<BPatch_object *> &objs) {
   // Make sure modules are created; we don't delay these
   getModules();

   std::vector<AddressSpace *> as;
   addSpace->getAS(as);
   
   for (unsigned i=0; i<as.size(); i++) {
      const std::vector<mapped_object *> &objs = as[i]->mappedObjects();

      for (unsigned j = 0; j < objs.size(); j++) {
         findOrCreateObject(objs[j]);
      }
   }

   for (ObjMap::iterator iter = objmap.begin(); iter != objmap.end(); ++iter) {
      objs.push_back(iter->second);
   }
}

/*
 * BPatch_image::findModule
 *
 * Returns module with <name>, NULL if not found
 */
BPatch_module *BPatch_image::findModule(const char *name, bool substring_match) 
{
   char buf[512];

   if (!name) {
      bperr("%s[%d]:  findModule:  no module name provided\n",
            __FILE__, __LINE__);
      return NULL;
   }

   BPatch_module *target = NULL;

   for (ModMap::iterator iter = modmap.begin(); iter != modmap.end(); ++iter) {
      BPatch_module *mod = iter->second;
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

   std::vector<AddressSpace *> as;
   addSpace->getAS(as);
   
   mapped_module *mod = NULL;
   for (unsigned i=0; i<as.size(); i++) {
      mod = as[i]->findModule(tmp,substring_match);
      if (mod)
         break;
   }

   free(tmp);
   if (!mod) return NULL;

   target = findOrCreateModule(mod);
   return target;
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

BPatch_Vector<BPatch_function*> *BPatch_image::findFunction(const char *name, 
      BPatch_Vector<BPatch_function*> &funcs, 
      bool showError,
      bool regex_case_sensitive,
      bool incUninstrumentable)
{
   std::vector<AddressSpace *> as;
   addSpace->getAS(as);
   assert(as.size());

   if (NULL == strpbrk(name, REGEX_CHARSET)) {
      //  usual case, no regex
      std::vector<func_instance *> foundIntFuncs;
      for (unsigned i=0; i<as.size(); i++) {
         as[i]->findFuncsByAll(std::string(name), foundIntFuncs);
      }
      if (!foundIntFuncs.size())
      {
         // Error callback...
         if (showError) {
            std::string msg = std::string("Image: Unable to find function: ") + 
               std::string(name);
            BPatch_reportError(BPatchSerious, 100, msg.c_str());
         }
         return NULL;
      }
      // We have a list; if we don't want to include uninstrumentable,
      // scan and check
      for (unsigned int fi = 0; fi < foundIntFuncs.size(); fi++) {
         if (foundIntFuncs[fi]->isInstrumentable() || incUninstrumentable) {
            BPatch_function *foo = addSpace->findOrCreateBPFunc(foundIntFuncs[fi], NULL);
            funcs.push_back(foo);
         }
      }

      if (funcs.size() > 0)
         return &funcs;
      if (showError) {
         std::string msg = std::string("Image: Unable to find function: ") + 
            std::string(name);
         BPatch_reportError(BPatchSerious, 100, msg.c_str());
      }
      return NULL;
   }

#if !defined(os_windows)
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
         std::string msg = std::string("Image: Unable to find function pattern: ") 
            + std::string(name) + ": regex error --" + std::string(errbuf);
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

   std::vector<func_instance *> all_funcs;
   for (unsigned i=0; i<as.size(); i++) {
      as[i]->getAllFunctions(all_funcs);
   }

   for (unsigned ai = 0; ai < all_funcs.size(); ai++) {
      func_instance *func = all_funcs[ai];
      // If it matches, push onto the vector
      // Check all pretty names (and then all mangled names if 
      // there is no match)
      bool found_match = false;

      for (auto piter = func->pretty_names_begin(); 
	   piter != func->pretty_names_end();
	   ++ piter)
      {
         const string &pName = *piter;
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

      for (auto miter = func->symtab_names_begin(); 
	   miter != func->symtab_names_end();
	   ++miter) 
      {
         const string &mName = *miter;
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
      std::string msg = std::string("Unable to find pattern: ") + std::string(name);
      BPatch_reportError(BPatchSerious, 100, msg.c_str());
   }
#endif
   return NULL;
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
      void *user_data, int showError, 
      bool incUninstrumentable)
{
   std::vector<func_instance *> all_funcs;
   std::vector<AddressSpace *> as;
   addSpace->getAS(as);
   assert(as.size());

   for (unsigned i=0; i<as.size(); i++)
      as[i]->getAllFunctions(all_funcs);

   for (unsigned ai = 0; ai < all_funcs.size(); ai++) {
      func_instance *func = all_funcs[ai];
      // If it matches, push onto the vector
      // Check all pretty names (and then all mangled names if there is no match)
      bool found_match = false;

      for (auto piter = func->pretty_names_begin(); 
	   piter != func->pretty_names_end();
	   ++piter) {
	const string &pName = *piter;
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

BPatch_function *BPatch_image::findFunction(unsigned long addr)
{
   std::vector<AddressSpace *> as;
   addSpace->getAS(as);
   assert(as.size());
   func_instance *ifunc = as[0]->findOneFuncByAddr(addr);
   if (!ifunc)
      return NULL;

   return addSpace->findOrCreateBPFunc(ifunc,NULL);
}

bool BPatch_image::findFunction(Dyninst::Address addr, 
                                   BPatch_Vector<BPatch_function *> &funcs)
{
   std::vector<AddressSpace *> as;
   std::set<func_instance *> ifuncs;
   bool result;

   addSpace->getAS(as);
   assert(as.size());

   result = as[0]->findFuncsByAddr(addr, ifuncs);
   if (!result)
      return false;

   assert(ifuncs.size());
   for (std::set<func_instance *>::iterator iter = ifuncs.begin();
       iter != ifuncs.end(); ++iter) 
   {
      BPatch_function *bpfunc = addSpace->findOrCreateBPFunc(*iter, NULL);
      funcs.push_back(bpfunc);
   }

   return true;
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
BPatch_variableExpr *BPatch_image::findVariable(const char *name, 
                                                   bool showError)
{
    std::vector<int_variable *> vars;
    std::vector<AddressSpace *> as;
    int_variable *var = NULL;
    
    if (!addSpace)
       return NULL;
    
    addSpace->getAS(as);

    for (unsigned i=0; i<as.size(); i++) {
      (void)as[i]->findVarsByAll(name, vars);
       if (vars.size()) {
          var = vars[0];
          break;
       }
    }

    if (!var) {
       std::string under_name = std::string("_") + std::string(name);
       for (unsigned i=0; i<as.size(); i++) {
	 (void)as[i]->findVarsByAll(under_name, vars);
          if (vars.size()) {
             var = vars[0];
             break;
          }
       }
    }
       
    if (!var) {
       for (unsigned i=0; i<as.size(); i++) {
          string defaultNamespacePref = as[i]->getAOut()->parse_img()->
             getObject()->getDefaultNamespacePrefix();
          string prefix_name = defaultNamespacePref + string(".") + string(name);
          (void)as[i]->findVarsByAll(prefix_name, vars);
          if (vars.size()) {
             var = vars[0];
             break;
          }
       }
    }

    if (!var) {
       if (showError) {
          string msg = std::string("Unable to find variable: ") + 
             string(name);
          showErrorCallback(100, msg);
       }
       return NULL;
    }

   BPatch_variableExpr *bpvar = addSpace->findOrCreateVariable(var);

   assert(bpvar);
   return bpvar;
}

//
// findVariable
//	scp	- a BPatch_point that defines the scope of the current search
//	name	- name of the variable to find.
//

BPatch_variableExpr *BPatch_image::findVariable(BPatch_point &scp,
                                                       const char *name, bool showError)
{
	// Get the function to search for it's local variables.
	// XXX - should really use more detailed scoping info here - jkh 6/30/99
	BPatch_function *func = const_cast<BPatch_function *> (scp.getFunction());

	if (!func) 
	{
		std::string msg = std::string("point passed to findVariable lacks a function\n address point type passed?");
		showErrorCallback(100, msg);
		return NULL;
	}

	AddressSpace *as = func->lladdSpace;

	BPatch_localVar *lv = func->findLocalVar(name);

	if (!lv) 
	{
		// look for it in the parameter scope now
		lv = func->findLocalParam(name);
	}

	if (lv) 
	{
		return new BPatch_variableExpr(addSpace, as, lv, lv->getType(), &scp); 
	}

	// finally check the global scope.
	// return findVariable(name);

	/* If we have something else to try, don't report errors on this failure. */
	bool reportErrors = true;
	char mangledName[100];
	func->getName( mangledName, 100 );
	char * lastScoping = NULL;      

	if ( strrchr( mangledName, ':' ) != NULL ) 
	{ 
		reportErrors = false; 
	}

	BPatch_variableExpr * gsVar = findVariable( name, reportErrors ? showError : false );

	if ( gsVar == NULL ) 
	{
		/* Try finding it with the function's scope prefixed. */

		if ( (lastScoping = strrchr( mangledName, ':' )) != NULL ) 
		{
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

bool BPatch_image::getSourceLines(unsigned long addr, 
                                     BPatch_Vector<BPatch_statement> &lines )
{
   unsigned int originalSize = lines.size();
   BPatch_Vector< BPatch_module * > * modules = getModules();

   /* Iteratate over the modules, looking for addr in each. */

   for (unsigned int i = 0; i < modules->size(); i++ ) 
   {
      BPatch_module *m = (*modules)[i];
      //  address-to-offset conversion done in module routine 
      m->getSourceLines(addr,lines);
   }

   if ( lines.size() != originalSize ) 
   {
      return true; 
   }	

   //fprintf(stderr, "%s[%d]:  fail to getSourceLines for addr %lu\n", FILE__, __LINE__, addr);

   return false;
} /* end getSourceLines() */

bool BPatch_image::getAddressRanges( const char * lineSource, 
      unsigned int lineNo, 
      std::vector< SymtabAPI::AddressRange > & ranges )
{
   unsigned int originalSize = ranges.size();

   //  First check to see if we can find the named module
   //  If so, use it for the lookup

   BPatch_module *target_mod = findModule(lineSource, false);

   if (target_mod)
   {
      target_mod->getAddressRanges(lineSource, lineNo, ranges);
      if ( ranges.size() != originalSize ) 
      {
         return true; 
      }
   }

   //  If we cannot find the given module, try (perhaps against all hope)
   //  looking in all the modules sequentially

   BPatch_Vector< BPatch_module * > * modules = getModules();

   // Iteratate over the modules, looking for ranges in each. 

   for ( unsigned int i = 0; i < modules->size(); i++ ) 
   {
      BPatch_module *m = (*modules)[i];
      m->getAddressRanges(lineSource, lineNo, ranges);
   }

   if ( ranges.size() != originalSize ) 
   {
      return true; 
   }

   return false;
} 


char *BPatch_image::getProgramName(char *name, unsigned int len) 
{
   std::vector<AddressSpace*> as;
   addSpace->getAS(as);
   AddressSpace *aout = as[0];

   if (!aout->mappedObjects().size()) {
      strncpy(name, "<no program defined>", len);
   }

   const char *imname =  aout->getAOut()->fullName().c_str();
   if (NULL == imname) imname = "<unnamed image>";

   strncpy(name, imname, len);
   return name;
}

char *BPatch_image::getProgramFileName(char *name, unsigned int len)
{
   std::vector<AddressSpace*> as;
   addSpace->getAS(as);
   AddressSpace *aout = as[0];

   if (!aout->mappedObjects().size()) {
      // No program defined yet
      strncpy(name, "<no program defined>", len);
   }

   string imname =  aout->getAOut()->fileName();
   if (imname.empty()) imname = "<unnamed image file>";

   strncpy(name, imname.c_str(), len);
   return name;
}

BPatch_module *BPatch_image::findModule(mapped_module *base) 
{
   ModMap::iterator iter = modmap.find(base);
   if (iter != modmap.end()) return iter->second;
   return NULL;
}

BPatch_object *BPatch_image::findObject(mapped_object *base) 
{
   std::map<mapped_object *, BPatch_object *>::iterator iter = objmap.find(base);
   if (iter != objmap.end()) return iter->second;
   return NULL;
}

BPatch_module *BPatch_image::findOrCreateModule(mapped_module *base) 
{
   BPatch_module *bpm = findModule(base);

   if (bpm == NULL) {
      bpm = new BPatch_module( addSpace, base->proc(), base, this );
      modmap[base] = bpm;
      modlist.push_back(bpm);
      assert(modmap.size() == modlist.size());
   }
   assert(bpm != NULL);
   return bpm;
}

BPatch_object *BPatch_image::findOrCreateObject(mapped_object *base) 
{
   BPatch_object *bpo = findObject(base);
   
   if (bpo == NULL) {
      bpo = new BPatch_object( base, this );
      objmap[base] = bpo;
   }
   assert(bpo != NULL);
   return bpo;
}


/* BPatch_image::parseNewFunctions
 *
 * Uses function entry addresses to trigger the parsing of new
 * functions.  It may re-trigger parsing on existing modules or create
 * new ones to be parsed. 
 *
 * 1. Assign entry points to mapped_objects or existing functions
 * 2. Trigger parsing in mapped_objects that contain function entry points
 * 3. Construct list of modules affected by the parsing
 *
 * (full description in ../h/BPatch_image.h)
 */
bool BPatch_image::parseNewFunctions(BPatch_Vector<BPatch_module*> &affectedModules, 
				     const BPatch_Vector<Dyninst::Address> &funcEntryAddrs)
{
    using namespace SymtabAPI;
    if (addSpace->getType() != TRADITIONAL_PROCESS) {
        fprintf(stderr,"%s[%d] ERROR: parseNewFunctions has only been "
                "implemented for live processes\n", __FILE__, __LINE__);
        return false;
    }
    if (funcEntryAddrs.empty()) {
        fprintf(stderr,"%s[%d] parseNewFunctions requires a non-empty vector "
                "of function entry points\n", __FILE__, __LINE__);
        return false;
    }

    //getProcess()->lowlevel_process()->updateActiveMultis();

/* 1. Assign entry points to mapped_objects or existing functions */
    std::vector<AddressSpace*> asv;
    addSpace->getAS(asv);
    AddressSpace *as = asv[0];
    std::vector<mapped_object *> allobjs = as->mappedObjects();

    // check that the functions have not been parsed yet
    vector<Address> funcEntryAddrs_(funcEntryAddrs); // make an editable copy
    vector<Address> objEntries;
    vector<ParseAPI::Function *> blockFuncs;

    // group funcEntryAddrs by the mapped_objects that they fit into
    for (unsigned int i=0; i < allobjs.size() && !funcEntryAddrs_.empty(); i++) { 

        Symtab *curSymtab = allobjs[i]->parse_img()->getObject();
        Address baseAddress = allobjs[i]->codeBase();

        // iterate through func entry addrs to see which of them correspond to
        //  the current mapped object
            
        for (vector<Address>::iterator curEntry = funcEntryAddrs_.begin();
             curEntry != funcEntryAddrs_.end();) 
        {
            Region *region = curSymtab->findEnclosingRegion(*curEntry-baseAddress);
            if (region) {
                objEntries.push_back(*curEntry);
                curEntry = funcEntryAddrs_.erase(curEntry);
            } else {
                curEntry++;
            }
        }

        /* 2. Trigger parsing in mapped_objects that contain function entry points */
        if (!objEntries.empty()) {
            allobjs[i]->parseNewFunctions(objEntries);

            /* 3. Construct list of modules affected by parsing */
            // Start by finding newly parsed blocks, and previously parsed 
            // blocks that were not previously marked as function entries
            const std::vector<parse_block*> newBlocks = 
                allobjs[i]->parse_img()->getNewBlocks();
            std::set<mapped_module*> mods;
            for (unsigned int j=0; j < newBlocks.size(); j++) {
                newBlocks[j]->getFuncs(blockFuncs);
                pdmodule *pdmod = 
                    dynamic_cast<parse_func*>(blockFuncs[0])->pdmod();
                mods.insert( allobjs[i]->findModule(pdmod) );
                blockFuncs.clear();
            }
            for (vector<Address>::iterator eit = objEntries.begin(); 
                 eit != objEntries.end();
                 eit++)
            {  // add entry blocks, they may have existed previously, in which
               // case they are not included in newBlocks
               func_instance *func = allobjs[i]->findFuncByEntry(*eit);
               if (func) {
                  mods.insert( func->mod() );
               }
            }
            objEntries.clear();
            // copy changed modules to affectedModules
            std::set<mapped_module*>::iterator miter = mods.begin();
            for (; miter != mods.end(); miter++) {
                affectedModules.push_back(findOrCreateModule(*miter));
            }
        }
    }


    if (!funcEntryAddrs_.empty()) {
        fprintf(stderr, "%s[%d] parseNewFunctions failed to parse %d "
                "functions which were not enclosed by known memory regions\n", 
                __FILE__,__LINE__,(int)funcEntryAddrs_.size());
        return false;
    }

    if (affectedModules.empty()) {
        return false;
    }
    return true;
}

void BPatch_image::removeAllModules()
{
   for (ModMap::iterator iter = modmap.begin(); iter != modmap.end(); ++iter) {
      iter->second->handleUnload();
   }
   modmap.clear();
   modlist.clear();
}

void BPatch_image::removeModule(BPatch_module *mod) 
{
   modmap.erase(mod->lowlevel_mod());
   modlist.erase(std::find(modlist.begin(),
                           modlist.end(),
                           mod));
   assert(modmap.size() == modlist.size());
   mod->handleUnload();
}

void BPatch_image::removeObject(BPatch_object* obj)
{
    std::vector<BPatch_module*> mods;
    obj->modules(mods);
    for(auto it = mods.begin();
	it != mods.end();
	++it)
    {
	removeModule(*it);
    }
}

bool BPatch_image::readString(BPatch_variableExpr *expr, std::string &str,
                                 unsigned size_limit)
{
   const BPatch_type *type = expr->getType();
   if (!type) {
      bperr("String read attempted on variable with no type information");
      return false;
   }
   
   Address addr = 0x0;
   if (type->getDataClass() == BPatch_dataPointer) {
      void *value;
      expr->readValue(&value);
      addr = (Address) value;
   }
   else if (type->getDataClass() == BPatch_dataArray) {
      addr = (Address) expr->getBaseAddr();
   }
   else {
      bperr("String read failed on variable with unexpected type");
      return false;
   }
   if (!addr)
      return false;

   return readString(addr, str, size_limit);
}


bool BPatch_image::readString(Address addr, std::string &str, unsigned size_limit)
{
   char *buffer = NULL;
   unsigned buffer_offset = 0;
   unsigned buffer_size = 0;
   bool result;
   bool should_continue = false;

   BPatch_process *proc = dynamic_cast<BPatch_process *>(getAddressSpace());
   if (!proc) {
      //Only works with dynamic processes;
      return false;
   }
   if (proc && !proc->isStopped()) {
      should_continue = true;
      proc->stopExecution();
   }

   std::vector<AddressSpace *> asv;
   getAddressSpace()->getAS(asv);
   AddressSpace *as = asv[0];
   assert(as);

   unsigned word_size = as->getAddressWidth();
   assert(word_size == 4 || word_size == 8);
   
   Address start_word = (word_size == 4) ? (addr & ~0x3) : (addr & ~0x7);
   unsigned start_offset = addr - start_word;

   for (;;) {
      if (!buffer) {
         buffer_size = 256;
         buffer = (char *) malloc(buffer_size);
         assert(buffer);
      }
      if (buffer_offset + word_size + 1 > buffer_size) {
         buffer_size *= 2;
         buffer = (char *) realloc(buffer, buffer_size);
         assert(buffer);
      }

      result = as->readDataSpace((void *) (start_word + buffer_offset), word_size, 
                                 buffer + buffer_offset, false);
      if (!result) {
         proccontrol_printf("[%s:%u] - ERROR reading address %x for string\n",
                       FILE__, __LINE__, start_word + buffer_offset);
         bperr("Error reading from target process");
         goto done;
      }

      buffer_offset += word_size;

      if (size_limit && 
          size_limit < buffer_offset - start_offset) {
         buffer[size_limit + start_offset] = '\0';
         proccontrol_printf("[%s:%u] - WARN string read at %x exceeded size limit of %d",
                       FILE__, __LINE__, addr, size_limit);
         bpwarn("String read exceeded size limit");
         break;
      }

      bool done = false;
      for (unsigned i=1; i<=word_size; i++) {
         if (buffer_offset-i < addr-start_word)
            continue;
         if (buffer[buffer_offset-i] == '\0') {
            done = true;
            break;
         }
      }
      if (done)
         break;
   }

   str = buffer + start_offset;
   result = true;

 done:
   if (buffer)
      free(buffer);
   if (should_continue) {
      assert(proc);
      proc->continueExecution();
   }
   
   return result;
}

void BPatch_image::getNewCodeRegions
        (std::vector<BPatch_function*>&newFuncs,
         std::vector<BPatch_function*>&modFuncs)
{
    std::vector<AddressSpace *> asv;
    getAddressSpace()->getAS(asv);
    AddressSpace *as = asv[0];
    assert(as);

    std::set<parse_func*> newImgFuncs;
    std::set<parse_func*> modImgFuncs;
    const std::vector<parse_block*> blocks = 
        as->getAOut()->parse_img()->getNewBlocks();
    for (unsigned int bidx=0; bidx < blocks.size(); bidx++) {
        parse_block *curB = blocks[bidx];
        vector<ParseAPI::Function*> bfuncs;
        curB->getFuncs(bfuncs);
        vector<ParseAPI::Function*>::iterator fIter = bfuncs.begin();
        for (; fIter !=  bfuncs.end(); fIter++) {
            parse_func *curFunc = dynamic_cast<parse_func*>(*fIter);
            if (curB == curFunc->entryBlock()) {
                newImgFuncs.insert(curFunc);
            } else {
                modImgFuncs.insert(curFunc); 
            }
        }
    }
    std::set<parse_func*>::iterator fIter = newImgFuncs.begin();
    for (; fIter !=  newImgFuncs.end(); fIter++) {
        newFuncs.push_back(addSpace->findOrCreateBPFunc
            (as->findFunction(*fIter),NULL));
    }
    // elements in modImgFuncs may also be in newImgFuncs, don't add such
    // functions
    for (fIter = modImgFuncs.begin(); fIter !=  modImgFuncs.end(); fIter++) {
        if (newImgFuncs.end() == newImgFuncs.find(*fIter)) {
            modFuncs.push_back(addSpace->findOrCreateBPFunc
                (as->findFunction(*fIter),NULL));
        }
    }
}

void BPatch_image::clearNewCodeRegions()
{
    std::vector<AddressSpace *> asv;
    getAddressSpace()->getAS(asv);
    AddressSpace *as = asv[0];
    assert(as);
    vector<mapped_object*> objs = as->mappedObjects();
    for (unsigned oix=0; oix < objs.size(); oix++) {
        if (BPatch_normalMode != objs[oix]->hybridMode()) {
            objs[oix]->parse_img()->clearNewBlocks();
        }
    }
}


Dyninst::PatchAPI::PatchMgrPtr Dyninst::PatchAPI::convert(const BPatch_image *i) {
   return Dyninst::PatchAPI::convert(i->addSpace);
}

bool BPatch_image::findPoints(Dyninst::Address addr,
                                         std::vector<BPatch_point *> &points) {
   BPatch_Vector<BPatch_module *> *mods = getModules();
   
   bool ret = false;

   for (unsigned int i = 0; i < (unsigned) mods->size(); i++) {
      if ((*mods)[i]->findPoints(addr, points))
         ret = true;
   }
   return ret;
}

