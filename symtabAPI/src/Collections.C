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
 
#include <stdio.h>

#include "symtabAPI/src/Collections.h"
#include "Symtab.h"
#include <string>
using namespace std;

using namespace Dyninst;
using namespace Dyninst::SymtabAPI;

/**************************************************************************
 * localVarCollection
 *************************************************************************/

/*
 * localVarCollection::~localVarCollection
 *
 * Destructor for localVarCollection.  Deletes all type objects that
 * have been inserted into the collection.
 */
localVarCollection::~localVarCollection()
{
   hash_map<std::string, localVar *>::iterator li = localVariablesByName.begin();
       
   // delete localVariablesByName collection
   for(;li!=localVariablesByName.end();li++)
	delete li->second;
   
   localVars.clear();
}

/*
 * localVarCollection::addLocalVar()
 * This function adds local variables to the set of local variables
 * for function.
 */

void localVarCollection::addLocalVar(localVar * var){
  if(var->getName() == "globalVariable18_1")
    cout << "found var" << endl;
  localVariablesByName[var->getName()]= var;
  localVars.push_back(var);
}

/*
 * localVarCollection::findLocalVar()
 * This function finds a local variable by name and returns a pointer to
 * it or NULL if the local variable does not exist in the set of function
 * local variables.
 */
localVar *localVarCollection::findLocalVar(std::string &name){

  if(localVariablesByName.find(name) != localVariablesByName.end())
    return localVariablesByName[name];
  else
    return (localVar *)NULL;
}

/*
 * localVarCollection::getAllVars()
 * this function returns all the local variables in the collection.
 */
std::vector<localVar *> *localVarCollection::getAllVars() {
    return &localVars;
}
  
// Could be somewhere else... for DWARF-work.
hash_map<std::string, typeCollection *> typeCollection::fileToTypesMap;

/*
 * Reference count
 */

typeCollection *typeCollection::getGlobalTypeCollection() {
    typeCollection *tc = new typeCollection();
    tc->refcount++;
    return tc;
}

typeCollection *typeCollection::getModTypeCollection(Module *mod) {
    assert(mod);

//#if defined(USES_DWARF_DEBUG)
    // TODO: can we use this on other platforms as well?    
    if( fileToTypesMap.find( mod->exec()->file()) != fileToTypesMap.end()) {
        // /* DEBUG */ fprintf( stderr, "%s[%d]: found cache for file '%s' (module '%s')\n", __FILE__, __LINE__, fileName, moduleFileName );
        typeCollection *cachedTC = fileToTypesMap [mod->exec()->file()];
        cachedTC->refcount++;
        return cachedTC;
    }
//#endif

    typeCollection *newTC = new typeCollection();
    fileToTypesMap[mod->exec()->file()] = newTC;
    newTC->refcount++;
    return newTC;
}

void typeCollection::freeTypeCollection(typeCollection *tc) {
    assert(tc);
    tc->refcount--;
    if (tc->refcount == 0) {
        hash_map<std::string, typeCollection *>::iterator iter = fileToTypesMap.begin();
        for (; iter!= fileToTypesMap.end(); iter++) {
            if (iter->second == tc) {
                fileToTypesMap.erase(iter->first);
                break;
            }
        }
        delete tc;
    }
}

/*
 * typeCollection::typeCollection
 *
 * Constructor for typeCollection.  Creates the two dictionaries
 * for the type, by Name and ID.
 */
typeCollection::typeCollection():
    refcount(0),
    dwarfParsed_(false)
{
  /* Initialize hash tables: typesByName, typesByID */
}

/*
 * typeCollection::~typeCollection
 *
 * Destructor for typeCollection.  Deletes all type objects that have
 * been inserted into the collection.
 */
typeCollection::~typeCollection()
{
    // We sometimes directly delete (refcount == 1) or go through the
    // decRefCount (which will delete when refcount == 0)
    assert(refcount == 0 ||
           refcount == 1);
    // delete all of the types
    // This doesn't seem to work - jkh 1/31/00
#if 0
    dictionary_hash_iter<std::string, type *> ti(typesByName);
    dictionary_hash_iter<int, type *> tid(typesByID);
    dictionary_hash_iter<std::string, type *> gi(globalVarsByName);
    
    std::string      gname; 
    std::string	name;
    type	*type;
    int         id;
    while (tid.next(id, type))
        delete type;
    
    
    // Underlying types deleted already just need to get rid of pointers
    while (ti.next(name, type))
        type = NULL;
    
    // delete globalVarsByName collection
    while (gi.next(name, type))
        delete type;
    
    for (dictionary_hash_iter<int, type *> it = typesByID.begin();
         it != typesByID.end();
         it ++) {
        (*it)->decrRefCount();
    }
    for (dictionary_hash_iter<std::string, type *> it2 = typesByName.begin();
         it2 != typesByName.end();
         it2 ++) {
        (*it2)->decrRefCount();
    }
#endif    
}

