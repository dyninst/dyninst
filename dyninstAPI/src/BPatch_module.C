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

#include <stdio.h>
#include <ctype.h>
#include <string>

#define BPATCH_FILE

#include "process.h"
#include "debug.h"
#include "BPatch.h"
#include "BPatch_module.h"
#include "BPatch_libInfo.h"
#include "BPatch_function.h"
#include "BPatch_statement.h"
#include "BPatch_collections.h"
#include "symtabAPI/h/Type.h"    // For BPatch_type related stuff

#include "mapped_module.h"
#include "mapped_object.h"
#include "instPoint.h"

#if defined(os_windows)
#include <dbghelp.h>
#include <cvconst.h>
#include <oleauto.h>
#endif

#if defined(arch_ia64)
#include "arch-ia64.h"
#endif

#if defined(TIMED_PARSE)
#include <sys/time.h>
#endif
#ifdef TIMED_PARSE
int max_addr_per_line =0;
int max_line_per_addr =0;
#endif

#if defined(USES_DWARF_DEBUG)
#include "elf.h"
#include "dwarf.h"
#include "libdwarf.h"
#endif

std::string current_func_name;
std::string current_mangled_func_name;
BPatch_function *current_func = NULL;

#if defined(os_windows)
BPatch_type *getType(HANDLE p, Address mod_base, int typeIndex, BPatch_module *mod = NULL);
#endif

/*
 * BPatch_module::getSourceObj()
 *
 * Return the contained source objects (e.g. functions).
 *
 */

bool BPatch_module::getSourceObj(BPatch_Vector<BPatch_sourceObj *> &vect)
{
   if (!mod) return false;

   BPatch_Vector<BPatch_function *> *temp;
   temp = getProcedures();

   if (temp) {
      for (unsigned int i = 0; i < temp->size(); ++i) {
         vect.push_back((*temp)[i]);
      }
      vect = *(BPatch_Vector<BPatch_sourceObj *> *) temp;
      return true;
   } else {
      return false;
   }
}

/*
 * BPatch_function::getObjParent()
 *
 * Return the parent of the function (i.e. the image)
 *
 */

BPatch_sourceObj *BPatch_module::getObjParent()
{
   return (BPatch_sourceObj *) img;
}

/* XXX temporary */
char *BPatch_module::getNameInt(char *buffer, int length)
{
   if (!mod)
      return NULL;

   string str = mod->fileName();

   strncpy(buffer, str.c_str(), length);

   return buffer;
}

const char *BPatch_module::libraryNameInt()
{
   if (!mod)
      return NULL;

   if (isSharedLib())      
      return mod->fullName().c_str();

   return NULL;
}

char *BPatch_module::getFullNameInt(char *buffer, int length)
{
   if (!mod)
      return NULL;
   string str = mod->fullName();

   strncpy(buffer, str.c_str(), length);

   return buffer;
}


BPatch_module::BPatch_module( BPatch_addressSpace *_addSpace, 
      mapped_module *_mod,
      BPatch_image *_img ) :
   addSpace( _addSpace ), 
   mod( _mod ), 
   img( _img ), 
   retfuncs(NULL),
   moduleTypes(NULL)
{
   _srcType = BPatch_sourceModule;
   nativeCompiler = _mod->isNativeCompiler();

   switch(mod->language()) {
      case lang_C:
         setLanguage( BPatch_c );
         break;

      case lang_CPlusPlus:
      case lang_GnuCPlusPlus:
         setLanguage( BPatch_cPlusPlus );
         break;

      case lang_Fortran_with_pretty_debug:
         setLanguage( BPatch_f90_demangled_stabstr );
         break;

      case lang_Fortran:
      case lang_CMFortran:
         setLanguage( BPatch_fortran );
         break;

      case lang_Assembly:
         setLanguage( BPatch_assembly );
         break;

      case lang_Unknown: 
      default:
         setLanguage( BPatch_unknownLanguage );
         break;
   }; /* end language switch */

} /* end BPatch_module() */

// Public 'destructor' function...
void BPatch_module::handleUnload() 
{
   // Hrm... what to do. For now, mark us as "deleted" so that
   // any other calls return an error.

   // Brainstorm: we can set mod to NULL (as it really is, 
   // having been deleted...) and key off that. Saves a boolean.

   mod = NULL;
}

bool BPatch_module::isValidInt() 
{
   return mod != NULL;
}

BPatch_module::~BPatch_module()
{
   if (moduleTypes) {
      BPatch_typeCollection::freeTypeCollection(moduleTypes);
   }

   for (unsigned i = 0; i < all_funcs.size(); i++) {
      delete all_funcs[i];
   }

   //  egad:  retfuncs seems unfortunately named...  see getProcedures()
   if (retfuncs)
      delete retfuncs;
}

/************Changes still to be made**************************
 * Build moduleTypes from Module::moduleTypes //DONE
 * Build API Types, builtin Types, std types in BPatch from symtab//DONE
 * Add new types directly to symtabAPI from BPatch:: //DONE
 * Make sure fixupUnknown is performed for all functions //I THINK SO(??)
 * Remove the type parsing stuff from here //NOT CALLED
 * class BPatch_type:public class Type //DONE
 * class BPatch_field:public class field //DONE
 * class BPatch_cblock:public class CBlock //DONE
 * Change the classes to use symtabAPI classes //DONE
 * Add the locationList to framePtr conversions on all platforms in dyninst
 * Store the lowAddr, hiAddr in loc_t for variables
 * Store the ranges of function address info from DWARF in SymtabAPI
 * Based on the address passed, construct an ASTNodeptr and return
 * To add new features: location lists, variablesInScope
 * Change storage classes wherever used in dyninstAPI  //DONE(Check again)
 * BPatch_variableExpr->type is basically AstNodePtr->type. change appropriately.//DONE(check)
 * Windows: Is it done on a module-by-module basis??(checking now)
 * Set return types for all functions from Symbol/ Access return types directly from Symbol class//DONE
 */

bool BPatch_module::parseTypesIfNecessary() 
{
   if ( moduleTypes != NULL ) 
      return false;

   if (!isValid())
      return false;

   bool is64 = (mod->pmod()->imExec()->getAddressWidth() == 8);

   if (sizeof(void *) == 8 && !is64) {
      // Terrible Hack:
      //   If mutatee and mutator address width is different,
      //   we need to patch up certain standard types.
      BPatch_type *typePtr;

      typePtr = BPatch::bpatch->builtInTypes->findBuiltInType(-10);
      typePtr->getSymtabType()->setSize(4);

      typePtr = BPatch::bpatch->builtInTypes->findBuiltInType(-19);
      typePtr->getSymtabType()->setSize(4);
   }

   mod->pmod()->mod()->exec()->parseTypesNow();
   moduleTypes = BPatch_typeCollection::getModTypeCollection(this);

   vector<Type *> *modtypes = mod->pmod()->mod()->getAllTypes();
   if (!modtypes)
      return false;

   for (unsigned i=0; i<modtypes->size(); i++) {
      Type *typ = (*modtypes)[i];
      BPatch_type *type = new BPatch_type(typ);
      moduleTypes->addType(type);
   }

   vector<pair<string, Type *> > *globalVars = mod->pmod()->mod()->getAllGlobalVars();
   if (!globalVars)
      return false;

   for (unsigned i=0; i<globalVars->size(); i++){
      if (!(*globalVars)[i].second->getUpPtr())
         BPatch_type *type = new BPatch_type((*globalVars)[i].second);
      moduleTypes->addGlobalVariable((*globalVars)[i].first.c_str(), 
            (BPatch_type *)(*globalVars)[i].second->getUpPtr());
   }
   return true; 
}

#if 0
moduleTypes = BPatch_typeCollection::getModTypeCollection( this );
// /* DEBUG */ fprintf( stderr, "%s[%d]: parsing module '%s' @ %p (file %s) with type collection %p\n",	__FILE__, __LINE__, mod->fileName().c_str(), this, mod->obj()->fileName().c_str(), moduleTypes );

#if ! defined( USES_DWARF_DEBUG )
if ( BPatch::bpatch->parseDebugInfo() ) {
   parseTypes();
}
#elif defined( arch_x86 ) || defined( arch_x86_64 ) || defined( arch_ia64 ) || defined(arch_sparc) || (defined(arch_power) && defined(os_linux))
/* I'm not actually sure about IA-64, but certainly on the other two,
   it's legal and not uncommon to mix STABS and DWARF debug information
   in the same file.  However, this causes problems because of the
   differences in DWARF and STABS numeric type IDs.  In DWARF, the numeric
   type IDs are unique accross the entire file, and are used to resolve
   forward type references.  Thus, we parse all STABS debug information
   before parsing any DWARF information.  Furthermore, DWARF requires
   that all the BPatch_functions exist before parsing.  Thus... */

if ( BPatch::bpatch->parseDebugInfo() ) {
   const pdvector< mapped_module  *> & map_mods = mod->obj()->getModules();

   /* Ensure all functions and type collections are defined. */
   for ( unsigned i = 0; i < map_mods.size(); i++ ) {
      // use map_mods[i] instead of a name to get a precise match
      BPatch_module * bpmod = img->findOrCreateModule( map_mods[i] );
      assert( bpmod != NULL );

      bpmod->getProcedures();

      if ( bpmod->moduleTypes == NULL ) {
         bpmod->moduleTypes = BPatch_typeCollection::getModTypeCollection( bpmod );
      }
   } /* end function instantiation */

   /* We'll need to have two loops anyway, so use three for clarity. */
   for ( unsigned i = 0; i < map_mods.size(); i++ ) {
      // use map_mods[i] instead of a name to get a precise match
      BPatch_module * bpmod = img->findOrCreateModule( map_mods[i] );
      assert( bpmod != NULL );

      image * moduleImage = bpmod->mod->obj()->parse_img();
      assert( moduleImage != NULL );

      bool found = true;
      Section *sec;
      Address   stab_off;
      unsigned  stab_size;          
      Address   stabstr_off;
      unsigned  stabstr_size;          
      if(!moduleImage->getObject()->findSection(sec,".stab"))
         found = false;
      else
      {
         stab_off = sec->getSecAddr();
         stab_size = sec->getSecSize();
         if (!moduleImage->getObject()->findSection(sec, ".stabstr"))
            found = false;
         else
         {
            stabstr_off = sec->getSecAddr();
            stabstr_size = sec->getSecSize();
         }    
      }

      //if( found && stab_off && stab_size && stabstr_off ) 
      /*
         Checking for sizes instead of offsets because offsets might be zero in some cases - Giri
       */
      if ( found && stab_size && stabstr_size) {
         /* This will blow away previous information, but not its own. */
         bpmod->parseStabTypes();

         /* Therefore, blow away left-over STABS information to avoid type conflicts. */
         bpmod->moduleTypes->clearNumberedTypes();            
      }
   } /* end STABS parsing */

   for ( unsigned i = 0; i < map_mods.size(); i++ ) {
      // use map_mods[i] instead of a name to get a precise match
      BPatch_module * bpmod = img->findOrCreateModule( map_mods[i] );
      assert( bpmod != NULL );

      image * moduleImage = bpmod->mod->obj()->parse_img();
      assert( moduleImage != NULL );
      Symtab *moduleObject = moduleImage->getObject();
      Section *sec;


      if ( moduleObject->findSection(sec, ".debug_info")) { bpmod->parseDwarfTypes(); }            
   } /* end DWARF parsing */
} /* end if we'rep parsing debug information at all */
#else 
#error DWARF on platforms other than 86, x86-64, and IA-64 is unsupported.
#endif /* ! defined( USES_DWARF_DEBUG ) */
return;
} /* end parseTypesIfNecessary() */
#endif

