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
#include <string>

#include "symutil.h"
#include "debug.h"
#include "Collections.h"
#include "Symtab.h"
#include "Module.h"
#include "Variable.h"
#include "Serialization.h"

#include "common/src/headers.h"
#include "common/src/serialize.h"

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
   std::vector<localVar *>::iterator li = localVars.begin();    
   for(;li!=localVars.end();li++)
   {
	   delete *li;
   }
   
   localVars.clear();
}

/*
 * localVarCollection::addLocalVar()
 * This function adds local variables to the set of local variables
 * for function.
 */

bool localVarCollection::addItem_impl(localVar * var)
{
  localVars.push_back(var);
  return true;
}

void localVarCollection::addLocalVar(localVar * var)
{
	if (!addItem(var))
	{
           create_printf("%s[%d]:  ERROR adding localVar\n", FILE__, __LINE__);
	}
}

/*
 * localVarCollection::findLocalVar()
 * This function finds a local variable by name and returns a pointer to
 * it or NULL if the local variable does not exist in the set of function
 * local variables.
 */
localVar *localVarCollection::findLocalVar(std::string &name){

   std::vector<localVar *>::iterator li = localVars.begin();    
   for(;li!=localVars.end();li++)
   {
      if (name == (*li)->getName()) {
         return *li;
      }
   }
   return NULL;
}

/*
 * localVarCollection::getAllVars()
 * this function returns all the local variables in the collection.
 */
std::vector<localVar *> *localVarCollection::getAllVars() 
{
    return &localVars;
}

#if !defined(SERIALIZATION_DISABLED)
Serializable *localVarCollection::ac_serialize_impl(SerializerBase *s, const char *tag) THROW_SPEC (SerializerError)
{
	unsigned short lvmagic = 72;
	serialize_printf("%s[%d]:  welcome to localVarCollection: ac_serialize_impl\n", 
			FILE__, __LINE__);
	ifxml_start_element(s, tag);
	gtranslate(s, lvmagic, "LocalVarMagicID");
	gtranslate(s, localVars, "LocalVariables");
	s->magic_check(FILE__, __LINE__);
	ifxml_end_element(s, tag);

	if (lvmagic != 72)
	{
           create_printf("\n\n%s[%d]: FIXME:  out-of-sync\n\n\n", FILE__, __LINE__);
	}

	serialize_printf("%s[%d]:  localVarCollection: ac_serialize_impl, translate done\n", FILE__, __LINE__);

	if (s->isInput())
	{
		//  rebuild name->variable mapping
		for (unsigned int i = 0; i < localVars.size(); ++i)
		{
			localVar *lv = localVars[i];
			assert(lv);
		}
		serialize_printf("%s[%d]:  deserialized %ld local vars\n", FILE__, __LINE__, localVars.size());
	}
	else
		serialize_printf("%s[%d]:  serialized %ld local vars\n", FILE__, __LINE__, localVars.size());

	return NULL;
}
#else
Serializable *localVarCollection::ac_serialize_impl(SerializerBase *, const char *) THROW_SPEC (SerializerError)
{
   return NULL;
}
#endif

// Could be somewhere else... for DWARF-work.
dyn_hash_map<void *, typeCollection *> typeCollection::fileToTypesMap;
#if 0
dyn_hash_map<int, std::vector<std::pair<dataClass, Type **> > > typeCollection::deferred_lookups;
#endif
dyn_hash_map<int, std::vector<std::pair<dataClass, Type **> > *> *deferred_lookups_p = NULL;

void typeCollection::addDeferredLookup(int tid, dataClass tdc,Type **th)
{
	if (!deferred_lookups_p)
		deferred_lookups_p = new dyn_hash_map<int, std::vector<std::pair<dataClass, Type **> > *>();
	dyn_hash_map<int, std::vector<std::pair<dataClass, Type **> > *> &deferred_lookups = *deferred_lookups_p;
	dyn_hash_map<int, std::vector<std::pair<dataClass, Type **> > *>::iterator iter;

	iter = deferred_lookups.find(tid);
	if (iter == deferred_lookups.end())
		deferred_lookups[tid] = new std::vector<std::pair<dataClass, Type **> >();
	deferred_lookups[tid]->push_back(std::make_pair(tdc, th));
}

