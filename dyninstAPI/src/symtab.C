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
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <string>
#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/arch.h"
#include "dyninstAPI/src/instPoint.h"

#include "dyninstAPI/src/parRegion.h"

#include <fstream>
#include "dyninstAPI/src/util.h"
#include "dyninstAPI/src/inst.h"
#include "common/h/Timer.h"
#include "dyninstAPI/src/debug.h"
#include "common/h/debugOstream.h"
#include "common/h/pathName.h"          // extract_pathname_tail()
#include "dyninstAPI/src/function.h"

#include "dyninstAPI/h/BPatch_flowGraph.h"
#include "dynutil/h/util.h"

#if defined(TIMED_PARSE)
#include <sys/time.h>
#endif

#if defined( USES_DWARF_DEBUG )
#include "dwarf.h"
#include "libdwarf.h"
#endif

#if defined(i386_unknown_nt4_0)
#include <dbghelp.h>
#include <cvconst.h>
#endif

AnnotationClass<image_variable> ImageVariableUpPtrAnno("ImageVariableUpPtrAnno");
AnnotationClass<image_func> ImageFuncUpPtrAnno("ImageFuncUpPtrAnno");
pdvector<image*> allImages;

using namespace Dyninst;
using namespace std;

string fileDescriptor::emptyString(string(""));
fileDescriptor::fileDescriptor() {
    // This shouldn't be called... must be public for pdvector, though
}

bool fileDescriptor::IsEqual(const fileDescriptor &fd) const {
    // Don't test isShared, only file name and addresses
    bool file_match_ = false;

    // Annoying... we often get "foo vs ./foo" or such. So consider it a match
    // if either file name is prefixed by the other; we don't get trailing crud.
    string::size_type len1 = file_.length();
    string::size_type len2 = fd.file_.length();
  
    if(((len1>=len2) && (file_.substr(len1-len2,len2) == fd.file_))
       || ((len2>len1) && (fd.file_.substr(len2-len1,len1) == file_)))
        file_match_ = true;   
#if defined(os_linux)
    struct stat buf1;
    struct stat buf2;
    if (!stat(file_.c_str(),&buf1)
        && !stat(fd.file_.c_str(),&buf2)
        && buf1.st_ino == buf2.st_ino) {
        file_match_ = true;
    }
#endif  
    bool addr_match = ((code_ == fd.code_ && data_ == fd.data_) ||
                       (dynamic_ && dynamic_ == fd.dynamic_));
    if (file_match_ &&
        (addr_match) &&
        (member_ == fd.member_) &&
        (pid_ == fd.pid_))
        return true;
    else
        return false;
}

void fileDescriptor::setLoadAddr(Address a) 
{ 
   loadAddr_ = a; 
   code_ += a;
   data_ += a;
}

// All debug_ostream vrbles are defined in process.C (for no particular reason)
extern unsigned enable_pd_sharedobj_debug;

int codeBytesSeen = 0;



// makeImageFunction(): define an image_func for the provided Function.
// 
// This used to attempt to derive module information; we now do that
// in Symtab.

image_func *image::makeImageFunction(Function *lookUp) 
{
    pdmodule *pdmod = getOrCreateModule(lookUp->getModule());

   image_func *func = new image_func(lookUp, 
                                     pdmod, 
                                     this,
                                     FS_SYMTAB);
   
   /* Among symbol table functions, the ones with @ in the name are most likely OpenMP functions,
      if any non-OpenMP functions sneak in here we'll take care of them later when 
      closer analysis is done */

#if defined(os_solaris)
   if(strstr(lookUp->getAllMangledNames()[0].c_str(), "_$") != NULL){
       image_parRegion * pR = new image_parRegion(lookUp->getOffset(),func);    
       parallelRegions.push_back(pR);
   }
#endif 


#if defined(os_aix)

   if(strstr(lookUp->getAllMangledNames()[0].c_str(), "@OL@") != NULL){
       image_parRegion * pR = new image_parRegion(lookUp->getOffset(),func);    
       parallelRegions.push_back(pR);
   } 
#endif
   
   assert(func);
 
   return func;
}


/* 
 * Search for the Main Symbols in the list of symbols, Only in case
 * if the file is a shared object. If not present add them to the
 * list
 */

