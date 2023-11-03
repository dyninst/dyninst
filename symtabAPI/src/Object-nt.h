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

/************************************************************************
 * Windows NT/2000 object files.
 ************************************************************************/




#if !defined(_Object_nt_h_)
#define _Object_nt_h_



/************************************************************************
 * header files.
************************************************************************/

#include <map>
#include <set>
#include <utility>
#include <vector>
#include <string>
#include <algorithm>
#include "symtabAPI/h/symutil.h"

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <assert.h>

#if !defined(__out_ecount_opt)
#define __out_ecount_opt(UNUSED)
#endif
#include <dbghelp.h>

namespace Dyninst{
namespace SymtabAPI{

class ExceptionBlock;

/************************************************************************
 * class Object
************************************************************************/

class Object : public AObject
{
	friend class Symtab;
 public:
    class intSymbol
	{
	private:
            std::string name;
            DWORD64 addr;
            DWORD type;
            DWORD linkage;
            DWORD size;
            Region *region;

	public:
            intSymbol( std::string _name,
                       DWORD64 _addr,
                       DWORD _type,
                       DWORD _linkage,
                       DWORD _size,
                       Region *_region)
                : name(_name),
                addr(_addr),
                type(_type),
                linkage(_linkage),
                size(_size),
                region(_region)
		{}

            std::string GetName( void ) const          { return name; }
            DWORD64 GetAddr( void ) const           { return addr; }
            DWORD	GetSize( void ) const				{ return size; }
            DWORD	GetType( void ) const				{ return type; }
            DWORD	GetLinkage( void ) const			{ return linkage; }
            Region *GetRegion( void ) const        { return region; }
            void	SetSize( DWORD cb )					{ size = cb; }

            void DefineSymbol( dyn_hash_map<std::string, std::vector< Symbol *> >& syms,
                               std::map<Symbol *, std::string> &symsToMods,
                               const std::string& modName ) const;
	};

    class File
	{
	private:
            std::string name;
            std::vector<intSymbol*> syms;

	public:
            File( std::string _name = "" )
                : name(_name)
		{}

            void AddSymbol( intSymbol* pSym )
		{
                    syms.push_back( pSym );
		}

            void DefineSymbols( dyn_hash_map<std::string, std::vector< Symbol *> >& syms,
                                std::map<Symbol *, std::string> &symsToMods,
                                const std::string& modName ) const;
            std::string GetName( void ) const		{ return name; }
            const std::vector<intSymbol*>& GetSymbols( void )	const		{ return syms; }
	};

    class Module
	{
	private:
            std::string name;
            DWORD64 baseAddr;
            DWORD64 extent;
            bool isDll;

            std::vector<File *> files;
            File* defFile;

            void PatchSymbolSizes( const Object* obj,
                                   const std::vector<intSymbol*>& allSyms ) const;

	public:
            Module( std::string name,
                    DWORD64 baseAddr,
                    DWORD64 extent = 0 );

            File* GetDefaultFile( void )			{ return defFile; }
            File* FindFile( std::string name );
            void AddFile( File* pFile )				{ files.push_back( pFile ); }

            void DefineSymbols( const Object* obj,
                                dyn_hash_map<std::string, std::vector< Symbol *> > & syms,
                                std::map<Symbol *, std::string> &symsToMods ) const;
            void BuildSymbolMap( const Object* obj ) const; 

            std::string GetName( void ) const            { return name; }
            bool IsDll( void ) const                { return isDll; }
            void SetIsDll( bool v )                 { isDll = v; }
	};

 private:
    Module* curModule;

    // declared but not implemented; no copying allowed
    Object(const Object &);
    const Object& operator=(const Object &);

 public:
    SYMTAB_EXPORT Object(MappedFile *, bool defensive, 
                         void (*)(const char *) = log_msg, bool alloc_syms = true, Symtab* st = NULL);
  
    SYMTAB_EXPORT virtual ~Object( void );
	SYMTAB_EXPORT std::string getFileName() const { return mf->filename(); }
    SYMTAB_EXPORT bool isForwarded( Offset addr );
    SYMTAB_EXPORT bool isEEL() const { return false; }
    SYMTAB_EXPORT bool isText( const Offset addr ) const; 
    SYMTAB_EXPORT Offset get_base_addr() const { return (Offset)mf->base_addr();} 
    SYMTAB_EXPORT Module* GetCurrentModule( void )				    { return curModule; }
   
