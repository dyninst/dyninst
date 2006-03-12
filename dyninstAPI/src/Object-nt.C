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

// $Id: Object-nt.C,v 1.39 2006/03/12 23:31:25 legendre Exp $

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <dbghelp.h>
#include <cvconst.h>
#include <oleauto.h>

#include <iostream>
#include <iomanip>
#include <limits.h>

#include "common/h/String.h"
#include "common/h/Vector.h"
#include "dyninstAPI/src/Object.h"
#include "dyninstAPI/src/Object-nt.h"
#include "dyninstAPI/src/arch-x86.h"
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

#include <crtdbg.h>

bool pd_debug_export_symbols = false;

extern void printSysError(unsigned errNo);
BPatch_type *getType(HANDLE p, Address mod_base, int typeIndex, BPatch_module *mod = NULL);

//---------------------------------------------------------------------------
// prototypes of functions used in this file
//---------------------------------------------------------------------------
int CompareSymAddresses( const void *x, const void *y );
BOOL CALLBACK SymEnumSymbolsCallback( PSYMBOL_INFO pSymInfo,
										ULONG symSize,
										PVOID userContext );



//---------------------------------------------------------------------------
// Object method implementation
//---------------------------------------------------------------------------
int
CompareSymAddresses( const void *x, const void *y )
{
	const Object::Symbol* s1 = *(const Object::Symbol**)x;
    const Object::Symbol* s2 = *(const Object::Symbol**)y;
    int ret = 0;


    // first try comparing by address
    if( s1->GetAddr() < s2->GetAddr() ) 
    {
        ret = -1;
    }
    else if( s1->GetAddr() > s2->GetAddr() )
    {
        ret = 1;
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
            ret = -1;
        }
        else if( (s1->GetSize() == 0) && (s2->GetSize() != 0) )
        {
            ret = 1;
        }
    }
    return ret;
}


Object::Module::Module( pdstring _name, DWORD64 _baseAddr, DWORD64 _extent )
  : name(_name),
    baseAddr(_baseAddr),
    extent(_extent),
    isDll( false )
{
	defFile = new Object::File();
	files.push_back( defFile );
}


