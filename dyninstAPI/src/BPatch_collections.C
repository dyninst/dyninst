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

#include <stdio.h>

#define BPATCH_FILE

#include "util.h"
#include "BPatch_collections.h"
#include "BPatch_Vector.h"
#include "BPatch.h"

// Dwarf stuff
#include "mapped_object.h"
#include "mapped_module.h"

BPatch_localVarCollection::~BPatch_localVarCollection()
{
   for (auto iter = localVariablesByName.begin(); iter != localVariablesByName.end(); ++iter) {
      delete iter->second;
   }
}

void BPatch_localVarCollection::addLocalVar(BPatch_localVar * var){

  localVariablesByName[var->getName()]= var;

}

BPatch_localVar * BPatch_localVarCollection::findLocalVar(const char *name){

   auto iter = localVariablesByName.find(name);
   if (iter == localVariablesByName.end()) return NULL;
   return iter->second;
}

BPatch_Vector<BPatch_localVar *> *BPatch_localVarCollection::getAllVars() {
    BPatch_Vector<BPatch_localVar *> *localVarVec = new BPatch_Vector<BPatch_localVar *>;

   for (auto iter = localVariablesByName.begin(); iter != localVariablesByName.end(); ++iter) {
      localVarVec->push_back(iter->second);
   }

    return localVarVec;
}
  
std::unordered_map<std::string, BPatch_typeCollection * > BPatch_typeCollection::fileToTypesMap;

BPatch_typeCollection *BPatch_typeCollection::getGlobalTypeCollection() {
    BPatch_typeCollection *tc = new BPatch_typeCollection();
    tc->refcount++;
    return tc;
}

BPatch_typeCollection *BPatch_typeCollection::getModTypeCollection(BPatch_module *bpmod) {
    assert(bpmod);
    mapped_object *moduleImage = bpmod->lowlevel_mod()->obj();
    assert( moduleImage != NULL );
#if defined(cap_dwarf)
    // TODO: can we use this on other platforms as well?    
    auto iter = fileToTypesMap.find(moduleImage->fullName());
    if (iter != fileToTypesMap.end()) {
       iter->second->refcount++;
       return iter->second;
    }
#endif

    BPatch_typeCollection *newTC = new BPatch_typeCollection();
    fileToTypesMap[moduleImage->fullName()] = newTC;
    newTC->refcount++;
    return newTC;
}

void BPatch_typeCollection::freeTypeCollection(BPatch_typeCollection *tc) {
    assert(tc);
    tc->refcount--;
    if (tc->refcount == 0) {
       for (auto iter = fileToTypesMap.begin(); iter != fileToTypesMap.end(); ++iter) {
          if (iter->second == tc) {
             fileToTypesMap.erase(iter);
             break;
          }
       }
       delete tc;
    }
}

BPatch_typeCollection::BPatch_typeCollection():
    refcount(0),
    dwarfParsed_(false)
{
  /* Initialize hash tables: typesByName, typesByID */
}

BPatch_typeCollection::~BPatch_typeCollection()
{
    // We sometimes directly delete (refcount == 1) or go through the
    // decRefCount (which will delete when refcount == 0)
    assert(refcount == 0 ||
           refcount == 1);

    for(const auto& t: typesByName) {
        t.second->decrRefCount();
    }

    for(const auto& t: typesByID) {
        t.second->decrRefCount();
    }
}

BPatch_type *BPatch_typeCollection::findType(const char *name)
{
   auto iter = typesByName.find(name);
   if (iter != typesByName.end()) {
      return iter->second;
   }
   else {
      if (BPatch::bpatch && BPatch::bpatch->builtInTypes)
         return BPatch::bpatch->builtInTypes->findBuiltInType(name);
      else
         return NULL;
   }
}

BPatch_type *BPatch_typeCollection::findTypeLocal(const char *name)
{
   auto iter = typesByName.find(name);
   if (iter != typesByName.end()) return iter->second;
   return NULL;
}

BPatch_type *BPatch_typeCollection::findTypeLocal(const int &ID)
{
   auto iter = typesByID.find(ID);
   if (iter != typesByID.end()) return iter->second;
   return NULL;
}


