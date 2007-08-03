/*
 * Copyright (c) 1996-2007 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

/************************************************************************
 * Windows NT/2000 object files.
 * $Id: Object-nt.h,v 1.6 2007/08/03 16:19:25 giri Exp $
************************************************************************/




#if !defined(_Object_nt_h_)
#define _Object_nt_h_



/************************************************************************
 * header files.
************************************************************************/

#include "common/h/Types.h"
#include <vector>
#include <string>
#include <algorithm>
#include "symtabAPI/h/util.h"

#include <stdio.h>
#include <stdlib.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <assert.h>

#if !defined(__out_ecount_opt)
#define __out_ecount_opt(UNUSED)
#endif
#include <dbghelp.h>

class ExceptionBlock;

/************************************************************************
 * class Object
************************************************************************/

class Object : public AObject
{
public:
	class Symbol
	{
	private:
		string name;
		DWORD64 addr;
		DWORD type;
		DWORD linkage;
		DWORD size;

	public:
		Symbol( string _name,
              DWORD64 _addr,
              DWORD _type,
              DWORD _linkage,
			  DWORD _size)
		  : name(_name),
			addr(_addr),
			type(_type),
			size(_size)
		{}

        string GetName( void ) const          { return name; }
        DWORD64 GetAddr( void ) const           { return addr; }
        DWORD	GetSize( void ) const				{ return size; }
        DWORD	GetType( void ) const				{ return type; }
        DWORD	GetLinkage( void ) const			{ return linkage; }

        void	SetSize( DWORD cb )					{ size = cb; }

        void DefineSymbol( hash_map<string, vector< ::Dyn_Symbol *> >& syms,
                            const string& modName ) const;
	};

	class File
	{
	private:
		string name;
		vector<Symbol*> syms;

	public:
		File( string _name = "" )
		  : name(_name)
		{}

		void AddSymbol( Symbol* pSym )
		{
			syms.push_back( pSym );
		}

      void DefineSymbols( hash_map<string, vector< ::Dyn_Symbol *> >& syms,
                          const string& modName ) const;
      string GetName( void ) const		{ return name; }
		const vector<Symbol*>& GetSymbols( void )	const		{ return syms; }
	};

	class Module
	{
	private:
		string name;
		DWORD64 baseAddr;
		DWORD64 extent;
      bool isDll;

		vector<File *> files;
		File* defFile;

		void PatchSymbolSizes( const Object* obj,
							   const vector<Symbol*>& allSyms ) const;

	public:
		Module( string name,
				DWORD64 baseAddr,
				DWORD64 extent = 0 );

		File* GetDefaultFile( void )			{ return defFile; }
		File* FindFile( string name );
		void AddFile( File* pFile )				{ files.push_back( pFile ); }

        void DefineSymbols( const Object* obj,
                            hash_map<string, vector< ::Dyn_Symbol *> > & syms ) const;
		void BuildSymbolMap( const Object* obj ) const; 

      string GetName( void ) const            { return name; }
      bool IsDll( void ) const                { return isDll; }
      void SetIsDll( bool v )                 { isDll = v; }
	};

private:
	Module* curModule;

public:
    DLLEXPORT Object(string &filename, void (*)(const char *) = log_msg);
    DLLEXPORT Object(char *mem_image, size_t image_size, void (*)(const char *) = log_msg);
    DLLEXPORT Object(){};
  
    DLLEXPORT virtual ~Object( void );

    DLLEXPORT bool isForwarded( OFFSET addr );
    DLLEXPORT bool isEEL() const { return false; }
    DLLEXPORT bool isText( const OFFSET& addr ) const; 
    DLLEXPORT OFFSET get_base_addr() const { return (OFFSET)mapAddr;} 
    DLLEXPORT Module* GetCurrentModule( void )				    { return curModule; }
   
    DLLEXPORT bool getCatchBlock(ExceptionBlock &b, OFFSET addr, unsigned size = 0) const;
    DLLEXPORT unsigned int GetTextSectionId( void ) const         { return textSectionId;}
    DLLEXPORT PIMAGE_NT_HEADERS   GetImageHeader( void ) const    { return peHdr; }
    DLLEXPORT PVOID GetMapAddr( void ) const                      { return mapAddr; }
	DLLEXPORT OFFSET getEntryPoint( void ) const                { return peHdr->OptionalHeader.AddressOfEntryPoint; }
													//+ desc.loadAddr(); } //laodAddr is always zero in our fake address space.
	// TODO. Change these later.
	DLLEXPORT OFFSET getLoadAddress() const { return get_base_addr(); }
	DLLEXPORT OFFSET getEntryAddress() const { return getEntryPoint(); }
	DLLEXPORT OFFSET getBaseAddress() const { return get_base_addr(); }
	DLLEXPORT OFFSET getTOCoffset() const { return 0; }
   DLLEXPORT ObjectType objType() const;
   DLLEXPORT const char *interpreter_name() const { return NULL; }
   
    DLLEXPORT void    ParseGlobalSymbol(PSYMBOL_INFO pSymInfo);
    DLLEXPORT const vector<OFFSET> &getPossibleMains() const   { return possible_mains; }
    DLLEXPORT void getModuleLanguageInfo(hash_map<string, supportedLanguages> *mod_langs);

private:
    DLLEXPORT void    ParseDebugInfo( void );
    DLLEXPORT void    FindInterestingSections();

	OFFSET	baseAddr;					// location of this object in 
								// mutatee address space

    PIMAGE_NT_HEADERS   peHdr;      // PE file headers
    PIMAGE_OPTIONAL_HEADER optHdr;

	unsigned int textSectionId;		// id of .text segment (section)
	unsigned int dataSectionId;		// id of .data segment (section)
   
    HANDLE  hMap;                   // handle to mapping object
	HANDLE  hFile;					// File Handle
	HANDLE  hProc;					// Process Handle
    LPVOID  mapAddr;                // location of mapped file in *our* address space
    vector<OFFSET> possible_mains; //Addresses of functions that may be main
	string filename;				//Name of the file.
};


inline
Object::Object(char *mem_image, size_t image_size,
                void (*err_func)(const char *)) 
    : AObject(NULL, err_func),
     //baseAddr(_desc.code()),
     hMap( INVALID_HANDLE_VALUE ),
     mapAddr( NULL ),
     curModule( NULL ),
     peHdr( NULL )
{
    hFile = LocalHandle( mem_image );	//For a mem image
    assert( hFile != NULL );
    assert( hFile != INVALID_HANDLE_VALUE );
    ParseDebugInfo();
}

// In recent versions of the Platform SDK, the macros naming
// the value for the Flags field of the SYMBOL_INFO struct have
// names with a SYMFLAG_ prefix.  Older Platform SDKs, including
// the version that shipped with the Visual Studio .NET product
// (i.e., VC7), use names for these macros with a SYMF_ prefix.
// If we find we are using these older headers, we define the
// new-style names.
#if !defined(SYMFLAG_FUNCTION)
#  define SYMFLAG_FUNCTION      SYMF_FUNCTION
#  define SYMFLAG_LOCAL         SYMF_LOCAL
#  define SYMFLAG_PARAMETER     SYMF_PARAMETER
#  define SYMFLAG_EXPORT        SYMF_EXPORT
#endif // !defined(SYMFLAG_FUNCTION)


const char WILDCARD_CHAR = '?';
const char MULTIPLE_WILDCARD_CHAR = '*';
std::string extract_pathname_tail(const std::string &path);


#endif /* !defined(_Object_nt_h_) */
