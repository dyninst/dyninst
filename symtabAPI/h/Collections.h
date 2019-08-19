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

#ifndef _Collections_h_
#define _Collections_h_

#include "concurrent.h"
#include "Type.h"
#include "Variable.h"
#include "Serialization.h"
#include <boost/core/enable_if.hpp>
#include <boost/type_traits/is_same.hpp>

namespace Dyninst {

namespace SymtabAPI {

class Module;
class Symtab;
class localVar;
class DwarfWalker;

/*
 * This class contains a collection of local variables.
 * Each function will have one of these objects associated with it.
 * This object will store all the local variables within this function.
 * Note: This class is unaware of scope.
 */


class SYMTAB_EXPORT localVarCollection : public AnnotationContainer<localVar *> {
  
  dyn_c_vector<localVar* > localVars;
  
  bool addItem_impl(localVar *);
public:
  localVarCollection(){}
  ~localVarCollection();

  void addLocalVar(localVar * var);
  localVar * findLocalVar(std::string &name);
  const dyn_c_vector<localVar *> &getAllVars() const;

  Serializable *ac_serialize_impl(SerializerBase *, const char * = "localVarCollection") THROW_SPEC (SerializerError);
};
  


/*
 * Due to DWARF weirdness, this can be shared between multiple BPatch_modules.
 * So we reference-count to make life easier.
 */
class SYMTAB_EXPORT typeCollection : public Serializable//, public AnnotatableSparse 
{
    friend class Symtab;
    friend class Object;
    friend class Module;
    friend class Type;
    friend class DwarfWalker;

    dyn_c_hash_map<std::string, boost::shared_ptr<Type>> typesByName;
    dyn_c_hash_map<std::string, boost::shared_ptr<Type>> globalVarsByName;
    dyn_c_hash_map<int, boost::shared_ptr<Type>> typesByID;


    // DWARF:
    /* Cache type collections on a per-image basis.  (Since
       BPatch_functions are solitons, we don't have to cache them.) */
    static dyn_c_hash_map< void *, typeCollection * > fileToTypesMap;
    static bool doDeferredLookups(typeCollection *);

    // DWARF...
    bool dwarfParsed_;

	Serializable *serialize_impl(SerializerBase *, const char * = "typeCollection") THROW_SPEC (SerializerError);
	public:
    typeCollection();
    ~typeCollection();
public:
	static void addDeferredLookup(int, dataClass, boost::shared_ptr<Type> *);
    static boost::mutex create_lock;

    static typeCollection *getModTypeCollection(Module *mod);

    // DWARF...
    bool dwarfParsed() { return dwarfParsed_; }
    void setDwarfParsed() { dwarfParsed_ = true; }

    boost::shared_ptr<Type> findType(std::string name);
    boost::shared_ptr<Type> findType(const int ID);
    boost::shared_ptr<Type> findTypeLocal(std::string name);
    boost::shared_ptr<Type> findTypeLocal(const int ID);
    void addType(boost::shared_ptr<Type> type);
    void addType(boost::shared_ptr<Type> type, boost::lock_guard<boost::mutex>&);
    void addGlobalVariable(std::string &name, boost::shared_ptr<Type> type);

    /* Some debug formats allow forward references.  Rather than
       fill in forward in a second pass, generate placeholder
       types, and fill them in as we go.  Because we require
       One True Pointer for each type (in parseStab.C), when
       updating a type, return that One True Pointer. */
    boost::shared_ptr<Type> findOrCreateType( const int ID );
    template<class T>
    typename boost::enable_if<
        boost::integral_constant<bool, !bool(boost::is_same<Type, T>::value)>,
    boost::shared_ptr<Type>>::type addOrUpdateType(boost::shared_ptr<T> type);

    boost::shared_ptr<Type> findVariableType(std::string &name);

    void getAllTypes(std::vector<boost::shared_ptr<Type>>&);
    void getAllGlobalVariables(std::vector<std::pair<std::string, boost::shared_ptr<Type>>>&);
    void clearNumberedTypes();
    private:
        boost::mutex placeholder_mutex; // The only intermodule contention should be around
        // typedefs/other placeholders, but we'll go ahead and lock around type add operations
        // to be safe
};

/*
 * This class defines the collection for the built-in Types
 * gnu use negative numbers to define other types
 * in terms of these built-in types.
 * This collection is global and built in the BPatch_image constructor.
 * This means that only one collection of built-in types is made
 * per image.  jdd 4/21/99
 *
 */

class SYMTAB_EXPORT builtInTypeCollection {
   
    dyn_c_hash_map<int, boost::shared_ptr<Type>> builtInTypesByID;
    dyn_c_hash_map<std::string, boost::shared_ptr<Type>> builtInTypesByName;
public:

    builtInTypeCollection();
    ~builtInTypeCollection();

    boost::shared_ptr<Type> findBuiltInType(std::string &name);
    boost::shared_ptr<Type> findBuiltInType(const int ID);
    void addBuiltInType(boost::shared_ptr<Type>);
    void getAllBuiltInTypes(std::vector<boost::shared_ptr<Type>>&);
   
};

}// namespace SymtabAPI
}// namespace Dyninst

#endif /* _Collections_h_ */



