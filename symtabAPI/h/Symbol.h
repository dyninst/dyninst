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
 * $Id: Symbol.h,v 1.15 2008/06/19 19:53:59 legendre Exp $
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

#include "symutil.h"
#include "Type.h"

#include "Annotatable.h"
#ifndef CASE_RETURN_STR
#define CASE_RETURN_STR(x) case x: return #x
#endif


typedef struct {} symbol_file_name_a;
typedef struct {} symbol_version_names_a;
typedef struct {} symbol_variables_a;
typedef struct {} symbol_parameters_a;

namespace Dyninst{
namespace SymtabAPI{

class Region;
class Module;
class typeCommon;
class localVarCollection;


/************************************************************************
 * class Symbol
************************************************************************/

class Symbol : public Annotatable <std::string, symbol_file_name_a, false>,
               public Annotatable <std::vector<std::string>, symbol_version_names_a, false>,
               public Annotatable <localVarCollection, symbol_variables_a, true>,
               public Annotatable <localVarCollection, symbol_parameters_a, true> {
    friend class typeCommon;
    friend class Symtab;
    friend class SymtabTranslatorBase;
    friend class SymtabTranslatorBin;

    friend std::string parseStabString(Module *, int linenum, char *, int, 
                              typeCommon *);
public:
    enum SymbolType {
        ST_UNKNOWN,
        ST_FUNCTION,
        ST_OBJECT,
        ST_MODULE,
        ST_NOTYPE
    };

    static const char *symbolType2Str(SymbolType t);

    enum SymbolLinkage {
       SL_UNKNOWN,
       SL_GLOBAL,
       SL_LOCAL,
       SL_WEAK
    };

    static const char *symbolLinkage2Str(SymbolLinkage t);

    enum SymbolTag {
       TAG_UNKNOWN,
       TAG_USER,
       TAG_LIBRARY,
       TAG_INTERNAL
    };

    static const char *symbolTag2Str(SymbolTag t);

    DLLEXPORT Symbol (); // note: this ctor is called surprisingly often!
    DLLEXPORT Symbol (unsigned);
    DLLEXPORT Symbol (const std::string name,const std::string modulename, SymbolType, SymbolLinkage,
          Offset, Region *sec = NULL, unsigned size = 0, bool isInDynsymtab_ = false, 
          bool isInSymtab_ = true);
    DLLEXPORT Symbol (const std::string name,Module *module, SymbolType, SymbolLinkage,
          Offset, Region *sec = NULL, unsigned size = 0, bool isInDynsymtab_ = false,
          bool isInSymtab_ = true);
#if 0
    DLLEXPORT Symbol (const std::string name,const std::string modulename, SymbolType, SymbolLinkage,
          Offset, Region *sec = NULL, unsigned size = 0, void *upPtr = NULL, bool isInDynsymtab_ = false, 
          bool isInSymtab_ = true);
    DLLEXPORT Symbol (const std::string name,Module *module, SymbolType, SymbolLinkage,
          Offset, Region *sec = NULL, unsigned size = 0, void *upPtr = NULL, bool isInDynsymtab_ = false,
          bool isInSymtab_ = true);
#endif
    DLLEXPORT Symbol (const Symbol &);
    DLLEXPORT ~Symbol();

    DLLEXPORT Symbol&        operator= (const Symbol &);
    DLLEXPORT bool          operator== (const Symbol &) const;

	DLLEXPORT const std::string&getModuleName ()        const;
    DLLEXPORT Module*	        getModule()		        const; 
    DLLEXPORT SymbolType        getType ()              const;
    DLLEXPORT SymbolLinkage     getLinkage ()           const;
    DLLEXPORT Offset            getAddr ()              const;
    DLLEXPORT Region		    *getSec ()      	    const;
    DLLEXPORT bool              isInDynSymtab()         const;
    DLLEXPORT bool              isInSymtab()            const;
    
    
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
#if 0
    DLLEXPORT bool	setUpPtr(void *newUpPtr);
    DLLEXPORT void 		        *getUpPtr()		        const;
#endif
    DLLEXPORT bool  setDynSymtab();
    DLLEXPORT bool  clearDynSymtab();
    DLLEXPORT bool  setIsInSymtab();
    DLLEXPORT bool  clearIsInSymtab();

    DLLEXPORT Type  *getReturnType();
    DLLEXPORT bool	setReturnType(Type *);
    DLLEXPORT bool  setFramePtrRegnum(int regnum);
    DLLEXPORT int   getFramePtrRegnum();
    DLLEXPORT bool  setVersionFileName(std::string &fileName);
    DLLEXPORT bool  setVersions(std::vector<std::string> &vers);
    DLLEXPORT bool  setVersionNum(unsigned verNum);
    DLLEXPORT bool  getVersionFileName(std::string &fileName);
    DLLEXPORT bool  getVersions(std::vector<std::string> *&vers);
    DLLEXPORT bool  VersionNum(unsigned &verNum);
    
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

    //  convenience functions, not really meant to be called outside symtabAPI
    bool addLocalVar(localVar *);
    bool addParam(localVar *);
    
private:
    Module*       module_;
    SymbolType    type_;
    SymbolLinkage linkage_;
    Offset        addr_;
    Region*      sec_;
    unsigned      size_;  // size of this symbol. This is NOT available on all platforms.
#if 0
    void*         upPtr_;
#endif
    bool          isInDynsymtab_;
    bool          isInSymtab_;

    std::vector<std::string> mangledNames;
    std::vector<std::string> prettyNames;
    std::vector<std::string> typedNames;
    SymbolTag     tag_;
    int           framePtrRegNum_;
    Type          *retType_;
    // Module Objects are created in Symtab and not in Object.
    // So we need a way to store the name of the module in 
    // which the symbol is present.
    std::string moduleName_;  

public:
#if 0
    localVarCollection *vars_;
    localVarCollection *params_;
#endif
};

inline
Symbol::Symbol(unsigned)
   : //name_("*bad-symbol*"), module_("*bad-module*"),
    module_(NULL), type_(ST_UNKNOWN), linkage_(SL_UNKNOWN), addr_(0), sec_(NULL), size_(0), 
    isInDynsymtab_(false), isInSymtab_(true), tag_(TAG_UNKNOWN), retType_(NULL), moduleName_("")
    //vars_(NULL), params_(NULL) 
{
}
#if 0
inline
Symbol::Symbol(unsigned)
   : //name_("*bad-symbol*"), module_("*bad-module*"),
    module_(NULL), type_(ST_UNKNOWN), linkage_(SL_UNKNOWN), addr_(0), sec_(NULL), size_(0), upPtr_(NULL),
    isInDynsymtab_(false), isInSymtab_(true), tag_(TAG_UNKNOWN), retType_(NULL), moduleName_(""), 
    vars_(NULL), params_(NULL) {
}
#endif

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
#if 0
	&& (upPtr_    == s.upPtr_)
#endif
    && (isInDynsymtab_ == s.isInDynsymtab_)
    && (isInSymtab_ == s.isInSymtab_)
	&& (retType_    == s.retType_)
    	&& (mangledNames == s.mangledNames)
    	&& (prettyNames == s.prettyNames)
    	&& (typedNames == s.typedNames)
        && (moduleName_ == s.moduleName_));
}

}//namespace SymtabAPI
}//namespace Dyninst

#endif /* !defined(_Symbol_h_) */
