/*
 * Copyright (c) 1996-2011 Barton P. Miller
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef _Collections_h_
#define _Collections_h_

#include "Type.h"
#include "Variable.h"
#include "Serialization.h"

namespace Dyninst {

namespace SymtabAPI {

class Module;
class Symtab;
class localVar;

/*
 * This class contains a collection of local variables.
 * Each function will have one of these objects associated with it.
 * This object will store all the local variables within this function.
 * Note: This class is unaware of scope.
 */
class localVarCollection : public AnnotationContainer<localVar *> {
  
  std::vector<localVar *> localVars;
  
  SYMTAB_EXPORT bool addItem_impl(localVar *);
public:
  SYMTAB_EXPORT localVarCollection(){}
  SYMTAB_EXPORT ~localVarCollection();

  SYMTAB_EXPORT void addLocalVar(localVar * var);
  SYMTAB_EXPORT localVar * findLocalVar(std::string &name);
  SYMTAB_EXPORT std::vector<localVar *> *getAllVars();  

  SYMTAB_EXPORT Serializable *ac_serialize_impl(SerializerBase *, const char * = "localVarCollection") THROW_SPEC (SerializerError);
};
  


/*
 * Due to DWARF weirdness, this can be shared between multiple BPatch_modules.
 * So we reference-count to make life easier.
 */
class typeCollection : public Serializable//, public AnnotatableSparse 
{
    friend class Symtab;
    friend class Object;
    friend class Module;
	friend class Type;

    dyn_hash_map<std::string, Type *> typesByName;
    dyn_hash_map<std::string, Type *> globalVarsByName;
    dyn_hash_map<int, Type *> typesByID;

    SYMTAB_EXPORT ~typeCollection();

    // DWARF:
    /* Cache type collections on a per-image basis.  (Since
       BPatch_functions are solitons, we don't have to cache them.) */
    static dyn_hash_map< void *, typeCollection * > fileToTypesMap;
	//static dyn_hash_map<int, std::vector<std::pair<dataClass, Type **> > > deferred_lookups;
	static bool doDeferredLookups(typeCollection *);

    // DWARF...
    bool dwarfParsed_;

	SYMTAB_EXPORT Serializable *serialize_impl(SerializerBase *, const char * = "typeCollection") THROW_SPEC (SerializerError);
	public:
    SYMTAB_EXPORT typeCollection();
public:
	static void addDeferredLookup(int, dataClass, Type **);

    SYMTAB_EXPORT static typeCollection *getModTypeCollection(Module *mod);
#if 0
    SYMTAB_EXPORT static typeCollection *getGlobalTypeCollection();
    SYMTAB_EXPORT static void freeTypeCollection(typeCollection *tc);
#endif

    // DWARF...
    SYMTAB_EXPORT bool dwarfParsed() { return dwarfParsed_; }
    SYMTAB_EXPORT void setDwarfParsed() { dwarfParsed_ = true; }

    SYMTAB_EXPORT Type	*findType(std::string name);
    SYMTAB_EXPORT Type	*findType(const int ID);
    SYMTAB_EXPORT Type 	*findTypeLocal(std::string name);
    SYMTAB_EXPORT Type 	*findTypeLocal(const int ID);
    SYMTAB_EXPORT void	addType(Type *type);
    SYMTAB_EXPORT void        addGlobalVariable(std::string &name, Type *type);

    /* Some debug formats allow forward references.  Rather than
       fill in forward in a second pass, generate placeholder
       types, and fill them in as we go.  Because we require
       One True Pointer for each type (in parseStab.C), when
       updating a type, return that One True Pointer. */
    SYMTAB_EXPORT Type * findOrCreateType( const int ID );
    template<class T>
    SYMTAB_EXPORT T* addOrUpdateType(T* type);

    SYMTAB_EXPORT Type *findVariableType(std::string &name);

    SYMTAB_EXPORT std::vector<Type *> *getAllTypes();
    SYMTAB_EXPORT std::vector<std::pair<std::string, Type *> > *getAllGlobalVariables();
    SYMTAB_EXPORT void clearNumberedTypes();
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

class SYMTAB_EXPORT builtInTypeCollection {
   
    dyn_hash_map<std::string, Type *> builtInTypesByName;
    dyn_hash_map<int, Type *> builtInTypesByID;
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



