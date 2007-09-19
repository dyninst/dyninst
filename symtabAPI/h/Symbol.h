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
 * $Id: Symbol.h,v 1.1 2007/09/19 21:53:48 giri Exp $
 * Symbol.h: symbol table objects.
************************************************************************/

#if !defined(_Symbol_h_)
#define _Symbol_h_


/************************************************************************
 * header files.
************************************************************************/

#include <string>
#include <vector>
#include <algorithm>
#include <iostream>

#include "util.h"
#include "Type.h"

namespace Dyninst{
namespace SymtabAPI{

class Section;
class Module;
class typeCommon;
class localVarCollection;


/************************************************************************
 * class Symbol
************************************************************************/

class Symbol {
    friend class typeCommon;
    friend std::string parseStabString(Module *, int linenum, char *, int, 
                              typeCommon *);
  #if defined(USES_DWARF_DEBUG)

       friend bool walkDwarvenTree(Dwarf_Debug &, Dwarf_Die,
                                   Module *,  Symtab *, Dwarf_Off, char **, 
                                   Symbol *,  typeCommon *, typeEnum *, fieldListType *);
  #endif
									
public:
    enum SymbolType {
        ST_UNKNOWN,
        ST_FUNCTION,
        ST_OBJECT,
        ST_MODULE,
	ST_NOTYPE
    };

    enum SymbolLinkage {
        SL_UNKNOWN,
        SL_GLOBAL,
        SL_LOCAL,
	SL_WEAK
    };

    enum SymbolTag {
        TAG_UNKNOWN,
        TAG_USER,
        TAG_LIBRARY,
        TAG_INTERNAL
    };

    DLLEXPORT Symbol (); // note: this ctor is called surprisingly often!
    DLLEXPORT Symbol (unsigned);
    DLLEXPORT Symbol (const std::string name,const std::string modulename, SymbolType, SymbolLinkage,
             Offset, Section *sec = NULL, unsigned size = 0, void *upPtr = NULL);
    DLLEXPORT Symbol (const std::string name,Module *module, SymbolType, SymbolLinkage,
             Offset, Section *sec = NULL, unsigned size = 0, void *upPtr = NULL);
    DLLEXPORT Symbol (const Symbol &);
    DLLEXPORT ~Symbol();

    DLLEXPORT Symbol&        operator= (const Symbol &);
    DLLEXPORT bool          operator== (const Symbol &) const;

	DLLEXPORT const std::string&     getModuleName ()        const;
    DLLEXPORT Module*	getModule()		const; 
    DLLEXPORT SymbolType        getType ()              const;
    DLLEXPORT SymbolLinkage     getLinkage ()           const;
    DLLEXPORT Offset            getAddr ()              const;
    DLLEXPORT Section		*getSec ()      	const;
    DLLEXPORT void 		*getUpPtr()		const;
    
    
    /***********************************************************
    	Name Output Functions
    ***********************************************************/		
    DLLEXPORT const std::string&      getName ()              const;
    DLLEXPORT const std::string&	getPrettyName()       	const;
    DLLEXPORT const std::string& 	getTypedName() 		const;
    
    DLLEXPORT const std::vector<std::string>&	getAllMangledNames()    const;
    DLLEXPORT const std::vector<std::string>&	getAllPrettyNames()     const;
    DLLEXPORT const std::vector<std::string>&	getAllTypedNames()      const;

    DLLEXPORT bool setAddr (Offset newAddr);

    DLLEXPORT bool setSymbolType(SymbolType sType);
   
    DLLEXPORT unsigned            getSize ()               const;
    DLLEXPORT SymbolTag            tag ()               const;
    DLLEXPORT bool	setSize(unsigned ns);
    DLLEXPORT bool	setModuleName(std::string module);
    DLLEXPORT bool 	setModule(Module *mod);
    DLLEXPORT bool	setUpPtr(void *newUpPtr);

    DLLEXPORT bool	setReturnType(Type *);
    
    // Bool: returns true if the name is new (and therefore added)
    DLLEXPORT bool addMangledName(std::string name, bool isPrimary = false);
    DLLEXPORT bool addPrettyName(std::string name, bool isPrimary = false);
    DLLEXPORT bool addTypedName(std::string name, bool isPrimary = false);

    /***** Local Variable Information *****/
    DLLEXPORT bool findLocalVariable(std::vector<localVar *>&vars, std::string name);
    DLLEXPORT bool getLocalVariables(std::vector<localVar *>&vars);
    DLLEXPORT bool getParams(std::vector<localVar *>&params);
    
    friend
    std::ostream& operator<< (std::ostream &os, const Symbol &s);

public:
    static std::string emptyString;
    
private:
    Module*   module_;
    SymbolType    type_;
    SymbolLinkage linkage_;
    Offset       addr_;
    Section*  sec_;
    unsigned      size_;  // size of this symbol. This is NOT available on
                          // all platforms.
    void*         upPtr_;
    std::vector<std::string> mangledNames;
    std::vector<std::string> prettyNames;
    std::vector<std::string> typedNames;
    SymbolTag     tag_;

    Type *retType_;
public:
    localVarCollection *vars_;
    localVarCollection *params_;
};

inline
Symbol::Symbol(unsigned)
   : //name_("*bad-symbol*"), module_("*bad-module*"),
    module_(NULL), type_(ST_UNKNOWN), linkage_(SL_UNKNOWN), addr_(0), sec_(NULL), size_(0), upPtr_(NULL),
    tag_(TAG_UNKNOWN), retType_(NULL), vars_(NULL), params_(NULL) {
}

inline
bool
Symbol::operator==(const Symbol& s) const {
    // explicitly ignore tags when comparing symbols
    return ((module_  == s.module_)
        && (type_    == s.type_)
        && (linkage_ == s.linkage_)
        && (addr_    == s.addr_)
	&& (sec_     == s.sec_)
	&& (size_    == s.size_)
	&& (upPtr_    == s.upPtr_)
	&& (retType_    == s.retType_)
    	&& (mangledNames == s.mangledNames)
    	&& (prettyNames == s.prettyNames)
    	&& (typedNames == s.typedNames));
}

}//namespace SymtabAPI
}//namespace Dyninst

#endif /* !defined(_Symbol_h_) */
