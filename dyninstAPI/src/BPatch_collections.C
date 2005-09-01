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

#include <stdio.h>

#define BPATCH_FILE

#include "util.h"
#include "BPatch_collections.h"
#include "BPatch_Vector.h"
#include "BPatch.h"

// Dwarf stuff
#include "mapped_object.h"
#include "mapped_module.h"

//#include "BPatch_type.h"


/**************************************************************************
 * BPatch_localVarCollection
 *************************************************************************/

/*
 * BPatch_localVarCollection::~BPatch_localVarCollection
 *
 * Destructor for BPatch_localVarCollection.  Deletes all type objects that
 * have been inserted into the collection.
 */
BPatch_localVarCollection::~BPatch_localVarCollection()
{
   dictionary_hash_iter<pdstring, BPatch_localVar *> li(localVariablesByName);
       
   pdstring	         name;
   BPatch_localVar	*localVar;

   // delete localVariablesByName collection
   while (li.next(name, localVar))
	delete localVar;
}

/*
 * BPatch_localVarCollection::addLocalVar()
 * This function adds local variables to the set of local variables
 * for function.
 */

void BPatch_localVarCollection::addLocalVar(BPatch_localVar * var){

  localVariablesByName[var->getName()]= var;

}

/*
 * BPatch_localVarCollection::findLocalVar()
 * This function finds a local variable by name and returns a pointer to
 * it or NULL if the local variable does not exist in the set of function
 * local variables.
 */
BPatch_localVar * BPatch_localVarCollection::findLocalVar(const char *name){

  if(localVariablesByName.defines(name) )
    return localVariablesByName[name];
  else
    return (BPatch_localVar *)NULL;
}

/*
 * BPatch_localVarCollection::getAllVars()
 * this function returns all the local variables in the collection.
 */
BPatch_Vector<BPatch_localVar *> *BPatch_localVarCollection::getAllVars() {
    dictionary_hash_iter<pdstring, BPatch_localVar *> li(localVariablesByName);

    pdstring               name;
    BPatch_localVar     *localVar;

    BPatch_Vector<BPatch_localVar *> *localVarVec = new BPatch_Vector<BPatch_localVar *>;

    // get all local vars in the localVariablesByName collection
    while (li.next(name, localVar))
	localVarVec->push_back(localVar);

    return localVarVec;
}
  
// Could be somewhere else... for DWARF-work.
dictionary_hash<pdstring, BPatch_typeCollection * > BPatch_typeCollection::fileToTypesMap(pdstring::hash);

/*
 * Reference count
 */

BPatch_typeCollection *BPatch_typeCollection::getGlobalTypeCollection() {
    BPatch_typeCollection *tc = new BPatch_typeCollection();
    tc->refcount++;
    return tc;
}

BPatch_typeCollection *BPatch_typeCollection::getModTypeCollection(BPatch_module *bpmod) {
    assert(bpmod);
    mapped_object *moduleImage = bpmod->lowlevel_mod()->obj();
    assert( moduleImage != NULL );
#if defined(USES_DWARF_DEBUG)
    // TODO: can we use this on other platforms as well?    
    if( fileToTypesMap.defines( moduleImage->fullName() ) ) {
        // /* DEBUG */ fprintf( stderr, "%s[%d]: found cache for file '%s' (module '%s')\n", __FILE__, __LINE__, fileName, moduleFileName );
        BPatch_typeCollection *cachedTC = fileToTypesMap [ moduleImage->fullName() ];
        cachedTC->refcount++;
        return cachedTC;
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
        // Nuke...
        // Do we want to? We could always leave 'em around to reduce parsing time.
#if 0
        dictionary_hash_iter<pdstring, BPatch_typeCollection *> iter(fileToTypesMap);
        for (; iter; iter++) {
            if (iter.currval() == tc) {
                fileToTypesMap.undef(iter.currkey());
                break;
            }
        }
        delete tc;
#endif
    }
}

/*
 * BPatch_typeCollection::BPatch_typeCollection
 *
 * Constructor for BPatch_typeCollection.  Creates the two dictionaries
 * for the type, by Name and ID.
 */
BPatch_typeCollection::BPatch_typeCollection():
    typesByName(pdstring::hash),
    globalVarsByName(pdstring::hash),
    typesByID(intHash),
    refcount(0),
    dwarfParsed_(false)
{
  /* Initialize hash tables: typesByName, typesByID */
}

/*
 * BPatch_typeCollection::~BPatch_typeCollection
 *
 * Destructor for BPatch_typeCollection.  Deletes all type objects that have
 * been inserted into the collection.
 */
BPatch_typeCollection::~BPatch_typeCollection()
{
    // We sometimes directly delete (refcount == 1) or go through the
    // decRefCount (which will delete when refcount == 0)
    assert(refcount == 0 ||
           refcount == 1);
    // delete all of the types
    // This doesn't seem to work - jkh 1/31/00
#if 0
    dictionary_hash_iter<pdstring, BPatch_type *> ti(typesByName);
    dictionary_hash_iter<int, BPatch_type *> tid(typesByID);
    dictionary_hash_iter<pdstring, BPatch_type *> gi(globalVarsByName);
    
    pdstring      gname; 
    pdstring	name;
    BPatch_type	*type;
    int         id;
    while (tid.next(id, type))
        delete type;
    
    
    // Underlying types deleted already just need to get rid of pointers
    while (ti.next(name, type))
        type = NULL;
    
    // delete globalVarsByName collection
    while (gi.next(name, type))
        delete type;
    
    for (dictionary_hash_iter<int, BPatch_type *> it = typesByID.begin();
         it != typesByID.end();
         it ++) {
        (*it)->decrRefCount();
    }
    for (dictionary_hash_iter<pdstring, BPatch_type *> it2 = typesByName.begin();
         it2 != typesByName.end();
         it2 ++) {
        (*it2)->decrRefCount();
    }
#endif    
}

