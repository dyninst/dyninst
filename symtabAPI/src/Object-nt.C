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
 * source or binary (including de rivatives), electronic or otherwise,
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

// $Id: Object-nt.C,v 1.3 2007/02/05 21:14:24 giri Exp $

#define WIN32_LEAN_AND_MEAN


#include <windows.h>
#include <cvconst.h>
#include <oleauto.h>

#include <iostream>
#include <iomanip>
#include <limits.h>
#include <crtdbg.h>
#include "Object.h"
#include "Object-nt.h"
/*#include "dyninstAPI/src/arch-x86.h"
#include "dyninstAPI/src/showerror.h"
#include "dyninstAPI/src/LineInformation.h"
#include "dyninstAPI/src/mapped_module.h"
#include "dyninstAPI/src/mapped_object.h"
#include "dyninstAPI/src/BPatch_collections.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/BPatch_typePrivate.h"
#include "dyninstAPI/src/instPoint.h"
#include "dyninstAPI/h/BPatch_module.h"
#include "dyninstAPI/h/BPatch_function.h"
*/

bool pd_debug_export_symbols = false;

//extern void printSysError(unsigned errNo);

//---------------------------------------------------------------------------
// prototypes of functions used in this file
//---------------------------------------------------------------------------
BOOL CALLBACK SymEnumSymbolsCallback( PSYMBOL_INFO pSymInfo,
										ULONG symSize,
										PVOID userContext );


/*
 * stripAtSuffix
 *
 * Strips off of a string any suffix that consists of an @ sign followed by
 * decimal digits.
 *
 * str	The string to strip the suffix from.  The string is altered in place.
 */
static void stripAtSuffix(char *str)
{
    // many symbols have a name like foo@4, we must remove the @4
    // just searching for an @ is not enough,
    // as it may occur on other positions. We search for the last one
    // and check that it is followed only by digits.
    char *p = strrchr(str, '@');
    if (p) {
      char *q = p+1;
      strtoul(p+1, &q, 10);
      if (q > p+1 && *q == '\0') {
	*p = '\0';
      }
    }
}

char *cplus_demangle(char *c, int, bool includeTypes) { 
    char buf[1000];
    if (c[0]=='_') {
       // VC++ 5.0 seems to decorate C symbols differently to C++ symbols
       // and the UnDecorateSymbolName() function provided by imagehlp.lib
       // doesn't manage (or want) to undecorate them, so it has to be done
       // manually, removing a leading underscore from functions & variables
       // and the trailing "$stuff" from variables (actually "$Sstuff")
       unsigned i;
       for (i=1; i<sizeof(buf) && c[i]!='$' && c[i]!='\0'; i++)
           buf[i-1]=c[i];
       buf[i-1]='\0';
       stripAtSuffix(buf);
       if (buf[0] == '\0') return 0; // avoid null names which seem to annoy Paradyn
       return strdup(buf);
    } else {
       if (includeTypes) {
          if (UnDecorateSymbolName(c, buf, 1000, UNDNAME_COMPLETE| UNDNAME_NO_ACCESS_SPECIFIERS|UNDNAME_NO_MEMBER_TYPE|UNDNAME_NO_MS_KEYWORDS)) {
            //   printf("Undecorate with types: %s = %s\n", c, buf);
            stripAtSuffix(buf);
            return strdup(buf);
          }
       }  else if (UnDecorateSymbolName(c, buf, 1000, UNDNAME_NAME_ONLY)) {
         //     else if (UnDecorateSymbolName(c, buf, 1000, UNDNAME_COMPLETE|UNDNAME_32_BIT_DECODE)) {
         //	printf("Undecorate: %s = %s\n", c, buf);
         stripAtSuffix(buf);          
         return strdup(buf);
       }
    }
    return 0;
}


