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

#ifndef _Collections_h_
#define _Collections_h_

#include "symtabAPI/h/Type.h"

using namespace std;
namespace Dyninst {

namespace SymtabAPI {

class Module;
class Symtab;

/*
 * This class contains a collection of local variables.
 * Each function will have one of these objects associated with it.
 * This object will store all the local variables within this function.
 * Note: This class is unaware of scope.
 */
class DLLEXPORT localVarCollection{
  
  hash_map<std::string, localVar *> localVariablesByName;
  vector<localVar *> localVars;

public:
  localVarCollection(){}
  ~localVarCollection();

  void addLocalVar(localVar * var);
  localVar * findLocalVar(std::string &name);
  vector<localVar *> *getAllVars();  
};
  


/*
 * Due to DWARF weirdness, this can be shared between multiple BPatch_modules.
 * So we reference-count to make life easier.
 */
class DLLEXPORT typeCollection {
    friend class Symtab;
    friend class Object;
    friend class Module;

    hash_map<std::string, Type *> typesByName;
    hash_map<std::string, Type *> globalVarsByName;
    hash_map<int, Type *> typesByID;

    ~typeCollection();

    unsigned refcount;
    typeCollection();

#if 0
    // DWARF:
    /* Cache type collections on a per-image basis.  (Since
       BPatch_functions are solitons, we don't have to cache them.) */
    static hash_map< std::string, typeCollection * > fileToTypesMap;
#endif

    // DWARF...
    bool dwarfParsed_;

public:
    static typeCollection *getGlobalTypeCollection();
#if 0
    static typeCollection *getModTypeCollection(Module *mod);
#endif
    static void freeTypeCollection(typeCollection *tc);

    // DWARF...
    bool dwarfParsed() { return dwarfParsed_; }
    void setDwarfParsed() { dwarfParsed_ = true; }

    Type	*findType(std::string name);
    Type	*findType(const int ID);
    Type 	*findTypeLocal(std::string name);
    Type 	*findTypeLocal(const int ID);
    void	addType(Type *type);
    void        addGlobalVariable(std::string &name, Type *type);

    /* Some debug formats allow forward references.  Rather than
       fill in forward in a second pass, generate placeholder
       types, and fill them in as we go.  Because we require
       One True Pointer for each type (in parseStab.C), when
       updating a type, return that One True Pointer. */
    Type * findOrCreateType( const int ID );
    Type * addOrUpdateType( Type * type );

    Type *findVariableType(std::string &name);

    std::vector<Type *> *getAllTypes();
    std::vector<std::pair<std::string, Type *> > *getAllGlobalVariables();
    void clearNumberedTypes();
};

/*
 * This class defines the collection for the built-in Types
 * gnu ( and AIX??) use negative numbers to define other types
 * in terms of these built-in types.
 * This collection is global and built in the BPatch_image constructor.
 * This means that only one collection of built-in types is made
 * per image.  jdd 4/21/99
 *
 */

class DLLEXPORT builtInTypeCollection {
   
    hash_map<std::string, Type *> builtInTypesByName;
    hash_map<int, Type *> builtInTypesByID;
public:

    builtInTypeCollection();
    ~builtInTypeCollection();

    Type	*findBuiltInType(std::string &name);
    Type	*findBuiltInType(const int ID);
    void	addBuiltInType(Type *type);
    std::vector<Type *> *getAllBuiltInTypes();
   
};

}// namespace SymtabAPI
}// namespace Dyninst

#endif /* _Collections_h_ */



