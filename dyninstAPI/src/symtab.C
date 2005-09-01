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

 // $Id: symtab.C,v 1.252 2005/09/01 22:18:43 bernat Exp $

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/arch.h"
#include "dyninstAPI/src/instPoint.h"
#include "dyninstAPI/src/Object.h"
#include <fstream>
#include "dyninstAPI/src/util.h"
#include "dyninstAPI/src/dyninstP.h"
#include "common/h/String.h"
#include "dyninstAPI/src/inst.h"
#include "common/h/Timer.h"
#include "dyninstAPI/src/showerror.h"
#include "common/h/debugOstream.h"
#include "common/h/pathName.h"          // extract_pathname_tail()
#include "dyninstAPI/src/function.h"

#ifndef BPATCH_LIBRARY
#include "paradynd/src/mdld.h"
#include "paradynd/src/main.h"
#include "paradynd/src/init.h"
#include "common/h/Dictionary.h"
#endif

#include "LineInformation.h"
#include "dyninstAPI/h/BPatch_flowGraph.h"

#if defined(TIMED_PARSE)
#include <sys/time.h>
#endif

#if defined( USES_DWARF_DEBUG )
#include "dwarf.h"
#include "libdwarf.h"
#endif

// All debug_ostream vrbles are defined in process.C (for no particular reason)
extern unsigned enable_pd_sharedobj_debug;

extern bool parseCompilerType(Object *);

pdvector<image*> image::allImages;

int codeBytesSeen = 0;


/* imported from platform specific library list.  This is lists all
   library functions we are interested in instrumenting. */

pdmodule *image::newModule(const pdstring &name, const Address addr, supportedLanguages lang)
{

    pdmodule *ret = NULL;
    // modules can be defined several times in C++ due to templates and
    //   in-line member functions.
    if ((ret = findModule(name))) {
      return(ret);
    }

    parsing_printf("=== image, creating new pdmodule %s, addr 0x%x\n",
                   name.c_str(), addr);

    pdstring fileNm, fullNm;
    fullNm = name;
    fileNm = extract_pathname_tail(name);

	// /* DEBUG */ fprintf( stderr, "%s[%d]: Creating new pdmodule '%s'/'%s'\n", __FILE__, __LINE__, fileNm.c_str(), fullNm.c_str() );
    ret = new pdmodule(lang, addr, fullNm, fileNm, this);
    modsByFileName[ret->fileName()] = ret;
    modsByFullName[ret->fullName()] = ret;
    _mods.push_back(ret);

    return(ret);
}


// TODO -- is this g++ specific
bool buildDemangledName( const pdstring & mangled, pdstring & use, bool nativeCompiler, 
			supportedLanguages lang )
{
 /* The C++ demangling function demangles MPI__Allgather (and other MPI__
  * functions with start with A) into the MPI constructor.  In order to
  * prevent this a hack needed to be made, and this seemed the cleanest
  * approach.
  */

  if( mangled.prefixed_by( "MPI__" ) ) { 
    return false;
    }	  

  /* If it's Fortran, eliminate the trailing underscores, if any. */
  if(    lang == lang_Fortran 
      || lang == lang_CMFortran 
      || lang == lang_Fortran_with_pretty_debug ) {

    if( mangled[ mangled.length() - 1 ] == '_' ) {
      char * demangled = strdup( mangled.c_str() );
      demangled[ mangled.length() - 1 ] = '\0';
      use = pdstring( demangled );

      free( demangled );
      return true;
    }
    else {
      /* No trailing underscores, do nothing */
      return false;
      }
    } /* end if it's Fortran. */

  //  Check to see if we have a gnu versioned symbol on our hands.
  //  These are of the form <symbol>@<version> or <symbol>@@<version>
  //
  //  If we do, we want to create a "demangled" name for the one that
  //  is of the form <symbol>@@<version> since this is, by definition,
  //  the default.  The "demangled" name will just be <symbol>

  //  NOTE:  this is just a 0th order approach to dealing with versioned
  //         symbols.  We may need to do something more sophisticated
  //         in the future.  JAW 10/03

#if !defined(i386_unknown_nt4_0)
  char *atat;
  if (NULL != (atat = strstr(mangled.c_str(), "@@"))) {
    use = mangled.substr(0 /*start pos*/, 
			 (int)(atat - mangled.c_str())/*len*/);
    //char msg[256];
    //sprintf(msg, "%s[%d]: 'demangling' versioned symbol: %s, to %s",
    //	    __FILE__, __LINE__, mangled.c_str(), use.c_str());
    //cerr << msg << endl;
    //logLine(msg);
      
    return true;
  }
#endif


  /* Try demangling it. */
  char * demangled = P_cplus_demangle( mangled.c_str(), nativeCompiler );
  if( demangled == NULL ) { return false; }    
  
  use = pdstring( demangled );

  free( demangled );  
  return true;
  } /* end buildDemangledName() */

#ifdef DEBUG_TIME
static timer loadTimer;
static FILE *timeOut=0;
#endif /* DEBUG_TIME */

// makeOneFunction(): find name of enclosing module and define function symbol
//
// module information comes from one of three sources:
//   #1 - debug format (stabs, DWARF, etc.)
//   #2 - file format (ELF, COFF)
//   #3 - file name (a.out, libXXX.so)
// (in order of decreasing reliability)
image_func *image::makeOneFunction(pdvector<Symbol> &mods,
				     const Symbol &lookUp) 
{
  // find module name
  Address modAddr = 0;
  pdstring modName = lookUp.module();
  // /* DEBUG */ fprintf( stderr, "%s[%d]: makeOneFunction()'s module: %s\n", __FILE__, __LINE__, modName.c_str() );
  
  if (modName == "") {
    modName = name_ + "_module";
  } else if (modName == "DEFAULT_MODULE") {
    pdstring modName_3 = modName;
    findModByAddr(lookUp, mods, modName, modAddr, modName_3);
  }

  pdmodule *use = getOrCreateModule(modName, modAddr);
  assert(use);

#if defined(arch_ia64)
  parsing_printf("New function %s at 0x%llx\n",
          lookUp.name().c_str(), 
          lookUp.addr());
#else
  parsing_printf("New function %s at 0x%x\n",
          lookUp.name().c_str(), 
          lookUp.addr());
#endif

  image_func *func = new image_func(lookUp.name(), 
                                    lookUp.addr(), 
                                    lookUp.size(), 
                                    use, 
                                    this);

  assert(func);

  return func;
}

#ifndef BPATCH_LIBRARY

//Add an extra pretty name to a known function (needed for handling
//overloaded functions in paradyn)
void image::addTypedPrettyName( image_func *func, const char *typedName) {
   pdvector<image_func*> *funcsByPrettyEntry = NULL;
   pdstring typed(typedName);

   //XXX
   //   fprintf(stderr,"addTypedPrettyName %s\n",typedName);
   if (!funcsByPretty.find(typed, funcsByPrettyEntry)) {
      funcsByPrettyEntry = new pdvector<image_func*>;
      funcsByPretty[typed] = funcsByPrettyEntry;
   }
   assert(funcsByPrettyEntry);
   (*funcsByPrettyEntry).push_back(func);
}

#endif

/*
 * Add another name for the current function to the names vector in
 * the function object.  We also need to add the extra names to the
 * lookup hash tables
 */
void image::addMultipleFunctionNames(image_func *dup)
					
{
  // Obtain the original function at the same address:
  image_func *orig;
  assert(funcsByEntryAddr.find(dup->getOffset(), orig));

  pdstring mangled_name = dup->symTabName();
  pdstring pretty_name = dup->prettyName();

  /* add the new names to the existing function */
  orig->addSymTabName(mangled_name);
  orig->addPrettyName(pretty_name);

  /* now we add the names and the function object to the hash tables */
  //  Mangled Hash:
  pdvector<image_func*> *funcsByMangledEntry = NULL;
  if (!funcsByMangled.find(mangled_name, funcsByMangledEntry)) {
    funcsByMangledEntry = new pdvector<image_func*>;
    funcsByMangled[mangled_name] = funcsByMangledEntry;
  }

  assert(funcsByMangledEntry);
  (*funcsByMangledEntry).push_back(orig); // might need to check/eliminate duplicates here??

  // Pretty Hash:
  //XXX
  //   fprintf(stderr,"addMultipleFunctionNames %s\n",pretty_name.c_str());

  pdvector<image_func*> *funcsByPrettyEntry = NULL;
  if(!funcsByPretty.find(pretty_name, funcsByPrettyEntry)) {
    funcsByPrettyEntry = new pdvector<image_func*>;
    funcsByPretty[pretty_name] = funcsByPrettyEntry;
  }
    
  assert(funcsByPrettyEntry);
  (*funcsByPrettyEntry).push_back(orig); // might need to check/eliminate duplicates here??
}

/*
 * Add all the functions (*) in the list of symbols to our data
 * structures. 
 *
 * We do a search for a "main" symbol (a couple of variants), and
 * if found we flag this image as the executable (a.out). 
 */

bool image::symbolsToFunctions(pdvector<Symbol> &mods, 
			       pdvector<image_func *> *raw_funcs)
{
#if defined(TIMED_PARSE)
  struct timeval starttime;
  gettimeofday(&starttime, NULL);
#endif

  Symbol lookUp;
  pdvector< Symbol > lookUps;
  pdstring symString;

  // is_a_out is a member variable
  Symbol mainFuncSymbol;  //Keeps track of info on "main" function

  //Checking "main" function names in same order as in the inst-*.C files
  if (linkedFile.get_symbols(symString="main",     lookUps) ||
      linkedFile.get_symbols(symString="_main",    lookUps)
#if defined(i386_unknown_nt4_0)
      ||
      linkedFile.get_symbols(symString="WinMain",  lookUps) ||
      linkedFile.get_symbols(symString="_WinMain", lookUps) ||
      linkedFile.get_symbols(symString="wWinMain", lookUps) ||
      linkedFile.get_symbols(symString="_wWinMain", lookUps)
#endif
      ) {
      assert( lookUps.size() == 1 );
      lookUp = lookUps[0];
      
      mainFuncSymbol = lookUp;
      is_a_out = true;
      
      if (lookUp.type() == Symbol::PDST_FUNCTION) {
          if (!isValidAddress(lookUp.addr())) {
              pdstring msg;
              char tempBuffer[40];
              sprintf(tempBuffer,"0x%lx",lookUp.addr());
              msg = pdstring("Function ") + lookUp.name() + pdstring(" has bad address ")
              + pdstring(tempBuffer);
              statusLine(msg.c_str());
              showErrorCallback(29, msg);
              bperr( "Whoops\n");
              
              return false;
          }      
          image_func *main_pdf = makeOneFunction(mods, lookUp);
          assert(main_pdf);
          raw_funcs->push_back(main_pdf);
      }
      else {
          bperr( "Type not function!\n");
      }
      
  }
  else
    is_a_out = false;

  // Checking for libdyninstRT (DYNINSTinit())
  if (linkedFile.get_symbols(symString="DYNINSTinit",  lookUps) ||
      linkedFile.get_symbols(symString="_DYNINSTinit", lookUps))
    is_libdyninstRT = true;
  else
    is_libdyninstRT = false;
 
  // JAW 02-03 -- restructured below slightly to get rid of multiple loops
  // through entire symbol list
  pdvector<Symbol> kludge_symbols;
  
  // find the real functions -- those with the correct type in the symbol table
  for(SymbolIter symIter(linkedFile); symIter;symIter++) {
    const Symbol &lookUp = symIter.currval();
    const char *np = lookUp.name().c_str();

    //parsing_printf("Scanning file: symbol %s\n", lookUp.name().c_str());

    //    fprintf(stderr,"np %s\n",np);

    if (linkedFile.isEEL() && np[0] == '.')
         /* ignore these EEL symbols; we don't understand their values */
	 continue; 
    if (is_a_out && 
	(lookUp.addr() == mainFuncSymbol.addr()) &&
	(lookUp.name() == mainFuncSymbol.name()))
      // We already added main(), so skip it now
      continue;
#ifdef NOTDEF
    //  This is now done later while building the "real" maps.
    //  We will have some duplication/aliasing while building up the raw (unclassed) map
    // but these will be weeded out later according to the same criteria.
    image_func *placeholder;
    if (funcsByEntryAddr.find(lookUp.addr(), placeholder)) {
        // We have already seen a function at this addr. add a second name
        // for this function.
        addMultipleFunctionNames(lookUp);
        continue;
    }
#endif

    if (lookUp.module() == "DYNINSTheap") {
        // Do nothing for now; we really don't want to report it as
        // a real symbol.
    }
    else {
        if (lookUp.type() == Symbol::PDST_FUNCTION) {
            // /* DEBUG */ fprintf( stderr, "%s[%d]: considering function symbol %s in module %s\n", __FILE__, __LINE__, lookUp.name().c_str(), lookUp.module().c_str() );
            
            pdstring msg;
            char tempBuffer[40];
            if (!isValidAddress(lookUp.addr())) {
                if (strncmp(lookUp.name().c_str(), "DYNINSTstaticHeap", strlen("DYNINSTstaticHeap"))) {
                    sprintf(tempBuffer,"0x%lx",lookUp.addr());
                    msg = pdstring("Function ") + lookUp.name() + pdstring(" has bad address ")
                        + pdstring(tempBuffer);
                    statusLine(msg.c_str());
                    showErrorCallback(29, msg);
                    return false;
                }
            }
            
            image_func *new_func = makeOneFunction(mods, lookUp);
            if (!new_func)
                cerr << __FILE__ << __LINE__ << ":  makeOneFunction returned NULL!" << endl;
            else
                raw_funcs->push_back(new_func);
        }
        
        if (lookUp.type() ==  Symbol::PDST_OBJECT) {
            //  JAW: This is here for legacy purposes, I do not know if it still applies:
            if (lookUp.kludge()) {
                //logLine(P_strdup(symString.c_str()));
                // Figure out where this happens
                
                // now find the pseudo functions -- this gets ugly
                // kludge has been set if the symbol could be a function
                // WHERE DO WE USE THESE KLUDGES? WHAT PLATFORM???
                
                cerr << "Found <KLUDGE> function " << lookUp.name().c_str() 
                     << ".  All <KLUDGE>  functions currently ignored!  see " 
                     << __FILE__ << __LINE__ << endl;
                //kludge_symbols.push_back(lookUp);
                
            }
        }
    }
  }

#ifdef NOTDEF
  // go through vector of kludge functions found and add ones that are not already def'd. 
  for (unsigned int i = 0; i < kludge_symbols.size(); ++i) {
    if (funcsByEntryAddr.defines(kludge_symbols[i].addr()))
      // Already defined a symbol at this addr
      continue;
    addOneFunction(mods, lookUp);
  }
#endif

#if defined(TIMED_PARSE)
  struct timeval endtime;
  gettimeofday(&endtime, NULL);
  unsigned long lstarttime = starttime.tv_sec * 1000 * 1000 + starttime.tv_usec;
  unsigned long lendtime = endtime.tv_sec * 1000 * 1000 + endtime.tv_usec;
  unsigned long difftime = lendtime - lstarttime;
  double dursecs = difftime/(1000 );
  cout << __FILE__ << ":" << __LINE__ <<": symbolsToFunctions took "<<dursecs <<" msecs" << endl;
#endif
  return true;
}

