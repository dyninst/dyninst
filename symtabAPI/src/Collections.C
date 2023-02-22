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

#include "common/src/headers.h"

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
localVarCollection::~localVarCollection(){
   auto li = localVars.begin();
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
	if (!addItem_impl(var))
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

   auto li = localVars.begin();
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
const dyn_c_vector<localVar *> &localVarCollection::getAllVars() const
{
    return localVars;
}

// Could be somewhere else... for DWARF-work.
dyn_c_hash_map<void *, typeCollection *> typeCollection::fileToTypesMap;
dyn_hash_map<int, std::vector<std::pair<dataClass, boost::shared_ptr<Type>*> > *> *deferred_lookups_p = NULL;

void typeCollection::addDeferredLookup(int tid, dataClass tdc, boost::shared_ptr<Type>*th)
{
	if (!deferred_lookups_p)
		deferred_lookups_p = new dyn_hash_map<int, std::vector<std::pair<dataClass, boost::shared_ptr<Type>*> > *>();
	auto& deferred_lookups = *deferred_lookups_p;

	auto iter = deferred_lookups.find(tid);
	if (iter == deferred_lookups.end())
		deferred_lookups[tid] = new std::vector<std::pair<dataClass, boost::shared_ptr<Type> *> >();
	deferred_lookups[tid]->push_back(std::make_pair(tdc, th));
}

bool typeCollection::doDeferredLookups(typeCollection *primary_tc)
{
	if (!deferred_lookups_p) return true; // nothing to do
	dyn_hash_map<int, std::vector<std::pair<dataClass, boost::shared_ptr<Type>*> > *> &deferred_lookups = *deferred_lookups_p;
	bool err = false;
	dyn_hash_map<int, std::vector<std::pair<dataClass, boost::shared_ptr<Type>*> > *>::iterator iter;
	for (iter = deferred_lookups.begin(); iter != deferred_lookups.end(); iter++)
	{
		std::vector<std::pair<dataClass, boost::shared_ptr<Type>*> > *to_assign = iter->second;
		if (!to_assign->size())
		{
			continue;
		}

		for (unsigned int i = 0; i < to_assign->size(); ++i)
		{
			dataClass ldc = (*to_assign)[i].first;
			boost::shared_ptr<Type>*th = (*to_assign)[i].second;

			boost::shared_ptr<Type> t = primary_tc->findType(iter->first, Type::share);
			if (t && (t->getDataClass() != ldc)) t = NULL;

			if (!t)
			{
				if (Symtab::builtInTypes())
				{
					t = Symtab::builtInTypes()->findBuiltInType(iter->first, Type::share);
					if (t && (t->getDataClass() != ldc)) t = NULL;
				}
			}
			if (!t)
			{
				if (Symtab::stdTypes())
				{
					t = Symtab::stdTypes()->findType(iter->first, Type::share);
					if (t && (t->getDataClass() != ldc)) t = NULL;
				}
			}
			if (!t)
			{
				dyn_c_hash_map<void *, typeCollection *>::iterator tciter;
				for (tciter = fileToTypesMap.begin(); tciter != fileToTypesMap.end(); tciter++)
				{
					if (tciter->second == primary_tc) continue;
					boost::shared_ptr<Type> localt = tciter->second->findType(iter->first, Type::share);
					if (localt)
					{
						if (localt->getDataClass() != ldc)
							continue;
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

typeCollection *typeCollection::getModTypeCollection(Module *mod)
{
    if (!mod) return NULL;
    {  // Fast-path
        dyn_c_hash_map<void *, typeCollection *>::const_accessor ca;
        if(fileToTypesMap.find(ca, (void *)mod))
          return ca->second;
    }
    dyn_c_hash_map<void *, typeCollection *>::accessor a;
    if(fileToTypesMap.insert(a, (void *)mod)) {
        a->second = new typeCollection();
    }
    return a->second;
}


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
typeCollection::~typeCollection() {}

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
boost::shared_ptr<Type> typeCollection::findType(std::string name, Type::do_share_t)
{
    dyn_c_hash_map<std::string, boost::shared_ptr<Type>>::const_accessor a;

    if (typesByName.find(a, name))
    	return a->second;
    else if (Symtab::builtInTypes())
        return Symtab::builtInTypes()->findBuiltInType(name, Type::share);
    else
        return boost::shared_ptr<Type>();
}

boost::shared_ptr<Type> typeCollection::findTypeLocal(std::string name, Type::do_share_t)
{
    dyn_c_hash_map<std::string, boost::shared_ptr<Type>>::const_accessor a;

    if (typesByName.find(a, name))
        return a->second;
    else
        return boost::shared_ptr<Type>();
}

boost::shared_ptr<Type> typeCollection::findTypeLocal(const int ID, Type::do_share_t)
{
    dyn_c_hash_map<int, boost::shared_ptr<Type>>::const_accessor a;
    if (typesByID.find(a, ID))
        return a->second;
    else
        return boost::shared_ptr<Type>();
}


boost::shared_ptr<Type> typeCollection::findOrCreateType( const int ID, Type::do_share_t)
{
    dyn_c_hash_map<int, boost::shared_ptr<Type>>::const_accessor ca;
    if (typesByID.find(ca, ID))
    {
        return ca->second;
    }

    if ( Symtab::builtInTypes() )
    {
        boost::shared_ptr<Type> returnType = Symtab::builtInTypes()->findBuiltInType(ID, Type::share);

        if (returnType)
            return returnType;
    }

    // If someone else added a placeholder in the meanwhile, return that.
    // Note: name == "", so typesByName doesn't need updated.
    dyn_c_hash_map<int, boost::shared_ptr<Type>>::accessor a;
    if (!typesByID.insert(a, {ID, boost::shared_ptr<Type>()}))
    {
      return a->second;
    }

    /* Create a placeholder type. */
    a->second = Type::createPlaceholder(ID);
    assert( a->second );

    return a->second;
} /* end findOrCreateType() */

boost::shared_ptr<Type> typeCollection::findType(const int ID, Type::do_share_t)
{
    dyn_c_hash_map<int, boost::shared_ptr<Type>>::const_accessor a;
    if (typesByID.find(a, ID))
        return a->second;
    else
    {
        boost::shared_ptr<Type> ret = NULL;

        if (Symtab::builtInTypes())
            ret = Symtab::builtInTypes()->findBuiltInType(ID, Type::share);

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
boost::shared_ptr<Type> typeCollection::findVariableType(std::string &name, Type::do_share_t)
{
    dyn_c_hash_map<std::string, boost::shared_ptr<Type>>::const_accessor a;
    if (globalVarsByName.find(a, name))
        return a->second;
    else
        return boost::shared_ptr<Type>();
}

/*
 * typeCollection::addType
 *
 * Add a new type to the type collection.  Note that when a type is added to
 * the collection, it becomes the collection's responsibility to delete it
 * when it is no longer needed.  For one thing, this means that a type
 * allocated on the stack should *NEVER* be put into a typeCollection.
 */
void typeCollection::addType(boost::shared_ptr<Type> type)
{
    dyn_c_hash_map<int, boost::shared_ptr<Type>>::accessor a;
    if(!typesByID.insert({type->getID(), type}))
      return;  // Type is already present
    if(type->getName() != "") { //Type could have no name.
        typesByName.insert({type->getName(), type});
    }
}

void typeCollection::addGlobalVariable(boost::shared_ptr<Type> type)
{
    globalVarsByName.insert({type->getName(), type});
}

void typeCollection::clearNumberedTypes()
{
   typesByID.clear();
}

/*
 * localVarCollection::getAllVars()
 * this function returns all the local variables in the collection.
 */
void typeCollection::getAllTypes(std::vector<boost::shared_ptr<Type>>& vec) {
   for (auto it = typesByName.begin();
        it != typesByName.end();
        it ++) {
	vec.push_back(it->second);
   }
}

void typeCollection::getAllGlobalVariables(vector<pair<string, boost::shared_ptr<Type>>>& vec) {
    for(auto it = globalVarsByName.begin();
        it != globalVarsByName.end(); it++) {
	vec.push_back(make_pair(it->first, it->second));
   }
}

/*
 * builtInTypeCollection::builtInTypeCollection
 *
 * Constructor for builtInTypeCollection.  Creates adictionary
 * for the builtInType, by Name and ID.
 *  XXX- Don't know if a collection is needed for types by name, but
 * it is created just in case. jdd 4/21/99
 */
builtInTypeCollection::builtInTypeCollection():
  builtInTypesByID(),
  builtInTypesByName()
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
boost::shared_ptr<Type> builtInTypeCollection::findBuiltInType(std::string &name, Type::do_share_t)
{
    dyn_c_hash_map<std::string, boost::shared_ptr<Type>>::const_accessor a;
    if (builtInTypesByName.find(a, name))
       return a->second;
    else
       return boost::shared_ptr<Type>();
}

boost::shared_ptr<Type> builtInTypeCollection::findBuiltInType(const int ID, Type::do_share_t)
{
    dyn_c_hash_map<int, boost::shared_ptr<Type>>::const_accessor a;
    if (builtInTypesByID.find(a, ID))
       return a->second;
    else
       return boost::shared_ptr<Type>();
}

void builtInTypeCollection::addBuiltInType(boost::shared_ptr<Type> type)
{
  if(type->getName() != "") { //Type could have no name.
    {
      builtInTypesByName.insert({type->getName(), type});
    }

  //All built-in types have unique IDs so far jdd 4/21/99
    {
      builtInTypesByID.insert({type->getID(), type});
    }
  }
}

void builtInTypeCollection::getAllBuiltInTypes(std::vector<boost::shared_ptr<Type>>& vec) {
   for (auto it = builtInTypesByID.begin();
       it != builtInTypesByID.end();
       it ++) {
     vec.push_back(it->second);
   }
}
