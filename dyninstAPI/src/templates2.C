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


// $Id: templates2.C,v 1.80 2008/06/19 19:53:49 legendre Exp $

#if defined(__XLC__) || defined(__xlC__)
#include "common/h/Dictionary.h"
#else
#pragma implementation "Dictionary.h"
#include "common/src/Dictionary.C"
#endif

#include "symtabAPI/h/Symbol.h"
#include <string>

template class pdvector<std::string>;

template class pdpair<std::string, pdvector<std::string> >;

template class pdvector<pdpair<std::string, pdvector<std::string> > >;

using namespace Dyninst;
using namespace Dyninst::SymtabAPI;
template class  dictionary_hash_iter <Address, Symbol*>;

//template class  dictionary_hash_iter <instPoint*, unsigned>;
//template class  dictionary_hash_iter <std::string, Symbol>;
class pdmodule;
template class  dictionary_hash_iter <std::string, pdmodule *>;

class parse_func;
template class  dictionary_hash_iter <std::string, pdvector<parse_func*>*>;

class mapped_object;
template class  pdvector<mapped_object *> ;

class syscallTrap;
template class pdvector<syscallTrap *>;

/* ***************************************************************************** */

//template class dictionary_hash <std::string, Symbol>;
//template class pdvector<dictionary_hash <std::string, Symbol>::entry>;

class BPatch_point;
template class pdvector<dictionary_hash<Address, BPatch_point *>::entry>;
template class dictionary_hash<Address, BPatch_point *>;

class BPatch_type;
template class dictionary_hash<std::string, BPatch_type *>;
template class dictionary_hash<int, BPatch_type *>;
template class dictionary_hash_iter<std::string, BPatch_type *>;
template class pdvector<dictionary_hash <std::string, BPatch_type *>::entry>;
template class dictionary_hash_iter<int, BPatch_type *>;
template class pdvector<dictionary_hash <int, BPatch_type *>::entry>;
class BPatch_localVar;
template class dictionary_hash<std::string, BPatch_localVar *>;
template class dictionary_hash_iter<std::string, BPatch_localVar *>;
template class pdvector<dictionary_hash <std::string, BPatch_localVar *>::entry>;

template class pdvector<dictionary_hash <unsigned int, Address>::entry>;

class BPatch_basicBlock;
template class  dictionary_hash<Address,BPatch_basicBlock*>;
template class  pdvector<dictionary_hash<Address,BPatch_basicBlock*>::entry>;

#if defined( USES_DWARF_DEBUG )
#include <stack>
template class std::deque< long int >;
template class std::stack< long int >;
#include <map>
template class std::map< unsigned int, char * >;
#endif

class BPatch_typeCollection;
template class dictionary_hash< std::string, BPatch_typeCollection * >;
template class pdvector<dictionary_hash <std::string, BPatch_typeCollection *>::entry>;

template class dictionary_hash< std::string, bool >;
template class pdvector< dictionary_hash< std::string, bool >::entry >;

#if defined(os_aix)
#include <set>
template class std::set< image * >;
#endif

class multiTramp;
template class dictionary_hash<int, multiTramp *>;
template class pdvector<dictionary_hash <int, multiTramp *>::entry>;

class replacedFunctionCall;
template class dictionary_hash<Address, replacedFunctionCall *>;
template class pdvector<dictionary_hash<Address, replacedFunctionCall *>::entry>;


class Statistic;
template class dictionary_hash<std::string, Statistic *>;

class registerSlot;
template class dictionary_hash<unsigned, registerSlot *>;
