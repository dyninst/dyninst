/*
 * Copyright (c) 1996 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
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

#include "BPatch_type.h"


/*
 * BPatch_type::BPatch_type
 *
 * Constructor for BPatch_type.  Creates a type object representing a type
 * with the features given in the parameters.
 */
BPatch_type::BPatch_type(const char *_name, bool _nullType) :
    nullType(_nullType), name(_name)
{
    /* XXX More later. */
}


/*
 * BPatch_type::isCompatible
 *
 * Returns true of the type is compatible with the other specified type, false
 * if it is not.
 *
 * oType	The other type to check for compatibility.
 */
bool BPatch_type::isCompatible(const BPatch_type &otype)
{
    if (nullType || otype.nullType)
	return true;

    // XXX Just compare names for now, we'll have to fix this later.
    if (name == otype.name) return true;
    else return false;
}


/*
 * BPatch_typeCollection::~BPatch_typeCollection
 *
 * Destructor for BPatch_typeCollection.  Deletes all type objects that have
 * been inserted into the collection.
 */
BPatch_typeCollection::~BPatch_typeCollection()
{
    dictionary_hash_iter<string, BPatch_type *> ti(typesByName);

    string	name;
    BPatch_type	*type;

    while (ti.next(name, type))
	delete type;
}


/*
 * BPatch_typeCollection::findType
 *
 * Retrieve a pointer to a BPatch_type object representing the named type from
 * the collection.  If no such type exists and no such type can be derived
 * from existing types, then the function returns NULL.
 *
 * name		The name of the type to look up.
 */
BPatch_type *BPatch_typeCollection::findType(const char *name)
{
    if (typesByName.defines(name))
    	return typesByName[name];
    else
	return NULL;
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
    typesByName[type->getName()] = type;
}