//---------------------------------------------------------------------------
// Object method implementation
//---------------------------------------------------------------------------
struct  CompareSymAddresses: public binary_function<const Object::Symbol*, const Object::Symbol*, bool> 
{
    bool operator()(const Object::Symbol *s1, const Object::Symbol* s2) {
    	 bool ret = true;

    	// first try comparing by address
    	if( s1->GetAddr() < s2->GetAddr() ) 
    	{
    	    ret = true;
    	}
    	else if( s1->GetAddr() > s2->GetAddr() )
    	{
   	    ret = false;
   	}
    	else
    	{
            // the two symbols have the same address
            // use our next criteria (the existence of a size)
            // for a given address, we want symbols with a size
            // to occur before those without so that we can
            // use the size if we wish
            if( (s1->GetSize() != 0) && (s2->GetSize() == 0) )
            {
                ret = true;
            }
            else if( (s1->GetSize() == 0) && (s2->GetSize() != 0) )
            {
                ret = false;
            }
        }
        return ret;
    }
};

Object::Module::Module( string _name, DWORD64 _baseAddr, DWORD64 _extent )
  : name(_name),
    baseAddr(_baseAddr),
    extent(_extent),
    isDll( false )
{
	defFile = new Object::File();
	files.push_back( defFile );
}


Object::File*
Object::Module::FindFile( string name )
{
	File* ret = NULL;

	for( vector<File *>::iterator iter = files.begin();
			iter != files.end();
			iter++ )
	{
		if( (*iter)->GetName() == name )
		{
			ret = *iter;
			break;
		}
	}
	return ret;
}

void
Object::File::DefineSymbols( hash_map<string, vector< ::Dyn_Symbol *> >& allSyms,
                                const string& modName ) const
{
    for( vector<Object::Symbol*>::const_iterator iter = syms.begin();
        iter != syms.end();
        iter++ )
    {
        const Object::Symbol* curSym = * iter;
        assert( curSym != NULL );

        curSym->DefineSymbol( allSyms, modName );
    }
}

void
Object::Symbol::DefineSymbol(hash_map<string,vector<::Dyn_Symbol *> >&allSyms,
                                const string& modName ) const
{
    allSyms[GetName()].push_back(new ::Dyn_Symbol(GetName(), 
										  modName,
                                          (::Dyn_Symbol::SymbolType) GetType(),
                                          (::Dyn_Symbol::SymbolLinkage) GetLinkage(),
                                          (Address)GetAddr(),
                                          NULL,				// TODO there should be a section pointer here
                                          GetSize()) );
}

void
Object::Module::DefineSymbols( const Object* obj,
                                hash_map<string, vector< ::Dyn_Symbol *> > & syms ) const
{
    // define Paradyn/dyninst modules and symbols
    if( !isDll )
    {
        // this is an EXE
        for( vector<Object::File*>::const_iterator iter = files.begin();
            iter != files.end();
            iter++ )
        {
            const File* curFile = *iter;
            assert( curFile != NULL );

            //fprintf(stderr, "ObjMod::DefineSymbols for %s\n", curFile->GetName().c_str());
            // add a Symbol for the file
			syms[curFile->GetName()].push_back( new ::Dyn_Symbol( curFile->GetName(), 
				"",
                ::Dyn_Symbol::ST_MODULE,
                ::Dyn_Symbol::SL_GLOBAL,
                obj->code_off(),        // TODO use real base of symbols for file
                NULL, 0 ) );              // TODO Pass Section pointer also
										// TODO also pass size

            // add symbols for each of the file's symbols
            curFile->DefineSymbols( syms, curFile->GetName() );
        }
    }
    else
    {
        // we represent a DLL
        // add one Symbol for the entire module
		syms[name].push_back( new ::Dyn_Symbol( name,
            "",
            ::Dyn_Symbol::ST_MODULE,
            ::Dyn_Symbol::SL_GLOBAL,
            obj->code_off(),
            NULL,					//TODO pass Sections pointer
            obj->code_len()) );

        // add symbols for each of the module's symbols
        for( vector<Object::File*>::const_iterator iter = files.begin();
                iter != files.end();
                iter++ )
        {
            const File* curFile = *iter;
            assert( curFile != NULL );

            // add symbols for each of the file's symbols
            curFile->DefineSymbols( syms, name );
        }
    }
}

