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

/************************************************************************
 * Windows NT/2000 object files.
 * $Id: Object-nt.h,v 1.33 2006/04/04 01:10:18 legendre Exp $
************************************************************************/




#if !defined(_Object_nt_h_)
#define _Object_nt_h_



/************************************************************************
 * header files.
************************************************************************/

#include "common/h/Dictionary.h"
#include "common/h/String.h"
#include "common/h/Symbol.h"
#include "common/h/Types.h"
#include "common/h/Vector.h"

#include <stdlib.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

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
		pdstring name;
		DWORD64 addr;
		DWORD type;
		DWORD linkage;
		DWORD size;

	public:
		Symbol( pdstring _name,
              DWORD64 _addr,
              DWORD _type,
              DWORD _linkage,
              DWORD _size)
		  : name(_name),
			addr(_addr),
			type(_type),
			size(_size)
		{}

        pdstring GetName( void ) const          { return name; }
        DWORD64 GetAddr( void ) const           { return addr; }
        DWORD	GetSize( void ) const				{ return size; }
        DWORD	GetType( void ) const				{ return type; }
        DWORD	GetLinkage( void ) const			{ return linkage; }

        void	SetSize( DWORD cb )					{ size = cb; }

        void DefineSymbol( dictionary_hash<pdstring, pdvector< ::Symbol > >& syms,
                            const pdstring& modName ) const;
	};

	class File
	{
	private:
		pdstring name;
		pdvector<Symbol*> syms;

	public:
		File( pdstring _name = "" )
		  : name(_name)
		{}

		void AddSymbol( Symbol* pSym )
		{
			syms.push_back( pSym );
		}

      void DefineSymbols( dictionary_hash<pdstring, pdvector< ::Symbol > >& syms,
                          const pdstring& modName ) const;
      pdstring GetName( void ) const		{ return name; }
		const pdvector<Symbol*>& GetSymbols( void )	const		{ return syms; }
	};

	class Module
	{
	private:
		pdstring name;
		DWORD64 baseAddr;
		DWORD64 extent;
      bool isDll;

		pdvector<File*> files;
		File* defFile;

		void PatchSymbolSizes( const Object* obj,
							   const pdvector<Symbol*>& allSyms ) const;

	public:
		Module( pdstring name,
				DWORD64 baseAddr,
				DWORD64 extent = 0 );

		File* GetDefaultFile( void )			{ return defFile; }
		File* FindFile( pdstring name );
		void AddFile( File* pFile )				{ files.push_back( pFile ); }

        void DefineSymbols( const Object* obj,
                            dictionary_hash<pdstring, pdvector< ::Symbol> > & syms ) const;
		void BuildSymbolMap( const Object* obj ) const; 

      pdstring GetName( void ) const            { return name; }
      bool IsDll( void ) const                { return isDll; }
      void SetIsDll( bool v )                 { isDll = v; }
	};

private:
	Module* curModule;

public:
    Object(fileDescriptor &desc,
           void (*)(const char *) = log_msg);

    virtual ~Object( void );

    bool isForwarded( Address addr );
    bool isEEL() const { return false; }
    bool isText( const Address& addr ) const; 
    Address get_base_addr() const { return baseAddr;} 
    const fileDescriptor& GetDescriptor( void ) const     { return desc; }
    Module* GetCurrentModule( void )				    { return curModule; }
   
    bool getCatchBlock(ExceptionBlock &b, Address addr, unsigned size = 0) const;
    unsigned int GetTextSectionId( void ) const         { return textSectionId;}
    PIMAGE_NT_HEADERS   GetImageHeader( void ) const    { return peHdr; }
    PVOID GetMapAddr( void ) const                      { return mapAddr; }
    Address getEntryPoint( void ) const                 { return peHdr->OptionalHeader.AddressOfEntryPoint + 
                                                                 desc.loadAddr(); }
   
    void    ParseGlobalSymbol(PSYMBOL_INFO pSymInfo);
    const pdvector<Address> &getPossibleMains() const   { return possible_mains; }
private:
    void    ParseDebugInfo( void );
    void    FindInterestingSections( HANDLE hProc, HANDLE hFile );

	Address	baseAddr;					// location of this object in 
								// mutatee address space

    PIMAGE_NT_HEADERS   peHdr;      // PE file headers
	PIMAGE_OPTIONAL_HEADER optHdr;

	unsigned int textSectionId;		// id of .text segment (section)
	unsigned int dataSectionId;		// id of .data segment (section)
   
    HANDLE  hMap;                   // handle to mapping object
    LPVOID  mapAddr;                // location of mapped file in *our* address space
    fileDescriptor desc;
    pdvector<Address> possible_mains; //Addresses of functions that may be main
};



inline
Object::Object(fileDescriptor &_desc,
                void (*err_func)(const char *)) 
    : AObject(_desc.file(), err_func),
     baseAddr(_desc.code()),
     hMap( INVALID_HANDLE_VALUE ),
     mapAddr( NULL ),
     desc( _desc ),
     curModule( NULL ),
     peHdr( NULL )
{
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



#endif /* !defined(_Object_nt_h_) */