/*
 * BPatch_typeCollection::findType
 *
 * Retrieve a pointer to a BPatch_type object representing the named type from
 * the collection.  If no such type exists and no such type can be derived
 * from existing types, then the function returns NULL.
 *
 * name		The name of the type to look up.
 * id           The unique type ID of the type tp look up.
 */
BPatch_type *BPatch_typeCollection::findType(const char *name)
{
    if (typesByName.defines(name))
    	return typesByName[name];
    else {
       if (BPatch::bpatch && BPatch::bpatch->builtInTypes)
          return BPatch::bpatch->builtInTypes->findBuiltInType(name);
       else
          return NULL;
    }
}

BPatch_type *BPatch_typeCollection::findTypeLocal(const char *name)
{
   if (typesByName.defines(name))
      return typesByName[name];
   else
      return NULL;
}

BPatch_type *BPatch_typeCollection::findTypeLocal(const int &ID)
{
   if (typesByID.defines(ID))
      return typesByID[ID];
   else
      return NULL;
}


BPatch_type * BPatch_typeCollection::findOrCreateType( const int & ID ) {
    if( typesByID.defines(ID) ) { return typesByID[ID]; }

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
#ifdef notdef
	//#if defined( USES_DWARF_DEBUG )
        /* Replace the existing type wholesale. */
        // bperr( "Updating existing type '%s' %d at %p with %p\n", type->getName(), type->getID(), existingType, type );
	memmove( existingType, type, sizeof( BPatch_type ) );
#else
        if (existingType->getDataClass() == BPatch_dataUnknownType) {
           typesByID[type->getID()] = type;
           existingType->decrRefCount();
           existingType = type;
           type->incrRefCount();
        } else {
           /* Merge the type information. */
           existingType->merge(type);
        }
#endif
    /* The type may have gained a name. */
    if( existingType->getName() != NULL) {
       if (typesByName.defines(existingType->getName())) {
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

BPatch_type *BPatch_typeCollection::findType(const int & ID)
{
    if (typesByID.defines(ID))
    	return typesByID[ID];
    else {
	BPatch_type *ret = NULL;
	
	if (BPatch::bpatch && BPatch::bpatch->builtInTypes) 
	    ret = BPatch::bpatch->builtInTypes->findBuiltInType(ID);

	return ret;
    }
}

/*
 * BPatch_typeCollection::findVariableType
 * (Global Variables)
 * Retrieve a pointer to a BPatch_type object representing the named type from
 * the collection.  If no such type exists and no such type can be derived
 * from existing types, then the function returns NULL.
 *
 * name		The name of the type to look up.
 */
BPatch_type *BPatch_typeCollection::findVariableType(const char *name)
{
    if (globalVarsByName.defines(name))
    	return globalVarsByName[name];
    else
	return (BPatch_type *) NULL;
}

/*
 * BPatch_typeCollection::addType
 *
 * Add a new type to the type collection.  Note that when a type is added to
 * the collection, it becomes the collection's responsibility to delete it
 * when it is no longer needed.  For one thing, this means that a type
 * allocated on the stack should *NEVER* be put into a BPatch_typeCollection.
 */
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
   for (dictionary_hash_iter<int, BPatch_type *> it = typesByID.begin();
        it != typesByID.end();
        it ++) {
      (*it)->decrRefCount();
   }
   typesByID.clear();
}


/*
 * BPatch_builtInTypeCollection::BPatch_builtInTypeCollection
 *
 * Constructor for BPatch_builtInTypeCollection.  Creates adictionary
 * for the builtInType, by Name and ID.
 *  XXX- Don't know if a collection is needed for types by name, but
 * it is created just in case. jdd 4/21/99
 */
BPatch_builtInTypeCollection::BPatch_builtInTypeCollection():
  builtInTypesByName(pdstring::hash),
  builtInTypesByID(intHash)
{
  /* Initialize hash tables: builtInTypesByName, builtInTypesByID */
}

/*
 * BPatch_builtInTypeCollection::~BPatch_builtInTypeCollection
 *
 * Destructor for BPatch_builtInTypeCollection.  Deletes all builtInType objects that have
 * been inserted into the collection.
 */
BPatch_builtInTypeCollection::~BPatch_builtInTypeCollection()
{
    dictionary_hash_iter<pdstring, BPatch_type *> bit(builtInTypesByName);
    dictionary_hash_iter<int, BPatch_type *> bitid(builtInTypesByID);
     
      pdstring	name;
    int         id;
    BPatch_type	*type;

    // delete builtInTypesByName collection
    while (bit.next(name, type))
	type->decrRefCount();
    // delete builtInTypesByID collection
    while (bitid.next(id, type))
	type->decrRefCount();
}


/*
 * BPatch_builtInTypeCollection::findBuiltInType
 *
 * Retrieve a pointer to a type object representing the named type from
 * the collection.  If no such type exists and no such type can be derived
 * from existing types, then the function returns NULL.
 *
 * name		The name of the type to look up.
 * id           The unique type ID of the type tp look up.
 */
BPatch_type *BPatch_builtInTypeCollection::findBuiltInType(const char *name)
{
    if (builtInTypesByName.defines(name))
    	return builtInTypesByName[name];
    else
	return (BPatch_type *)NULL;
}

BPatch_type *BPatch_builtInTypeCollection::findBuiltInType(const int & ID)
{
    if (builtInTypesByID.defines(ID))
    	return builtInTypesByID[ID];
    else
	return (BPatch_type *)NULL;
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