BPatch_typeCollection *BPatch_module::getModuleTypesInt() 
{
   parseTypesIfNecessary();
   return moduleTypes;
}

/*
 * BPatch_module::getProcedures
 *
 * Returns a list of all procedures in the module upon success, and NULL
 * upon failure.
 */

   BPatch_Vector<BPatch_function *> *
BPatch_module::getProceduresInt(bool incUninstrumentable)
{
   if (!isValid())
      return NULL;

   if (retfuncs)
      return retfuncs;

   retfuncs = new BPatch_Vector<BPatch_function *>;

   const pdvector<int_function *> &funcs = mod->getAllFunctions();

   for (unsigned int f = 0; f < funcs.size(); f++)
      if (incUninstrumentable || funcs[f]->isInstrumentable()) {
         BPatch_function *bpfunc = addSpace->findOrCreateBPFunc(funcs[f], this);
         retfuncs->push_back(bpfunc);
      }

   return retfuncs;
}

/*
 * BPatch_module::findFunction
 *
 * Returns a vector of BPatch_function* with the same name that is provided or
 * NULL if no function with that name is in the module.  This function
 * searches the BPatch_function vector of the module followed by
 * the int_function of the module.  If a int_function is found
 * a BPatch_function is created and added to the BPatch_function vector of
 * the module.
 * name The name of function to look up.
 */

   BPatch_Vector<BPatch_function *> *
BPatch_module::findFunctionInt(const char *name, 
      BPatch_Vector<BPatch_function *> & funcs,
      bool notify_on_failure, bool regex_case_sensitive,
      bool incUninstrumentable, bool dont_use_regex)
{
   if (!isValid())
      return NULL;

   unsigned size = funcs.size();

   if (!name) {
      char msg[512];
      sprintf(msg, "%s[%d]:  Module %s: findFunction(NULL)...  failing",
            __FILE__, __LINE__, mod->fileName().c_str());
      BPatch_reportError(BPatchSerious, 100, msg);
      return NULL;
   }

   // Do we want regex?
   if (dont_use_regex 
         ||  (NULL == strpbrk(name, REGEX_CHARSET))) {
      pdvector<int_function *> int_funcs;
      if (mod->findFuncVectorByPretty(name, int_funcs)) {
         for (unsigned piter = 0; piter < int_funcs.size(); piter++) {
            if (incUninstrumentable || int_funcs[piter]->isInstrumentable()) 
            {
               BPatch_function * bpfunc = addSpace->findOrCreateBPFunc(int_funcs[piter], this);
               //BPatch_function * bpfunc = proc->findOrCreateBPFunc(int_funcs[piter], this);
               funcs.push_back(bpfunc);
            }
         }
      }
      else {
         if (mod->findFuncVectorByMangled(name,
                  int_funcs)) {
            for (unsigned miter = 0; miter < int_funcs.size(); miter++) {
               if (incUninstrumentable || int_funcs[miter]->isInstrumentable()) 
               {
                  BPatch_function * bpfunc = addSpace->findOrCreateBPFunc(int_funcs[miter], this);
                  //						BPatch_function * bpfunc = proc->findOrCreateBPFunc(int_funcs[miter], this);
                  funcs.push_back(bpfunc);
               }
            }
         }
      }
      if (size != funcs.size())
         return &funcs;
   }
   else {
      // Regular expression search. As with BPatch_image, we handle it here

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
         if (notify_on_failure) {
            cerr << __FILE__ << ":" << __LINE__ << ":  REGEXEC ERROR: "<< errbuf << endl;
            std::string msg = std::string("Image: Unable to find function pattern: ") 
               + std::string(name) + ": regex error --" + std::string(errbuf);
            BPatch_reportError(BPatchSerious, 100, msg.c_str());
         }
         return NULL;
      }

      // Regular expression search. This used to be handled at the image
      // class level, but was moved up here to simplify semantics. We
      // have to iterate over every function known to the process at some
      // point, so it might as well be top-level. This is also an
      // excellent candidate for a "value-added" library.

      const pdvector<int_function *> &int_funcs = mod->getAllFunctions();

      for (unsigned ai = 0; ai < int_funcs.size(); ai++) {
         int_function *func = int_funcs[ai];
         // If it matches, push onto the vector
         // Check all pretty names (and then all mangled names if there is no match)
         bool found_match = false;
         for (unsigned piter = 0; piter < func->prettyNameVector().size(); piter++) {
            const string &pName = func->prettyNameVector()[piter];
            int err;     
            if (0 == (err = regexec(&comp_pat, pName.c_str(), 1, NULL, 0 ))){
               if (func->isInstrumentable() || incUninstrumentable) {
                  BPatch_function *foo = addSpace->findOrCreateBPFunc(func, NULL);
                  //	   BPatch_function *foo = proc->findOrCreateBPFunc(func, NULL);
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
                  BPatch_function *foo = addSpace->findOrCreateBPFunc(func, NULL);
                  //	   BPatch_function *foo = proc->findOrCreateBPFunc(func, NULL);
                  funcs.push_back(foo);
               }
               found_match = true;
               break;
            }
         }
      }

      regfree(&comp_pat);

      if (funcs.size() != size) {
         return &funcs;
      } 

      if (notify_on_failure) {
         std::string msg = std::string("Unable to find pattern: ") + std::string(name);
         BPatch_reportError(BPatchSerious, 100, msg.c_str());
      }
#endif
   }

   if (notify_on_failure) {
      char msg[1024];
      sprintf(msg, "%s[%d]:  Module %s: unable to find function %s",
            __FILE__, __LINE__, mod->fileName().c_str(), name);
      BPatch_reportError(BPatchSerious, 100, msg);

   }
   return &funcs;
}

   BPatch_Vector<BPatch_function *> *
BPatch_module::findFunctionByAddressInt(void *addr, BPatch_Vector<BPatch_function *> &funcs,
      bool notify_on_failure, 
      bool incUninstrumentable)
{
   if (!isValid()) return NULL;

   int_function *pdfunc = NULL;
   BPatch_function *bpfunc = NULL;

   pdfunc = mod->findFuncByAddr((Address)addr);
   if (!pdfunc) {
      if (notify_on_failure) {
         char msg[1024];
         sprintf(msg, "%s[%d]:  Module %s: unable to find function %p",
               __FILE__, __LINE__, mod->fileName().c_str(), addr);
         BPatch_reportError(BPatchSerious, 100, msg);
      }
      return NULL;
   }

   if (incUninstrumentable || pdfunc->isInstrumentable()) {
      bpfunc = addSpace->findOrCreateBPFunc(pdfunc, this);
      //bpfunc = proc->findOrCreateBPFunc(pdfunc, this);
      if (bpfunc) {
         funcs.push_back(bpfunc);
      }
   }
   return &funcs;
}

BPatch_function * BPatch_module::findFunctionByMangledInt(const char *mangled_name,
      bool incUninstrumentable)
{
   if (!isValid()) return NULL;

   BPatch_function *bpfunc = NULL;

   pdvector<int_function *> int_funcs;
   std::string mangled_str(mangled_name);

   if (!mod->findFuncVectorByMangled(mangled_str,
            int_funcs))
      return NULL;

   if (int_funcs.size() > 1) {
      fprintf(stderr, "%s[%d]: Warning: found multiple name matches for %s, returning first\n",
            FILE__, __LINE__, mangled_name);
   }

   int_function *pdfunc = int_funcs[0];

   if (incUninstrumentable || pdfunc->isInstrumentable()) {
      bpfunc = addSpace->findOrCreateBPFunc(pdfunc, this);
   }

   return bpfunc;
}

bool BPatch_module::dumpMangledInt(char * prefix)
{
   mod->dumpMangled(prefix);
   return true;
}

#if 0
extern std::string parseStabString(BPatch_module *, int linenum, char *str, 
      int fPtr, BPatch_typeCommon *commonBlock = NULL);


#if defined(rs6000_ibm_aix4_1)
#include <xcoff.h>

