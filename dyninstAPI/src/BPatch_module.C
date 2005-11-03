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

#define BPATCH_FILE

#include "process.h"
#include "showerror.h"
#include "BPatch.h"
#include "BPatch_module.h"
#include "BPatch_libInfo.h"
#include "BPatch_function.h"
#include "BPatch_collections.h"
#include "common/h/String.h"
#include "BPatch_typePrivate.h"    // For BPatch_type related stuff
#include "BPatch_Vector.h"

#include "mapped_module.h"
#include "mapped_object.h"
#include "symtab.h"

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

//char * BPatch_module::current_func_name = NULL;
//char * BPatch_module::current_mangled_func_name = NULL;
//BPatch_function * BPatch_module::current_func = NULL;
char * current_func_name = NULL;
char * current_mangled_func_name = NULL;
BPatch_function *current_func = NULL;


/*
 * BPatch_module::getSourceObj()
 *
 * Return the contained source objects (e.g. functions).
 *
 */
bool BPatch_module::getSourceObj(BPatch_Vector<BPatch_sourceObj *> &vect)
{
    BPatch_Vector<BPatch_function *> *temp;
    temp = getProcedures();
    if (temp) {
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
    pdstring str = mod->fileName();

    strncpy(buffer, str.c_str(), length);

    return buffer;
}

const char *BPatch_module::libraryNameInt()
{
   if (isSharedLib())      
      return mod->fullName().c_str();
   return NULL;
}

char *BPatch_module::getFullNameInt(char *buffer, int length)
{
    pdstring str = mod->fullName();

    strncpy(buffer, str.c_str(), length);

    return buffer;
}


BPatch_module::BPatch_module( BPatch_process *_proc, mapped_module *_mod,
                              BPatch_image *_img ) :
   proc( _proc ), mod( _mod ), img( _img ), 
   moduleTypes(NULL)
{
   unsigned i;
#if defined(TIMED_PARSE)
    struct timeval starttime;
    gettimeofday(&starttime, NULL);
#endif
    
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
    } /* end language switch */
#if 0    
    if (!BPatch::bpatch->delayedParsingOn()) {
        
        const pdvector< int_function * >  &functions = mod->getAllFunctions();
        for( i = 0; i < functions.size(); i++ ) {
            /* The bpfs for a shared object module won't have been built by now,
               but generating them on the fly is OK because each .so is a single module
               for our purposes. */
            int_function * function = functions[i];
            if(!proc->func_map->defines( function )) {
                // We're not delaying and there's no function. Make one.
                // Don't assign the module owner yet; causes recursion into
                // DWARF parsing in some situations, and the module will 
                // pick it up later.
                BPatch_function *bpf = new BPatch_function(proc, function, this);
                assert( bpf != NULL );
            }
            else {
                assert(proc->func_map->get(functions[i])->getModule() == this);
            }
        }
    }
#endif

#if defined(TIMED_PARSE)
    struct timeval endtime;
    gettimeofday(&endtime, NULL);
    unsigned long lstarttime = starttime.tv_sec * 1000 * 1000 + starttime.tv_usec;
    unsigned long lendtime = endtime.tv_sec * 1000 * 1000 + endtime.tv_usec;
    unsigned long difftime = lendtime - lstarttime;
    double dursecs = difftime/(1000 );
    cout << __FILE__ << ":" << __LINE__ <<": BPatch_module("<< mod->fileName()
	 <<") took "<<dursecs <<" msecs" << endl;
#endif
} /* end BPatch_module() */

BPatch_module::~BPatch_module()
{
    if (moduleTypes) {
        BPatch_typeCollection::freeTypeCollection(moduleTypes);
    }
    for (unsigned i = 0; i < all_funcs.size(); i++) {
        delete all_funcs[i];
    }
}

void BPatch_module::parseTypesIfNecessary() {
    if (moduleTypes) return; // Already done
    
    moduleTypes = BPatch_typeCollection::getModTypeCollection(this);
    
    // We create all functions before parsing types; this is necessary
    // to prevent infinite recursion.
    
    if( BPatch::bpatch->parseDebugInfo() ) {
        // This gets slightly complex. We need to ensure that all
        // modules in the image are defined, since there is a slight
        // mismatch between BPatch's view of a "module" and the
        // debugging info's view.  It's easy enough to do.

        const pdvector<mapped_module *> &map_mods = mod->obj()->getModules();
        for (unsigned i = 0; i < map_mods.size(); i++) {
            // use map_mods[i] instead of a name to get a precise match
            BPatch_module *bpmod = img->findOrCreateModule(map_mods[i]);
            assert(bpmod);
            bpmod->getProcedures(); // Ensures that all functions are defined.
            bpmod->parseTypesIfNecessary(); // Since we should debug parse at the object level
        }
        parseTypes();
    } 
}

