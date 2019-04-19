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

#if !defined(TYPE_MEM_H_)
#define TYPE_MEM_H_

#include "symtabAPI/h/Type.h"
#include "boost/static_assert.hpp"
#include <utility>

namespace Dyninst {
  namespace SymtabAPI {
    extern dyn_c_hash_map<void *, size_t> type_memory;
  }
}

using namespace Dyninst;
using namespace SymtabAPI;

template<class T>
T *upgradePlaceholder(Type *placeholder, T *new_type)
{
  void *mem = (void *) placeholder;
  size_t size = 0;
  // acquire(type_memory);
  {
    dyn_c_hash_map<void*, size_t>::accessor a;
    assert(type_memory.find(a, placeholder));
    size = a->second;
  }
  // release(type_memory);

  assert(sizeof(T) < size);
  memset(mem, 0, size);

  T *ret = new(mem) T();
  assert(mem == (void *) ret);
  *ret = *new_type;
  // forget(ret, sizeof(T));
  return ret;
}

template<class T>
T* typeCollection::addOrUpdateType(T *type) 
{
	//Instanciating this function for 'Type' would be a mistake, which
	//the following assert tries to guard against.  If you trigger this,
	//then a caller to this function is likely using 'Type'.  Change
	//this to a more specific call, e.g. typeFunction instead of Type
	BOOST_STATIC_ASSERT(sizeof(T) != sizeof(Type));
    boost::lock_guard<boost::mutex> g(placeholder_mutex);

	Type *existingType = findTypeLocal(type->getID());
    dyn_c_hash_map<int, Type*>::accessor id_accessor;
    dyn_c_hash_map<std::string, Type*>::accessor name_accessor;
	if (!existingType) 
	{
		if ( type->getName() != "" ) 
		{
			typesByName.insert(name_accessor, std::make_pair(type->getName(), type));
		}
		typesByID.insert(id_accessor, std::make_pair(type->getID(), type));
		type->incrRefCount();
		return type;
	}

	/* Multiple inclusions of the same object file can result
	   in us parsing the same module types repeatedly. GCC does this
	   with some of its internal routines */

	T *existingT = dynamic_cast<T*>(existingType);
	if (existingT && (*existingT == *type)) 
	{
		return (T*) existingType;
	}

	if (existingType->getDataClass() == dataUnknownType) 
	{
		upgradePlaceholder(existingType, type);
	} 
	else 
	{
		/* Merge the type information. */
		existingType->merge(type);
	}

	/* The type may have gained a name. */
	if ( existingType->getName() != "") 
	{
		dyn_c_hash_map<std::string, Type*>::accessor a;
		bool found = typesByName.find(a, existingType->getName());
		if (found)
		{
			if (a->second != existingType)
			{
				a->second->decrRefCount();
				a->second = existingType;
				existingType->incrRefCount();
			}
		} 
		else 
		{
			typesByName.insert(a, std::make_pair(existingType->getName(), existingType));
			existingType->incrRefCount();
		}
	}

	/* Tell the parser to update its type pointer. */
	return (T*) existingType;
} /* end addOrUpdateType() */


#endif