bool typeCollection::doDeferredLookups(typeCollection *primary_tc)
{
	if (!deferred_lookups_p) return true; // nothing to do
	dyn_hash_map<int, std::vector<std::pair<dataClass, Type **> > *> &deferred_lookups = *deferred_lookups_p;
	bool err = false;
	dyn_hash_map<int, std::vector<std::pair<dataClass, Type **> > *>::iterator iter;
	for (iter = deferred_lookups.begin(); iter != deferred_lookups.end(); iter++)
	{
		std::vector<std::pair<dataClass, Type **> > *to_assign = iter->second;
		if (!to_assign->size())
		{
			continue;
		}

		for (unsigned int i = 0; i < to_assign->size(); ++i)
		{
			dataClass ldc = (*to_assign)[i].first;
			Type **th = (*to_assign)[i].second;

			Type *t = primary_tc->findType(iter->first);
			if (t && (t->getDataClass() != ldc)) t = NULL;

			if (!t)
			{
				if (Symtab::builtInTypes())
				{
					t = Symtab::builtInTypes()->findBuiltInType(iter->first);
					if (t && (t->getDataClass() != ldc)) t = NULL;
				}
			}
			if (!t)
			{
				if (Symtab::stdTypes())
				{
					t = Symtab::stdTypes()->findType(iter->first);
					if (t && (t->getDataClass() != ldc)) t = NULL;
				}
			}
			if (!t)
			{
				int nfound = 0;
				dyn_hash_map<void *, typeCollection *>::iterator tciter; 
				for (tciter = fileToTypesMap.begin(); tciter != fileToTypesMap.end(); tciter++)
				{
					Type *localt = NULL;
					if (tciter->second == primary_tc) continue;
					localt = tciter->second->findType(iter->first);
					if (localt)
					{
						if (localt->getDataClass() != ldc) 
							continue;
						nfound++;
						t = localt;
					}
					//if (t) break;
				}
			}
			if (t)
			{
				*th = t;
			}
			if (!t)
			{
                           create_printf("%s[%d]:  FIXME:  cannot find type id %d\n", 
                                         FILE__, __LINE__, iter->first);
                           err = true;
                           continue;
			}
		}
	}
	deferred_lookups.clear();
	return (!err);
}

/*
 * Reference count
 */

#if 0
typeCollection *typeCollection::getGlobalTypeCollection() 
{
    typeCollection *tc = new typeCollection();
    //tc->refcount++;
    return tc;
}
#endif

typeCollection *typeCollection::getModTypeCollection(Module *mod) 
{
	if (!mod) return NULL;
	dyn_hash_map<void *, typeCollection *>::iterator iter = fileToTypesMap.find((void *)mod);

    if ( iter != fileToTypesMap.end()) 
	{
		return iter->second;
    }

    typeCollection *newTC = new typeCollection();
    fileToTypesMap[(void *)mod] = newTC;
    return newTC;
}

#if 0
void typeCollection::freeTypeCollection(typeCollection *tc) {
    assert(tc);
    tc->refcount--;
    if (tc->refcount == 0) {
        dyn_hash_map<Module *, typeCollection *>::iterator iter = fileToTypesMap.begin();
        for (; iter!= fileToTypesMap.end(); iter++) {
            if (iter->second == tc) {
                fileToTypesMap.erase(iter->first);
                break;
            }
        }
        delete tc;
    }
}
#endif

/*
 * typeCollection::typeCollection
 *
 * Constructor for typeCollection.  Creates the two dictionaries
 * for the type, by Name and ID.
 */
typeCollection::typeCollection() :
	typesByName(),
	globalVarsByName(),
	typesByID(),
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
    // delete all of the types
    for(const auto& t: typesByName) {
        t.second->decrRefCount();
    }

    for(const auto& t: typesByID) {
        t.second->decrRefCount();
    }
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
	else if (Symtab::builtInTypes())
        return Symtab::builtInTypes()->findBuiltInType(name);
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


Type * typeCollection::findOrCreateType( const int ID ) 
{
	if ( typesByID.find(ID) != typesByID.end()) 
	{ 
		return typesByID[ID]; 
	}

	Type * returnType = NULL;

	if ( Symtab::builtInTypes() ) 
	{
		returnType = Symtab::builtInTypes()->findBuiltInType(ID);

		if (returnType)
			return returnType;
	}

	/* Create a placeholder type. */
	returnType = Type::createPlaceholder(ID);
	assert( returnType != NULL );

	/* Having created the type, add it. */
	addType( returnType );

    return returnType;
} /* end findOrCreateType() */

