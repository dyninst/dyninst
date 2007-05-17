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

 // $Id: symtab.C,v 1.293 2007/05/17 20:01:32 legendre Exp $

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/arch.h"
#include "dyninstAPI/src/instPoint.h"
#include <fstream>
#include "dyninstAPI/src/util.h"
#include "common/h/String.h"
#include "dyninstAPI/src/inst.h"
#include "common/h/Timer.h"
#include "dyninstAPI/src/debug.h"
#include "common/h/debugOstream.h"
#include "common/h/pathName.h"          // extract_pathname_tail()
#include "dyninstAPI/src/function.h"

#include "LineInformation.h"
#include "dyninstAPI/h/BPatch_flowGraph.h"

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

pdstring fileDescriptor::emptyString(pdstring(""));
fileDescriptor::fileDescriptor() {
    // This shouldn't be called... must be public for pdvector, though
}

bool fileDescriptor::IsEqual(const fileDescriptor &fd) const {
  // Don't test isShared, only file name and addresses
  bool file_match_ = false;

  // Annoying... we often get "foo vs ./foo" or such. So consider it a match
  // if either file name is prefixed by the other; we don't get trailing crud.
  if (fd.file_.suffixed_by(file_) ||
      file_.suffixed_by(fd.file_)) file_match_ = true;

  if (file_match_ &&
      (code_ == fd.code_) &&
      (data_ == fd.data_) &&
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
    fileNm = extract_pathname_tail(name).c_str();

	// /* DEBUG */ fprintf( stderr, "%s[%d]: Creating new pdmodule '%s'/'%s'\n", FILE__, __LINE__, fileNm.c_str(), fullNm.c_str() );
    ret = new pdmodule(lang, addr, fullNm, this);
    modsByFileName[ret->fileName().c_str()] = ret;
    modsByFullName[ret->fullName().c_str()] = ret;
    _mods.push_back(ret);

    return(ret);
}


// makeOneFunction(): find name of enclosing module and define function symbol
//
// module information comes from one of three sources:
//   #1 - debug format (stabs, DWARF, etc.)
//   #2 - file format (ELF, COFF)
//   #3 - file name (a.out, libXXX.so)
// (in order of decreasing reliability)
image_func *image::makeOneFunction(vector<Dyn_Symbol *> &mods,
				     Dyn_Symbol *lookUp) 
{
  // find module name
  Address modAddr = 0;
  pdstring modName = lookUp->getModuleName().c_str();
  // /* DEBUG */ fprintf( stderr, "%s[%d]: makeOneFunction()'s module: %s\n", FILE__, __LINE__, modName.c_str() );
  
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
          lookUp->getName().c_str(), 
          lookUp->getAddr());
#else
  parsing_printf("New function %s at 0x%x\n",
          lookUp->getName().c_str(), 
          lookUp->getAddr());
#endif

  image_func *func = new image_func(lookUp, 
                                    use, 
                                    this);
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
    	vector <Dyn_Symbol *>syms;
    	if((linkedFile->findSymbolByType(syms,"main",Dyn_Symbol::ST_UNKNOWN)==true)||(linkedFile->findSymbolByType(syms,"_main",Dyn_Symbol::ST_UNKNOWN)==true))
    		foundMain = true;
    	else if(linkedFile->findSymbolByType(syms,"_start",Dyn_Symbol::ST_UNKNOWN)==true)
    		foundStart = true;
    	else if(linkedFile->findSymbolByType(syms,"_fini",Dyn_Symbol::ST_UNKNOWN)==true)
    		foundFini = true;
    
    	Dyn_Section *textsec;
    	linkedFile->findSection(".text", textsec);
	
    	if( !foundMain )
    	{
    	    //find and add main to allsymbols
            const unsigned char* p;
	    Dyn_Section *sec;
	    linkedFile->findSection(".text", sec);
		                   
	    //p = ( const unsigned char* )elf_vaddr_to_ptr( sec->getSecAddr());
	    p = (( const unsigned char * )linkedFile->code_ptr()) + (sec->getSecAddr() - codeOffset_);
            const unsigned char *lastP = 0;

            switch(linkedFile->getAddressWidth()) {
       	    	case 4:
            	    // 32-bit...
            	    ia32_set_mode_64(false);
                    break;
         	case 8:
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
            if (linkedFile->getAddressWidth() == 8)
                mainAddress = get_disp(&preCall);
            else
                // get_disp doesn't work here... skip the "push" opcode and grab the immediate
            	mainAddress = *(unsigned *)(lastP+1);


            logLine( "No main symbol found: creating symbol for main\n" );

    	    Dyn_Section *pltsec;
            if((linkedFile->findSection(".plt",pltsec)) && pltsec->isOffsetInSection(mainAddress))
            {
            	//logLine( "No static symbol for function main\n" );
                Dyn_Symbol *newSym = new Dyn_Symbol("DYNINST_pltMain", "DEFAULT_MODULE",
                          Dyn_Symbol::ST_FUNCTION,
                          Dyn_Symbol::SL_GLOBAL, mainAddress,textsec, UINT_MAX );
        
                linkedFile->addSymbol( newSym );
           }
           else
           {
           	Dyn_Symbol *newSym= new Dyn_Symbol( "main", "DEFAULT_MODULE", Dyn_Symbol::ST_FUNCTION,
                           Dyn_Symbol::SL_GLOBAL, mainAddress,textsec, UINT_MAX );
	        linkedFile->addSymbol(newSym);		
           }
        }
    	if( !foundStart )
    	{
            Dyn_Symbol *startSym = new Dyn_Symbol( "_start", "DEFAULT_MODULE", Dyn_Symbol::ST_FUNCTION,
                   Dyn_Symbol::SL_GLOBAL, textsec->getSecAddr(),textsec, UINT_MAX );
    
            //cout << "sim for start!" << endl;
        
	    linkedFile->addSymbol(startSym);		
    	}
    	if( !foundFini )
    	{
	    Dyn_Section *finisec;
	    linkedFile->findSection(".fini",finisec);
            Dyn_Symbol *finiSym = new Dyn_Symbol( "_fini","DEFAULT_MODULE", Dyn_Symbol::ST_FUNCTION,
                        Dyn_Symbol::SL_GLOBAL, finisec->getSecAddr(), finisec, UINT_MAX );
	    linkedFile->addSymbol(finiSym);		
    	}
    }	

    Dyn_Section *dynamicsec;
    vector < Dyn_Symbol *>syms;
    if(linkedFile->findSection(".dynamic", dynamicsec)==true)
    {
        if(linkedFile->findSymbolByType(syms,"_DYNAMIC",Dyn_Symbol::ST_UNKNOWN)==false)
        {
	    Dyn_Symbol *newSym = new Dyn_Symbol( "_DYNAMIC", "DEFAULT_MODULE", 
					Dyn_Symbol::ST_OBJECT, Dyn_Symbol::SL_GLOBAL,
					dynamicsec->getSecAddr(), dynamicsec, 0 );
	    linkedFile->addSymbol(newSym);
	}
    }
    
#elif defined(rs6000_ibm_aix4_1) || defined(rs6000_ibm_aix5_1)
   
   bool foundMain = false;
   vector <Dyn_Symbol *>syms;
   if(linkedFile->findSymbolByType(syms,"main",Dyn_Symbol::ST_UNKNOWN)==true)
   	foundMain = true;
   
   Dyn_Section *sec;
   linkedFile->findSection(".text", sec); 	

   if( !foundMain && linkedFile->isExec() )
   {
       //we havent found a symbol for main therefore we have to parse _start
       //to find the address of main

       //last two calls in _start are to main and exit
       //find the end of _start then back up to find the target addresses
       //for exit and main
      
       int c;
       instructUnion i;
       int calls = 0;
       Word *code_ptr_ = (Word *) linkedFile->code_ptr();
       
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
       
       OFFSET currAddr = sec->getSecAddr() + c * instruction::size();
       OFFSET mainAddr = 0;
       
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
               mainAddr = (OFFSET)disp;
           }
           else
               mainAddr = (OFFSET)( currAddr + disp );      
       }  
       
       Dyn_Symbol *sym = new Dyn_Symbol( "main",  "DEFAULT_MODULE", Dyn_Symbol::ST_FUNCTION,
                   Dyn_Symbol::SL_GLOBAL, mainAddr, sec);
       linkedFile->addSymbol(sym);
      
   
       //since we are here make up a sym for _start as well

       Dyn_Symbol *sym1 = new Dyn_Symbol( "__start", "DEFAULT_MODULE", Dyn_Symbol::ST_FUNCTION,
                   Dyn_Symbol::SL_GLOBAL, sec->getSecAddr(), sec);
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
																	
	if(linkedFile->isExec())
	//if( !( peHdr->FileHeader.Characteristics & IMAGE_FILE_DLL ) )
    {
        const unsigned char *t, *p;
		vector <Dyn_Symbol *>syms;
		Address eAddr = linkedFile->getEntryAddress();
        //Address eAddr = peHdr->OptionalHeader.AddressOfEntryPoint;

        bool found_main = false;
        for (unsigned i=0; i<NUMBER_OF_MAIN_POSSIBILITIES; i++) {
			if(linkedFile->findSymbolByType(syms,main_function_names[i],Dyn_Symbol::ST_UNKNOWN)) {
        //    if (symbols_.defines(main_function_names[i])) {
                found_main = true;
                break;
            }
        }
	    if (!found_main)
		{
			Address eAddr = linkedFile->getEntryAddress();
            //Address curr = eAddr + (Address) dw64BaseAddr;
            Address curr = eAddr;
			PVOID mapAddr = (PVOID)linkedFile->getBaseAddress();
			PIMAGE_NT_HEADERS peHdr = ImageNtHeader( mapAddr ); //PE File Header
   
			p = (const unsigned char*) ImageRvaToVa(peHdr, mapAddr, eAddr, 0);
            instruction insn((const void *)p);
            while( !insn.isReturn() )
            {
              if ( insn.isTrueCallInsn() )  {
                Address callTarget = insn.getTarget(curr);
                Address endTarget = callTarget;

                // Sometimes the call to main bounces through a direct ILT table jump.
                //  If so, take the jump target as main's address.
                t = (const unsigned char *) ImageRvaToVa(peHdr, mapAddr, callTarget, 0);
                if (t) {
                    instruction insn(t);
                    if (insn.isJumpDir())
                        endTarget = insn.getTarget(callTarget);
                }
 
                bool found = false;
                for (unsigned i=0; i<possible_mains.size(); i++) {
                    if (possible_mains[i] == endTarget) {
                        found = true;
                        break;
                    }                
                }
                if (!found) {
                    possible_mains.push_back( endTarget );
                }
              }
              curr += insn.size();
              p += insn.size();
              insn.setInstruction(p);
            }
			syms.clear();
			if(linkedFile->findSymbolByType(syms,"DEFAULT_MODULE",Dyn_Symbol::ST_UNKNOWN)) {
        //  if (!symbols_.defines("DEFAULT_MODULE")) {
                //make up a symbol for default module too
				Dyn_Symbol *modSym = new Dyn_Symbol("DEFAULT_MODULE", "DEFAULT_MODULE", Dyn_Symbol::ST_MODULE, 
					Dyn_Symbol::SL_GLOBAL, codeOffset_, NULL, 0);
				linkedFile->addSymbol(modSym);
            }
			syms.clear();
			if(linkedFile->findSymbolByType(syms,"start",Dyn_Symbol::ST_UNKNOWN)) {
        //  if (!symbols_.defines("start")) {
                //use 'start' for mainCRTStartup.
                Dyn_Symbol *startSym = new Dyn_Symbol( "start", "DEFAULT_MODULE", Dyn_Symbol::ST_FUNCTION,
                                Dyn_Symbol::SL_GLOBAL, eAddr , 0, UINT_MAX );
				linkedFile->addSymbol(startSym);
            }
			syms.clear();
			if(linkedFile->findSymbolByType(syms,"winStart",Dyn_Symbol::ST_UNKNOWN)) {
        //  if (!symbols_.defines("winStart")) {
                //make up a func name for the start of the text section
                Dyn_Symbol *sSym = new Dyn_Symbol( "winStart", "DEFAULT_MODULE", Dyn_Symbol::ST_FUNCTION,
					Dyn_Symbol::SL_GLOBAL, codeOffset_, 0, UINT_MAX );
            	linkedFile->addSymbol(sSym);
            }
			syms.clear();
			if(linkedFile->findSymbolByType(syms,"winFini",Dyn_Symbol::ST_UNKNOWN)) {
        //  if (!symbols_.defines("winFini")) {
                //make up one for the end of the text section
                Dyn_Symbol *fSym = new Dyn_Symbol( "winFini", "DEFAULT_MODULE", Dyn_Symbol::ST_FUNCTION,
                            Dyn_Symbol::SL_GLOBAL, codeOffset_ + linkedFile->codeLength() - 1, 
                            0, UINT_MAX );
            	linkedFile->addSymbol(fSym);
			}
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

bool image::symbolsToFunctions(vector<Dyn_Symbol *> &mods,
			       pdvector<image_func *> *raw_funcs)
{
#if defined(TIMED_PARSE)
  struct timeval starttime;
  gettimeofday(&starttime, NULL);
#endif

  Dyn_Symbol *lookUp;
  pdvector< Dyn_Symbol *> lookUps;
  pdstring symString;

  // is_a_out is a member variable
  Dyn_Symbol *mainFuncSymbol;  //Keeps track of info on "main" function

  //Checking "main" function names in same order as in the inst-*.C files
  vector <Dyn_Symbol *>syms;
   if (linkedFile->findSymbolByType(syms,"main",Dyn_Symbol::ST_FUNCTION)       ||
   	linkedFile->findSymbolByType(syms,"_main",Dyn_Symbol::ST_FUNCTION)     
#if defined(os_windows)
	|| linkedFile->findSymbolByType(syms,"WinMain",Dyn_Symbol::ST_FUNCTION)   ||
	linkedFile->findSymbolByType(syms,"_WinMain",Dyn_Symbol::ST_FUNCTION)  ||
	linkedFile->findSymbolByType(syms,"wWinMain",Dyn_Symbol::ST_FUNCTION)  ||
	linkedFile->findSymbolByType(syms,"_wWinMain",Dyn_Symbol::ST_FUNCTION) 
#endif
      )
  {
      assert( syms.size() == 1 );
      lookUp = syms[0];
      
      mainFuncSymbol = lookUp;
      is_a_out = true;
      if (lookUp->getType() == Dyn_Symbol::ST_FUNCTION) {
          if (!isValidAddress(lookUp->getAddr())) {
              pdstring msg;
              char tempBuffer[40];
              sprintf(tempBuffer,"0x%lx",lookUp->getAddr());
              msg = pdstring("Function ") + lookUp->getName().c_str() + pdstring(" has bad address ")
              + pdstring(tempBuffer);
              statusLine(msg.c_str());
              showErrorCallback(29, msg);
              bperr( "Whoops\n");
              
              return false;
          }      
          image_func *main_pdf = makeOneFunction(mods, lookUp);
		  lookUp->setUpPtr((void *)main_pdf);	// set the back ptr in the symbol;
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
  vector< Dyn_Symbol *>symvector;
  if (linkedFile->findSymbolByType(symvector,"DYNINSTinit",Dyn_Symbol::ST_UNKNOWN) ||
      linkedFile->findSymbolByType(symvector,"_DYNINSTinit",Dyn_Symbol::ST_UNKNOWN))
    is_libdyninstRT = true;
  else
    is_libdyninstRT = false;
 
  // find the real functions -- those with the correct type in the symbol table
  vector<Dyn_Symbol *> allFuncs;
  if(!linkedFile->getAllSymbolsByType(allFuncs,Dyn_Symbol::ST_FUNCTION))
  	return true;
  vector<Dyn_Symbol *>::iterator symIter = allFuncs.begin();
  for(; symIter!=allFuncs.end();symIter++) {
    Dyn_Symbol *lookUp = *symIter;
    const char *np = lookUp->getName().c_str();

    //parsing_printf("Scanning file: symbol %s\n", lookUp.getName().c_str());

    //    fprintf(stderr,"np %s\n",np);

    if (np[0] == '.')
         /* ignore these EEL symbols; we don't understand their values */
	 continue; 
    if (is_a_out && 
	(lookUp->getAddr() == mainFuncSymbol->getAddr()) &&
	(lookUp->getName() == mainFuncSymbol->getName()))
      // We already added main(), so skip it now
      continue;

    if (lookUp->getModuleName() == "DYNINSTheap") {
        // Do nothing for now; we really don't want to report it as
        // a real symbol.
    }
    else 
    {
        image_func *new_func = makeOneFunction(mods, lookUp);
		//new_func->symbol()->setUpPtr(new_func);
		lookUp->setUpPtr((void *)new_func);
        if (!new_func)
            fprintf(stderr, "%s[%d]:  makeOneFunction failed\n", FILE__, __LINE__);
        else
            raw_funcs->push_back(new_func);
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

   vector<Dyn_Symbol *> allVars;
   linkedFile->getAllSymbolsByType(allVars,Dyn_Symbol::ST_OBJECT);
   vector<Dyn_Symbol *>::iterator symIter = allVars.begin();

   for(; symIter!=allVars.end() ; symIter++) {
      const pdstring &mangledName = (*symIter)->getName().c_str();
      Dyn_Symbol *symInfo = *symIter;
#if 0
      fprintf(stderr, "Symbol %s, mod %s, addr 0x%x, type %d, linkage %d (obj %d, func %d)\n",
              symInfo.getName().c_str(),
              symInfo.getModuleName().c_str(),
              symInfo.getAddr(),
              symInfo.getType(),
              symInfo.getLinkage(),
              Symbol::ST_OBJECT,
              Symbol::ST_FUNCTION);
#endif
#if !defined(os_windows)
      // Windows: variables are created with an empty module
      if (symInfo->getModuleName().length() == 0) {
          //fprintf(stderr, "SKIPPING EMPTY MODULE\n");
          continue;
      }
#endif
      image_variable *var;
      //bool addToPretty = false;
      if (varsByAddr.defines(symInfo->getAddr())) {
          var = varsByAddr[symInfo->getAddr()];
          var->addSymTabName(mangledName);
	  //addVariableName(var,mangledName,true);
	  //addVariableName(var,symInfo->getPrettyName().c_str(),false);
      }
      else {
          parsing_printf("New variable, mangled %s, module %s...\n",
                          mangledName.c_str(),
                          symInfo->getModuleName().c_str());
          pdmodule *use = getOrCreateModule(symInfo->getModuleName().c_str(),
                                              symInfo->getAddr());
          assert(use);
	  var = new image_variable(symInfo, use);
	  var->symbol()->setUpPtr((void *)var);
          exportedVariables.push_back(var);
          everyUniqueVariable.push_back(var);
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

   if(linkedFile->get_symbols(name,lookUps)){
      if( lookUps.size() == 1 ) { lookUp = lookUps[0]; }
      else { return false; } 
      ret_sym = internalSym(lookUp.addr(),name); 
      return true;
   }
   else {
       pdstring new_sym;
       new_sym = pdstring("_") + name;
       if(linkedFile->get_symbols(new_sym,lookUps)){
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
          if (name.wildcardEquiv(mod->fileName().c_str()) ||
              name.wildcardEquiv(mod->fullName().c_str())) {
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

const pdvector <pdmodule*> &image::getModules() 
{
  return _mods;
}

void print_module_vector_by_short_name(pdstring prefix ,
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
void image::findModByAddr (const Dyn_Symbol *lookUp, vector<Dyn_Symbol *> &mods,
			   pdstring &modName, Address &modAddr, 
			   const pdstring &defName)
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

image *image::parseImage(fileDescriptor &desc)
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
          return allImages[u]->clone();
      }
  }

  stats_parse.startTimer(PARSE_SYMTAB_TIMER);

  /*
   * load the symbol table. (This is the a.out format specific routine).
   */
  if(desc.isSharedObject()) 
    statusLine("Processing a shared object file");
  else  
    statusLine("Processing an executable file");
  
  bool err=false;

#if defined(TIMED_PARSE)
  struct timeval starttime;
  gettimeofday(&starttime, NULL);
#endif

  image *ret = new image(desc, err); 

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
      if (err)
          fprintf(stderr, "Error parsing file %s, skipping...\n", desc.file().c_str());

     if (ret) {
        delete ret;
     }
     else {
         fprintf(stderr, "Failed to allocate memory for parsing %s!\n", 
                 desc.file().c_str());
     }
     stats_parse.stopTimer(PARSE_SYMTAB_TIMER);
     return NULL;
  }

  image::allImages.push_back(ret);

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

int image::destroy() {
    refCount--;
    if (refCount == 0) {
        if (!desc().isSharedObject()) {
            // a.out... destroy it
#if defined(os_aix)
            // "If we're on a platform where the Object-class destructor is trusted...
            delete this;
#endif
            return 0;
        }
    }
    if (refCount < 0)
        assert(0 && "NEGATIVE REFERENCE COUNT FOR IMAGE!");
    return refCount; 
}

// Enter a function in all the appropriate tables
void image::enterFunctionInTables(image_func *func, bool wasSymtab) {
    if (!func) return;
    
    //funcsByEntryAddr[func->getOffset()] = func;
    
    // Functions added during symbol table parsing do not necessarily
    // have valid sizes set, and should therefor not be added to
    // the code range tree. They will be added after parsing. 
    if(!wasSymtab) {
        // TODO: out-of-line insertion here
        if (func->get_size_cr())
          funcsByRange.insert(func);
        Dyn_Symbol *sym = func->symbol();
        getObject()->addSymbol(sym);
    }
    
    everyUniqueFunction.push_back(func);
    if (wasSymtab)
        exportedFunctions.push_back(func);
    else
        createdFunctions.push_back(func);
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

	// Now, we see if there's already a function object for this
        // address. If so, add a new name;

	image_func *possiblyExistingFunction = NULL;
	funcsByEntryAddr.find(raw->getOffset(), possiblyExistingFunction);
        if (!possiblyExistingFunction)
	{
            funcsByEntryAddr[raw->getOffset()] = raw;
	    pdstring name = (raw->symTabNameVector())[raw->symTabNameVector().size()-1].c_str();
            raw->addSymTabName(name);
        }
    }
    
    // Now that we have a 1) unique and 2) demangled list of function
    // names, loop through once more and build the address range tree
    // and name lookup tables. 
    for (unsigned j = 0; j < raw_funcs.size(); j++) {
        image_func *func = raw_funcs[j];
        if (!func) continue;
        
        // May be NULL if it was an alias.
        enterFunctionInTables(func, true);
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
   knownJumpTargets(int_addrHash, 8192),
   _mods(0),
   funcsByEntryAddr(addrHash4),
   nextBlockID_(0),
   modsByFileName(pdstring::hash),
   modsByFullName(pdstring::hash),
   varsByAddr(addrHash4),
   refCount(1),
   parseState_(unparsed)
{
#if defined(os_aix)
   string file = desc_.file().c_str();
   SymtabError serr = Not_An_Archive;
   Dyn_Archive *archive;
   if(!Dyn_Archive::openArchive(file, archive))
   {
   	err = true;
	if(archive->getLastError() != serr)
		return;
	else
	{
   		if(!Dyn_Symtab::openFile(file, linkedFile)) 
   		{
   			err = true;
			return;
		}
	}
   }
   else
   {
   	string member = desc_.member().c_str();
   	if(!archive->getMember(member, linkedFile))
   	{
   		err = true;
		return;
	}
   }
#else
   string file = desc_.file().c_str();
   //linkedFile = new Dyn_Symtab();
   if(!Dyn_Symtab::openFile(file, linkedFile)) 
   {
   	err = true;
	return;
   }
#endif  
   baseAddr_ = desc.loadAddr();
   err = false;
   name_ = extract_pathname_tail(desc.file()).c_str();

   //   fprintf(stderr,"img name %s\n",name_.c_str());
   pathname_ = desc.file();

   // initialize (data members) codeOffset_, dataOffset_,
   //  codeLen_, dataLen_.
   
   codeOffset_ = linkedFile->codeOffset();
   dataOffset_ = linkedFile->dataOffset();
   
   codeLen_ = linkedFile->codeLength();
   dataLen_ = linkedFile->dataLength();

   
   // if unable to parse object file (somehow??), try to
   //  notify user/calling process + return....    
   if (!codeLen_ || !linkedFile->code_ptr()) {
      pdstring msg = pdstring("Parsing problem with executable file: ") + desc.file();
      statusLine(msg.c_str());
      msg += "\n";
      logLine(msg.c_str());
      err = true;
      BPatch_reportError(BPatchWarning, 27, msg.c_str()); 
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

   pdstring msg;
   // give luser some feedback....
   msg = pdstring("Parsing object file: ") + desc.file();
   
   statusLine(msg.c_str());

   //Now add Main and Dynamic Symbols if they are not present
   findMain();

   vector<Dyn_Symbol *> uniqMods;
   linkedFile->getAllSymbolsByType(uniqMods, Dyn_Symbol::ST_MODULE);

#if 0		// Moved to symtabAPI 
#if defined(os_solaris) || defined(os_aix) || defined(os_linux)
   // make sure we're using the right demangler

   nativeCompiler = parseCompilerType(&linkedFile);
   parsing_printf("isNativeCompiler: %d\n", nativeCompiler);
#endif
#endif

   // define all of the functions
   statusLine("winnowing functions");
  
   // a vector to hold all created functions until they are properly classified
   	
   pdvector<image_func *> raw_funcs; 

   // define all of the functions, this also defines all of the modules
   if (!symbolsToFunctions(uniqMods, &raw_funcs)) {
       fprintf(stderr, "Error converting symbols to functions in file %s\n", desc.file().c_str());
      err = true;
      return;
   }
 
   //Now done in the symtab land
 #if 0  
   // wait until all modules are defined before applying languages to
   // them we want to do it this way so that module information comes
   // from the function symbols, first and foremost, to avoid any
   // internal module-function mismatching.
  
   // get Information on the language each modules is written in
   // (prior to making modules)
   dictionary_hash<pdstring, supportedLanguages> mod_langs(pdstring::hash);
   getModuleLanguageInfo(&mod_langs);
   setModuleLanguages(&mod_langs);
 #endif  

   // Once languages are assigned, we can build demangled names (in
   // the wider sense of demangling which includes stripping _'s from
   // fortran names -- this is why language information must be
   // determined before this step).

   // Also identifies aliases (multiple names with equal addresses)

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

    // linkedFile : static, will go away automatically
    for (i = 0; i < _mods.size(); i++) {
        delete _mods[i];
    }
    _mods.clear();

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

    // Finally, remove us from the image list.
    for (i = 0; i < allImages.size(); i++) {
        if (allImages[i] == this)
            allImages.erase(i,i);
    }

}

bool pdmodule::findFunction( const pdstring &name, pdvector<image_func *> &found ) {
    if (findFunctionByMangled(name, found))
        return true;
    return findFunctionByPretty(name, found);
}

bool pdmodule::findFunctionByMangled( const pdstring &name,
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


bool pdmodule::findFunctionByPretty( const pdstring &name,
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

void pdmodule::dumpMangled(pdstring &prefix) const
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

#ifdef CHECK_ALL_CALL_POINTS
void image::checkAllCallPoints() {
    for (unsigned i = 0; i < _mods.size(); i++) {
        _mods[i]->checkAllCallPoints();
    }
}
#endif
pdmodule *image::getOrCreateModule(const pdstring &modName, 
				   const Address modAddr) {
    pdstring nameToUse;
    if (modName.length())
        nameToUse = modName;
    else
        nameToUse = "DEFAULT_MODULE";

    pdmodule *fm = findModule(nameToUse);
    if (fm) return fm;

    const char *str = nameToUse.c_str();
    int len = nameToUse.length();
    assert(len>0);

    // TODO ignore directory definitions for now
    if (str[len-1] == '/') 
        return NULL;
    return (newModule(nameToUse, modAddr, lang_Unknown));
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

image_func *image::findFuncByOffset(const Address &offset)  {
    codeRange *range = findCodeRangeByOffset(offset);
    image_func *func = range->is_image_func();

    return func;
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

const pdvector<image_func *> *image::findFuncVectorByPretty(const pdstring &name) {
    //Have to change here
    pdvector<image_func *>* res = new pdvector<image_func *>;
    vector<Dyn_Symbol *>syms;
    linkedFile->findSymbolByType(syms,name.c_str(),Dyn_Symbol::ST_FUNCTION);
    for(unsigned index=0; index<syms.size(); index++)
    {
    	if(syms[index]->getUpPtr())
	    res->push_back((image_func *)syms[index]->getUpPtr());
    }		
    if(res->size())	
	return res;	    
    else
    	return NULL;
    /*
    if (funcsByPretty.defines(name)) {
        //analyzeIfNeeded();
        return funcsByPretty[name];
    }
    
    return NULL;*/
}

// Return the vector of functions associated with a mangled name
// Very well might be more than one! -- multiple static functions in different .o files

const pdvector <image_func *> *image::findFuncVectorByMangled(const pdstring &name)
{

    //    fprintf(stderr,"findFuncVectorByMangled %s\n",name.c_str());
#ifdef IBM_BPATCH_COMPAT_STAB_DEBUG
  bperr( "%s[%d]:  inside findFuncVectorByMangled\n", FILE__, __LINE__);
#endif
    
    pdvector<image_func *>* res = new pdvector<image_func *>;
    vector<Dyn_Symbol *>syms;
    linkedFile->findSymbolByType(syms,name.c_str(),Dyn_Symbol::ST_FUNCTION,true);
    for(unsigned index=0; index<syms.size(); index++)
    {
    	if(syms[index]->getUpPtr())				//Every Dyn_Symbol might not have a corresponding image_func
    	    res->push_back((image_func *)syms[index]->getUpPtr());
    }	    
    if(res->size())	
	return res;	    
    else
    	return NULL;
   
  #if 0 
    if (funcsByMangled.defines(name)) {
      //analyzeIfNeeded();
      return funcsByMangled[name];
    }
  
    return NULL;
  #endif
}

const pdvector <image_variable *> *image::findVarVectorByPretty(const pdstring &name)
{
    //    fprintf(stderr,"findVariableVectorByPretty %s\n",name.c_str());
#ifdef IBM_BPATCH_COMPAT_STAB_DEBUG
  bperr( "%s[%d]:  inside findVariableVectorByPretty\n", FILE__, __LINE__);
#endif

    pdvector<image_variable *>* res = new pdvector<image_variable *>;
    vector<Dyn_Symbol *>syms;
    linkedFile->findSymbolByType(syms,name.c_str(),Dyn_Symbol::ST_OBJECT);
    for(unsigned index=0; index<syms.size(); index++)
    {
    	if(syms[index]->getUpPtr())
    	    res->push_back((image_variable *)syms[index]->getUpPtr());
    }	    
    if(res->size())	
	return res;	    
    else
    	return NULL;
  #if 0
  if (varsByPretty.defines(name)) {
      //analyzeIfNeeded();
      return varsByPretty[name];
  }
  return NULL;
  #endif
}

const pdvector <image_variable *> *image::findVarVectorByMangled(const pdstring &name)
{
    //    fprintf(stderr,"findVariableVectorByPretty %s\n",name.c_str());
#ifdef IBM_BPATCH_COMPAT_STAB_DEBUG
  bperr( "%s[%d]:  inside findVariableVectorByPretty\n", FILE__, __LINE__);
#endif
    pdvector<image_variable *>* res = new pdvector<image_variable *>;
    vector<Dyn_Symbol *>syms;
    linkedFile->findSymbolByType(syms,name.c_str(),Dyn_Symbol::ST_OBJECT,true);
    for(unsigned index=0; index<syms.size(); index++)
    {
    	if(syms[index]->getUpPtr())
    	    res->push_back((image_variable *)syms[index]->getUpPtr());
    }	    
    if(res->size())	
	return res;	    
    else
    	return NULL;
  /*
  if (varsByMangled.defines(name)) {
      //analyzeIfNeeded();
      return varsByMangled[name];
  }
  return NULL;*/
}


//  image::findOnlyOneFunction(const pdstring &name
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
      fprintf(stderr, "%s[%d]:  findOnlyOneFunction(%s), found more than one, failing\n",
              FILE__, __LINE__, name.c_str());
      return NULL;
    }
    //analyzeIfNeeded();
    return (*pdfv)[0];
  }

  pdfv = findFuncVectorByMangled(name);
  if (pdfv != NULL && pdfv->size() > 0) {
    if (pdfv->size() > 1) {
      fprintf(stderr, "%s[%d]:  findOnlyOneFunction(%s), found more than one, failing\n",
              FILE__, __LINE__, name.c_str());
      return NULL;
    }
    //analyzeIfNeeded();
    return (*pdfv)[0];
  }
  
  return NULL;
}

#if 0
#if !defined(os_windows)
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
  cerr << FILE__ << ":" << __LINE__ << ":  REGEXEC ERROR: "<< errbuf << endl;
  
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
  cerr << FILE__ << ":" << __LINE__ << ":  REGEXEC ERROR: "<< errbuf << endl;
  
  return -1;
}
#endif

#endif

#if 0		
pdvector<image_func *> *image::findFuncVectorByPretty(functionNameSieve_t bpsieve, 
						       void *user_data,
						       pdvector<image_func *> *found)
{
  pdvector<image_func *>::iterator iter = everyUniqueFunction.begin();
  while(iter!= everyUniqueFunction.end())
  {
  	if(*(bpsieve)(*iter->symTabName().c_str(), user_data))
		found->push_back(iter);
	iter++;	
  }
  if(found->size())
  	return found;
  return NULL;	
  
 #if 0 
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
    //analyzeIfNeeded();
    return found;
  }
  return NULL;
 #endif 
}

pdvector<image_func *> *image::findFuncVectorByMangled(functionNameSieve_t bpsieve, 
							void *user_data,
							pdvector<image_func *> *found)
{
  pdvector<image_func *>::iterator iter = everyUniqueVariable.begin();
  while(iter!= everyUniqueVariable.end())
  {
  	if(*(bpsieve)(*iter->symTabName().c_str(), user_data))
		found->push_back(iter);
	iter++;	
  }
  if(found->size())
  	return found;
  return NULL;	

 #if 0
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
    //analyzeIfNeeded();
    return found;
  }
  return NULL;
 #endif 
}
#endif

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


void * image::getPtrToDataInText( Address offset ) const {
	if( isData(offset) ) { return NULL; }
	if( ! isCode(offset) ) { return NULL; }
	
	offset -= codeOffset_;
	unsigned char * inst = (unsigned char *) (linkedFile->code_ptr());
	return (void *)(& inst[ offset ]);	
	} /* end getPtrToDataInText() */

void *image::getPtrToData(Address offset) const {
    if (!isData(offset)) return NULL;
    offset -= dataOffset_;
    unsigned char *inst = (unsigned char *)(linkedFile->data_ptr());
    return (void *)(&inst[offset]);
}
    
// return a pointer to the instruction at address adr
void *image::getPtrToInstruction(Address offset) const {
   assert(isValidAddress(offset));

   if (isCode(offset)) {
      offset -= codeOffset_;
      unsigned char *inst = (unsigned char *)(linkedFile->code_ptr());
      return (void *)(&inst[offset]);
   } else if (isData(offset)) {
      offset -= dataOffset_;
      unsigned char *inst = (unsigned char *)(linkedFile->data_ptr());
      return (void *)(&inst[offset]);
   } else {
      abort();
      return 0;
   }
}

// Address must be in code or data range since some code may end up
// in the data segment
bool image::isValidAddress(const Address &where) const{
	Address addr = where;
   return linkedFile->isValidOffset(addr) && isAligned(where);
}

bool image::isCode(const Address &where)  const{
   Address addr = where;
   return linkedFile->isCode(addr); 
}

bool image::isData(const Address &where)  const{
   Address addr = where;
   return linkedFile->isData(addr); 
}

bool image::symbol_info(const pdstring& symbol_name, Dyn_Symbol &ret_sym) {

   /* We temporarily adopt the position that an image has exactly one
      symbol per name.  While local functions (etc) make this untrue, it
      dramatically minimizes the amount of rewriting. */
   vector< Dyn_Symbol *> symbols;
   if(!(linkedFile->findSymbolByType(symbols,symbol_name.c_str(),Dyn_Symbol::ST_UNKNOWN)))
   	return false;
   if(symbols.size() == 1 ) {
       ret_sym = *(symbols[0]);
	   return true;
   }
   else if ( symbols.size() > 1 )
       return false;
   return false;
}


bool image::findSymByPrefix(const pdstring &prefix, pdvector<Dyn_Symbol *> &ret) {
    unsigned start;
    vector <Dyn_Symbol *>found;	
    pdstring reg = prefix+pdstring("*");
    if(!linkedFile->findSymbolByType(found, reg.c_str(), Dyn_Symbol::ST_UNKNOWN, false, true))
    	return false;
    for(start=0;start< found.size();start++)
		ret.push_back(found[start]);
	return true;	
}
