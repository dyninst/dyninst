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

// $Id: templates-nt.C,v 1.69 2008/02/23 02:09:11 jaw Exp $

/* The VC++ v5.0 compiler (probably correctly) generates warning C4660's 
 * "template class specialization XXXX is already instantiated"
 * however when linking the executable, the linker is unfortunately not
 * able to resolve these external symbols and produces error LNK2001's,
 * therefore the compiler warning is being disabled for this template file.
 */
#pragma warning (disable: 4660)

#include <string>
#include "common/src/Dictionary.C"
#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/h/BPatch_Set.h"

template class dictionary_hash<unsigned int, unsigned int>;
template class dictionary_hash<unsigned int, heapItem *>;
template class dictionary_hash<unsigned int, func_instance *>;
class parse_func;
template class pdvector<parse_func *>;
template class dictionary_hash<std::string, pdvector<parse_func *> *>;
template class dictionary_hash<Address, parse_func *>;
template class dictionary_hash<const parse_func *, func_instance *>;

class int_variable;
template class pdvector<int_variable *>;
template class dictionary_hash<std::string, int_variable *>;
template class dictionary_hash<std::string, pdvector<int_variable *> *>;
template class dictionary_hash<Address, int_variable *>;

class image_variable;
template class pdvector<image_variable *>;
template class dictionary_hash<std::string, image_variable *>;
template class dictionary_hash<std::string, pdvector<image_variable *> *>;
template class dictionary_hash<Address, image_variable *>;
template class dictionary_hash<const image_variable *, int_variable *>;

class instPoint;
template class dictionary_hash<Address, instPoint *>;

class multiTramp;
template class dictionary_hash<int, multiTramp *>;

class replacedFunctionCall;
template class dictionary_hash<Address, replacedFunctionCall *>;

class relocatedInstruction;
template class dictionary_hash<Address, relocatedInstruction *>;

class BPatch_point;
template class dictionary_hash<const instPoint *, BPatch_point *>;

class parse_block;
template class dictionary_hash<Address, parse_block *>;
template class  BPatch_Set<parse_block *>;

class fileDescriptor;
template class pdvector<fileDescriptor>;

template class dictionary_hash<std::string, unsigned int>;
template class dictionary_hash<std::string, std::string>;
template class dictionary_hash<std::string, SymtabAPI::Symbol>;
template class dictionary_hash<std::string, pdmodule *>;
template class dictionary_hash<std::string, func_instance *>;
//template class dictionary_hash<std::string, internalSym *>;

template class dictionary_hash<std::string, pdvector<std::string> *>;
template class dictionary_hash<std::string, pdvector<func_instance *> *>;

class BPatch_typeCollection;
template class dictionary_hash<std::string, BPatch_typeCollection *>;


#include "dyninstAPI/h/BPatch_thread.h"
#include "dyninstAPI/h/BPatch_type.h"

template class dictionary_hash<std::string, BPatch_type *>;
template class dictionary_hash<int, BPatch_thread *>;
template class dictionary_hash<int, BPatch_type *>;
template class dictionary_hash<std::string, BPatch_localVar *>;
template class dictionary_hash<func_instance*, BPatch_function*>;
template class  dictionary_hash <Address, BPatch_variableExpr*>;
template class dictionary_hash<Address, BPatch_point *>;

template class dictionary_hash<u_int, Address>;
template class dictionary_hash<Address, Address>;
template class dictionary_hash<Address, heapItem *>;
template class dictionary_hash<Address, func_instance *>;
template class dictionary_hash<Address, unsigned>;

class BPatch_basicBlock;

template class  dictionary_hash<Address,BPatch_basicBlock*>;
template class  pdvector<dictionary_hash<Address,BPatch_basicBlock*>::entry>;

template class BPatch_Vector<BPatch_frame>;

template class dictionary_hash<Address, unsigned char>;
template class pdvector<dictionary_hash <Address, unsigned char>::entry >;

template class dictionary_hash<std::string,SymtabAPI::supportedLanguages>;
template class pdvector<dictionary_hash<std::string,SymtabAPI::supportedLanguages>::entry>;

template class pdvector< SymtabAPI::Symbol >;
template class dictionary_hash< std::string, pdvector< SymtabAPI::Symbol > >;

template class dictionary_hash<int, BPatch_process *>;
template class pdvector<dictionary_hash <int, BPatch_process *>::entry>;

template class dictionary_hash<const func_instance *, BPatch_function *>;
template class pdvector<dictionary_hash <const func_instance *, BPatch_function *>::entry>;

class replacedFunctionCall;
template class pdvector<replacedFunctionCall *>;

class dominatorBB;
template class  dictionary_hash <unsigned, dominatorBB *>;
template class  pdvector<dictionary_hash<unsigned, dominatorBB *>::entry >;
template class  BPatch_Vector<dominatorBB *>;
template class  BPatch_Set<dominatorBB *>;


class EventGate;

template class pdvector<EventGate *>;

template class  dictionary_hash <int, int>;
template class  pdvector<dictionary_hash<int,int>::entry >;

template class  dictionary_hash <unsigned long, std::string>;
template class  pdvector<dictionary_hash<unsigned long,std::string>::entry >;

class registerSlot;
template class dictionary_hash<unsigned int, registerSlot*>;

template class dictionary_hash<AstNode *, regTracker_t::commonExpressionTracker>;
template class pdvector<dictionary_hash<AstNode *, regTracker_t::commonExpressionTracker>::entry>;

class Statistic;
template class dictionary_hash<std::string, Statistic *>;

#include "dyninstAPI/src/block.h"
template class ParseAPI::ContainerWrapper<
       std::vector<edge_instance *>,
       edge_instance *,
       edge_instance *,
       EdgePredicateAdapter>;