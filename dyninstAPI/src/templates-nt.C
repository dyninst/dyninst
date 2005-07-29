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

// $Id: templates-nt.C,v 1.51 2005/07/29 19:19:53 bernat Exp $

/* The VC++ v5.0 compiler (probably correctly) generates warning C4660's 
 * "template class specialization XXXX is already instantiated"
 * however when linking the executable, the linker is unfortunately not
 * able to resolve these external symbols and produces error LNK2001's,
 * therefore the compiler warning is being disabled for this template file.
 */
#pragma warning (disable: 4660)

#include "common/src/vectorSet.C"
#include "common/src/Dictionary.C"
#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/process.h"

template class dictionary_hash<unsigned int, unsigned int>;
template class dictionary_hash<unsigned int, heapItem *>;
template class dictionary_hash<unsigned int, int_function *>;
class image_func;
template class pdvector<image_func *>;
template class dictionary_hash<pdstring, pdvector<image_func *> *>;
template class dictionary_hash<Address, image_func *>;

class int_variable;
template class pdvector<int_variable *>;
template class dictionary_hash<pdstring, int_variable *>;
template class dictionary_hash<pdstring, pdvector<int_variable *> *>;
template class dictionary_hash<Address, int_variable *>;

class image_variable;
template class pdvector<image_variable *>;
template class dictionary_hash<pdstring, image_variable *>;
template class dictionary_hash<pdstring, pdvector<image_variable *> *>;
template class dictionary_hash<Address, image_variable *>;

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

class fileDescriptor;
template class pdvector<fileDescriptor>;

template class dictionary_hash<pdstring, unsigned int>;
template class dictionary_hash<pdstring, pdstring>;
template class dictionary_hash<pdstring, Symbol>;
template class dictionary_hash<pdstring, pdmodule *>;
template class dictionary_hash<pdstring, int_function *>;
template class dictionary_hash<pdstring, internalSym *>;

template class dictionary_hash<pdstring, pdvector<pdstring> *>;
template class dictionary_hash<pdstring, pdvector<int_function *> *>;

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

#include "common/h/String.h"
#include "dyninstAPI/h/BPatch_thread.h"
#include "dyninstAPI/h/BPatch_type.h"

template class dictionary_hash<pdstring, BPatch_type *>;
template class dictionary_hash<int, BPatch_thread *>;
template class dictionary_hash<int, BPatch_type *>;
template class dictionary_hash<pdstring, BPatch_localVar *>;
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

#if defined(BPATCH_LIBRARY)
class BPatch_basicBlock;

template class  dictionary_hash<Address,BPatch_basicBlock*>;
template class  pdvector<dictionary_hash<Address,BPatch_basicBlock*>::entry>;

template class BPatch_Vector<BPatch_frame>;
#endif

// Library callback templates
template class dictionary_hash<pdstring, libraryCallback *>;
template class pdvector<dictionary_hash <pdstring, libraryCallback *>::entry>;

template class dictionary_hash<pdstring,supportedLanguages>;
template class pdvector<dictionary_hash<pdstring,supportedLanguages>::entry>;

template class pdvector< Symbol >;
template class dictionary_hash< pdstring, pdvector< Symbol > >;

template class dictionary_hash<int, BPatch_process *>;
template class pdvector<dictionary_hash <int, BPatch_process *>::entry>;

template class dictionary_hash<const int_function *, BPatch_function *>;
template class pdvector<dictionary_hash <const int_function *, BPatch_function *>::entry>;

class replacedFunctionCall;
template class pdvector<replacedFunctionCall *>;
