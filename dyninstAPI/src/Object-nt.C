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

// $Id: Object-nt.C,v 1.27 2004/04/14 21:40:44 pcroth Exp $

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <dbghelp.h>

#include <iostream>
#include <iomanip>
#include <limits.h>

#include "common/h/String.h"
#include "common/h/Vector.h"
#include "dyninstAPI/src/CodeView.h"
#include "dyninstAPI/src/Object.h"
#include "dyninstAPI/src/Object-nt.h"


bool pd_debug_export_symbols = false;

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
    isDll( false ),
	defFile( new Object::File )
{
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
Object::File::DefineSymbols( dictionary_hash<pdstring,::Symbol>& allSyms,
                                const pdstring& modName ) const
{
    for( pdvector<Object::Symbol*>::const_iterator iter = syms.begin();
        iter != syms.end();
        iter++ )
    {
        const Object::Symbol* curSym = *iter;
        assert( curSym != NULL );

        curSym->DefineSymbol( allSyms, modName );
    }
}


void
Object::Symbol::DefineSymbol( dictionary_hash<pdstring,::Symbol>& allSyms,
                                const pdstring& modName ) const
{
    allSyms[GetName()] = ::Symbol( GetName(),
        modName,
        (::Symbol::SymbolType)GetType(),
		(::Symbol::SymbolLinkage)GetLinkage(),
        (Address)GetAddr(),
        false,
		GetSize());
}



void
Object::Module::DefineSymbols( const Object* obj,
                                dictionary_hash<pdstring,::Symbol>& syms ) const
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

            // add a Symbol for the file
            syms[curFile->GetName()] = ::Symbol( curFile->GetName(),
                "",
                ::Symbol::PDST_MODULE,
                ::Symbol::SL_GLOBAL,
                obj->code_off(),        // TODO use real base of symbols for file
                false );                // TODO also pass size

            // add symbols for each of the file's symbols
            curFile->DefineSymbols( syms, curFile->GetName() );
        }
    }
    else
    {
        // we represent a DLL
        // add one Symbol for the entire module
        syms[name] = ::Symbol( name,
            "",
            ::Symbol::PDST_MODULE,
            ::Symbol::SL_GLOBAL,
            obj->code_off(),
            false,
            obj->code_len() * sizeof(Word) );

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
    Address lastFuncAddr = NULL;
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
                unsigned int cb;

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
                        cb = (obj->code_off() + obj->code_len()*sizeof(Word)) - 
							sym->GetAddr();
                    }
                    else
                    {
                        // size is remainder of the .data section
                        cb = (obj->data_off() + obj->data_len()*sizeof(Word)) - 
							sym->GetAddr();
                    }
                }
                sym->SetSize( cb );
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
	for( pdvector<Object::File*>::const_iterator iter = files.begin();
		iter != files.end();
		iter++ )
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

#if READY
	// check whether each file's symbols are contiguous
	// set each file's size
#endif // READY
}



Object::~Object( void )
{
    if( mapAddr != NULL )
    {
        UnmapViewOfFile( mapAddr );
        CloseHandle( hMap );
    }
}