Type *typeCollection::findType(const int ID)
{
	if (typesByID.find(ID) != typesByID.end())
		return typesByID[ID];
	else 
	{
		Type *ret = NULL;

		if (Symtab::builtInTypes()) 
			ret = Symtab::builtInTypes()->findBuiltInType(ID);

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

void typeCollection::addGlobalVariable(std::string &name, Type *type) 
{
   
   globalVarsByName[name] = type;
}

void typeCollection::clearNumberedTypes() 
{
   for (dyn_hash_map<int, Type *>::iterator it = typesByID.begin();
        it != typesByID.end();
        it ++) 
   {
      if (it->second)
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
   //for (dyn_hash_map<int, Type *>::iterator it = typesByID.begin();
   //     it != typesByID.end();
   //     it ++) {
   for (dyn_hash_map<string, Type *>::iterator it = typesByName.begin();
        it != typesByName.end();
        it ++) {
	typesVec->push_back(it->second);
   }
   if(!typesVec->size()){
       delete typesVec;
       return NULL;
   }
   return typesVec;
}

vector<pair<string, Type *> > *typeCollection::getAllGlobalVariables() {
    vector<pair<string, Type *> > *varsVec = new vector<pair<string, Type *> >;
    for(dyn_hash_map<string, Type *>::iterator it = globalVarsByName.begin();
        it != globalVarsByName.end(); it++) {
	varsVec->push_back(pair<string, Type *>(it->first, it->second));
   }	
   if(!varsVec->size()){
       delete varsVec;
       return NULL;
   }
   return varsVec;
}

#if !defined(SERIALIZATION_DISABLED)
Serializable *typeCollection::serialize_impl(SerializerBase *sb, const char *tag) THROW_SPEC (SerializerError)
{
	serialize_printf("%s[%d]:  enter typeCollection::serialize_impl\n", FILE__, __LINE__);

	std::vector<std::pair<std::string, int> >  gvars;
	dyn_hash_map<std::string, Type *>::iterator iter;
	for (iter = globalVarsByName.begin(); iter != globalVarsByName.end(); iter++)
		gvars.push_back(std::make_pair(iter->first, iter->second->getID()));

	std::vector<Type *> ltypes;
	dyn_hash_map<int, Type *>::iterator iter2;
	for (iter2 = typesByID.begin(); iter2 != typesByID.end(); iter2++)
	{
		if (!iter2->second) assert(0);
		//  try skipping field list types
		//if (dynamic_cast<fieldListType *>(iter2->second)) continue;
		assert (iter2->first == iter2->second->getID());
		ltypes.push_back(iter2->second);
	}

	ifxml_start_element(sb, tag);
	//gtranslate(sb, typesByID, "TypesByIDMap", "TypeToIDMapEntry");
	gtranslate(sb, ltypes, "TypesInCollection", "TypeEntry");
	gtranslate(sb, gvars, "GlobalVarNameToTypeMap", "GlobalVarType");
	gtranslate(sb, dwarfParsed_, "DwarfParsedFlag");
	ifxml_end_element(sb, tag);

	if (is_input(sb))
	{
		for (unsigned int i = 0; i < ltypes.size(); ++i)
		{
			typesByID[ltypes[i]->getID()] = ltypes[i];
		}
		doDeferredLookups(this);

		for (unsigned int i = 0; i < gvars.size(); ++i)
		{
			dyn_hash_map<int, Type *>::iterator iter = typesByID.find(gvars[i].second);
			if (iter == typesByID.end())
			{
				serialize_printf("%s[%d]:  cannot find type w/ID %d\n", 
						FILE__, __LINE__, gvars[i].second);
				continue;
			}
			Type *t = iter->second;
			globalVarsByName[gvars[i].first] = t;
		}

		dyn_hash_map<int, Type *>::iterator iter;
		for (iter = typesByID.begin(); iter != typesByID.end(); iter++)
			typesByName[iter->second->getName()] = iter->second;
	}

	serialize_printf("%s[%d]:  leave typeCollection::serialize_impl\n", FILE__, __LINE__);

	return NULL;
}
#else
Serializable *typeCollection::serialize_impl(SerializerBase *, const char *) THROW_SPEC (SerializerError)
{
   return NULL;
}
#endif

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
   dyn_hash_map<std::string, Type *>::iterator bit = builtInTypesByName.begin();
   dyn_hash_map<int, Type *>::iterator bitid = builtInTypesByID.begin();
     
    // delete builtInTypesByName collection
    for(;bit!=builtInTypesByName.end();bit++)
	bit->second->decrRefCount();
    // delete builtInTypesByID collection
    for(;bitid!=builtInTypesByID.end();bitid++)
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
   for (dyn_hash_map<int, Type *>::iterator it = builtInTypesByID.begin();
        it != builtInTypesByID.end();
        it ++) {
	typesVec->push_back(it->second);
   }
   if(!typesVec->size()){
       delete typesVec;
       return NULL;
   }
   return typesVec;
}