Object::File*
Object::Module::FindFile( pdstring name )
{
	File* ret = NULL;

	for( pdvector<File*>::iterator iter = files.begin();
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
Object::File::DefineSymbols( dictionary_hash<pdstring, pdvector< ::Symbol > >& allSyms,
                                const pdstring& modName ) const
{
    for( pdvector<Object::Symbol*>::const_iterator iter = syms.begin();
        iter != syms.end();
        iter++ )
    {
        const Object::Symbol* curSym = * iter;
        assert( curSym != NULL );

        curSym->DefineSymbol( allSyms, modName );
    }
}


void
Object::Symbol::DefineSymbol(dictionary_hash<pdstring,pdvector<::Symbol> >&allSyms,
                                const pdstring& modName ) const
{
    allSyms[GetName()].push_back(::Symbol(GetName(),
                                          modName,
                                          (::Symbol::SymbolType) GetType(),
                                          (::Symbol::SymbolLinkage) GetLinkage(),
                                          (Address)GetAddr(),
                                          false,
                                          GetSize()) );
}



void
Object::Module::DefineSymbols( const Object* obj,
                                dictionary_hash<pdstring, pdvector< ::Symbol > > & syms ) const
{
    // define Paradyn/dyninst modules and symbols
    if( !isDll )
    {
        // this is an EXE
        for( pdvector<const Object::File*>::const_iterator iter = files.begin();
            iter != files.end();
            iter++ )
        {
            const File* curFile = *iter;
            assert( curFile != NULL );

            //fprintf(stderr, "ObjMod::DefineSymbols for %s\n", curFile->GetName().c_str());
            // add a Symbol for the file
            syms[curFile->GetName()].push_back( ::Symbol( curFile->GetName(),
                "",
                ::Symbol::PDST_MODULE,
                ::Symbol::SL_GLOBAL,
                obj->code_off(),        // TODO use real base of symbols for file
                false ) );                // TODO also pass size

            // add symbols for each of the file's symbols
            curFile->DefineSymbols( syms, curFile->GetName() );
        }
    }
    else
    {
        // we represent a DLL
        // add one Symbol for the entire module
        syms[name].push_back( ::Symbol( name,
            "",
            ::Symbol::PDST_MODULE,
            ::Symbol::SL_GLOBAL,
            obj->code_off(),
            false,
            obj->code_len()) );

        // add symbols for each of the module's symbols
        for( pdvector<Object::File*>::const_iterator iter = files.begin();
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
								  const pdvector<Object::Symbol*>& allSyms ) const
{
    DWORD64 lastFuncAddr = NULL;
    unsigned int i;


    for( i = 0; i < allSyms.size(); i++ )
    {
		Object::Symbol* sym = allSyms[i];
		assert( sym != NULL );

      if( (sym->GetName() != "") && (sym->GetSize() == 0) &&
            ((sym->GetType() == ::Symbol::PDST_FUNCTION) ||
             (sym->GetType() == ::Symbol::PDST_OBJECT)))
        {
            // check for function aliases
            // note that this check depends on the allSymbols
            // array being sorted so that aliases are considered
            // after the "real" function symbol
            bool isAlias = false;
            if( (sym->GetType() == ::Symbol::PDST_FUNCTION) &&
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
					( ((allSyms[j]->GetType() != ::Symbol::PDST_FUNCTION) &&
					(allSyms[j]->GetType() != ::Symbol::PDST_OBJECT)
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
                    if( sym->GetType() == ::Symbol::PDST_FUNCTION )
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
            if( sym->GetType() == ::Symbol::PDST_FUNCTION )
            {
                lastFuncAddr = sym->GetAddr();
            }
        }
    }
}


void
Object::Module::BuildSymbolMap( const Object* obj ) const
{
	pdvector<Object::Symbol*> allSyms;

	// add all symbols to our allSyms vector
   pdvector<Object::File*>::const_iterator iter = files.begin();
	for(;	iter != files.end();	iter++ )
	{
		assert( *iter != NULL );

		const pdvector<Object::Symbol*>& curSyms = (*iter)->GetSymbols();
		for( pdvector<Object::Symbol*>::const_iterator symIter = curSyms.begin();
				symIter != curSyms.end();
				symIter++ )
		{
			assert( *symIter != NULL );
			allSyms.push_back( *symIter );
		}
	}

	// sort the symbols by address
	VECTOR_SORT( allSyms, CompareSymAddresses );

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
   const fileDescriptor &desc = GetDescriptor();
   HANDLE hProc = desc.procHandle();

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
   DWORD symType = ::Symbol::PDST_UNKNOWN;
   DWORD symLinkage = ::Symbol::SL_UNKNOWN;
   DWORD64 codeLen = code_len();
   DWORD64 codeBase = code_off();
   symType = ::Symbol::PDST_FUNCTION;
   codeBase += get_base_addr();

   if ((pSymInfo->Flags & SYMFLAG_FUNCTION) ||
       (pSymInfo->Tag == SymTagFunction && !pSymInfo->Flags))
   {
      symLinkage = ::Symbol::SL_UNKNOWN;
   }
   else if ((pSymInfo->Flags == SYMFLAG_EXPORT && 
            isText((Address) pSymInfo->Address - baseAddr)) ||
            !strcmp(pSymInfo->Name, "_loadsnstores"))
   {
      symType = ::Symbol::PDST_FUNCTION;
      symLinkage = ::Symbol::SL_UNKNOWN;
   }
   else
   {
      symType = ::Symbol::PDST_OBJECT;
      symLinkage = ::Symbol::SL_GLOBAL;
   }

   // register the symbol
  Address baseAddr = 0;
  if (desc.isSharedObject())
     baseAddr = get_base_addr();

   if( !isForwarded( ((Address) pSymInfo->Address) - baseAddr ) )
   {
      pFile->AddSymbol( new Object::Symbol( pSymInfo->Name,
                                            pSymInfo->Address - baseAddr,
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
      parsing_printf("Is a local variable\n");
      //obj->ParseLocalSymbol(pSymInfo);
   }
   else {
      parsing_printf(" skipping\n");
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
    curModule = new Object::Module( desc.file(), desc.code() );

    HANDLE hProc = desc.procHandle();
    HANDLE hFile = desc.fileHandle();

    assert( hProc != NULL );
    assert( hProc != INVALID_HANDLE_VALUE );
    assert( hFile != NULL );
    assert( hFile != INVALID_HANDLE_VALUE );

    // find the sections we need to know about (.text and .data)
    FindInterestingSections( hProc, hFile );

    // load symbols for this module
    DWORD64 dw64BaseAddr = (DWORD64)desc.loadAddr();

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
            printSysError(dwErr);
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
       printSysError(lasterr);
    }
    
    // We have a module object, with one or more files,
    // each with one or more symbols.  However, the symbols
    // are not necessarily in order, nor do they necessarily have valid sizes.
    assert( curModule != NULL );
    curModule->BuildSymbolMap( this );
    curModule->DefineSymbols( this, symbols_ );

    if( !( peHdr->FileHeader.Characteristics & IMAGE_FILE_DLL ) )
    {
        const unsigned char *t, *p;
        Address eAddr = peHdr->OptionalHeader.AddressOfEntryPoint;

        bool found_main = false;
        for (unsigned i=0; i<NUMBER_OF_MAIN_POSSIBILITIES; i++) {
            if (symbols_.defines(main_function_names[i])) {
                found_main = true;
                break;
            }
        }

        if (!found_main)
        {
            Address curr = eAddr;
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
            if (!symbols_.defines("DEFAULT_MODULE")) {
                //make up a symbol for default module too
                symbols_["DEFAULT_MODULE"].push_back( ::Symbol( "DEFAULT_MODULE",
                                                     "DEFAULT_MODULE",
                                                    ::Symbol::PDST_MODULE,
                                                    ::Symbol::SL_GLOBAL,
                                                    code_off(),
                                                    0,
                                                    0) );
            }            
            if (!symbols_.defines("start")) {
                //use 'start' for mainCRTStartup.
                ::Symbol startSym( "start", "DEFAULT_MODULE", ::Symbol::PDST_FUNCTION,
                                ::Symbol::SL_GLOBAL, eAddr, 0, UINT_MAX );
                symbols_[ "start" ].push_back( startSym );
            }
            if (!symbols_.defines("winStart")) {
                //make up a func name for the start of the text section
                ::Symbol sSym( "winStart", "DEFAULT_MODULE",::Symbol::PDST_FUNCTION,
                            ::Symbol::SL_GLOBAL, code_off(), 0, UINT_MAX );
                symbols_[ "winStart" ].push_back( sSym );
            }
            if (!symbols_.defines("winFini")) {
                //make up one for the end of the text section
                ::Symbol fSym( "winFini", "DEFAULT_MODULE",::Symbol::PDST_FUNCTION,
                            ::Symbol::SL_GLOBAL, code_off() + code_len_ - 1, 
                            0, UINT_MAX );
                symbols_[ "winFini" ].push_back( fSym );
            }
        }
    }
    
 
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
Object::FindInterestingSections( HANDLE hProc, HANDLE hFile )
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
   
   unsigned int nSections = peHdr->FileHeader.NumberOfSections;
   PIMAGE_SECTION_HEADER pScnHdr = (PIMAGE_SECTION_HEADER)(((char*)peHdr) + 
                                sizeof(DWORD) +         // for signature
                                sizeof(IMAGE_FILE_HEADER) +
                                peHdr->FileHeader.SizeOfOptionalHeader);
 		
   for( unsigned int i = 0; i < nSections; i++ )
   {
      if( strncmp( (const char*)pScnHdr->Name, ".text", 5 ) == 0 )
      {
         // note that section numbers are one-based
         textSectionId = i + 1;
         
         code_ptr_    = (Word*)(((char*)mapAddr) +
                                pScnHdr->PointerToRawData);
         if (GetDescriptor().isSharedObject())
            code_off_    = pScnHdr->VirtualAddress;
         else
            code_off_    = desc.loadAddr() + pScnHdr->VirtualAddress;
         code_len_    = pScnHdr->Misc.VirtualSize;
      }
      else if( strncmp( (const char*)pScnHdr->Name, ".data", 5 ) == 0 )
      {
         // note that section numbers are one-based
         dataSectionId = i + 1;
         
         data_ptr_    = (Word*)(((char*)mapAddr) +
                                pScnHdr->PointerToRawData);
         if (GetDescriptor().isSharedObject())
            data_off_    = pScnHdr->VirtualAddress;
         else
            data_off_    = desc.loadAddr() + pScnHdr->VirtualAddress;
         data_len_    = pScnHdr->Misc.VirtualSize;
      }
      
      pScnHdr += 1;
   }
   }


bool Object::isForwarded( Address addr )
{
	//calls to forwarded symbols are routed to another dll and 
    //are not in the current dll's code space
	//we MUST NOT try to parse these - bad things happen
	
	//we detect forwarded symbols by checking if the relative 
    //virtual address of the symbol falls within the dll's exports section
	if( peHdr->FileHeader.Characteristics & IMAGE_FILE_DLL )
	{
		PIMAGE_DATA_DIRECTORY dataDir = peHdr->OptionalHeader.DataDirectory;
	    Address exportStart = dataDir->VirtualAddress;
		Address exportEnd = exportStart + dataDir->Size;

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

typedef struct localsStruct {
    BPatch_function *func;
    Address base;
    HANDLE p;
    dictionary_hash<unsigned, unsigned> foundSyms;
    localsStruct() : foundSyms(uiHash) {}
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
    if (locals->foundSyms.defines(pSymInfo->Index))
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

//We want to pass both the module and lineInformation object through
// the (void *) param parameter of SymEnumLinesProc.  This struct
// helps with that.
struct mod_linfo_pair {
   mapped_module *mod;
   LineInformation *li;
};

static SRCCODEINFO *last_srcinfo;
BOOL CALLBACK add_line_info(SRCCODEINFO *srcinfo, void *param)
{
   struct mod_linfo_pair *pair = (struct mod_linfo_pair *) param;
   mapped_module *mod = pair->mod;
   LineInformation *li = pair->li;
   
   if (last_srcinfo && srcinfo) {
      //All the middle iterations.  Use the previous line information with the 
      // current line info to build LineInformation structure.
      assert(last_srcinfo->Address <= srcinfo->Address);
      li->addLine(last_srcinfo->FileName, last_srcinfo->LineNumber, 
                  (Address) last_srcinfo->Address, (Address) srcinfo->Address);
      return TRUE;
   }
   else if (!last_srcinfo && srcinfo) {
      //First iteration.  Don't add anything until the next
      last_srcinfo = srcinfo;
      return TRUE;
   }
   else if (last_srcinfo && !srcinfo) {
      //Last iteration.  Add current srcinfo up to end of module.
      li->addLine(last_srcinfo->FileName, last_srcinfo->LineNumber, 
                  (Address) last_srcinfo->Address, (Address) last_srcinfo->Address);
      return TRUE;
   }
   return TRUE;
}

static int variantValue(VARIANT *v) {
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
static void addTypeToCollection(BPatch_type *type, BPatch_module *mod) {
   BPatch_typeCollection *collection;
   collection = mod ? mod->getModuleTypes() : BPatch::bpatch->stdTypes;
   assert(collection);
   assert(!collection->findType(type->getID()));
   collection->addType(type);
}

static char *getTypeName(HANDLE p, Address base, int typeIndex) {
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
        printSysError(lasterror);
        return NULL;
    }
    return name;
}

static BPatch_dataClass getDataClass(HANDLE p, Address base, int typeIndex) {
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

static BPatch_type *getEnumType(HANDLE p, Address base, int typeIndex, BPatch_module *mod) {
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

static BPatch_type *getArrayType(HANDLE p, Address base, int typeIndex, BPatch_module *mod) {
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
    pdvector<BPatch_type *> params;
    TI_FINDCHILDREN_PARAMS *args = NULL;
    pdstring name;
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

void BPatch_module::parseTypes() {
    proc_mod_pair pair;
    BOOL result;
    //
    //Parse global variable type information
    //

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
}

void mapped_module::parseFileLineInfo()
{   
   bool result;
   static Address last_file = 0x0;

   struct mod_linfo_pair pair;
   pair.li = &lineInfo_;
   pair.mod = this;

   const char *src_file_name = NULL;
   const char *libname = NULL;

   if (obj()->parse_img()->isAOut()) 
      src_file_name = fileName().c_str();
   else 
      libname = fileName().c_str();

   Address baseAddr = obj()->getBaseAddress();
   if (last_file == baseAddr)
	   return;
   last_file = baseAddr;

   last_srcinfo = NULL;
   result = SymEnumSourceLines(proc()->processHandle_, 
                               baseAddr,
                               libname, 
                               src_file_name,
                               0,
                               0,
                               add_line_info, 
                               &pair); 
   if (!result) {
      //Not a big deal. The module probably didn't have any debug information.
      DWORD dwErr = GetLastError();
      parsing_printf("[%s:%u] - Couldn't SymEnumLines on %s in %s\n", 
              __FILE__, __LINE__, src_file_name, libname);
	  return;
   }
   add_line_info(NULL, &pair);
}