    SYMTAB_EXPORT bool getCatchBlock(ExceptionBlock &b, Offset addr, unsigned size = 0) const;
    SYMTAB_EXPORT unsigned int GetTextSectionId( void ) const         { return textSectionId;}
    SYMTAB_EXPORT PIMAGE_NT_HEADERS   GetImageHeader( void ) const    { return peHdr; }
    SYMTAB_EXPORT PVOID GetMapAddr( void ) const                      { return mf->base_addr(); }
    SYMTAB_EXPORT Offset getEntryPoint( void ) const                {
		if (peHdr) return peHdr->OptionalHeader.AddressOfEntryPoint;
		return 0;}
    //+ desc.loadAddr(); } //laodAddr is always zero in our fake address space.
    // TODO. Change these later.
    SYMTAB_EXPORT Offset getLoadAddress() const { return imageBase; }
	SYMTAB_EXPORT Offset getPreferedBase() const { return preferedBase; }
    SYMTAB_EXPORT Offset getEntryAddress() const { return getEntryPoint(); }
    SYMTAB_EXPORT Offset getBaseAddress() const { return get_base_addr(); }
    SYMTAB_EXPORT Offset getTOCoffset(Offset /*ignored*/) const { return 0; }
    SYMTAB_EXPORT ObjectType objType() const;
    SYMTAB_EXPORT const char *interpreter_name() const { return NULL; }
    SYMTAB_EXPORT dyn_hash_map <std::string, LineInformation> &getLineInfo();
    SYMTAB_EXPORT void parseTypeInfo();
    SYMTAB_EXPORT virtual Dyninst::Architecture getArch() const;
    SYMTAB_EXPORT void    ParseGlobalSymbol(PSYMBOL_INFO pSymInfo);
    SYMTAB_EXPORT const std::vector<Offset> &getPossibleMains() const   { return possible_mains; }
    SYMTAB_EXPORT void getModuleLanguageInfo(dyn_hash_map<std::string, supportedLanguages> *mod_langs);
    SYMTAB_EXPORT bool emitDriver(std::string fName, std::set<Symbol*> &allSymbols, unsigned flag);
    SYMTAB_EXPORT unsigned int getSecAlign() const {return SecAlignment;}
    SYMTAB_EXPORT void insertPrereqLibrary(std::string lib);
    virtual char *mem_image() const 
    {
        assert(mf);
        return (char *)mf->base_addr();
    }
    void setTrapHeader(Offset ptr);
    Offset trapHeader();

    SYMTAB_EXPORT DWORD ImageOffset2SectionNum(DWORD dwRO);
    SYMTAB_EXPORT PIMAGE_SECTION_HEADER ImageOffset2Section(DWORD dwRO);
    SYMTAB_EXPORT PIMAGE_SECTION_HEADER ImageRVA2Section(DWORD dwRVA);
    SYMTAB_EXPORT DWORD RVA2Offset(DWORD dwRVA);
    SYMTAB_EXPORT DWORD Offset2RVA(DWORD dwRO);
    SYMTAB_EXPORT void addReference(Offset, std::string, std::string);
    SYMTAB_EXPORT std::map<std::string, std::map<Offset, std::string> > & getRefs() { return ref; }

    std::vector<std::pair<std::string, IMAGE_IMPORT_DESCRIPTOR> > & getImportDescriptorTable();
    std::map<std::string, std::map<std::string, WORD> > & getHintNameTable();
    PIMAGE_NT_HEADERS getPEHdr() { return peHdr; }
	void setTOCoffset(Offset) {};
	// Adjusts the data in all the sections to reflect what
	// the loader will do if the binary is loaded at actualBaseAddress
	SYMTAB_EXPORT void rebase(Offset off);
	SYMTAB_EXPORT Region* findRegionByName(const std::string& name) const;
	SYMTAB_EXPORT void applyRelocs(Region* relocs, Offset delta);
	SYMTAB_EXPORT virtual void getSegmentsSymReader(std::vector<SymSegment> &);

private:
    SYMTAB_EXPORT void    ParseSymbolInfo( bool );
    SYMTAB_EXPORT void parseFileLineInfo();
    SYMTAB_EXPORT void parseLineInfoForAddr(Offset)
    {
      parseFileLineInfo();
    }
    
    SYMTAB_EXPORT void    FindInterestingSections( bool, bool );
    Region *          findEnclosingRegion(const Offset where);
    void AddTLSFunctions();
	DWORD* get_dword_ptr(Offset rva);
    Offset baseAddr;     // location of this object in mutatee address space

	Offset preferedBase; // Virtual address at which the binary is prefered to be loaded
    Offset imageBase; // Virtual Address at which the binary is loaded in its address space

    PIMAGE_NT_HEADERS   peHdr;      // PE file headers
    Offset trapHeaderPtr_; // address & size
	unsigned int SecAlignment; //Section Alignment

	//structure of import table
    std::vector<std::pair<std::string, IMAGE_IMPORT_DESCRIPTOR> > idt_;
    std::map<std::string, std::map<std::string, WORD> > hnt_;

	//external reference info
   std::map<std::string,std::map<Offset, std::string> > ref;

	unsigned int textSectionId;		// id of .text segment (section)
	unsigned int dataSectionId;		// id of .data segment (section)
   
	HANDLE  hProc;					// Process Handle
    std::vector<Offset> possible_mains; //Addresses of functions that may be main
};

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

}//namespace Dyninst
}//namespace SymtabAPI

#endif /* !defined(_Object_nt_h_) */