BPatch_type * BPatch_typeCollection::findOrCreateType( const int & ID ) {
   auto iter = typesByID.find(ID);
   if (iter != typesByID.end()) {
      return iter->second;
   }

   BPatch_type * returnType = NULL;
   if( BPatch::bpatch && BPatch::bpatch->builtInTypes ) {
      returnType = BPatch::bpatch->builtInTypes->findBuiltInType(ID);
   }
   
   if( returnType == NULL ) {
      /* Create a placeholder type. */
      returnType = BPatch_type::createPlaceholder(ID);
      assert( returnType != NULL );
      
      /* Having created the type, add it. */
      addType( returnType );
   }
  
    return returnType;
} /* end findOrCreateType() */

BPatch_type * BPatch_typeCollection::addOrUpdateType( BPatch_type * type ) {
    BPatch_type * existingType = findTypeLocal( type->getID() );
    if( existingType == NULL ) {
        if( type->getName() != NULL ) {
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
        if (existingType->getDataClass() == BPatch_dataUnknownType) {
           typesByID[type->getID()] = type;
           type->incrRefCount();
           existingType->decrRefCount();
           existingType = type;
        } else {
           /* Merge the type information. */
//           existingType->merge(type);  //TODO - change
        }
    /* The type may have gained a name. */
    if( existingType->getName() != NULL) {
       auto iter = typesByName.find(existingType->getName());
       if (iter == typesByName.end()) {
          typesByName[ existingType->getName() ] = existingType;
          existingType->incrRefCount();
       }
       else if (iter->second != existingType) {
             typesByName[ existingType->getName() ]->decrRefCount();
             typesByName[ existingType->getName() ] = existingType;
             existingType->incrRefCount();
       }
    }

    /* Tell the parser to update its type pointer. */
    return existingType;
    }
} /* end addOrUpdateType() */

BPatch_type *BPatch_typeCollection::findType(const int & ID)
{
   auto iter = typesByID.find(ID);
   if (iter != typesByID.end()) return iter->second;
   else {
      BPatch_type *ret = NULL;
      
      if (BPatch::bpatch && BPatch::bpatch->builtInTypes) 
         ret = BPatch::bpatch->builtInTypes->findBuiltInType(ID);
      
      return ret;
   }
}

BPatch_type *BPatch_typeCollection::findVariableType(const char *name)
{
   auto iter = globalVarsByName.find(name);
   if (iter != globalVarsByName.end()) {
      return iter->second;
   }
   return NULL;
}

void BPatch_typeCollection::addType(BPatch_type *type)
{
  if(type->getName() != NULL) { //Type could have no name.
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

void BPatch_typeCollection::clearNumberedTypes() {
   for (auto it = typesByID.begin();
        it != typesByID.end();
        it ++) {
      it->second->decrRefCount();
   }
   typesByID.clear();
}


BPatch_builtInTypeCollection::BPatch_builtInTypeCollection()
{
  /* Initialize hash tables: builtInTypesByName, builtInTypesByID */
}

BPatch_builtInTypeCollection::~BPatch_builtInTypeCollection()
{

   for (auto iter = builtInTypesByName.begin(); iter != builtInTypesByName.end(); ++iter) {
      iter->second->decrRefCount();
   }
   for (auto iter = builtInTypesByID.begin(); iter != builtInTypesByID.end(); ++iter) {
      iter->second->decrRefCount();
   }
}

BPatch_type *BPatch_builtInTypeCollection::findBuiltInType(const char *name)
{
   auto iter = builtInTypesByName.find(name);
   if (iter != builtInTypesByName.end()) return iter->second;
   return NULL;
}

BPatch_type *BPatch_builtInTypeCollection::findBuiltInType(const int & ID)
{
   auto iter = builtInTypesByID.find(ID);
   if (iter != builtInTypesByID.end()) return iter->second;
   return NULL;
}

void BPatch_builtInTypeCollection::addBuiltInType(BPatch_type *type)
{
  if(type->getName() != NULL) { //Type could have no name.
    builtInTypesByName[type->getName()] = type;
    type->incrRefCount();
  }
  //All built-in types have unique IDs so far jdd 4/21/99
  builtInTypesByID[type->getID()] = type;
  type->incrRefCount();
}



