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
 * $Id: Dyn_Symbol.h,v 1.8 2007/07/02 22:17:56 legendre Exp $
 * Symbol.h: symbol table objects.
************************************************************************/

#if !defined(_Dyn_Symbol_h_)
#define _Dyn_Symbol_h_


/************************************************************************
 * header files.
************************************************************************/

#include <string>
#include <vector>
#include <algorithm>
#include <iostream>

#include "util.h"

class Dyn_Section;
class Dyn_Module;


/************************************************************************
 * class Symbol
************************************************************************/

class Dyn_Symbol {
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

    DLLEXPORT Dyn_Symbol (); // note: this ctor is called surprisingly often!
    DLLEXPORT Dyn_Symbol (unsigned);
	DLLEXPORT Dyn_Symbol (const string &name,const string &modulename, SymbolType, SymbolLinkage,
             Address, Dyn_Section *sec = NULL, unsigned size = 0, void *upPtr = NULL);
	DLLEXPORT Dyn_Symbol (const string &name,Dyn_Module *module, SymbolType, SymbolLinkage,
             Address, Dyn_Section *sec = NULL, unsigned size = 0, void *upPtr = NULL);
    DLLEXPORT Dyn_Symbol (const Dyn_Symbol &);
    DLLEXPORT ~Dyn_Symbol();

    DLLEXPORT Dyn_Symbol&        operator= (const Dyn_Symbol &);
    DLLEXPORT bool          operator== (const Dyn_Symbol &) const;

	DLLEXPORT const string&     getModuleName ()        const;
    DLLEXPORT Dyn_Module*	getModule()		const; 
    DLLEXPORT SymbolType        getType ()              const;
    DLLEXPORT SymbolLinkage     getLinkage ()           const;
    DLLEXPORT Address           getAddr ()              const;
    DLLEXPORT Dyn_Section	*getSec ()      	const;
    DLLEXPORT void 		*getUpPtr()		const;
    
    
    /***********************************************************
    	Name Output Functions
    ***********************************************************/		
	DLLEXPORT const string&      getName ()              const;
	DLLEXPORT const string&	getPrettyName()       	const;
	DLLEXPORT const string& 	getTypedName() 		const;
    
	DLLEXPORT const vector<string>&	getAllMangledNames()    const;
	DLLEXPORT const vector<string>&	getAllPrettyNames()     const;
	DLLEXPORT const vector<string>&	getAllTypedNames()      const;

   DLLEXPORT void setAddr (Address newAddr);

   DLLEXPORT bool setType(SymbolType sType);
   
    DLLEXPORT unsigned            getSize ()               const;
    DLLEXPORT SymbolTag            tag ()               const;
    DLLEXPORT void	setSize(unsigned ns);
	DLLEXPORT void	setModuleName(string module);
    DLLEXPORT void 	setModule(Dyn_Module *mod);
    DLLEXPORT void	setUpPtr(void *newUpPtr);
    
    // Bool: returns true if the name is new (and therefore added)
	DLLEXPORT bool addMangledName(string name, bool isPrimary = false);
	DLLEXPORT bool addPrettyName(string name, bool isPrimary = false);
	DLLEXPORT bool addTypedName(string name, bool isPrimary = false);
    
    friend
    ostream&      operator<< (ostream &os, const Dyn_Symbol &s) {
      return os << "{"
		<< " mangled=" << s.getName() 
		<< " pretty="  << s.getPrettyName()
		<< " module="  << s.module_
		<< " type="    << (unsigned) s.type_
		<< " linkage=" << (unsigned) s.linkage_
		<< " addr="    << s.addr_
		<< " tag="     << (unsigned) s.tag_
		<< " }" << endl;
    }

public:
	static string emptyString;
    
private:
    Dyn_Module*   module_;
    SymbolType    type_;
    SymbolLinkage linkage_;
    Address       addr_;
    Dyn_Section*  sec_;
    unsigned      size_;  // size of this symbol. This is NOT available on
                          // all platforms.
    void*         upPtr_;
	vector<string> mangledNames;
	vector<string> prettyNames;
	vector<string> typedNames;
    SymbolTag     tag_;
};

inline
Dyn_Symbol::Dyn_Symbol(unsigned)
   : //name_("*bad-symbol*"), module_("*bad-module*"),
    module_(NULL), type_(ST_UNKNOWN), linkage_(SL_UNKNOWN), addr_(0), sec_(NULL), size_(0), upPtr_(NULL),
    tag_(TAG_UNKNOWN) {
}

inline
bool
Dyn_Symbol::operator==(const Dyn_Symbol& s) const {
    // explicitly ignore tags when comparing symbols
    return ((module_  == s.module_)
        && (type_    == s.type_)
        && (linkage_ == s.linkage_)
        && (addr_    == s.addr_)
	&& (sec_     == s.sec_)
	&& (size_    == s.size_)
	&& (upPtr_    == s.upPtr_)
    	&& (mangledNames == s.mangledNames)
    	&& (prettyNames == s.prettyNames)
    	&& (typedNames == s.typedNames));
}


#endif /* !defined(_Dyn_Symbol_h_) */