/*
 * typeCollection::findType
 *
 * Retrieve a pointer to a type object representing the named type from
 * the collection.  If no such type exists and no such type can be derived
 * from existing types, then the function returns NULL.
 *
 * name		The name of the type to look up.
 * id           The unique type ID of the type tp look up.
 */
Type *typeCollection::findType(std::string name)
{
    if (typesByName.find(name) != typesByName.end())
    	return typesByName[name];
	else if (Symtab::builtInTypes)
        return Symtab::builtInTypes->findBuiltInType(name);
    else
		return NULL;
}

Type *typeCollection::findTypeLocal(std::string name)
{
   if (typesByName.find(name) != typesByName.end())
      return typesByName[name];
   else
      return NULL;
}

Type *typeCollection::findTypeLocal(const int ID)
{
   if (typesByID.find(ID) != typesByID.end())
      return typesByID[ID];
   else
      return NULL;
}


Type * typeCollection::findOrCreateType( const int ID ) {
    if( typesByID.find(ID) != typesByID.end()) { return typesByID[ID]; }

    Type * returnType = NULL;
    if( Symtab::builtInTypes ) {
        returnType = Symtab::builtInTypes->findBuiltInType(ID);
    }

    if( returnType == NULL ) {
        /* Create a placeholder type. */
        returnType = Type::createPlaceholder(ID);
	assert( returnType != NULL );

        /* Having created the type, add it. */
        addType( returnType );
    }
  
    return returnType;
} /* end findOrCreateType() */

Type * typeCollection::addOrUpdateType( Type * type ) {
    if(type->getID() == 14)
        cout << "found type with ID 14" << endl;
    Type * existingType = findTypeLocal( type->getID() );
    if( existingType == NULL ) {
        if( type->getName() != "" ) {
            typesByName[ type->getName() ] = type;
            type->incrRefCount();
        }
        typesByID[ type->getID() ] = type;
        type->incrRefCount();
        return type;
    } else {
        /* Multiple inclusions of the same object file can result
           in us parsing the same module types repeatedly. GCC does this
           with some of its internal routines */
        if (*existingType == *type) {
           return existingType;
        }
#ifdef notdef
	//#if defined( USES_DWARF_DEBUG )
        /* Replace the existing type wholesale. */
        // bperr( "Updating existing type '%s' %d at %p with %p\n", type->getName(), type->getID(), existingType, type );
	memmove( existingType, type, sizeof( type ) );
#else
        if (existingType->getDataClass() == dataUnknownType) {
           typesByID[type->getID()] = type;
           type->incrRefCount();
           existingType->decrRefCount();
           existingType = type;
        } else {
           /* Merge the type information. */
           existingType->merge(type);
        }
#endif
    /* The type may have gained a name. */
    if( existingType->getName() != "") {
       if (typesByName.find(existingType->getName()) != typesByName.end()) {
          if (typesByName[ existingType->getName() ] != existingType) {
             typesByName[ existingType->getName() ]->decrRefCount();
             typesByName[ existingType->getName() ] = existingType;
             existingType->incrRefCount();
          }
       } else {
          typesByName[ existingType->getName() ] = existingType;
          existingType->incrRefCount();
       }
    }

    /* Tell the parser to update its type pointer. */
    return existingType;
    }
} /* end addOrUpdateType() */

Type *typeCollection::findType(const int ID)
{
    if (typesByID.find(ID) != typesByID.end())
    	return typesByID[ID];
    else {
	Type *ret = NULL;
	
	if (Symtab::builtInTypes) 
	    ret = Symtab::builtInTypes->findBuiltInType(ID);

	return ret;
    }
}

/*
 * typeCollection::findVariableType
 * (Global Variables)
 * Retrieve a pointer to a type object representing the named type from
 * the collection.  If no such type exists and no such type can be derived
 * from existing types, then the function returns NULL.
 *
 * name		The name of the type to look up.
 */
Type *typeCollection::findVariableType(std::string &name)
{
    if (globalVarsByName.find(name) != globalVarsByName.end())
    	return globalVarsByName[name];
    else
		return (Type *) NULL;
}

/*
 * typeCollection::addType
 *
 * Add a new type to the type collection.  Note that when a type is added to
 * the collection, it becomes the collection's responsibility to delete it
 * when it is no longer needed.  For one thing, this means that a type
 * allocated on the stack should *NEVER* be put into a typeCollection.
 */
