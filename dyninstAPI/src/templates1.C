/*
 * Copyright (c) 1996-2009 Barton P. Miller
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

//$Id: templates1.C,v 1.83 2008/06/19 19:53:48 legendre Exp $

#if defined(__XLC__) || defined(__xlC__)
#pragma implementation("Dictionary.h")
#else
#pragma implementation "Dictionary.h"
#endif
#include "common/src/Dictionary.C"

#include <string>

//begin from templates05
//end from templates05

#include "dyninstAPI/h/BPatch_Set.h"
#include "dyninstAPI/h/BPatch_type.h"
#include "dyninstAPI/h/BPatch_Vector.h"
#include "dyninstAPI/h/BPatch_thread.h"

class BPatch_thread;
class BPatch_field;
class BPatch_variableExpr;

#include "symtab.h" // supportedLanguages is a typedef; could move.

using namespace Dyninst;
using namespace Dyninst::SymtabAPI;

template class  dictionary_hash <std::string, supportedLanguages>;
template class  pdvector<dictionary_hash <std::string, supportedLanguages>::entry>;

template class  dictionary_hash <Address, unsigned>;
// symbolNamesByAddr
template class  dictionary_hash <Address, std::string>;
template class  pdvector<dictionary_hash <Address, unsigned>::entry>;
template class  dictionary_hash_iter <Address, unsigned>;
template class  dictionary_hash <std::string, unsigned>;
template class  pdvector<dictionary_hash <std::string, unsigned>::entry>;
template class  pdvector<dictionary_hash <Address, std::string>::entry>;

#include "common/src/List.C"
using namespace Dyninst;
using namespace Dyninst::SymtabAPI;
template class  dictionary_hash <Address, Symbol*>;
template class  pdvector<dictionary_hash <Address, Symbol*>::entry>;

class instPoint;
template class  dictionary_hash <Address, instPoint*>;
template class  pdvector<dictionary_hash <Address, instPoint *>::entry>;

class pdmodule;
template class  dictionary_hash <std::string, pdmodule *>;
template class  pdvector<dictionary_hash <std::string, pdmodule *>::entry>;

class int_function;
template class  dictionary_hash <std::string, pdvector<int_function*>*>;
template class  pdvector<dictionary_hash <std::string, pdvector<int_function*>*>::entry>;

class int_variable;
template class  dictionary_hash <std::string, pdvector<int_variable*>*>;
template class  pdvector<dictionary_hash <std::string, pdvector<int_variable*> *>::entry>;

class image_func;
template class  dictionary_hash <std::string, pdvector<image_func*> *>;
template class  pdvector<dictionary_hash <std::string, pdvector<image_func*> *>::entry>;
template class  dictionary_hash <Address, image_func*>;
template class  pdvector<dictionary_hash <Address, image_func*>::entry>;
template class  dictionary_hash<const image_func *, int_function *>;
template class  pdvector<dictionary_hash<const image_func *, int_function *>::entry>;

class image_variable;
template class  dictionary_hash <Address, image_variable*>;
template class  pdvector<dictionary_hash <Address, image_variable*>::entry>;
template class  dictionary_hash <std::string, pdvector<image_variable*>*>;
template class  pdvector<dictionary_hash <std::string, pdvector<image_variable*> *>::entry>;
template class  dictionary_hash<const image_variable *, int_variable *>;
template class  pdvector<dictionary_hash<const image_variable *, int_variable *>::entry>;

class heapItem;
template class  dictionary_hash <Address, heapItem*>;
template class  pdvector<dictionary_hash <Address, heapItem*>::entry>;

class BPatch_process;
template class dictionary_hash<int, BPatch_process *>;
template class pdvector<dictionary_hash <int, BPatch_process *>::entry>;

class BPatch_function;
template class dictionary_hash<const int_function *, BPatch_function *>;
template class pdvector<dictionary_hash <const int_function *, BPatch_function *>::entry>;
template class  dictionary_hash <int_function*, BPatch_function*>;
template class  pdvector<dictionary_hash<int_function*, BPatch_function*>::entry>;

class BPatch_variableExpr;
template class  dictionary_hash <Address, BPatch_variableExpr*>;
template class  pdvector<dictionary_hash <Address, BPatch_variableExpr*>::entry>;

#include "BPatch_frame.h"
template class BPatch_Vector<BPatch_frame>;

class BPatch_point;
template class dictionary_hash <const instPoint *, BPatch_point *>;
template class pdvector<dictionary_hash<const instPoint *, BPatch_point *>::entry>;
template class dictionary_hash_iter<const instPoint *, BPatch_point *>;

template class  dictionary_hash <unsigned, unsigned>;
template class  pdvector<dictionary_hash <unsigned, unsigned>::entry>;
template class  dictionary_hash <int, int>;
template class  pdvector<dictionary_hash <int, int>::entry>;

template class  dictionary_hash <unsigned, Address>;
template class  dictionary_hash <Address, Address>;
template class  pdvector<dictionary_hash <Address, Address>::entry>;
template class  dictionary_hash_iter <Address, Address>;

#if defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) \
 || defined(i386_unknown_solaris2_4) \
 || defined(sparc_sun_solaris2_4)
class Elf_X_Shdr;
template class pdvector<Elf_X_Shdr *>;
#endif

class image_basicBlock;
template class dictionary_hash<Address, image_basicBlock *>;
template class pdvector<dictionary_hash<Address, image_basicBlock *>::entry>;

class relocatedCode;
template class dictionary_hash<Address, relocatedCode *>;
template class pdvector<dictionary_hash<Address, relocatedCode *>::entry>;

class int_basicBlock;
template class BPatch_Set<int_basicBlock *>;

class dominatorBB;
template class  dictionary_hash <unsigned, dominatorBB *>;
template class  pdvector<dictionary_hash<unsigned, dominatorBB *>::entry >;
template class  BPatch_Vector<dominatorBB *>;
template class  BPatch_Set<dominatorBB *>;

class image_basicBlock;
template class  BPatch_Set<image_basicBlock *>;

class image_edge;
template class  pdvector<image_edge*>;

#include "ast.h"
template class dictionary_hash<AstNode *, regTracker_t::commonExpressionTracker>;
template class pdvector<dictionary_hash<AstNode *, regTracker_t::commonExpressionTracker>::entry >;

template class  pdvector<dictionary_hash<std::string, Address>::entry >;
template class dictionary_hash<std::string, Address>;