bool image::addDiscoveredVariables() {
    // Hrm... we could identify globals via address analysis...
    return true;
}

bool image::addSymtabVariables()
{
   /* Eventually we'll have to do this on all platforms (because we'll retrieve
    * the type information here).
    */
   
#if defined(TIMED_PARSE)
   struct timeval starttime;
   gettimeofday(&starttime, NULL);
#endif

   pdstring mangledName; 
   Symbol symInfo;

   for(SymbolIter symIter(linkedFile); symIter; symIter++) {
      const pdstring &mangledName = symIter.currkey();
      const Symbol &symInfo = symIter.currval();
      if (strlen(symInfo.module().c_str()) == 0) {
          continue;
      }
#if 0
      fprintf(stderr, "Symbol %s, mod %s, addr 0x%x, type %d, linkage %d\n",
              symInfo.name().c_str(),
              symInfo.module().c_str(),
              symInfo.addr(),
              symInfo.type(),
              symInfo.linkage());
#endif
      if (symInfo.type() == Symbol::PDST_OBJECT) {
          image_variable *var;
          bool addToMangled = false;
          bool addToPretty = false;
          if (varsByAddr.defines(symInfo.addr())) {
              var = varsByAddr[symInfo.addr()];
              addToMangled = var->addSymTabName(mangledName);
          }
          else {
              parsing_printf("New variable, mangled %s, module %s...\n",
                             mangledName.c_str(),
                             symInfo.module().c_str());
              pdmodule *use = getOrCreateModule(symInfo.module(),
                                                symInfo.addr());
              assert(use);
              var = new image_variable(symInfo.addr(),
                                                       mangledName,
                                                       use);
              addToMangled = true;
              exportedVariables.push_back(var);
          }

          char * unmangledName =
              P_cplus_demangle( mangledName.c_str(), nativeCompiler );

          if (unmangledName) {
              pdstring prettyName(unmangledName);
              addToPretty = var->addPrettyName(prettyName);
          }

          pdvector<image_variable *> *varEntries;
          if (addToPretty) {
              if (varsByPretty.defines(unmangledName)) {
                  varEntries = varsByPretty[unmangledName];
              }
              else {
                  varEntries = new pdvector<image_variable *>;
                  varsByPretty[unmangledName] = varEntries;
              }
              (*varEntries).push_back(var);
          }

          if (addToMangled) {
              if (varsByMangled.defines(mangledName)) {
                  varEntries = varsByMangled[mangledName];
              }
              else {
                  varEntries = new pdvector<image_variable *>;
                  varsByMangled[mangledName] = varEntries;
              }
              (*varEntries).push_back(var);
          }
          
          free( unmangledName );
      }
   }

#if defined(TIMED_PARSE)
   struct timeval endtime;
   gettimeofday(&endtime, NULL);
   unsigned long lstarttime = starttime.tv_sec * 1000 * 1000 + starttime.tv_usec;
   unsigned long lendtime = endtime.tv_sec * 1000 * 1000 + endtime.tv_usec;
   unsigned long difftime = lendtime - lstarttime;
   double dursecs = difftime/(1000 );
   cout << __FILE__ << ":" << __LINE__ <<": addSymtabVariables took "<<dursecs <<" msecs" << endl;
#endif

   return true;
}

/*
 * will search for symbol NAME or _NAME
 * returns false on failure 
 */
#if 0
bool image::findInternalSymbol(const pdstring &name, 
			       const bool warn, 
			       internalSym &ret_sym)
{
   pdvector< Symbol > lookUps;
   Symbol lookUp;

   if(linkedFile.get_symbols(name,lookUps)){
      if( lookUps.size() == 1 ) { lookUp = lookUps[0]; }
      else { return false; } 
      ret_sym = internalSym(lookUp.addr(),name); 
      return true;
   }
   else {
       pdstring new_sym;
       new_sym = pdstring("_") + name;
       if(linkedFile.get_symbols(new_sym,lookUps)){
          if( lookUps.size() == 1 ) { lookUp = lookUps[0]; }
          else { return false; } 
          ret_sym = internalSym(lookUp.addr(),name); 
          return true;
       }
   } 
   if(warn){
      pdstring msg;
      msg = pdstring("Unable to find symbol: ") + name;
      statusLine(msg.c_str());
      showErrorCallback(28, msg);
   }
   return false;
}
#endif

pdmodule *image::findModule(const pdstring &name, bool wildcard)
{
  pdmodule *found = NULL;
  //cerr << "image::findModule " << name << " , " << find_if_excluded
  //     << " called" << endl;

  if (!wildcard) {
      if (modsByFileName.defines(name)) {
          //cerr << " (image::findModule) found module in modsByFileName" << endl;
          found = modsByFileName[name];
      }
      else if (modsByFullName.defines(name)) {
          //cerr << " (image::findModule) found module in modsByFullName" << endl;
          found = modsByFullName[name];
      }
  }
  else {
      //  if we want a substring, have to iterate over all module names
      //  this is ok b/c there are not usually more than a handful or so
      //
      dictionary_hash_iter<pdstring, pdmodule *> mi(modsByFileName);
      pdstring pds; pdmodule *mod;
      
      while (mi.next(pds, mod)){
          if (name.wildcardEquiv(mod->fileName()) ||
              name.wildcardEquiv(mod->fullName())) {
              found = mod; 
              break;
          }
      }
  }
  
  //cerr << " (image::findModule) did not find module, returning NULL" << endl;
  if (found) {
      // Just-in-time...
      //if (parseState_ == symtab)
      //analyzeImage();
      return found;
  }
  
  return NULL;
}

#ifndef BPATCH_LIBRARY
/* 
 * return 0 if symbol <symname> exists in image, non-zero if it does not
 */
bool image::symbolExists(const pdstring &symname)
{
  pdvector<image_func *> *pdfv;
  if (NULL != (pdfv = findFuncVectorByPretty(symname)) && pdfv->size())
    return true;

  if (NULL != (pdfv = findFuncVectorByMangled(symname)) && pdfv->size())
    return true;

  return false;
}

void image::postProcess(const pdstring pifname)
{
  FILE *Fil;
  pdstring fname, errorstr;
  char key[5000];
  char tmp1[5000], abstraction[500];
  resource *parent;

  return;

  /* What file to open? */
  if (!(pifname == (char*)NULL)) {
      fname = pifname;
  } else {
      fname = desc_.file() + ".pif";
  }

  /* Open the file */
  Fil = P_fopen(fname.c_str(), "r");

  if (Fil == NULL) {
    errorstr = pdstring("Tried to open PIF file ") + fname;
    errorstr += pdstring(", but could not (continuing)\n");
    logLine(P_strdup(errorstr.c_str()));
    showErrorCallback(35, errorstr); 
    return;
  }

  /* Process the file */
  while (!feof(Fil)) {
    fscanf(Fil, "%s", key);
    switch (key[0]) {
    case 'M':
      /* Ignore mapping information for right now */
      fgets(tmp1, 5000, Fil);
      break;
    case 'R':
#ifndef BPATCH_LIBRARY
      /* Create a new resource */
      fscanf(Fil, "%s {", abstraction);
      parent = rootResource;
      do {
	fscanf(Fil, "%s", tmp1);
        if (tmp1[0] != '}') {
          parent = resource::newResource(parent, NULL,
					 abstraction,
					 tmp1, timeStamp::ts1970(),
					 nullString, // uniqifier
#if READY
                     0,         // what type to use?
#else
                     OtherResourceType,
#endif // READY
					 MDL_T_STRING,
					 true);
        } else {
	  parent = NULL;
	}
      } while (parent != NULL);
#endif
      break;
    default:
      errorstr = pdstring("Ignoring bad line key (") + fname;
      errorstr += pdstring(") in file %s\n");
      logLine(P_strdup(errorstr.c_str()));
      fgets(tmp1, 5000, Fil);
      break;
    }
  }
  return;
}
#endif

void image::defineModules(process *proc) {
    //    fprintf(stderr,"%s defineModules\n",name_.c_str());

  pdstring pds; pdmodule *mod;
  dictionary_hash_iter<pdstring, pdmodule*> mi(modsByFileName);

  while (mi.next(pds, mod)){
    mod->define(proc);
  }

#ifndef BPATCH_LIBRARY
#ifdef DEBUG_MDL
  std::ostringstream osb(std::ios::out);
  osb << "IMAGE_" << name() << "__" << getpid() << std::ends;
  ofstream of(osb, std::ios::app);
#endif
#endif
}

#ifndef BPATCH_LIBRARY
void dfsCreateLoopResources(BPatch_loopTreeNode *n, resource *res,
                            image_func *pdf)
{
    resource *r = res;

    if (n->loop != NULL) {
        r = resource::newResource(res, pdf,
                                  nullString, // abstraction
                                  pdstring(n->name()),
                                  timeStamp::ts1970(),
                                  nullString, // uniquifier
                                  LoopResourceType,
                                  MDL_T_LOOP,
                                  false );
    }

    for (unsigned i = 0; i < n->children.size(); i++) {
        // loop resource objects are nested under their parent function rather
        // than each other. using r instead of res would cause the resource
        // hierarchy to have loops nested under each other.
        // dfsCreateLoopResources(n->children[i], r, pdf);
        dfsCreateLoopResources(n->children[i], res, pdf);
    }
}
#endif