void
Object::Module::PatchSymbolSizes( const Object* obj,
								  const vector<Object::Symbol*>& allSyms ) const
{
    DWORD64 lastFuncAddr = NULL;
    unsigned int i;


    for( i = 0; i < allSyms.size(); i++ )
    {
		Object::Symbol* sym = allSyms[i];
		assert( sym != NULL );

      if( (sym->GetName() != "") && (sym->GetSize() == 0) &&
            ((sym->GetType() == ::Dyn_Symbol::ST_FUNCTION) ||
             (sym->GetType() == ::Dyn_Symbol::ST_OBJECT)))
        {
            // check for function aliases
            // note that this check depends on the allSymbols
            // array being sorted so that aliases are considered
            // after the "real" function symbol
            bool isAlias = false;
            if( (sym->GetType() == ::Dyn_Symbol::ST_FUNCTION) &&
                (sym->GetAddr() == lastFuncAddr) &&
                (sym->GetSize() == 0) )
            {
                // this function is an alias
                // we currently leave their size as zero to indicate 
                // that they are uninstrumentable.  Ideally, this will
                // change once a mechanism becomes available to identify
                // these as function aliases.
                isAlias = true;
            }


            if( !isAlias )
            {
                //
                // patch the symbol's size
                //
                // We consider the symbol's size to be the distance
                // to the next symbol.  (Sometimes this causes us to
                // overestimate, because compilers sometimes leave some
                // "padding" between the end of a function and the beginning
                // of the next.)
                //
                // Note that we have to use the next symbol whose
                // address is different from the current one, to handle
                // cases where aliases are included in the symbol table
                //
                DWORD64 cb;

                //
                // find next function or object symbol in our section with
                // an address different from ours
                //
                // the while test looks complicated -
                // all we're trying to do is skip to the next
                // function or object symbol within the array whose
                // address is not the same as allSymbols[i].
                unsigned int j = i + 1;
                while( (j < allSyms.size()) &&
					( ((allSyms[j]->GetType() != ::Dyn_Symbol::ST_FUNCTION) &&
					(allSyms[j]->GetType() != ::Dyn_Symbol::ST_OBJECT)
                         ) ||
                         (allSyms[j]->GetAddr() == sym->GetAddr())
                       )
                     )
                {
                   j++;
                }

                if( j < allSyms.size() &&
                    (allSyms[j]->GetType() == sym->GetType()) )
                {
                    // we found a symbol from the same section
                    // with a different address -
                    // size is just the delta between symbols
                    cb = allSyms[j]->GetAddr() - sym->GetAddr();
                }
                else
                {
                    // we couldn't find another symbol in our section
                    // with a different address -
                    // size is the remainder of the current section
                    if( sym->GetType() == ::Dyn_Symbol::ST_FUNCTION )
                    {
                        // size is remainder of the .text section
                        cb = (obj->code_off() + obj->code_len()) - 
							sym->GetAddr();
                    }
                    else
                    {
                        // size is remainder of the .data section
                        cb = (obj->data_off() + obj->data_len()) - 
							sym->GetAddr();
                    }
                }
                sym->SetSize( (unsigned int) cb );
            }

            // update the last known function symbol
            if( sym->GetType() == ::Dyn_Symbol::ST_FUNCTION )
            {
                lastFuncAddr = sym->GetAddr();
            }
        }
    }
}


void
Object::Module::BuildSymbolMap( const Object* obj ) const
{
	vector<Object::Symbol*> allSyms;

	// add all symbols to our allSyms vector
   	vector<Object::File*>::const_iterator iter = files.begin();
	for(;	iter != files.end();	iter++ )
	{
		assert( *iter != NULL );

		const vector<Object::Symbol*>& curSyms = (*iter)->GetSymbols();
		for( vector<Object::Symbol*>::const_iterator symIter = curSyms.begin();
				symIter != curSyms.end();
				symIter++ )
		{
			assert( *symIter != NULL );
			allSyms.push_back( *symIter );
		}
	}

	// sort the symbols by address
	sort( allSyms.begin(), allSyms.end(), CompareSymAddresses());

	for( unsigned int i = 1; i < allSyms.size(); i++ )
	{
		if( allSyms[i-1]->GetAddr() > allSyms[i]->GetAddr() )
		{
			cout << "WARNING - sort failed" << endl;
			assert( false );
		}
	}

	// patch up any symbol sizes which weren't given to us
	PatchSymbolSizes( obj, allSyms );

}



