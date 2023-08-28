/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <windows.h>
#include <cvconst/cvconst.h>
#include <oleauto.h>
#if !defined __out_ecount_opt
#define __out_ecount_opt(x)
#endif
#include <dbghelp.h>

#include <iostream>
#include <iomanip>
#include <limits.h>
#include <crtdbg.h>
#include <winnt.h>

#include "symtabAPI/src/Object.h"
#include "symtabAPI/src/Object-nt.h"

#include "LineInformation.h"
#include "Collections.h"
#include "Symtab.h"
#include "Module.h"
#include "Function.h"
#include "Variable.h"
#include "emitWin.h"
#include "SymReader.h"
#include "common/src/headers.h"

using namespace Dyninst;
using namespace Dyninst::SymtabAPI;

Type *getType(HANDLE p, Offset mod_base, int typeIndex, Module *mod = NULL);
bool pd_debug_export_symbols = false;
using namespace std;

std::string convertCharToString(const char *ptr){
    std::string str;
    if(ptr)
	str = ptr;
    else
	str = "";
    return str;
}

static void printSysError(unsigned errNo) {
    char buf[1000];
	
	int result = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, errNo,
							MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
								buf, 1000, NULL);
    if (!result) {
	fprintf(stderr, "Couldn't print error message\n");
		printSysError(GetLastError());
    }
    fprintf(stderr, "*** System error [%u]: %s\n", errNo, buf);
    fflush(stderr);
}

//---------------------------------------------------------------------------
// prototypes of functions used in this file
//---------------------------------------------------------------------------
BOOL CALLBACK SymEnumSymbolsCallback( PSYMBOL_INFO pSymInfo,
										ULONG symSize,
										PVOID userContext );

