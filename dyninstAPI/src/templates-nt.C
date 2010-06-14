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
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/ast.h"

template class dictionary_hash<unsigned int, unsigned int>;
template class dictionary_hash<unsigned int, heapItem *>;
template class dictionary_hash<unsigned int, int_function *>;
class image_func;
template class pdvector<image_func *>;
template class dictionary_hash<std::string, pdvector<image_func *> *>;
template class dictionary_hash<Address, image_func *>;
template class dictionary_hash<const image_func *, int_function *>;

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

class image_basicBlock;
template class dictionary_hash<Address, image_basicBlock *>;
template class  BPatch_Set<image_basicBlock *>;

class fileDescriptor;
template class pdvector<fileDescriptor>;

template class dictionary_hash<std::string, unsigned int>;
template class dictionary_hash<std::string, std::string>;
template class dictionary_hash<std::string, SymtabAPI::Symbol>;
template class dictionary_hash<std::string, pdmodule *>;
template class dictionary_hash<std::string, int_function *>;
//template class dictionary_hash<std::string, internalSym *>;

template class dictionary_hash<std::string, pdvector<std::string> *>;
template class dictionary_hash<std::string, pdvector<int_function *> *>;

class BPatch_typeCollection;
template class dictionary_hash<std::string, BPatch_typeCollection *>;

#include "dyninstAPI/src/rpcMgr.h"
template class  dictionary_hash<unsigned, rpcLWP *>;
template class  pdvector<dictionary_hash <unsigned, rpcLWP *>::entry>;
template class  pdvector<rpcThr *>;
template class pdvector<inferiorRPCtoDo *>;
template class pdvector<inferiorRPCinProgress *>;

#include "common/src/List.C"
template class List<miniTramp*>;
template class ListBase<miniTramp*, void*>;
template class dictionary_hash<instPoint const *, baseTramp *>;

#include "dyninstAPI/h/BPatch_thread.h"
#include "dyninstAPI/h/BPatch_type.h"

template class dictionary_hash<std::string, BPatch_type *>;
template class dictionary_hash<int, BPatch_thread *>;
template class dictionary_hash<int, BPatch_type *>;
template class dictionary_hash<std::string, BPatch_localVar *>;
template class dictionary_hash<int_function*, BPatch_function*>;
template class  dictionary_hash <Address, BPatch_variableExpr*>;
template class dictionary_hash<Address, BPatch_point *>;

template class dictionary_hash<u_int, Address>;
template class dictionary_hash<Address, Address>;
template class dictionary_hash<Address, heapItem *>;
template class dictionary_hash<Address, int_function *>;
template class dictionary_hash<Address, unsigned>;

#include "dyn_lwp.h"
template class dictionary_hash<unsigned, dyn_lwp *>;

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

template class dictionary_hash<const int_function *, BPatch_function *>;
template class pdvector<dictionary_hash <const int_function *, BPatch_function *>::entry>;

class replacedFunctionCall;
template class pdvector<replacedFunctionCall *>;

class dominatorBB;
template class  dictionary_hash <unsigned, dominatorBB *>;
template class  pdvector<dictionary_hash<unsigned, dominatorBB *>::entry >;
template class  BPatch_Vector<dominatorBB *>;
template class  BPatch_Set<dominatorBB *>;

#include "callbacks.h"
#include "signalhandler.h"
#include "mailbox.h"
template class dictionary_hash< eventType, pdvector< CallbackBase * > >;
template class pdvector<dictionary_hash < eventType, pdvector <CallbackBase *> >::entry>;

class EventGate;

template class pdvector<CallbackBase *>;
template class pdvector<eventLock::lock_stack_elem>;
template class pdvector<EventGate *>;
template class pdvector<SignalHandler *>;


class image_edge;
template class  pdvector<image_edge*>;

template class  dictionary_hash <int, int>;
template class  pdvector<dictionary_hash<int,int>::entry >;

template class  dictionary_hash <unsigned long, std::string>;
template class  pdvector<dictionary_hash<unsigned long,std::string>::entry >;

template class dictionary_hash<Address, threadmap_t *>;
template class pdvector<dictionary_hash<Address, threadmap_t *>::entry>;

class relocatedCode;
template class dictionary_hash<Address, relocatedCode *>;
template class pdvector<dictionary_hash<Address, relocatedCode *>::entry>;

template class dictionary_hash<AstNode *, regTracker_t::commonExpressionTracker>;
template class pdvector<dictionary_hash<AstNode *, regTracker_t::commonExpressionTracker>::entry>;

class Statistic;
template class dictionary_hash<std::string, Statistic *>;

class registerSlot;
template class dictionary_hash<unsigned, registerSlot *>;