Object::~Object( void )
{
    if( mapAddr != NULL )
    {
        UnmapViewOfFile( mapAddr );
        CloseHandle( hMap );
    }
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

void Object::ParseGlobalSymbol(PSYMBOL_INFO pSymInfo)
{
   Object::Module* curMod = GetCurrentModule();
   assert( curMod != NULL );
   //HANDLE hProc = desc.procHandle();		//TODO hproc. Try to make a fake address space. hProc can be any
											//unique identifier

   // get this symbol's file and line information
   IMAGEHLP_LINE64 lineInfo;
   DWORD dwDisplacement = 0;
   ZeroMemory( &lineInfo, sizeof(lineInfo) );
   lineInfo.SizeOfStruct = sizeof(lineInfo);
   Object::File* pFile = NULL;
   if( SymGetLineFromAddr64( hProc,
                             pSymInfo->Address,
                             &dwDisplacement,
                             &lineInfo ) ) {
      // ensure we have a file for this object
      pFile = curMod->FindFile( lineInfo.FileName );
      if( pFile == NULL ) {
         pFile = new Object::File( lineInfo.FileName );
         curMod->AddFile( pFile );
      }
   }
   else {
      pFile = curMod->GetDefaultFile();
   }
   assert( pFile != NULL );
            
   // is it a function or not?
   // TODO why is there a discrepancy between code base addr for
   // EXEs and DLLs?
   DWORD symType = ::Dyn_Symbol::ST_UNKNOWN;
   DWORD symLinkage = ::Dyn_Symbol::SL_UNKNOWN;
   DWORD64 codeLen = code_len();
   DWORD64 codeBase = code_off();
   symType = ::Dyn_Symbol::ST_FUNCTION;
   //codeBase += get_base_addr();

   if ((pSymInfo->Flags & SYMFLAG_FUNCTION) ||
       (pSymInfo->Tag == SymTagFunction && !pSymInfo->Flags))
   {
      symLinkage = ::Dyn_Symbol::SL_UNKNOWN;
   }
   else if ((pSymInfo->Flags == SYMFLAG_EXPORT && 
            isText((Address) pSymInfo->Address - baseAddr)) ||
            !strcmp(pSymInfo->Name, "_loadsnstores"))
   {
      symType = ::Dyn_Symbol::ST_FUNCTION;
      symLinkage = ::Dyn_Symbol::SL_UNKNOWN;
   }
   else
   {
      symType = ::Dyn_Symbol::ST_OBJECT;
      symLinkage = ::Dyn_Symbol::SL_GLOBAL;
   }

   // register the symbol
  Address baseAddr = 0;
  //  if (desc.isSharedObject())
  //if(curModule->IsDll())
  //   baseAddr = get_base_addr();

   if( !isForwarded( ((Address) pSymInfo->Address) - baseAddr ) )
   {
      pFile->AddSymbol( new Object::Symbol( pSymInfo->Name,
                                            pSymInfo->Address - get_base_addr(),
                                            symType,
                                            symLinkage,
                                            pSymInfo->Size ) );
   } 
}

BOOL CALLBACK SymEnumSymbolsCallback( PSYMBOL_INFO pSymInfo,
                                      ULONG symSize,
                                      PVOID userContext )
{
   assert( pSymInfo != NULL );

   Object* obj = (Object*) userContext;
   assert( obj != NULL );

#if 0
   fprintf(stderr, "symEnumSymsCallback, %s, Flags:0x%x, Tag:0x%x, Type:%d, Addr:0x%x...\n",
           pSymInfo->Name,
           pSymInfo->Flags,
           pSymInfo->Tag,
           pSymInfo->TypeIndex,
           pSymInfo->Address);
#endif

   if (isGlobalSymbol(pSymInfo))
   {
      obj->ParseGlobalSymbol(pSymInfo);
   }
   else if ((pSymInfo->Flags & SYMFLAG_LOCAL) ||
            (pSymInfo->Flags & SYMFLAG_PARAMETER)) {
      //parsing_printf("Is a local variable\n");
      //obj->ParseLocalSymbol(pSymInfo);
   }
   else {
      //parsing_printf(" skipping\n");
   }
    
   // keep enumerating symbols
   return TRUE;
}

/**
 * The name here is a little desceptive.  We're not parsing debug info
 * as in line or type information.  This function finds all the global symbols
 * in a module and all of the source files. (i.e. so this function would find
 * the 'main' symbol and find the starting point of 'foo.c'
 **/
void Object::ParseDebugInfo( void )
{
    // build a Module object for the current module (EXE or DLL)
    // Note that the CurrentModuleScoper object ensures that the
    // curModule member will be reset when we leave the scope of
    // this function.
    // curModule = new Object::Module( file_, desc.code() );
	curModule = new Object::Module( file_, 0 );
	static unsigned count = 1;
    hProc = (HANDLE) count++;
	if(!SymInitialize(hProc, NULL, false)){
        DWORD dwErr = GetLastError();
        if(dwErr) {
			fprintf( stderr, "SymInitialize failed for %s\n",
                     ((file_.length() > 0) ? file_.c_str() 
                      : "<no name available>"));
            goto done;
        }
    }

	
    assert( hProc != NULL );
    assert( hProc != INVALID_HANDLE_VALUE );

    // find the sections we need to know about (.text and .data)
    FindInterestingSections();

    // load symbols for this module
    //DWORD64 dw64BaseAddr = (DWORD64)desc.loadAddr();
	DWORD64 dw64BaseAddr = (DWORD64)mapAddr;		//load address is always same(fake address space)

    DWORD64 loadRet = SymLoadModule64( hProc,          // proc handle
                                       hFile,          // file handle
                                       NULL,           // image name
                                       NULL,           // shortcut name
                                       dw64BaseAddr,   // load address
                                       0 );            // size of DLL    
    if(!loadRet) {
        DWORD dwErr = GetLastError();
        if(dwErr) {
            fprintf( stderr, "SymLoadModule64 failed for %s\n",
                     ((file_.length() > 0) ? file_.c_str() 
                      : "<no name available>"));
            //printSysError(dwErr);
            goto done;
        }
    }

    // parse symbols for the module
    if( !SymEnumSymbols(hProc,                     // process handle
                        dw64BaseAddr,               // load address
                        "",                     // symbol mask (we use none)
                        SymEnumSymbolsCallback, // called for each symbol
                        this ) )                // client data
    {
       int lasterr = GetLastError();
       fprintf( stderr, "Failed to enumerate symbols\n");
       //printSysError(lasterr);
    }
    
    // We have a module object, with one or more files,
    // each with one or more symbols.  However, the symbols
    // are not necessarily in order, nor do they necessarily have valid sizes.
    assert( curModule != NULL );
    curModule->BuildSymbolMap( this );
    curModule->DefineSymbols( this, symbols_ );
	no_of_symbols_ = symbols_.size();
 
    // Since PE-COFF is very similar to COFF (in that it's not like ELF),
    // the .text and .data sections are perfectly mapped to code/data segments

    code_vldS_ = code_off_;
    code_vldE_ = code_off_ + code_len_;
    data_vldS_ = data_off_;
    data_vldE_ = data_off_ + data_len_;
 done:
    delete curModule;
}


void
Object::FindInterestingSections()
{
    // map the file to our address space
    // first, create a file mapping object
   hMap = CreateFileMapping( hFile,
                             NULL,           // security attrs
                             PAGE_READONLY,  // protection flags
                             0,              // max size - high DWORD
                             0,              // max size - low DWORD
                             NULL );         // mapping name - not used
   if( hMap == NULL )
   {
      // TODO how to handle the error?
      fprintf( stderr, "CreateFileMapping failed: %x\n", GetLastError() );
	     LPVOID lpMsgBuf;
  FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM
|     FORMAT_MESSAGE_IGNORE_INSERTS,    NULL,
  GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)
&lpMsgBuf,    0,    NULL );

  cout << (LPCTSTR)lpMsgBuf << endl;
  LocalFree( lpMsgBuf );
      return;
   }  
   // next, map the file to our address space
   mapAddr = MapViewOfFileEx( hMap,             // mapping object
                              FILE_MAP_READ,  // desired access
                              0,              // loc to map - hi DWORD
                              0,              // loc to map - lo DWORD
                              0,              // #bytes to map - 0=all
                              NULL );         // suggested map addr
   if( mapAddr == NULL )
   {
      // TODO how to handle the error?
      fprintf( stderr, "MapViewOfFileEx failed: %x\n", GetLastError() );
      CloseHandle( hMap );
      hMap = INVALID_HANDLE_VALUE;
      return;
   }
   
   // now that we have the file mapped, look for 
   // the .text and .data sections
   assert( peHdr == NULL );
   peHdr = ImageNtHeader( mapAddr );
   
	assert( peHdr->FileHeader.SizeOfOptionalHeader > 0 ); 
   
   assert( curModule != NULL );
   curModule->SetIsDll( peHdr->FileHeader.Characteristics & IMAGE_FILE_DLL );
   if(curModule->IsDll())
	   is_aout_ = false;
   else
	   is_aout_ = true;

   unsigned int nSections = peHdr->FileHeader.NumberOfSections;
   no_of_sections_ = nSections;
   PIMAGE_SECTION_HEADER pScnHdr = (PIMAGE_SECTION_HEADER)(((char*)peHdr) + 
                                sizeof(DWORD) +         // for signature
                                sizeof(IMAGE_FILE_HEADER) +
                                peHdr->FileHeader.SizeOfOptionalHeader);
 		
   for( unsigned int i = 0; i < nSections; i++ )
   {
      //TODO Insert into sections_ here.
      if( strncmp( (const char*)pScnHdr->Name, ".text", 5 ) == 0 )
      {
         // note that section numbers are one-based
         textSectionId = i + 1;
         code_ptr_    = (Word*)(((char*)mapAddr) +
                                pScnHdr->PointerToRawData);
         //if (GetDescriptor().isSharedObject())
		 //if (curModule->IsDll())
            code_off_    = pScnHdr->VirtualAddress;
         //else
         //   code_off_    = get_base_addr() + pScnHdr->VirtualAddress;	//loadAddr = mapAddr
		 //code_off_    = pScnHdr->VirtualAddress + desc.loadAddr();
         code_len_    = pScnHdr->Misc.VirtualSize;
      }
      else if( strncmp( (const char*)pScnHdr->Name, ".data", 5 ) == 0 )
      {
         // note that section numbers are one-based
         dataSectionId = i + 1;
         
         data_ptr_    = (Word*)(((char*)mapAddr) +
                                pScnHdr->PointerToRawData);
         //if (GetDescriptor().isSharedObject())
		 //if (curModule->IsDll())
            data_off_    = pScnHdr->VirtualAddress;
         //else
            //data_off_    = desc.loadAddr() + pScnHdr->VirtualAddress;
		 //	data_off_    = get_base_addr()+pScnHdr->VirtualAddress;
         data_len_    = pScnHdr->Misc.VirtualSize;
      }
      
      pScnHdr += 1;
   }
   }