// Gets the stab and stabstring section and parses it for types
// and variables
void BPatch_module::parseTypes()
{
   int i, j;
   int nstabs;
   SYMENT *syms;
   SYMENT *tsym;
   char *stringPool;
   char tempName[9];
   char *stabstr=NULL;
   union auxent *aux;
   image * imgPtr=NULL;
   std::string funcName;
   Address staticBlockBaseAddr = 0;
   BPatch_typeCommon *commonBlock = NULL;
   BPatch_variableExpr *commonBlockVar = NULL;
   std::string currentSourceFile;
   bool inCommonBlock = false;

   if (!mod) return;

   current_func_name = "";
   imgPtr = mod->obj()->parse_img();

   Symtab *objPtr = imgPtr->getObject();
   bool is64 = (mod->pmod()->imExec()->getAddressWidth() == 8);

   fprintf(stderr, "BPatch_module::parseTypes run with is64 == %s\n", is64 ? "true" : "false");
   // If mutatee and mutator address width is different,
   // we need to patch up certain standard types.
   if (sizeof(void *) == 8 && !is64) {
      BPatch_type *oldType;
      BPatch_typeScalar *newType;

      oldType  = BPatch::bpatch->builtInTypes->findBuiltInType(-10);
      newType  = dynamic_cast<BPatch_typeScalar *>(oldType);
      *newType = BPatch_typeScalar(-10, 4, "unsigned long");

      oldType  = BPatch::bpatch->builtInTypes->findBuiltInType(-19);
      newType  = dynamic_cast<BPatch_typeScalar *>(oldType);
      *newType = BPatch_typeScalar(-19, 4, "stringptr");
   }

   void *syms_void = NULL;
   objPtr->get_stab_info(stabstr, nstabs, syms_void, stringPool); 
   syms = (SYMENT *) syms_void;

   bool parseActive = true;
   //fprintf(stderr, "%s[%d]:  parseTypes for module %s: nstabs = %d\n", FILE__, __LINE__,mod->fileName().c_str(),nstabs);
   //int num_active = 0;

   for (i=0; i < nstabs; i++) {
      /* do the pointer addition by hand since sizeof(struct syment)
       *   seems to be 20 not 18 as it should be */
      SYMENT *sym = (SYMENT *) (((char *) syms) + i * SYMESZ);
      unsigned long sym_value = (is64 ? sym->n_value64 : sym->n_value32);

      if (sym->n_sclass == C_FILE) {
         char *moduleName;
         if (is64) {
            moduleName = &stringPool[sym->n_offset64];
         } else if (!sym->n_zeroes32) {
            moduleName = &stringPool[sym->n_offset32];
         } else {
            memset(tempName, 0, 9);
            strncpy(tempName, sym->n_name32, 8);
            moduleName = tempName;
         }
         /* look in aux records */
         for (j=1; j <= sym->n_numaux; j++) {
            aux = (union auxent *) ((char *) sym + j * SYMESZ);
            if (aux->x_file._x.x_ftype == XFT_FN) {
               if (!aux->x_file._x.x_zeroes) {
                  moduleName = &stringPool[aux->x_file._x.x_offset];
               } else {
                  // x_fname is 14 bytes
                  memset(moduleName, 0, 15);
                  strncpy(moduleName, aux->x_file.x_fname, 14);
               }
            }
         }

         currentSourceFile = std::string(moduleName);
         currentSourceFile = mod->processDirectories(currentSourceFile);

         if (strrchr(moduleName, '/')) {
            moduleName = strrchr(moduleName, '/');
            moduleName++;
         }

         if (!strcmp(moduleName, mod->fileName().c_str())) {
            parseActive = true;
            // Clear out old types
            moduleTypes->clearNumberedTypes();
         } else {
            parseActive = false;
         }
      }

      if (!parseActive) continue;

      //num_active++;

      char *nmPtr;
      if (!sym->n_zeroes32 && ((sym->n_sclass & DBXMASK) ||
               (sym->n_sclass == C_BINCL) ||
               (sym->n_sclass == C_EINCL))) {
         long sym_offset = (is64 ? sym->n_offset64 : sym->n_offset32);

         // Symbol name stored in STABS, not string pool.
         if (sym_offset < 3) {
            if (sym_offset == 2 && stabstr[0]) {
               nmPtr = &stabstr[0];
            } else {
               nmPtr = &stabstr[sym_offset];
            }
         } else if (!stabstr[sym_offset-3]) {
            nmPtr = &stabstr[sym_offset];
         } else {
            /* has off by two error */
            nmPtr = &stabstr[sym_offset-2];
         }
#if 0
         bperr("using nmPtr = %s\n", nmPtr);
         bperr("got n_offset = (%d) %s\n", sym_offset, &stabstr[sym_offset]);
         if (sym_offset>=2)
            bperr("got n_offset-2 = %s\n", &stabstr[sym_offset-2]);
         if (sym_offset>=3)
            bperr("got n_offset-3 = %x\n", stabstr[sym_offset-3]);
         if (sym_offset>=4)
            bperr("got n_offset-4 = %x\n", stabstr[sym_offset-4]);
#endif

      } else if (is64) {
         nmPtr = &stringPool[sym->n_offset64];
      } else if (!sym->n_zeroes32) {
         nmPtr = &stringPool[sym->n_offset32];
      } else {
         // names 8 or less chars on inline, not in stabstr
         memset(tempName, 0, 9);
         strncpy(tempName, sym->n_name32, 8);
         nmPtr = tempName;
      }

      if ((sym->n_sclass == C_BINCL) ||
            (sym->n_sclass == C_EINCL) ||
            (sym->n_sclass == C_FUN)) {
         funcName = nmPtr;
         /* The call to parseLineInformation(), below, used to modify the symbols passed to it. */
         if (funcName.find(":") < funcName.length())
            funcName = funcName.substr(0,funcName.find(":"));

         //		I'm not sure why we bother with this here, since we fetch line numbers in symtab.C anyway.
         //		mod->parseLineInformation(proc->llproc, currentSourceFile, 
         //					  funcName, sym,
         //					  linesfdptr, lines, nlines);
      }

      if (sym->n_sclass & DBXMASK) {
         if (sym->n_sclass == C_BCOMM) {
            char *commonBlockName;

            inCommonBlock = true;
            commonBlockName = nmPtr;

            // find the variable for the common block
            BPatch_image *progam = (BPatch_image *) getObjParent();

            commonBlockVar = progam->findVariable(commonBlockName);
            if (!commonBlockVar) {
               bperr("unable to find variable %s\n", commonBlockName);
            } else {
               commonBlock = 
                  dynamic_cast<BPatch_typeCommon *>(const_cast<BPatch_type *> (commonBlockVar->getType()));
               if (commonBlock == NULL) {
                  // its still the null type, create a new one for it
                  commonBlock = new BPatch_typeCommon(commonBlockName);
                  commonBlockVar->setType(commonBlock);
                  moduleTypes->addGlobalVariable(commonBlockName, commonBlock);
               }
               // reset field list
               commonBlock->beginCommonBlock();
            }
         } else if (sym->n_sclass == C_ECOMM) {
            inCommonBlock = false;
            if (commonBlock == NULL)
               continue;

            // copy this set of fields
            BPatch_Vector<BPatch_function *> bpmv;
            if (NULL == findFunction(funcName.c_str(), bpmv) || !bpmv.size()) {
               bperr("unable to locate current function %s\n", funcName.c_str());
            } else {
               BPatch_function *func = bpmv[0];
               commonBlock->endCommonBlock(func, commonBlockVar->getBaseAddr());
            }

            // update size if needed
            if (commonBlockVar)
               commonBlockVar->setSize(commonBlock->getSize());
            commonBlockVar = NULL;
            commonBlock = NULL;
         } else if (sym->n_sclass == C_BSTAT) {
            // begin static block
            // find the variable for the common block
            tsym = (SYMENT *) (((char *) syms) + sym_value * SYMESZ);

            // We can't lookup the value by name, because the name might have been
            // redefined later on (our lookup would then pick the last one)

            // Since this whole function is AIX only, we're ok to get this info

            staticBlockBaseAddr = (is64 ? tsym->n_value64 : tsym->n_value32);;

            /*
               char *staticName, tempName[9];
               if (is64) {
               staticName = &stringPool[tsym->n_offset64];
               } else if (!tsym->n_zeroes32) {
               staticName = &stringPool[tsym->n_offset32];
               } else {
               memset(tempName, 0, 9);
               strncpy(tempName, tsym->n_name32, 8);
               staticName = tempName;
               }
               BPatch_image *progam = (BPatch_image *) getObjParent();

               BPatch_variableExpr *staticBlockVar = progam->findVariable(staticName);
               if (!staticBlockVar) {
               bperr("unable to find static block %s\n", staticName);
               staticBlockBaseAddr = 0;
               } else {
               staticBlockBaseAddr = (Address) staticBlockVar->getBaseAddr();
               }
             */

         } else if (sym->n_sclass == C_ESTAT) {
            staticBlockBaseAddr = 0;
         }

         // There's a possibility that we were parsing a common block that
         // was never instantiated (meaning there's type info, but no
         // variable info

         if (inCommonBlock && commonBlock == NULL)
            continue;

         if (staticBlockBaseAddr && (sym->n_sclass == C_STSYM)) {
            parseStabString(this, 0, nmPtr, 
                  sym_value+staticBlockBaseAddr, commonBlock);
         } else {
            parseStabString(this, 0, nmPtr, sym_value, commonBlock);
         }
      }
   }

   //  fprintf(stderr, "%s[%d]:  parseTypes for %s, num_active = %d\n", FILE__, __LINE__, mod->fileName().c_str(), num_active);
}

#endif

#if ! defined( USES_DWARF_DEBUG ) && ! defined( rs6000_ibm_aix4_1 ) && ! defined( alpha_dec_osf4_0 ) && ! defined( os_windows )
/* Platforms which use DWARF call parseStabTypes() and parseDwarfTypes() directly.
   Our POWER, Alpha, and Windows ports have their own custom parseTypes() functions.  */
void BPatch_module::parseTypes() {
   if (!mod) return;
   image *moduleImage = mod->obj()->parse_img();
   assert( moduleImage != NULL );
   Symtab *moduleObject = moduleImage->getObject();

   bool found = true;
   Section *sec;
   Address   stab_off_= 0;
   unsigned  stab_size_ = 0;          
   Address   stabstr_off_ = 0;
   if (!moduleObject->findSection(sec, ".stab"))
      found = false;
   else
   {
      stab_off_ = sec->getSecAddr();
      stab_size_ = sec->getSecSize();
      if (!moduleObject->findSection(sec, ".stabstr"))
         found = false;
      else
         stabstr_off_ = sec->getSecAddr();	
   }
   if (found && stab_size_) {//has Stab Info
      parseStabTypes();
   }

} /* end BPatch_module::parseTypes() */
#endif /* ! defined( USES_DWARF_DEBUG ) */


#if defined(sparc_sun_solaris2_4) \
   || defined(i386_unknown_solaris2_5) \
|| defined(os_linux)

