/*
 * Copyright (c) 1996-2002 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
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

// $Id: Object-nt.C,v 1.13 2002/05/13 19:51:53 mjbrim Exp $

#include <iostream.h>
#include <iomanip.h>
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
int sym_offset_compare( const void *x, const void *y );




//---------------------------------------------------------------------------
// Object method implementation
//---------------------------------------------------------------------------


Object::~Object( void )
{
    if( pDebugInfo != NULL )
    {
        UnmapDebugInformation( pDebugInfo );
        pDebugInfo = NULL;
    }
}



void
Object::ParseDebugInfo( void )
{
    IMAGE_DEBUG_INFORMATION* pDebugInfo = NULL;
	bool bHaveSymbols = false;
    
    // access the module's debug information
    pDebugInfo = MapDebugInformation(NULL, (LPTSTR)file_.c_str(), NULL, 0);
    if( pDebugInfo != NULL )
    {
        // ensure that the base address is valid
        if( baseAddr == NULL )
        {
            // use the image base address from the disk image
            // (this should only happen for EXEs; we should have
            // the in-core base address of DLLs.)
            // 
            // TODO: we should be able to use the in-core address
            // for EXEs as well
            baseAddr = pDebugInfo->ImageBase;
        }
        assert( baseAddr != NULL );

        // determine the location of the relevant sections
        ParseSectionMap( pDebugInfo );

        //
        // parse the symbols, if available
        // (note that we prefer CodeView over COFF)
        //
        if( pDebugInfo->CodeViewSymbols != NULL && //ccw 27 apr 2001 
		strncmp((char*)pDebugInfo->CodeViewSymbols, "NB10",4))//ccw 27 apr 2001 
			//ccw 17 july 2000
			//ccw 8 aug 2000 added the strncmp section 
			//ccw 28 mar 2001
        {
            // we have CodeView debug information
			bHaveSymbols = true;
            ParseCodeViewSymbols( pDebugInfo );
        }
#ifndef mips_unknown_ce2_11 //ccw 20 mar 2001 : 28 mar 2001
       else if( pDebugInfo->CoffSymbols != NULL )
        {
            // we have COFF debug information
			bHaveSymbols = true;
            ParseCOFFSymbols( pDebugInfo );
        }
#endif
        else
        {
            // TODO - what to do when there's no debug information?
 		//ccw 28 mar 2001 : the following
 		//ccw 19 july 2000 -- begin
		//cout << "COFF SYMBOLS! " << file_.c_str() <<endl; 
		char *start;
		char tmpStr[1024];
		strcpy(tmpStr, file_.c_str());

		start = strstr(tmpStr, ".exe");
		if(!start){
			start = strstr(tmpStr, ".dll");
		}
//		DebugBreak();
		if(start){
			strcpy(start, ".map");
			//cout << "USING MAP! "<< tmpStr << endl;
#ifdef mips_unknown_ce2_11			
			if(!ParseMapSymbols(pDebugInfo,tmpStr)){
				//ccw 14 aug 2000
				char tmp[256], *ext;

				strcpy(tmp, file_.c_str());
				ext = strstr(tmp, ".dll");
				if(ext){
					strcpy(ext,".lib");
					pDebugInfo = MapDebugInformation(NULL, (LPTSTR)file_.c_str(), NULL, 0);
					if( pDebugInfo->CoffSymbols != NULL )
					{
						// we have COFF debug information
						bHaveSymbols = true;
						ParseCOFFSymbols( pDebugInfo ); 
					}else{
						//cout << " NO DEBUG INFO IN LIB" <<endl;
					}
				}else{
					//cout << " NO DEBUG INFO " <<endl;
				}
				// TODO - what to do when there's no debug information?
			}
#endif
		}else{
			//cout << " WHAT FILE IS THIS? " <<endl;
			// TODO - what to do when there's no debug information?
		}
		//ccw 19 july 2000 -- end
       }

#ifndef mips_unknown_ce2_11			
		// if we couldn't find any symbol information, see if we can
		// get something useful from the export table
		if( !bHaveSymbols )
		{
			ParseExports( pDebugInfo );
		}
#endif // mips_unknown_ce2_11
    }
    else
    {
        // indicate the failure to access the debug information
        log_perror(err_func_, "MapDebugInformation");
    }
}


#ifndef mips_unknown_ce2_11			
void
Object::ParseExports( IMAGE_DEBUG_INFORMATION* pDebugInfo )
{
    vector<Symbol> allSymbols;

    // map the object into our address space
    HANDLE hFile = CreateFile( file_.c_str(),
                            GENERIC_READ,
                            FILE_SHARE_READ,
                            NULL,   // lpSecurityAttributes - default, not inheritable
                            OPEN_EXISTING,
                            0,
                            NULL );
    if( hFile == INVALID_HANDLE_VALUE )
    {
        // we failed to open the file
	if( pd_debug_export_symbols )
	{
        	// report the error to the user
		cerr << "Object::ParseExports: failed to open file "
			<< file_.c_str()
			<< ", error = "
			<< GetLastError()
			<< endl;
	}
        return;
    }
    HANDLE hMapping = CreateFileMapping( hFile,
                        NULL,   // lpSecurityAttributes - default, not inheritable
                        PAGE_READONLY | SEC_IMAGE,
                        0, 0,   // max size of mapping - use size of file
                        NULL ); // make an unnamed mapping
    if( hMapping == NULL )
    {
        // we failed to create a file mapping
	if( pd_debug_export_symbols )
	{
        	// report the error to the user
		cerr << "Object::ParseExports: failed to create file mapping for file "
			<< file_.c_str()
			<< ", error = "
			<< GetLastError()
			<< endl;
	}
        CloseHandle( hFile );
        return;
    }
    LPVOID mapBase = MapViewOfFile( hMapping,
                        FILE_MAP_READ,
                        0, 0,   // start at beginning of file
                        0 );    // map whole file
    if( mapBase == NULL )
    {
        // we failed to map the file
	if( pd_debug_export_symbols )
	{
        	// report the error to the user
		cerr << "Object::ParseExports: failed to map file "
			<< file_.c_str()
			<< ", error = "
			<< GetLastError()
			<< endl;
	}
        CloseHandle( hFile );
        CloseHandle( hMapping );
        return;
    }

    // find the image's Export Directory
    unsigned long expDirSize;
    IMAGE_EXPORT_DIRECTORY* expDir = 
        (IMAGE_EXPORT_DIRECTORY*)ImageDirectoryEntryToData( mapBase,
                            TRUE,   // MappedAsImage
                            IMAGE_DIRECTORY_ENTRY_EXPORT,
                            &expDirSize );

	if( expDir != NULL )
	{
        // there is an Export Directory
        // add symbol items by name if possible, by ordinal if necessary
        //
        // Note that we have little indication whether these exports
        // are functions or data.  We assume any symbol in the .text
		// section is a function, and all others are data.
		//
		// We also don't have an indication of the object's size.
        //

        // find the name of the object
        string objName;
        if( expDir->Name != 0 )
        {
            // use the name from the export directory
            objName = (LPCTSTR)(((char*)mapBase) + expDir->Name);
        }
        else
        {
            // use the file name
            objName = file_;
        }

        // add a symbol for the module
        allSymbols.push_back( Symbol( objName,
            "",
            Symbol::PDST_MODULE,
            Symbol::SL_GLOBAL,
            code_off_,
            false,
            code_len_ * sizeof(Word) ));


        // add symbols for the exports
        DWORD* expAddressTable = (DWORD*)(((char*)mapBase) + expDir->AddressOfFunctions);
        if( expDir->NumberOfNames > 0 )
        {
            // the object exports items by name - add symbols by name
            DWORD* namePointerTable = (DWORD*)(((char*)mapBase) + expDir->AddressOfNames);
            WORD* ordinalTable = (WORD*)(((char*)mapBase) + expDir->AddressOfNameOrdinals);
            for( unsigned int i = 0; i < expDir->NumberOfNames; i++ )
            {
                // obtain the name for this export
                DWORD symNameRVA = namePointerTable[i];
                LPCTSTR symName = (LPCTSTR)(((char*)mapBase) + symNameRVA);

                // obtain the address for this export...
                // ...first, find the ordinal associated with this export...
                WORD symInternalOrdinal = ordinalTable[i];

                // ...next, find the address associated with that ordinal
                // note that the address has to be in the inferior process, not
                // its address as mapped in our process
                DWORD symRVA = expAddressTable[symInternalOrdinal];
                Address symAddr = baseAddr + symRVA;

				if( pd_debug_export_symbols )
				{
					cout << "Export: " << symName 
						<< ", ordinal = " << symInternalOrdinal + expDir->Base 
						<< ", RVA = " << symRVA
						<< ", addr = " << symAddr
						<< endl;
				}

                // add the symbol
				Symbol::SymbolType symType;
				if( (symAddr >= code_off_) && 
					(symAddr <= (code_off_ + (code_len_ * sizeof(Word)))) )
				{
					symType = Symbol::PDST_FUNCTION;
				}
				else
				{
					symType = Symbol::PDST_OBJECT;
				}
                allSymbols.push_back(( Symbol( symName,
                    objName,
                    symType,
                    Symbol::SL_GLOBAL,
                    symAddr,
                    false,
                    0 )));      // length is unknown
            }
        }
        else
        {
            // the object exports items by ordinal only
            // add symbols by building a name from the ordinal
            for( unsigned int i = 0; i < expDir->NumberOfFunctions; i++ )
            {
                // construct a name from this export
                // from its ordinal and image name
                char ordBuf[16];
                sprintf( ordBuf, "%d", i + expDir->Base );
                string symName = objName + "$" + ordBuf;

                // obtain the address for this export
                DWORD symRVA = expAddressTable[i];
                Address symAddr = baseAddr + symRVA;

				if( pd_debug_export_symbols )
				{
					 cout << "Export: " << symName 
						<< ", ordinal = " << i + expDir->Base + expDir->Base 
						<< ", RVA = " << symRVA
						<< ", addr = " << symAddr
						<< endl;
				}

                // add the symbol
				Symbol::SymbolType symType;
				if( (symAddr >= code_off_) && 
					(symAddr <= (code_len_ + (code_len_ * sizeof(Word)))) )
				{
					symType = Symbol::PDST_FUNCTION;
				}
				else
				{
					symType = Symbol::PDST_OBJECT;
				}
                allSymbols.push_back(( Symbol( symName,
                    objName,
                    symType,
                    Symbol::SL_GLOBAL,
                    symAddr,
                    false,
                    0 )));      // length is unknown
            }
        }

	// The rest of our code doesn't handle symbols with zero size well.
	// Unless we build a complete CFG and can identify all exit points of
	// a function, the best approximation we have for the size of the symbol is
	// the difference between a symbol and its successor.
        //
	VECTOR_SORT(allSymbols, sym_offset_compare);
	CVPatchSymbolSizes( allSymbols );

	// now that we've sorted and sized symbols for the exports,
	// we add them to the global set
        for( unsigned int i = 0; i < allSymbols.size(); i++ )
        {
            symbols_[allSymbols[i].name()] = allSymbols[i];
        }
	}

    // release the object
    UnmapViewOfFile( mapBase );
    CloseHandle( hMapping );
    CloseHandle( hFile );
}
#endif // mips_unknown_ce2_11


void
Object::ParseSectionMap( IMAGE_DEBUG_INFORMATION* pDebugInfo )
{
    DWORD i;

    // currently we care only about the .text and .data segments
    for( i = 0; i < pDebugInfo->NumberOfSections; i++ )
    {
        IMAGE_SECTION_HEADER& section = pDebugInfo->Sections[i];

        if( strncmp( (const char*)section.Name, ".text", 5 ) == 0 )
        {
            // note that section numbers are one-based
            textSectionId = i + 1;

            code_ptr_    = (Word*)(((char*)pDebugInfo->MappedBase) +
                            section.PointerToRawData);
            code_off_    = baseAddr + section.VirtualAddress;
            code_len_    = section.Misc.VirtualSize / sizeof(Word);
        }
        else if( strncmp( (const char*)section.Name, ".data", 5 ) == 0 )
        {
            // note that section numbers are one-based
            dataSectionId = i + 1;

            data_ptr_    = (Word*)(((char*)pDebugInfo->MappedBase) +
                            section.PointerToRawData);
            data_off_    = baseAddr + section.VirtualAddress;
            data_len_    = section.Misc.VirtualSize / sizeof(Word);
        }
    }
}


bool
Object::ParseCodeViewSymbols( IMAGE_DEBUG_INFORMATION* pDebugInfo )
{
    CodeView cv( (const char*)pDebugInfo->CodeViewSymbols, textSectionId );
    bool ret = true;

    if( cv.Parse() )
    {
        bool isDll = ((pDebugInfo->Characteristics & IMAGE_FILE_DLL)!=0);
        dictionary_hash<string, unsigned int> libDict( string::hash, 19 );
        vector<Symbol> allSymbols;
        vector<ModInfo> cvMods;         // CodeView's notion of modules
        vector<PDModInfo> pdMods;       // Paradyn's notion of modules
        unsigned int midx;
        unsigned int i;

        //
        // build a module map of the .text section
        // by creating a list of CodeView modules that contribute to 
        // the .text section, sorted by offset
        // 
        // note that the CodeView modules vector uses one-based indexing
        //
        const vector<CodeView::Module>& modules = cv.GetModules();
        for( midx = 1; midx < modules.size(); midx++ )
        {
            const CodeView::Module& mod = modules[midx];

            //
            // determine the Paradyn module that this
            // module will be associated with...
            //

            // ...first determine the library that contains
            // this module, if any...
            string libName;
            if( mod.GetLibraryIndex() != 0 )
            {
                libName = cv.GetLibraries()[mod.GetLibraryIndex()];
            }

            // ...next figure out the Paradyn module with which to associate
            // this CodeView module...
            unsigned int pdModIdx = UINT_MAX;
            if( !isDll && (mod.GetLibraryIndex() != 0) )
            {
                // associate symbol with static library

                // handle the case where this is the first time we've
                // seen this library
                if( !libDict.defines( libName ) )
                {
                    // add a Paradyn module for the library
                    // offset and size will be patched later
                    pdMods.push_back(PDModInfo( libName, 0, 0 ));
                    pdModIdx = pdMods.size() - 1;

                    // keep track of where we added the library,
                    // so we can patch the location of the library's code later
                    libDict[libName] = pdModIdx;
                }
                else
                {
                    // look up the index we saved earlier
                    pdModIdx = libDict[libName];
                }
            }
            else if( !isDll )
            {
                // add a Paradyn module for the module's source file

                DWORD offset;    // offset of code in text section
                DWORD cb;        // size of code in text section


                // find a source code name to associate with this module
                if( mod.GetTextBounds( offset, cb ) )
                {
                    pdMods.push_back(PDModInfo( mod.GetSourceName(), offset, cb ));
                    pdModIdx = pdMods.size() - 1;
                }
                else
                {
                    // the module doesn't contribute to the .text section
                    // TODO - so do we care about this module?  should
                    // we be keeping track of contributions
                    // to the data section?
                }
            }
            else
            {
                // module is part of a DLL, so we 
                // associate any symbols directly with the DLL
                pdMods.push_back(PDModInfo( pDebugInfo->ImageFileName, 0,
                                        code_len_ * sizeof(Word) ));
                pdModIdx = pdMods.size() - 1;
            }

            // add the module info to our vector for later sorting
            // (but only if it contributes code to the .text section)
            DWORD offText;
            DWORD cbText;
            if( mod.GetTextBounds( offText, cbText ) && (cbText > 0) )
            {
                assert( pdModIdx != UINT_MAX );
                cvMods.push_back(ModInfo( &mod, pdModIdx ));
            }
        }

        // sort list of modules by offset to give us our CodeView module map
	VECTOR_SORT(cvMods, ModInfo::CompareByOffset);

#ifdef CV_DEBUG
        // dump the CodeView module map
        cout << "CodeView module .text map:\n";
        for( midx = 0; midx < cvMods.size(); midx++ )
        {
            DWORD offText = 0;
            DWORD cbText = 0;
            string name = cvMods[midx].pCVMod->GetName();
            cvMods[midx].pCVMod->GetTextBounds( offText, cbText );

            cout << hex
                << "0x" << setw( 8 ) << setfill( '0' ) << offText
                << "-0x" << setw( 8 ) << setfill( '0' ) << offText + cbText - 1
                << " (" << setw( 8 ) << setfill( '0' ) << cbText << ")\t"
                << name
                << dec
                << endl;
        }
#endif // CV_DEBUG

        // compute bounds for Paradyn modules
        for( midx = 0; midx < cvMods.size(); midx++ )
        {
            PDModInfo& pdMod = pdMods[cvMods[midx].pdModIdx];
            DWORD offTextCV = 0;
            DWORD cbTextCV = 0;

            // determine the bounds of the CodeView text
            cvMods[midx].pCVMod->GetTextBounds( offTextCV, cbTextCV );

            if( cbTextCV > 0 )
            {
                // expand the PD module to cover the CV module's bounds
                if( pdMod.cbText == 0 )
                {
                    // this is the first CodeView module we've seen
                    // for this Paradyn module
                    pdMod.offText = offTextCV;
                    pdMod.cbText = cbTextCV;
                }
                else
                {
                    // we have to handle the (potential) expansion of
                    // the existing bounds
                    if( offTextCV < pdMod.offText )
                    {
                        DWORD oldOffset = pdMod.offText;

                        // reset the base and extend the bound
                        pdMod.offText = offTextCV;
                        pdMod.cbText += (oldOffset - offTextCV);
                    }

                    if((offTextCV + cbTextCV) > (pdMod.offText + pdMod.cbText))
                    {
                        // extend the bound
                        pdMod.cbText += 
                            ((offTextCV + cbTextCV) -
                                (pdMod.offText + pdMod.cbText));
                    }
                }
            }
        }

#ifdef CV_DEBUG
        // dump the Paradyn module map
        cout << "Paradyn module .text map:\n";
        for( midx = 0; midx < pdMods.size(); midx++ )
        {
            DWORD offText = pdMods[midx].offText;
            DWORD cbText = pdMods[midx].cbText;
            string name = pdMods[midx].name;

            cout << hex
                << "0x" << setw( 8 ) << setfill( '0' ) << offText
                << "-0x" << setw( 8 ) << setfill( '0' ) << offText + cbText - 1
                << " (" << setw( 8 ) << setfill( '0' ) << cbText << ")\t"
                << name
                << dec
                << endl;
        }
#endif // CV_DEBUG

        // add entries for our Paradyn modules
        for( midx = 0; midx < pdMods.size(); midx++ )
        {
            allSymbols.push_back(Symbol( pdMods[midx].name,
                "",
                Symbol::PDST_MODULE,
                Symbol::SL_GLOBAL,
                code_off_ + pdMods[midx].offText,
                false,
                pdMods[midx].cbText ));
        }


        //
        // now that we have a module map of the .text segment,
        // consider the symbols defined by each module
        //
        CVProcessSymbols( cv, cvMods, pdMods, allSymbols );


        // our symbols are sorted by offset
        // so we can patch up any outstanding sizes
        CVPatchSymbolSizes( allSymbols );

        // our symbols are finally ready to enter into
        // the main symbol dictionary
        for( i = 0; i < allSymbols.size(); i++ )
        {
            if(allSymbols[i].name() != "")
            {
                symbols_[allSymbols[i].name()] = allSymbols[i];
            }
        }
    }
    else
    {
        // indicate failure to parse symbols
        ret = false;
    }

    return ret;
}



void
Object::CVPatchSymbolSizes( vector<Symbol>& allSymbols )
{
    Address lastFuncAddr = NULL;
    unsigned int i;


    for( i = 0; i < allSymbols.size(); i++ )
    {
        Symbol& sym = allSymbols[i];

        if( (sym.name() != "") && (sym.size() == 0) &&
            ((sym.type() == Symbol::PDST_FUNCTION) ||
             (sym.type() == Symbol::PDST_OBJECT)))
        {
            // check for function aliases
            // note that this check depends on the allSymbols
            // array being sorted so that aliases are considered
            // after the "real" function symbol
            bool isAlias = false;
            if( (sym.type() == Symbol::PDST_FUNCTION) &&
                (sym.addr() == lastFuncAddr) &&
                (sym.size() == 0) )
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
                while( (j < allSymbols.size()) &&
                       ( ((allSymbols[j].type() != Symbol::PDST_FUNCTION) &&
                          (allSymbols[j].type() != Symbol::PDST_OBJECT)
                         ) ||
                         (allSymbols[j].addr() == sym.addr())
                       )
                     )
                {
                   j++;
                }

                if( j < allSymbols.size() &&
                    (allSymbols[j].type() == sym.type()) )
                {
                    // we found a symbol from the same section
                    // with a different address -
                    // size is just the delta between symbols
                    cb = allSymbols[j].addr() - sym.addr();
                }
                else
                {
                    // we couldn't find another symbol in our section
                    // with a different address -
                    // size is the remainder of the current section
                    if( sym.type() == Symbol::PDST_FUNCTION )
                    {
                        // size is remainder of the .text section
                        cb = (code_off_ + code_len_*sizeof(Word)) - sym.addr();
                    }
                    else
                    {
                        // size is remainder of the .data section
                        cb = (data_off_ + data_len_*sizeof(Word)) - sym.addr();
                    }
                }
                sym.change_size( cb );
            }

            // update the last known function symbol
            if( sym.type() == Symbol::PDST_FUNCTION )
            {
                lastFuncAddr = sym.addr();
            }
        }
    }
}


void
Object::CVProcessSymbols( CodeView& cv, 
                          vector<Object::ModInfo>& cvMods,
                          vector<Object::PDModInfo>& pdMods,
                          vector<Symbol>& allSymbols )
{
    unsigned int midx;
    unsigned int i;


    for( midx = 0; midx < cvMods.size(); midx++ )
    {
        const CodeView::Module& mod = *(cvMods[midx].pCVMod);
        PDModInfo& pdMod = pdMods[cvMods[midx].pdModIdx];
                    
        // add symbols for each global function defined in the module
        {
            const vector<CodeView::SymRecordProc*>& gprocs =
                mod.GetSymbols().GetGlobalFunctions();
            for( i = 0; i < gprocs.size(); i++ )
            {
                const CodeView::SymRecordProc* proc = gprocs[i];

                // build a symbol from the proc information
                LPString lpsName( proc->name );
                string strName = (string)lpsName;

                Address addr = code_off_ + proc->offset;

                allSymbols.push_back(( Symbol( strName,
                    pdMod.name,
                    Symbol::PDST_FUNCTION,
                    Symbol::SL_GLOBAL,
                    addr,
                    false,
                    proc->procLength )));
            }
        }

        // add symbols for each local function defined in the module
        {
            const vector<CodeView::SymRecordProc*>& lprocs =
                   mod.GetSymbols().GetLocalFunctions();
            for( i = 0; i < lprocs.size(); i++ )
            {
                const CodeView::SymRecordProc* proc = lprocs[i];
                LPString lpsName( proc->name );
                string strName = (string)lpsName;

                Address addr = code_off_ + proc->offset;

                allSymbols.push_back(( Symbol( strName,
                    pdMod.name,
                    Symbol::PDST_FUNCTION,
                    Symbol::SL_LOCAL,
                    addr,
                    false,
                    proc->procLength )));
            }
        }

        // handle thunks
        {
            const vector<CodeView::SymRecordThunk*>& thunks =
                mod.GetSymbols().GetThunks();
            for( i = 0; i < thunks.size(); i++ )
            {
                const CodeView::SymRecordThunk* thunk = thunks[i];
                LPString lpsName( thunk->name );
                string strName = (string)lpsName;

                Address addr = code_off_ + thunk->offset;

                allSymbols.push_back(( Symbol( strName,
                    pdMod.name,
                    Symbol::PDST_FUNCTION,
                    Symbol::SL_GLOBAL,
                    addr,
                    false,
                    thunk->thunkLength ) ));
            }
        }

        // add symbols for each global variable defined in the module
        {
            const vector<CodeView::SymRecordData*>& gvars =
                mod.GetSymbols().GetGlobalVariables();
            for( i = 0; i < gvars.size(); i++ )
            {
                const CodeView::SymRecordData* pVar = gvars[i];
                LPString lpsName( pVar->name );
                string strName = (string)lpsName;

                Address addr = data_off_ + pVar->offset;

                allSymbols.push_back(( Symbol( strName,
                    pdMod.name,
                    Symbol::PDST_OBJECT,
                    Symbol::SL_GLOBAL,
                    addr,
                    false,
                    0 )));               // will be patched later (?)
            }
        }

        {
            const vector<CodeView::SymRecordData*>& lvars =
                mod.GetSymbols().GetGlobalVariables();
            for( i = 0; i < lvars.size(); i++ )
            {
                const CodeView::SymRecordData* pVar = lvars[i];
                LPString lpsName( pVar->name );
                string strName = (string)lpsName;

                Address addr = data_off_ + pVar->offset;

                allSymbols.push_back(( Symbol( strName,
                    pdMod.name,
                    Symbol::PDST_OBJECT,
                    Symbol::SL_LOCAL,
                    addr,
                    false,
                    0 )));               // will be patched later (?)
            }
        }
    }

    // once we've handled the symbols that the CodeView object
    // could discover and associate with a module, we've
    // got to do something with symbols that were not explicitly
    // associated with a module in the CodeView information

    // Unfortunately, VC++/DF produce S_PUB32 symbols
    // for functions in some cases.  (For example,
    // when building a Digital Fortran program, the
    // software produces an executable with symbols
    // from the Fortran runtime libraries as S_PUB32
    // records.)  We do our best to try to determine
    // whether the symbol is a function, and if so,
    // how large it is, which module it belongs to, etc.
    const vector<CodeView::SymRecordData*>& pubs =
                                            cv.GetSymbols().GetPublics();
    for( i = 0; i < pubs.size(); i++ )
    {
        const CodeView::SymRecordData* sym = pubs[i];

        LPString lpsName( sym->name );
        string strName = (string)lpsName;

        // we now have to try to determine the type of the
        // symbol.  Since we're only given a type and a location,
        // (and the type might not even be valid) we assume
        // that any public symbol in the code section is
        // a function and we try to determine which module it
        // belongs to based on the module map we constructed earlier
        if( sym->segment == textSectionId )
        {
            Address addr = code_off_ + sym->offset;

            // save the symbol
            allSymbols.push_back(Symbol( strName,
                FindModuleByOffset( sym->offset, pdMods ),
                Symbol::PDST_FUNCTION,
                Symbol::SL_GLOBAL,
                addr,
                false,
                0 ));              // will be patched later
        }
        else if( sym->segment == dataSectionId )
        {
            Address addr = data_off_ + sym->offset;

            allSymbols.push_back(Symbol( strName,
                FindModuleByOffset( sym->offset, pdMods ),
                Symbol::PDST_OBJECT,
                Symbol::SL_GLOBAL,
                addr,
                false,
                0 ));              // will be patched later
        }
        else
        {
            // TODO - the symbol is not in the text or data
            // sections - do we care about it?
        }
    }
    
    // sort symbols by offset
    VECTOR_SORT(allSymbols, sym_offset_compare);
}


void
Object::ParseCOFFSymbols( IMAGE_DEBUG_INFORMATION* pDebugInfo )
{
    IMAGE_COFF_SYMBOLS_HEADER* pHdr = pDebugInfo->CoffSymbols;
    vector<Symbol>    allSymbols;
    bool gcc_compiled = false;
    bool isDll = ((pDebugInfo->Characteristics & IMAGE_FILE_DLL)!=0);
    DWORD u, v;

    
    // find the location of the symbol records and string table
    IMAGE_SYMBOL* syms = (IMAGE_SYMBOL*)(((char*)pHdr) +
                            pHdr->LvaToFirstSymbol);
    char* stringTable = ((char*)syms) +
                            pHdr->NumberOfSymbols * sizeof( IMAGE_SYMBOL );


    // for DLLs, we ignore filename information and associate 
    // symbols with a module representing the DLL
    if( isDll )
    {
        allSymbols.push_back(Symbol( pDebugInfo->ImageFileName,
                                "",
                                Symbol::PDST_MODULE,
                                Symbol::SL_GLOBAL,
                                code_off_,
                                false ));
    }


    // parse the COFF records
    for( v = 0; v < pDebugInfo->CoffSymbols->NumberOfSymbols; v++ )
    {
        string name = FindName( stringTable, syms[v] );
        Address sym_addr = NULL;


        //
        // handle the various types of COFF records...
        //

        if( name.prefixed_by("_$$$") || name.prefixed_by("$$$") )
        {
            // the record represents a branch target (?)
            // skip it
            v += syms[v].NumberOfAuxSymbols;
        }
        else if( syms[v].StorageClass == IMAGE_SYM_CLASS_FILE )
        {
            // the record is a file record
            //
            // note that for DLLs, we associate symbols directly with
            // the DLL and ignore any filename information we find
            if( !isDll )
            {
                // extract the name of the source file
                name = (char*)(&syms[v+1]);

                // skip auxiliary records containing the filename
                v += (strlen(name.c_str()) / sizeof(IMAGE_SYMBOL)) + 1;

                // find a .text record following the file name
                // if there is one, it contains the starting address for
                // this file's text
                // (note - there may not be one!  If not, we detect this by
                // finding the next .file record or running off the end of
                // the symbol information)
                DWORD tidx = v + 1;
                while( (tidx < pDebugInfo->CoffSymbols->NumberOfSymbols) &&
                        ((syms[tidx].N.Name.Short == 0) ||
                         ((strncmp( (const char*)(&syms[tidx].N.ShortName),
                                    ".text", 5 ) != 0) &&
                          (syms[tidx].StorageClass == IMAGE_SYM_CLASS_FILE))))
                {
                    // advance to next record
                    tidx++;
                }
                if( (tidx < pDebugInfo->CoffSymbols->NumberOfSymbols) &&
                    (syms[tidx].N.Name.Short != 0) &&
                    (strncmp( (const char*)(&syms[tidx].N.ShortName),
                                ".text", 5 ) == 0) )
                {
                    // this is text record for the recently-seen .file record -
                    // extract the starting address for symbols from this file
                    sym_addr = baseAddr + syms[tidx].Value;
                }
                else
                {
                    // there is not a .text record for the recently-seen .file
                    // TODO: is there any way we can
                    // determine the needed information in this case?
                    sym_addr = 0;
                }
            
                // make note of the symbol
                allSymbols.push_back(Symbol(name,
                                        "",
                                        Symbol::PDST_MODULE,
                                        Symbol::SL_GLOBAL,
                                        sym_addr,
                                        false));
            }
        }
        else if( syms[v].StorageClass == IMAGE_SYM_CLASS_LABEL )
        {
            // the record is a label

            // check whether the label indicates that the
            // module was compiled by gcc
            if( (name == "gcc2_compiled.") || (name == "___gnu_compiled_c") )
            {
                gcc_compiled = true;
            }
        }
        else if(( (syms[v].StorageClass != IMAGE_SYM_CLASS_TYPE_DEFINITION)
                    && ISFCN(syms[v].Type))
                || (gcc_compiled &&
                    (name == "__exit" || name == "_exit" || name == "exit")))
        {
            // the record represents a "type" (including functions)
            
            // the test for gcc and the exit variants is a kludge
            // to work around our difficulties in parsing the CygWin32 DLL
            sym_addr = (gcc_compiled ?
                        syms[v].Value :
                        baseAddr + syms[v].Value);

            if( syms[v].StorageClass == IMAGE_SYM_CLASS_EXTERNAL )
            {
                allSymbols.push_back(Symbol(name,
                                    "DEFAULT_MODULE",
                                    Symbol::PDST_FUNCTION,
                                    Symbol::SL_GLOBAL,
                                    sym_addr,
                                    false));
            }
            else
            {
                allSymbols.push_back(Symbol(name,
                                    "DEFAULT_MODULE",
                                    Symbol::PDST_FUNCTION,
                                    Symbol::SL_LOCAL,
                                    sym_addr,
                                    false));
            }

            // skip any auxiliary records with the function
            v += syms[v].NumberOfAuxSymbols;
        }
        else if( syms[v].SectionNumber > 0 )
        {
            // the record represents a variable (?)

            // determine the address to associate with the symbol
            sym_addr = (gcc_compiled ?
                        syms[v].Value :
                        baseAddr + syms[v].Value );

            if( name != ".text" )
            {
                if (syms[v].StorageClass == IMAGE_SYM_CLASS_EXTERNAL)
                {
                    allSymbols.push_back(Symbol(name,
                                        "DEFAULT_MODULE",
                                        Symbol::PDST_OBJECT,
                                        Symbol::SL_GLOBAL,
                                        sym_addr,
                                        false));
                }
                else
                {
                    allSymbols.push_back(Symbol(name,
                                        "DEFAULT_MODULE",
                                        Symbol::PDST_OBJECT,
                                        Symbol::SL_LOCAL,
                                        sym_addr,
                                        false));
                }
            }
            else
            {
                // we processed the .text record when we saw
                // its corresponding .file record - skip it
            }

            // skip any auxiliary records
            v += syms[v].NumberOfAuxSymbols;
        }
        else
        {
            // the record is of a type that we don't care about
            // skip it and all of its auxiliary records
            v += syms[v].NumberOfAuxSymbols;
        }


    }

    //
    // now that we've seen all the symbols,
    // we need to post-process them into something usable
    //

    // add an extra symbol to mark the end of the text segment
    allSymbols.push_back(Symbol("",
                    "DEFAULT_MODULE",
                    Symbol::PDST_OBJECT,
                    Symbol::SL_GLOBAL, 
                    code_off_ + code_len_ * sizeof(Word),
                    false));

    // Sort the symbols on address to find the function boundaries
    VECTOR_SORT(allSymbols, sym_offset_compare);

    // find the function boundaries
    for( u = 0; u < allSymbols.size(); u++ )
    {
        unsigned int size = 0;
        if( allSymbols[u].type() == Symbol::PDST_FUNCTION )
        {
            // find the function boundary
            v = u+1;
            while(v < allSymbols.size())
            {
                // The .ef below is a special symbol that gcc puts in to
                // mark the end of a function.
                if(allSymbols[v].addr() != allSymbols[u].addr() &&
                    (allSymbols[v].type() == Symbol::PDST_FUNCTION ||
                    allSymbols[v].name() == ".ef"))
                {
                    break;
                }
                v++;
            }
            if(v < allSymbols.size())
            {
                size = (unsigned)allSymbols[v].addr() 
                        - (unsigned)allSymbols[u].addr();
            }
            else
            {
                size = (unsigned)(code_off_ + code_len_*sizeof(Word))
                         - (unsigned)allSymbols[u].addr();
            }
        }

        // save the information about this symbol
        if(allSymbols[u].name() != "")
        {
            symbols_[allSymbols[u].name()] =
                Symbol(allSymbols[u].name(), 
                    isDll ? allSymbols[u].module() : "DEFAULT_MODULE", 
                    allSymbols[u].type(), allSymbols[u].linkage(),
                    allSymbols[u].addr(), allSymbols[u].kludge(),
                    size);
        }
    }
}




string
Object::FindName( const char* stringTable, const IMAGE_SYMBOL& sym )
{
    string name;

    if (sym.N.Name.Short != 0) {
        char sname[9];
        strncpy(sname, (char *)(&sym.N.ShortName), 8);
        sname[8] = 0;
        name = sname;
    } else {
        name = stringTable + sym.N.Name.Long;
    }

    return name;
}


// compare function for vector sort of
// CodeView modules
int
Object::ModInfo::CompareByOffset( const void* x, const void* y )
{
    const ModInfo* px = (const ModInfo*)x;
    const ModInfo* py = (const ModInfo*)y;
    assert( (px != NULL) && (px->pCVMod != NULL) );
    assert( (py != NULL) && (py->pCVMod != NULL) );

    // access the offset for each module
    DWORD offTextx = 0;
    DWORD cbTextx = 0;
    DWORD offTexty = 0;
    DWORD cbTexty = 0;
    px->pCVMod->GetTextBounds( offTextx, cbTextx );
    py->pCVMod->GetTextBounds( offTexty, cbTexty );

    int ret = 0;
    if( offTextx > offTexty )
    {
        ret = 1;
    }
    else if( offTextx < offTexty )
    {
        ret = -1;
    }
    else
    {
        // the offsets are equal - try our next comparison criteria
        if( (cbTextx != 0) && (cbTexty == 0) )
        {
            ret = 1;
        }
        else if( (cbTextx == 0) && (cbTexty != 0) )
        {
            ret = -1;
        }
    }
    
    return ret;
}


int
sym_offset_compare( const void *x, const void *y )
{
    const Symbol *s1 = (const Symbol *)x;
    const Symbol *s2 = (const Symbol *)y;
    int ret = 0;


    // first try comparing by address
    if( s1->addr() < s2->addr() ) 
    {
        ret = -1;
    }
    else if( s1->addr() > s2->addr() )
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
        if( (s1->size() != 0) && (s2->size() == 0) )
        {
            ret = -1;
        }
        else if( (s1->size() == 0) && (s2->size() != 0) )
        {
            ret = 1;
        }
    }
    return ret;
}



// FindModuleByOffset
// Determines the Paradyn module name
// based on the given offset into the .text section
string
Object::FindModuleByOffset( unsigned int offset,
                             const vector<Object::PDModInfo>& pdMods )
{
    string retval = "";
    unsigned int i;

    // we do simple linear search on Paradyn modules
    for( i = 0; i < pdMods.size(); i++ )
    {
        const PDModInfo& pdMod = pdMods[i];

        if( (offset >= pdMod.offText) &&
            (offset < pdMod.offText + pdMod.cbText) )
        {
            retval = pdMod.name;
            break;
        }
    }

    return retval;
}

#ifdef mips_unknown_ce2_11
///////////  ccw 30 mar 2001
//// 19 july 2000
#include "MapSymbols.h"

bool Object::ParseMapSymbols(IMAGE_DEBUG_INFORMATION *pDebugInfo, char *mapFile){

	MapSymbols mapSym(mapFile);
	//MapSymbols *dllSymbols;
	//int numberOfDLL;
	char symName[1024];
	Address address, baseAddress;
	bool function;
	unsigned long len;
	bool staticN;
	bool result, isDll;
	unsigned int u, v;//, size;

	/** from ParseCOFFSymbols */

	vector<Symbol> allSymbols;
	//bool gcc_compiled = false;
	bool isDLL = ((pDebugInfo->Characteristics & IMAGE_FILE_DLL) != 0 ); //ccw 27 apr 2001

	//DebugBreak();//ccw 4 mar 2001
	/***/
	if(!mapSym.parseSymbols()){
		return false;
	}
	//unsigned long code_off_ = 0, baseAddr = 0, code_len_ = 0;
	if(isDLL){
		allSymbols.push_back(Symbol(pDebugInfo->ImageFileName, "", Symbol::PDST_MODULE, 
			Symbol::SL_GLOBAL, code_off_, false));
		baseAddress = baseAddr; //ccw 10 apr 2001
	}else{
		baseAddress = mapSym.getPreferredLoadAddress();
	}

	result = mapSym.getFirstSymbol(symName, &address, &function, &len, &staticN);	
	while(result){
		
		if(function){

			allSymbols.push_back(Symbol(symName, "DEFAULT_MODULE", 
				Symbol::PDST_FUNCTION, staticN ? Symbol::SL_LOCAL: Symbol::SL_GLOBAL, address+ baseAddress, false,0 ));
		}else{
			allSymbols.push_back(Symbol(symName, "DEFAULT_MODULE",
				Symbol::PDST_OBJECT, staticN ? Symbol::SL_LOCAL: Symbol::SL_GLOBAL, address+ baseAddress, false,0));
		}

		result = mapSym.getNextSymbol(symName, &address, &function, &len, &staticN);
	}

	/** do size/length fix up here **/
	VECTOR_SORT(allSymbols, sym_offset_compare);

	/* from Object-nt.C */
	// find the function boundaries

    for( u = 0; u < allSymbols.size(); u++ )
    {
        unsigned int size = 0;
        if( allSymbols[u].type() == Symbol::PDST_FUNCTION )
        {
            // find the function boundary
            v = u+1;
            while(v < allSymbols.size())
            {
                // The .ef below is a special symbol that gcc puts in to
                // mark the end of a function.
                if(allSymbols[v].addr() != allSymbols[u].addr() &&
                    allSymbols[v].type() == Symbol::PDST_FUNCTION )
                {
                    break;
                }
                v++;
            }
            if(v < allSymbols.size())
            {
                size = (unsigned)allSymbols[v].addr() 
                        - (unsigned)allSymbols[u].addr();
            }
            else
            {
                size = (unsigned)(code_off_ + code_len_*sizeof(Word))
                         - (unsigned)allSymbols[u].addr();
            }
       }

        // save the information about this symbol
        if(allSymbols[u].name() != "")
        {
            symbols_[allSymbols[u].name()] =
                Symbol(allSymbols[u].name(), 
                    isDll ? allSymbols[u].module() : "DEFAULT_MODULE", 
                    allSymbols[u].type(), allSymbols[u].linkage(),
                    allSymbols[u].addr(), allSymbols[u].kludge(),
                    size);
			//cout << symbols_[allSymbols[u].name()];
        }
    }
	

	//delete [] dllSymbols;
	return true;	
}

#endif