void image::findMain()
{
#if defined(i386_unknown_linux2_0) \
|| defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ \
|| defined(i386_unknown_solaris2_5) \
   
    if(!desc_.isSharedObject())
    {
    	bool foundMain = false;
    	bool foundStart = false;
    	bool foundFini = false;
    	//check if 'main' is in allsymbols
        vector <Function *> funcs;
        if (linkedFile->findFunctionsByName(funcs, "main") ||
            linkedFile->findFunctionsByName(funcs, "_main"))
            foundMain = true;
        else if (linkedFile->findFunctionsByName(funcs, "_start"))
            foundStart = true;
        else if (linkedFile->findFunctionsByName(funcs, "_fini"))
            foundFini = true;
    
    	Region *textsec = NULL;
    	bool foundText = linkedFile->findRegion(textsec, ".text");
        if (foundText == false) {
            return;
        }
	
    	if( !foundMain )
    	{
            logLine( "No main symbol found: creating symbol for main\n" );

    	    //find and add main to allsymbols
            const unsigned char* p;
		                   
            p = (( const unsigned char * ) textsec->getPtrToRawData());
            const unsigned char *lastP = 0;

            switch(linkedFile->getAddressWidth()) {
       	    	case 4:
            	    // 32-bit...
                    startup_printf("%s[%u]:  setting 32-bit mode\n",
                        FILE__,__LINE__);
            	    ia32_set_mode_64(false);
                    break;
         	case 8:
                    startup_printf("%s[%u]:  setting 64-bit mode\n",
                        FILE__,__LINE__);
            	    ia32_set_mode_64(true);
            	    break;
        	default:
            	    assert(0 && "Illegal address width");
            	    break;
            }

            instruction insn;
            insn.setInstruction( p );
 
            while( !insn.isCall() )
            {
            	lastP = p;
            	p += insn.size();
            	insn.setInstruction( p );
            }

            // We _really_ can't handle a call with nothing before it....
            assert(lastP);

            // FIXME: this assumes that the instruction immediately before the call sets
            // the main address - this may not be true.
            instruction preCall;
            preCall.setInstruction(lastP);

            Address mainAddress;
            mainAddress = get_immediate_operand(&preCall);

            if(!mainAddress || !isValidAddress(mainAddress)) {
                startup_printf("%s[%u]:  invalid main address 0x%lx\n",
                    mainAddress);   
            } else {
                startup_printf("%s[%u]:  set main address to 0x%lx\n",
                    FILE__,__LINE__,mainAddress);
            }

            /* Note: creating a symbol for main at the invalid address 
               anyway, because there is guard code for this later in the
               process and otherwise we end up in a weird "this is not an
               a.out" path.

               findMain, like all important utility functions, should have
               a way of gracefully indicating that it has failed. It should
               not return void. NR
            */

    	    Region *pltsec;
            if((linkedFile->findRegion(pltsec, ".plt")) && pltsec->isOffsetInRegion(mainAddress))
            {
            	//logLine( "No static symbol for function main\n" );
                Symbol *newSym = new Symbol("DYNINST_pltMain", 
                                            Symbol::ST_FUNCTION, 
                                            Symbol::SL_GLOBAL,
                                            Symbol::SV_DEFAULT,
                                            mainAddress,
                                            linkedFile->getDefaultModule(),
                                            textsec, 
                                            UINT_MAX );
                linkedFile->addSymbol( newSym );
           }
           else
           {
           	Symbol *newSym= new Symbol( "main", 
                                            Symbol::ST_FUNCTION,
                                            Symbol::SL_GLOBAL, 
                                            Symbol::SV_DEFAULT, 
                                            mainAddress,
                                            linkedFile->getDefaultModule(),
                                            textsec, 
                                            UINT_MAX );
	        linkedFile->addSymbol(newSym);		
            }
        }
    	if( !foundStart )
    	{
            Symbol *startSym = new Symbol( "_start",
                                           Symbol::ST_FUNCTION,
                                           Symbol::SL_GLOBAL,
                                           Symbol::SV_DEFAULT, 
                                           textsec->getRegionAddr(),
                                           linkedFile->getDefaultModule(),
                                           textsec,
                                           UINT_MAX );
            //cout << "sim for start!" << endl;
        
	    linkedFile->addSymbol(startSym);		
    	}
    	if( !foundFini )
    	{
	    Region *finisec;
	    linkedFile->findRegion(finisec,".fini");
            Symbol *finiSym = new Symbol( "_fini",
                                          Symbol::ST_FUNCTION,
                                          Symbol::SL_GLOBAL, 
                                          Symbol::SV_DEFAULT, 
                                          finisec->getRegionAddr(),
                                          linkedFile->getDefaultModule(),
                                          finisec, 
                                          UINT_MAX );
	    linkedFile->addSymbol(finiSym);		
    	}
    }

    Region *dynamicsec;
    vector < Symbol *>syms;
    if(linkedFile->findRegion(dynamicsec, ".dynamic")==true)
    {
        if(linkedFile->findSymbol(syms,
                                  "_DYNAMIC",
                                  Symbol::ST_UNKNOWN,
                                  mangledName)==false)
        {
	    Symbol *newSym = new Symbol( "_DYNAMIC", 
					Symbol::ST_OBJECT, 
                                         Symbol::SL_GLOBAL, 
                                         Symbol::SV_DEFAULT,
                                         dynamicsec->getRegionAddr(), 
                                         linkedFile->getDefaultModule(),
                                         dynamicsec, 
                                         0 );
	    linkedFile->addSymbol(newSym);
	}
    }
    
#elif defined(rs6000_ibm_aix4_1) || defined(rs6000_ibm_aix5_1)
   
   bool foundMain = false;
   vector <Function *> funcs;
   if (linkedFile->findFunctionsByName(funcs, "main") ||
       linkedFile->findFunctionsByName(funcs, "usla_main"))
       foundMain = true;

   Region *sec;
   linkedFile->findRegion(sec, ".text"); 	

   if( !foundMain && linkedFile->isExec() && sec )
   {
       //we havent found a symbol for main therefore we have to parse _start
       //to find the address of main

       //last two calls in _start are to main and exit
       //find the end of _start then back up to find the target addresses
       //for exit and main
      
       int c;
       instructUnion i;
       int calls = 0;
       Word *code_ptr_ = (Word *) sec->getPtrToRawData();
       
       for( c = 0; code_ptr_[ c ] != 0; c++ );

       while( c > 0 )
       {
           i.raw = code_ptr_[ c ];

           if(i.iform.lk && 
              ((i.iform.op == Bop) || (i.bform.op == BCop) ||
               ((i.xlform.op == BCLRop) && 
                ((i.xlform.xo == 16) || (i.xlform.xo == 528)))))
           {
               calls++;
               if( calls == 2 )
                   break;
           }
           c--;
       }
       
       Offset currAddr = sec->getRegionAddr() + c * instruction::size();
       Offset mainAddr = 0;
       
       if( ( i.iform.op == Bop ) || ( i.bform.op == BCop ) )
       {
           int  disp = 0;
           if(i.iform.op == Bop)
           {
               disp = i.iform.li;
           }
           else if(i.bform.op == BCop)
           {
               disp = i.bform.bd;
           }

           disp <<= 2;

           if(i.iform.aa)
           {
               mainAddr = (Offset)disp;
           }
           else
               mainAddr = (Offset)( currAddr + disp );      
       }  
       
       Symbol *sym = new Symbol( "main", 
                                 Symbol::ST_FUNCTION,
                                 Symbol::SL_GLOBAL,
                                 Symbol::SV_DEFAULT, 
                                 mainAddr,
                                 linkedFile->getDefaultModule(),
                                 sec);
       linkedFile->addSymbol(sym);
      
   
       //since we are here make up a sym for _start as well

       Symbol *sym1 = new Symbol( "__start", 
                                  Symbol::ST_FUNCTION,
                                  Symbol::SL_GLOBAL, 
                                  Symbol::SV_DEFAULT, 
                                  sec->getRegionAddr(), 
                                  linkedFile->getDefaultModule(),
                                  sec);
       linkedFile->addSymbol(sym1);
   }

#elif defined(i386_unknown_nt4_0)

#define NUMBER_OF_MAIN_POSSIBILITIES 7
   char main_function_names[NUMBER_OF_MAIN_POSSIBILITIES][20] = {
       "main",
       "DYNINST_pltMain",
       "_main",
       "WinMain",
       "_WinMain",
       "wWinMain",
       "_wWinMain"};
   
   if(linkedFile->isExec()) {
       vector <Symbol *>syms;
       vector<Function *> funcs;
       Address eAddr = linkedFile->getEntryOffset();
       
       bool found_main = false;
       for (unsigned i=0; i<NUMBER_OF_MAIN_POSSIBILITIES; i++) {
           if(linkedFile->findFunctionsByName(funcs, main_function_names[i])) {
               found_main = true;
               break;
           }
       }
       if (!found_main) {
           syms.clear();
           if(!linkedFile->findSymbol(syms,"start",Symbol::ST_UNKNOWN, mangledName)) {
               //use 'start' for mainCRTStartup.
               Symbol *startSym = new Symbol( "start", 
                                              Symbol::ST_FUNCTION,
                                              Symbol::SL_GLOBAL, 
                                              Symbol::SV_DEFAULT, 
                                              eAddr ,
                                              linkedFile->getDefaultModule(),
                                              NULL,
                                              UINT_MAX );
               linkedFile->addSymbol(startSym);
           }
           syms.clear();
           if(!linkedFile->findSymbol(syms,"winStart",Symbol::ST_UNKNOWN, mangledName)) {
               //make up a func name for the start of the text section
               Symbol *sSym = new Symbol( "winStart", 
                                          Symbol::ST_FUNCTION,
                                          Symbol::SL_GLOBAL,
                                          Symbol::SV_DEFAULT, 
                                          imageOffset_,
                                          linkedFile->getDefaultModule(),
                                          NULL, 
                                          UINT_MAX );
               linkedFile->addSymbol(sSym);
           }
           syms.clear();
           if(!linkedFile->findSymbol(syms,"winFini",Symbol::ST_UNKNOWN, mangledName)) {
               //make up one for the end of the text section
               Symbol *fSym = new Symbol( "winFini", 
                                          Symbol::ST_FUNCTION,
                                          Symbol::SL_GLOBAL, 
                                          Symbol::SV_DEFAULT, 
                                          imageOffset_ + linkedFile->imageLength() - 1, 
                                          linkedFile->getDefaultModule(),
                                          NULL, 
                                          UINT_MAX );
               linkedFile->addSymbol(fSym);
           }
           // add entry point as main given that nothing else was found
           startup_printf("[%s:%u] - findmain could not find symbol "
                          "for main, using binary entry point %x\n",
                          __FILE__, __LINE__, eAddr);
           linkedFile->addSymbol(new Symbol("main",
                                            Symbol::ST_FUNCTION, 
                                            Symbol::SL_GLOBAL, 
                                            Symbol::SV_DEFAULT,
                                            eAddr,
                                            linkedFile->getDefaultModule()));
       }
   }
#endif    
}