#include "symtabAPI/src/Object.h" //TODO: Move stabs to symtab and remove
// parseStabTypes:  parses type and variable info, does some init
// does NOT parse file-line info anymore, this is done later, upon request.
void BPatch_module::parseStabTypes() 
{
   if (!mod) return;
   stab_entry *stabptr = NULL;
   const char *next_stabstr = NULL;

   unsigned i;
   char *modName = NULL;
   std::string temp;
   image * imgPtr=NULL;
   char *ptr = NULL, *ptr2 = NULL, *ptr3 = NULL;
   bool parseActive = false;

   std::string* currentFunctionName = NULL;
   Address currentFunctionBase = 0;
   BPatch_variableExpr *commonBlockVar = NULL;
   char *commonBlockName = NULL;
   BPatch_typeCommon *commonBlock = NULL;
   int mostRecentLinenum = 0;

#if defined(TIMED_PARSE)
   struct timeval starttime;
   gettimeofday(&starttime, NULL);
   unsigned int pss_count = 0;
   double pss_dur = 0;
   unsigned int src_count = 0;
   double src_dur = 0;
   unsigned int fun_count = 0;
   double fun_dur = 0;
   struct timeval t1, t2;
#endif

   imgPtr = mod->obj()->parse_img();
   imgPtr->analyzeIfNeeded();
   Symtab *objPtr = imgPtr->getObject();

   bool found = true;
   Section *sec;
   char* stab_off_ = 0;
   unsigned stab_size_ = 0;
   char* stabstr_off_ = 0;

   if (!objPtr->findSection(sec, ".stab"))
      found = false;
   else
   {
      stab_off_ = (char *)sec->getPtrToRawData();
      stab_size_ = sec->getSecSize();
      if (!objPtr->findSection(sec, ".stabstr"))
         found = false;
      else
         stabstr_off_ = (char *)sec->getPtrToRawData();
   }

   char *file_ptr_ = objPtr->mem_image();
   if (found 
         && (stab_off_!=file_ptr_) 
         && stab_size_ 
         && (stabstr_off_!=file_ptr_)) {
      switch (objPtr->getAddressWidth()) {
         case 4: // 32-bit object
            stabptr = new stab_entry_32(stab_off_, stabstr_off_,
                  stab_size_ / sizeof(stab32));
            break;			     
         case 8: // 64-bit object
            stabptr = new stab_entry_64(stab_off_, stabstr_off_,
                  stab_size_ / sizeof(stab32));
            break;			     
      }
   }
   else
      stabptr = new stab_entry_64();

   //Using the Object to get the pointers to the .stab and .stabstr
   // XXX - Elf32 specific needs to be in seperate file -- jkh 3/18/99
   next_stabstr = stabptr->getStringBase();

   for (i=0; i<stabptr->count(); i++) {
      switch(stabptr->type(i)){
         case N_UNDF: /* start of object file */
            /* value contains offset of the next string table for next module */
            // assert(stabptr->nameIdx(i) == 1);
            stabptr->setStringBase(next_stabstr);
            next_stabstr = stabptr->getStringBase() + stabptr->val(i);

            //N_UNDF is the start of object file. It is time to 
            //clean source file name at this moment.
            /*
               if(currentSourceFile){
               delete currentSourceFile;
               currentSourceFile = NULL;
               delete absoluteDirectory;
               absoluteDirectory = NULL;
               delete currentFunctionName;
               currentFunctionName = NULL;
               currentFileInfo = NULL;
               currentFuncInfo = NULL;
               }
             */
            break;

         case N_ENDM: /* end of object file */
            break;

         case N_SO: /* compilation source or file name */
            /* bperr("Resetting CURRENT FUNCTION NAME FOR NEXT OBJECT FILE\n");*/
#ifdef TIMED_PARSE
            src_count++;
            gettimeofday(&t1, NULL);
#endif
            current_func_name = ""; // reset for next object file
            current_mangled_func_name = ""; // reset for next object file
            current_func = NULL;

            modName = const_cast<char*>(stabptr->name(i));
            // cerr << "checkpoint B" << endl;
            ptr = strrchr(modName, '/');
            //  cerr << "checkpoint C" << endl;
            if (ptr) {
               ptr++;
               modName = ptr;
            }

            if (!strcmp(modName, mod->fileName().c_str())) {
               parseActive = true;
               moduleTypes->clearNumberedTypes();
               BPatch_language lang;
               // language should be set in the constructor, this is probably redundant
               switch (stabptr->desc(i)) {
                  case N_SO_FORTRAN:
                     lang = BPatch_fortran;
                     break;

                  case N_SO_F90:
                     lang = BPatch_fortran90;
                     break;

                  case N_SO_AS:
                     lang = BPatch_assembly;
                     break;

                  case N_SO_ANSI_C:
                  case N_SO_C:
                     lang = BPatch_c;
                     break;

                  case N_SO_CC:
                     lang = BPatch_cPlusPlus;
                     break;

                  default:
                     lang = BPatch_unknownLanguage;
                     break;
               }
               if (BPatch_f90_demangled_stabstr != getLanguage())
                  setLanguage(lang);
            } else {
               parseActive = false;
            }

#ifdef TIMED_PARSE
            gettimeofday(&t2, NULL);
            src_dur += (t2.tv_sec - t1.tv_sec)*1000.0 + (t2.tv_usec - t1.tv_usec)/1000.0;
            //src_dur += (t2.tv_sec/1000 + t2.tv_usec*1000) - (t1.tv_sec/1000 + t1.tv_usec*1000) ;
#endif
            break;
         case N_SLINE:
            mostRecentLinenum = stabptr->desc(i);
            break;
         default:
            break;
      }


      if (parseActive || mod->obj()->isSharedLib()) {
         BPatch_Vector<BPatch_function *> bpfv;
         switch(stabptr->type(i)){
            case N_FUN:
#ifdef TIMED_PARSE
               fun_count++;
               gettimeofday(&t1, NULL);
#endif
               //all we have to do with function stabs at this point is to assure that we have
               //properly set the var currentFunctionName for the later case of (parseActive)
               current_func = NULL;
               int currentEntry = i;
               int funlen = strlen(stabptr->name(currentEntry));
               ptr = new char[funlen+1];
               strcpy(ptr, stabptr->name(currentEntry));
               while(strlen(ptr) != 0 && ptr[strlen(ptr)-1] == '\\'){
                  ptr[strlen(ptr)-1] = '\0';
                  currentEntry++;
                  strcat(ptr,stabptr->name(currentEntry));
               }

               char* colonPtr = NULL;
               if(currentFunctionName) delete currentFunctionName;
               if(!ptr || !(colonPtr = strchr(ptr,':')))
                  currentFunctionName = NULL;
               else {
                  char* tmp = new char[colonPtr-ptr+1];
                  strncpy(tmp,ptr,colonPtr-ptr);
                  tmp[colonPtr-ptr] = '\0';
                  currentFunctionName = new std::string(tmp);

                  currentFunctionBase = 0;
                  Symbol info;
                  // Shouldn't this be a function name lookup?

                  /*if (!proc->llproc->getSymbolInfo(*currentFunctionName,
                    info))*/
                  if (!addSpace->getAS()->getSymbolInfo(*currentFunctionName,
                           info))
                  {
                     std::string fortranName = *currentFunctionName + std::string("_");
                     //if (proc->llproc->getSymbolInfo(fortranName,info))
                     if (addSpace->getAS()->getSymbolInfo(fortranName,info))
                     {
                        delete currentFunctionName;
                        currentFunctionName = new std::string(fortranName);
                     }
                  }

                  currentFunctionBase = info.getAddr();

                  delete[] tmp;

                  //	if(currentSourceFile && (currentFunctionBase > 0)){
                  //	lineInformation->insertSourceFileName(
                  //			*currentFunctionName,
                  //			*currentSourceFile,
                  //			&currentFileInfo,&currentFuncInfo);
                  //}
               }
               //  used to be a symbol lookup here to find currentFunctionBase, do we need it?
               delete[] ptr;
#ifdef TIMED_PARSE
               gettimeofday(&t2, NULL);
               fun_dur += (t2.tv_sec - t1.tv_sec)*1000.0 + (t2.tv_usec - t1.tv_usec)/1000.0;
               //fun_dur += (t2.tv_sec/1000 + t2.tv_usec*1000) - (t1.tv_sec/1000 + t1.tv_usec*1000);
#endif
               break;
         }
         if (!parseActive) continue;

         switch(stabptr->type(i)){
            case N_BCOMM:	
               {
                  // begin Fortran named common block 
                  commonBlockName = const_cast<char*>(stabptr->name(i));

                  // find the variable for the common block
                  BPatch_image *progam = (BPatch_image *) getObjParent();
                  commonBlockVar = progam->findVariable(commonBlockName);
                  if (!commonBlockVar) {
                     bperr("unable to find variable %s\n", commonBlockName);
                  } else {
                     commonBlock = dynamic_cast<BPatch_typeCommon *>(const_cast<BPatch_type *> (commonBlockVar->getType()));
                     if (commonBlock == NULL) {
                        // its still the null type, create a new one for it
                        commonBlock = new BPatch_typeCommon(commonBlockName);
                        commonBlockVar->setType(commonBlock);
                        moduleTypes->addGlobalVariable(commonBlockName, commonBlock);
                     }
                     // reset field list
                     commonBlock->beginCommonBlock();
                  }
                  break;
               }

            case N_ECOMM: 
               {
                  // copy this set of fields

                  assert(currentFunctionName);
                  if (NULL == findFunction(currentFunctionName->c_str(), bpfv) || !bpfv.size()) {
                     bperr("unable to locate current function %s\n", currentFunctionName->c_str());
                  } else {
                     if (bpfv.size() > 1) {
                        // warn if we find more than one function with this name
                        bperr("%s[%d]:  WARNING: found %d funcs matching name %s, using the first\n",
                              __FILE__, __LINE__, bpfv.size(), currentFunctionName->c_str());
                     }

                     BPatch_function *func = bpfv[0];
                     commonBlock->endCommonBlock(func, commonBlockVar->getBaseAddr());
                  }

                  // update size if needed
                  if (commonBlockVar)
                     commonBlockVar->setSize(commonBlock->getSize());
                  commonBlockVar = NULL;
                  commonBlock = NULL;
                  break;
               }

               // case C_BINCL: -- what is the elf version of this jkh 8/21/01
               // case C_EINCL: -- what is the elf version of this jkh 8/21/01
            case 32:    // Global symbols -- N_GYSM 
            case 38:    // Global Static -- N_STSYM
            case N_FUN:
            case 128:   // typedefs and variables -- N_LSYM
            case 160:   // parameter variable -- N_PSYM 
            case 0xc6:  // position-independant local typedefs -- N_ISYM
            case 0xc8: // position-independant external typedefs -- N_ESYM
#ifdef TIMED_PARSE
               pss_count++;
               gettimeofday(&t1, NULL);
#endif
               if (stabptr->type(i) == N_FUN) current_func = NULL;
               ptr = const_cast<char *>(stabptr->name(i));
               while (ptr[strlen(ptr)-1] == '\\') {
                  //ptr[strlen(ptr)-1] = '\0';
                  ptr2 =  const_cast<char *>(stabptr->name(i+1));
                  ptr3 = (char *) malloc(strlen(ptr) + strlen(ptr2));
                  strcpy(ptr3, ptr);
                  ptr3[strlen(ptr)-1] = '\0';
                  strcat(ptr3, ptr2);

                  ptr = ptr3;
                  i++;
                  // XXX - memory leak on multiple cont. lines
               }

               // bperr("stab #%d = %s\n", i, ptr);
               // may be nothing to parse - XXX  jdd 5/13/99
               if (nativeCompiler)
                  temp = parseStabString(this, mostRecentLinenum, 
                        (char *)ptr, stabptr->val(i), commonBlock);
               else
                  temp = parseStabString(this, stabptr->desc(i), 
                        (char *)ptr, stabptr->val(i), commonBlock);
               if (temp.length()) {
                  //Error parsing the stabstr, return should be \0
                  bperr( "Stab string parsing ERROR!! More to parse: %s\n",
                        temp.c_str());
                  bperr( "  symbol: %s\n", ptr);
               }

#ifdef TIMED_PARSE
               gettimeofday(&t2, NULL);
               pss_dur += (t2.tv_sec - t1.tv_sec)*1000.0 + (t2.tv_usec - t1.tv_usec)/1000.0;
               //      pss_dur += (t2.tv_sec/1000 + t2.tv_usec*1000) - (t1.tv_sec/1000 + t1.tv_usec*1000);
#endif
               break;
            default:
               break;
         }
      }       		    
   }

#if defined(TIMED_PARSE)
   struct timeval endtime;
   gettimeofday(&endtime, NULL);
   unsigned long lstarttime = starttime.tv_sec * 1000 * 1000 + starttime.tv_usec;
   unsigned long lendtime = endtime.tv_sec * 1000 * 1000 + endtime.tv_usec;
   unsigned long difftime = lendtime - lstarttime;
   double dursecs = difftime/(1000 );
   cout << __FILE__ << ":" << __LINE__ <<": parseTypes("<< mod->fileName()
      <<") took "<<dursecs <<" msecs" << endl;
   cout << "Breakdown:" << endl;
   cout << "     Functions: " << fun_count << " took " << fun_dur << "msec" << endl;
   cout << "     Sources: " << src_count << " took " << src_dur << "msec" << endl;
   cout << "     parseStabString: " << pss_count << " took " << pss_dur << "msec" << endl;
   cout << "     Total: " << pss_dur + fun_dur + src_dur 
      << " msec" << endl;
#endif
}