void typeCollection::addType(Type *type)
{
  if(type->getName() != "") { //Type could have no name.
    typesByName[type->getName()] = type;
    type->incrRefCount();
  }

  //Types can share the same ID for typedef, thus not adding types with
  //same ID to the collection

  // XXX - Fortran seems to restart type numbers for each subroutine
  // if(!(this->findType(type->getID())))
       typesByID[type->getID()] = type;
  type->incrRefCount();
}

void typeCollection::addGlobalVariable(std::string &name, Type *type) {
    globalVarsByName[name] = type;
}

void typeCollection::clearNumberedTypes() {
   for (hash_map<int, Type *>::iterator it = typesByID.begin();
        it != typesByID.end();
        it ++) {
      it->second->decrRefCount();
   }
   typesByID.clear();
}

/*
 * localVarCollection::getAllVars()
 * this function returns all the local variables in the collection.
 */
std::vector<Type *> *typeCollection::getAllTypes() {
   std::vector<Type *> *typesVec = new std::vector<Type *>;
   for (hash_map<int, Type *>::iterator it = typesByID.begin();
        it != typesByID.end();
        it ++) {
	typesVec->push_back(it->second);
   }
   if(!typesVec->size()){
       free(typesVec);
       return NULL;
   }
   return typesVec;
}

vector<pair<string, Type *> > *typeCollection::getAllGlobalVariables() {
    vector<pair<string, Type *> > *varsVec = new vector<pair<string, Type *> >;
    for(hash_map<string, Type *>::iterator it = globalVarsByName.begin();
        it != globalVarsByName.end(); it++) {
	varsVec->push_back(pair<string, Type *>(it->first, it->second));
   }	
   if(!varsVec->size()){
       free(varsVec);
       return NULL;
   }
   return varsVec;
}

/*
 * builtInTypeCollection::builtInTypeCollection
 *
 * Constructor for builtInTypeCollection.  Creates adictionary
 * for the builtInType, by Name and ID.
 *  XXX- Don't know if a collection is needed for types by name, but
 * it is created just in case. jdd 4/21/99
 */
builtInTypeCollection::builtInTypeCollection()
{
  /* Initialize hash tables: builtInTypesByName, builtInTypesByID */
}

/*
 * builtInTypeCollection::~builtInTypeCollection
 *
 * Destructor for builtInTypeCollection.  Deletes all builtInType objects that have
 * been inserted into the collection.
 */
builtInTypeCollection::~builtInTypeCollection()
{
    hash_map<std::string, Type *>::iterator bit = builtInTypesByName.begin();
    hash_map<int, Type *>::iterator bitid = builtInTypesByID.begin();
     
    // delete builtInTypesByName collection
    for(;bit!=builtInTypesByName.end();bit++)
	bit->second->decrRefCount();
    // delete builtInTypesByID collection
    for(;bitid!=builtInTypesByID.end();bit++)
	bitid->second->decrRefCount();
}


/*
 * builtInTypeCollection::findBuiltInType
 *
 * Retrieve a pointer to a type object representing the named type from
 * the collection.  If no such type exists and no such type can be derived
 * from existing types, then the function returns NULL.
 *
 * name		The name of the type to look up.
 * id           The unique type ID of the type tp look up.
 */
Type *builtInTypeCollection::findBuiltInType(std::string &name)
{
    if (builtInTypesByName.find(name) != builtInTypesByName.end())
    	return builtInTypesByName[name];
    else
	return (Type *)NULL;
}

Type *builtInTypeCollection::findBuiltInType(const int ID)
{
    if (builtInTypesByID.find(ID) != builtInTypesByID.end())
    	return builtInTypesByID[ID];
    else
	return (Type *)NULL;
}

void builtInTypeCollection::addBuiltInType(Type *type)
{
  if(type->getName() != "") { //Type could have no name.
    builtInTypesByName[type->getName()] = type;
    type->incrRefCount();
  }
  //All built-in types have unique IDs so far jdd 4/21/99
  builtInTypesByID[type->getID()] = type;
  type->incrRefCount();
}

std::vector<Type *> *builtInTypeCollection::getAllBuiltInTypes() {
   std::vector<Type *> *typesVec = new std::vector<Type *>;
   for (hash_map<int, Type *>::iterator it = builtInTypesByID.begin();
        it != builtInTypesByID.end();
        it ++) {
	typesVec->push_back(it->second);
   }
   if(!typesVec->size()){
       free(typesVec);
       return NULL;
   }
   return typesVec;
}