BOOL
CALLBACK
SymEnumSymbolsCallback( PSYMBOL_INFO pSymInfo,
                    ULONG symSize,
                    PVOID userContext )
{
	assert( pSymInfo != NULL );

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
	// The values for pSymInfo->Tag are not currently documented in the PSDK.
	// They are defined in the 'cvconst.h' header from the DIA SDK.
	//
	if( (pSymInfo->Flags & SYMFLAG_EXPORT) ||
		(pSymInfo->Flags & SYMFLAG_FUNCTION) ||
		(pSymInfo->Flags & SYMFLAG_PARAMETER) ||
		(pSymInfo->Flags & SYMFLAG_LOCAL) ||
		((pSymInfo->Flags == 0) && 
			((pSymInfo->Tag == 0x05) ||			// SymTagFunction
			 (pSymInfo->Tag == 0x07) ||			// SymTagData
			 (pSymInfo->Tag == 0x0a) ||			// SymTagPublicSymbol
			 (pSymInfo->Tag == 0x3808))) )		// Seen with NB11, VC++6-produced executables
	{
        Object* obj = (Object*)userContext;
        assert( obj != NULL );

		Object::Module* curMod = obj->GetCurrentModule();
		assert( curMod != NULL );

		// get this symbol's file and line information
        const fileDescriptor_Win* desc = obj->GetDescriptor();
        assert( desc != NULL );
        HANDLE hProc = desc->GetProcessHandle();

        IMAGEHLP_LINE64 lineInfo;
		DWORD dwDisplacement = 0;
        ZeroMemory( &lineInfo, sizeof(lineInfo) );
        lineInfo.SizeOfStruct = sizeof(lineInfo);
		Object::File* pFile = NULL;
        if( SymGetLineFromAddr64( hProc,
									pSymInfo->Address,
									&dwDisplacement,
									&lineInfo ) )
        {
			// ensure we have a file for this object
			pFile = curMod->FindFile( lineInfo.FileName );
			if( pFile == NULL )
			{
				pFile = new Object::File( lineInfo.FileName );
				curMod->AddFile( pFile );
			}
        }
        else
        {
			pFile = curMod->GetDefaultFile();
        }
		assert( pFile != NULL );

		// is it a function or not?
        // TODO why is there a discrepancy between code base addr for
        // EXEs and DLLs?
		DWORD symType = ::Symbol::PDST_UNKNOWN;
		DWORD symLinkage = ::Symbol::SL_UNKNOWN;
        DWORD64 codeLen = obj->code_len() * sizeof(Word);
        DWORD64 codeBase = obj->code_off();
        if( obj->GetDescriptor()->isSharedObject() )
        {
            codeBase += obj->get_base_addr();
        }

		if( (pSymInfo->Address >= codeBase) &&
			(pSymInfo->Address < (codeBase + codeLen)) &&
			!(pSymInfo->Flags & SYMFLAG_LOCAL) &&
			!(pSymInfo->Flags & SYMFLAG_PARAMETER) )
		{
			symType = ::Symbol::PDST_FUNCTION;
			symLinkage = ::Symbol::SL_UNKNOWN;
		}
		else
		{
			symType = ::Symbol::PDST_OBJECT;
			if( (pSymInfo->Flags & SYMFLAG_LOCAL) ||
				(pSymInfo->Flags & SYMFLAG_PARAMETER) )
			{
				symLinkage = ::Symbol::SL_LOCAL;
			}
			else
			{
				symLinkage = ::Symbol::SL_GLOBAL;
			}
		}

		// register the symbol
        Address baseAddr = 0;
        if (obj->GetDescriptor()->isSharedObject()) {
            baseAddr = obj->get_base_addr();
        }
		pFile->AddSymbol( new Object::Symbol( pSymInfo->Name,
												pSymInfo->Address - baseAddr,
												symType,
												symLinkage,
												pSymInfo->Size,
												lineInfo.LineNumber ) );
    }

    // keep enumerating symbols
    return TRUE;
}