#endif //end of #if defined(i386_unknown_linux2_0)
#endif //if 0

// Parsing symbol table for Alpha platform
// Mehmet

//#if defined(os_windows)
#if 0
typedef struct localsStruct {
   BPatch_function *func;
   Address base;
   HANDLE p;
   map<unsigned, unsigned> foundSyms;
   localsStruct() : foundSyms() {}
} localsStruct;

BOOL CALLBACK enumLocalSymbols(PSYMBOL_INFO pSymInfo, unsigned long symSize,
      void *userContext)
{
   BPatch_type *type;
   BPatch_function *func;
   BPatch_storageClass storage;
   BPatch_localVar *newvar;
   int reg;
   signed long frameOffset;
   Address base;
   HANDLE p;

   char *storageName;
   char *paramType;

   //
   //Skip this variable if it's already been found.
   //
   localsStruct *locals = (localsStruct *) userContext;
   if (locals->foundSyms.find(pSymInfo->Index) != locals->foundSyms.end())
      return true;
   locals->foundSyms[pSymInfo->Index] = 1;
   base = locals->base;
   func = locals->func;
   p = locals->p;

   //Get type
   type = getType(p, base, pSymInfo->TypeIndex, func->getModule());

   //Get variable storage location information
   if ((pSymInfo->Flags & IMAGEHLP_SYMBOL_INFO_FRAMERELATIVE) ||
         ((pSymInfo->Flags & IMAGEHLP_SYMBOL_INFO_REGRELATIVE) && 
          (pSymInfo->Register = CV_REG_EBP)))
   {
      reg = pSymInfo->Register;
      frameOffset = (signed) pSymInfo->Address;
      storage = BPatch_storageFrameOffset;
      storageName = "Frame Relative";
   }
   else if (pSymInfo->Flags & IMAGEHLP_SYMBOL_INFO_REGRELATIVE)
   {
      reg = pSymInfo->Register;
      frameOffset = (signed) pSymInfo->Address;
      storage = BPatch_storageRegOffset;
      storageName = "Register Relative";
   }
   else if (pSymInfo->Flags & IMAGEHLP_SYMBOL_INFO_REGISTER) {
      reg = pSymInfo->Register;
      frameOffset = 0;
      storage = BPatch_storageReg;
      storageName = "Register";
   }
   else {
      reg = 0;
      frameOffset = (signed) pSymInfo->Address;
      storage = BPatch_storageAddr;
      storageName = "Absolute";
   }

   newvar = new BPatch_localVar(pSymInfo->Name, type, -1, frameOffset,
         reg, storage);

   //Store the variable as a local or parameter appropriately
   if (pSymInfo->Flags & IMAGEHLP_SYMBOL_INFO_PARAMETER) {
      func->funcParameters->addLocalVar(newvar);
      paramType = "parameter";
   }
   else if (pSymInfo->Flags & IMAGEHLP_SYMBOL_INFO_LOCAL) {
      func->localVariables->addLocalVar(newvar);
      paramType = "local";
   }
   else {
      fprintf(stderr, "[%s:%u] - Local variable of unknown type.  %s in %s\n",
            __FILE__, __LINE__, pSymInfo->Name, func->lowlevel_func()->prettyName().c_str());
      paramType = "unknown";
   }


   const char *typeName;
   if (type) {
      typeName = type->getName();
   }
   else {
      typeName = "unknown";
   }

   return true;
}


static void enumLocalVars(BPatch_function *func, 
      const pdvector<instPoint *> &points,
      localsStruct *locals) 
{
   IMAGEHLP_STACK_FRAME frame;
   memset(&frame, 0, sizeof(IMAGEHLP_STACK_FRAME));



   for (unsigned i=0; i<points.size(); i++) {
      frame.InstructionOffset = points[i]->addr();
      bool result = SymSetContext(locals->p, &frame, NULL);
      /*if (!result) {            
        fprintf(stderr, "[%s:%u] - Couldn't SymSetContext\n", __FILE__, __LINE__);
        printSysError(GetLastError());
        }*/
      result = SymEnumSymbols(locals->p, 0, NULL, enumLocalSymbols, locals);
      /*if (!result) {
        fprintf(stderr, "[%s:%u] - Couldn't SymEnumSymbols\n", __FILE__, __LINE__);
        printSysError(GetLastError());
        }*/
   }
}

static int variantValue(VARIANT *v) 
{
   switch(v->vt) {    
      case VT_I8:
         return (int) v->llVal;
      case VT_I4:
         return (int) v->lVal;
      case VT_UI1:
         return (int) v->bVal;
      case VT_I2:
         return (int) v->iVal;
      case VT_I1:
         return (int) v->cVal;
      case VT_UI2:
         return (int) v->uiVal;
      case VT_UI4:
         return (int) v->ulVal;
      case VT_UI8:
         return (int) v->ullVal;
      case VT_INT:
         return (int) v->intVal;
      case VT_UINT:
         return (int) v->uintVal;
      default:
         return 0;
   }
}

static void addTypeToCollection(BPatch_type *type, BPatch_module *mod) 
{
   BPatch_typeCollection *collection;
   collection = mod ? mod->getModuleTypes() : BPatch::bpatch->stdTypes;
   assert(collection);
   assert(!collection->findType(type->getID()));
   collection->addType(type);
}

static char *getTypeName(HANDLE p, Address base, int typeIndex) 
{
   int result, length;
   WCHAR *wname = NULL;
   char *name = NULL;

   result = SymGetTypeInfo(p, base, typeIndex, TI_GET_SYMNAME, &wname);
   if (!result) 
      return NULL;

   length = wcslen(wname) + 1;
   name = (char *) malloc(length + 1);
   result = WideCharToMultiByte(CP_ACP, 0, wname, -1, name, length, NULL, NULL);
   LocalFree(wname); 
   if (!result) {
      int lasterror = GetLastError();
      //        printSysError(lasterror);
      return NULL;
   }
   return name;
}

static BPatch_dataClass getDataClass(HANDLE p, Address base, int typeIndex) 
{
   enum SymTagEnum wintype;
   int result, basetype;

   result = SymGetTypeInfo(p, base, typeIndex, TI_GET_SYMTAG, &wintype);
   if (!result)
      return BPatch_dataUnknownType;
   switch (wintype) {
      case SymTagFunction:
      case SymTagFunctionType:
         return BPatch_dataFunction;
      case SymTagPointerType:
         return BPatch_dataPointer;
      case SymTagArrayType:
         return BPatch_dataArray;
      case SymTagBaseType:
         return BPatch_dataScalar;
      case SymTagEnum:
         return BPatch_dataEnumerated;
      case SymTagTypedef:
         return BPatch_dataTypeDefine;
      case SymTagUDT:
         enum UdtKind udtType;
         result = SymGetTypeInfo(p, base, typeIndex, TI_GET_UDTKIND, &udtType);
         if (!result)
            return BPatch_dataUnknownType;
         switch (udtType) {
            case UdtUnion:
               return BPatch_dataUnion;
            case UdtStruct:
            case UdtClass:
               return BPatch_dataStructure;
            default:
               return BPatch_dataUnknownType;
         }
      case SymTagFunctionArgType:
         result = SymGetTypeInfo(p, base, typeIndex, TI_GET_TYPEID, &basetype);
         if (!result)
            return BPatch_dataUnknownType;
         return getDataClass(p, base, basetype);
      default:
         return BPatch_dataUnknownType;
   }
}

static BPatch_type *getEnumType(HANDLE p, Address base, int typeIndex, BPatch_module *mod) 
{
   unsigned i;
   char *name = NULL;
   char *entryName = NULL;
   VARIANT entryValue;
   BPatch_typeEnum *type;
   int result;
   unsigned numEntries, entriesSize;
   TI_FINDCHILDREN_PARAMS *entries = NULL;

   name = getTypeName(p, base, typeIndex);
   type = new BPatch_typeEnum(typeIndex, name);
   addTypeToCollection(type, mod);
   free(name);
   name = NULL;

   //
   //Get the number of entries in this enum, and store them in the entries structure
   //
   result = SymGetTypeInfo(p, base, typeIndex, TI_GET_CHILDRENCOUNT, &numEntries);
   if (!result)
      numEntries = 0;

   if (numEntries) {
      entriesSize = sizeof(TI_FINDCHILDREN_PARAMS) + (numEntries + 1) * sizeof(ULONG);
      entries = (TI_FINDCHILDREN_PARAMS *) malloc(entriesSize);
      memset(entries, 0, entriesSize);
      entries->Count = numEntries;
      result = SymGetTypeInfo(p, base, typeIndex, TI_FINDCHILDREN, entries);
      if (!result)
         numEntries = 0;
   }

   for (i=0; i<numEntries; i++) {
      entryName = getTypeName(p, base, entries->ChildId[i]);
      VariantInit(&entryValue);
      result = SymGetTypeInfo(p, base, entries->ChildId[i], TI_GET_VALUE, &entryValue);
      if (!result)
         continue;
      type->addField(entryName, variantValue(&entryValue));
   }

   if (entries)
      free(entries);
   return type;    
}

static BPatch_type *getPointerType(HANDLE p, Address base, int typeIndex, BPatch_module *mod) {
   int baseTypeIndex, result;
   BPatch_type *baseType;
   BPatch_typePointer *newType;

   result = SymGetTypeInfo(p, base, typeIndex, TI_GET_TYPEID, &baseTypeIndex);
   if (!result) {
      fprintf(stderr, "[%s:%u] - TI_GET_TYPEID failed\n", __FILE__, __LINE__);
      return NULL;
   }

   //
   // Add a place-holder for the pointer type first and fill in it's 
   //  base type latter.  This prevents recursion that may happen beneath 
   //  the getType function call below.
   //
   newType = new BPatch_typePointer(typeIndex, NULL);
   addTypeToCollection(newType, mod);

   baseType = getType(p, base, baseTypeIndex);
   if (!baseType) {
      fprintf(stderr, "[%s:%u] - getType failed\n", __FILE__, __LINE__);
      return NULL;
   }

   newType->setPtr(baseType);
   return newType;
}