BPatch_typeCollection *BPatch_module::getModuleTypesInt() {
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
    BPatch_Vector<BPatch_function *> *retfuncs = new BPatch_Vector<BPatch_function *>;
    
    const pdvector<int_function *> &funcs = mod->getAllFunctions();

    for (unsigned int f = 0; f < funcs.size(); f++)
        if (incUninstrumentable || funcs[f]->isInstrumentable()) {
            BPatch_function * bpfunc = proc->findOrCreateBPFunc(funcs[f], this);
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
  unsigned size = funcs.size();

  if (!name) {
    char msg[512];
    sprintf(msg, "%s[%d]:  Module %s: findFunction(NULL)...  failing",
           __FILE__, __LINE__, mod->fileName().c_str());
    BPatch_reportError(BPatchSerious, 100, msg);
    return NULL;
  }

  // Do we want regex?
  if (dont_use_regex ||
      (NULL == strpbrk(name, REGEX_CHARSET))) {
      pdvector<int_function *> int_funcs;
      if (mod->findFuncVectorByPretty(name,
                                      int_funcs)) {
          for (unsigned piter = 0; piter < int_funcs.size(); piter++) {
              if (incUninstrumentable || int_funcs[piter]->isInstrumentable()) 
                  {
                      BPatch_function * bpfunc = proc->findOrCreateBPFunc(int_funcs[piter], this);
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
                          BPatch_function * bpfunc = proc->findOrCreateBPFunc(int_funcs[miter], this);
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
   
   if( !regex_case_sensitive )
     cflags |= REG_ICASE;
   
   //cerr << "compiling regex: " <<name<<endl;
   
   if (0 != (err = regcomp( &comp_pat, name, cflags ))) {
     char errbuf[80];
     regerror( err, &comp_pat, errbuf, 80 );
     if (notify_on_failure) {
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
   
   const pdvector<int_function *> &int_funcs = mod->getAllFunctions();
   
   for (unsigned ai = 0; ai < int_funcs.size(); ai++) {
     int_function *func = int_funcs[ai];
     // If it matches, push onto the vector
     // Check all pretty names (and then all mangled names if there is no match)
     bool found_match = false;
     for (unsigned piter = 0; piter < func->prettyNameVector().size(); piter++) {
       const pdstring &pName = func->prettyNameVector()[piter];
       int err;     
       if (0 == (err = regexec(&comp_pat, pName.c_str(), 1, NULL, 0 ))){
	 if (func->isInstrumentable() || incUninstrumentable) {
	   BPatch_function *foo = proc->findOrCreateBPFunc(func, NULL);
	   funcs.push_back(foo);
	 }
	 found_match = true;
	 break;
       }
     }
     if (found_match) continue; // Don't check mangled names

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
   }

   regfree(&comp_pat);
   
   if (funcs.size() != size) {
      return &funcs;
   } 
   
   if (notify_on_failure) {
     pdstring msg = pdstring("Unable to find pattern: ") + pdstring(name);
     BPatch_reportError(BPatchSerious, 100, msg.c_str());
   }
#endif
  }

  if(notify_on_failure) {
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
      bpfunc = proc->findOrCreateBPFunc(pdfunc, this);
      if (bpfunc) {
          funcs.push_back(bpfunc);
     }
  }
  return &funcs;
}

BPatch_function * BPatch_module::findFunctionByMangledInt(const char *mangled_name,
						       bool incUninstrumentable)
{
  BPatch_function *bpfunc = NULL;

  pdvector<int_function *> int_funcs;
  pdstring mangled_str(mangled_name);

  if (!mod->findFuncVectorByMangled(mangled_str,
                                    int_funcs))
      return NULL;
  
  if (int_funcs.size() > 1) {
    cerr << "Warning: found multiple matches for " << mangled_name << ", returning first" << endl;
  }
  int_function *pdfunc = int_funcs[0];
  
  if (incUninstrumentable || pdfunc->isInstrumentable()) {
      bpfunc = proc->findOrCreateBPFunc(pdfunc, this);
  }
  
  return bpfunc;
}

bool BPatch_module::dumpMangledInt(char * prefix)
{
  mod->dumpMangled(prefix);
  return true;
}

extern char *parseStabString(BPatch_module *, int linenum, char *str, 
			     int fPtr, BPatch_typeCommon *commonBlock = NULL);


#if defined(rs6000_ibm_aix4_1)

#include <linenum.h>
#include <syms.h>

// Gets the stab and stabstring section and parses it for types
// and variables
void BPatch_module::parseTypes()
{

    int i, j;
    int nlines;
    int nstabs;
    char* lines;
    SYMENT *syms;
    SYMENT *tsym;
    char *stringPool;
    char tempName[9];
    char *stabstr=NULL;
    union auxent *aux;
    image * imgPtr=NULL;
    char* funcName = NULL;
    Address staticBlockBaseAddr = 0;
    unsigned long linesfdptr;
    BPatch_typeCommon *commonBlock = NULL;
    BPatch_variableExpr *commonBlockVar = NULL;
    pdstring* currentSourceFile = NULL;
    bool inCommonBlock = false;

#if defined(TIMED_PARSE)
  struct timeval starttime;
  gettimeofday(&starttime, NULL);
#endif

  imgPtr = mod->obj()->parse_img();
  
  const Object &objPtr = imgPtr->getObject();

  //Using the Object to get the pointers to the .stab and .stabstr
  objPtr.get_stab_info(stabstr, nstabs, syms, stringPool); 

    objPtr.get_line_info(nlines,lines,linesfdptr); 

    bool parseActive = true;
    for (i=0; i < nstabs; i++) {
      /* do the pointer addition by hand since sizeof(struct syment)
       *   seems to be 20 not 18 as it should be */
      SYMENT *sym = (SYMENT *) (((unsigned) syms) + i * SYMESZ);
      // SYMENT *sym = (SYMENT *) (((unsigned) syms) + i * sizeof(struct syment));


      if (sym->n_sclass == C_FILE) {
	 char *moduleName;
	 if (!sym->n_zeroes) {
	    moduleName = &stringPool[sym->n_offset];
	 } else {
	    memset(tempName, 0, 9);
	    strncpy(tempName, sym->n_name, 8);
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

	 if(currentSourceFile) delete currentSourceFile;
	 currentSourceFile = new pdstring(moduleName);
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

      char *nmPtr;
      if (!sym->n_zeroes && ((sym->n_sclass & DBXMASK) ||
			     (sym->n_sclass == C_BINCL) ||
			     (sym->n_sclass == C_EINCL))) {
	  if (sym->n_offset < 3) {
	      if (sym->n_offset == 2 && stabstr[0]) {
		  nmPtr = &stabstr[0];
	      } else {
		  nmPtr = &stabstr[sym->n_offset];
	      }
	  } else if (!stabstr[sym->n_offset-3]) {
	      nmPtr = &stabstr[sym->n_offset];
	  } else {
	      /* has off by two error */
	      nmPtr = &stabstr[sym->n_offset-2];
	  }
#ifdef notdef
	  bperr("using nmPtr = %s\n", nmPtr);
	  bperr("got n_offset = (%d) %s\n", sym->n_offset, &stabstr[sym->n_offset]);
	  if (sym->n_offset>=2) 
	      bperr("got n_offset-2 = %s\n", &stabstr[sym->n_offset-2]);
	  if (sym->n_offset>=3) 
	      bperr("got n_offset-3 = %x\n", stabstr[sym->n_offset-3]);
	  if (sym->n_offset>=4) 
	      bperr("got n_offset-4 = %x\n", stabstr[sym->n_offset-4]);
#endif
      } else {
	  // names 8 or less chars on inline, not in stabstr
	  memset(tempName, 0, 9);
	  strncpy(tempName, sym->n_name, 8);
	  nmPtr = tempName;
      }

      if ((sym->n_sclass == C_BINCL) ||
	  (sym->n_sclass == C_EINCL) ||
	  (sym->n_sclass == C_FUN)) {
		if (funcName) { 
		    free(funcName);
		    funcName = NULL;
		}
		funcName = strdup(nmPtr);
		/* The call to parseLineInformation(), below, used to modify the symbols passed to it. */
		char * colonPtr = strchr( funcName, ':' );
		if( colonPtr != NULL ) {
			* colonPtr = '\0';
		}

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
   	    if (NULL == findFunction(funcName, bpmv) || !bpmv.size()) {
	      bperr("unable to locate current function %s\n", funcName);
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
	      tsym = (SYMENT *) (((unsigned) syms) + sym->n_value * SYMESZ);

	      // We can't lookup the value by name, because the name might have been
	      // redefined later on (our lookup would then pick the last one)

	      // Since this whole function is AIX only, we're ok to get this info

	      staticBlockBaseAddr = tsym->n_value;

	      /*
	      char *staticName, tempName[9];
	      if (!tsym->n_zeroes) {
		  staticName = &stringPool[tsym->n_offset];
	      } else {
		  memset(tempName, 0, 9);
		  strncpy(tempName, tsym->n_name, 8);
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
		  sym->n_value+staticBlockBaseAddr, commonBlock);
	  } else {
	      parseStabString(this, 0, nmPtr, sym->n_value, commonBlock);
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
#endif
}

#endif

#if defined(sparc_sun_solaris2_4) \
 || defined(i386_unknown_solaris2_5) \
 || defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ \
 || defined(ia64_unknown_linux2_4)


void BPatch_module::parseTypes() 
{
   image *moduleImage = mod->obj()->parse_img();
   assert( moduleImage != NULL );
   const Object & moduleObject = moduleImage->getObject();	

   if (moduleObject.hasStabInfo()) { 
      parseStabTypes(); 
   }
	
#if defined( USES_DWARF_DEBUG )
   if (moduleObject.hasDwarfInfo()) { 
       parseDwarfTypes(); 
   }
#endif
} 


// parseStabTypes:  parses type and variable info, does some init
// does NOT parse file-line info anymore, this is done later, upon request.
void BPatch_module::parseStabTypes() 
{
  stab_entry *stabptr;
  const char *next_stabstr;

  unsigned i;
  char *modName;
  char * temp=NULL;
  image * imgPtr=NULL;
  char *ptr, *ptr2, *ptr3;
  bool parseActive = false;

  pdstring* currentFunctionName = NULL;
  Address currentFunctionBase = 0;
  BPatch_variableExpr *commonBlockVar = NULL;
 char *commonBlockName;
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
  const Object &objPtr = imgPtr->getObject();

  //Using the Object to get the pointers to the .stab and .stabstr
  // XXX - Elf32 specific needs to be in seperate file -- jkh 3/18/99
  stabptr = objPtr.get_stab_info();
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
      current_func_name = NULL; // reset for next object file
      current_mangled_func_name = NULL; // reset for next object file
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


    if(parseActive || mod->obj()->isSharedLib()) {
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
            currentFunctionName = new pdstring(tmp);
            
            currentFunctionBase = 0;
            Symbol info;
            // Shouldn't this be a function name lookup?
            if (!proc->llproc->getSymbolInfo(*currentFunctionName,
                                             info))
                {
                    pdstring fortranName = *currentFunctionName + pdstring("_");
                    if (proc->llproc->getSymbolInfo(fortranName,info))
                        {
                            delete currentFunctionName;
                            currentFunctionName = new pdstring(fortranName);
                        }
                }
            
            currentFunctionBase = info.addr();

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
      case N_BCOMM:	{
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
      
      case N_ECOMM: {
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
	  temp = parseStabString(this, mostRecentLinenum, (char *)ptr, stabptr->val(i), commonBlock);
	else
	  temp = parseStabString(this, stabptr->desc(i), (char *)ptr, stabptr->val(i), commonBlock);
	if (temp && *temp) {
	  //Error parsing the stabstr, return should be \0
	  bperr( "Stab string parsing ERROR!! More to parse: %s\n",
		  temp);
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

// Parsing symbol table for Alpha platform
// Mehmet

#if defined(alpha_dec_osf4_0)
extern void parseCoff(BPatch_module *mod, char *exeName, 
			const pdstring& modName, LineInformation& linfo);

void BPatch_module::parseTypes()
{
  image * imgPtr=NULL;

  //Using mapped_module to get the image Object.
  imgPtr = mod->pmod()->exec();

  //Get the path name of the process
  char *file = (char *)(imgPtr->file()).c_str();

  parseCoff(this, file, mod->fileName(),mod->getLineInformation());
}

#endif



#if defined(i386_unknown_nt4_0) \
 || defined(mips_unknown_ce2_11) //ccw 6 apr 2001

// Parsing symbol table for NT platform
// Mehmet 7/24/00

#include "CodeView.h"

void BPatch_module::parseTypes()
{
  
  image * imgPtr=NULL;

  //Using mapped_module to get the image Object.
  imgPtr = mod->obj()->parse_img();

  // trying to get at codeview symbols
  // need text section id and addr of codeview symbols
  unsigned int textSectionId = imgPtr->getObject().GetTextSectionId();
  PIMAGE_NT_HEADERS peHdr = imgPtr->getObject().GetImageHeader();

  DWORD modBaseAddr = imgPtr->getObject().code_off();
  PVOID mapAddr = imgPtr->getObject().GetMapAddr();

  PVOID pCodeViewSymbols = NULL;
  ULONG dirEntSize = 0;
  PIMAGE_SECTION_HEADER pDebugSectHeader = NULL;
  PIMAGE_DEBUG_DIRECTORY baseDirEnt = 
	  (PIMAGE_DEBUG_DIRECTORY)ImageDirectoryEntryToDataEx( mapAddr,
											FALSE,   // we mapped using MapViewOfFile
											IMAGE_DIRECTORY_ENTRY_DEBUG,
											&dirEntSize,
											&pDebugSectHeader );
  if( baseDirEnt != NULL )
  {
	PIMAGE_DEBUG_DIRECTORY pDirEnt = baseDirEnt;
	unsigned int nDirEnts = dirEntSize / sizeof(IMAGE_DEBUG_DIRECTORY);
	for( unsigned int i = 0; i < nDirEnts; i++ )
	{
		if( pDirEnt->Type == IMAGE_DEBUG_TYPE_CODEVIEW )
		{
			pCodeViewSymbols = (PVOID)(((char*)mapAddr) + pDirEnt->PointerToRawData);
			break;
		}
		// advance to next entry
		pDirEnt++;
	}
  }

  if( pCodeViewSymbols != NULL )
  {
	CodeView* cv = new CodeView( (const char*)pCodeViewSymbols, 
					textSectionId );

	cv->CreateTypeAndLineInfo(this, modBaseAddr, mod->getLineInformation());
  }
}
#endif


bool BPatch_module::getVariablesInt(BPatch_Vector<BPatch_variableExpr *> &vars)
{
 
  BPatch_variableExpr *var;
  parseTypesIfNecessary();
  
  pdvector<pdstring> keys = moduleTypes->globalVarsByName.keys();
  int limit = keys.size();
  for (int j = 0; j < limit; j++) {
    pdstring name = keys[j];
    var = img->createVarExprByName(this, name.c_str());
    if (var != NULL)
        vars.push_back(var);
  }
  if (limit) 
    return true;
  
#ifdef IBM_BPATCH_COMPAT
  //  IBM getVariables can be successful while returning an empty set of vars
  return true;
#else
  return false;
#endif
}

bool BPatch_module::getAddressRangesInt( const char * fileName, unsigned int lineNo, std::vector< std::pair< Address, Address > > & ranges ) {
    LineInformation & li = mod->getLineInformation();
    if( fileName == NULL ) { fileName = mod->fileName().c_str(); }
    return li.getAddressRanges( fileName, lineNo, ranges );
} /* end getAddressRanges() */

LineInformation & BPatch_module::getLineInformationInt() {
    return this->mod->getLineInformation();
} /* end getLineInformation() */

bool BPatch_module::isSharedLibInt() {
  return mod->obj()->isSharedLib();
}

bool BPatch_module::isNativeCompilerInt()
{
  return nativeCompiler;
}

size_t BPatch_module::getAddressWidthInt()
{
  return mod->obj()->parse_img()->getObject().getAddressWidth();
}

void BPatch_module::setDefaultNamespacePrefix(char *name) 
{ 
    img->setDefaultNamespacePrefix(name); 
}

#ifdef IBM_BPATCH_COMPAT

bool BPatch_module::getLineNumbersInt(unsigned int &start, unsigned int &end)
{
    start = 0;
    end = 0;
    return true;
}

bool BPatch_module::getAddressRangeInt(void * &start, void * &end)
{
    // Code? Data? We'll do code for now...
    start = (void *)(mod->obj()->codeAbs());
    end = (void *)(mod->obj()->codeAbs() + mod->obj->codeSize());
    return true;
}
char *BPatch_module::getUniqueStringInt(char *buffer, int length)
{
    // Get the start and end address of this module
    void* start = NULL;
    void* end = NULL;
    getAddressRange(start, end);
    
    // Form unique name from module name and start address
    int written = snprintf(buffer, length, "%llx|%s",
                           start, mod->fileName().c_str());
    assert((written >= 0) && (written < length));
    
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

#endif