void
Object::ParseDebugInfo( void )
{
    if( baseAddr == 0 )
    {
		if( desc->addr() == 0 )
		{
			// we're being called before we are attached to a process
			// trick image class into thinking we've done something
			deferredParse = true;
			return;
		}
		else
		{
			// now we have a module base address
			baseAddr = desc->addr();

			// Initialize our symbol handler
			//cout << "calling SymInitialize with proc=" << desc->GetProcessHandle() << endl;
			if( !SymInitialize( desc->GetProcessHandle(), NULL, FALSE ) )
			{
				fprintf( stderr, "SymInitialize failed: %x\n", GetLastError() );
			}

			// ensure we load line number information when we load
			// modules
			DWORD dwOpts = SymGetOptions();
			dwOpts &= ~(SYMOPT_UNDNAME); //want mangled names
			SymSetOptions(dwOpts | SYMOPT_LOAD_LINES);
		}
    }
	assert( baseAddr != 0 );

	// build a Module object for the current module (EXE or DLL)
	// Note that the CurrentModuleScoper object ensures that the
	// curModule member will be reset when we leave the scope of
	// this function.
	CurrentModuleScoper cms( &curModule );
	assert( curModule == NULL );
	curModule = new Object::Module( desc->file(), desc->addr() );

    HANDLE hProc = desc->GetProcessHandle();
    HANDLE hFile = desc->GetFileHandle();
    assert( hProc != NULL );
    assert( hProc != INVALID_HANDLE_VALUE );
    assert( hFile != NULL );
    assert( hFile != INVALID_HANDLE_VALUE );

    // find the sections we need to know about (.text and .data)
    FindInterestingSections( hProc, hFile );

    // load symbols for this module
    DWORD64 dw64BaseAddr = (DWORD64)baseAddr;
    DWORD64 loadRet = SymLoadModule64( hProc,           // proc handle
                                        hFile,          // file handle
                                        NULL,          // image name
                                        NULL,           // shortcut name
                                        dw64BaseAddr,   // load address
                                        0 );            // size of DLL
    if( loadRet == 0 )
    {
        DWORD dwErr = GetLastError();
        if( dwErr != 0 )
        {
            // TODO how to indicate the error?
            fprintf( stderr, "SymLoadModule64 failed for %s: %x\n",
				((file_.length() > 0) ? file_.c_str() : "<no name available>"),
                GetLastError() );
            return;
        }
    }

    // obtain info about the module - how can we without a module handle?
    // parse symbols for the module
    if( !SymEnumSymbols( hProc,                     // process handle
                            baseAddr,               // load address
                            "",                     // symbol mask (we use none)
                            SymEnumSymbolsCallback, // called for each symbol
                            this ) )                // client data
    {
        fprintf( stderr, "Failed to enumerate symbols: %x\n", GetLastError() );
    }


    // We have a module object, with one or more files,
    // each with one or more symbols.  However, the symbols
    // are not necessarily in order, nor do they necessarily have valid sizes.
    assert( curModule != NULL );
	curModule->BuildSymbolMap( this );
    curModule->DefineSymbols( this, symbols_ );

    // tell the symbol handler it can cleanup this module's symbols
	// TODO do we do this here, or do we wait till the destructor?
    if( !SymUnloadModule64( hProc, (DWORD64)baseAddr ) )
    {
        // TODO how to indicate the error
        fprintf( stderr, "SymUnloadModule64 failed: %x\n", GetLastError() );
    }

    // Since PE-COFF is very similar to COFF (in that it's not like ELF),
    // the .text and .data sections are perfectly mapped to code/data segments

    code_vldS_ = code_off_;
    code_vldE_ = code_off_ + code_len_ * sizeof(Word);
    data_vldS_ = data_off_;
    data_vldE_ = data_off_ + data_len_ * sizeof(Word);
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
            if (GetDescriptor()->isSharedObject())
                code_off_    = pScnHdr->VirtualAddress;
            else
                code_off_    = baseAddr + pScnHdr->VirtualAddress;
            code_len_    = pScnHdr->Misc.VirtualSize / sizeof(Word);
        }
        else if( strncmp( (const char*)pScnHdr->Name, ".data", 5 ) == 0 )
        {
            // note that section numbers are one-based
            dataSectionId = i + 1;

            data_ptr_    = (Word*)(((char*)mapAddr) +
                            pScnHdr->PointerToRawData);
            if (GetDescriptor()->isSharedObject())
                data_off_    = pScnHdr->VirtualAddress;
            else
                data_off_    = baseAddr + pScnHdr->VirtualAddress;
            data_len_    = pScnHdr->Misc.VirtualSize / sizeof(Word);
        }

        pScnHdr += 1;
    }
}