static BPatch_type *getArrayType(HANDLE p, Address base, int typeIndex, BPatch_module *mod) 
{
   int result, baseIndex, index;
   BPatch_type *indexType, *newType, *baseType;
   unsigned size, num_elements;
   ULONG64 size64;
   const char *bname;
   char *name;

   //Get the index type (usually an int of some kind).  Currently not used.
   result = SymGetTypeInfo(p, base, typeIndex, TI_GET_ARRAYINDEXTYPEID, &index);
   if (!result) {
      fprintf(stderr, "[%s:%u] - TI_GET_ARRAYINDEXTYPEID failed\n", 
            __FILE__, __LINE__);
      return NULL;
   }
   indexType = getType(p, base, index, mod);

   //Get the base type (the type of the elements in the array)
   result = SymGetTypeInfo(p, base, typeIndex, TI_GET_TYPEID, &baseIndex);
   if (!result) {
      fprintf(stderr, "[%s:%u] - TI_GET_TYPEID failed\n", __FILE__, __LINE__);
      return NULL;
   }
   baseType = getType(p, base, baseIndex, mod);

   bname = baseType->getName();
   name = (char *) malloc(strlen(bname) + 4);
   strcpy(name, bname);
   strcat(name, "[]");

   result = SymGetTypeInfo(p, base, typeIndex, TI_GET_LENGTH, &size64);
   if (!result) {
      num_elements = 0;
   }
   else {
      size = (unsigned) size64;
      num_elements = size / baseType->getSize();
   }

   newType = new BPatch_typeArray(typeIndex, baseType, 0, num_elements-1, name);
   newType->getSize();
   addTypeToCollection(newType, mod);
   assert(newType->getID() == typeIndex);

   if (name)
      free(name);
   return newType;
}


static BPatch_type *getTypedefType(HANDLE p, Address base, int typeIndex, BPatch_module *mod) {
   int result, baseTypeIndex;
   BPatch_type *baseType, *newType;
   char *name;

   result = SymGetTypeInfo(p, base, typeIndex, TI_GET_TYPEID, &baseTypeIndex);
   if (!result) {
      fprintf(stderr, "[%s:%u] - TI_GET_TYPEID failed\n", __FILE__, __LINE__);
      return NULL;
   }
   baseType = getType(p, base, baseTypeIndex, mod);
   if (!baseType) {
      return NULL;
   }

   name = getTypeName(p, base, typeIndex);

   newType = new BPatch_typeTypedef(typeIndex, baseType, name);
   addTypeToCollection(newType, mod);
   return newType;
}

static BPatch_type *getUDTType(HANDLE p, Address base, int typeIndex, BPatch_module *mod) {
   int result, symtag;
   unsigned size, numChildren, childrenSize, child_offset, i, child_size;
   BPatch_fieldListType *newType;
   UINT64 size64;
   const char *name, *childName;
   enum UdtKind udtType;
   TI_FINDCHILDREN_PARAMS *children = NULL;
   BPatch_dataClass dataType;

   //
   // Get name for structure
   //
   name = getTypeName(p, base, typeIndex);
   result = SymGetTypeInfo(p, base, typeIndex, TI_GET_LENGTH, &size64);
   if (!result) {
      fprintf(stderr, "[%s:%u] - TI_GET_LENGTH return error\n");
      return NULL;
   }
   size = (unsigned) size64;

   //
   // Determine whether it's a class, struct, or union and create the 
   //  new_type appropriately
   //
   result = SymGetTypeInfo(p, base, typeIndex, TI_GET_UDTKIND, &udtType);
   if (!result) {
      fprintf(stderr, "[%s:%u] - TI_GET_UDTKIND returned error\n");
      return NULL;
   }
   switch (udtType) {
      case UdtUnion:
         newType = new BPatch_typeUnion(typeIndex, name);
         break;
      case UdtStruct:
      case UdtClass:
      default:
         newType = new BPatch_typeStruct(typeIndex, name);
         break;
   }
   addTypeToCollection(newType, mod);
   if (name)
      free((void *) name);
   name = NULL;


   //
   // Store the number of member variables/functions/stuff in numChildren
   //
   result = SymGetTypeInfo(p, base, typeIndex, TI_GET_CHILDRENCOUNT, &numChildren);
   if (!result)
      numChildren = 0;
   //
   // Get the list of variables/functions/stuff
   //
   if (numChildren) {
      childrenSize = sizeof(TI_FINDCHILDREN_PARAMS) + (numChildren + 1) * sizeof(ULONG);
      children = (TI_FINDCHILDREN_PARAMS *) malloc(childrenSize);
      memset(children, 0, childrenSize);
      children->Count = numChildren;
      result = SymGetTypeInfo(p, base, typeIndex, TI_FINDCHILDREN, children);
      if (!result)
         numChildren = 0;
   }

   //
   // Create/Find the type of each child and add it to newType appropriately
   //
   for (i=0; i<numChildren; i++) {
      // Create/Get child type
      BPatch_type *child_type = getType(p, base, children->ChildId[i], mod);
      if (!child_type)
         continue;

      // Figure out a name of this object
      childName = NULL;
      result = SymGetTypeInfo(p, base, children->ChildId[i], TI_GET_SYMTAG, &symtag);
      if (result && symtag == SymTagBaseClass) {
         childName = strdup("{superclass}");
      }
      if (!childName)
         childName = getTypeName(p, base, children->ChildId[i]);
      if (!childName) 
         childName = strdup(child_type->getName());

      // Find the offset of this member in the structure
      result = SymGetTypeInfo(p, base, children->ChildId[i], TI_GET_OFFSET, &child_offset);
      if (!result) {
         child_offset = 0; //Probably a member function
         child_size = 0;
      }
      else {
         child_offset *= 8; //Internally measured in bits
         child_size = child_type->getSize();
      }

      dataType = getDataClass(p, base, child_type->getID());
      newType->addField(childName, dataType, child_type, child_offset, child_size);
      if (childName)
         free((void *) childName);
      childName = NULL;
   }

   if (children)
      free(children);

   return newType;
}

static BPatch_type *getLayeredType(HANDLE p, Address base, int typeIndex, BPatch_module *mod) {
   int result, newTypeIndex;
   BPatch_type *newType;

   result = SymGetTypeInfo(p, base, typeIndex, TI_GET_TYPEID, &newTypeIndex);
   if (!result) {
      fprintf(stderr, "TI_GET_TYPEID failed\n");
      return NULL;
   }

   newType = getType(p, base, newTypeIndex, mod);
   return newType;
}

static BPatch_type *getFunctionType(HANDLE p, Address base, int typeIndex, BPatch_module *mod) {
   int result, retTypeIndex;
   BPatch_typeFunction *newType;
   BPatch_type *retType;
   unsigned num_params, args_size, i;
   vector<BPatch_type *> params;
   TI_FINDCHILDREN_PARAMS *args = NULL;
   string name;
   char *param_name;
   BPatch_dataClass dataType;

   //Create the function early to avoid recursive references
   newType = new BPatch_typeFunction(typeIndex, NULL, NULL);
   addTypeToCollection(newType, mod);

   result = SymGetTypeInfo(p, base, typeIndex, TI_GET_TYPEID, &retTypeIndex);
   if (!result) {
      fprintf(stderr, "[%s:%u] - Couldn't TI_GET_TYPEID\n", __FILE__, __LINE__);
      return NULL;
   }

   retType = getType(p, base, retTypeIndex, mod);
   if (!retType) {
      return NULL;
   }
   newType->setRetType(retType);

   result = SymGetTypeInfo(p, base, typeIndex, TI_GET_COUNT, &num_params);
   if (!result)
      goto done_params;

   args_size = sizeof(TI_FINDCHILDREN_PARAMS) + (num_params + 1) * sizeof(ULONG);
   args = (TI_FINDCHILDREN_PARAMS *) malloc(args_size);
   memset(args, 0, args_size);
   args->Count = num_params;
   result = SymGetTypeInfo(p, base, typeIndex, TI_FINDCHILDREN, args);
   if (!result)
      goto done_params;

   for (i=0; i<num_params; i++) {
      BPatch_type *arg_type = getType(p, base, args->ChildId[i], mod);
      if (!arg_type) {
         continue;
      }
      params.push_back(arg_type);
   }

done_params:

   //
   // Build a type name that looks like the following:
   //   (return_type)(param1_type, param2_type, ...)
   name = "(";
   name += retType->getName();
   name += ")(";
   for (i=0; i<params.size(); i++) {
      if (i != 0)
         name += ", ";
      name += params[i]->getName();
   }
   name += ")";

   newType->setName(name.c_str());

   for (i=0; i<params.size(); i++) {
      dataType = getDataClass(p, base, params[i]->getID());
      param_name = getTypeName(p, base, params[i]->getID());
      if (!param_name)
         param_name = strdup("parameter");
      newType->addField(param_name, dataType, params[i], i, params[i]->getSize());
      if (param_name)
         free(param_name);
   }

   if (args)
      free(args);

   return newType;
}

static BPatch_type *getBaseType(HANDLE p, Address base, int typeIndex, BPatch_module *mod) {
   BasicType baseType;
   int result;
   ULONG64 size64;
   unsigned size;
   BPatch_type *newType;

   result = SymGetTypeInfo(p, base, typeIndex, TI_GET_BASETYPE, &baseType);
   if (!result) {
      fprintf(stderr, "[%s:%u] - TI_GET_BASETYPE return error\n");
      return NULL;
   }

   result = SymGetTypeInfo(p, base, typeIndex, TI_GET_LENGTH, &size64);
   if (!result) {
      fprintf(stderr, "[%s:%u] - TI_GET_LENGTH return error\n");
      return NULL;
   }
   size = (unsigned) size64;
   switch(baseType) {
      case btNoType:
         newType = NULL;
         break;
      case btVoid:
         newType = new BPatch_typeScalar(typeIndex, size, "void");
         break;
      case btChar:
         newType = new BPatch_typeScalar(typeIndex, size, "char");
         break;
      case btWChar:
         newType = new BPatch_typeScalar(typeIndex, size, "wchar");
         break;
      case btInt:
         if (size == 8)
            newType = new BPatch_typeScalar(typeIndex, size, "long long int");
         else if (size == 4)
            newType = new BPatch_typeScalar(typeIndex, size, "int");
         else if (size == 2)
            newType = new BPatch_typeScalar(typeIndex, size, "short");
         else if (size == 1)
            newType = new BPatch_typeScalar(typeIndex, size, "char");
         else
            newType = new BPatch_typeScalar(typeIndex, size, "");
         break;
      case btUInt:
         if (size == 8)
            newType = new BPatch_typeScalar(typeIndex, size, "unsigned long long int");
         else if (size == 4)
            newType = new BPatch_typeScalar(typeIndex, size, "unsigned int");
         else if (size == 2)
            newType = new BPatch_typeScalar(typeIndex, size, "unsigned short");
         else if (size == 1)
            newType = new BPatch_typeScalar(typeIndex, size, "unsigned char");
         else
            newType = new BPatch_typeScalar(typeIndex, size, "");
         break;
      case btFloat:
         if (size == 8)
            newType = new BPatch_typeScalar(typeIndex, size, "double");
         else
            newType = new BPatch_typeScalar(typeIndex, size, "float");
         break;
      case btBCD:
         newType = new BPatch_typeScalar(typeIndex, size, "BCD");
         break;
      case btBool:
         newType = new BPatch_typeScalar(typeIndex, size, "bool");
         break;
      case btLong:
         newType = new BPatch_typeScalar(typeIndex, size, "long");
         break;
      case btULong:
         newType = new BPatch_typeScalar(typeIndex, size, "unsigned long");
         break;
      case btCurrency:
         newType = new BPatch_typeScalar(typeIndex, size, "currency");
         break;
      case btDate:
         newType = new BPatch_typeScalar(typeIndex, size, "Date");
         break;
      case btVariant:
         newType = new BPatch_typeScalar(typeIndex, size, "variant");
         break;
      case btComplex:
         newType = new BPatch_typeScalar(typeIndex, size, "complex");
         break;
      case btBit:
         newType = new BPatch_typeScalar(typeIndex, size, "bit");
         break;
      case btBSTR:
         newType = new BPatch_typeScalar(typeIndex, size, "bstr");
         break;
      case btHresult:
         newType = new BPatch_typeScalar(typeIndex, size, "Hresult");
         break;
      default:
         fprintf(stderr, "Couldn't parse baseType %d for %d\n", baseType, typeIndex);
         assert(0);
         break;
   }
   if (newType)
      addTypeToCollection(newType, mod);
   return newType;
}

