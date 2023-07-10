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
#include <assert.h>
#include <string>
#include <string.h>
#include <utility>

using namespace Dyninst;
using namespace SymtabAPI;

template<class T>
T *upgradePlaceholder(Type *placeholder, T *new_type)
{
    assert(sizeof(T) <= Type::max_size);
    memset(static_cast<void*>(placeholder), 0, Type::max_size);
    T* ret = new(placeholder) T{};
    assert(static_cast<void*>(placeholder) == static_cast<void*>(ret));
    *ret = *new_type;
    return ret;
}

template<class T>
typename boost::enable_if<
    boost::integral_constant<bool, !bool(boost::is_same<Type, T>::value)>,
boost::shared_ptr<Type>>::type typeCollection::addOrUpdateType(boost::shared_ptr<T> type)
{
	//Instanciating this function for 'Type' would be a mistake, which
	//the following assert tries to guard against.  If you trigger this,
	//then a caller to this function is likely using 'Type'.  Change
	//this to a more specific call, e.g. typeFunction instead of Type
    // NOTE: Disabled, we use SFINAE instead to handle this.
    // BOOST_STATIC_ASSERT(sizeof(T) != sizeof(Type));

    dyn_c_hash_map<int, boost::shared_ptr<Type>>::accessor a;
	if (typesByID.insert(a, {type->getID(), type}))
	{
		if ( type->getName() != "" )
			typesByName.insert({type->getName(), type});
		return type;
	}

	/* Multiple inclusions of the same object file can result
	   in us parsing the same module types repeatedly. GCC does this
	   with some of its internal routines */

	T *existingT = dynamic_cast<T*>(a->second.get());
	if (existingT && (*existingT == *type))
	{
		return a->second;
	}

	if (a->second->getDataClass() == dataUnknownType)
	{
		upgradePlaceholder(a->second.get(), type.get());
	}
	else
	{
		/* Merge the type information. */
		a->second->merge(type.get());
	}

	/* The type may have gained a name. */
	if ( a->second->getName() != "")
	{
        dyn_c_hash_map<std::string, boost::shared_ptr<Type>>::accessor o;
		if (typesByName.find(o, a->second->getName()))
		{
			if (a->second != o->second)
			{
                o->second = a->second;
			}
		}
		else
		{
            typesByName.insert({a->second->getName(), a->second});
		}
	}

	/* Tell the parser to update its type pointer. */
	return a->second;
} /* end addOrUpdateType() */


#endif