#ifndef BPATCH_LIBRARY
extern bool should_report_loops;
#endif

//  Comments on what this does would be nice....
//  Appears to run over a pdmodule, after all code in it has been processed
//   and parsed into functions, and define a resource for the module + a 
//   resource for every function found in the module (apparently includes 
//   excluded functions, but not uninstrumentable ones)....
//  Can't directly register call graph relationships here as resources
//   are being defined, because need all resources defined to 
//   do that....
void pdmodule::define(process *proc) {
#ifdef DEBUG_MODS
   std::ostringstream osb(std::ios::out);
   osb << "MODS_" << exec()->name() << "__" << getpid() << std::ends;
   ofstream of(osb, std::ios::app);
#endif

#ifndef BPATCH_LIBRARY
   for(unsigned i = 0; i < allUniqueFunctions.size(); i++) 
   {
      image_func * pdf = allUniqueFunctions[i]; 

      if (!pdf->isInstrumentable() ||
          (pdf->getEndOffset() == pdf->getOffset()))
          continue;
      // ignore line numbers for now 
      
      //check if the function is overloaded, and store types with the name
      //in the case that it is.  This way, we can differentiate
      //between overloaded functions in the paradyn front-end.
      bool useTyped = false;

      pdvector<image_func *> *pdfv =
         allFunctionsByPrettyName[pdf->prettyName()];
      char * prettyWithTypes = NULL;
      
      if(pdfv != NULL && pdfv->size() > 1) {
         prettyWithTypes = P_cplus_demangle(pdf->symTabName().c_str(), 
                                            exec()->isNativeCompiler(), true);
         if( prettyWithTypes != NULL ) {
            
            useTyped = true;
            // Add to image...
            exec()->addTypedPrettyName(pdf, prettyWithTypes);
            // And module...
            addTypedPrettyName(pdf, prettyWithTypes);
            // And function
            pdf->addPrettyName(pdstring(prettyWithTypes));
         } else {
            prettyWithTypes = strdup( pdf->prettyName().c_str() );
            assert( prettyWithTypes != NULL );
         }
      }

      if( prettyWithTypes != NULL ) { free(prettyWithTypes); }
   }
   
#endif
}

// Tests if a symbol starts at a given point
bool image::hasSymbolAtPoint(Address point) const
{
    return knownSymAddrs.defines(point);
}

const pdvector<image_func*> &image::getAllFunctions() 
{
  analyzeIfNeeded();
  return everyUniqueFunction;
}

const pdvector<image_variable*> &image::getAllVariables()
{
    analyzeIfNeeded();
    return everyUniqueVariable;
}

const pdvector<image_func*> &image::getExportedFunctions() const { return exportedFunctions; }

const pdvector<image_func*> &image::getCreatedFunctions()
{
  analyzeIfNeeded();
  return createdFunctions;
}

const pdvector<image_variable*> &image::getExportedVariables() const { return exportedVariables; }

const pdvector<image_variable*> &image::getCreatedVariables()
{
  analyzeIfNeeded();
  return createdVariables;
}

const pdvector <pdmodule*> &image::getModules() 
{
  return _mods;
}

void print_module_vector_by_short_name(pdstring prefix ,
				       pdvector<pdmodule*> *mods) {
    unsigned int i;
    pdmodule *mod;
    for(i=0;i<mods->size();i++) {
        mod = ((*mods)[i]);
	cerr << prefix << mod->fileName() << endl;
    }
}


#ifndef BPATCH_LIBRARY
// rip module name out of constraint....
// Assumes that constraint is of form module/function, or
// module.... 
pdstring getModuleName(pdstring constraint) {
    pdstring ret;

    const char *data = constraint.c_str();
    const char *first_slash = P_strchr(data, RH_SEPERATOR);
    
    // no "/", assume string holds module name....
    if (first_slash == NULL) {
	return constraint;
    }    
    // has "/", assume everything up to "/" is module name....
    return pdstring(data, first_slash - data);
}

// rip function name out of constraint....
// Assumes that constraint is of form module/function, or
// module.... 
pdstring getFunctionName(pdstring constraint) {
    pdstring ret;

    const char *data = constraint.c_str();
    const char *first_slash = P_strchr(data, RH_SEPERATOR);
    
    // no "/", assume constraint is module only....
    if (first_slash == NULL) {
	return pdstring("");
    }    
    // has "/", assume everything after "/" is function....
    return pdstring(first_slash+1);
}




// mcheyney, Oct. 6, 1997
static dictionary_hash<pdstring, pdstring> func_constraint_hash(pdstring::hash);
static bool cache_func_constraint_hash() {
    static bool func_constraint_hash_loaded = FALSE;

    // strings holding exclude constraints....
    pdvector<pdstring> func_constraints;
    // if unble to get list of excluded functions, assume all functions
    //  are NOT excluded!!!!
    if(mdl_get_lib_constraints(func_constraints) == FALSE) {
	return FALSE;
    }
    func_constraint_hash_loaded = TRUE;

    unsigned i;
    for(i=0;i<func_constraints.size();i++) {
	func_constraint_hash[func_constraints[i]] = func_constraints[i];
    }
    return TRUE;
}

// mcheyney, Oct. 3, 1997
// Return boolean value indicating whether function is found to
//  be excluded via "exclude module_name/function_name" (but NOT
//  via "exclude module_name").
bool function_is_excluded(image_func *func, pdstring module_name) {
    static bool func_constraint_hash_loaded = FALSE;

    pdstring function_name = func->prettyName();
    pdstring full_name = module_name + pdstring("/") + function_name;

    if (func_constraint_hash_loaded == FALSE) {
        if (!cache_func_constraint_hash()) {
	    return FALSE;
        }
    }

    if (func_constraint_hash.defines(full_name)) {
        return TRUE;
    }
    return FALSE;
}


bool module_is_excluded(pdmodule *module) {
    static bool func_constraint_hash_loaded = FALSE;

    pdstring full_name = module->fileName();

    if (func_constraint_hash_loaded == FALSE) {
        if (!cache_func_constraint_hash()) {
	    return FALSE;
        }
    }

    if (func_constraint_hash.defines(full_name)) {
        return TRUE;
    }
    return FALSE;
}

//
// mcheyney, Sep 28, 1997
// Take a list of functions (in vector <all_funcs>.  Copy all
//  of those functions which are not excluded (via "exclude" 
//  {module==module_name}/{function==function_name) into
//  <some_functions>.  DONT filter out those excluded via
//  exclude module==module_name...., eh????
// Returns status of mdl_get_lib_constraints() call.
//  If this status == FALSE< some_funcs is not modified....
// We assume that all_funcs is generally longer than the list
//  of constrained functions.  As such, internally proc. copies
//  all funcs into some funcs, then runs over excluded funcs
//  removing any matches, as opposed to doing to checking 
//  while adding to some_funcs....

bool filter_excluded_functions(pdvector<image_func*> all_funcs,
    pdvector<image_func*>& some_funcs, pdstring module_name) {
    unsigned i;

    pdstring full_name;
    static bool func_constraint_hash_loaded = FALSE;

    if (func_constraint_hash_loaded == FALSE) {
        if (!cache_func_constraint_hash()) {
	    return FALSE;
        }
    }

    // run over all_funcs, check if module/function is caught
    //  by an exclude....
    for(i=0;i<all_funcs.size();i++) {
        full_name = module_name + pdstring("/") + all_funcs[i]->prettyName();
	if (!(func_constraint_hash.defines(full_name))) {
            some_funcs += all_funcs[i];
        }
    }
    
    return TRUE;
}

#endif /* BPATCH_LIBRARY */

// identify module name from symbol address (binary search)
// based on module tags found in file format (ELF/COFF)
void image::findModByAddr (const Symbol &lookUp, pdvector<Symbol> &mods,
			   pdstring &modName, Address &modAddr, 
			   const pdstring &defName)
{
  if (mods.size() == 0) {
    modAddr = 0;
    modName = defName;
    return;
  }

  Address symAddr = lookUp.addr();
  int index;
  int start = 0;
  int end = mods.size() - 1;
  int last = end;
  bool found = false;
  while ((start <= end) && !found) {
    index = (start+end)/2;
    if ((index == last) ||
	((mods[index].addr() <= symAddr) && 
	 (mods[index+1].addr() > symAddr))) {
      modName = mods[index].name();
      modAddr = mods[index].addr();      
      found = true;
    } else if (symAddr < mods[index].addr()) {
      end = index - 1;
    } else {
      start = index + 1;
    }
  }
  if (!found) { 
    // must be (start > end)
    modAddr = 0;
    modName = defName;
    //modName = mods[0].name();
    //modAddr = mods[0].addr();
  }
}

unsigned int int_addrHash(const Address& addr) {
  return (unsigned int)addr;
}

/*
 * load an executable:
 *   1.) parse symbol table and identify rotuines.
 *   2.) scan executable to identify inst points.
 *
 *  offset is normally zero except on CM-5 where we have two images in one
 *    file.  The offset passed to parseImage is the logical offset (0/1), not
 *    the physical point in the file.  This makes it faster to "parse" multiple
 *    copies of the same image since we don't have to stat and read to find the
 *    physical offset. 
 */

image *image::parseImage(fileDescriptor &desc)
{
  /*
   * Check to see if we have parsed this image at this offset before.
   * We only match if the entire file descriptor matches, which can
   * can be filename matching or filename/offset matching.
   */
  unsigned numImages = allImages.size();
  
  // AIX: it's possible that we're reparsing a file with better information
  // about it. If so, yank the old one out of the images vector -- replace
  // it, basically.
  for (unsigned u=0; u<numImages; u++) {
#if 0
      fprintf(stderr, "Comparing %s/0x%x/0x%x/%s/%d to %s/0x%x/0x%x/%s/%d\n",
              desc.file().c_str(),
              desc.code(),
              desc.data(),
              desc.member().c_str(),
              desc.pid(),
              allImages[u]->desc().file().c_str(),
              allImages[u]->desc().code(),
              allImages[u]->desc().data(),
              allImages[u]->desc().member().c_str(),
              allImages[u]->desc().pid());
#endif
      if (desc == allImages[u]->desc()) {
          // We reference count...
          return allImages[u]->clone();
      }
  }
  /*
   * load the symbol table. (This is the a.out format specific routine).
   */

  if(desc.isSharedObject()) 
    statusLine("Processing a shared object file");
  else  
    statusLine("Processing an executable file");
  
  bool err=false;

  
  // TODO -- kill process here
  image *ret = new image(desc, err); 

  if (err || !ret) {
     if (ret) {
        delete ret;
     }
     return NULL;
  }

  image::allImages.push_back(ret);

  // define all modules.
#ifndef BPATCH_LIBRARY
  tp->resourceBatchMode(true);
#endif

  // XXX callers of parseImage now defineModules
  //statusLine("defining modules");
  //ret->defineModules();

  statusLine("ready"); // this shouldn't be here, right? (cuz we're not done, right?)

#ifndef BPATCH_LIBRARY
  tp->resourceBatchMode(false);
#endif
  return ret;
}

/*
 * Remove a parsed executable from the global list. Used if the old handle
 * is no longer valid.
 */
void image::removeImage(image *img)
{

  // Here's a question... do we want to actually delete images?
  // Pro: free up memory. Con: we'd just have to parse them again...
  // I guess the question is "how often do we serially open files".
  /* int refCount = */ img->destroy();
  

  /*
    // We're not deleting when the refcount hits 0, so we may as well
    // keep the vector. It's a time/memory problem. 
  if (refCount == 0) {
    pdvector<image*> newImages;
    // It's gone... remove from image vector
    for (unsigned i = 0; i < allImages.size(); i++) {
      if (allImages[i] != img)
	newImages.push_back(allImages[i]);
    }
    allImages = newImages;
  }
  */
}

void image::removeImage(const pdstring file)
{
  image *img = NULL;
  for (unsigned i = 0; i < allImages.size(); i++) {
    if (allImages[i]->file() == file)
      img = allImages[i];
  }
  // removeImage plays with the allImages vector... so do this
  // outside the for loop.
  if (img) image::removeImage(img);
}