static BPatch_type *getType(HANDLE p, Address base, int typeIndex, BPatch_module *mod) 
{
   static unsigned depth = 0;
   BOOL result;
   BPatch_type *foundType = NULL;
   BPatch_typeCollection *collection;
   enum SymTagEnum symtag;

   if (!typeIndex)
      return NULL;

   //
   // Check if this type has already been created (they're indexed by typeIndex).
   // If it has, go ahead and return the existing one.
   // If not, then start creating a new type.
   //
   if (mod)
      collection = mod->getModuleTypes();
   else
      collection = BPatch::bpatch->stdTypes;
   assert(collection);


   //
   // Check to see if we've already parsed this type
   //
   foundType = collection->findType(typeIndex);
   if (foundType) {
      return foundType;
   }

   //
   // Types on Windows are stored as part of a special type of symbol.  TI_GET_SYMTAG 
   // Gets the meta information about the type.
   //
   result = SymGetTypeInfo(p, base, typeIndex, TI_GET_SYMTAG, &symtag);
   if (!result) {
      depth--;
      return NULL;
   }
   switch (symtag) {
      case SymTagBaseType:
         foundType = getBaseType(p, base, typeIndex, mod);
         break;
      case SymTagEnum:
         foundType = getEnumType(p, base, typeIndex, mod);
         break;
      case SymTagFunctionType:
         foundType = getFunctionType(p, base, typeIndex, mod);
         break;
      case SymTagPointerType:
         foundType = getPointerType(p, base, typeIndex, mod);
         break;
      case SymTagArrayType:
         foundType = getArrayType(p, base, typeIndex, mod);
         break;
      case SymTagTypedef:
         foundType = getTypedefType(p, base, typeIndex, mod);
         break;
      case SymTagUDT:
         foundType = getUDTType(p, base, typeIndex, mod);
         break;
      case SymTagFunctionArgType:
      case SymTagData:
      case SymTagFunction:
      case SymTagBaseClass:
         foundType = getLayeredType(p, base, typeIndex, mod);
         if (foundType)
            typeIndex = foundType->getID();
         break;
      case SymTagThunk:
         foundType = NULL;
         break;
      case SymTagVTableShape:
      case SymTagVTable:
         break;
      default:
         fprintf(stderr, "Unknown type %d\n", symtag);
         assert(0);
         foundType = NULL;
         break;
   }

   return foundType;
}

typedef struct proc_mod_pair {
   process *proc;
   BPatch_module *module;
   mapped_module *mmod;
   Address base_addr;
} proc_mod_pair;

static void findLocalVars(BPatch_function *func, Address base) {
   BPatch_module *mod = func->getModule();
   int_function *ifunc = func->lowlevel_func();
   localsStruct locals;
   BPatch_process *proc = func->getProc();
   HANDLE p = proc->lowlevel_process()->processHandle_;

   locals.func = func;
   locals.base = base;
   locals.p = p;

   //
   // The windows debugging interface allows us to get local variables
   // at specific points, which makes it hard to enumerate all locals (as we want).
   // Instead we'll get the local variables at the most common points below.
   //
   const pdvector<instPoint*> &points = ifunc->funcEntries();
   enumLocalVars(func, ifunc->funcEntries(), &locals);
   enumLocalVars(func, ifunc->funcExits(), &locals);
   enumLocalVars(func, ifunc->funcCalls(), &locals);
   enumLocalVars(func, ifunc->funcArbitraryPoints(), &locals);
}

#define SymTagFunction 0x5
#define SymTagData 0x7
#define SymTagPublicSymbol 0xa
#define SymTagMisc 0x3808 		// Seen with NB11, VC++6-produced executables

//
// Our recognition of interesting symbols (functions and global data)
// is complicated due to lack of consistency in how they are
// presented to us in the pSymInfo struct.  For example,
// Microsoft's own system DLLs like kernel32.dll only seem to provide
// us their exports - these have the SYMFLAG_EXPORT bit set in
// pSymInfo->Flags.  In contrast, EXEs with full debug information
// may have pSymInfo->Flags == 0, with pSymInfo->Tag indicating the
// type of symbol.
//
static BOOL isGlobalSymbol(PSYMBOL_INFO pSymInfo) {
   return ((pSymInfo->Flags & SYMFLAG_EXPORT) ||
         (pSymInfo->Flags & SYMFLAG_FUNCTION) ||
         ((!pSymInfo->Flags) && 
          ((pSymInfo->Tag == SymTagFunction) ||
           (pSymInfo->Tag == SymTagData) ||
           (pSymInfo->Tag == SymTagPublicSymbol) ||
           (pSymInfo->Tag == SymTagMisc))) );
}

BOOL CALLBACK add_type_info(PSYMBOL_INFO pSymInfo, ULONG SymbolSize, void *info)
{
   HANDLE p;
   Address mod_base;
   proc_mod_pair *pair;
   mapped_module *map_mod;
   BPatch_module *mod;
   BPatch_type *type;
   char *name;
   BPatch_typeCollection *collection;

   if (!isGlobalSymbol(pSymInfo)) {
      //We do local symbols elsewhere
      return TRUE;
   }

   pair = (proc_mod_pair *) info;
   p = pair->proc->processHandle_;
   mod_base = pair->base_addr;
   mod = pair->module;
   map_mod = pair->mmod;
   name = pSymInfo->Name;

   if (map_mod->obj()->parse_img()->isAOut()) {
      //When parsing the a.out, sort the type information into specific modules.  This doesn't matter
      // for libraries, because there is a 1:1 mapping between modules and objects.
      //
      //A module is a collection of functions, but doesn't include global data types.  Global variables
      // will go into the DEFAULT_MODULE
      int_function *f = map_mod->obj()->findFuncByAddr((Address) pSymInfo->Address);
      if (!f) {
         //No containing module.  Only insert this into DEFAULT_MODULE
         if (strcmp(map_mod->fileName().c_str(), "DEFAULT_MODULE"))
            return true;
      }
      else if (f->mod() != map_mod) {
         //This is a variable for another module.
         return true;
      }
   }

   type = getType(p, mod_base, pSymInfo->TypeIndex, mod);
   collection = mod->getModuleTypes();


   /*
      fprintf(stderr, "[%s:%u] - Variable %s had type %s\n", __FILE__, __LINE__,
      name, type ? type->getName() : "{NO TYPE}");
    */
   if (type && name)
      collection->addGlobalVariable(name, type);

   return TRUE;
}
#endif

void BPatch_module::parseTypes() 
{
   //TODO?? Is this the only change required for windows to work??
   mod->pmod()->mod()->exec()->parseTypesNow();

#if 0
   proc_mod_pair pair;
   BOOL result;
   //
   //Parse global variable type information
   //

   assert(addSpace->getType() == TRADITIONAL_PROCESS);
   BPatch_process *proc  = dynamic_cast<BPatch_process *>(addSpace);

   pair.proc = proc->lowlevel_process();
   pair.module = this;
   pair.base_addr = mod->obj()->getBaseAddress();
   pair.mmod = mod;

   if (!pair.base_addr) {
      pair.base_addr = mod->obj()->getFileDesc().loadAddr();
   }

   result = SymEnumSymbols(pair.proc->processHandle_, pair.base_addr, NULL, 
         add_type_info, &pair);
   if (!result) {
      parsing_printf("SymEnumSymbols was unsuccessful.  Type info may be incomplete\n");
   }

   //
   // Parse local variables and local type information
   //
   BPatch_Vector<BPatch_function *> *funcs;
   funcs = getProcedures();
   for (unsigned i=0; i < funcs->size(); i++) {
      findLocalVars((*funcs)[i], pair.base_addr);
   }
#endif
}

bool BPatch_module::getVariablesInt(BPatch_Vector<BPatch_variableExpr *> &vars)
{
   if (!isValid()) return false;

   BPatch_variableExpr *var;
   parseTypesIfNecessary();

   pdvector<std::string> keys = moduleTypes->globalVarsByName.keys();
   int limit = keys.size();
   for (int j = 0; j < limit; j++) {
      std::string name = keys[j];
      var = img->createVarExprByName(this, name.c_str());
      if (var != NULL)
         vars.push_back(var);
   }

   if (limit) 
      return true;

   // We may not have top-level (debugging derived) variable info.
   // If not, go into the low-level code.
   const pdvector<int_variable *> &allVars = mod->getAllVariables();

   for (unsigned i = 0; i < allVars.size(); i++) {
      BPatch_variableExpr *var = img->createVarExprByName(this, 
            allVars[i]->symTabName().c_str());
      if (var)
         vars.push_back(var);
   }

   if (vars.size())
      return true;

#ifdef IBM_BPATCH_COMPAT
   //  IBM getVariables can be successful while returning an empty set of vars
   return true;
#else
   return false;
#endif
}

/* This function should be deprecated. */
bool BPatch_module::getLineToAddrInt( unsigned int lineNo, 
      BPatch_Vector< unsigned long > & buffer, bool ) 
{
   if (!isValid()) {
      fprintf(stderr, "%s[%d]: module is not valid\n", FILE__, __LINE__);
      return false;
   }

   std::vector< std::pair< Address, Address > > ranges;
   if ( ! getAddressRangesInt( NULL, lineNo, ranges ) ) { 
      fprintf(stderr, "%s[%d]:  getAddressRanges failed!\n", FILE__, __LINE__);
      return false; 
   }

   for ( unsigned int i = 0; i < ranges.size(); ++i ) {
      buffer.push_back( ranges[i].first + mod->obj()->codeBase());
   }

   return true;
} /* end getLineToAddr() */