//---------------------------------------------------------------------------
// Object method implementation
//---------------------------------------------------------------------------
struct	CompareSymAddresses: public binary_function<const Object::intSymbol*, const Object::intSymbol*, bool> 
{
	bool operator()(const Object::intSymbol *s1, const Object::intSymbol* s2) const {
		bool ret = false;
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

Object::Module::Module( std::string _name, DWORD64 _baseAddr, DWORD64 _extent )	 :
						name(_name),
						baseAddr(_baseAddr),
						extent(_extent),
						isDll( false )
{
	defFile = new Object::File();
	files.push_back( defFile );
}

Object::File*
Object::Module::FindFile( std::string name )
{
	File* ret = NULL;
	for( std::vector<File *>::iterator iter = files.begin();
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
Object::File::DefineSymbols( dyn_hash_map<std::string, std::vector< Symbol *> >& allSyms,
                             const std::string& modName ) const
{
    for( std::vector<Object::intSymbol*>::const_iterator iter = syms.begin(); iter != syms.end(); iter++ ) {
        const Object::intSymbol* curSym = * iter;
	assert( curSym != NULL );
	curSym->DefineSymbol( allSyms, modName );
    }
}

void
Object::intSymbol::DefineSymbol(dyn_hash_map<std::string,std::vector<Symbol *> >&allSyms,
                                const std::string& modName ) const
{
    Symbol *sym = new Symbol(GetName(), 
                             (Symbol::SymbolType) GetType(),
                             (Symbol::SymbolLinkage) GetLinkage(),
                             Symbol::SV_UNKNOWN,
                             (Offset)GetAddr(),
                             NULL,
                             GetRegion(),
                             GetSize());
    allSyms[GetName()].push_back(sym);
}

void
Object::Module::DefineSymbols( const Object* obj,
                               dyn_hash_map<std::string, std::vector< Symbol *> > & syms) const
{
    // define Paradyn/dyninst modules and symbols
    if( !isDll )
	{
        // this is an EXE
        for( std::vector<Object::File*>::const_iterator iter = files.begin();
                iter != files.end();
             iter++ ) {
            const File* curFile = *iter;
            assert( curFile != NULL );
            
            //fprintf(stderr, "ObjMod::DefineSymbols for %s\n", curFile->GetName().c_str());
            // add a Symbol for the file
            Symbol *sym = new Symbol( curFile->GetName(), 
                                      Symbol::ST_MODULE,
                                      Symbol::SL_GLOBAL,
                                      Symbol::SV_UNKNOWN,
                                      obj->code_off(),	// TODO use real base of symbols for file
                                      NULL,
                                      NULL, 0 );	// TODO Pass Section pointer also
            // TODO also pass size
            // add symbols for each of the file's symbols
            syms[curFile->GetName()].push_back(sym);

            curFile->DefineSymbols( syms, curFile->GetName() );
        }
    }
    else
    {
        // we represent a DLL
        // add one Symbol for the entire module

        Symbol *sym = new Symbol(name,
                                 Symbol::ST_MODULE,
                                 Symbol::SL_GLOBAL,
                                 Symbol::SV_UNKNOWN,
                                 obj->code_off(),
                                 NULL,
                                 NULL,					//TODO pass Sections pointer
                                 obj->code_len());
    
        syms[name].push_back(sym); 
        symsToMods[sym] = name;

        // add symbols for each of the module's symbols
        for( std::vector<Object::File*>::const_iterator iter = files.begin();
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
								  const std::vector<Object::intSymbol*>& allSyms ) const
{
	DWORD64 lastFuncAddr = NULL;
	unsigned int i;
	
	for( i = 0; i < allSyms.size(); i++ )
    {
		Object::intSymbol* sym = allSyms[i];
		assert( sym != NULL );
		if( (sym->GetName() != "") && (sym->GetSize() == 0) &&
	    ((sym->GetType() == Symbol::ST_FUNCTION) ||
	     (sym->GetType() == Symbol::ST_OBJECT)))
	{
	    // check for function aliases
	    // note that this check depends on the allSymbols
	    // array being sorted so that aliases are considered
	    // after the "real" function symbol
	    bool isAlias = false;
	    if( (sym->GetType() == Symbol::ST_FUNCTION) &&
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
				// to the next symbol.	(Sometimes this causes us to
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
				while((j < allSyms.size()) &&
				(((allSyms[j]->GetType() != Symbol::ST_FUNCTION) &&
								(allSyms[j]->GetType() != Symbol::ST_OBJECT)) ||
								(allSyms[j]->GetAddr() == sym->GetAddr())))
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
					if( sym->GetType() == Symbol::ST_FUNCTION )
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
			if( sym->GetType() == Symbol::ST_FUNCTION )
			{
				lastFuncAddr = sym->GetAddr();
			}
		}
    }
}

void
Object::Module::BuildSymbolMap( const Object* obj ) const
{
	std::vector<Object::intSymbol*> allSyms;
	// add all symbols to our allSyms std::vector
	std::vector<Object::File*>::const_iterator iter = files.begin();
	for(;	iter != files.end();	iter++ )
	{
		assert( *iter != NULL );
		const std::vector<Object::intSymbol*>& curSyms = (*iter)->GetSymbols();
		for( std::vector<Object::intSymbol*>::const_iterator symIter = curSyms.begin(); symIter != curSyms.end();symIter++ )
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
}

#define SymTagFunction 0x5
#define SymTagData 0x7
#define SymTagPublicSymbol 0xa
#define SymTagMisc 0x3808		// Seen with NB11, VC++6-produced executables
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
	DWORD symType = Symbol::ST_UNKNOWN;
	DWORD symLinkage = Symbol::SL_UNKNOWN;
	DWORD64 codeLen = code_len();
	DWORD64 codeBase = code_off();
	symType = Symbol::ST_FUNCTION;
	//codeBase += get_base_addr();
	// Logic: if it's tagged/flagged as a function, function.
	// If it's tagged/flagged as a public symbol and it points
	// to text, function.
	// Otherwise, variable.
	if ((pSymInfo->Flags & SYMFLAG_FUNCTION) ||
		(pSymInfo->Tag == SymTagFunction))
	{
		symLinkage = Symbol::SL_UNKNOWN;
	}
	else if (((pSymInfo->Flags & SYMFLAG_EXPORT) || 
			  (pSymInfo->Tag == SymTagPublicSymbol)) && 
	         isText((Offset) pSymInfo->Address - (Offset)mf->base_addr()))
	{
		symType = Symbol::ST_FUNCTION;
		symLinkage = Symbol::SL_UNKNOWN;
	}
	else
	{
		symType = Symbol::ST_OBJECT;
		symLinkage = Symbol::SL_GLOBAL;
	}
    // register the symbol
    Offset baseAddr = 0;
    //	if (desc.isSharedObject())
    //if(curModule->IsDll())
    //	 baseAddr = get_base_addr();

    if( !isForwarded( ((Offset) pSymInfo->Address) - baseAddr ) )
    {
        pFile->AddSymbol( new Object::intSymbol
                          ( pSymInfo->Name,
                            pSymInfo->Address - get_base_addr(),
                            symType,
                            symLinkage,
                            pSymInfo->Size,
                            findEnclosingRegion((Offset)(pSymInfo->Address - get_base_addr())) ));
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
	if(pSymInfo->Name && (strcmp(pSymInfo->Name, "test1_1_func1_1") == 0))
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

/*
 * This function finds all the global symbols
 * in a module and all of the source files. (i.e. so this function would find
 * the 'main' symbol and find the starting point of 'foo.c'
 *
 */
void Object::ParseSymbolInfo( bool alloc_syms )
{
   // build a Module object for the current module (EXE or DLL)
   // Note that the CurrentModuleScoper object ensures that the
   // curModule member will be reset when we leave the scope of
   // this function.
   // curModule = new Object::Module( file_, desc.code() );
   string file_ = mf->filename();
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

   // grab load address
   if (peHdr) imageBase = peHdr->OptionalHeader.ImageBase;
   else imageBase = 0; 

   // load symbols for this module
   //DWORD64 dw64BaseAddr = (DWORD64)desc.loadAddr();
   HANDLE mapAddr = mf->base_addr();
   DWORD64 dw64BaseAddr = (DWORD64)mapAddr;
   //load address is always same(fake address space)
   HANDLE hFile = mf->getFileHandle();
   DWORD64 loadRet = SymLoadModule64( hProc,			// proc handle
         hFile,		// file handle
         NULL,		// image name
         NULL,		// shortcut name
         dw64BaseAddr,	// load address
         0 );		// size of DLL	  
   if(!loadRet) {
      DWORD dwErr = GetLastError();
      if(dwErr) {
         string file_ = mf->filename();
         fprintf( stderr, "SymLoadModule64 failed for %s\n",
               ((file_.length() > 0) ? file_.c_str() 
                : "<no name available>"));
         //printSysError(dwErr);
         goto done;
      }
   }
   // parse symbols for the module
   if( !SymEnumSymbols(hProc,			   // process handle
            dw64BaseAddr,		    // load address
            "",			// symbol mask (we use none)
            SymEnumSymbolsCallback, // called for each symbol
            this ) )		// client data
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
   if (alloc_syms)
      curModule->DefineSymbols( this, symbols_);
   no_of_symbols_ = symbols_.size();

   //fprintf(stderr, "%s[%d]:  removed call to parseFileLineInfo here\n", FILE__, __LINE__);

   // Since PE-COFF is very similar to COFF (in that it's not like ELF),
   // the .text and .data sections are perfectly mapped to code/data segments
   code_vldS_ = code_off_;
   code_vldE_ = code_off_ + code_len_;
   data_vldS_ = data_off_;
   data_vldE_ = data_off_ + data_len_;

done:
   delete curModule;
}

// Ensure that the optional header has a TLS directory entry
// calculate the TLS directory address and make sure it's valid
// calculate the address of the TLS callback array and make sure it's valid
// for each TLS callback, add a function symbol
void Object::AddTLSFunctions()
{
   // ensure that the optional header has a TLS directory entry
   if (!peHdr || peHdr->OptionalHeader.NumberOfRvaAndSizes <= IMAGE_DIRECTORY_ENTRY_TLS) {
      return;
   }

   // calculate the TLS directory address and make sure it's valid
   unsigned long tlsSize = peHdr->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].Size;
   if (!tlsSize) {
      return;
   }
   Address imgBase = peHdr->OptionalHeader.ImageBase;
   Offset tlsMemOff = peHdr->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress;
   Region *secn = findEnclosingRegion(tlsMemOff);
   if (!secn || (tlsMemOff - secn->getMemOffset()) > secn->getDiskSize()) {
      return;
   }
   Offset tlsDiskOff = tlsMemOff 
      + (Offset)secn->getDiskOffset() 
      - (Offset)secn->getMemOffset();
   IMAGE_TLS_DIRECTORY *tlsDir = (IMAGE_TLS_DIRECTORY*) 
      ( tlsDiskOff + (Offset)mf->base_addr() );

   // calculate the address of the TLS callback array and make sure it's valid
   secn = findEnclosingRegion(tlsDir->AddressOfCallBacks - imgBase);
   if (!secn) {
      return;
   }
   Offset cbOffSec = tlsDir->AddressOfCallBacks 
      - secn->getMemOffset() 
      - imgBase;
   if (cbOffSec > secn->getDiskSize()) {
      return;
   }
   Offset cbOffDisk = cbOffSec + secn->getDiskOffset();
   PIMAGE_TLS_CALLBACK *tlsCBs = (PIMAGE_TLS_CALLBACK*) 
      ( cbOffDisk + (Offset)mf->base_addr() );
   unsigned maxCBs = (secn->getDiskSize() - cbOffSec) / sizeof(PIMAGE_TLS_CALLBACK);

   // for each TLS callback, add a function symbol
   for (unsigned tidx=0; tidx < maxCBs && tlsCBs[tidx] != NULL ; tidx++) {
      Offset funcOff = ((Address) tlsCBs[tidx]) - imgBase;
      secn = findEnclosingRegion(funcOff);
      if (!secn) {
         continue;
      }
      Offset baseAddr = 0;
      Object::File *pFile = curModule->GetDefaultFile();
      char funcName [128];
      snprintf(funcName, 128, "tls_cb_%u", tidx);
      pFile->AddSymbol( new Object::intSymbol
                       ( funcName,
                         funcOff,
                         Symbol::ST_FUNCTION,
                         Symbol::SL_GLOBAL,
                         0, // unknown size
                         secn ));
   }
}

Region::perm_t getRegionPerms(DWORD flags){
    if((flags & IMAGE_SCN_MEM_EXECUTE) && (flags & IMAGE_SCN_MEM_WRITE))
        return Region::RP_RWX;
    else if(flags & IMAGE_SCN_MEM_EXECUTE)
        return Region::RP_RX;
    else if(flags & IMAGE_SCN_MEM_WRITE)
        return Region::RP_RW;
    else
        return Region::RP_R;
}

Region::RegionType getRegionType(DWORD flags){
    if((flags & IMAGE_SCN_CNT_CODE) && (flags & IMAGE_SCN_CNT_INITIALIZED_DATA))
        return Region::RT_TEXTDATA;
    else if(flags & IMAGE_SCN_CNT_CODE)
        return Region::RT_TEXT;
    else if((flags & IMAGE_SCN_CNT_INITIALIZED_DATA) || (flags & IMAGE_SCN_CNT_UNINITIALIZED_DATA))
        return Region::RT_DATA;
    else
        return Region::RT_OTHER;
}

std::vector<std::pair<string, IMAGE_IMPORT_DESCRIPTOR> > & Object::getImportDescriptorTable()
{
   if (!idt_.empty()) {
      return idt_;
   }

   if (peHdr->OptionalHeader.NumberOfRvaAndSizes <= IMAGE_DIRECTORY_ENTRY_IMPORT)
      assert(0 && "PE header doesn't specify the IDT address");

   //1. get the RVA of import table from Data directory
   DWORD dwITrva = peHdr->OptionalHeader.
      DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
   //printf("Import Table RVA: %lx\n", dwITrva);

   //2. get the offset in disk file
   DWORD dwIToffset = RVA2Offset(dwITrva);
   //printf("import table disk offset: %lx\n", dwIToffset);

   PIMAGE_IMPORT_DESCRIPTOR import_d = (PIMAGE_IMPORT_DESCRIPTOR)
      (((char*)mf->base_addr())+dwIToffset);

   while(import_d ->Name != NULL && import_d->FirstThunk !=NULL){
      IMAGE_IMPORT_DESCRIPTOR ie;
      memcpy(&ie, import_d, sizeof(IMAGE_IMPORT_DESCRIPTOR));
      string str((char*)(((char*)mf->base_addr())+RVA2Offset(import_d->Name)));
      idt_.push_back(pair<string,IMAGE_IMPORT_DESCRIPTOR>(str,ie));
      //printf("%s\n",ie.name);
      import_d ++;
   }
   return idt_;
   //cout<<"size of import table"<<image_import_descriptor.size()<<endl;
}

map<string, map<string, WORD> > & Object::getHintNameTable()
{
   if (!hnt_.empty()) {
      return hnt_;
   }

   vector<pair<string, IMAGE_IMPORT_DESCRIPTOR> > idt = getImportDescriptorTable();
   for (vector<pair<string, IMAGE_IMPORT_DESCRIPTOR> >::iterator dit = idt.begin();
        dit != idt.end();
        dit++) 
   {
      assert(sizeof(Offset) == getAddressWidth());
      Offset * iat = (Offset*)((char*)mf->base_addr() + RVA2Offset(dit->second.FirstThunk));

      for (unsigned idx=0; iat[idx] != 0; idx++) {
         assert (0 == (0x80000000 & iat[idx])); //ensure IAT is not ordinal-based
         IMAGE_IMPORT_BY_NAME *hintName = (IMAGE_IMPORT_BY_NAME *)
            ((char*)mf->base_addr() + RVA2Offset(iat[idx]));
         hnt_[dit->first][string((char*)hintName->Name)] = hintName->Hint;
      }
   }
   
   return hnt_;
}

void Object::FindInterestingSections(bool alloc_syms, bool defensive)
{
   // now that we have the file mapped, look for 
   // the .text and .data sections
   assert( peHdr == NULL );
   HANDLE mapAddr = mf->base_addr();
   peHdr = ImageNtHeader( mapAddr );

   if (peHdr == NULL) {
      code_ptr_ = (char*)mapAddr;
      code_off_ = 0;
      HANDLE hFile = mf->getFileHandle();
      code_len_ = mf->size();
      is_aout_ = false;
      fprintf(stderr,"Adding Symtab object with no program header, will " 
              "designate it as code, code_ptr_=%s code_len_=%lx\n",
              code_ptr_,code_len_);
      if (alloc_syms) {
          Region *bufReg = new Region
                    (0, //region number
                     ".text", 
                     code_off_, // disk offset
                     code_len_, // disk size
                     code_off_, // mem offset
                     code_len_, // mem size
                     code_ptr_, // raw data ptr
                     Region::RP_RWX, 
                     Region::RT_TEXT,
                     true);// is loadable
          regions_.push_back(bufReg);
      }
      return;
   }

   assert( peHdr->FileHeader.SizeOfOptionalHeader > 0 ); 

   string file_ = mf->filename();
   curModule = new Object::Module( file_, 0 );
   assert( curModule != NULL );

   curModule->SetIsDll( (peHdr->FileHeader.Characteristics & IMAGE_FILE_DLL) != 0 );
   if(curModule->IsDll())
      is_aout_ = false;
   else
      is_aout_ = true;

   getImportDescriptorTable(); //save the binary's original table, we may change it later
   


   //get exported functions
   // note: there is an error in the PE specification regarding the export 
   //       table Base.  The spec claims that you are supposed to subtract 
   //       the Base to get correct ordinal indices into the Export Address 
   //       table, but this is false, at least in the typical case for which 
   //       Base=1, I haven't observed any binaries with different bases
	if (!is_aout_ && peHdr->OptionalHeader.NumberOfRvaAndSizes > IMAGE_DIRECTORY_ENTRY_EXPORT) {
		assert(sizeof(Offset) == getAddressWidth());
		unsigned long size;
		IMAGE_EXPORT_DIRECTORY *eT2 = (IMAGE_EXPORT_DIRECTORY *)::ImageDirectoryEntryToData(mapAddr, false, IMAGE_DIRECTORY_ENTRY_EXPORT, &size);
		if (eT2) {
			DWORD *funcNamePtrs = (DWORD *) ::ImageRvaToVa(ImageNtHeader(mapAddr), mapAddr, ULONG(eT2->AddressOfNames), NULL);
			DWORD *funcAddrs = (DWORD *) ::ImageRvaToVa(ImageNtHeader(mapAddr), mapAddr, ULONG(eT2->AddressOfFunctions), NULL);
			WORD *funcAddrNameMap = (WORD *) ::ImageRvaToVa(ImageNtHeader(mapAddr), mapAddr, ULONG(eT2->AddressOfNameOrdinals), NULL);
			if (funcNamePtrs && funcAddrs && funcAddrNameMap) {
				for (unsigned i = 0; i < eT2->NumberOfNames; ++i) {
					char *name = (char *) ::ImageRvaToVa(ImageNtHeader(mapAddr), mapAddr, funcNamePtrs[i], NULL);
					
					if (!strcmp(name,"??_7__non_rtti_object@@6B@") || !strcmp(name,"??_7bad_cast@@6B@") || !strcmp(name,"??_7bad_typeid@@6B@") || !strcmp(name,"??_7exception@@6B@") || !strcmp(name,"sys_errlist"))
					{
					continue;
					}
					if (!strcmp(name,"??_7__non_rtti_object@std@@6B@") || !strcmp(name,"??_7bad_cast@std@@6B@") || !strcmp(name,"??_7bad_typeid@std@@6B@") || !strcmp(name,"??_7exception@std@@6B@"))
					{
					continue;
					}
					int funcIndx = funcAddrNameMap[i];
					Address funcAddr = funcAddrs[funcIndx];
					if ((funcAddr >= (Address) eT2) &&
						(funcAddr < ((Address) eT2 + size))) continue;
					Symbol *sym = new Symbol(name,
						Symbol::ST_FUNCTION, 
				        Symbol::SL_GLOBAL, 
			            Symbol::SV_DEFAULT,
				        funcAddr);
					sym->setDynamic(true); // it's exported, equivalent to ELF dynamic syms
					symbols_[name].push_back(sym);
				}
			}
		}
	}

   SecAlignment = peHdr ->OptionalHeader.SectionAlignment;
   unsigned int nSections = peHdr->FileHeader.NumberOfSections;
   no_of_sections_ = nSections;
   Address prov_begin = (Address)-1;
   Address prov_end = (Address)-1;
   code_off_ = (Address)-1;
   code_len_ = (Address)-1;

   if (defensive) {
       // add section for peHdr, determine the size taken up by the section 
       // in the program's address space.  
       unsigned long secSize = ( peHdr->OptionalHeader.SizeOfHeaders 
                                 / peHdr->OptionalHeader.SectionAlignment ) 
                              * peHdr->OptionalHeader.SectionAlignment;
       if (  peHdr->OptionalHeader.SizeOfHeaders 
           % peHdr->OptionalHeader.SectionAlignment ) 
       {
          secSize += peHdr->OptionalHeader.SectionAlignment;
       }
       prov_begin = 0;
       prov_end = prov_begin + secSize;
       regions_.push_back(
           new Region(
            0, "PROGRAM_HEADER", 0, peHdr->OptionalHeader.SizeOfHeaders, 
            0, secSize, (char*)mapAddr,
            getRegionPerms(IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_WRITE), 
            getRegionType(IMAGE_SCN_CNT_CODE | IMAGE_SCN_CNT_INITIALIZED_DATA),
            true));
   }

   PIMAGE_SECTION_HEADER pScnHdr = (PIMAGE_SECTION_HEADER)(((char*)peHdr) + 
                                 sizeof(DWORD) + sizeof(IMAGE_FILE_HEADER) +
                                 peHdr->FileHeader.SizeOfOptionalHeader);
   bool foundText = false;
   for( unsigned int i = 0; i < nSections; i++ ) {
      // rawDataPtr should be set to be zero if the amount of raw data
      // for the section is zero

      Offset diskOffset = 0; 
      if (pScnHdr->SizeOfRawData != 0) {
         // the loader rounds PointerToRawData to the previous fileAlignment 
         // boundary  (usually 512 bytes)
         diskOffset = (Offset)
             ((pScnHdr->PointerToRawData / peHdr->OptionalHeader.FileAlignment) 
              * peHdr->OptionalHeader.FileAlignment);
      }
      Offset secSize = (pScnHdr->Misc.VirtualSize > pScnHdr->SizeOfRawData) ? 
          pScnHdr->Misc.VirtualSize : pScnHdr->SizeOfRawData;
      if (alloc_syms)
          regions_.push_back
              (new Region(i+1, 
                          (const char *)pScnHdr->Name,
                          diskOffset,
                          pScnHdr->SizeOfRawData,
                          pScnHdr->VirtualAddress, 
                          secSize,
                          (char *)(diskOffset + (Offset)mapAddr), 
                          getRegionPerms(pScnHdr->Characteristics),
                          getRegionType(pScnHdr->Characteristics)));
//        regions_.push_back(new Section(i, (const char*)pScnHdr->Name, 
//                                      pScnHdr->VirtualAddress, 
//                                      pScnHdr->Misc.VirtualSize, 
//                                      rawDataPtr));

      if( strncmp( (const char*)pScnHdr->Name, ".text", 8 ) == 0 ) {
         // note that section numbers are one-based
         textSectionId = i + 1;
         code_ptr_    = (char*)(((char*)mapAddr) +
                                pScnHdr->PointerToRawData);
         code_off_    = pScnHdr->VirtualAddress;

         // Since we're reporting the size of sections on the disk,
         // we need to check whether the size of raw data is smaller.
         //code_len_    = pScnHdr->Misc.VirtualSize;
         code_len_ = ((pScnHdr->SizeOfRawData < pScnHdr->Misc.VirtualSize) ?
                      pScnHdr->SizeOfRawData : pScnHdr->Misc.VirtualSize);

         foundText = true;
         if (prov_begin == -1) {
             prov_begin = code_off_;
             prov_end = code_off_ + code_len_;
         } else {
             if (prov_begin > code_off_) {
                 prov_begin = code_off_;
             }
             if ( prov_end < (code_off_ + code_len_) ) {
                  prov_end = (code_off_ + code_len_);
             }
         }
      }
      else if( strncmp( (const char*)pScnHdr->Name, ".data", 8 ) == 0 ) {
         // note that section numbers are one-based
         dataSectionId = i + 1;
         data_ptr_    = (char *)(((char*)mapAddr) +
                                 pScnHdr->PointerToRawData);
         data_off_    = pScnHdr->VirtualAddress;
         data_len_ = (pScnHdr->SizeOfRawData < pScnHdr->Misc.VirtualSize ?
                      pScnHdr->SizeOfRawData : pScnHdr->Misc.VirtualSize);
         if (defensive) { // don't parse .data in a non-defensive binary
             if (prov_begin == -1) {
                prov_begin = data_off_;
                prov_end = data_off_ + data_len_;
             } else {
                 if (prov_begin > data_off_) {
                     prov_begin = data_off_;
                 }
                 if (prov_end < (data_off_ + data_len_)) {
                     prov_end = (data_off_ + data_len_);
                 }
             }
         }
      }
      else {
         Offset sec_len = (pScnHdr->SizeOfRawData < pScnHdr->Misc.VirtualSize) ?
                           pScnHdr->SizeOfRawData : pScnHdr->Misc.VirtualSize;
         if (-1 == prov_begin) {
            prov_begin = pScnHdr->VirtualAddress;
            prov_end = prov_begin + sec_len;
         } else {
             if (prov_begin > pScnHdr->VirtualAddress) {
                 prov_begin = pScnHdr->VirtualAddress;
             }
             if (prov_end < (pScnHdr->VirtualAddress + sec_len)) {
                 prov_end = (pScnHdr->VirtualAddress + sec_len);
             }
         }
      }
      pScnHdr += 1;
   } // end section for loop

   if (-1 == code_len_ || defensive) {
       // choose the smaller/larger of the two offsets/lengths, 
       // if both are initialized (i.e., are not equal to -1)
       if (code_off_ == -1)
           code_off_ = prov_begin;
       else if (prov_begin != -1 && 
                code_off_ > prov_begin) 
           code_off_ = prov_begin;

       if (code_len_ == -1)
           code_len_ = prov_end - code_off_;
       else if (prov_end != -1 &&
                code_len_ < (prov_end - code_off_))
           code_len_ = (prov_end - code_off_);

       assert(code_off_ != -1 && code_len_ != -1); // no sections in binary? 
   }
}

// Assumes region list is sorted and regions don't overlap
Region *Object::findEnclosingRegion(const Offset where)
{
    // search for "where" in regions (regions must not overlap)
    int first = 0; 
    int last = regions_.size() - 1;
    while (last >= first) {
        Region *curreg = regions_[(first + last) / 2];
        if (where >= curreg->getMemOffset()
            && where < (curreg->getMemOffset()
                        + curreg->getMemSize())) {
            return curreg;
        }
        else if (where < curreg->getMemOffset()) {
            last = ((first + last) / 2) - 1;
        }
        else {/* where >= (cursec->getSecAddr()
                           + cursec->getSecSize()) */
            first = ((first + last) / 2) + 1;
        }
    }
    return NULL;
}

bool Object::isForwarded( Offset addr )
{
	//calls to forwarded symbols are routed to another dll and 
    //are not in the current dll's code space
	//we MUST NOT try to parse these - bad things happen
	
	//we detect forwarded symbols by checking if the relative 
	//virtual address of the symbol falls within the dll's exports section
	if(peHdr && peHdr->FileHeader.Characteristics & IMAGE_FILE_DLL )
	{
		PIMAGE_DATA_DIRECTORY dataDir = peHdr->OptionalHeader.DataDirectory;
		Offset exportStart = dataDir->VirtualAddress;
		Offset exportEnd = exportStart + dataDir->Size;
		if( addr >= exportStart && addr < exportEnd )
			return true;  //this sym is forwarded
	}
	return false;
}

bool Object::getCatchBlock(ExceptionBlock &b, Offset addr, 
                           unsigned size) const 
{ 
   return false; 
}

bool Object::isText( const Offset addr ) const 
{
   return( addr >= code_off_ && addr < code_off_ + code_len_ );
}

void fixup_filename(std::string &filename)
{
	if (filename.substr(0,22) == "\\Device\\HarddiskVolume") {
		TCHAR volumePath[1024];
		if (GetVolumePathName(filename.c_str(), volumePath, 1024)) {
			std::string::size_type filePathIndex = filename.find_first_of("\\/", 22);
			if (filePathIndex != std::string::npos)
				filename = volumePath + filename.substr(++filePathIndex);
			else
				filename = volumePath + filename.substr(23);
		} else {
			filename = "c:"+filename.substr(23);
		}
	}
}

Object::Object(MappedFile *mf_,
               bool defensive, 
               void (*err_func)(const char *), bool alloc_syms, Symtab *st) :
    AObject(mf_, err_func, st),
    curModule( NULL ),
    baseAddr( 0 ),
    imageBase( 0 ),
	preferedBase( 0 ),
    peHdr( NULL ),
    trapHeaderPtr_( 0 ),
    SecAlignment( 0 ),
    textSectionId( 0 ),
    dataSectionId( 0 ),
    hProc( NULL )
{
   FindInterestingSections(alloc_syms, defensive);
   if (alloc_syms && defensive) {
      AddTLSFunctions();
   }
   ParseSymbolInfo(alloc_syms);
   preferedBase = imageBase;
   rebase(0);
}

SYMTAB_EXPORT ObjectType Object::objType() const 
{
	return is_aout() ? obj_Executable : obj_SharedLib;
}


void Object::getModuleLanguageInfo(dyn_hash_map<std::string, supportedLanguages> *mod_langs)
{
	return;
}

struct line_info_tmp_t {
	line_info_tmp_t(unsigned long a, unsigned int l) {addr = a; line_no = l;}
	unsigned long addr;
	unsigned int line_no;
};

struct line_info_tmp_lt {
	bool operator()(const line_info_tmp_t &a, const line_info_tmp_t &b) const {
      return a.addr < b.addr;
	};
};

typedef std::multiset<line_info_tmp_t, line_info_tmp_lt> info_for_file_t;
typedef std::map<std::string, info_for_file_t*> info_for_all_files_t;

static BOOL CALLBACK add_line_info(SRCCODEINFO *srcinfo, void *param)
{
	info_for_all_files_t *all_info = (info_for_all_files_t *) param;
	info_for_all_files_t::iterator iter = all_info->find(std::string(srcinfo->FileName));
	info_for_file_t *finfo = NULL;
	if (iter == all_info->end()) {
		finfo = new info_for_file_t();
		(*all_info)[std::string(srcinfo->FileName)] = finfo;
	}
	else {
		finfo = (*iter).second;
	}
	finfo->insert(line_info_tmp_t((unsigned long) srcinfo->Address, srcinfo->LineNumber));
	return true;
}

static bool store_line_info(Symtab* st,	info_for_all_files_t *baseInfo)
{
   for (info_for_all_files_t::iterator i = baseInfo->begin(); i != baseInfo->end(); i++)
   {
	   const char *filename = (*i).first.c_str();
	   Module* mod;
	   
	   if(!st->findModuleByName(mod, filename)) 
	   {
	     mod = st->getDefaultModule();
	   }    
	   LineInformation* li_for_module = mod->getLineInformation();
	   if(!li_for_module) 
	   {
	     li_for_module = new LineInformation;
	     mod->setLineInfo(li_for_module);
	   }

	   for (info_for_file_t::iterator j = (*i).second->begin(); j != (*i).second->end(); j++) {
		   info_for_file_t::iterator next = j;
		   next++;
		   if (next != (*i).second->end())
			   li_for_module->addLine(filename, j->line_no, 0, j->addr, next->addr);
		   else
			   li_for_module->addLine(filename, j->line_no, 0, j->addr, j->addr);
	   }
	   delete (*i).second;
   }
   return true;
}

void Object::parseFileLineInfo()
{   
  if(parsedAllLineInfo) return;
  
  int result;
  static Offset last_file = 0x0;

  Offset baseAddr = get_base_addr();
  if (last_file == baseAddr)
    return;
  last_file = baseAddr;
  info_for_all_files_t inf;
  result = SymEnumLines(hProc, 
			      baseAddr,
			      NULL, 
			      NULL,
			      add_line_info, 
			      &inf); 
  // Set to true once we know we've done as much as we can
  parsedAllLineInfo = true;
  if (!result) {
    //Not a big deal. The module probably didn't have any debug information.
    DWORD dwErr = GetLastError();
    //printf("[%s:%u] - Couldn't SymEnumLines on %s in %s\n", 
	//	   __FILE__, __LINE__, src_file_name, libname);
    return;
  }
  store_line_info(associated_symtab, &inf);
  
}

typedef struct localsStruct {
    Function *func{};
    Offset base{};
    HANDLE p{};
    map<unsigned, unsigned> foundSyms;
    localsStruct() : foundSyms() {}
} localsStruct;

Dyninst::MachRegister WinConvert(Register reg) {
	//  Info from CV_HREG_e structure; from comments online this is correct
	switch(reg) {
	case CV_REG_EAX:
		return x86::eax;
	case CV_REG_EBX:
		return x86::ebx;
	case CV_REG_ECX:
		return x86::ecx;
	case CV_REG_EDX:
		return x86::edx;
	case CV_REG_ESP:
		return x86::esp;
	case CV_REG_EBP:
		return x86::ebp;
	case CV_REG_ESI:
		return x86::esi;
	case CV_REG_EDI:
		return x86::edi;
	default:
		return Dyninst::InvalidReg;
	}
}

BOOL CALLBACK enumLocalSymbols(PSYMBOL_INFO pSymInfo, unsigned long symSize,
                               void *userContext)
{
    Type *type;
    Function *func;
    storageClass storage;
    localVar *newvar;
    MachRegister reg;
    signed long frameOffset;
    Offset base;
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
		reg = x86::ebp;
		frameOffset = (signed) pSymInfo->Address;
        storage = storageRegOffset;
        storageName = "Frame Relative";
    }
    else if (pSymInfo->Flags & IMAGEHLP_SYMBOL_INFO_REGRELATIVE)
    {
        reg = WinConvert(pSymInfo->Register);
        frameOffset = (signed) pSymInfo->Address;
        storage = storageRegOffset;
        storageName = "Register Relative";
    }
    else if (pSymInfo->Flags & IMAGEHLP_SYMBOL_INFO_REGISTER) {
        reg = WinConvert(pSymInfo->Register);
        frameOffset = 0;
        storage = storageReg;
        storageName = "Register";
    }
    else {
        frameOffset = (signed) pSymInfo->Address;
        storage = storageAddr;
        storageName = "Absolute";
    }
	
	VariableLocation loc;
	loc.stClass = storage;
	loc.refClass = storageNoRef;
	loc.frameOffset = frameOffset;
	loc.lowPC = 0;
	loc.hiPC = (Address) -1;
	loc.mr_reg = reg;
	
	std::string vName = convertCharToString(pSymInfo->Name);
	std::string fName = convertCharToString(func->getModule()->fileName().c_str());
   newvar = new localVar(vName, type, fName, -1, func);
	newvar->addLocation(loc);

    //Store the variable as a local or parameter appropriately
   if (pSymInfo->Flags & IMAGEHLP_SYMBOL_INFO_PARAMETER) {
      assert(func);
      if (!func->addParam(newvar)) {
         fprintf(stderr, "%s[%d]:  addParam failed\n", FILE__, __LINE__);
         return false;
      }
      paramType = "parameter";
   }
   else if (pSymInfo->Flags & IMAGEHLP_SYMBOL_INFO_LOCAL) {
	  assert(func);
      if (!func->addLocalVar(newvar)) {
         fprintf(stderr, "%s[%d]:  addLocalVar failed\n", FILE__, __LINE__);
         return false;
      }
      paramType = "local";
   }
   else {
	   
      fprintf(stderr, "[%s:%d] - Local variable of unknown type.  %s in %s\n",
              __FILE__, __LINE__, pSymInfo->Name, func->pretty_names_begin()->c_str());
      paramType = "unknown";
   }

    
   const char *typeName;
   if (type) {
      typeName = type->getName().c_str();
   }
   else {
      typeName = "unknown";
   }
   
   return true;
}


static void enumLocalVars(Function *func, 
                          localsStruct *locals) 
{
    IMAGEHLP_STACK_FRAME frame;
    memset(&frame, 0, sizeof(IMAGEHLP_STACK_FRAME));

    frame.InstructionOffset = locals->base + func->getOffset();
    int result = SymSetContext(locals->p, &frame, NULL);
	/*if (!result) {            
		fprintf(stderr, "[%s:%u] - Couldn't SymSetContext\n", __FILE__, __LINE__);
        printSysError(GetLastError());
    }*/
    result = SymEnumSymbols(locals->p, 0, NULL, enumLocalSymbols, locals);
	/*if (!result) {
        fprintf(stderr, "[%s:%u] - Couldn't SymEnumSymbols\n", __FILE__, __LINE__);
        printSysError(GetLastError());
    }*/
	
	if(func->getSize())
	{
		memset(&frame, 0, sizeof(IMAGEHLP_STACK_FRAME));

		frame.InstructionOffset = locals->base +
                    func->getOffset() + 
                    func->getSize();
		result = SymSetContext(locals->p, &frame, NULL);
		result = SymEnumSymbols(locals->p, 0, NULL, enumLocalSymbols, locals);
	}

//TODO?? replace this with??
#if 0
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
#endif

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

// Changed. Not adding to stdTypes now
static void addTypeToCollection(Type *type, Module *mod) 
{
	typeCollection *tc = typeCollection::getModTypeCollection(mod);
	assert(tc);
	tc->addType(type);
/*	   
   typeCollection *collection;

   collection = mod ? tc : Symtab::stdTypes;
   assert(collection);
   assert(!collection->findType(type->getID()));
   collection->addType(type);
*/
}

static char *getTypeName(HANDLE p, Offset base, int typeIndex) {
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

static dataClass getDataClass(HANDLE p, Offset base, int typeIndex) {
    enum SymTagEnum wintype;
    int result, basetype;

    result = SymGetTypeInfo(p, base, typeIndex, TI_GET_SYMTAG, &wintype);
    if (!result)
        return dataUnknownType;
    switch (wintype) {
        case SymTagFunction:
        case SymTagFunctionType:
            return dataFunction;
        case SymTagPointerType:
            return dataPointer;
        case SymTagArrayType:
            return dataArray;
        case SymTagBaseType:
            return dataScalar;
        case SymTagEnum:
            return dataEnum;
        case SymTagTypedef:
            return dataTypedef;
        case SymTagUDT:
            enum UdtKind udtType;
            result = SymGetTypeInfo(p, base, typeIndex, TI_GET_UDTKIND, &udtType);
            if (!result)
                return dataUnknownType;
            switch (udtType) {
                case UdtUnion:
                    return dataUnion;
                case UdtStruct:
                case UdtClass:
                    return dataStructure;
                default:
                    return dataUnknownType;
            }
        case SymTagFunctionArgType:
            result = SymGetTypeInfo(p, base, typeIndex, TI_GET_TYPEID, &basetype);
            if (!result)
                return dataUnknownType;
            return getDataClass(p, base, basetype);
        default:
            return dataUnknownType;
    }
}

static Type *getEnumType(HANDLE p, Offset base, int typeIndex, Module *mod) {
    unsigned i;
    char *name = NULL;
    char *entryName = NULL;
    VARIANT entryValue;
    typeEnum *type;
    int result;
    unsigned numEntries, entriesSize;
    TI_FINDCHILDREN_PARAMS *entries = NULL;

    name = getTypeName(p, base, typeIndex);
	std::string tName = convertCharToString(name);
    type = new typeEnum(typeIndex, tName);
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
        type->addConstant(entryName, variantValue(&entryValue));
    }
  
    if (entries)
        free(entries);
    return type;    
}

static Type *getPointerType(HANDLE p, Offset base, int typeIndex, Module *mod) {
    int baseTypeIndex, result;
    Type *baseType;
    typePointer *newType;

    result = SymGetTypeInfo(p, base, typeIndex, TI_GET_TYPEID, &baseTypeIndex);
    if (!result) {
        fprintf(stderr, "[%s:%d] - TI_GET_TYPEID failed\n", __FILE__, __LINE__);
        return NULL;
    }

    //
    // Add a place-holder for the pointer type first and fill in it's 
    //  base type latter.  This prevents recursion that may happen beneath 
    //  the getType function call below.
    //
    newType = new typePointer(typeIndex, NULL);
    addTypeToCollection(newType, mod);

    baseType = getType(p, base, baseTypeIndex, mod);
    if (!baseType) {
        fprintf(stderr, "[%s:%d] - getType failed\n", __FILE__, __LINE__);
        return NULL;
    }

    newType->setPtr(baseType);
    return newType;
}

static Type *getArrayType(HANDLE p, Offset base, int typeIndex, Module *mod) {
    int result, baseIndex, index;
    Type *indexType, *newType, *baseType;
    unsigned size, num_elements;
    ULONG64 size64;
    std::string bname;
    std::string name;

    //Get the index type (usually an int of some kind).  Currently not used.
    result = SymGetTypeInfo(p, base, typeIndex, TI_GET_ARRAYINDEXTYPEID, &index);
    if (!result) {
        fprintf(stderr, "[%s:%d] - TI_GET_ARRAYINDEXTYPEID failed\n",
                __FILE__, __LINE__);
        return NULL;
    }
    indexType = getType(p, base, index, mod);

    //Get the base type (the type of the elements in the array)
    result = SymGetTypeInfo(p, base, typeIndex, TI_GET_TYPEID, &baseIndex);
    if (!result) {
        fprintf(stderr, "[%s:%d] - TI_GET_TYPEID failed\n", __FILE__, __LINE__);
        return NULL;
    }
    baseType = getType(p, base, baseIndex, mod);

    bname = baseType->getName();
    name = bname + "[]";
	
    result = SymGetTypeInfo(p, base, typeIndex, TI_GET_LENGTH, &size64);
    if (!result) {
        num_elements = 0;
    }
    else {
      size = (unsigned) size64;
      num_elements = size / baseType->getSize();
    }

    newType = new typeArray(typeIndex, baseType, 0, num_elements-1, name);
    newType->getSize();
    addTypeToCollection(newType, mod);
    assert(newType->getID() == typeIndex);

    return newType;
}


static Type *getTypedefType(HANDLE p, Offset base, int typeIndex, Module *mod) {
    int result, baseTypeIndex;
    Type *baseType, *newType;
    char *name;

    result = SymGetTypeInfo(p, base, typeIndex, TI_GET_TYPEID, &baseTypeIndex);
    if (!result) {
        fprintf(stderr, "[%s:%d] - TI_GET_TYPEID failed\n", __FILE__, __LINE__);
        return NULL;
    }
    baseType = getType(p, base, baseTypeIndex, mod);
    if (!baseType) {
        return NULL;
    }
 
    name = getTypeName(p, base, typeIndex);
	std::string tName = convertCharToString(name);
    newType = new typeTypedef(typeIndex, baseType, tName);
    addTypeToCollection(newType, mod);
    return newType;
}

static Type *getUDTType(HANDLE p, Offset base, int typeIndex, Module *mod) {
    int result, symtag;
    unsigned size, numChildren, childrenSize, child_offset, i, child_size;
    fieldListType *newType;
    UINT64 size64;
    const char *name, *childName;
    enum UdtKind udtType;
    TI_FINDCHILDREN_PARAMS *children = NULL;

    //
    // Get name for structure
    //
    name = getTypeName(p, base, typeIndex);
	std::string tName = convertCharToString(name);
	result = SymGetTypeInfo(p, base, typeIndex, TI_GET_LENGTH, &size64);
    if (!result) {
        fprintf(stderr, "%s - TI_GET_LENGTH return error\n", name);
        return NULL;
    }
    size = (unsigned) size64;

    //
    // Determine whether it's a class, struct, or union and create the 
    //  new_type appropriately
    //
    result = SymGetTypeInfo(p, base, typeIndex, TI_GET_UDTKIND, &udtType);
    if (!result) {
        fprintf(stderr, "%s - TI_GET_UDTKIND returned error\n", name);
        return NULL;
    }
    switch (udtType) {
        case UdtUnion:
            newType = new typeUnion(typeIndex, tName);
            break;
        case UdtStruct:
        case UdtClass:
        default:
            newType = new typeStruct(typeIndex, tName);
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
        Type *child_type = getType(p, base, children->ChildId[i], mod);
        if (!child_type)
            continue;

        // Figure out a name of this object
        childName = NULL;
        result = SymGetTypeInfo(p, base, children->ChildId[i], TI_GET_SYMTAG, &symtag);
        if (result && symtag == SymTagBaseClass) {
            childName = P_strdup("{superclass}");
        }
        if (!childName)
            childName = getTypeName(p, base, children->ChildId[i]);
        if (!childName) 
            childName = P_strdup(child_type->getName().c_str());

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

		std::string fName = convertCharToString(childName);
        newType->addField(fName, child_type, child_offset);
        if (childName)
            free((void *) childName);
        childName = NULL;
    }

    if (children)
        free(children);

    return newType;
}

static Type *getLayeredType(HANDLE p, Offset base, int typeIndex, Module *mod) {
    int result, newTypeIndex;
    Type *newType;

    result = SymGetTypeInfo(p, base, typeIndex, TI_GET_TYPEID, &newTypeIndex);
    if (!result) {
        fprintf(stderr, "TI_GET_TYPEID failed\n");
        return NULL;
    }

    newType = getType(p, base, newTypeIndex, mod);
    return newType;
}

static Type *getFunctionType(HANDLE p, Offset base, int typeIndex, Module *mod) {
    int result, retTypeIndex;
    typeFunction *newType;
    Type *retType;
    unsigned num_params, args_size, i;
    std::vector<Type *> params;
    TI_FINDCHILDREN_PARAMS *args = NULL;
    std::string name;

    //Create the function early to avoid recursive references
	std::string tName = "";
    newType = new typeFunction(typeIndex, NULL, tName);
    addTypeToCollection(newType, mod);

    result = SymGetTypeInfo(p, base, typeIndex, TI_GET_TYPEID, &retTypeIndex);
    if (!result) {
        fprintf(stderr, "[%s:%d] - Couldn't TI_GET_TYPEID\n", __FILE__, __LINE__);
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
        Type *arg_type = getType(p, base, args->ChildId[i], mod);
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
    name += "()";
	
	std::string newName = name;
    newType->setName(newName);

    for (i=0; i<params.size(); i++) {
        //TODO?? have a name for the parameter. Required??
        newType->addParam(params[i]);
    }

    if (args)
        free(args);

    return newType;
}

static Type *getBaseType(HANDLE p, Offset base, int typeIndex, Module *mod) {
    BasicType baseType;
    int result;
    ULONG64 size64;
    unsigned size;
    Type *newType;

    result = SymGetTypeInfo(p, base, typeIndex, TI_GET_BASETYPE, &baseType);
    if (!result) {
        fprintf(stderr, "TI_GET_BASETYPE return error\n");
        return NULL;
    }

    result = SymGetTypeInfo(p, base, typeIndex, TI_GET_LENGTH, &size64);
    if (!result) {
        fprintf(stderr, "TI_GET_LENGTH return error\n");
        return NULL;
    }
    size = (unsigned) size64;
    switch(baseType) {
	 case btNoType:
		 newType = NULL;
		 break;
	 case btVoid:
         newType = new typeScalar(typeIndex, size, "void");
		 break;
	 case btChar:
         newType = new typeScalar(typeIndex, size, "char");
		 break;
	 case btWChar:
         newType = new typeScalar(typeIndex, size, "wchar");
		 break;
	 case btInt:
         if (size == 8)
           newType = new typeScalar(typeIndex, size, "long long int");
         else if (size == 4)
           newType = new typeScalar(typeIndex, size, "int");
         else if (size == 2)
           newType = new typeScalar(typeIndex, size, "short");
         else if (size == 1)
           newType = new typeScalar(typeIndex, size, "char");
         else
           newType = new typeScalar(typeIndex, size, "");
		 break;
	 case btUInt:
         if (size == 8)
           newType = new typeScalar(typeIndex, size, "unsigned long long int");
         else if (size == 4)
           newType = new typeScalar(typeIndex, size, "unsigned int");
         else if (size == 2)
           newType = new typeScalar(typeIndex, size, "unsigned short");
         else if (size == 1)
           newType = new typeScalar(typeIndex, size, "unsigned char");
         else
           newType = new typeScalar(typeIndex, size, "");
		 break;
	 case btFloat:
         if (size == 8)
             newType = new typeScalar(typeIndex, size, "double");
         else
             newType = new typeScalar(typeIndex, size, "float");
		 break;
	 case btBCD:
         newType = new typeScalar(typeIndex, size, "BCD");
		 break;
	 case btBool:
         newType = new typeScalar(typeIndex, size, "bool");
		 break;
	 case btLong:
         newType = new typeScalar(typeIndex, size, "long");
		 break;
	 case btULong:
         newType = new typeScalar(typeIndex, size, "unsigned long");
		 break;
	 case btCurrency:
         newType = new typeScalar(typeIndex, size, "currency");
		 break;
	 case btDate:
         newType = new typeScalar(typeIndex, size, "Date");
		 break;
	 case btVariant:
         newType = new typeScalar(typeIndex, size, "variant");
		 break;
	 case btComplex:
         newType = new typeScalar(typeIndex, size, "complex");
		 break;
	 case btBit:
         newType = new typeScalar(typeIndex, size, "bit");
		 break;
	 case btBSTR:
         newType = new typeScalar(typeIndex, size, "bstr");
		 break;
	 case btHresult:
         newType = new typeScalar(typeIndex, size, "Hresult");
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

static Type *getType(HANDLE p, Offset base, int typeIndex, Module *mod) 
{
   static unsigned depth = 0;
   BOOL result;
   Type *foundType = NULL;
   typeCollection *collection;
   enum SymTagEnum symtag;

   if (!typeIndex)
       return NULL;

   //
   // Check if this type has already been created (they're indexed by typeIndex).
   // If it has, go ahead and return the existing one.
   // If not, then start creating a new type.
   //
   if (mod)
       collection = typeCollection::getModTypeCollection(mod);
   else
	   collection = (typeCollection*)Symtab::stdTypes;
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
	HANDLE handle;
	Symtab *obj;
    Offset base_addr;
} proc_mod_pair;

static void findLocalVars(Function *func, proc_mod_pair base) {
    Module *mod = func->getModule();
    localsStruct locals;
    HANDLE p = base.handle;

    locals.func = func;
    locals.base = base.base_addr;
    locals.p = p;

	enumLocalVars(func, &locals);
    //
    // The windows debugging interface allows us to get local variables
    // at specific points, which makes it hard to enumerate all locals (as we want).
    // Instead we'll get the local variables at the most common points below.
    //
    //TODO?
	//=const std::vector<instPoint*> &points = ifunc->funcEntries();
    //=enumLocalVars(func, ifunc->funcEntries(), &locals);
    //=enumLocalVars(func, ifunc->funcExits(), &locals);
    //=enumLocalVars(func, ifunc->funcCalls(), &locals);
    //=enumLocalVars(func, ifunc->funcArbitraryPoints(), &locals);
}

BOOL CALLBACK add_type_info(PSYMBOL_INFO pSymInfo, ULONG SymbolSize, void *info)
{
   HANDLE p;
   Offset obj_base;
   proc_mod_pair *pair;
   Symtab *obj;
   Type *type;
   char *name;
   Address addr;

   if (!isGlobalSymbol(pSymInfo)) {
       //We do local symbols elsewhere
       return TRUE;
   }

   pair = (proc_mod_pair *) info;
   p = pair->handle;
   obj_base = pair->base_addr;
   obj = pair->obj;
   name = pSymInfo->Name;
   addr = (Address) pSymInfo->Address - obj_base;

   std::vector<Module *> mods;
   Module *mod;
   //TODO?? change later
   if(!obj->getAllModules(mods))
   {
	   return true;
   }
   else
	   mod = mods[0];
   
   if (obj->isExec()) {
      //When parsing the a.out, sort the type information into specific modules.  This doesn't matter
      // for libraries, because there is a 1:1 mapping between modules and objects.
      //
      //A module is a collection of functions, but doesn't include global data types.  Global variables
      // will go into the DEFAULT_MODULE
		Function *f = NULL;
	   if(obj->findFuncByEntryOffset(f, (Offset) pSymInfo->Address))
	   {
	     //No containing module.  Only insert this into DEFAULT_MODULE
          if (strcmp(f->getModule()->fileName().c_str(), "DEFAULT_MODULE"))
              return true;
      }
   }

   type = getType(p, obj_base, pSymInfo->TypeIndex, mod);

   
//   fprintf(stderr, "[%s:%u] - Variable %s had type %s\n", __FILE__, __LINE__,
//       name, type ? type->getName().c_str() : "{NO TYPE}");
   
   if (type)
   {
      std::vector<Variable *> vars;
      bool result = obj->findVariablesByOffset(vars, addr);
      if (result) {
         for (auto v: vars)  {
            v->setType(type);
         }
      }
      if (name) {
		 typeCollection *tc = typeCollection::getModTypeCollection(mod);
		 assert(tc);
         tc->addGlobalVariable(type);
      }
   }
   return TRUE;
}

void Object::parseTypeInfo() {
    proc_mod_pair pair;
    BOOL result;
    //
    //Parse global variable type information
    //

    pair.handle = hProc;
    pair.obj = associated_symtab;
    pair.base_addr = getBaseAddress();
    
    if (!pair.base_addr) {
        pair.base_addr = getLoadAddress();
    }
  
   HANDLE mapAddr = mf->base_addr();
    result = SymEnumSymbols(hProc, (DWORD64)mapAddr, NULL, 
                            add_type_info, &pair);
	if (!result){
		printSysError(GetLastError());
//        parsing_printf("SymEnumSymbols was unsuccessful.  Type info may be incomplete\n");
    }

    //
    // Parse local variables and local type information
    //
    std::vector<Function *> funcs;
	associated_symtab->getAllFunctions(funcs);
    for (unsigned i=0; i < funcs.size(); i++) {
        findLocalVars(funcs[i], pair);
    }
}

bool AObject::getSegments(vector<Segment> &segs) const
{
	for(unsigned int i=0; i<regions_.size(); i++){
		Segment seg;
		seg.data = regions_[i]->getPtrToRawData();
		//seg.loadaddr = regions_[i] -> getDiskOffset();
		seg.loadaddr = regions_[i] -> getMemOffset();
		seg.size = regions_[i] -> getDiskSize();
		seg.name = regions_[i] -> getRegionName();
		//seg.segFlags = 
		segs.push_back(seg);
	}
    return true;
}

void Object::getSegmentsSymReader(std::vector<SymSegment> & sym_segs)
{
	for(auto i = regions_.begin();
			i != regions_.end();
			++i)
	{
		SymSegment s;
		s.file_offset = (*i)->getDiskOffset();
		s.file_size = (*i)->getDiskSize();
		s.mem_addr = (*i)->getMemOffset();
		s.mem_size = (*i)->getMemSize();
		s.perms = (*i)->getRegionPermissions();
		s.type = (*i)->getRegionType();
	}
}

bool Object::emitDriver(string fName, std::set<Symbol *> &allSymbols, unsigned flag)
{
	emitWin *em = new emitWin((PCHAR)GetMapAddr(), this, err_func_);
	return em -> driver(associated_symtab, fName);
}

// automatically discards duplicates
void Object::addReference(Offset off, std::string lib, std::string fun){
   ref[lib][off] = fun;
}
						
// retrieve Section Number for an image offset
// dwRO - the image offset to calculate
// returns -1 if an error occurred else returns the corresponding section number
DWORD Object::ImageOffset2SectionNum(DWORD dwRO)
{
   PIMAGE_SECTION_HEADER sectionHeader = 
      (PIMAGE_SECTION_HEADER)((DWORD)peHdr 
                              + sizeof(DWORD) // PE signature
                              + sizeof(IMAGE_FILE_HEADER) 
                              + peHdr->FileHeader.SizeOfOptionalHeader);
	unsigned int SecCount = peHdr ->FileHeader.NumberOfSections;
	for(unsigned int i=0;i < SecCount; i++)
	{
		if((dwRO>=sectionHeader->PointerToRawData) && (dwRO<(sectionHeader->PointerToRawData+sectionHeader->SizeOfRawData)))
		{
			return (i);
		}
		sectionHeader++;
	}
	return(-1);
}

PIMAGE_SECTION_HEADER Object::ImageOffset2Section(DWORD dwRO)
{
   PIMAGE_SECTION_HEADER sectionHeader = 
      (PIMAGE_SECTION_HEADER)((DWORD)peHdr 
                              + sizeof(DWORD) // PE signature
                              + sizeof(IMAGE_FILE_HEADER) 
                              + peHdr->FileHeader.SizeOfOptionalHeader);
	unsigned int SecCount = peHdr ->FileHeader.NumberOfSections;

	for(unsigned int i=0;i<SecCount;i++)
	{
		if((dwRO >= sectionHeader->PointerToRawData) && 
           (dwRO < (sectionHeader->PointerToRawData + sectionHeader->SizeOfRawData)))
		{
			return sectionHeader;
		}
		sectionHeader++;
	}
	return(NULL);
}

PIMAGE_SECTION_HEADER Object::ImageRVA2Section(DWORD dwRVA)
{
   PIMAGE_SECTION_HEADER sectionHeader = 
      (PIMAGE_SECTION_HEADER)((DWORD)peHdr 
                              + sizeof(DWORD) // PE signature
                              + sizeof(IMAGE_FILE_HEADER) 
                              + peHdr->FileHeader.SizeOfOptionalHeader);
	unsigned int SecCount = peHdr ->FileHeader.NumberOfSections;

	for(unsigned int i=0;i<SecCount;i++)
	{
		if((dwRVA>=sectionHeader->VirtualAddress) && (dwRVA<=(sectionHeader->VirtualAddress+sectionHeader->SizeOfRawData)))
		{
			return sectionHeader;
		}
		sectionHeader++;
	}
	return(NULL);
}

DWORD Object::RVA2Offset(DWORD dwRVA)
{
	DWORD offset;
	PIMAGE_SECTION_HEADER section = ImageRVA2Section(dwRVA);
	if(section==NULL)
	{
		return(0);
	}
	offset=dwRVA+section->PointerToRawData-section->VirtualAddress;
	return offset;
}

DWORD Object::Offset2RVA(DWORD dwRO)
{
	PIMAGE_SECTION_HEADER section = ImageOffset2Section(dwRO);
	if(section==NULL)
	{
		return(0);
	}
	return(dwRO+section->VirtualAddress-section->PointerToRawData);
}

void Object::setTrapHeader(Offset addr)
{
   trapHeaderPtr_ = addr;
}
Offset Object::trapHeader()
{
   return trapHeaderPtr_;
}

void Object::insertPrereqLibrary(std::string lib)
{
   // must include some function from the library for Windows to load it
   ref[lib] = std::map<Offset, std::string>();
}

 bool Region::isStandardCode()
{
   return (getRegionPermissions() == RP_RX ||
           getRegionPermissions() == RP_RWX);
}

Dyninst::Architecture Object::getArch() const
{
    switch (peHdr->FileHeader.Machine)
    {
    case IMAGE_FILE_MACHINE_I386:
        return Dyninst::Arch_x86;
    case IMAGE_FILE_MACHINE_AMD64:
        return Dyninst::Arch_x86_64;
    default:
        return Dyninst::Arch_none;
    }
}

/*
	for(it=ref.begin(); it!=ref.end(); it++){
		IMAGE_IMPORT_DESCRIPTOR newID;
		newID.ForwarderChain=0;
		newID.TimeDateStamp=0;
		newID.OriginalFirstThunk = 0;
		newID.FirstThunk = (*it).first;
		//printf("IAT address: %x\n", newID.FirstThunk);

		//look through the old import table to check if the library has been there
		bool isExisting = false;
		for(unsigned int i=0; i<oldImp.size(); i++){

			//if already been there, use the same of RVA of name
			if(strcmp(oldImp[i].name, (*it).second.first.c_str()) == 0){
				isExisting = true;
				newID.Name = oldImp[i].id.Name;
				break;
			}
		}	

		char* ptrLib;
		unsigned long strLen;
		//otherwise, it's a new library
		if(!isExisting){
			newID.Name = strOff;
			strLen =(*it).second.first.size();
			//library name must be '\0' terminated, so len plus one
			ptrLib = (char*) GlobalAlloc(GMEM_FIXED|GMEM_ZEROINIT, strLen+1);
			memcpy(ptrLib,(*it).second.first.c_str(), strLen);
			info.push_back(std::pair<char*,unsigned long> (ptrLib, strLen+1));
			strOff+=(strLen+1);
		}

		memcpy(newIT+pos*sizeof(IMAGE_IMPORT_DESCRIPTOR),(char*)&newID, sizeof(IMAGE_IMPORT_DESCRIPTOR));

		//write the pointer to function name into (*it).first
		Offset o = (Offset)((char*)dynSec->getPtrToRawData())+(*it).first-dynSec->getMemOffset();
		printf("Offset to write the pointer to function name: %x\n", o);
		memcpy(((char*)dynSec->getPtrToRawData())+(*it).first-dynSec->getMemOffset(), (char*)&strOff, 4);
		strLen = (*it).second.second.size();
	
		//functin name must start with a two byte hint
		//function name also '0\' terminated
		ptrLib = (char*)GlobalAlloc(GMEM_FIXED|GMEM_ZEROINIT, 2+strLen+1);
		memcpy(ptrLib+2, (*it).second.second.c_str(), strLen);
		info.push_back(std::pair<char*, unsigned long> (ptrLib, strLen+3));
		strOff+=(2+strLen+1);

		pos++;
	}
*/

DWORD* Object::get_dword_ptr(Offset rva)
{
	Offset off = RVA2Offset(rva);
	int sectionNum = ImageOffset2SectionNum(off);
	Region* r = regions_[sectionNum];
	char* region_buf = (char*)(r->getPtrToRawData());
	return (DWORD*)(region_buf + (off - r->getDiskOffset()));
}

Region* Object::findRegionByName(const std::string& name) const
{
	for(auto reg = regions_.begin();
		reg != regions_.end();
		++reg)
	{
		if((*reg)->getRegionName() == name)
		{
			return *reg;
		}
	}
	return NULL;
}
void Object::applyRelocs(Region* relocs, Offset delta)
{
	unsigned char* section_pointer = (unsigned char*)relocs->getPtrToRawData();
	unsigned char* section_end = section_pointer+relocs->getMemSize();
	while(section_pointer < section_end)
	{
		PIMAGE_BASE_RELOCATION curRelocPage = (PIMAGE_BASE_RELOCATION)(section_pointer);
		section_pointer += sizeof(IMAGE_BASE_RELOCATION);
		Offset pageBase = curRelocPage->VirtualAddress;
		if(pageBase == 0) break;
		int numRelocs = (curRelocPage->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
		for(int i = 0; i < numRelocs; ++i)
		{
			WORD curReloc = *(WORD*)(section_pointer);
			section_pointer += sizeof(WORD);
			Offset addr = (curReloc & 0x0FFF) + pageBase;
			WORD type = curReloc >> 12;
			switch(type)
			{
			case IMAGE_REL_BASED_ABSOLUTE:
				// These are placeholders only
				break;
			case IMAGE_REL_BASED_HIGHLOW:
				{
					// These should be the only things we deal with on Win32; revisit when we hit 64-bit windows
					DWORD* loc_to_fix = get_dword_ptr(addr);
					*(loc_to_fix) += delta;
				}
				break;
			default:
				fprintf(stderr, "Unknown relocation type 0x%x for addr %lx\n", type, addr);
				break;
			}
		}
	}
}
void Object::rebase(Offset off)
{
	if(off == imageBase) return;
	Region* relocs = findRegionByName(".reloc");
	if(!relocs) {
		fprintf(stderr, "rebase found no .reloc section, bailing\n");
		return;
	}
	Offset delta = off - imageBase;
	applyRelocs(relocs, delta);
	imageBase = off;
}
