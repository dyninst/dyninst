/*
 * Copyright (c) 1996-2006 Barton P. Miller
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
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "common/h/debugOstream.h"
#include "common/h/Timer.h"
#include "symtabAPI/src/Object.h"
#include "symtabAPI/h/Dyn_Symtab.h"
#if !defined(os_windows)
#include "common/h/pathName.h"
#include <dlfcn.h>
#else
#include "windows.h"
#include <libxml/xmlversion.h>
#undef LIBXML_ICONV_ENABLED
#endif

#include <libxml/xmlwriter.h>

static string errMsg;
extern bool parseCompilerType(Object *);
bool regexEquiv( const string &str,const string &them, bool checkCase );
bool pattern_match( const char *p, const char *s, bool checkCase );
void pd_log_perror(const char *msg){
   errMsg = msg;
};

#if !defined(os_windows)
    //libxml2 functions
	void *hXML;
#else
	HINSTANCE hXML; 
#endif

xmlTextWriterPtr(*my_xmlNewTextWriterFilename)(const char *,int) = NULL; 
int(*my_xmlTextWriterStartDocument)(xmlTextWriterPtr, const char *, const char *, const char * ) = NULL;
int(*my_xmlTextWriterStartElement)(xmlTextWriterPtr, const xmlChar *) = NULL;
int(*my_xmlTextWriterWriteFormatElement)(xmlTextWriterPtr,const xmlChar *,const char *,...) = NULL;
int(*my_xmlTextWriterEndDocument)(xmlTextWriterPtr) = NULL;
void(*my_xmlFreeTextWriter)(xmlTextWriterPtr) = NULL;
int(*my_xmlTextWriterWriteFormatAttribute)(xmlTextWriterPtr, const xmlChar *,const char *,...) = NULL;
int(*my_xmlTextWriterEndElement)(xmlTextWriterPtr) = NULL;


// generateXML helper functions
bool generateXMLforSyms(xmlTextWriterPtr &writer, vector<Dyn_Symbol *> &everyUniqueFunction, vector<Dyn_Symbol *> &everyUniqueVariable, vector<Dyn_Symbol *> &modSyms, vector<Dyn_Symbol *> &notypeSyms);
bool generateXMLforSymbol(xmlTextWriterPtr &writer, Dyn_Symbol *sym);
bool generateXMLforExcps(xmlTextWriterPtr &writer, vector<Dyn_ExceptionBlock *> &excpBlocks);
bool generateXMLforRelocations(xmlTextWriterPtr &writer, vector<relocationEntry> &fbt);


static SymtabError serr;

vector<Dyn_Symtab *> Dyn_Symtab::allSymtabs;
 
SymtabError Dyn_Symtab::getLastSymtabError(){
 	return serr;
}

string Dyn_Symtab::printError(SymtabError serr)
{
 	switch (serr){
		case Obj_Parsing:
			return "Failed to parse the Object"+errMsg;
		case Syms_To_Functions:
			return "Failed to convert Symbols to Functions";
		case No_Such_Function:
			return "Function does not exist";
		case No_Such_Variable:
			return "Variable does not exist";
		case No_Such_Module:
			return "Module does not exist";
		case No_Such_Section:
			return "Section does not exist";
		case No_Such_Symbol:
			return "Symbol does not exist";
		case Not_A_File:
			return "Not a File. Call openArchive()";
		case Not_An_Archive:
			return "Not an Archive. Call openFile()";
		case Export_Error:
			return "Error Constructing XML"+errMsg;
		case Invalid_Flags:
			return "Flags passed are invalid.";
		default:
			return "Unknown Error";
	}		
}

DLLEXPORT unsigned Dyn_Symtab::getAddressWidth() const {
   return linkedFile->getAddressWidth();
}
 
DLLEXPORT bool Dyn_Symtab::isNativeCompiler() const {
   return nativeCompiler; 
}
 
DLLEXPORT Dyn_Symtab::Dyn_Symtab(){
}

DLLEXPORT bool Dyn_Symtab::isExec() const {
   return is_a_out; 
}

DLLEXPORT OFFSET Dyn_Symtab::codeOffset() const { 
   return codeOffset_;
}

DLLEXPORT OFFSET Dyn_Symtab::dataOffset() const { 
   return dataOffset_;
}

DLLEXPORT OFFSET Dyn_Symtab::dataLength() const { 
   return dataLen_;
} 

DLLEXPORT OFFSET Dyn_Symtab::codeLength() const { 
   return codeLen_;
}
 
DLLEXPORT void* Dyn_Symtab::code_ptr ()  const {
   return linkedFile->code_ptr(); 
}

DLLEXPORT void* Dyn_Symtab::data_ptr ()  const { 
   return linkedFile->data_ptr();
}

DLLEXPORT const char*  Dyn_Symtab::getInterpreterName() const {
   return linkedFile->interpreter_name();
}
 
DLLEXPORT OFFSET Dyn_Symtab::getEntryAddress() const { 
   return linkedFile->getEntryAddress(); 
}

DLLEXPORT OFFSET Dyn_Symtab::getBaseAddress() const {
   return linkedFile->getBaseAddress(); 
}
	
// TODO -- is this g++ specific
bool Dyn_Symtab::buildDemangledName( const string &mangled, 
                                     string &pretty,
                                     string &typed,
                                     bool nativeCompiler, 
                                     supportedLanguages lang )
{
   /* The C++ demangling function demangles MPI__Allgather (and other MPI__
    * functions with start with A) into the MPI constructor.  In order to
    * prevent this a hack needed to be made, and this seemed the cleanest
    * approach.
    */

 	if((mangled.length()>5) && (mangled.substr(0,5)==string("MPI__"))) { 
      return false;
  	}	  

  	/* If it's Fortran, eliminate the trailing underscores, if any. */
  	if(lang == lang_Fortran 
      || lang == lang_CMFortran 
      || lang == lang_Fortran_with_pretty_debug )
	{
      if( mangled[ mangled.length() - 1 ] == '_' ) 
		{
         char * demangled = strdup( mangled.c_str() );
         demangled[ mangled.length() - 1 ] = '\0';
         pretty = string( demangled );
          
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
  
#if !defined(os_windows)
   char *atat;
   if (NULL != (atat = strstr(mangled.c_str(), "@@"))) 
   {
      pretty = mangled.substr(0 /*start pos*/, 
                              (int)(atat - mangled.c_str())/*len*/);
      char msg[256];
      sprintf(msg, "%s[%d]: 'demangling' versioned symbol: %s, to %s",
              __FILE__, __LINE__, mangled.c_str(), pretty.c_str());
      //cerr << msg << endl;
      //logLine(msg);
      
      return true;
   }
#endif

  	bool retval = false;
  
  	/* Try demangling it. */
  	char * demangled = P_cplus_demangle( mangled.c_str(), nativeCompiler, false);
  	if (demangled) 
	{
      pretty = string( demangled );
      retval = true;
  	}
  
  	char *t_demangled = P_cplus_demangle(mangled.c_str(), nativeCompiler, true);
  	if (t_demangled && (strcmp(t_demangled, demangled) != 0)) 
	{
      typed = string(t_demangled);
      retval = true;
  	}

  	if (demangled)
      free(demangled);
  	if (t_demangled)
  		free(t_demangled);

  	return retval;
} /* end buildDemangledName() */

 
/*
 * Add all the functions (*) in the list of symbols to our data
 * structures. 
 *
 * We do a search for a "main" symbol (a couple of variants), and
 * if found we flag this image as the executable (a.out). 
 */

bool Dyn_Symtab::symbolsToFunctions(vector<Dyn_Symbol *> *raw_funcs) 
{

#if defined(TIMED_PARSE)
   struct timeval starttime;
   gettimeofday(&starttime, NULL);
#endif

  	vector< Dyn_Symbol *> lookUps;
  	string symString;

	is_a_out = linkedFile->is_aout();

  	// JAW 02-03 -- restructured below slightly to get rid of multiple loops
  	// through entire symbol list
  
  	// find the real functions -- those with the correct type in the symbol table
  	for(SymbolIter symIter(*linkedFile); symIter;symIter++) 
	{
      Dyn_Symbol *lookUp = symIter.currval();
      const char *np = lookUp->getName().c_str();

      //parsing_printf("Scanning file: symbol %s\n", lookUp->getName().c_str());

      //fprintf(stderr,"np %s\n",np);

      if (linkedFile->isEEL() && np[0] == '.')
         /* ignore these EEL symbols; we don't understand their values */
	 		continue; 

     		
		if (lookUp->getType() == Dyn_Symbol::ST_FUNCTION) 
		{
         // /* DEBUG */ fprintf( stderr, "%s[%d]: considering function symbol %s in module %s\n", FILE__, __LINE__, lookUp.getName().c_str(), lookUp.getModuleName().c_str() );
            
         string msg;
         char tempBuffer[40];
         if (!isValidOffset(lookUp->getAddr())) 
			{
            sprintf(tempBuffer,"0x%lx",lookUp->getAddr());
            msg = string("Function ") + lookUp->getName() + string(" has bad address ")
               + string(tempBuffer);
            return false;
         }
			// Fill in _mods.
			Dyn_Module *newMod = getOrCreateModule(lookUp->getModuleName(),lookUp->getAddr());
			delete(lookUp->getModule());
			lookUp->setModule(newMod);
         raw_funcs->push_back(lookUp);
      }
		if(lookUp->getType() == Dyn_Symbol::ST_MODULE)
		{
			const string mangledName = symIter.currkey();
			char * unmangledName =
            		P_cplus_demangle( mangledName.c_str(), nativeCompiler, false);
		        if (unmangledName)
		            lookUp->addPrettyName(unmangledName, true);
			else
		            lookUp->addPrettyName(mangledName, true);
			Dyn_Module *newMod = getOrCreateModule(lookUp->getModuleName(),lookUp->getAddr());
			delete(lookUp->getModule());
			lookUp->setModule(newMod);
			addModuleName(lookUp,mangledName);
			modSyms.push_back(lookUp);
		}	
		else if(lookUp->getType() == Dyn_Symbol::ST_NOTYPE)
		{
			const string mangledName = symIter.currkey();
			char * unmangledName =
            		P_cplus_demangle( mangledName.c_str(), nativeCompiler, false);
            		if (unmangledName)
            		    lookUp->addPrettyName(unmangledName, true);
	    		else
  	          	    lookUp->addPrettyName(mangledName, true);
			notypeSyms.push_back(lookUp);
		}	
		else if(lookUp->getType() == Dyn_Symbol::ST_OBJECT)
		{
			const string mangledName = symIter.currkey();
#if 0
         fprintf(stderr, "Symbol %s, mod %s, addr 0x%x, type %d, linkage %d (obj %d, func %d)\n",
                 symInfo.name().c_str(),
                 symInfo.module().c_str(),
                 symInfo.addr(),
                 symInfo.type(),
                 symInfo.linkage(),
                 Dyn_Symbol::ST_OBJECT,
                 Dyn_Symbol::ST_FUNCTION);
#endif
#if !defined(os_windows)
         // Windows: variables are created with an empty module
         if (lookUp->getModuleName().length() == 0) 
         {
            //fprintf(stderr, "SKIPPING EMPTY MODULE\n");
            continue;
         }
#endif
			char * unmangledName =
            P_cplus_demangle( mangledName.c_str(), nativeCompiler, false);
			
			//Fill in _mods.
			Dyn_Module *newMod = getOrCreateModule(lookUp->getModuleName(),lookUp->getAddr());
			delete(lookUp->getModule());
			lookUp->setModule(newMod);
			Dyn_Symbol *var;
         //bool addToPretty = false;
         if (varsByAddr.find(lookUp->getAddr())!=varsByAddr.end()) 
			{
				var = varsByAddr[lookUp->getAddr()];
				
            // Keep the new mangled name
            var->addMangledName(mangledName);
            if (unmangledName)
               var->addPrettyName(unmangledName, true);
				else
               var->addPrettyName(mangledName, true);
				
         }
         else
			{
				var = lookUp;
            varsByAddr[lookUp->getAddr()] = var;
            if (unmangledName)
					var->addPrettyName(unmangledName, true);
				else
               var->addPrettyName(mangledName, true);
            everyUniqueVariable.push_back(var);
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
  	cout << __FILE__ << ":" << __LINE__ <<": symbolsToFunctions took "<<dursecs <<" msecs" << endl;
#endif
  	return true;
}

//  setModuleLanguages is only called after modules have been defined.
//  it attempts to set each module's language, information which is needed
//  before names can be demangled.
void Dyn_Symtab::setModuleLanguages(hash_map<string, supportedLanguages> *mod_langs)
{
 	if (!mod_langs->size())
		return;  // cannot do anything here
  	//  this case will arise on non-stabs platforms until language parsing can be introduced at this level
  	vector<Dyn_Module *> *modlist;
  	Dyn_Module *currmod = NULL;
  	modlist = &_mods;
  	//int dump = 0;

  	for (unsigned int i = 0;  i < modlist->size(); ++i)
	{
      currmod = (*modlist)[i];
      supportedLanguages currLang;
      if (currmod->isShared())
			continue;  // need to find some way to get shared object languages?
      if(mod_langs->find(currmod->fileName()) != mod_langs->end())
		{
         currLang = (*mod_langs)[currmod->fileName()];
			currmod->setLanguage(currLang);
      }
      else
		{
         //cerr << __FILE__ << __LINE__ << ":  module " << currmod->fileName() 
         //      				   << " not found in module<->language map" << endl;
         //dump = 1;
         //  here we should probably try to guess, based on filename conventions
      }
  	}
}

Dyn_Module *Dyn_Symtab::getOrCreateModule(const string &modName, 
                                          const OFFSET modAddr)
{
 	string nameToUse;
  	if (modName.length() > 0)
      nameToUse = modName;
   else
      nameToUse = "DEFAULT_MODULE";

   Dyn_Module *fm = new Dyn_Module();
   if (findModule(nameToUse,fm))
      return fm;
	delete fm;

   const char *str = nameToUse.c_str();
   int len = nameToUse.length();
   assert(len>0);

   // TODO ignore directory definitions for now
   if (str[len-1] == '/') 
      return NULL;
   return (newModule(nameToUse, modAddr, lang_Unknown));
}
 
Dyn_Module *Dyn_Symtab::newModule(const string &name, const OFFSET addr, supportedLanguages lang)
{
 	Dyn_Module *ret = new Dyn_Module();
 	// modules can be defined several times in C++ due to templates and
   //   in-line member functions.
   if (findModule(name,ret)) {
      return(ret);
   }

   //parsing_printf("=== image, creating new pdmodule %s, addr 0x%x\n",
   //				name.c_str(), addr);

   string fileNm, fullNm;
   fullNm = name;
   fileNm = extract_pathname_tail(name);

	// /* DEBUG */ fprintf( stderr, "%s[%d]: Creating new pdmodule '%s'/'%s'\n", FILE__, __LINE__, fileNm.c_str(), fullNm.c_str() );
   ret = new Dyn_Module(lang, addr, fullNm, this);
   modsByFileName[ret->fileName()] = ret;
   modsByFullName[ret->fullName()] = ret;
   _mods.push_back(ret);

   return(ret);
}

 
//buildFunctionLists() iterates through image_funcs and constructs demangled 
//names. Demangling was moved here (names used to be demangled as image_funcs 
//were built) so that language information could be obtained _after_ the 
//functions and modules were built, but before name demangling takes place.  
//Thus we can use language information during the demangling process.

bool Dyn_Symtab::buildFunctionLists(vector <Dyn_Symbol *> &raw_funcs)
{
#if defined(TIMED_PARSE)
  	struct timeval starttime;
  	gettimeofday(&starttime, NULL);
#endif
 	for (unsigned int i = 0; i < raw_funcs.size(); i++) 
	{
      Dyn_Symbol *raw = raw_funcs[i];
      Dyn_Module *rawmod = getOrCreateModule(raw->getModuleName(),raw->getAddr());
        
      assert(raw);
      assert(rawmod);
        
      // At this point we need to generate the following information:
      // A symtab name.
      // A pretty (demangled) name.
      // The symtab name goes in the global list as well as the module list.
      // Same for the pretty name.
      // Finally, check addresses to find aliases.
        
      string mangled_name = raw->getName();
      string working_name = mangled_name;
            
      string pretty_name = "<UNSET>";
      string typed_name = "<UNSET>";
#if !defined(os_windows)        
      //Remove extra stabs information
      const char *p = strchr(working_name.c_str(), ':');
      if( p )
		{
         unsigned nchars = p - mangled_name.c_str();
         working_name = string(mangled_name.c_str(), nchars);
      }
#endif        
      if (!buildDemangledName(working_name, pretty_name, typed_name,
                              nativeCompiler, rawmod->language())) 
		{
         pretty_name = working_name;
      }
        
      //parsing_printf("%s: demangled %s, typed %s\n",
      //	       mangled_name.c_str(),
      //	       pretty_name.c_str(),
      //	       typed_name.c_str());
        
      // Now, we see if there's already a function object for this
      // address. If so, add a new name; 
      Dyn_Symbol *possiblyExistingFunction = NULL;
      //funcsByEntryAddr.find(raw->getAddr(), possiblyExistingFunction);
      if (funcsByEntryAddr.find(raw->getAddr())!=funcsByEntryAddr.end()) 
      {
         vector<Dyn_Symbol *> &funcs = funcsByEntryAddr[raw->getAddr()];
	 unsigned flag = 0;
      	 for(unsigned int j=0;j<funcs.size();j++)
	 {
         	possiblyExistingFunction = funcsByEntryAddr[raw->getAddr()][j];
        	// On some platforms we see two symbols, one in a real module
         	// and one in DEFAULT_MODULE. Replace DEFAULT_MODULE with
         	// the real one
	 	Dyn_Module *use = getOrCreateModule(possiblyExistingFunction->getModuleName(),
         	             			               possiblyExistingFunction->getAddr());
         	if(!(*rawmod == *use))
	 	{
         	     if (rawmod->fileName() == "DEFAULT_MODULE")
         	         rawmod = use;
         	     if(use->fileName() == "DEFAULT_MODULE") 
         	     {
	        	  possiblyExistingFunction->setModuleName(string(rawmod->fullName()));
			  use = rawmod; 
		     }
         	}
            
         	if(*rawmod == *use)
		{
         		// Keep the new mangled name
         		possiblyExistingFunction->addMangledName(mangled_name);
         		if (pretty_name != "<UNSET>")
         		   possiblyExistingFunction->addPrettyName(pretty_name);
         		if (typed_name != "<UNSET>")
		 	   possiblyExistingFunction->addTypedName(typed_name);
	        	raw_funcs[i] = NULL;
	        	delete raw; // Don't leak
			flag = 1;	
			break;
		}	
	 }	
	 if(!flag)
	 {
          	 funcsByEntryAddr[raw->getAddr()].push_back(raw);
	       	 addFunctionName(raw, mangled_name, true);
          	 if (pretty_name != "<UNSET>")
		 	raw->addPrettyName(pretty_name, true);
           	 if (typed_name != "<UNSET>")
         		raw->addTypedName(typed_name, true);
	 }
      
      }
      else
		{
         funcsByEntryAddr[raw->getAddr()].push_back(raw);
         addFunctionName(raw, mangled_name, true);
         if (pretty_name != "<UNSET>")
				raw->addPrettyName(pretty_name, true);
         if (typed_name != "<UNSET>")
            raw->addTypedName(typed_name, true);
            
      }
   }
    
   // Now that we have a 1) unique and 2) demangled list of function
   // names, loop through once more and build the address range tree
   // and name lookup tables. 
   for (unsigned j = 0; j < raw_funcs.size(); j++) 
	{
      Dyn_Symbol *func = raw_funcs[j];
      if (!func) continue;
        
      // May be NULL if it was an alias.
      enterFunctionInTables(func, true);
   }
    
   // Conspicuous lack: inst points. We're delaying.
#if defined(TIMED_PARSE)
 	struct timeval endtime;
   gettimeofday(&endtime, NULL);
   unsigned long lstarttime = starttime.tv_sec * 1000 * 1000 + starttime.tv_usec;
   unsigned long lendtime = endtime.tv_sec * 1000 * 1000 + endtime.tv_usec;
   unsigned long difftime = lendtime - lstarttime;
   double dursecs = difftime/(1000 );
   cout << __FILE__ << ":" << __LINE__ <<": buildFunction Lists took "<<dursecs <<" msecs" << endl;
#endif
   return true;
} 

// Enter a function in all the appropriate tables
void Dyn_Symtab::enterFunctionInTables(Dyn_Symbol *func, bool wasSymtab)
{
 	if (!func)
		return;
    
    funcsByEntryAddr[func->getAddr()].push_back(func);
   // Functions added during symbol table parsing do not necessarily
   // have valid sizes set, and should therefor not be added to
   // the code range tree. They will be added after parsing. 
   /*if(!wasSymtab)
     {
     // TODO: out-of-line insertion here
     //if (func->get_size_cr())
     //	funcsByRange.insert(func);
     }*/
    
   everyUniqueFunction.push_back(func);
   if (wasSymtab)
      exportedFunctions.push_back(func);
   else
      createdFunctions.push_back(func);
}

/* 
   bool Dyn_Symtab::addSymbol(Dyn_Symbol *newsym)
   {
 	string sname = newsym->getName();
   #if !defined(os_windows)
   // Windows: variables are created with an empty module
   if (newsym->getModuleName().length() == 0) 
	{
   //fprintf(stderr, "SKIPPING EMPTY MODULE\n");
   return false;
	}
   #endif
	Dyn_Module *newMod = getOrCreateModule(sname, newsym->getAddr());
	delete(newsym->getModule());
	newsym->setModule(newMod);
   char * unmangledName = P_cplus_demangle(sname.c_str(), nativeCompiler,false);
	if(unmangledName)
	{
   newsym->addPrettyName(unmangledName, true);
   if(newsym->getType() == Dyn_Symbol::ST_FUNCTION)
   addFunctionName(newsym, sname, true);
   else if(newsym->getType() == Dyn_Symbol::ST_OBJECT)
   addVariableName(newsym, sname, true);
   else if(newsym->getType() == Dyn_Symbol::ST_MODULE)
   modSyms.push_back(newsym);
   else
   notypeSyms.push_back(newsym);
   }
	else
	{
   newsym->addPrettyName(sname, true);
   if(newsym->getType() == Dyn_Symbol::ST_FUNCTION)
   addFunctionName(newsym, sname, true);
   else if(newsym->getType() == Dyn_Symbol::ST_OBJECT)
   addVariableName(newsym, sname, true);
   else if(newsym->getType() == Dyn_Symbol::ST_MODULE)
   modSyms.push_back(newsym);
   else
   notypeSyms.push_back(newsym);
   }
	//linkedFile->symbols_[sname].push_back(newsym);
	return true;
   }
*/

bool Dyn_Symtab::addSymbol(Dyn_Symbol *newSym)
{
 	vector<string> names;
	char *unmangledName = NULL;
 	string sname = newSym->getName();
#if !defined(os_windows)
   // Windows: variables are created with an empty module
   if (newSym->getModuleName().length() == 0) 
	{
      //fprintf(stderr, "SKIPPING EMPTY MODULE\n");
      return false;
	}
#endif
	Dyn_Module *newMod = getOrCreateModule(sname, newSym->getAddr());
	delete(newSym->getModule());
	newSym->setModule(newMod);
	if(newSym->getAllPrettyNames().size() == 0)
      		unmangledName = P_cplus_demangle(sname.c_str(), nativeCompiler,false);
	if(newSym->getType() == Dyn_Symbol::ST_FUNCTION)
	{
		names = newSym->getAllMangledNames();
		for(unsigned i=0;i<names.size();i++)
			addFunctionName(newSym, names[i], true);
		names = newSym->getAllPrettyNames();
		for(unsigned i=0;i<names.size();i++)
			addFunctionName(newSym, names[i], false);
		names = newSym->getAllTypedNames();
		for(unsigned i=0;i<names.size();i++)
			addFunctionName(newSym, names[i], false);
	}
	else if(newSym->getType() == Dyn_Symbol::ST_OBJECT)
	{
		names = newSym->getAllMangledNames();
		for(unsigned i=0;i<names.size();i++)
			addVariableName(newSym, names[i], true);
		names = newSym->getAllPrettyNames();
		for(unsigned i=0;i<names.size();i++)
			addVariableName(newSym, names[i], false);
		names = newSym->getAllTypedNames();
		for(unsigned i=0;i<names.size();i++)
			addVariableName(newSym, names[i], false);
	}
	else if(newSym->getType() == Dyn_Symbol::ST_MODULE)
	{
		names = newSym->getAllMangledNames();
		for(unsigned i=0;i<names.size();i++)
			addModuleName(newSym, names[i]);
		names = newSym->getAllPrettyNames();
		for(unsigned i=0;i<names.size();i++)
			addModuleName(newSym, names[i]);
		modSyms.push_back(newSym);
	}	
	else
		notypeSyms.push_back(newSym);
	if(newSym->getAllPrettyNames().size() == 0)		// add the unmangledName if there are no prettyNames
	{
		if(unmangledName)
			newSym->addPrettyName(unmangledName, true);
		else
			newSym->addPrettyName(sname,true);
	}		
	return true;
}

void Dyn_Symtab::addFunctionName(Dyn_Symbol *func,
                                 const string newName,
                                 bool isMangled /*=false*/)
{    
   // Ensure a vector exists
   if (isMangled == false) {
    	if(funcsByPretty.find(newName)==funcsByPretty.end())
         funcsByPretty[newName] = new vector<Dyn_Symbol *>;
      funcsByPretty[newName]->push_back(func);    
   }
   else {
    	if(funcsByMangled.find(newName)==funcsByMangled.end())
         funcsByMangled[newName] = new vector<Dyn_Symbol *>;
      funcsByMangled[newName]->push_back(func);    
   }
}

void Dyn_Symtab::addVariableName(Dyn_Symbol *var,
                                 const string newName,
                                 bool isMangled /*=false*/)
{    
   // Ensure a vector exists
   if (isMangled == false) {
    	if(varsByPretty.find(newName)==varsByPretty.end())
         varsByPretty[newName] = new vector<Dyn_Symbol *>;
      varsByPretty[newName]->push_back(var);    
   }
   else {
    	if(varsByMangled.find(newName)==varsByMangled.end())
         varsByMangled[newName] = new vector<Dyn_Symbol *>;
      varsByMangled[newName]->push_back(var);    
   }
}

 
void Dyn_Symtab::addModuleName(Dyn_Symbol *mod, const string newName)
{
   if(modsByName.find(newName)==modsByName.end())
  	modsByName[newName] = new vector<Dyn_Symbol *>;
   modsByName[newName]->push_back(mod);    
}

Dyn_Symtab::Dyn_Symtab(string &filename,bool &err)
   : filename_(filename), is_a_out(false), main_call_addr_(0),
     nativeCompiler(false)
{
   linkedFile = new Object(filename, pd_log_perror);
	name_ = extract_pathname_tail(filename);
	err = extractInfo();
}

Dyn_Symtab::Dyn_Symtab(char *mem_image, size_t image_size, bool &err):
   is_a_out(false),
   main_call_addr_(0),
   nativeCompiler(false)
{
   linkedFile = new Object(mem_image, image_size, pd_log_perror);
  	err = extractInfo();
}

#if defined(os_aix)
Dyn_Symtab::Dyn_Symtab(string &filename, string &member_name, OFFSET offset, 
                       bool &err)
   : filename_(filename), member_name_(member_name), is_a_out(false),
     main_call_addr_(0), nativeCompiler(false)
{
   linkedFile = new Object(filename, member_name, offset, pd_log_perror);
	name_ = extract_pathname_tail(filename);
	err = extractInfo();
}
#else
Dyn_Symtab::Dyn_Symtab(string &, string &, OFFSET, bool &)
{
   assert(0);
}
#endif

#if defined(os_aix)
Dyn_Symtab::Dyn_Symtab(char *mem_image, size_t image_size, string &member_name,
                       OFFSET offset, bool &err)
   : member_name_(member_name), is_a_out(false), main_call_addr_(0),
     nativeCompiler(false)
{
   linkedFile = new Object(mem_image, image_size, member_name, offset, 
                           pd_log_perror);
  	err = extractInfo();
}
#else 
Dyn_Symtab::Dyn_Symtab(char *, size_t, string &, OFFSET, bool &)
{
   assert(0);
}
#endif

bool Dyn_Symtab::extractInfo()
{
#if defined(TIMED_PARSE)
   struct timeval starttime;
   gettimeofday(&starttime, NULL);
#endif
 	bool err = true;
	codeOffset_ = linkedFile->code_off();
	dataOffset_ = linkedFile->data_off();
   
   codeLen_ = linkedFile->code_len();
   dataLen_ = linkedFile->data_len();

   codeValidStart_ = linkedFile->code_vldS();
   codeValidEnd_ = linkedFile->code_vldE();
   dataValidStart_ = linkedFile->data_vldS();
   dataValidEnd_ = linkedFile->data_vldE();
	
	if (!codeLen_ || !linkedFile->code_ptr()) {
      serr = Obj_Parsing; 
      return false; 
   }

	no_of_sections = linkedFile->no_of_sections();
	no_of_symbols = linkedFile->no_of_symbols();

	sections_ = linkedFile->getAllSections();
	for(unsigned index=0;index<sections_.size();index++)
		secsByEntryAddr[sections_[index]->getSecAddr()] = sections_[index];

	/* insert error check here. check if parsed */
#if 0
	vector <Dyn_Symbol *> tmods;
	
  	SymbolIter symIter(linkedFile);

   for(;symIter;symIter++)
	{
      Dyn_Symbol *lookUp = symIter.currval();
      if (lookUp.getType() == Dyn_Symbol::ST_MODULE)
		{
         const string &lookUpName = lookUp->getName();
         const char *str = lookUpName.c_str();

         assert(str);
         int ln = lookUpName.length();
          
         // directory definition -- ignored for now
         if (str[ln-1] != '/')
            tmods.push_back(lookUp);
      }
   }
 
	// sort the modules by address
   //statusLine("sorting modules");
   //sort(tmods.begin(), tmods.end(), symbol_compare);
#if defined(TIMED_PARSE)
 	struct timeval endtime;
   gettimeofday(&endtime, NULL);
   unsigned long lstarttime = starttime.tv_sec * 1000 * 1000 + starttime.tv_usec;
   unsigned long lendtime = endtime.tv_sec * 1000 * 1000 + endtime.tv_usec;
   unsigned long difftime = lendtime - lstarttime;
   double dursecs = difftime/(1000 );
   cout << __FILE__ << ":" << __LINE__ <<": extract mods & sort took "<<dursecs <<" msecs" << endl;
#endif
  
   // remove duplicate entries -- some .o files may have the same 
   // address as .C files.  kludge is true for module symbols that 
   // I am guessing are modules
  
   unsigned int num_zeros = 0;
   // must use loop+1 not mods.size()-1 since it is an unsigned compare
   //  which could go negative - jkh 5/29/95
   for (unsigned loop=0; loop < tmods.size(); loop++)
	{
      if (tmods[loop].getAddr() == 0)
			num_zeros++;
      if ((loop+1 < tmods.size()) && (tmods[loop].getAddr() == tmods[loop+1].getAddr()))
         tmods[loop+1] = tmods[loop];
      else
         uniq.push_back(tmods[loop]);
   }
   // avoid case where all (ELF) module symbols have address zero
   
   if (num_zeros == tmods.size())
      uniq.resize(0);

#endif //if 0

#if defined(os_solaris) || defined(os_aix) || defined(os_linux)
   // make sure we're using the right demangler

   nativeCompiler = parseCompilerType(linkedFile);
   //parsing_printf("isNativeCompiler: %d\n", nativeCompiler);
#endif
  	 
   // define all of the functions
   //statusLine("winnowing functions");
  
   // a vector to hold all created functions until they are properly classified
   vector<Dyn_Symbol *> raw_funcs;
	
	// define all of the functions, this also defines all of the modules
	if (!symbolsToFunctions(&raw_funcs))
	{
      fprintf(stderr, "Error converting symbols to functions in file %s\n", filename_.c_str());
		err = false;
		serr = Syms_To_Functions;
      return false;
   }
	sort(raw_funcs.begin(),raw_funcs.end(),symbol_compare);
	
   // wait until all modules are defined before applying languages to
   // them we want to do it this way so that module information comes
   // from the function symbols, first and foremost, to avoid any
   // internal module-function mismatching.
  	
  	// get Information on the language each modules is written in
   // (prior to making modules)
   hash_map<string, supportedLanguages> mod_langs;
   linkedFile->getModuleLanguageInfo(&mod_langs);
   setModuleLanguages(&mod_langs);
	
	// Once languages are assigned, we can build demangled names (in
   // the wider sense of demangling which includes stripping _'s from
   // fortran names -- this is why language information must be
   // determined before this step).

   // Also identifies aliases (multiple names with equal addresses)

   if (!buildFunctionLists(raw_funcs))
	{
      fprintf(stderr, "Error building function lists in file %s\n", filename_.c_str());
      err = false;
		serr = Build_Function_Lists;
      return false;
   }

   // And symtab variables
   //addSymtabVariables();
	linkedFile->getAllExceptions(excpBlocks);
	return true;
}

Dyn_Symtab::Dyn_Symtab(const Dyn_Symtab& obj)
   : Dyn_LookupInterface()
{
 	filename_ = obj.filename_;
 	member_name_ = obj.member_name_;
	name_ = obj.name_;
   codeOffset_ = obj.codeOffset_;
   codeLen_ = obj.codeLen_;
	dataOffset_ = obj.dataOffset_;
	dataLen_ = obj.dataLen_;

   codeValidStart_ = obj.codeValidStart_;
   codeValidEnd_ = obj.codeValidEnd_;
	dataValidStart_ = obj.dataValidStart_;
   dataValidEnd_ = obj.dataValidEnd_;
        
	is_a_out = obj.is_a_out;;
   main_call_addr_ = obj.main_call_addr_; // address of call to main()

   nativeCompiler = obj.nativeCompiler;

   // data from the symbol table  //what should be done here??
   linkedFile = obj.linkedFile; 
	
	//sections
	no_of_sections = obj.no_of_sections;
	unsigned i;
	for(i=0;i<obj.sections_.size();i++)
		sections_.push_back(new Dyn_Section(*(obj.sections_[i])));
	for(i=0;i<sections_.size();i++)
		secsByEntryAddr[sections_[i]->getSecAddr()] = sections_[i];
	
	//symbols
	no_of_symbols = obj.no_of_symbols;
	
	for(i=0;i<obj.everyUniqueFunction.size();i++)
		everyUniqueFunction.push_back(new Dyn_Symbol(*(obj.everyUniqueFunction[i])));
	for(i=0;i<everyUniqueFunction.size();i++)
	{
 		funcsByEntryAddr[everyUniqueFunction[i]->getAddr()].push_back(everyUniqueFunction[i]);
		addFunctionName(everyUniqueFunction[i],everyUniqueFunction[i]->getName(),true);
		addFunctionName(everyUniqueFunction[i],everyUniqueFunction[i]->getPrettyName(),false);
	}	
	
	for(i=0;i<obj.everyUniqueVariable.size();i++)
		everyUniqueVariable.push_back(new Dyn_Symbol(*(obj.everyUniqueVariable[i])));
	for(i=0;i<everyUniqueVariable.size();i++)
	{
		varsByAddr[everyUniqueVariable[i]->getAddr()] = everyUniqueVariable[i];
		addVariableName(everyUniqueVariable[i],everyUniqueVariable[i]->getName(),true);
		addVariableName(everyUniqueVariable[i],everyUniqueVariable[i]->getPrettyName(),false);
	}

	for(i=0;i<obj._mods.size();i++)
		_mods.push_back(new Dyn_Module(*(obj._mods[i])));
	for(i=0;i<_mods.size();i++)
	{
		modsByFileName[_mods[i]->fileName()] = _mods[i];
		modsByFullName[_mods[i]->fullName()] = _mods[i];
	}
	for(i=0;i<modSyms.size();i++)
		modSyms.push_back(new Dyn_Symbol(*(modSyms[i])));
	
	for(i=0;i<notypeSyms.size();i++)
		notypeSyms.push_back(new Dyn_Symbol(*(notypeSyms[i])));
	
	for(i=0;i<excpBlocks.size();i++)
		excpBlocks.push_back(new Dyn_ExceptionBlock(*(obj.excpBlocks[i])));
}

bool Dyn_Symtab::findModule(const string &name, Dyn_Module *&ret)
{
   if (modsByFileName.find(name)!=modsByFileName.end()) 
	{
      ret = modsByFileName[name];
		return true;
   }
   else if (modsByFullName.find(name)!=modsByFullName.end()) 
	{
      ret = modsByFullName[name];
		return true;
   }
  
	serr = No_Such_Module;
	return false;
}

bool Dyn_Symtab::getAllSymbolsByType(vector<Dyn_Symbol *> &ret, Dyn_Symbol::SymbolType sType)
{
 	if(sType == Dyn_Symbol::ST_FUNCTION)
		return getAllFunctions(ret);
	else if(sType == Dyn_Symbol::ST_OBJECT)
		return getAllVariables(ret);
	else if(sType == Dyn_Symbol::ST_MODULE)
	{
		if(modSyms.size()>0)
		{
			ret =  modSyms;
			return true;
		}
		serr = No_Such_Symbol;
		return false;
			
	}
	else if(sType == Dyn_Symbol::ST_NOTYPE)
 	{
		if(notypeSyms.size()>0)
		{
			ret =  notypeSyms;
			return true;
		}
		serr = No_Such_Symbol;
		return false;
	}
	else
		return getAllSymbols(ret);
}
 
 
bool Dyn_Symtab::getAllFunctions(vector<Dyn_Symbol *> &ret)
{
 	if(everyUniqueFunction.size() > 0)
	{
		ret = everyUniqueFunction;
		return true;
	}	
	serr = No_Such_Function;
	return false;
}
 
bool Dyn_Symtab::getAllVariables(vector<Dyn_Symbol *> &ret)
{
 	if(everyUniqueVariable.size() > 0)
	{
		ret = everyUniqueVariable;
		return true;
	}	
	serr = No_Such_Variable;
	return false;
}

bool Dyn_Symtab::getAllSymbols(vector<Dyn_Symbol *> &ret)
{
 	vector<Dyn_Symbol *> temp;
	getAllFunctions(ret);
	getAllVariables(temp);
	ret.insert(ret.end(), temp.begin(), temp.end());
	temp.clear();
	for(unsigned i=0;i<modSyms.size();i++)
		ret.push_back(modSyms[i]);
	for(unsigned i=0;i<notypeSyms.size();i++)
		ret.push_back(notypeSyms[i]);
	if(ret.size() > 0)
		return true;
	serr = No_Such_Symbol;
	return false;
}

bool Dyn_Symtab::getAllModules(vector<Dyn_Module *> &ret)
{
 	if(_mods.size()>0)
	{
		ret = _mods;
		return true;
	}	
	serr = No_Such_Module;
	return false;
}

bool Dyn_Symtab::getAllSections(vector<Dyn_Section *>&ret)
{
 	if(sections_.size() > 0)
	{
		ret = sections_;
		return true;
	}
	return false;
}

bool Dyn_Symtab::getAllExceptions(vector<Dyn_ExceptionBlock *> &exceptions)
{
 	if(excpBlocks.size()>0)
	{
		exceptions = excpBlocks;
		return true;
	}	
	return false;
}

bool Dyn_Symtab::findException(OFFSET &addr, Dyn_ExceptionBlock &excp)
{
 	for(unsigned i=0; i<excpBlocks.size(); i++)
	{
		if(excpBlocks[i]->contains(addr))
		{
			excp = *(excpBlocks[i]);
			return true;
		}	
	}
	return false;
}

/**
 * Returns true if the Address range addr -> addr+size contains
 * a catch block, with excp pointing to the appropriate block
 **/
bool Dyn_Symtab::findCatchBlock(Dyn_ExceptionBlock &excp, OFFSET addr, unsigned size)
{
 	int min = 0;
	int max = excpBlocks.size();
	int cur = -1, last_cur;

   if (max == 0)
      return false;

   //Binary search through vector for address
	while (true)
	{
      last_cur = cur;
      cur = (min + max) / 2;

      if (last_cur == cur)
         return false;

      OFFSET curAddr = excpBlocks[cur]->catchStart();
      if ((curAddr <= addr && curAddr+size > addr) ||
          (size == 0 && curAddr == addr))
      {
         //Found it
	     	excp = *(excpBlocks[cur]);
	     	return true;
      }
      if (addr < curAddr)
         max = cur;
      else if (addr > curAddr)
	     	min = cur;
   }
}
 
bool Dyn_Symtab::findFuncByEntryOffset(const OFFSET &entry, vector<Dyn_Symbol *>& ret)
{
   if(funcsByEntryAddr.find(entry)!=funcsByEntryAddr.end()) {
        ret = funcsByEntryAddr[entry];
	return true;
   }
   serr = No_Such_Function;
   return false;
}
 
#if 0 
bool Dyn_Symtab::findSymbolByType(vector<Dyn_Symbol *> &ret, const string &name,
                                  Dyn_Symbol::SymbolType sType, unsigned flags)
{
 	switch(flags)
	{
		case IS_PRETTY|NOT_REGEX|NOT_CASE_SENSITIVE:
		case IS_PRETTY|NOT_REGEX|IS_CASE_SENSITIVE:
			return findSymbolByType(ret, name, sType);
		case IS_MANGLED|NOT_REGEX|NOT_CASE_SENSITIVE:
		case IS_MANGLED|NOT_REGEX|IS_CASE_SENSITIVE:
			return findSymbolByType(ret, name, sType, true);
		case IS_PRETTY|IS_REGEX|NOT_CASE_SENSITIVE:
			return findSymbolByType(ret, name, sType, false, true);
		case IS_PRETTY|IS_REGEX|IS_CASE_SENSITIVE:
			return findSymbolByType(ret, name, sType, false, true, true);
		case IS_MANGLED|IS_REGEX|NOT_CASE_SENSITIVE:
			return findSymbolByType(ret, name, sType, true, true);
		case IS_MANGLED|IS_REGEX|IS_CASE_SENSITIVE:
			return findSymbolByType(ret, name, sType, true, true, true);
		default:
			serr = Invalid_Flags;
			return false;
	}
}
#endif 
 
bool Dyn_Symtab::findSymbolByType(vector<Dyn_Symbol *> &ret, const string &name,
                                  Dyn_Symbol::SymbolType sType, bool isMangled,
                                  bool isRegex, bool checkCase)
{
	exportXML("file.xml");
 	if(sType == Dyn_Symbol::ST_FUNCTION)
		return findFunction(ret, name, isMangled, isRegex, checkCase);
	else if(sType == Dyn_Symbol::ST_OBJECT)
		return findVariable(ret, name, isMangled, isRegex, checkCase);
	else if(sType == Dyn_Symbol::ST_MODULE)
		return findMod(ret, name, isMangled, isRegex, checkCase);
	else if(sType == Dyn_Symbol::ST_NOTYPE)
	{
		unsigned start = ret.size(),i;
		if(!isMangled && !isRegex)
		{
		    for(i=0;i<notypeSyms.size();i++)
		    {
			if(notypeSyms[i]->getPrettyName() == name)
			    ret.push_back(notypeSyms[i]);
		    }	    
		}
		else if(isMangled&&!isRegex)
		{
		    for(i=0;i<notypeSyms.size();i++)
		    {
			if(notypeSyms[i]->getName() == name)
			    ret.push_back(notypeSyms[i]);
		    }	
		}
		else if(!isMangled&&isRegex)
		{
		    for(i=0;i<notypeSyms.size();i++)
		    {
			if(regexEquiv(name, notypeSyms[i]->getPrettyName(), checkCase))
		 	    ret.push_back(notypeSyms[i]);
		    }	
		}
		else
		{
		    for(i=0;i<notypeSyms.size();i++)
		    {
			if(regexEquiv(name, notypeSyms[i]->getName(), checkCase))
			    ret.push_back(notypeSyms[i]);
		    }	
		}
		if(ret.size() > start)
			return true;
		serr = No_Such_Symbol;
		return false;
	}
	else if(sType == Dyn_Symbol::ST_UNKNOWN)
	{
		findFunction(ret, name, isMangled, isRegex, checkCase);
		vector<Dyn_Symbol *>syms;
		findVariable(syms, name, isMangled, isRegex, checkCase);
		ret.insert(ret.end(), syms.begin(), syms.end());
		syms.clear();
		findSymbolByType(syms, name, Dyn_Symbol::ST_MODULE, isMangled, isRegex, checkCase);
		ret.insert(ret.end(), syms.begin(), syms.end());
		syms.clear();
		findSymbolByType(syms, name, Dyn_Symbol::ST_NOTYPE, isMangled, isRegex, checkCase);
		ret.insert(ret.end(), syms.begin(), syms.end());
		syms.clear();
		if(ret.size() > 0)
			return true;
		else
		{
			serr = No_Such_Symbol;
			return false;
		}
	}
	return false;
}
				
bool Dyn_Symtab::findFunction(vector <Dyn_Symbol *> &ret,const string &name, 
                              bool isMangled, bool isRegex, 
                              bool checkCase)
{
	if(!isMangled&&!isRegex)
		return findFuncVectorByPretty(name, ret);
	else if(isMangled&&!isRegex)
		return findFuncVectorByMangled(name, ret);
	else if(!isMangled&&isRegex)
		return findFuncVectorByPrettyRegex(name, checkCase, ret);
	else
		return findFuncVectorByMangledRegex(name, checkCase, ret);
}

bool Dyn_Symtab::findVariable(vector <Dyn_Symbol *> &ret, const string &name,
                              bool isMangled, bool isRegex,
                              bool checkCase)
{
	if(!isMangled&&!isRegex)
		return findVarVectorByPretty(name, ret);
	else if(isMangled&&!isRegex)
		return findVarVectorByMangled(name, ret);
	else if(!isMangled&&isRegex)
		return findVarVectorByPrettyRegex(name, checkCase, ret);
	else
		return findVarVectorByMangledRegex(name, checkCase, ret);
}

bool Dyn_Symtab::findMod(vector <Dyn_Symbol *> &ret, const string &name,
                            bool isMangled, bool isRegex,
                            bool checkCase)
{
	if(!isRegex)
	{
		if(modsByName.find(name)!=modsByName.end())
		{
			ret = *(modsByName[name]);
			return true;	
		}
		serr = No_Such_Module;
		return false;
	}
	else
		return findModByRegex(name, checkCase, ret);
}

// Return the vector of functions associated with a mangled name
// Very well might be more than one! -- multiple static functions in different .o files
bool Dyn_Symtab::findFuncVectorByPretty(const string &name, vector<Dyn_Symbol *> &ret)
{
 	if (funcsByPretty.find(name)!=funcsByPretty.end()) 
	{
		ret = *(funcsByPretty[name]);
		return true;	
  	}
	serr = No_Such_Function;
  	return false;
}
 
bool Dyn_Symtab::findFuncVectorByMangled(const string &name, vector<Dyn_Symbol *> &ret)
{
 	//fprintf(stderr,"findFuncVectorByMangled %s\n",name.c_str());
 	//#ifdef IBM_BPATCH_COMPAT_STAB_DEBUG
   //bperr( "%s[%d]:  inside findFuncVectorByMangled\n", FILE__, __LINE__);
	//#endif
  	if (funcsByMangled.find(name)!=funcsByMangled.end()) 
	{
		ret = *(funcsByMangled[name]);
		return true;	
  	}
	serr = No_Such_Function;
  	return false;
}
 
bool Dyn_Symtab::findVarVectorByPretty(const string &name, vector<Dyn_Symbol *> &ret)
{
 	//fprintf(stderr,"findVariableVectorByPretty %s\n",name.c_str());
	//#ifdef IBM_BPATCH_COMPAT_STAB_DEBUG
  	//bperr( "%s[%d]:  inside findVariableVectorByPretty\n", FILE__, __LINE__);
	//#endif
  	if (varsByPretty.find(name)!=varsByPretty.end()) 
	{
		ret = *(varsByPretty[name]);
		return true;	
  	}
	serr = No_Such_Variable;
  	return false;
}

bool Dyn_Symtab::findVarVectorByMangled(const string &name, vector<Dyn_Symbol *> &ret)
{
 	// fprintf(stderr,"findVariableVectorByPretty %s\n",name.c_str());
 	//#ifdef IBM_BPATCH_COMPAT_STAB_DEBUG
 	//bperr( "%s[%d]:  inside findVariableVectorByPretty\n", FILE__, __LINE__);
	//#endif
	if (varsByMangled.find(name)!=varsByMangled.end()) 
	{
		ret = *(varsByMangled[name]);
		return true;	
  	}
	serr = No_Such_Variable;
  	return false;
}

bool Dyn_Symtab::findFuncVectorByMangledRegex(const string &rexp, bool checkCase, vector<Dyn_Symbol *>&ret)
{
 	unsigned start = ret.size();	
	hash_map <string, vector<Dyn_Symbol *>*>::iterator iter;
	for(iter = funcsByMangled.begin(); iter!=funcsByMangled.end(); iter++)
	{
		if(regexEquiv(rexp,iter->first,checkCase))
		{
			vector<Dyn_Symbol *> funcs = *(iter->second);
			int size = funcs.size();
			for(int index = 0; index < size; index++)
				ret.push_back(funcs[index]);
		}
	}
	if(ret.size() > start)
		return true;
	serr = No_Such_Function;
	return false;
}
 
bool Dyn_Symtab::findFuncVectorByPrettyRegex(const string &rexp, bool checkCase, vector<Dyn_Symbol *>&ret)
{
 	unsigned start = ret.size();
	hash_map <string, vector<Dyn_Symbol *>*>::iterator iter;
	for(iter = funcsByPretty.begin(); iter!=funcsByPretty.end(); iter++)
	{
		if(regexEquiv(rexp,iter->first,checkCase))
		{
			vector<Dyn_Symbol *> funcs = *(iter->second);
			int size = funcs.size();
			for(int index = 0; index < size; index++)
				ret.push_back(funcs[index]);
		}
	}
	if(ret.size() > start)
		return true;
	serr = No_Such_Function;
	return false;
}

bool Dyn_Symtab::findVarVectorByMangledRegex(const string &rexp, bool checkCase, vector<Dyn_Symbol *>&ret)
{
 	unsigned start = ret.size();
	hash_map <string, vector<Dyn_Symbol *>*>::iterator iter;
	for(iter = varsByMangled.begin(); iter!=varsByMangled.end(); iter++)
	{
		if(regexEquiv(rexp,iter->first,checkCase))
		{
			vector<Dyn_Symbol *> vars = *(iter->second);
			int size = vars.size();
			for(int index = 0; index < size; index++)
				ret.push_back(vars[index]);
		}
	}
	if(ret.size() > start)
		return true;
	serr = No_Such_Variable;
	return false;
}

bool Dyn_Symtab::findVarVectorByPrettyRegex(const string &rexp, bool checkCase, vector<Dyn_Symbol *>&ret)
{
	unsigned start = ret.size();
	hash_map <string, vector<Dyn_Symbol *>*>::iterator iter;
	for(iter = varsByPretty.begin(); iter!=varsByPretty.end(); iter++)
	{
		if(regexEquiv(rexp,iter->first,checkCase))
		{
			vector<Dyn_Symbol *> vars = *(iter->second);
			int size = vars.size();
			for(int index = 0; index < size; index++)
				ret.push_back(vars[index]);
		}
	}
	if(ret.size() > start)
		return true;
	serr = No_Such_Variable;
	return false;
}

bool Dyn_Symtab::findModByRegex(const string &rexp, bool checkCase, vector<Dyn_Symbol *>&ret)
{
	unsigned start = ret.size();
	hash_map <string, vector<Dyn_Symbol *>*>::iterator iter;
	for(iter = modsByName.begin(); iter!=modsByName.end(); iter++)
	{
		if(regexEquiv(rexp,iter->first,checkCase))
		{
			vector<Dyn_Symbol *> vars = *(iter->second);
			int size = vars.size();
			for(int index = 0; index < size; index++)
				ret.push_back(vars[index]);
		}
	}
	if(ret.size() > start)
		return true;
	serr = No_Such_Module;
	return false;
}

bool Dyn_Symtab::findSectionByEntry(const OFFSET &offset, Dyn_Section *&ret)
{
 	if(secsByEntryAddr.find(offset) != secsByEntryAddr.end())
	{
		ret = secsByEntryAddr[offset];
		return true;
	}
	serr = No_Such_Section;
	return false;
}

bool Dyn_Symtab::findSection(const string &secName, Dyn_Section *&ret)
{
	for(unsigned index=0;index<sections_.size();index++)
	{
		if(sections_[index]->getSecName() == secName)
		{
			ret = sections_[index];
			return true;
		}
	}
	serr = No_Such_Section;
	return false;
}
 
// Address must be in code or data range since some code may end up
// in the data segment
bool Dyn_Symtab::isValidOffset(const OFFSET &where) const
{
   return isCode(where) || isData(where);
}

bool Dyn_Symtab::isCode(const OFFSET &where)  const
{
 	return (linkedFile->code_ptr() && 
           (where >= codeOffset_) && (where < (codeOffset_+codeLen_)));
}

bool Dyn_Symtab::isData(const OFFSET &where)  const
{
 	return (linkedFile->data_ptr() && 
           (where >= dataOffset_) && (where < (dataOffset_+dataLen_)));
}

bool Dyn_Symtab::getFuncBindingTable(vector<relocationEntry> &fbt) const
{
 	return linkedFile->get_func_binding_table(fbt);	
}
 
Dyn_Symtab::~Dyn_Symtab()
{
   // Doesn't do anything yet, moved here so we don't mess with symtab.h
   // Only called if we fail to create a process.
   // Or delete the a.out...
	
   unsigned i;
	
   for (i = 0; i < sections_.size(); i++) {
      delete sections_[i];
   }
	sections_.clear();

	for (i = 0; i < _mods.size(); i++) {
      delete _mods[i];
   }
	_mods.clear();

	for(i=0; i< modSyms.size(); i++)
		delete modSyms[i];
	modSyms.clear();	

	for(i=0; i< notypeSyms.size(); i++)
		delete notypeSyms[i];
	notypeSyms.clear();	
	
	for (i = 0; i < everyUniqueVariable.size(); i++) {
      delete everyUniqueVariable[i];
   }
   everyUniqueVariable.clear();

	for (i = 0; i < everyUniqueFunction.size(); i++) {
      delete everyUniqueFunction[i];
   }
   everyUniqueFunction.clear();
   createdFunctions.clear();
   exportedFunctions.clear();

	for (i = 0; i < allSymtabs.size(); i++) {
      if (allSymtabs[i] == this)
         allSymtabs.erase(allSymtabs.begin()+i);
   }
}	
 
bool Dyn_Module::findSymbolByType(vector<Dyn_Symbol *> &found, const string &name,
                                  Dyn_Symbol::SymbolType sType, bool isMangled,
                                  bool isRegex, bool checkCase)
{
 	unsigned orig_size = found.size();
	vector<Dyn_Symbol *> obj_syms;
	if(!exec()->findSymbolByType(obj_syms, name, sType, isMangled, isRegex, checkCase))
		return false;
	for (unsigned i = 0; i < obj_syms.size(); i++) 
	{
		if(obj_syms[i]->getModule() == this)
         found.push_back(obj_syms[i]);
   }
   if (found.size() > orig_size) 
      return true;
   return false;	
}
 
DLLEXPORT const string &Dyn_Module::fileName() const {
   return fileName_; 
}

DLLEXPORT const string &Dyn_Module::fullName() const { 
   return fullName_; 
}
 
DLLEXPORT Dyn_Symtab *Dyn_Module::exec() const { 
   return exec_;
}

DLLEXPORT supportedLanguages Dyn_Module::language() const { 
   return language_;
}
 
DLLEXPORT Dyn_Module::Dyn_Module(supportedLanguages lang, OFFSET adr, 
                                 string fullNm, Dyn_Symtab *img)
   : fullName_(fullNm), language_(lang), addr_(adr), exec_(img)
{
   fileName_ = extract_pathname_tail(fullNm);
}

DLLEXPORT Dyn_Module::Dyn_Module()
  : language_(lang_Unknown), addr_(0), exec_(NULL)
{
}

DLLEXPORT Dyn_Module::Dyn_Module(const Dyn_Module &mod)
   : Dyn_LookupInterface(),fullName_(mod.fullName_),
     language_(mod.language_), addr_(mod.addr_), exec_(mod.exec_)
{
}

DLLEXPORT Dyn_Module::~Dyn_Module()
{
}

bool Dyn_Module::isShared() const 
{ 
 	return !exec_->isExec();
}

bool Dyn_Module::getAllSymbolsByType(vector<Dyn_Symbol *> &found, Dyn_Symbol::SymbolType sType)
{
 	unsigned orig_size = found.size();
	vector<Dyn_Symbol *> obj_syms;
	if(!exec()->getAllSymbolsByType(obj_syms, sType))
		return false;
	for (unsigned i = 0; i < obj_syms.size(); i++) 
	{
		if(obj_syms[i]->getModule() == this)
         found.push_back(obj_syms[i]);
   }
   if (found.size() > orig_size) 
      return true;
	serr = No_Such_Symbol;	
   return false;
}

DLLEXPORT bool Dyn_Module::operator==(const Dyn_Module &mod) const {
   return ( (language_==mod.language_) &&
            (addr_==mod.addr_) &&
            (fullName_==mod.fullName_) &&
            (exec_==mod.exec_) 
          );
}

DLLEXPORT bool Dyn_Module::setName(string newName) {
   fullName_ = newName;
   fileName_ = extract_pathname_tail(fullName_);
   return true;
}

DLLEXPORT void Dyn_Module::setLanguage(supportedLanguages lang) 
{
   language_ = lang;
}

DLLEXPORT OFFSET Dyn_Module::addr() const { 
   return addr_; 
}

// Use POSIX regular expression pattern matching to check if string s matches
// the pattern in this string
bool regexEquiv( const string &str,const string &them, bool checkCase ) 
{
	const char *str_ = str.c_str();
	const char *s = them.c_str();
	// Would this work under NT?  I don't know.
#if !defined(os_windows)
   regex_t r;
   bool match = false;
   int cflags = REG_NOSUB;
   if( !checkCase )
      cflags |= REG_ICASE;

   // Regular expressions must be compiled first, see 'man regexec'
   int err = regcomp( &r, str_, cflags );

   if( err == 0 ) {
      // Now we can check for a match
      err = regexec( &r, s, 0, NULL, 0 );
      if( err == 0 )
         match = true;
   }

   // Deal with errors
   if( err != 0 && err != REG_NOMATCH ) {
      char errbuf[80];
      regerror( err, &r, errbuf, 80 );
      cerr << "regexEquiv -- " << errbuf << endl;
   }

   // Free the pattern buffer
   regfree( &r );
   return match;
#else
 	return pattern_match(str_, s, checkCase);
#endif

}

// This function will match string s against pattern p.
// Asterisks match 0 or more wild characters, and a question
// mark matches exactly one wild character.  In other words,
// the asterisk is the equivalent of the regex ".*" and the
// question mark is the equivalent of "."

bool
pattern_match( const char *p, const char *s, bool checkCase ) {
   //const char *p = ptrn;
   //char *s = str;

   while ( true ) {
      // If at the end of the pattern, it matches if also at the end of the string
      if( *p == '\0' )
         return ( *s == '\0' );

      // Process a '*'
      if( *p == MULTIPLE_WILDCARD_CHAR ) {
         ++p;

         // If at the end of the pattern, it matches
         if( *p == '\0' )
            return true;

         // Try to match the remaining pattern for each remaining substring of s
         for(; *s != '\0'; ++s )
            if( pattern_match( p, s, checkCase ) )
               return true;
         // Failed
         return false;
      }

      // If at the end of the string (and at this point, not of the pattern), it fails
      if( *s == '\0' )
         return false;

      // Check if this character matches
      bool matchChar = false;
      if( *p == WILDCARD_CHAR || *p == *s )
         matchChar = true;
      else if( !checkCase ) {
         if( *p >= 'A' && *p <= 'Z' && *s == ( *p + ( 'a' - 'A' ) ) )
            matchChar = true;
         else if( *p >= 'a' && *p <= 'z' && *s == ( *p - ( 'a' - 'A' ) ) )
            matchChar = true;
      }

      if( matchChar ) {
         ++p;
         ++s;
         continue;
      }

      // Did not match
      return false;
   }
}

bool Dyn_Symtab::openFile( string &filename, Dyn_Symtab *&obj){
	bool err;
#if defined(TIMED_PARSE)
   struct timeval starttime;
   gettimeofday(&starttime, NULL);
#endif
	unsigned numSymtabs = allSymtabs.size();
	  
	// AIX: it's possible that we're reparsing a file with better information
	// about it. If so, yank the old one out of the allSymtabs vector -- replace
	// it, basically.
	if(filename.find("/proc") == string::npos)
	{
	   for (unsigned u=0; u<numSymtabs; u++) {
	      if (filename == allSymtabs[u]->file()) {
            // return it
            obj = allSymtabs[u];
            return true;
	      }
	   }   
  	}
	obj = new Dyn_Symtab(filename, err);
#if defined(TIMED_PARSE)
  	struct timeval endtime;
  	gettimeofday(&endtime, NULL);
  	unsigned long lstarttime = starttime.tv_sec * 1000 * 1000 + starttime.tv_usec;
  	unsigned long lendtime = endtime.tv_sec * 1000 * 1000 + endtime.tv_usec;
  	unsigned long difftime = lendtime - lstarttime;
  	double dursecs = difftime/(1000 );
  	cout << __FILE__ << ":" << __LINE__ <<": openFile "<< filename<< " took "<<dursecs <<" msecs" << endl;
#endif
	if(err == true)
	{
		if(filename.find("/proc") == string::npos)
			allSymtabs.push_back(obj);
	}
	else
		obj = NULL;
	return err;
}
	
bool Dyn_Symtab::openFile( char *mem_image, size_t size, Dyn_Symtab *&obj){
	bool err;
	obj = new Dyn_Symtab(mem_image, size, err);
	if(err == false)
		obj = NULL;
	return err;
}

bool Dyn_Symtab::changeType(Dyn_Symbol *sym, Dyn_Symbol::SymbolType oldType)
{
	vector<string>names;
	if(oldType == Dyn_Symbol::ST_FUNCTION)
	{
		unsigned i;
		vector<Dyn_Symbol *> *funcs;
		vector<Dyn_Symbol *>::iterator iter;
		names = sym->getAllMangledNames();
		for(i=0;i<names.size();i++)
		{
			funcs = funcsByMangled[names[i]];
			iter = find(funcs->begin(), funcs->end(), sym);
			funcs->erase(iter);
		}
		names.clear();
		names = sym->getAllPrettyNames();
		for(i=0;i<names.size();i++)
		{
			funcs = funcsByPretty[names[i]];
			iter = find(funcs->begin(), funcs->end(), sym);
			funcs->erase(iter);
		}
		names.clear();
		names = sym->getAllTypedNames();
		for(i=0;i<names.size();i++)
		{
			funcs = funcsByPretty[names[i]];
			iter = find(funcs->begin(), funcs->end(), sym);
			funcs->erase(iter);
		}
		names.clear();
		iter = find(everyUniqueFunction.begin(), everyUniqueFunction.end(), sym);
		everyUniqueFunction.erase(iter);
	}
	else if(oldType == Dyn_Symbol::ST_OBJECT)
	{
		unsigned i;
		vector<Dyn_Symbol *> *vars;
		vector<Dyn_Symbol *>::iterator iter;
		names = sym->getAllMangledNames();
		for(i=0;i<names.size();i++)
		{
			vars = varsByMangled[names[i]];
			iter = find(vars->begin(), vars->end(), sym);
			vars->erase(iter);
		}
		names.clear();
		names = sym->getAllPrettyNames();
		for(i=0;i<names.size();i++)
		{
			vars = varsByPretty[names[i]];
			iter = find(vars->begin(), vars->end(), sym);
			vars->erase(iter);
		}
		names.clear();
		names = sym->getAllTypedNames();
		for(i=0;i<names.size();i++)
		{
			vars= varsByPretty[names[i]];
			iter = find(vars->begin(), vars->end(), sym);
			vars->erase(iter);
		}
		iter = find(everyUniqueVariable.begin(), everyUniqueVariable.end(), sym);
		everyUniqueVariable.erase(iter);
	}
	else if(oldType == Dyn_Symbol::ST_MODULE)
	{
		unsigned i;
		vector<Dyn_Symbol *> *mods;
		vector<Dyn_Symbol *>::iterator iter;
		names = sym->getAllMangledNames();
		for(i=0;i<names.size();i++)
		{
			mods = modsByName[names[i]];
			iter = find(mods->begin(), mods->end(), sym);
			mods->erase(iter);
		}
		names.clear();
		iter = find(modSyms.begin(),modSyms.end(),sym);
		modSyms.erase(iter);
	}
	else if(oldType == Dyn_Symbol::ST_NOTYPE)
	{
		vector<Dyn_Symbol *>::iterator iter;
		iter = find(notypeSyms.begin(),notypeSyms.end(),sym);
		notypeSyms.erase(iter);
	}
	addSymbol(sym);
	return true;
}

bool Dyn_Symtab::delSymbol(Dyn_Symbol *sym)
{
	vector<string>names;
	if(sym->getType() == Dyn_Symbol::ST_FUNCTION)
	{
		unsigned i;
		vector<Dyn_Symbol *> *funcs;
		vector<Dyn_Symbol *>::iterator iter;
		names = sym->getAllMangledNames();
		for(i=0;i<names.size();i++)
		{
			funcs = funcsByMangled[names[i]];
			iter = find(funcs->begin(), funcs->end(), sym);
			funcs->erase(iter);
		}
		names.clear();
		names = sym->getAllPrettyNames();
		for(i=0;i<names.size();i++)
		{
			funcs = funcsByPretty[names[i]];
			iter = find(funcs->begin(), funcs->end(), sym);
			funcs->erase(iter);
		}
		names.clear();
		names = sym->getAllTypedNames();
		for(i=0;i<names.size();i++)
		{
			funcs = funcsByPretty[names[i]];
			iter = find(funcs->begin(), funcs->end(), sym);
			funcs->erase(iter);
		}
		names.clear();
		iter = find(everyUniqueFunction.begin(), everyUniqueFunction.end(), sym);
		everyUniqueFunction.erase(iter);
	}
	else if(sym->getType() == Dyn_Symbol::ST_OBJECT)
	{
		unsigned i;
		vector<Dyn_Symbol *> *vars;
		vector<Dyn_Symbol *>::iterator iter;
		names = sym->getAllMangledNames();
		for(i=0;i<names.size();i++)
		{
			vars = varsByMangled[names[i]];
			iter = find(vars->begin(), vars->end(), sym);
			vars->erase(iter);
		}
		names.clear();
		names = sym->getAllPrettyNames();
		for(i=0;i<names.size();i++)
		{
			vars = varsByPretty[names[i]];
			iter = find(vars->begin(), vars->end(), sym);
			vars->erase(iter);
		}
		names.clear();
		names = sym->getAllTypedNames();
		for(i=0;i<names.size();i++)
		{
			vars= varsByPretty[names[i]];
			iter = find(vars->begin(), vars->end(), sym);
			vars->erase(iter);
		}
		iter = find(everyUniqueVariable.begin(), everyUniqueVariable.end(), sym);
		everyUniqueVariable.erase(iter);
	}
	else if(sym->getType() == Dyn_Symbol::ST_MODULE)
	{
		vector<Dyn_Symbol *>::iterator iter;
		iter = find(modSyms.begin(),modSyms.end(),sym);
		modSyms.erase(iter);
	}
	else if(sym->getType() == Dyn_Symbol::ST_NOTYPE)
	{
		vector<Dyn_Symbol *>::iterator iter;
		iter = find(notypeSyms.begin(),notypeSyms.end(),sym);
		notypeSyms.erase(iter);
	}
	delete(sym);
	return true;
}


/********************************************************************************
// Dyn_Symtab::exportXML
// This functions generates the XML document for all the data in Dyn_Symtab.
********************************************************************************/

bool Dyn_Symtab::exportXML(string file)
{
    int rc;
#if defined(_MSC_VER)
	hXML = LoadLibrary(TEXT("libxml2.dll"));
	if(hXML == NULL){
    	serr = Export_Error;
    	errMsg = "Unable to find libxml2";
    }
	my_xmlNewTextWriterFilename = (xmlTextWriterPtr(*)(const char *,int))GetProcAddress(hXML,"xmlNewTextWriterFilename");
    my_xmlTextWriterStartDocument = (int(*)(xmlTextWriterPtr, const char *, const char *, const char * ))GetProcAddress(hXML,"xmlTextWriterStartDocument");
    my_xmlTextWriterStartElement = (int(*)(xmlTextWriterPtr, const xmlChar *))GetProcAddress(hXML,"xmlTextWriterStartElement");
    my_xmlTextWriterWriteFormatElement = (int(*)(xmlTextWriterPtr,const xmlChar *,const char *,...))GetProcAddress(hXML,"xmlTextWriterWriteFormatElement");
    my_xmlTextWriterEndDocument = (int(*)(xmlTextWriterPtr))GetProcAddress(hXML,"xmlTextWriterEndDocument");
    my_xmlFreeTextWriter = (void(*)(xmlTextWriterPtr))GetProcAddress(hXML,"xmlFreeTextWriter");
    my_xmlTextWriterWriteFormatAttribute = (int(*)(xmlTextWriterPtr, const xmlChar *,const char *,...))GetProcAddress(hXML,"xmlTextWriterWriteFormatAttribute");
    my_xmlTextWriterEndElement = (int(*)(xmlTextWriterPtr))GetProcAddress(hXML,"xmlTextWriterEndElement");
#else
    hXML = dlopen("libxml2.so", RTLD_LAZY);
    if(hXML == NULL){
    	serr = Export_Error;
    	errMsg = "Unable to find libxml2";
    }	
    my_xmlNewTextWriterFilename = (xmlTextWriterPtr(*)(const char *,int))dlsym(hXML,"xmlNewTextWriterFilename");
    my_xmlTextWriterStartDocument = (int(*)(xmlTextWriterPtr, const char *, const char *, const char * ))dlsym(hXML,"xmlTextWriterStartDocument");
    my_xmlTextWriterStartElement = (int(*)(xmlTextWriterPtr, const xmlChar *))dlsym(hXML,"xmlTextWriterStartElement");
    my_xmlTextWriterWriteFormatElement = (int(*)(xmlTextWriterPtr,const xmlChar *,const char *,...))dlsym(hXML,"xmlTextWriterWriteFormatElement");
    my_xmlTextWriterEndDocument = (int(*)(xmlTextWriterPtr))dlsym(hXML,"xmlTextWriterEndDocument");
    my_xmlFreeTextWriter = (void(*)(xmlTextWriterPtr))dlsym(hXML,"xmlFreeTextWriter");
    my_xmlTextWriterWriteFormatAttribute = (int(*)(xmlTextWriterPtr, const xmlChar *,const char *,...))dlsym(hXML,"xmlTextWriterWriteFormatAttribute");
    my_xmlTextWriterEndElement = (int(*)(xmlTextWriterPtr))dlsym(hXML,"xmlTextWriterEndElement");
#endif    	 
	    
    /* Create a new XmlWriter for DOM */
    xmlTextWriterPtr writer = my_xmlNewTextWriterFilename(file.c_str(), 0);
    if (writer == NULL) {
    	serr = Export_Error;
        errMsg = "testXmlwriterDoc: Error creating the xml writer";
	return false;
    }	
    rc = my_xmlTextWriterStartDocument(writer, NULL, "ISO-8859-1", NULL);
    if (rc < 0) {
    	serr = Export_Error;
        errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterStartDocument";
	return false;
    }
    rc = my_xmlTextWriterStartElement(writer, BAD_CAST "Dyn_Symtab");
    if (rc < 0) {
    	serr = Export_Error;
        errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterStartElement";
        return false;
    }
    rc = my_xmlTextWriterWriteFormatElement(writer, BAD_CAST "file",
                                             "%s", filename_.c_str());
    if (rc < 0) {
    	serr = Export_Error;
        errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterStartElement";
        return false;
    }
    
    rc = my_xmlTextWriterWriteFormatElement(writer, BAD_CAST "codeOff",
                                             "0x%lx", codeOffset_);
    if (rc < 0) {
    	serr = Export_Error;
        errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterStartElement";
        return false;
    }
    rc = my_xmlTextWriterWriteFormatElement(writer, BAD_CAST "codeLen",
                                             "%ld", codeLen_);
    if (rc < 0) {
    	serr = Export_Error;
        errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterStartElement";
        return false;
    }
    rc = my_xmlTextWriterWriteFormatElement(writer, BAD_CAST "dataOff",
                                             "0x%lx", dataOffset_);
    if (rc < 0) {
    	serr = Export_Error;
        errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterStartElement";
        return false;
    }
    rc = my_xmlTextWriterWriteFormatElement(writer, BAD_CAST "dataLen",
                                             "%ld", dataLen_);
    if (rc < 0) {
    	serr = Export_Error;
        errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterStartElement";
        return false;
    }
    rc = my_xmlTextWriterWriteFormatElement(writer, BAD_CAST "isExec",
                                                 "%d", is_a_out);
    if (rc < 0) {
    	serr = Export_Error;
        errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterStartElement";
        return false;
    }
    
    generateXMLforSyms(writer, everyUniqueFunction, everyUniqueVariable, modSyms, notypeSyms);
    generateXMLforExcps(writer, excpBlocks);
    vector<relocationEntry> fbt;
    getFuncBindingTable(fbt);
    generateXMLforRelocations(writer, fbt);
    
    rc = my_xmlTextWriterEndDocument(writer);
    if (rc < 0) {
    	serr = Export_Error;
        errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterEndDocument";
        return false;
    }
    my_xmlFreeTextWriter(writer);
    
    return true;
}

bool generateXMLforSyms( xmlTextWriterPtr &writer, vector<Dyn_Symbol *> &everyUniqueFunction, vector<Dyn_Symbol *> &everyUniqueVariable, vector<Dyn_Symbol *> &modSyms, vector<Dyn_Symbol *> &notypeSyms)
{
    unsigned tot = everyUniqueFunction.size()+everyUniqueVariable.size()+modSyms.size()+notypeSyms.size();
    unsigned i;
    if(!tot)
    	return true;
    int rc;
    rc = my_xmlTextWriterStartElement(writer, BAD_CAST "Dyn_Symbols");
    if (rc < 0) {
    	serr = Export_Error;
        errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterStartElement";
        return false;
    }

    rc = my_xmlTextWriterWriteFormatAttribute(writer, BAD_CAST "number",
                                         "%d", tot);
    if (rc < 0) {
    	serr = Export_Error;
        errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterWriteFormatAttribute";
        return false;
    }
    
    for(i=0;i<everyUniqueFunction.size();i++)
    {
        if(!generateXMLforSymbol(writer, everyUniqueFunction[i]))
	    return false;
    }		
    for(i=0;i<everyUniqueVariable.size();i++)
    {
        if(!generateXMLforSymbol(writer, everyUniqueVariable[i]))
	    return false;
    }	    
    for(i=0;i<modSyms.size();i++)
    {
        if(!generateXMLforSymbol(writer, modSyms[i]))
	    return false;
    }	    
    for(i=0;i<notypeSyms.size();i++)
    {
        if(!generateXMLforSymbol(writer, notypeSyms[i]))
	    return false;
    }	   
    
    rc = my_xmlTextWriterEndElement(writer);
    if (rc < 0) {
    	serr = Export_Error;
	errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterEndElement";
        return false;
    }
    return true;
}

bool generateXMLforSymbol(xmlTextWriterPtr &writer, Dyn_Symbol *sym)
{
    int rc,j,tot;
    rc = my_xmlTextWriterStartElement(writer, BAD_CAST "Dyn_Symbol");
    if (rc < 0) {
    	serr = Export_Error;
        errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterStartElement";
        return false;
    }
    rc = my_xmlTextWriterWriteFormatElement(writer, BAD_CAST "type",
                                             "%d", sym->getType());
    if (rc < 0) {
    	serr = Export_Error;
        errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterFormatElement";
        return false;
    }
    rc = my_xmlTextWriterWriteFormatElement(writer, BAD_CAST "linkage",
                                             "%d", sym->getLinkage());
    if (rc < 0) {
    	serr = Export_Error;
        errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterFormatElement";
        return false;
    }
    rc = my_xmlTextWriterWriteFormatElement(writer, BAD_CAST "addr",
                                             "0x%lx", sym->getAddr());
    if (rc < 0) {
    	serr = Export_Error;
        errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterFormatElement";
        return false;
    }
    rc = my_xmlTextWriterWriteFormatElement(writer, BAD_CAST "size",
                                             "%ld", sym->getSize());
    if (rc < 0) {
    	serr = Export_Error;
        errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterFormatElement";
        return false;
    }
    tot = sym->getAllMangledNames().size();
    rc = my_xmlTextWriterStartElement(writer, BAD_CAST "mangledNames");
    if (rc < 0) {
    	serr = Export_Error;
        errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterStartElement";
        return false;
    }
    rc = my_xmlTextWriterWriteFormatAttribute(writer, BAD_CAST "number",
       			                                 "%d", tot);
    if (rc < 0) {
    	serr = Export_Error;
        errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterWriteFormatAttribute";
        return false;
    }
    if(tot)
    {
	vector<string> names = sym->getAllMangledNames();
        for(j=0;j<tot;j++)
	{
    	    rc = my_xmlTextWriterWriteFormatElement(writer, BAD_CAST "name",
            	                                 "%s", names[j].c_str());
	    if (rc < 0) {
    		serr = Export_Error;
        	errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterFormatElement";
        	return false;
    	    }
 	}
    }	
    rc = my_xmlTextWriterEndElement(writer);
    if (rc < 0) {
    	serr = Export_Error;
        errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterEndElement";
        return false;
    }
    tot = sym->getAllPrettyNames().size();
    rc = my_xmlTextWriterStartElement(writer, BAD_CAST "prettyNames");
    if (rc < 0) {
    	serr = Export_Error;
        errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterStartElement";
        return false;
    }
    rc = my_xmlTextWriterWriteFormatAttribute(writer, BAD_CAST "number",
       			                                 "%d", tot);
    if (rc < 0) {
    	serr = Export_Error;
        errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterWriteFormatAttribute";
        return false;
    }
    if(tot)
    {
	vector<string> names = sym->getAllPrettyNames();
        for(j=0;j<tot;j++)
	{
    	    rc = my_xmlTextWriterWriteFormatElement(writer, BAD_CAST "name",
            	                                 "%s", names[j].c_str());
	    if (rc < 0) {
    		serr = Export_Error;
        	errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterFormatElement";
        	return false;
    	    }
 	}
    }	
    rc = my_xmlTextWriterEndElement(writer);
    if (rc < 0) {
    	serr = Export_Error;
        errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterEndElement";
        return false;
    }
    tot = sym->getAllTypedNames().size();
    rc = my_xmlTextWriterStartElement(writer, BAD_CAST "typedNames");
    if (rc < 0) {
    	serr = Export_Error;
        errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterStartElement";
        return false;
    }
    rc = my_xmlTextWriterWriteFormatAttribute(writer, BAD_CAST "number",
       			                                 "%d", tot);
    if (rc < 0) {
    	serr = Export_Error;
        errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterWriteFormatAttribute";
        return false;
    }
    if(tot)
    {
	vector<string> names = sym->getAllTypedNames();
        for(j=0;j<tot;j++)
	{
    	    rc = my_xmlTextWriterWriteFormatElement(writer, BAD_CAST "name",
            	                                 "%s", names[j].c_str());
	    if (rc < 0) {
    		serr = Export_Error;
        	errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterFormatElement";
        	return false;
    	    }
 	}
    }	
    rc = my_xmlTextWriterEndElement(writer);
    if (rc < 0) {
    	serr = Export_Error;
        errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterEndElement";
        return false;
    }
    rc = my_xmlTextWriterWriteFormatElement(writer, BAD_CAST "moduleName",
                                             "%s", sym->getModuleName().c_str());
    if (rc < 0) {
    	serr = Export_Error;
        errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterFormatElement";
        return false;
    }
    rc = my_xmlTextWriterEndElement(writer);
    if (rc < 0) {
    	serr = Export_Error;
        errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterEndElement";
        return false;
    }
    return true;
}

bool generateXMLforExcps(xmlTextWriterPtr &writer, vector<Dyn_ExceptionBlock *> &excpBlocks)
{
    unsigned tot = excpBlocks.size(), i;
    int rc;
    rc = my_xmlTextWriterStartElement(writer, BAD_CAST "Dyn_ExcpBlocks");
    if (rc < 0) {
    	serr = Export_Error;
        errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterStartElement";
        return false;
    }
    rc = my_xmlTextWriterWriteFormatAttribute(writer, BAD_CAST "number",
                                         "%d", tot);
    if (rc < 0) {
    	serr = Export_Error;
        errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterWriteFormatAttribute";
        return false;
    }
    for(i=0;i<excpBlocks.size();i++)
    {
        rc = my_xmlTextWriterStartElement(writer, BAD_CAST "Dyn_ExcpBlock");
        if (rc < 0) {
    	    serr = Export_Error;
            errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterStartElement";
            return false;
    	}
    	rc = my_xmlTextWriterWriteFormatElement(writer, BAD_CAST "hasTry",
                                             "%d", excpBlocks[i]->hasTry());
   	if (rc < 0) {
    	    serr = Export_Error;
            errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterFormatElement";
            return false;
    	}
	if(excpBlocks[i]->hasTry())
	{
    	    rc = my_xmlTextWriterWriteFormatElement(writer, BAD_CAST "tryStart",
            		                                 "0x%lx", excpBlocks[i]->tryStart());
    	    if (rc < 0) {
    		serr = Export_Error;
        	errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterFormatElement";
        	return false;
    	    }
    	    rc = my_xmlTextWriterWriteFormatElement(writer, BAD_CAST "trySize",
            		                                 "%ld", excpBlocks[i]->trySize());
    	    if (rc < 0) {
    		serr = Export_Error;
       		errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterFormatElement";
		return false;
	    }	
	}
    	rc = my_xmlTextWriterWriteFormatElement(writer, BAD_CAST "catchStart",
        	                                 "0x%lx", excpBlocks[i]->catchStart());
    	if (rc < 0) {
    	    serr = Export_Error;
            errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterFormatElement";
            return false;
    	}
    	rc = my_xmlTextWriterEndElement(writer);
	if (rc < 0) {
    	    serr = Export_Error;
            errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterEndElement";
            return false;
	}
    }
    rc = my_xmlTextWriterEndElement(writer);
    if (rc < 0) {
    	serr = Export_Error;
        errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterEndElement";
        return false;
    }
    return true;
}


bool generateXMLforRelocations(xmlTextWriterPtr &writer, vector<relocationEntry> &fbt)
{
    unsigned tot = fbt.size(), i;
    int rc;
    rc = my_xmlTextWriterStartElement(writer, BAD_CAST "Dyn_Relocations");
    if (rc < 0) {
    	serr = Export_Error;
        errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterStartElement";
        return false;
    }
    rc = my_xmlTextWriterWriteFormatAttribute(writer, BAD_CAST "number",
                                         "%d", tot);
    if (rc < 0) {
    	serr = Export_Error;
        errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterWriteFormatAttribute";
        return false;
    }
    for(i=0;i<fbt.size();i++)
    {
        rc = my_xmlTextWriterStartElement(writer, BAD_CAST "Dyn_Relocation");
        if (rc < 0) {
    	    serr = Export_Error;
            errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterStartElement";
            return false;
    	}
    	rc = my_xmlTextWriterWriteFormatElement(writer, BAD_CAST "targetAddr",
        	                                 "0x%lx", fbt[i].target_addr());
    	if (rc < 0) {
    	    serr = Export_Error;
            errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterFormatElement";
            return false;
    	}
    	rc = my_xmlTextWriterWriteFormatElement(writer, BAD_CAST "relAddr",
        	                                 "0x%lx", fbt[i].rel_addr());
    	if (rc < 0) {
    	    serr = Export_Error;
            errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterFormatElement";
            return false;
    	}
    	rc = my_xmlTextWriterWriteFormatElement(writer, BAD_CAST "name",
        	                                     "%s", fbt[i].name().c_str());
    	if (rc < 0) {
    	    serr = Export_Error;
            errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterFormatElement";
            return false;
    	}
    	rc = my_xmlTextWriterEndElement(writer);
	if (rc < 0) {
    	    serr = Export_Error;
            errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterEndElement";
            return false;
	}
    }
    rc = my_xmlTextWriterEndElement(writer);
    if (rc < 0) {
    	serr = Export_Error;
        errMsg = "testXmlwriterDoc: Error at my_xmlTextWriterEndElement";
        return false;
    }
    return true;
}
   

DLLEXPORT ObjectType Dyn_Symtab::getObjectType() const {
   return linkedFile->objType();
}

DLLEXPORT char *Dyn_Symtab::mem_image() const { 
   return linkedFile->mem_image(); 
}

DLLEXPORT string Dyn_Symtab::file() const {
   return filename_;
}

DLLEXPORT string Dyn_Symtab::name() const {
   return name_;
}

DLLEXPORT OFFSET Dyn_Symtab::getLoadAddress() const { 
   return linkedFile->getLoadAddress(); 
}

DLLEXPORT OFFSET Dyn_Symtab::getTOCoffset() const { 
   return linkedFile->getTOCoffset(); 
}
	
DLLEXPORT unsigned Dyn_Symtab::getNumberofSections() const { 
   return no_of_sections; 
}

DLLEXPORT unsigned Dyn_Symtab::getNumberofSymbols() const { 
   return no_of_symbols; 
}

#if defined(os_aix)
void Dyn_Symtab::get_stab_info(char *&stabstr, int &nstabs, void *&stabs, 
                               char *&stringpool) const

{
   linkedFile->get_stab_info(stabstr,nstabs, stabs, stringpool);
}

void Dyn_Symtab::get_line_info(int& nlines, char*& lines,
                               unsigned long& fdptr) const
{
   linkedFile->get_line_info(nlines,lines,fdptr);
}
#else
void Dyn_Symtab::get_stab_info(char *&, int &, void *&, char *&) const 
{
}

void Dyn_Symtab::get_line_info(int &, char *&, unsigned long&) const
{
}
#endif


DLLEXPORT Dyn_LookupInterface::Dyn_LookupInterface() 
{
}

DLLEXPORT Dyn_LookupInterface::~Dyn_LookupInterface()
{
}


DLLEXPORT Dyn_ExceptionBlock::Dyn_ExceptionBlock(OFFSET tStart, 
                                                 unsigned tSize, 
                                                 OFFSET cStart) 
   : tryStart_(tStart), trySize_(tSize), catchStart_(cStart), hasTry_(true) 
{
}

DLLEXPORT Dyn_ExceptionBlock::Dyn_ExceptionBlock(OFFSET cStart) 
   : tryStart_(0), trySize_(0), catchStart_(cStart), hasTry_(false) 
{
}

DLLEXPORT Dyn_ExceptionBlock::Dyn_ExceptionBlock(const Dyn_ExceptionBlock &eb) 
   : tryStart_(eb.tryStart_), trySize_(eb.trySize_), 
     catchStart_(eb.catchStart_), hasTry_(eb.hasTry_) 
{
}
 
DLLEXPORT bool Dyn_ExceptionBlock::hasTry() const
{ 
   return hasTry_; 
}

DLLEXPORT OFFSET Dyn_ExceptionBlock::tryStart() const
{ 
   return tryStart_; 
}

DLLEXPORT OFFSET Dyn_ExceptionBlock::tryEnd() const
{ 
   return tryStart_ + trySize_; 
}

DLLEXPORT OFFSET Dyn_ExceptionBlock::trySize() const
{
   return trySize_; 
}

DLLEXPORT bool Dyn_ExceptionBlock::contains(OFFSET a) const
{ 
   return (a >= tryStart_ && a < tryStart_ + trySize_); 
}

DLLEXPORT Dyn_Section::Dyn_Section()
{
}

DLLEXPORT Dyn_Section::Dyn_Section(unsigned sidnumber, string sname, 
                                   OFFSET saddr, unsigned long ssize, 
                                   void *secPtr, unsigned long sflags) 
   : sidnumber_(sidnumber), sname_(sname), saddr_(saddr), ssize_(ssize), 
     rawDataPtr_(secPtr), sflags_(sflags)
{
}

DLLEXPORT Dyn_Section::Dyn_Section(unsigned sidnumber, string sname, 
                                   unsigned long ssize, void *secPtr, 
                                   unsigned long sflags)
   : sidnumber_(sidnumber), sname_(sname), saddr_(0), ssize_(ssize), 
     rawDataPtr_(secPtr), sflags_(sflags)
{
}


DLLEXPORT Dyn_Section::Dyn_Section(const Dyn_Section &sec)
   : sidnumber_(sec.sidnumber_),sname_(sec.sname_), saddr_(sec.saddr_), 
     ssize_(sec.ssize_), rawDataPtr_(sec.rawDataPtr_), 
		sflags_(sec.sflags_)
{
}

DLLEXPORT Dyn_Section& Dyn_Section::operator=(const Dyn_Section &sec)
{
   sidnumber_ = sec.sidnumber_;
   sname_ = sec.sname_;
   saddr_ = sec.saddr_;
   ssize_ = sec.ssize_;
   rawDataPtr_ = sec.rawDataPtr_;
   sflags_ = sec.sflags_;
   
   return *this;
}
	
DLLEXPORT ostream& Dyn_Section::operator<< (ostream &os) 
{
   return os << "{"
             << " id="      << sidnumber_
             << " name="    << sname_
             << " addr="    << saddr_
             << " size="    << ssize_
             << " }" << endl;
}

DLLEXPORT bool Dyn_Section::operator== (const Dyn_Section &sec)
{
   return ((sidnumber_ == sec.sidnumber_)&&
           (sname_ == sec.sname_)&&
           (saddr_ == sec.saddr_)&&
           (ssize_ == sec.ssize_)&&
           (rawDataPtr_ == sec.rawDataPtr_));
}

DLLEXPORT Dyn_Section::~Dyn_Section() 
{
}
 
DLLEXPORT unsigned Dyn_Section::getSecNumber() const
{ 
   return sidnumber_; 
}

DLLEXPORT string Dyn_Section::getSecName() const
{ 
   return sname_; 
}

DLLEXPORT OFFSET Dyn_Section::getSecAddr() const
{ 
   return saddr_; 
}

DLLEXPORT void *Dyn_Section::getPtrToRawData() const
{ 
   return rawDataPtr_; 
}

DLLEXPORT unsigned long Dyn_Section::getSecSize() const
{ 
   return ssize_; 
}

DLLEXPORT bool Dyn_Section::isBSS() const
{ 
   return sname_==".bss";
}

DLLEXPORT bool Dyn_Section::isText() const
{ 
   return sname_ == ".text"; 
}

DLLEXPORT bool Dyn_Section::isData() const
{ 
   return (sname_ == ".data"||sname_ == ".data2"); 
}

DLLEXPORT bool Dyn_Section::isOffsetInSection(const OFFSET &offset) const
{
   return (offset > saddr_ && offset < saddr_ + ssize_);
}


DLLEXPORT relocationEntry::relocationEntry()
   :target_addr_(0),rel_addr_(0)
{
}   

DLLEXPORT relocationEntry::relocationEntry(OFFSET ta,OFFSET ra, string n)
   : target_addr_(ta), rel_addr_(ra),name_(n)
{
}   

DLLEXPORT const relocationEntry& relocationEntry::operator=(const relocationEntry &ra) 
{
   target_addr_ = ra.target_addr_; rel_addr_ = ra.rel_addr_; 
   name_ = ra.name_; 
   return *this;
}