/*
 * Add all the functions (*) in the list of symbols to our data
 * structures. 
 *
 * We do a search for a "main" symbol (a couple of variants), and
 * if found we flag this image as the executable (a.out). 
 */

bool image::symbolsToFunctions(pdvector<image_func *> &raw_funcs)
{
#if defined(TIMED_PARSE)
  struct timeval starttime;
  gettimeofday(&starttime, NULL);
#endif

  //Checking "main" function names in same order as in the inst-*.C files
  vector <Function *>funcs;
  if (linkedFile->findFunctionsByName(funcs,"main")       ||
      linkedFile->findFunctionsByName(funcs,"_main")     
#if defined(os_windows)
      || linkedFile->findFunctionsByName(funcs,"WinMain")   ||
      linkedFile->findFunctionsByName(funcs,"_WinMain")  ||
      linkedFile->findFunctionsByName(funcs,"wWinMain")  ||
      linkedFile->findFunctionsByName(funcs,"_wWinMain") 
#endif
      ) {
      is_a_out = true;
  }
  else
      is_a_out = false;
  
  // Checking for libdyninstRT (DYNINSTinit())
  if (linkedFile->findFunctionsByName(funcs, "DYNINSTinit") ||
      linkedFile->findFunctionsByName(funcs, "_DYNINSTinit"))
      is_libdyninstRT = true;
  else
      is_libdyninstRT = false;
  
  // find the real functions -- those with the correct type in the symbol table
  vector<Function *> allFuncs;
  if(!linkedFile->getAllFunctions(allFuncs))
      return true;
  vector<Function *>::iterator funcIter = allFuncs.begin();
  for(; funcIter!=allFuncs.end();funcIter++) {
      Function *lookUp = *funcIter;
       
       if (lookUp->getModule()->fullName() == "DYNINSTheap") {
           // Do nothing for now; we really don't want to report it as
           // a real symbol.
       }
       else {
           image_func *new_func = makeImageFunction(lookUp);
           if (!new_func)
               fprintf(stderr, "%s[%d]:  makeImageFunction failed\n", FILE__, __LINE__);
           else {
               raw_funcs.push_back(new_func);
               if (!lookUp->addAnnotation(new_func, ImageFuncUpPtrAnno))
                   {
                       fprintf(stderr, "%s[%d]:  failed to add annotation here\n", FILE__, __LINE__);
                       continue;
                   }
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

bool image::addDiscoveredVariables() {
    // Hrm... we could identify globals via address analysis...
    return true;
}

bool image::getInferiorHeaps(vector<pair<string,Address> > &codeHeaps,
                             vector<pair<string,Address> > &dataHeaps) {
    if ((codeHeaps_.size() == 0) &&
        (dataHeaps_.size() == 0)) return false;

    for (unsigned i = 0; i < codeHeaps_.size(); i++) {
        codeHeaps.push_back(codeHeaps_[i]);
    }

    for (unsigned i = 0; i < dataHeaps_.size(); i++) {
        dataHeaps.push_back(dataHeaps_[i]);
    }
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

   std::string mangledName; 

   vector<Variable *> allVars;

   linkedFile->getAllVariables(allVars); 

   for (vector<Variable *>::iterator varIter = allVars.begin();
        varIter != allVars.end(); 
        varIter++) {
       Variable *symVar = *varIter;

       parsing_printf("New variable, mangled %s, module %s...\n",
                      symVar->getAllMangledNames()[0].c_str(),
                      symVar->getFirstSymbol()->getModuleName().c_str());
       pdmodule *use = getOrCreateModule(symVar->getModule());

       assert(use);
       image_variable *var = new image_variable(symVar, use);
       if (!var->svar()->addAnnotation(var, ImageVariableUpPtrAnno)) {
           fprintf(stderr, "%s[%d]: failed to add annotation here\n", FILE__, __LINE__);
           return false;
       }

       // If this is a Dyninst dynamic heap placeholder, add it to the
       // list of inferior heaps...
       string compString = "DYNINSTstaticHeap";
       if (!var->symTabName().compare(0, compString.size(), compString)) {
           dataHeaps_.push_back(pair<string,Address>(var->symTabName(), var->getOffset()));
       }

       exportedVariables.push_back(var);
       everyUniqueVariable.push_back(var);
       varsByAddr[var->getOffset()] = var;
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

pdmodule *image::findModule(const string &name, bool wildcard)
{
   pdmodule *found = NULL;
   //cerr << "image::findModule " << name << " , " << find_if_excluded
   //     << " called" << endl;

   if (!wildcard) {
      if (modsByFileName.find(name) != modsByFileName.end()) {
         //cerr << " (image::findModule) found module in modsByFileName" << endl;
         found = modsByFileName[name];
      }
      else if (modsByFullName.find(name) != modsByFullName.end()) {
         //cerr << " (image::findModule) found module in modsByFullName" << endl;
         found = modsByFullName[name];
      }
   }
   else {
      //  if we want a substring, have to iterate over all module names
      //  this is ok b/c there are not usually more than a handful or so
      //
      dyn_hash_map <string, pdmodule *>::iterator mi;
      string str; pdmodule *mod;
      std::string pds = name.c_str();

      for(mi = modsByFileName.begin(); mi != modsByFileName.end() ; mi++)
      {
         str = mi->first;
         mod = mi->second;
         if (wildcardEquiv(pds, mod->fileName()) ||
               wildcardEquiv(pds, mod->fullName())) {
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

bool image::getModules(vector<pdmodule *> &mods) 
{
    bool ret = false;
   pdvector<pdmodule *> toReturn;
    for (map<Module *, pdmodule *>::const_iterator iter = mods_.begin();
         iter != mods_.end(); iter++) {
        ret = true;
        mods.push_back(iter->second);
    }
    return ret;
}

void print_module_vector_by_short_name(std::string prefix ,
				       pdvector<pdmodule*> *mods) 
{
    unsigned int i;
    pdmodule *mod;
    for(i=0;i<mods->size();i++) {
        mod = ((*mods)[i]);
	cerr << prefix << mod->fileName() << endl;
    }
}

// identify module name from symbol address (binary search)
// based on module tags found in file format (ELF/COFF)
void image::findModByAddr (const Symbol *lookUp, vector<Symbol *> &mods,
			   string &modName, Address &modAddr, 
			   const string &defName)
{
  if (mods.size() == 0) {
    modAddr = 0;
    modName = defName;
    return;
  }

  Address symAddr = lookUp->getAddr();
  int index;
  int start = 0;
  int end = mods.size() - 1;
  int last = end;
  bool found = false;
  while ((start <= end) && !found) {
    index = (start+end)/2;
    if ((index == last) ||
	((mods[index]->getAddr() <= symAddr) && 
	 (mods[index+1]->getAddr() > symAddr))) {
      modName = mods[index]->getName().c_str();
      modAddr = mods[index]->getAddr();      
      found = true;
    } else if (symAddr < mods[index]->getAddr()) {
      end = index - 1;
    } else {
      start = index + 1;
    }
  }
  if (!found) { 
    // must be (start > end)
    modAddr = 0;
    modName = defName;
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

image *image::parseImage(fileDescriptor &desc, bool parseGaps)
{
  /*
   * Check to see if we have parsed this image before. We will
   * consider it a match if the filename matches (Our code is now able
   * to cache the parsing results even if a library is loaded at a
   * different address for the second time).
   */
  unsigned numImages = allImages.size();
  
  // AIX: it's possible that we're reparsing a file with better information
  // about it. If so, yank the old one out of the images vector -- replace
  // it, basically.
  for (unsigned u=0; u<numImages; u++) {
      if (desc.isSameFile(allImages[u]->desc())) {
          // We reference count...
          startup_printf("%s[%d]: returning pre-parsed image\n", FILE__, __LINE__);
          return allImages[u]->clone();
      }
  }

  stats_parse.startTimer(PARSE_SYMTAB_TIMER);

  /*
   * load the symbol table. (This is the a.out format specific routine).
   */
  
  bool err=false;

#if defined(TIMED_PARSE)
  struct timeval starttime;
  gettimeofday(&starttime, NULL);
#endif

  startup_printf("%s[%d]:  about to create image\n", FILE__, __LINE__);
  image *ret = new image(desc, err, parseGaps); 
  startup_printf("%s[%d]:  created image\n", FILE__, __LINE__);

  if(desc.isSharedObject()) 
      startup_printf("%s[%d]: processing shared object\n", FILE__, __LINE__);
  else  
      startup_printf("%s[%d]: processing executable object\n", FILE__, __LINE__);
      

#if defined(TIMED_PARSE)
  struct timeval endtime;
  gettimeofday(&endtime, NULL);
  unsigned long lstarttime = starttime.tv_sec * 1000 * 1000 + starttime.tv_usec;
  unsigned long lendtime = endtime.tv_sec * 1000 * 1000 + endtime.tv_usec;
  unsigned long difftime = lendtime - lstarttime;
  double dursecs = difftime/(1000 );
  cout << __FILE__ << ":" << __LINE__ <<": parsing image "<< desc.file().c_str() <<" took "<<dursecs <<" msecs" << endl;
#endif

  if (err || !ret) {
     if (ret) {
         startup_printf("%s[%d]: error in processing, deleting image and returning\n",
                        FILE__, __LINE__);
         delete ret;
     }
     else {
        fprintf(stderr, "Failed to allocate memory for parsing %s!\n", 
                desc.file().c_str());
     }
     stats_parse.stopTimer(PARSE_SYMTAB_TIMER);
     return NULL;
  }

  allImages.push_back(ret);

  // define all modules.

  statusLine("ready"); // this shouldn't be here, right? (cuz we're not done, right?)
  stats_parse.stopTimer(PARSE_SYMTAB_TIMER);

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

void image::removeImage(const string file)
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

int image::destroy() {
    refCount--;
    if (refCount == 0) {
        if (!desc().isSharedObject()) {
            // a.out... destroy it
            //delete this;
            return 0;
        }
    }
    if (refCount < 0)
        assert(0 && "NEGATIVE REFERENCE COUNT FOR IMAGE!");
    return refCount; 
}

// Enter a function in all the appropriate tables
void image::enterFunctionInTables(image_func *func) {
    if (!func) return;

    parsing_printf("[%s:%u] entering function at 0x%lx (%s) in tables\n",
        FILE__,__LINE__,func->getOffset(),func->symTabName().c_str());

    funcsByEntryAddr[func->getOffset()] = func;
   
    // XXX the origin & meaning of the following comment is unknown; preserving.
    // TODO: out-of-line insertion here
    if(func->get_size()) {
        funcsByRange.insert(func);
    }

    // This has already been done for symtab functions
    if(func->howDiscovered() != FS_SYMTAB) {
        Symbol *sym = func->getSymtabFunction()->getFirstSymbol();
        getObject()->addSymbol(sym);
    }
   
    // List of all image_func objects 
    everyUniqueFunction.push_back(func);

    if(func->howDiscovered() == FS_SYMTAB)
        exportedFunctions.push_back(func);
    else
        createdFunctions.push_back(func);
}  

//buildFunctionLists() iterates through image_funcs and constructs demangled 
//names. Demangling was moved here (names used to be demangled as image_funcs 
//were built) so that language information could be obtained _after_ the 
//functions and modules were built, but before name demangling takes place.  
//Thus we can use language information during the demangling process.
//
//  * Adds functions to funcsByEntryAddr hash
//  * Consolidates duplicate functions created from symbols at the same addr.
//  * Adds functions to symtabCandidateFuncs list for future parsing

bool image::buildFunctionLists(pdvector <image_func *> &raw_funcs) 
{
    for (unsigned int i = 0; i < raw_funcs.size(); i++) {
        image_func *raw = raw_funcs[i];
        image_func *existingFunction = NULL;

        funcsByEntryAddr.find(raw->getOffset(), existingFunction);
        if(!existingFunction)
        {
            funcsByEntryAddr[raw->getOffset()] = raw;
            std::string name = raw->symTabNameVector().back();
            // Apparently there's some logic here that needs to be invoked,
            // despite this name already being know for this function
            raw->addSymTabName(name);
    
            // add to parsing list
            symtabCandidateFuncs.push_back(raw);
        } else {
            // Add this [possible] alias to the existing function
            existingFunction->addSymTabName(raw->symTabName());

            // clean up this unused function
            raw_funcs[i] = NULL;
            delete raw;
        }
    }

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

// Constructor for the image object. The fileDescriptor simply
// wraps (in the normal case) the object name and a relocation
// address (0 for a.out file). On the following platforms, we
// are handling a special case:
//   AIX: objects can possibly have a name like /lib/libc.so:shr.o
//          since libraries are archives
//        Both text and data sections have a relocation address


image::image(fileDescriptor &desc, bool &err, bool parseGaps) :
   desc_(desc),
   activelyParsing(addrHash4),
   is_libdyninstRT(false),
   is_a_out(false),
   main_call_addr_(0),
   nativeCompiler(false),    
   funcsByEntryAddr(addrHash4),
   nextBlockID_(0),
   pltFuncs(NULL),
   varsByAddr(addrHash4),
   refCount(1),
   parseState_(unparsed),
   parseGaps_(parseGaps)
{

#if defined(os_aix)
   archive = NULL;
   string file = desc_.file().c_str();
   SymtabError serr = Not_An_Archive;

   startup_printf("%s[%d]:  opening file %s (or archive)\n", FILE__, __LINE__, file.c_str());
   if (!Archive::openArchive(archive, file))
   {
      err = true;
      if (archive->getLastError() != serr) {
         startup_printf("%s[%d]:  opened archive\n", FILE__, __LINE__);
         return;
      }
      else
      {
         startup_printf("%s[%d]:  opening file (not archive)\n", FILE__, __LINE__);
         if (!Symtab::openFile(linkedFile, file)) 
         {
            startup_printf("%s[%d]:  opening file (not archive) failed\n", FILE__, __LINE__);
            err = true;
            return;
         }
         startup_printf("%s[%d]:  opened file\n", FILE__, __LINE__);
      }
   }
   else
   {
      assert (archive);
      startup_printf("%s[%d]:  getting member\n", FILE__, __LINE__);
      string member = std::string(desc_.member());
      if (member == fileDescriptor::emptyString) {
         fprintf(stderr, "%s[%d]:  WARNING:  not asking for unnamed member\n", FILE__, __LINE__);
      }
      else {
         if (!archive->getMember(linkedFile, member))
         {
            startup_printf("%s[%d]:  getting member failed\n", FILE__, __LINE__);
            err = true;
            return;
         }
         startup_printf("%s[%d]:  got member\n", FILE__, __LINE__);
      }
   }
   startup_printf("%s[%d]:  opened file %s (or archive)\n", FILE__, __LINE__, file.c_str());
#else
   string file = desc_.file().c_str();
   startup_printf("%s[%d]:  opening file %s\n", FILE__, __LINE__, file.c_str());
   //linkedFile = new Symtab();
   if(!Symtab::openFile(linkedFile, file)) 
   {
      err = true;
      return;
   }
#endif  

   // fix isSharedObject flag in file descriptor
   desc.setIsShared(!linkedFile->isExec());
   desc_.setIsShared(!linkedFile->isExec());

   err = false;
   name_ = extract_pathname_tail(string(desc.file().c_str()));

   //   fprintf(stderr,"img name %s\n",name_.c_str());
   pathname_ = desc.file().c_str();

   // initialize (data members) codeOffset_, dataOffset_,
   //  codeLen_, dataLen_.

   imageOffset_ = linkedFile->imageOffset();
   dataOffset_ = linkedFile->dataOffset();

   imageLen_ = linkedFile->imageLength();
   dataLen_ = linkedFile->dataLength();

   // if unable to parse object file (somehow??), try to
   //  notify user/calling process + return....    
   if (!imageLen_) {
      string msg = string("Parsing problem with executable file: ") + desc.file();
      statusLine(msg.c_str());
      msg += "\n";
      logLine(msg.c_str());
      err = true;
      return; 
   }

   // on some platforms (e.g. Windows) we try to parse
   // the image too soon, before we have a process we can
   // work with reliably.  If so, we must recognize it
   // and reparse at some later time.
   // check if this is been used anywhere??  
   /*if( linkedFile->have_deferred_parsing() )
     {
   // nothing else to do here
   return;
   }*/

   string msg;
   // give user some feedback....
   msg = string("Parsing object file: ") + desc.file();

   statusLine(msg.c_str());

   //Now add Main and Dynamic Symbols if they are not present
   startup_printf("%s[%d]:  before findMain\n", FILE__, __LINE__);
   findMain();

   // define all of the functions
   statusLine("winnowing functions");

   // a vector to hold all created functions until they are properly classified
   	
   pdvector<image_func *> raw_funcs; 


   // define all of the functions, this also defines all of the modules
   startup_printf("%s[%d]:  before symbolsToFunctions\n", FILE__, __LINE__);
   if (!symbolsToFunctions(raw_funcs)) {
       fprintf(stderr, "Error converting symbols to functions in file %s\n", desc.file().c_str());
       err = true;
       return;
   }
 
   startup_printf("%s[%d]:  before buildFunctionLists\n", FILE__, __LINE__);
   if (!buildFunctionLists(raw_funcs)) {
       fprintf(stderr, "Error building function lists in file %s\n", desc.file().c_str());
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
   startup_printf("%s[%d]:  before checkAllCallPoints\n", FILE__, __LINE__);
   statusLine("checking call points");
   checkAllCallPoints();
#endif

}

image::~image() 
{
    // Doesn't do anything yet, moved here so we don't mess with symtab.h
    // Only called if we fail to create a process.
    // Or delete the a.out...

    unsigned i;

    for (map<Module *, pdmodule *>::iterator iter = mods_.begin();
         iter != mods_.end(); iter++) {
        delete (iter->second);
    }

    for (i = 0; i < everyUniqueFunction.size(); i++) {
        delete everyUniqueFunction[i];
    }
    everyUniqueFunction.clear();
    createdFunctions.clear();
    exportedFunctions.clear();

    for (i = 0; i < everyUniqueVariable.size(); i++) {
        delete everyUniqueVariable[i];
    }
    everyUniqueVariable.clear();
    createdVariables.clear();
    exportedVariables.clear();

   
    for (i = 0; i < parallelRegions.size(); i++)
      delete parallelRegions[i];
    parallelRegions.clear();
   
    
    // Finally, remove us from the image list.
    for (i = 0; i < allImages.size(); i++) {
        if (allImages[i] == this)
            VECTOR_ERASE(allImages,i,i);
    }

    if (pltFuncs) {
       delete pltFuncs;
       pltFuncs = NULL;
    }

    if (linkedFile) delete linkedFile;
#if defined (os_aix)
    //fprintf(stderr, "%s[%d]:  IMAGE DTOR:  archive = %p\n", FILE__, __LINE__, archive);
    if (archive) delete archive;
#endif
}

bool pdmodule::findFunction( const std::string &name, pdvector<image_func *> &found ) {
    if (findFunctionByMangled(name, found))
        return true;
    return findFunctionByPretty(name, found);
}

bool pdmodule::findFunctionByMangled( const std::string &name,
                                      pdvector<image_func *> &found)
{
    // For efficiency sake, we grab the image vector and strip out the
    // functions we want.
    // We could also keep them all in modules and ditch the image-wide search; 
    // the problem is that BPatch goes by module and internal goes by image. 
    unsigned orig_size = found.size();
    
    const pdvector<image_func *> *obj_funcs = imExec()->findFuncVectorByMangled(name.c_str());
    if (!obj_funcs) {
        return false;
    }
    for (unsigned i = 0; i < obj_funcs->size(); i++) {
        if ((*obj_funcs)[i]->pdmod() == this)
            found.push_back((*obj_funcs)[i]);
    }
    if (found.size() > orig_size) {
        //exec()->analyzeIfNeeded();
        return true;
    }
    
    return false;
}


bool pdmodule::findFunctionByPretty( const std::string &name,
                                     pdvector<image_func *> &found)
{
    // For efficiency sake, we grab the image vector and strip out the
    // functions we want.
    // We could also keep them all in modules and ditch the image-wide search; 
    // the problem is that BPatch goes by module and internal goes by image. 
    unsigned orig_size = found.size();
    
    const pdvector<image_func *> *obj_funcs = imExec()->findFuncVectorByPretty(name);
    if (!obj_funcs) {
        return false;
    }
    for (unsigned i = 0; i < obj_funcs->size(); i++) {
        if ((*obj_funcs)[i]->pdmod() == this)
            found.push_back((*obj_funcs)[i]);
    }
    if (found.size() > orig_size) {
        //exec()->analyzeIfNeeded();
        return true;
    }
    
    return false;
}

void pdmodule::dumpMangled(std::string &prefix) const
{
  cerr << fileName() << "::dumpMangled("<< prefix << "): " << endl;

  const pdvector<image_func *> allFuncs = imExec()->getAllFunctions();

  for (unsigned i = 0; i < allFuncs.size(); i++) {
      image_func * pdf = allFuncs[i];
      if (pdf->pdmod() != this) continue;

      if( ! strncmp( pdf->symTabName().c_str(), prefix.c_str(), strlen( prefix.c_str() ) ) ) {
          cerr << pdf->symTabName() << " ";
      }
      else {
          // bperr( "%s is not a prefix of %s\n", prefix, pdf->symTabName().c_str() );
      }
  }
  cerr << endl;
}

void pdmodule::addUnresolvedControlFlow(image_instPoint *badPt)
{ 
    unresolvedControlFlow.insert(badPt);
}

const std::set<image_instPoint*> &pdmodule::getUnresolvedControlFlow()
{ 
    imExec()->analyzeIfNeeded();
    return unresolvedControlFlow; 
}

/* This function is useful for seeding image parsing with function
 * stubs to start the control-flow-traversal parsing from.  The
 * function creates a module to add the symbol to if none exists.  
 * If parseState_ == analyzed, triggers parsing of the function
 */
image_func *image::addFunctionStub(Address functionEntryAddr, const char *fName)
 {
     // get or create module
     pdmodule *mod = getOrCreateModule(linkedFile->getDefaultModule());
     //KEVINTODO: if there are functions both preceding and succeeding
     //this one that lie in the same module, use that module instead

     // copy or create function name
     char funcName[32];
     if (fName) {
         snprintf( funcName, 32, "%s", fName);
     } else {
         snprintf( funcName, 32, "entry_%lx", functionEntryAddr);	
     }
     Symbol *funcSym = new Symbol(funcName,
                                  Symbol::ST_FUNCTION,
                                  Symbol::SL_GLOBAL, 
                                  Symbol::SV_DEFAULT,
                                  functionEntryAddr, 
                                  mod->mod(),
                                  NULL,
                                  UINT_MAX);
     // create function stub, update datastructures
     if (!linkedFile->addSymbol( funcSym )) {
         return NULL;
     }
     
     // Adding the symbol finds or creates a Function object...
     assert(funcSym->getFunction());
     image_func *func = 
        new image_func(funcSym->getFunction(), mod, this, FS_ONDEMAND);

     if (!func->getSymtabFunction()->addAnnotation(func, ImageFuncUpPtrAnno))
     {
        fprintf(stderr, "%s[%d]: failed to add annotation here\n", FILE__, __LINE__);
        return NULL;
     }

     // If this is a Dyninst dynamic heap placeholder, add it to the
     // list of inferior heaps...
     string compString = "DYNINSTstaticHeap";
     if (!func->symTabName().compare(0, compString.size(), compString)) {
         codeHeaps_.push_back(pair<string, Address>(func->symTabName(), func->getOffset()));
     }

     func->addSymTabName( funcName ); 
     func->addPrettyName( funcName );
     // funcsByEntryAddr[func->getOffset()] = func;
     everyUniqueFunction.push_back(func);
     createdFunctions.push_back(func);
     return func;
}

const string &pdmodule::fileName() const
{
    return mod_->fileName();
}

const string &pdmodule::fullName() const
{
    return mod_->fullName();
}

supportedLanguages pdmodule::language() const
{
    return mod_->language();
}

Address pdmodule::addr() const
{
    return mod_->addr();
}

bool pdmodule::isShared() const
{
    return mod_->isShared();
}

Module *pdmodule::mod()
{
    return mod_;
}

pdmodule *image::getOrCreateModule(Module *mod) {
    if (mods_.find(mod) != mods_.end())
        return mods_[mod];

    pdmodule *pdmod = new pdmodule(mod, this);

    mods_[mod] = pdmod;
    modsByFileName[pdmod->fileName()] = pdmod;
    modsByFullName[pdmod->fullName()] = pdmod;
    
    return pdmod;
}


/*********************************************************************/
/**** Function lookup (by name or address) routines               ****/
/*********************************************************************/

// Find the function that occupies the given offset. ONLY LOOKS IN THE
// IMAGE, and does not consider relocated functions. Those must be searched
// for on the process level (as relocated functions are per-process)
codeRange *image::findCodeRangeByOffset(const Address &offset) {
    codeRange *range;
    analyzeIfNeeded();
    if (!funcsByRange.find(offset, range)) {
        return NULL;
    }
    return range;
}

// Similar to above, but checking by entry (only). Useful for 
// "who does this call" lookups
image_func *image::findFuncByEntry(const Address &entry) {
    image_func *func;
    analyzeIfNeeded();
    if (funcsByEntryAddr.find(entry, func)) {
      return func;
    } 
    else
      return NULL;
}

image_basicBlock *image::findBlockByAddr(const Address &addr) {
    codeRange *range;
    if (!basicBlocksByRange.find(addr, range)) {
        return NULL;
    }
    return range->is_image_basicBlock();
}

// Return the vector of functions associated with a pretty (demangled) name
// Very well might be more than one!

const pdvector<image_func *> *image::findFuncVectorByPretty(const std::string &name) {
    //Have to change here
    pdvector<image_func *>* res = new pdvector<image_func *>;
    vector<Function *> funcs;
    linkedFile->findFunctionsByName(funcs, name.c_str(), prettyName);

    for(unsigned index=0; index < funcs.size(); index++)
    {
        Function *symFunc = funcs[index];
        image_func *imf = NULL;
        
        if (!symFunc->getAnnotation(imf, ImageFuncUpPtrAnno)) {
            fprintf(stderr, "%s[%d]:  failed to getAnnotations here\n", FILE__, __LINE__);
            return NULL;
        }
        
        if (imf) {
            res->push_back(imf);
        }


    }		
    if(res->size())	
	return res;	    
    else {
        delete res;
    	return NULL;
    }
}

// Return the vector of functions associated with a mangled name
// Very well might be more than one! -- multiple static functions in different .o files

const pdvector <image_func *> *image::findFuncVectorByMangled(const std::string &name)
{
    pdvector<image_func *>* res = new pdvector<image_func *>;

    vector<Function *> funcs;
    linkedFile->findFunctionsByName(funcs, name.c_str(), mangledName);

    for(unsigned index=0; index < funcs.size(); index++) {
        Function *symFunc = funcs[index];
        image_func *imf = NULL;
        
        if (!symFunc->getAnnotation(imf, ImageFuncUpPtrAnno)) {
            fprintf(stderr, "%s[%d]:  failed to getAnnotations here\n", FILE__, __LINE__);
            return NULL;
        }
        
        if (imf) {
            res->push_back(imf);
        }

    }	    
    if(res->size()) 
	return res;	    
    else {
        delete res;
    	return NULL;
    }   
}

const pdvector <image_variable *> *image::findVarVectorByPretty(const std::string &name)
{
    pdvector<image_variable *>* res = new pdvector<image_variable *>;

    vector<Variable *> vars;
    linkedFile->findVariablesByName(vars, name.c_str(), prettyName);
    
    for (unsigned index=0; index < vars.size(); index++) {
        Variable *symVar = vars[index];
        image_variable *imv = NULL;
        
        if (!symVar->getAnnotation(imv, ImageVariableUpPtrAnno)) {
            fprintf(stderr, "%s[%d]:  failed to getAnnotations here\n", FILE__, __LINE__);
            return NULL;
        }

       if (imv) {
           res->push_back(imv);
       }
    }	    
    if(res->size())	
	return res;	    
    else {
        delete res;
    	return NULL;
    }
}

const pdvector <image_variable *> *image::findVarVectorByMangled(const std::string &name)
{
    //    fprintf(stderr,"findVariableVectorByPretty %s\n",name.c_str());
#ifdef IBM_BPATCH_COMPAT_STAB_DEBUG
    bperr( "%s[%d]:  inside findVariableVectorByPretty\n", FILE__, __LINE__);
#endif
    pdvector<image_variable *>* res = new pdvector<image_variable *>;

    vector<Variable *> vars;
    linkedFile->findVariablesByName(vars, name.c_str(), mangledName);
    
    for (unsigned index=0; index < vars.size(); index++) {
        Variable *symVar = vars[index];
        image_variable *imv = NULL;
        
        if (!symVar->getAnnotation(imv, ImageVariableUpPtrAnno)) {
            fprintf(stderr, "%s[%d]:  failed to getAnnotations here\n", FILE__, __LINE__);
            return NULL;
        }

        if (imv) {
            res->push_back(imv);
        }
    }	    
    if(res->size())	
	return res;	    
    else {
        delete res;
    	return NULL;
    }
  /*
  if (varsByMangled.defines(name)) {
      //analyzeIfNeeded();
      return varsByMangled[name];
  }
  return NULL;*/
}


/* Instrumentable-only, by the last version's source. */
bool pdmodule::getFunctions(pdvector<image_func *> &funcs)  {
    pdvector<image_func *> allFuncs = imExec()->getAllFunctions();
    unsigned curFuncSize = funcs.size();

    for (unsigned i = 0; i < allFuncs.size(); i++) {
        if (allFuncs[i]->pdmod() == this)
            funcs.push_back(allFuncs[i]);
    }
  
    return (funcs.size() > curFuncSize);
} /* end getFunctions() */

/* Instrumentable-only, by the last version's source. */
bool pdmodule::getVariables(pdvector<image_variable *> &vars)  {
    pdvector<image_variable *> allVars = imExec()->getAllVariables();
    unsigned curVarSize = vars.size();

    for (unsigned i = 0; i < allVars.size(); i++) {
        if (allVars[i]->pdmod() == this)
            vars.push_back(allVars[i]);
    }
  
    return (vars.size() > curVarSize);
} /* end getFunctions() */


void *image::getPtrToDataInText( Address offset ) const {
    if( isData(offset) ) { return NULL; }
    if( ! isCode(offset) ) { return NULL; }
	
    Region *reg = linkedFile->findEnclosingRegion(offset);
    if(reg != NULL) {
        return (void*) ((Address)reg->getPtrToRawData() + offset 
                        - reg->getRegionAddr());
    }
    return NULL;
} /* end getPtrToDataInText() */

void *image::getPtrToData(Address offset) const {
    if (!isData(offset)) return NULL;
    Region *reg = linkedFile->findEnclosingRegion(offset);
#if 0
    if (reg != NULL &&
        (reg->getRegionPermissions() == Region::RP_RW ||
         reg->getRegionPermissions() == Region::RP_RWX)) {
        return (void*) ((Address)reg->getPtrToRawData() + offset 
                        - reg->getRegionAddr());
    }
#endif
    if (reg) return (void *) ((Address)reg->getPtrToRawData() +
                              offset -
                              reg->getRegionAddr());
    return NULL;
}
    
// return a pointer to the instruction at address adr
void *image::getPtrToInstruction(Address offset) const 
{
    // isCode and isData cover this already, no point in duplicating effort
//   if (!isValidAddress(offset))
//      return NULL;

   if (isCode(offset)) {
      Region *reg = linkedFile->findEnclosingRegion(offset);
#if 0
      if (reg != NULL &&
          (reg->getRegionPermissions() == Region::RP_RX ||
           reg->getRegionPermissions() == Region::RP_RWX ||
           reg->getRegionType() == Region::RT_TEXT ||
           reg->getRegionType() == Region::RT_TEXTDATA)) {
          return (void*) ((Address)reg->getPtrToRawData() 
                          + offset - reg->getRegionAddr());
      }
#endif
      if (reg) return (void *) ((Address)reg->getPtrToRawData() +
                                offset -
                                reg->getRegionAddr());
      //return NULL;
   }
   else if (isData(offset)) { // not sure why we allow this
       return getPtrToData(offset);
   }
   return NULL;
}

// Address must be in code or data range since some code may end up
// in the data segment
bool image::isValidAddress(const Address &where) const{
    Address addr = where;
    return linkedFile->isValidOffset(addr) && isAligned(addr);
}

bool image::isExecutableAddress(const Address &where) const {
    return isCode(where);
}

bool image::isCode(const Address &where)  const{
   Address addr = where;
   return linkedFile->isCode(addr); 
}

bool image::isData(const Address &where)  const{
   Address addr = where;
   return linkedFile->isData(addr); 
}

Symbol *image::symbol_info(const std::string& symbol_name) {
   vector< Symbol *> symbols;
   if(!(linkedFile->findSymbol(symbols,symbol_name.c_str(),Symbol::ST_UNKNOWN, anyName))) 
       return false;

   return symbols[0];
}


bool image::findSymByPrefix(const std::string &prefix, pdvector<Symbol *> &ret) {
    unsigned start;
    vector <Symbol *>found;	
    std::string reg = prefix+std::string("*");
    if(!linkedFile->findSymbol(found, reg.c_str(), Symbol::ST_UNKNOWN, anyName, true))
    	return false;
    for(start=0;start< found.size();start++)
		ret.push_back(found[start]);
	return true;	
}

dictionary_hash<Address, std::string> *image::getPltFuncs()
{
   bool result;
   if (pltFuncs)
      return pltFuncs;


   vector<relocationEntry> fbt;
   result = getObject()->getFuncBindingTable(fbt);
   if (!result)
      return NULL;

   pltFuncs = new dictionary_hash<Address, std::string>(addrHash);
   assert(pltFuncs);
   for(unsigned k = 0; k < fbt.size(); k++)
      (*pltFuncs)[fbt[k].target_addr()] = fbt[k].name().c_str();
   return pltFuncs;
}

image_variable* image::createImageVariable(Offset offset, std::string name, int size, pdmodule *mod)
{
    // What to do here?
    if (varsByAddr.defines(offset))
        return varsByAddr[offset];

    Variable *sVar = getObject()->createVariable(name, offset, size, mod->mod());

    image_variable *ret = new image_variable(sVar, mod);

    extern AnnotationClass<image_variable> ImageVariableUpPtrAnno;
    if (!sVar->addAnnotation(ret, ImageVariableUpPtrAnno)) {
        fprintf(stderr, "%s[%d]:  failed to add annotation here\n", FILE__, __LINE__);
    }

    createdVariables.push_back(ret);
    everyUniqueVariable.push_back(ret);
    varsByAddr[offset] = ret;
    return ret;
}