bool BPatch_module::getSourceLinesInt(unsigned long addr, 
      BPatch_Vector< BPatch_statement> &lines) 
{
   if (!isValid()) return false;

   unsigned int originalSize = lines.size();
   LineInformation *li = mod->getLineInformation();
   std::vector<LineInformationImpl::LineNoTuple> lines_ll;
   if (li) {
      li->getSourceLines( addr - mod->obj()->codeBase(), lines_ll );
   }
   else {
      return false;
   }

   for (unsigned int j = 0; j < lines_ll.size(); ++j) {
      LineInformationImpl::LineNoTuple &t = lines_ll[j];
      lines.push_back(BPatch_statement(this, t.first, 
               t.second, t.column));
   }

   return (lines.size() != originalSize);
} /* end getSourceLines() */

bool BPatch_module::getStatementsInt(BPatch_Vector<BPatch_statement> &statements)
{
   // Iterate over each address range in the line information
   LineInformation *li = mod->getLineInformation();
   if (!li) {
      string str = mod->fileName();
      fprintf(stderr, "%s[%d]:  getStatements() failing for module %s\n", FILE__, __LINE__,str.c_str()); 
      fprintf(stderr, " \tfailed to get lineInformation\n" );

      //  Here we have a conundrum...  we used to return a ptr to an empty
      //  LineInformation object -- now we do not keep that object around unless
      //  there's something in it.
      //
      //  This case would lead to a true return value for an empty set of statements.
      //  This is probably not correct, but I'm setting it regardless.

      //return false;
      return true;
   }

   for (LineInformation::const_iterator i = li->begin();
         i != li->end();
         ++i) {

      // Form a BPatch_statement object for this entry
      // Note:  Line information stores offsets, so we need to adjust to
      //  addresses
      BPatch_statement statement(this, i->second.first, i->second.second,
            i->second.column, 
            (void *)(i->first.first + mod->obj()->codeBase()), 
            (void *)(i->first.second + mod->obj()->codeBase()));

      // Add this statement
      statements.push_back(statement);

   }
   return true;
}

bool BPatch_module::getAddressRangesInt( const char * fileName, 
      unsigned int lineNo, std::vector< std::pair< Address, Address > > & ranges ) 
{
   unsigned int starting_size = ranges.size();

   if (!isValid()) {
      fprintf(stderr, "%s[%d]:  module is not valid\n", FILE__, __LINE__);
      return false;
   }

   LineInformation *li = mod->getLineInformation();
   if ( fileName == NULL ) {
      fileName = mod->fileName().c_str(); 
   }
   if (li) {
      bool ok = li->getAddressRanges( fileName, lineNo, ranges );
      if (ok) {
         //  Iterate over the returned offset ranges to turn them into addresses
         for (unsigned int i = starting_size; i < ranges.size(); ++i) {
            ranges[i].first += mod->obj()->codeBase();
            ranges[i].second += mod->obj()->codeBase();
         }
      }
      else {
         //fprintf(stderr, "%s[%d]:  failed to get address ranges for %s:%d, lineInfo %p, lowlevel module = %p\n", FILE__, __LINE__, fileName, lineNo, li, mod->pmod());
      }
      return ok;
   }
   else {
      return false;
   }
} /* end getAddressRanges() */

bool BPatch_module::isSharedLibInt() 
{
   return mod->obj()->isSharedLib();
}

/*
 * BPatch_module::getBaseAddr
 *
 * Returns the starting address of the module.
 */
void *BPatch_module::getBaseAddrInt()
{
   return (void *)mod->obj()->codeAbs();
}

/*
 * BPatch_module::getSize
 *
 * Returns the size of the module in bytes.
 */
unsigned long BPatch_module::getSizeInt() 
{
   if (!mod) return 0;
   return (unsigned long) mod->obj()->imageSize();
}


bool BPatch_module::isNativeCompilerInt()
{
   return nativeCompiler;
}

size_t BPatch_module::getAddressWidthInt()
{
   if (!mod) return 0;
   return mod->obj()->parse_img()->getObject()->getAddressWidth();
}

void BPatch_module::setDefaultNamespacePrefix(char *name) 
{ 
   img->setDefaultNamespacePrefix(name); 
}

bool BPatch_module::isSystemLib() 
{
   if (!mod) return false;
   string str = mod->fileName();

   // Solaris 2.8... we don't grab the initial func always,
   // so fix up this code as well...
#if defined(os_solaris)
   if (strstr(str.c_str(), "libthread"))
      return true;
#endif
#if defined(os_linux)
   if (strstr(str.c_str(), "libc.so"))
      return true;
   if (strstr(str.c_str(), "libpthread"))
      return true;
#endif

   if (strstr(str.c_str(), "libdyninstAPI_RT"))
      return true;
#if defined(os_windows)
   if (strstr(str.c_str(), "kernel32.dll"))
      return true;
   if (strstr(str.c_str(), "user32.dll"))
      return true;
   if (strstr(str.c_str(), "ntdll.dll"))
      return true;
#endif

   return false;
}


// for use with VECTOR_SORT in getUnresolvedControlFlow
bool callTargetCompare(image_instPoint *pt1, image_instPoint *pt2) 
{ 
   return pt1->callTarget() < pt2->callTarget(); 
}

/* Build up a vector of control flow instructions with suspicious targets
 * 1. Gather all instructions with static targets that leave the low-level
 *    mapped_object's image and check to see if they jump into another
 *    existing module
 * 2. Add all dynamic call instructions to the vector
 * 3. Add all dynamic jump instructions to the vector
 */
BPatch_Vector<BPatch_point *> *BPatch_module::getUnresolvedControlFlowInt()
{
   if (unresolvedCF.size()) {
      return &unresolvedCF;
   }

   pdvector<image_instPoint*> ctrlTransfers 
      = lowlevel_mod()->obj()->parse_img()->getBadControlFlow();
   VECTOR_SORT(ctrlTransfers, callTargetCompare);

   Address prevAddr = 0;

   for (unsigned int i=0; i < ctrlTransfers.size(); i++) {
      image_instPoint *curPoint = ctrlTransfers[i];
      Address curAddr = curPoint->offset() 
         + curPoint->func()->img()->desc().loadAddr();

      // eliminate dups from ctrlTransfers
      if (prevAddr != curAddr) { 
         prevAddr = curAddr;

         // if this is a static CT skip if target leads to known code
         if ( ! curPoint->isDynamicCall() &&
               (addSpace->getAS()->findOrigByAddr(curPoint->callTarget()) ||
                addSpace->getAS()->findModByAddr(curPoint->callTarget()))) {
            continue;
         }

         // find or create BPatch_point from image_instPoint
         BPatch_point *bpPoint = NULL;
         BPatch_function *func = img->findFunctionInt
            ((unsigned long)curAddr);

         if (func) {
            instPoint *intPt = func->lowlevel_func()
               ->findInstPByAddr(curAddr);

            if (!intPt) {// instPoint does not exist, create it
               intPt = instPoint::createParsePoint(func->lowlevel_func(), curPoint);
            }

            if (!intPt) {
               fprintf(stderr,"%s[%d] Could not find instPoint corresponding to "
                     "unresolved control transfer at %lx\n",
                     __FILE__,__LINE__, (long)curAddr);
               continue;
            }

            if (intPt->getPointType() == callSite) {
               bpPoint = addSpace->findOrCreateBPPoint(func, intPt, BPatch_locSubroutine);
            } else {
               bpPoint = addSpace->findOrCreateBPPoint(func, intPt, BPatch_locLongJump);
            }

         } else {
            fprintf(stderr,"%s[%d] Could not find function corresponding to "
                  "unresolved control transfer at %lx\n",
                  __FILE__,__LINE__, (long)curAddr);
         } 

         // add BPatch_point to vector of unresolved control transfers
         if (bpPoint == NULL) {
            fprintf(stderr,"%s[%d] Control transfer at %lx not instrumentable\n",
                  __FILE__,__LINE__, (long)curAddr);
         } else {
            unresolvedCF.push_back(bpPoint);
         }
      }
   }

   return &unresolvedCF;
}


#ifdef IBM_BPATCH_COMPAT

bool BPatch_module::getLineNumbersInt( unsigned int & startLine, unsigned int & endLine )
{
   /* I don't think this function has ever returned nonzeroes.  Approximate a better 
      result by with the line numbers for the first and last addresses in the module. */
   if (!mod) return false;

   void * startAddr, * endAddr;
   if( ! getAddressRangeInt( startAddr, endAddr ) ) {
      return false;
   }

   bool setAValue = false;
   BPatch_Vector<BPatch_statement> lines;
   getSourceLines( (Address)startAddr, lines );
   if( lines.size() != 0 ) {
      startLine = lines[0].lineNumber();
      setAValue = true;
   }

   lines.clear();
   getSourceLines( (Address)endAddr, lines );
   if( lines.size() != 0 ) {
      endLine = lines[0].lineNumber();
      setAValue = true;
   }

   return setAValue;
}

bool BPatch_module::getAddressRangeInt(void * &start, void * &end)
{
   // Code? Data? We'll do code for now...
   if (!mod) return false;
   start = (void *)(mod->obj()->codeAbs());
   end = (void *)(mod->obj()->codeAbs() + mod->obj()->imageSize());
   return true;
}
char *BPatch_module::getUniqueStringInt(char *buffer, int length)
{
#if 0
   // Old version, did not allow sharing even if the file was the same
   // Get the start and end address of this module
   void* start = NULL;
   void* end = NULL;
   getAddressRange(start, end);

   // Form unique name from module name and start address
   int written = snprintf(buffer, length, "%p|%s",
         start, mod->fileName().c_str());
   assert((written >= 0) && (written < length));

   // Return the unique name to the caller
   return buffer;
#endif
   // Use "<program_name>|<module_name>" as the unique name if this module is
   // part of the executable and "<module_name>" if it is not.
   if (!mod) return NULL;
   if(isSharedLib())
      snprintf(buffer, length, "%s", mod->fileName().c_str());
   else {
      char prog[1024];
      addSpace->getImage()->getProgramFileName(prog, 1024);
      snprintf(buffer, length, "%s|%s",
            prog, mod->fileName().c_str());
   }
   // Return the unique name to the caller
   return buffer;
}

int BPatch_module::getSharedLibTypeInt()	
{
   return 0;
}

int BPatch_module::getBindingTypeInt()
{
   return 0;
}

std::vector<struct BPatch_module::Statement> BPatch_module::getStatementsInt()
{
   std::vector<struct BPatch_module::Statement> statements;
   if (!mod) return statements;

   LineInformation *li = mod->getLineInformation();
   if (!li)
      return statements;

   // Iterate over each address range in the line information
   for (LineInformation::const_iterator i = li->begin(); i != li->end(); ++i) {
      // Form a Statement object for this entry
      struct BPatch_module::Statement statement;
      statement.begin = i->first.first + mod->obj()->codeBase();
      statement.end = i->first.second + mod->obj()->codeBase();
      statement.path = i->second.first;
      statement.line = i->second.second;
      statement.column = 0;

      // Add this statement
      statements.push_back(statement);

   }

   // Return the statements to the caller
   return statements;
}
#endif