bool Object::isForwarded( OFFSET addr )
{
	//calls to forwarded symbols are routed to another dll and 
    //are not in the current dll's code space
	//we MUST NOT try to parse these - bad things happen
	
	//we detect forwarded symbols by checking if the relative 
    //virtual address of the symbol falls within the dll's exports section
	if( peHdr->FileHeader.Characteristics & IMAGE_FILE_DLL )
	{
		PIMAGE_DATA_DIRECTORY dataDir = peHdr->OptionalHeader.DataDirectory;
	    	OFFSET exportStart = dataDir->VirtualAddress;
		OFFSET exportEnd = exportStart + dataDir->Size;

		if( addr >= exportStart && addr < exportEnd )
			return true;  //this sym is forwarded
	}

	return false;
}

bool Object::getCatchBlock(ExceptionBlock &b, Address addr, 
                           unsigned size) const 
{ 
   return false; 
}

bool Object::isText( const Address& addr ) const 
{
   return( addr >= code_off_ && addr <= code_len_ );
}

Object::Object(string &filename,
                void (*err_func)(const char *)) 
    : AObject(filename, err_func),
     //baseAddr(_desc.code()),
     hMap( INVALID_HANDLE_VALUE ),
     mapAddr( NULL ),
     curModule( NULL ),
     peHdr( NULL )
{
	if (strcmp(filename.c_str(), "ntdll.dll") == 0)
		filename = "c:\\windows\\system32\\ntdll.dll";
	if (filename.substr(0,23) == "\\Device\\HarddiskVolume1")
		filename = "c:"+filename.substr(23,filename.size());
	hFile = CreateFile(filename.c_str(), GENERIC_READ, FILE_SHARE_READ, 
    				NULL,OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    assert( hFile != NULL );
    assert( hFile != INVALID_HANDLE_VALUE );
    ParseDebugInfo();
}