void image::removeImage(fileDescriptor &desc)
{
  image *img = NULL;
  for (unsigned i = 0; i < allImages.size(); i++) {
    // Never bothered to implement a != operator
    if (allImages[i]->desc() == desc)
      img = allImages[i];
  }
  if (img) image::removeImage(img);
}

//  a helper routine that selects a language based on information from the symtab
supportedLanguages pickLanguage(pdstring &working_module, char *working_options, 
				supportedLanguages working_lang)
{
  supportedLanguages lang = lang_Unknown;
  static int sticky_fortran_modifier_flag = 0;

  // (2) -- check suffixes -- try to keep most common suffixes near the top of the checklist
  if (working_module.suffixed_by(".c", 2)) lang = lang_C;
  else if (working_module.suffixed_by(".C",2)) lang = lang_CPlusPlus;
  else if (working_module.suffixed_by(".cpp",4)) lang = lang_CPlusPlus;
  else if (working_module.suffixed_by(".F",2)) lang = lang_Fortran; 
  else if (working_module.suffixed_by(".f",2)) lang = lang_Fortran; 
  else if (working_module.suffixed_by(".cc",3)) lang = lang_C;
  else if (working_module.suffixed_by(".a",2)) lang = lang_Assembly; // is this right?
  else if (working_module.suffixed_by(".S",2)) lang = lang_Assembly; 
  else if (working_module.suffixed_by(".s",2)) lang = lang_Assembly; 
  else {
    //(3) -- try to use options string -- if we have 'em
    if (working_options) {
      //  NOTE:  a binary is labeled "gcc2_compiled" even if compiled w/g77 -- thus this is
      //         quite inaccurate to make such assumptions
      if (strstr(working_options, "gcc")) 
	lang = lang_C; 
      else if (strstr(working_options, "g++")) 
	lang = lang_CPlusPlus; 
    }
  }

  //  This next section tries to determine the version of the debug info generator for a
  //  Sun fortran compiler.  Some leave the underscores on names in the debug info, and some
  //  have the "pretty" names, we need to detect this in order to properly read the debug.
  if (working_lang == lang_Fortran) {
    if (sticky_fortran_modifier_flag) {
      //cerr << __FILE__ << __LINE__ << ": UPDATE: lang_Fortran->lang_Fortran_with_pretty_debug." << endl;
      working_lang = lang_Fortran_with_pretty_debug;
    }
    else if (working_options) {
      char *dbg_gen = NULL;
      //      cerr << __FILE__ << __LINE__ << ":  OPT: " << working_options << endl; 
      if (NULL != (dbg_gen = strstr(working_options, "DBG_GEN="))) {
	//cerr << __FILE__ << __LINE__ << ":  OPT: " << dbg_gen << endl; 
	// Sun fortran compiler (probably), need to examine version
	char *dbg_gen_ver_maj = dbg_gen + strlen("DBG_GEN=");
	//cerr << __FILE__ << __LINE__ << ":  OPT: " << dbg_gen_ver_maj << endl; 
	char *next_dot = strchr(dbg_gen_ver_maj, '.');
	if (NULL != next_dot) {
	  next_dot = '\0';  //terminate major version number string
	  int ver_maj = atoi(dbg_gen_ver_maj);
	  //cerr <<"Major Debug Ver. "<<ver_maj<< endl;
	  if (ver_maj < 3) {
	    working_lang = lang_Fortran_with_pretty_debug;
	    sticky_fortran_modifier_flag = 1;
	    //cerr << __FILE__ << __LINE__ << ": UPDATE: lang_Fortran->lang_Fortran_with_pretty_debug.  "
	    //	 <<"Major Debug Ver. "<<ver_maj<< endl;
	  }
	}
      }
    }
  }
  
  return lang;
}
void image::getModuleLanguageInfo(dictionary_hash<pdstring, supportedLanguages> *mod_langs)
{
   pdstring working_module;
#if defined(sparc_sun_solaris2_4) \
 || defined(i386_unknown_solaris2_5) \
 || defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ \
 || defined(ia64_unknown_linux2_4) /* Temporary duplication -- TLM. */
 
   char *ptr;
   // check .stabs section to get language info for modules:
//   int stab_nsyms;
//   char *stabstr_nextoffset;
//   const char *stabstrs = 0;

   pdstring mod_string;

   // This ugly flag is set when certain (sun) fortran compilers are detected.
   // If it is set at any point during the following iteration, this routine
   // ends with "backtrack mode" and reiterates through all chosen languages, changing
   // lang_Fortran to lang_Fortran_with_pretty_debug.
   //
   // This may be ugly, but it is set up this way since the information that is used
   // to determine whether this flag is set comes from the N_OPT field, which 
   // seems to come only once per image.  The kludge is that we assume that all
   // fortran sources in the module have this property (they probably do, but
   // could conceivably be mixed (???)).
   int fortran_kludge_flag = 0;

   // "state variables" we use to accumulate potentially useful information
   //  A final module<->language decision is not made until we have arrived at the
   //  next module entry, at which point we use any and all info we have to 
   //  make the most sensible guess
   supportedLanguages working_lang = lang_Unknown;
   char *working_options = NULL, *working_name = NULL;

   stab_entry *stabptr = NULL;
   const char *next_stabstr = NULL;
#if defined(TIMED_PARSE)
   struct timeval starttime;
   gettimeofday(&starttime, NULL);
#endif

   //Using the Object to get the pointers to the .stab and .stabstr
   // XXX - Elf32 specific needs to be in seperate file -- jkh 3/18/99
   stabptr = linkedFile.get_stab_info();
   next_stabstr = stabptr->getStringBase();

   for( unsigned int i = 0; i < stabptr->count(); i++ ) {
      if (stabptr->type(i) == N_UNDF) {/* start of object file */
         /* value contains offset of the next string table for next module */
         // assert(stabptr->nameIdx(i) == 1);
	 stabptr->setStringBase(next_stabstr);
	 next_stabstr = stabptr->getStringBase() + stabptr->val(i); 
      }
      else if (stabptr->type(i) == N_OPT){
         //  We can use the compiler option string (in a pinch) to guess at the source file language
         //  There is possibly more useful information encoded somewhere around here, but I lack
         //  an immediate reference....
         if (working_name)
            working_options = const_cast<char *> (stabptr->name(i)); 
      }
      else if ((stabptr->type(i) == N_SO)  || (stabptr->type(i) == N_ENDM)){ /* compilation source or file name */
         // We have arrived at the next source file, finish up with the last one and reset state
         // before starting next


         //   XXXXXXXXXXX  This block is mirrored near the end of routine, if you edit it,
         //   XXXXXXXXXXX  change it there too.
         if  (working_name) {
            working_lang = pickLanguage(working_module, working_options, working_lang);
            if (working_lang == lang_Fortran_with_pretty_debug)
               fortran_kludge_flag = 1;
            (*mod_langs)[working_module] = working_lang;

         }
         //   XXXXXXXXXXX
	
         // reset "state" here
         working_lang = lang_Unknown;
         working_options = NULL;

         //  Now:  out with the old, in with the new

         if (stabptr->type(i) == N_ENDM) {
            // special case:
            // which is most likely both broken (and ignorable ???)
            working_name = "DEFAULT_MODULE";
         }
         else {
            working_name = const_cast<char*>(stabptr->name(i));
            ptr = strrchr(working_name, '/');
            if (ptr) {
               ptr++;
               working_name = ptr;
            }
         }
         working_module = pdstring(working_name);

         if (mod_langs->defines(working_module) && (*mod_langs)[working_module] != lang_Unknown) {
            //  we already have a module with this name in the map.  If it has been given
            //  a language assignment (not lang_Unknown), we can just skip ahead
            working_name = NULL;
            working_options = NULL;
            continue;
         } 
         else {
            //cerr << __FILE__ << __LINE__ << ":  Module: " <<working_module<< " has language "<< stabptr->desc(i) << endl;  
            switch (stabptr->desc(i)) {
              case N_SO_FORTRAN: 
                 working_lang = lang_Fortran;
                 break;
              case N_SO_F90:
                 working_lang = lang_Fortran;  // not sure if this should be different from N_SO_FORTRAN
                 break;
              case N_SO_AS:
                 working_lang = lang_Assembly;
                 break;
              case N_SO_ANSI_C:
              case N_SO_C:
                 working_lang = lang_C;
                 break;
              case N_SO_CC:
                 working_lang = lang_CPlusPlus;
                 break;
              default:
                 //  currently uncovered options are lang_CMFortran, and lang_GnuCPlusPlus
                 //  do we need to make this kind of distinction here?
                 working_lang = lang_Unknown;
                 break;
            }
	
         } 
      } // end N_SO section
#ifdef NOTDEF
      else {
         //  This is here only to trace the parse, for my edification and knowledge, should be removed
         //  Throw away most known symbols here
         if ( (N_FUN != stabptr->type(i)) &&
              (N_GSYM != stabptr->type(i)) &&
              (N_STSYM != stabptr->type(i)) &&
              (N_LCSYM != stabptr->type(i)) &&
              (N_ROSYM != stabptr->type(i)) &&
              (N_SLINE != stabptr->type(i)) &&
              (N_SOL != stabptr->type(i)) &&
              (N_ENTRY != stabptr->type(i)) &&
              (N_BCOMM != stabptr->type(i)) &&
              (N_ECOMM != stabptr->type(i))) {
            char hexbuf[10];
            sprintf(hexbuf, "%p",stabptr->type(i) );
            cerr << __FILE__ << __LINE__ << ":  got " << hexbuf << endl;
         }
      }
#endif
   } // for loop

   //  Need to make sure we finish up with the module we were last collecting information 
   //  about

   //   XXXXXXXXXXX  see note above (find the X's)
   if  (working_name) {
      working_lang = pickLanguage(working_module, working_options, working_lang);	  
      if (working_lang == lang_Fortran_with_pretty_debug)
         fortran_kludge_flag = 1;
      (*mod_langs)[working_module] = working_lang;
   }
   //   XXXXXXXXXXX

   if (fortran_kludge_flag) {
      // go through map and change all lang_Fortran to lang_Fortran_with_pretty_symtab
      dictionary_hash_iter<pdstring, supportedLanguages> iter(*mod_langs);
      pdstring aname;
      supportedLanguages alang;
      while (iter.next(aname, alang)) {
         if (lang_Fortran == alang) {
            (*mod_langs)[aname] = lang_Fortran_with_pretty_debug;
            cerr << __FILE__ << __LINE__ << ": UPDATE: lang_Fortran->lang_Fortran_with_pretty_debug.  " << endl;
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
   cout << __FILE__ << ":" << __LINE__ <<": getModuleLanguageInfo took "<<dursecs <<" msecs" << endl;
#endif


#endif // stabs platforms

#if defined( USES_DWARF_DEBUG )
   if (linkedFile.hasDwarfInfo()) {
      const char *fileName = linkedFile.getFileName();

      int fd = open( fileName, O_RDONLY );
      assert ( fd != -1 );
      Dwarf_Debug dbg;
      int status = dwarf_init( fd, DW_DLC_READ, NULL, NULL, & dbg, NULL );
      assert( status == DW_DLV_OK );

      Dwarf_Unsigned hdr;

      while( dwarf_next_cu_header( dbg, NULL, NULL, NULL, NULL, & hdr, NULL ) == DW_DLV_OK ) {
         Dwarf_Die moduleDIE;
         status = dwarf_siblingof(dbg, NULL, &moduleDIE, NULL);
         assert ( status == DW_DLV_OK );
         
         Dwarf_Half moduleTag;
         status = dwarf_tag( moduleDIE, & moduleTag, NULL);
         assert( status == DW_DLV_OK );
         assert( moduleTag == DW_TAG_compile_unit );
         
         /* Extract the name of this module. */
         char * moduleName;
         status = dwarf_diename( moduleDIE, & moduleName, NULL );
         ptr = strrchr(moduleName, '/');
         if (ptr) {
            ptr++;
         } else {
            ptr = moduleName;
         }

         working_module = pdstring(ptr);

         if (status != DW_DLV_NO_ENTRY) {
            assert( status != DW_DLV_ERROR );
            Dwarf_Attribute languageAttribute;
            status = dwarf_attr( moduleDIE, DW_AT_language, & languageAttribute, NULL );
            assert( status != DW_DLV_ERROR );
            if( status == DW_DLV_OK ) {
               Dwarf_Unsigned languageConstant;
               status = dwarf_formudata( languageAttribute, & languageConstant, NULL );
               assert( status == DW_DLV_OK );
               
               switch( languageConstant ) {
               case DW_LANG_C:
               case DW_LANG_C89:
                  (*mod_langs)[working_module] = lang_C;
                  break;
               case DW_LANG_C_plus_plus:
                  (*mod_langs)[working_module] = lang_CPlusPlus;
                  break;
               case DW_LANG_Fortran77:
                  (*mod_langs)[working_module] = lang_Fortran;
                  break;
               case DW_LANG_Fortran90:
                  (*mod_langs)[working_module] = lang_Fortran;
                  break;
               default:
                  /* We know what the language is but don't care. */
                  break;
               } /* end languageConstant switch */
               
               dwarf_dealloc( dbg, languageAttribute, DW_DLA_ATTR );
            }
            dwarf_dealloc( dbg, moduleName, DW_DLA_STRING );
         }
         dwarf_dealloc( dbg, moduleDIE, DW_DLA_DIE );
      }

      status = dwarf_finish( dbg, NULL );
      assert( status == DW_DLV_OK );
      close( fd );
   }
#endif

   //
   // eCoff Platforms
   //
#if defined(alpha_dec_osf4_0)

   LDFILE *ldptr;
   pCHDRR symtab;

   ldptr = ldopen((char *)linkedFile.GetFile().c_str(), NULL);
   symtab = SYMTAB(ldptr);
   for (int i = 0; i < symtab->hdr.ifdMax; ++i) {
      pCFDR file = symtab->pcfd + i;
      char *tmp = ((symtab->psym + file->pfd->isymBase)->iss + 
                   (symtab->pss + file->pfd->issBase));
      pdstring modname = (strrchr(tmp, '/') ? strrchr(tmp, '/') + 1 : tmp);

      if (file->pfd->csym && modname.length()) {
         switch (file->pfd->lang) {
           case langAssembler:
              (*mod_langs)[modname] = lang_Assembly;
              break;

           case langC:
           case langStdc:
              (*mod_langs)[modname] = lang_C;
              break;

           case langCxx:
           case langDECCxx:
              (*mod_langs)[modname] = lang_CPlusPlus;
              break;

           case langFortran:
           case langFortran90:
              (*mod_langs)[modname] = lang_Fortran;
              break;

           default:
              (*mod_langs)[modname] = lang_Unknown;
         }

      }
   }
   ldaclose(ldptr);

#endif // eCoff Platforms

}

//  setModuleLanguages is only called after modules have been defined.
//  it attempts to set each module's language, information which is needed
//  before names can be demangled.
void image::setModuleLanguages(dictionary_hash<pdstring, supportedLanguages> *mod_langs)
{
  if (!mod_langs->size()) return;  // cannot do anything here
  //  this case will arise on non-stabs platforms until language parsing can be introduced at this level
  pdvector<pdmodule *> *modlist;
  pdmodule *currmod = NULL;
  modlist = &_mods;
  //int dump = 0;

  for (unsigned int i = 0;  i < modlist->size(); ++i) {
    currmod = (*modlist)[i];
    supportedLanguages currLang;

    if (currmod->isShared()) continue;  // need to find some way to get shared object languages?
    if(mod_langs->find(currmod->fileName(), currLang)) {
      currmod->setLanguage(currLang);
    }
    else {
      //cerr << __FILE__ << __LINE__ << ":  module " << currmod->fileName() 
      //	   << " not found in module<->language map" << endl;
      //dump = 1;
      //  here we should probably try to guess, based on filename conventions
    }
  }
#ifdef NOTDEF
  // REMOVE!!  this is for debuggering
  if (dump) {
    dictionary_hash_iter<pdstring, supportedLanguages> iter(*mod_langs);
    pdstring aname;
    supportedLanguages alang;
    cerr << __FILE__ << __LINE__ << ": contents of module<->language map:" << endl;
    while (iter.next(aname, alang)) {
      cerr << aname << " : " << alang << endl;
    }
  }
#endif
}

int addrfunccmp( image_func*& pdf1, image_func*& pdf2 )
{
    if( pdf1->getOffset() > pdf2->getOffset() )
        return 1;
    if( pdf1->getOffset() < pdf2->getOffset() )
        return -1;
    return 0;
}

bool image::parseFunction(image_func* pdf, pdvector< Address >& callTargets)
{
    // callTargets: targets of this function (to help identify new functions
    // May be returned unmodified
    return pdf->findInstPoints( callTargets);
}

void image::parseStaticCallTargets( pdvector< Address >& callTargets,
                                    pdvector< image_func* > &raw_funcs,
                                    pdmodule* mod )
{

  // TODO: I don't think "mod" is the right thing here; should
  // do a lookup by address
    char name[20] = "f";
    image_func* pdf;

    for( unsigned j = 0; j < callTargets.size(); j++ )
    {
        if( !isCode( callTargets[ j ] ) )
            continue;    
        if( funcsByEntryAddr.defines( callTargets[ j ] ) )
            continue;
               
        sprintf(name, "f%lx", callTargets[j] );
        //some (rare) compiler generated symbols have 0 for the size.
        //most of these belong to screwy functions and, it seems
        //best to avoid them. to distinguish between these screwy functions
        //and the unparsed ones that we make up we use UINT_MAX for the sizes 
        //of the unparsed functions
        pdf = new image_func( name, callTargets[ j ], UINT_MAX, mod, this);
        pdf->addPrettyName( name );
        
        //we no longer keep separate lists for instrumentable and 
        //uninstrumentable
        if (parseFunction( pdf, callTargets )) {
            everyUniqueFunction.push_back(pdf);
            createdFunctions.push_back(pdf);
            enterFunctionInTables( pdf, mod );
            raw_funcs.push_back( pdf );
        }
    }
}


// Enter a function in all the appropriate tables
void image::enterFunctionInTables(image_func *func, pdmodule *mod) {
  if (!func) return;

  funcsByEntryAddr[func->getOffset()] = func;

  // TODO: out-of-line insertion here
  funcsByRange.insert(func);
  
  // Possibly multiple demangled (pretty) names...
  // And multiple functions (different addr) with the same pretty
  // name. So we have a many::many mapping...
  for (unsigned pretty_iter = 0; 
       pretty_iter < func->prettyNameVector().size();
       pretty_iter++) {
    pdstring pretty_name = func->prettyNameVector()[pretty_iter];
    pdvector<image_func *> *funcsByPrettyEntry = NULL;
    
    // Ensure a vector exists
    if (!funcsByPretty.find(pretty_name,			      
			    funcsByPrettyEntry)) {
      funcsByPrettyEntry = new pdvector<image_func*>;
      funcsByPretty[pretty_name] = funcsByPrettyEntry;
    }

    (*funcsByPrettyEntry).push_back(func);
  }
  
  // And multiple symtab names...
  for (unsigned symtab_iter = 0; 
       symtab_iter < func->symTabNameVector().size();
       symtab_iter++) {
    pdstring symtab_name = func->symTabNameVector()[symtab_iter];
    pdvector<image_func *> *funcsBySymTabEntry = NULL;
    
    // Ensure a vector exists
    if (!funcsByMangled.find(symtab_name,			      
			    funcsBySymTabEntry)) {
      funcsBySymTabEntry = new pdvector<image_func*>;
      funcsByMangled[symtab_name] = funcsBySymTabEntry;
    }

    (*funcsBySymTabEntry).push_back(func);
  }

  // And modules....
  mod->addFunction(func);
  
}  

//buildFunctionLists() iterates through image_funcs and constructs demangled 
//names. Demangling was moved here (names used to be demangled as image_funcs 
//were built) so that language information could be obtained _after_ the 
//functions and modules were built, but before name demangling takes place.  
//Thus we can use language information during the demangling process.

bool image::buildFunctionLists(pdvector <image_func *> &raw_funcs) 
{
  for (unsigned int i = 0; i < raw_funcs.size(); i++) {
    image_func *raw = raw_funcs[i];
    pdmodule *rawmod = raw->pdmod();

    assert(raw);
    assert(rawmod);
    
    // At this point we need to generate the following information:
    // A symtab name.
    // A pretty (demangled) name.
    // The symtab name goes in the global list as well as the module list.
    // Same for the pretty name.
    // Finally, check addresses to find aliases.

    pdstring mangled_name = raw->symTabName();
    pdstring working_name = mangled_name;
    pdstring pretty_name;

    //strip scoping information from mangled name before demangling:
    const char *p = P_strchr(working_name.c_str(), ':');
    if( p ) 
    {
        unsigned nchars = p - mangled_name.c_str();
        working_name = pdstring(mangled_name.c_str(), nchars);
    }
    
    if (!buildDemangledName(working_name, pretty_name, 
                            nativeCompiler, rawmod->language())) {
        pretty_name = working_name;
    }
    
    // Now, we see if there's already a function object for this
    // address. If so, add a new name; 
    image_func *possiblyExistingFunction = NULL;
    funcsByEntryAddr.find(raw->getOffset(), possiblyExistingFunction);
    if (possiblyExistingFunction) {
        // On some platforms we see two symbols, one in a real module
        // and one in DEFAULT_MODULE. Replace DEFAULT_MODULE with
        // the real one
        if (rawmod != possiblyExistingFunction->pdmod()) {
            if (rawmod->fileName() == "DEFAULT_MODULE")
                rawmod = possiblyExistingFunction->pdmod();
            if (possiblyExistingFunction->pdmod()->fileName() == "DEFAULT_MODULE") {
                possiblyExistingFunction->changeModule(rawmod);
            }
        }
        
        assert(rawmod == possiblyExistingFunction->pdmod());
      // Keep the new mangled name
      possiblyExistingFunction->addSymTabName(mangled_name);
      possiblyExistingFunction->addPrettyName(pretty_name);
      raw_funcs[i] = NULL;
      delete raw; // Don't leak
    }
    else {
      funcsByEntryAddr[raw->getOffset()] = raw;
      // Already have symtab name
      raw->addPrettyName(pretty_name);
    }
  }

  // Now that we have a 1) unique and 2) demangled list of function
  // names, loop through once more and build the address range tree
  // and name lookup tables. 
  for (unsigned j = 0; j < raw_funcs.size(); j++) {
    image_func *func = raw_funcs[j];
    if (!func) continue;

    pdmodule *mod = func->pdmod();
    // May be NULL if it was an alias.
    everyUniqueFunction.push_back(func);
    exportedFunctions.push_back(func);
    enterFunctionInTables(func, mod);
  }

  // Conspicuous lack: inst points. We're delaying.
  return true;
}

void image::analyzeIfNeeded() {
  if (parseState_ == symtab) {
      parsing_printf("ANALYZING IMAGE %s\n",
              file().c_str());
      analyzeImage();
      addDiscoveredVariables();
  }
}

//analyzeImage() iterates through image_funcs and constructs demangled 
//names. Demangling was moved here (names used to be demangled as image_funcs 
//were built) so that language information could be obtained _after_ the 
//functions and modules were built, but before name demangling takes place.  
//Thus we can use language information during the demangling process.
//After name demangling is done, each function's inst points are found and 
//the function is classified as either instrumentable or non-instrumentable 
//and filed accordingly 

bool image::analyzeImage()
{
    // TODO: remove arch_x86 from here - it's just for testing
#if defined(arch_x86_64) || defined(arch_x86)
    ia32_set_mode_64(getObject().getAddressWidth() == 8);
#endif
  
 // Hold unseen call targets
  pdvector< Address > callTargets;
  image_func *pdf;
  pdmodule *mod = NULL;

  assert(parseState_ < analyzed);
  // Prevent recursion: with this set we can call findFoo as often as we want
  parseState_ = analyzing;

  if (parseState_ < symtab) {
    fprintf(stderr, "Error: attempt to analyze before function lists built\n");
    return true;
  }
  
  pdvector<image_func *> new_functions;  

  for (unsigned i = 0; i < everyUniqueFunction.size(); i++) {
    
    pdf = everyUniqueFunction[i];
    mod = pdf->pdmod();
    assert(pdf); assert(mod);
    
    pdstring name = pdf->symTabName();
    parseFunction( pdf, callTargets);
  }      
 
  // callTargets now holds a big list of target addresses; some are already
  // in functions that we know about, some point to new functions. 
  
#if defined(cap_stripped_binaries)
  
  int numIndir = 0;
  unsigned p = 0;
  // We start over until things converge; hence the goto target
 top:
  new_functions.clear();
  // Also adds to lists
  parseStaticCallTargets( callTargets, new_functions, mod );
  // Any new call destinations show up in new_functions
  callTargets.clear(); 
   
  // nothing to do, exit
  if( everyUniqueFunction.size() <= 0 )
  {
      return true;
  }

  VECTOR_SORT(everyUniqueFunction, addrfunccmp);
  
  Address lastPos;
  lastPos = everyUniqueFunction[0]->getOffset() + 
      everyUniqueFunction[0]->getSymTabSize();
  
  unsigned int rawFuncSize = everyUniqueFunction.size();
  
  for( ; p + 1 < rawFuncSize; p++ )
  {
      image_func* func1 = everyUniqueFunction[p];
      image_func* func2 = everyUniqueFunction[p + 1];
      
      Address gapStart = func1->getOffset() + func1->getSymTabSize();
      Address gapEnd = func2->getOffset();
      Address gap = gapEnd - gapStart;
      
      //gap should be big enough to accomodate a function prologue
      if( gap >= 5 )
      {
          Address pos = gapStart;
          while( pos < gapEnd && isCode( pos ) )
          {
              const unsigned char* instPtr;
              instPtr = (const unsigned char *)getPtrToInstruction( pos );
              
              instruction insn;
              insn.setInstruction( instPtr );
              if( isFunctionPrologue(insn) && !funcsByEntryAddr.defines(pos))
              {
                  char name[20];
                  numIndir++;
                  sprintf( name, "f%lx", pos );
                  pdf = new image_func( name, pos, UINT_MAX, mod, this);
                  pdf->addPrettyName( name );
                  
                  everyUniqueFunction.push_back(pdf);
		  createdFunctions.push_back(pdf);
                  enterFunctionInTables(pdf, pdf->pdmod());
                  parseFunction( pdf, callTargets);
                  
                  if( callTargets.size() > 0 )
                  {
                      for( unsigned r = 0; r < callTargets.size(); r++ )
                      {
                          if( callTargets[r] < func1->getOffset() )
                              p++;
                      }
                      goto top; //goto is the devil's construct. repent!! 
                  }
              }
              pos++;
          }   
      }
  }
#endif
  
#if defined(cap_stripped_binaries)

  //phase 2 - error detection and recovery 
  VECTOR_SORT( everyUniqueFunction, addrfunccmp );
  for( unsigned int k = 0; k + 1 < everyUniqueFunction.size(); k++ )
    {
      image_func* func1 = everyUniqueFunction[ k ];
      image_func* func2 = everyUniqueFunction[ k + 1 ];
      
      if( func1->getOffset() == 0 ) 
          continue;

      assert( func1->getOffset() != func2->getOffset() );
      
      //look for overlapping functions
#if defined(os_linux) && (defined(arch_x86) || defined(arch_x86_64))
      if ((func2->getOffset() < func1->getEndOffset()) &&
	  (strstr(func2->prettyName().c_str(), "nocancel") ||
	   strstr(func2->prettyName().c_str(), "_L_mutex_lock")))
          
          { 
              func1->markAsNeedingRelocation(true);
          }
      else
#endif
          if( func2->getOffset() < func1->getEndOffset() )
              {
                  //use the start address of the second function as the upper bound
                  //on the end address of the first
                  Address addr = func2->getOffset();
                  func1->updateFunctionEnd(addr);
              }
    }    
#endif

  // And bind all intra-module call points
  for (unsigned b_iter = 0; b_iter < everyUniqueFunction.size(); b_iter++) {
      everyUniqueFunction[b_iter]->checkCallPoints();
  }
  
  parseState_ = analyzed;
  return true;
}


// Constructor for the image object. The fileDescriptor simply
// wraps (in the normal case) the object name and a relocation
// address (0 for a.out file). On the following platforms, we
// are handling a special case:
//   AIX: objects can possibly have a name like /lib/libc.so:shr.o
//          since libraries are archives
//        Both text and data sections have a relocation address


image::image(fileDescriptor &desc, bool &err)
   : 
   desc_(desc),
   is_libdyninstRT(false),
   is_a_out(false),
   main_call_addr_(0),
   nativeCompiler(false),    
   linkedFile(desc,pd_log_perror),//ccw jun 2002
   knownJumpTargets(int_addrHash, 8192),
   _mods(0),
   knownSymAddrs(addrHash4),
   funcsByEntryAddr(addrHash4),
   funcsByPretty(pdstring::hash),
   funcsByMangled(pdstring::hash),
   modsByFileName(pdstring::hash),
   modsByFullName(pdstring::hash),
   varsByPretty(pdstring::hash),
   varsByMangled(pdstring::hash),
   varsByAddr(addrHash4),
   refCount(1),
   parseState_(unparsed)
{
   err = false;
   name_ = extract_pathname_tail(desc.file());

   //   fprintf(stderr,"img name %s\n",name_.c_str());
   pathname_ = desc.file();

   // on some platforms (e.g. Windows) we try to parse
   // the image too soon, before we have a process we can
   // work with reliably.  If so, we must recognize it
   // and reparse at some later time.
   if( linkedFile.have_deferred_parsing() )
   {
      // nothing else to do here
      return;
   }

   // initialize (data members) codeOffset_, dataOffset_,
   //  codeLen_, dataLen_.
   
   codeOffset_ = linkedFile.code_off();
   dataOffset_ = linkedFile.data_off();
   
   codeLen_ = linkedFile.code_len();
   dataLen_ = linkedFile.data_len();

   codeValidStart_ = linkedFile.code_vldS();
   codeValidEnd_ = linkedFile.code_vldE();
   dataValidStart_ = linkedFile.data_vldS();
   dataValidEnd_ = linkedFile.data_vldE();

   // if unable to parse object file (somehow??), try to
   //  notify luser/calling process + return....    
   if (!codeLen_ || !linkedFile.code_ptr()) {
      pdstring msg = pdstring("Parsing problem with executable file: ") + desc.file();
      statusLine(msg.c_str());
      msg += "\n";
      logLine(msg.c_str());
      err = true;
#ifndef mips_unknown_ce2_11 //ccw 29 mar 2001
      
#if defined(BPATCH_LIBRARY)
      BPatch_reportError(BPatchWarning, 27, msg.c_str()); 
#else
      showErrorCallback(27, msg); 
#endif
#endif
      bperr( "Error parsing\n");
      
      return; 
   }
  

   pdstring msg;
   // give luser some feedback....
   msg = pdstring("Parsing object file: ") + desc.file();
   
   statusLine(msg.c_str());

   // use the *DUMMY_MODULE* until a module is defined
   //pdmodule *dynModule = newModule(DYN_MODULE, 0);
   //pdmodule *libModule = newModule(LIBRARY_MODULE, 0);
   // TODO -- define inst points in define function ?
  
   // The functions cannot be verified until all of them have been seen
   // because calls out of each function must be tagged as calls to user
   // functions or call to "library" functions
  
   //
   // sort the modules by address into a vector to allow a binary search to 
   // determine the module that a symbol will map to -- this 
   // may be bsd specific....
   //
   pdvector <Symbol> tmods;
  
   for (SymbolIter symIter(linkedFile); symIter; symIter++) {
      const Symbol &lookUp = symIter.currval();
      if (lookUp.type() == Symbol::PDST_MODULE) {
        
         const pdstring &lookUpName = lookUp.name();
         const char *str = lookUpName.c_str();

         //XXX
         //fprintf(stderr,"symbol %s\n",str);

         assert(str);
         int ln = lookUpName.length();
          
         // directory definition -- ignored for now
         if (str[ln-1] != '/') {
            tmods.push_back(lookUp);
         }
      }
      // As a side project, fill knownSymAddrs
      knownSymAddrs[lookUp.addr()] = 0; // 0 is a dummy value
   }
  
   // sort the modules by address
   statusLine("sorting modules");
   VECTOR_SORT(tmods, symbol_compare);
  
   // remove duplicate entries -- some .o files may have the same 
   // address as .C files.  kludge is true for module symbols that 
   // I am guessing are modules
   pdvector<Symbol> uniq;
  
   unsigned int num_zeros = 0;
   // must use loop+1 not mods.size()-1 since it is an unsigned compare
   //  which could go negative - jkh 5/29/95
   for (unsigned loop=0; loop < tmods.size(); loop++) {
      if (tmods[loop].addr() == 0) num_zeros++;
      if ((loop+1 < tmods.size()) && 
          (tmods[loop].addr() == tmods[loop+1].addr())) {
         if (!tmods[loop].kludge())
            tmods[loop+1] = tmods[loop];
      } 
      else {
         uniq.push_back(tmods[loop]);
      }
   }
   // avoid case where all (ELF) module symbols have address zero
   
   if (num_zeros == tmods.size()) { 
     uniq.resize(0);
   }
  

#if defined(sparc_sun_solaris2_4) \
 || defined(i386_unknown_solaris2_5) \
 || defined(rs6000_ibm_aix4_1)
   // make sure we're using the right demangler

   nativeCompiler = parseCompilerType(&linkedFile);
#endif

   // define all of the functions
   statusLine("winnowing functions");
  
   // a vector to hold all created functions until they are properly classified
   pdvector<image_func *> raw_funcs; 

   // define all of the functions, this also defines all of the modules
   if (!symbolsToFunctions(uniq, &raw_funcs)) {
      err = true;
      return;
   }
  
   // wait until all modules are defined before applying languages to
   // them we want to do it this way so that module information comes
   // from the function symbols, first and foremost, to avoid any
   // internal module-function mismatching.
  
   // get Information on the language each modules is written in
   // (prior to making modules)
   dictionary_hash<pdstring, supportedLanguages> mod_langs(pdstring::hash);
   getModuleLanguageInfo(&mod_langs);
   setModuleLanguages(&mod_langs);

   // Once languages are assigned, we can build demangled names (in
   // the wider sense of demangling which includes stripping _'s from
   // fortran names -- this is why language information must be
   // determined before this step).

   // Also identifies aliases (multiple names with equal addresses)

   if (!buildFunctionLists(raw_funcs)) {
     err = true;
     return;
   }

   // And symtab variables
   addSymtabVariables();

   parseState_ = symtab;
   
   // We now go through each function to identify and build lists
   // of instrumentation points (entry, exit, and call site). For
   // platforms that support stripped binary parsing, this pass
   // is also used to fix function sizes and identify functions not in
   // the symbol table.
   
#ifdef CHECK_ALL_CALL_POINTS
   statusLine("checking call points");
   checkAllCallPoints();
#endif

}

image::~image() 
{
  // Doesn't do anything yet, moved here so we don't mess with symtab.h
  // Only called if we fail to create a process.
}

pdvector<image_func *> *
pdmodule::findFunction( const pdstring &name, pdvector<image_func *> * found ) {
  assert( found != NULL );
  
  if( allFunctionsByPrettyName.defines( name ) ) {
    pdvector< image_func * > * prettilyNamedFunctions =
      allFunctionsByPrettyName.get( name );
    for( unsigned int i = 0; i < prettilyNamedFunctions->size(); i++ ) {
      found->push_back( (*prettilyNamedFunctions)[i] );
    }
    exec()->analyzeIfNeeded();
    return found;
  }
  
  if( allFunctionsByMangledName.defines( name ) ) {
    pdvector< image_func *> * mangledNameFunctions = 
      allFunctionsByMangledName.get(name);
    for (unsigned int j = 0; j < mangledNameFunctions->size(); j++) {
      found->push_back( (*mangledNameFunctions)[j]); 
    }
    exec()->analyzeIfNeeded();
    return found;
  }
  return NULL;
} /* end findFunction() */


#if !defined(i386_unknown_nt4_0) \
 && !defined(mips_unknown_ce2_11)

/* private refactoring functions */
typedef pdpair< pdstring, image_func * > nameFunctionPair;
typedef pdpair< pdstring, pdvector< image_func * > * > nameFunctionListPair;

void runCompiledRegexOn( regex_t * compiledRegex, pdvector< nameFunctionPair > * nameToFunctionMap, pdvector< image_func * > * found ) {
	int status = 0;
	for( unsigned int i = 0; i < nameToFunctionMap->size(); i++ ) {
		const char * name = (*nameToFunctionMap)[i].first.c_str();
		status = regexec( compiledRegex, name, 1, NULL, 0 );
		if( status == REG_NOMATCH ) { continue; }
		if( status != 0 ) {
			char errorString[80];
			regerror( status, compiledRegex, errorString, 80 );
			bperr("runCompiledRegexOn() regular expression execution error: '%s'\n", errorString ); 
			return;
			}

		found->push_back( (*nameToFunctionMap)[i].second );
		}
	} /* end runCompiledRegexOn() */

void runCompiledRegexOnList( regex_t * compiledRegex, pdvector< nameFunctionListPair > * nameToFunctionListMap, pdvector< image_func * > * found ) {
	int status = 0;
	for( unsigned int i = 0; i < nameToFunctionListMap->size(); i++ ) {
		const char * name = (*nameToFunctionListMap)[i].first.c_str();
		status = regexec( compiledRegex, name, 1, NULL, 0 );
		if( status == REG_NOMATCH ) { continue; }
		if( status != 0 ) {
			char errorString[80];
			regerror( status, compiledRegex, errorString, 80 );
			bperr("runCompiledRegexOn() regular expression execution error: '%s'\n", errorString ); 
			return;
			}

		pdvector< image_func * > * matchingFunctions = (*nameToFunctionListMap)[i].second;
		for( unsigned int j = 0; j < matchingFunctions->size(); j++ ) {
			found->push_back( (*matchingFunctions)[j] );
			}
		}
	} /* end runCompiledRegexOnList() */
#endif

pdvector<image_func *> *
pdmodule::findFunctionFromAll(const pdstring &name,
			      pdvector<image_func *> *found, 
			      bool regex_case_sensitive,
               bool dont_use_regex) 
{	    
  /* Are we doing a regex search? */
  if( dont_use_regex || NULL == strpbrk( name.c_str(), REGEX_CHARSET )) {
    /* Checks pretty and mangled instrumentable function names. */
    if( NULL != findFunction( name, found ) && found->size() != 0 ) {
      exec()->analyzeIfNeeded();
      return found;
    }
    
    return NULL;
  }
  
  /* We're doing a regular expression search, which is not yet implemented
     on Microsoft platforms. */
#if !defined(i386_unknown_nt4_0) \
 && !defined(mips_unknown_ce2_11)
  regex_t comp_pat;
  int err = 0;
  int cflags = REG_NOSUB | REG_EXTENDED;
  
  if( !regex_case_sensitive ) {
    cflags |= REG_ICASE;
  }
  
  if (0 != (err = regcomp( &comp_pat, name.c_str(), cflags ) ) ) {
    char errbuf[80];
    regerror( err, &comp_pat, errbuf, 80 );
    cerr << __FILE__ << ":" << __LINE__ << ":  REGEXEC ERROR: "<< errbuf << endl;
    return NULL;
  }
  
  /* Run the regular expression against, in order:
     the pretty names of instrumentable functions,
     the mangled names of instrumentable functions,
     the pretty names of uninstrumentable functions,
     and the mangled names of uninstrumentable functions,
     returning as soon as any regex succceeds.  (Note:
     this is wrong, but follows the previous semantics.) */
  
  pdvector< nameFunctionListPair > instrumentablePrettyPairs = allFunctionsByPrettyName.keysAndValues();
  runCompiledRegexOnList( & comp_pat, & instrumentablePrettyPairs, found );
  if( found->size() != 0 ) { 
    regfree( & comp_pat ); 
    exec()->analyzeIfNeeded();
    return found; 
  }
  
  pdvector< nameFunctionListPair > instrumentableMangledPairs = allFunctionsByMangledName.keysAndValues();
  runCompiledRegexOnList( & comp_pat, & instrumentableMangledPairs, found );
  if( found->size() != 0 ) { 
    regfree( & comp_pat ); 
    exec()->analyzeIfNeeded();
    return found; 
  }
  
  regfree( & comp_pat );
#endif
  
  /* We didn't find anything. */
  return NULL;
} /* end findFunctionFromAll() */

pdvector<image_func *> *pdmodule::findFunctionByMangled( const pdstring &name )
{
  /* By inference from the previous version, we're not interested
     in the uninstrumentable functions. */
  if( allFunctionsByMangledName.defines( name ) ) {
    exec()->analyzeIfNeeded();
    return allFunctionsByMangledName[name];
  }
  else {
    return NULL;
  }
}

void pdmodule::dumpMangled(pdstring &prefix) const
{
  cerr << fileName() << "::dumpMangled("<< prefix << "): " << endl;
  
  for (unsigned i = 0; i < allUniqueFunctions.size(); i++) {
      image_func * pdf = allUniqueFunctions[i];
      if( ! strncmp( pdf->symTabName().c_str(), prefix.c_str(), strlen( prefix.c_str() ) ) ) {
          cerr << pdf->symTabName() << " ";
      }
      else {
          // bperr( "%s is not a prefix of %s\n", prefix, pdf->symTabName().c_str() );
      }
  }
  cerr << endl;
}

#ifdef CHECK_ALL_CALL_POINTS
void image::checkAllCallPoints() {
  dictionary_hash_iter<pdstring, pdmodule*> di(modsByFullName);
  pdstring s; pdmodule *mod;
  while (di.next(s, mod))
    mod->checkAllCallPoints();
}
#endif
pdmodule *image::getOrCreateModule(const pdstring &modName, 
				   const Address modAddr) {
    pdmodule *fm = findModule(modName);
    if (fm) return fm;

    const char *str = modName.c_str();
    int len = modName.length();
    //    assert(len>0);
    if (!len) {
        cerr << "WARNING: module with no name!" << endl;
        return findModule("DEFAULT_MODULE");
    }
    // TODO ignore directory definitions for now
    if (str[len-1] == '/') 
        return NULL;
    return (newModule(modName, modAddr, lang_Unknown));
}


/*********************************************************************/
/**** Function lookup (by name or address) routines               ****/
/*********************************************************************/

// Find the function that occupies the given offset. ONLY LOOKS IN THE
// IMAGE, and does not consider relocated functions. Those must be searched
// for on the process level (as relocated functions are per-process)
codeRange *image::findCodeRangeByOffset(const Address &offset) {
    codeRange *range;
    if (!funcsByRange.find(offset, range)) {
        return NULL;
    }
    analyzeIfNeeded();
    return range;
}

image_func *image::findFuncByOffset(const Address &offset)  {
    codeRange *range = findCodeRangeByOffset(offset);
    image_func *func = range->is_image_func();

    return func;
}

// Similar to above, but checking by entry (only). Useful for 
// "who does this call" lookups
image_func *image::findFuncByEntry(const Address &entry) {

    image_func *func;
    if (funcsByEntryAddr.find(entry, func)) {
      analyzeIfNeeded();
      return func;
    } 
    else
      return NULL;
}

// Return the vector of functions associated with a pretty (demangled) name
// Very well might be more than one!

const pdvector <image_func *> *image::findFuncVectorByPretty(const pdstring &name)
{

    //    fprintf(stderr,"findFuncVectorByPretty %s\n",name.c_str());
#ifdef IBM_BPATCH_COMPAT_STAB_DEBUG
  bperr( "%s[%d]:  inside findFuncVectorByPretty\n", __FILE__, __LINE__);
#endif
  if (funcsByPretty.defines(name)) {
    analyzeIfNeeded();
    return funcsByPretty[name];
  }

  return NULL;
}

// Return the vector of functions associated with a mangled name
// Very well might be more than one! -- multiple static functions in different .o files

const pdvector <image_func *> *image::findFuncVectorByMangled(const pdstring &name)
{

    //    fprintf(stderr,"findFuncVectorByMangled %s\n",name.c_str());
#ifdef IBM_BPATCH_COMPAT_STAB_DEBUG
  bperr( "%s[%d]:  inside findFuncVectorByMangled\n", __FILE__, __LINE__);
#endif
  if (funcsByMangled.defines(name)) {
      analyzeIfNeeded();
      return funcsByMangled[name];
  }
  
  return NULL;
}

const pdvector <image_variable *> *image::findVarVectorByPretty(const pdstring &name)
{
    //    fprintf(stderr,"findVariableVectorByPretty %s\n",name.c_str());
#ifdef IBM_BPATCH_COMPAT_STAB_DEBUG
  bperr( "%s[%d]:  inside findVariableVectorByPretty\n", __FILE__, __LINE__);
#endif
  if (varsByPretty.defines(name)) {
      analyzeIfNeeded();
      return varsByPretty[name];
  }
  return NULL;
}

const pdvector <image_variable *> *image::findVarVectorByMangled(const pdstring &name)
{
    //    fprintf(stderr,"findVariableVectorByPretty %s\n",name.c_str());
#ifdef IBM_BPATCH_COMPAT_STAB_DEBUG
  bperr( "%s[%d]:  inside findVariableVectorByPretty\n", __FILE__, __LINE__);
#endif
  if (varsByMangled.defines(name)) {
      analyzeIfNeeded();
      return varsByMangled[name];
  }
  return NULL;
}


//  image::findOnlyOneFunction(const pdstring &name)
//  
//  searches for function with name <name> and fails if it finds more than
//  one match.
//
//  In order to be as comprehensive as possible, if no <name> is found in the 
//  "pretty" list, a search on the mangled list is performed.  Due to
//  duplication between many pretty and mangled names, this function does not
//  care about the same name appearing in both the pretty and mangled lists.
image_func *image::findOnlyOneFunction(const pdstring &name) {
  const pdvector<image_func *> *pdfv;

  pdfv = findFuncVectorByPretty(name);
  if (pdfv != NULL && pdfv->size() > 0) {
    // fail if more than one match
    if (pdfv->size() > 1) {
      cerr << __FILE__ << ":" << __LINE__ << ": findOnlyOneFunction(" << name
	   << ")...  found more than one... failing... " << endl;
      return NULL;
    }
    analyzeIfNeeded();
    return (*pdfv)[0];
  }

  pdfv = findFuncVectorByMangled(name);
  if (pdfv != NULL && pdfv->size() > 0) {
    if (pdfv->size() > 1) {
      cerr << __FILE__ << ":" << __LINE__ << ": findOnlyOneFunction(" << name
	   << ")...  found more than one... failing... " << endl;
      return NULL;
    }
    analyzeIfNeeded();
    return (*pdfv)[0];
  }
  
  return NULL;
}

#if 0
#if !defined(i386_unknown_nt4_0) \
 && !defined(mips_unknown_ce2_11) // no regex for M$
int image::findFuncVectorByPrettyRegex(pdvector<image_func *> *found, pdstring pattern,
				       bool case_sensitive)
{
  // This function compiles regex patterns and then calls its overloaded variant,
  // which does not.  This behavior is desirable so we can avoid re-compiling
  // expressions for broad searches (across images) -- allowing for higher level
  // functions to compile expressions just once. jaw 01-03


  regex_t comp_pat;
  int err, cflags = REG_NOSUB;
  
  if( !case_sensitive )
    cflags |= REG_ICASE;
  
  if (0 == (err = regcomp( &comp_pat, pattern.c_str(), cflags ))) {
    int ret = findFuncVectorByPrettyRegex(found, &comp_pat); 
    regfree(&comp_pat);
    analyzeIfNeeded();
    return ret;  
  }
  
  // errors fall through
  char errbuf[80];
  regerror( err, &comp_pat, errbuf, 80 );
  cerr << __FILE__ << ":" << __LINE__ << ":  REGEXEC ERROR: "<< errbuf << endl;
  
  return -1;
}

int image::findFuncVectorByMangledRegex(pdvector<image_func *> *found, pdstring pattern,
					bool case_sensitive)
{
  // Almost identical to its "Pretty" counterpart
  regex_t comp_pat;
  int err, cflags = REG_NOSUB;
  
  if( !case_sensitive )
    cflags |= REG_ICASE;
  
  if (0 == (err = regcomp( &comp_pat, pattern.c_str(), cflags ))) {
    int ret = findFuncVectorByMangledRegex(found, &comp_pat); 
    regfree(&comp_pat);

    analyzeIfNeeded();
    return ret;
  }
  // errors fall through
  char errbuf[80];
  regerror( err, &comp_pat, errbuf, 80 );
  cerr << __FILE__ << ":" << __LINE__ << ":  REGEXEC ERROR: "<< errbuf << endl;
  
  return -1;
}
#endif

#endif

pdvector<image_func *> *image::findFuncVectorByPretty(functionNameSieve_t bpsieve, 
						       void *user_data,
						       pdvector<image_func *> *found)
{
  pdvector<pdvector<image_func *> *> result;
  //result = funcsByPretty.linear_filter(bpsieve, user_data);
  dictionary_hash_iter <pdstring, pdvector<image_func*>*> iter(funcsByPretty);
  pdstring fname;
  pdvector<image_func *> *fmatches;
  while (iter.next(fname, fmatches)) {
    if ((*bpsieve)(iter.currkey().c_str(), user_data)) {
      result.push_back(fmatches);
    }
  }

  // Need to consolodate vector of vectors into just one vector  
  // This is wasteful in general, but hopefully result.size() is small
  // Besides, a more efficient approach would probably require different
  // data structs
  for (unsigned int i = 0; i < result.size(); ++i) 
    for (unsigned int j = 0; j < result[i]->size(); ++j) 
      found->push_back((*result[i])[j]);

  if (found->size()) {
    analyzeIfNeeded();
    return found;
  }
  return NULL;
}

pdvector<image_func *> *image::findFuncVectorByMangled(functionNameSieve_t bpsieve, 
							void *user_data,
							pdvector<image_func *> *found)
{

  pdvector<pdvector<image_func *> *> result;
  // result = funcsByMangled.linear_filter(bpsieve, user_data);

  dictionary_hash_iter <pdstring, pdvector<image_func*>*> iter(funcsByMangled);
  pdstring fname;
  pdvector<image_func *> *fmatches;
  while (iter.next(fname, fmatches)) {
    if ((*bpsieve)(fname.c_str(), user_data)) {
      result.push_back(fmatches);
    }
  }


  // Need to consolodate vector of vectors into just one vector  
  // This is wasteful in general, but hopefully result.size() is small
  // Besides, a more efficient approach would probably require different
  // data structs
  for (unsigned int i = 0; i < result.size(); ++i) 
    for (unsigned int j = 0; j < result[i]->size(); ++j) 
      found->push_back((*result[i])[j]);

  if (found->size()) {
    analyzeIfNeeded();
    return found;
  }
  return NULL;
}

#if 0
int image::findFuncVectorByPrettyRegex(pdvector<image_func *> *found, regex_t *comp_pat)
{
  //  This is a bit ugly in that it has to iterate through the entire dict hash 
  //  to find matches.  If this turns out to be a popular way of searching for
  //  functions (in particular "name*"), we might consider adding a data struct that
  //  preserves the pdstring ordering realation to make this O(1)-ish in that special case.
  //  jaw 01-03

  pdvector<pdvector<image_func *> *> result;
  //  result = funcsByPretty.linear_filter(&regex_filter_func, (void *) comp_pat);

  dictionary_hash_iter <pdstring, pdvector<image_func*>*> iter(funcsByPretty);
  pdstring fname;
  pdvector<image_func *> *fmatches;
  while (iter.next(fname, fmatches)) {
    if ((*regex_filter_func)(fname.c_str(), comp_pat)) {
      result.push_back(fmatches);
    }
  }

  // Need to consolodate vector of vectors into just one vector  
  // This is wasteful in general, but hopefully result.size() is small
  // Besides, a more efficient approach would probably require different
  // data structs
  //cerr <<"pretty regex result.size() = " << result.size() <<endl;
  for (unsigned int i = 0; i < result.size(); ++i) 
    for (unsigned int j = 0; j < result[i]->size(); ++j) 
      found->push_back((*result[i])[j]);

  if (found->size()) {
    analyzeIfNeeded();
  }
  return found->size();
}

int image::findFuncVectorByMangledRegex(pdvector<image_func *> *found, regex_t *comp_pat)
{
  // almost identical to its "Pretty" counterpart.

  pdvector<pdvector<image_func *> *> result;
  //result = funcsByMangled.linear_filter(&regex_filter_func, (void *) comp_pat);
  //cerr <<"mangled regex result.size() = " << result.size() <<endl;
  dictionary_hash_iter <pdstring, pdvector<image_func*>*> iter(funcsByMangled);
  pdstring fname;
  pdvector<image_func *> *fmatches;
  while (iter.next(fname, fmatches)) {
    if ((*regex_filter_func)(fname.c_str(), comp_pat)) {
      result.push_back(fmatches);
    }
  }

  for (unsigned int i = 0; i < result.size(); ++i) 
    for (unsigned int j = 0; j < result[i]->size(); ++j) 
      found->push_back((*result[i])[j]);
  
  if (found->size()) {
    analyzeIfNeeded();
  }
  return found->size();
}
#endif // !windows

// Returns TRUE if module belongs to a shared library, and FALSE otherwise

bool pdmodule::isShared() const { 
  return !exec_->isAOut();
}

/* Instrumentable-only, by the last version's source. */
const pdvector< image_func * >  &pdmodule::getFunctions()  {
  exec()->analyzeIfNeeded();
  return allUniqueFunctions;
} /* end getFunctions() */

void pdmodule::addFunction( image_func * func ) {
	allUniqueFunctions.push_back( func );

	for(	unsigned pretty_iter = 0; 
			pretty_iter < func->prettyNameVector().size();
			pretty_iter++) {
		pdstring pretty_name = func->prettyNameVector()[pretty_iter];
		pdvector<image_func *> * funcsByPrettyEntry = NULL;
		
		// Ensure a vector exists
		if( ! allFunctionsByPrettyName.find( pretty_name, 
                                                     funcsByPrettyEntry ) ) {
			funcsByPrettyEntry = new pdvector< image_func * >;
			allFunctionsByPrettyName[pretty_name] = funcsByPrettyEntry;
			}
		(*funcsByPrettyEntry).push_back( func );
		}
  
	// And multiple symtab names...
	for(	unsigned symtab_iter = 0; 
			symtab_iter < func->symTabNameVector().size();
			symtab_iter++) {
		pdstring symtab_name = func->symTabNameVector()[symtab_iter];
		pdvector< image_func * > * scratchvec;

		if ( allFunctionsByMangledName.find( symtab_name, scratchvec ) ) {
			bool newAddress = true;
			pdvector<image_func *> * mangleds = allFunctionsByMangledName[ symtab_name ];
			for( unsigned i = 0; i < mangleds->size(); i++ ) {
				if( func->getOffset() == (*mangleds)[i]->getOffset() ) { newAddress = false; }
				} /* end iteration over existing vector */
			if( newAddress ) { mangleds->push_back( func ); }
			} /* end if mangled name already existed */
		else {
			pdvector<image_func *> * newvec = new pdvector<image_func *>;
			(* newvec).push_back( func );
			allFunctionsByMangledName[symtab_name] = newvec;
			}
	  	} /* end iteration over symtab names */
	} /* end addFunction() */

void pdmodule::addTypedPrettyName(image_func *func, const char *prettyName) {
  // Add a new pretty name to our lists
  
  pdvector<image_func *> *funcsByPrettyEntry = NULL;
  // Ensure a vector exists
  if (!allFunctionsByPrettyName.find(pdstring(prettyName),
				     funcsByPrettyEntry)) {
    funcsByPrettyEntry = new pdvector<image_func*>;
    allFunctionsByPrettyName[pdstring(prettyName)] = funcsByPrettyEntry;
  }
  (*funcsByPrettyEntry).push_back(func);
}

  
void pdmodule::removeFunction(image_func *func) {
  pdvector <image_func *> newUniqueFuncs;
  for (unsigned i = 0; i < allUniqueFunctions.size(); i++) {
    if (allUniqueFunctions[i] != func)
      newUniqueFuncs.push_back(allUniqueFunctions[i]);
  }
  allUniqueFunctions = newUniqueFuncs;

  for (unsigned j = 0; j < func->symTabNameVector().size(); j++) {
    pdvector<image_func *> *temp_vec;
    if (allFunctionsByMangledName.find(func->symTabNameVector()[j],
				       temp_vec)) {
      pdvector<image_func *> *new_vec = new pdvector<image_func *>;
      for (unsigned l = 0; l < temp_vec->size(); l++) {
	if ((*temp_vec)[l] != func)
	  new_vec->push_back((*temp_vec)[l]);
      }
      allFunctionsByMangledName[func->symTabNameVector()[j]] = new_vec;
      delete temp_vec;
    }
  }
  
  for (unsigned l = 0; l < func->prettyNameVector().size(); l++) {
    pdvector<image_func *> *temp_vec;
    if (allFunctionsByPrettyName.find(func->prettyNameVector()[l],
				      temp_vec)) {
      pdvector<image_func *> *new_vec = new pdvector<image_func *>;
      for (unsigned k = 0; k < temp_vec->size(); k++) {
	if ((*temp_vec)[k] != func)
	  new_vec->push_back((*temp_vec)[k]);
      }
      allFunctionsByPrettyName[func->prettyNameVector()[l]] = new_vec;
      delete temp_vec;
    }
  }
}

void *image::getPtrToData(Address offset) const {
    if (!isData(offset)) return NULL;
    offset -= dataOffset_;
    unsigned char *inst = (unsigned char *)linkedFile.data_ptr();
    return (void *)(&inst[offset]);
}
    
// return a pointer to the instruction at address adr
void *image::getPtrToInstruction(Address offset) const {
   assert(isValidAddress(offset));
   if (isCode(offset)) {
      offset -= codeOffset_;
      unsigned char *inst = (unsigned char *)linkedFile.code_ptr();
      return (void *)(&inst[offset]);
   } else if (isData(offset)) {
      offset -= dataOffset_;
      unsigned char *inst = (unsigned char *)linkedFile.data_ptr();
      return (void *)(&inst[offset]);
   } else {
      abort();
      return 0;
   }
}


// Address must be in code or data range since some code may end up
// in the data segment
bool image::isValidAddress(const Address &where) const{
    return (instruction::isAligned(where) && (isCode(where) || isData(where)));
}

bool image::isAllocedAddress(const Address &where) const{
    return (instruction::isAligned(where) && (isAllocedCode(where) || isAllocedData(where)));
}

bool image::isCode(const Address &where)  const{
   return (linkedFile.code_ptr() && 
           (where >= codeOffset_) && (where < (codeOffset_+codeLen_)));
}

#if defined( arch_x86 )
bool image::isText( const Address &where) const
{
    return ( linkedFile.isText( where ) );
}
#endif
bool image::isData(const Address &where)  const{
   return (linkedFile.data_ptr() && 
           (where >= dataOffset_) && (where < (dataOffset_+dataLen_)));
}

bool image::isAllocedCode(const Address &where)  const{
   return (linkedFile.code_ptr() && 
           (where >= codeValidStart_) && (where < codeValidEnd_));
}

bool image::isAllocedData(const Address &where)  const{
   return (linkedFile.data_ptr() && 
           (where >= dataValidStart_) && (where < dataValidEnd_));
}

bool image::symbol_info(const pdstring& symbol_name, Symbol &ret_sym) {

   /* We temporarily adopt the position that an image has exactly one
      symbol per name.  While local functions (etc) make this untrue, it
      dramatically minimizes the amount of rewriting. */
   pdvector< Symbol > symbols;
   linkedFile.get_symbols( symbol_name, symbols );
   if( symbols.size() == 1 ) {
       ret_sym = symbols[0];
       return true;
       } else if ( symbols.size() > 1 ) {
       return false;
    }

   return false;
}


bool image::findSymByPrefix(const pdstring &prefix, pdvector<Symbol> &ret) {
    unsigned start = ret.size();
    for (SymbolIter symIter(linkedFile); symIter; symIter++) {
        const Symbol &lookUp = symIter.currval();
        if (lookUp.name().prefixed_by(prefix))
            ret.push_back(symIter.currval());
    }
    return ret.size() > start;
}

 
