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

//$Id: templates1.C,v 1.64 2005/04/18 20:55:51 legendre Exp $

#if defined(__XLC__) || defined(__xlC__)
#pragma implementation("Dictionary.h")
#else
#pragma implementation "Dictionary.h"
#endif
#include "common/src/Dictionary.C"

#include "common/h/String.h"

#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/dyninstP.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/util.h"
#include "dyninstAPI/src/Object.h"
#include "dyninstAPI/src/sharedobject.h"
#include "common/h/List.h"
#include "dyninstAPI/src/FunctionExpansionRecord.h"

class dyn_lwp;

//begin from templates05
template class pdvector<instWaitingList *>;
template class refCounter<string_ll>;
template class pdvector<heapDescriptor>;
//end from templates05

#if defined(BPATCH_LIBRARY)
#include "dyninstAPI/h/BPatch_Set.h"
#include "dyninstAPI/h/BPatch_type.h"
#include "dyninstAPI/h/BPatch_Vector.h"
#include "dyninstAPI/h/BPatch_thread.h"

class BPatch_thread;
class BPatch_field;
class BPatch_variableExpr;
#endif

#if defined(mips_sgi_irix6_4)
#include "dyninstAPI/src/irixDL.h" // dsoEvent_t
template class  pdvector<pdDsoEvent *>;
template class  pdvector<pdElfObjInfo *>;
template class  pdvector<pdvector<int> >;
template class  pdvector<pdElfSym *>;
#endif
template class  dictionary_hash <Address, unsigned>;
// symbolNamesByAddr
template class  dictionary_hash <Address, pdstring>;

template class  pdvector<dictionary_hash <Address, unsigned>::entry>;
template class  dictionary_hash_iter <Address, unsigned>;

// For the LWP list in the process class
template class  dictionary_hash <unsigned, dyn_lwp *>;
template class  pdvector<dictionary_hash <unsigned, dyn_lwp *>::entry>;
template class  dictionary_hash_iter <unsigned, dyn_lwp *>;

#include "dyninstAPI/src/rpcMgr.h"
template class  dictionary_hash<unsigned, rpcLWP *>;
template class  pdvector<dictionary_hash <unsigned, rpcLWP *>::entry>;
template class pdvector<rpcThr *>;

template class pdvector<inferiorRPCtoDo *>;
template class pdvector<inferiorRPCinProgress *>;

#include "common/src/List.C"
template class  List<miniTrampHandle*>;
template class  ListBase<miniTrampHandle*, void*>;
template class  dictionary_hash <const instPoint *, trampTemplate *>;
template class  pdvector<dictionary_hash <const instPoint *, trampTemplate *>::entry>;
template class  dictionary_hash_iter <const instPoint *, trampTemplate *>;

template class  dictionary_hash <Address, Symbol*>;
template class  pdvector<dictionary_hash <Address, Symbol*>::entry>;
template class  dictionary_hash <instPoint*, unsigned>;
template class  pdvector<dictionary_hash <instPoint*, unsigned>::entry>;
template class  pdvector<dictionary_hash <instPoint*, unsigned long>::entry>;
template class  dictionary_hash <pdstring, internalSym*>;
template class  pdvector<dictionary_hash <pdstring, internalSym*>::entry>;
template class  dictionary_hash <pdstring, pdmodule *>;
template class  pdvector<dictionary_hash <pdstring, pdmodule *>::entry>;
template class  dictionary_hash <pdstring, int_function*>;
template class  pdvector<dictionary_hash <pdstring, int_function*>::entry>;
template class  dictionary_hash <pdstring, supportedLanguages>;
template class  pdvector<dictionary_hash <pdstring, supportedLanguages>::entry>;
template class  dictionary_hash <pdstring, unsigned>;
template class  pdvector<dictionary_hash <pdstring, unsigned>::entry>;
template class  dictionary_hash <pdstring, pdvector<int_function*>*>;
template class  pdvector<dictionary_hash <pdstring, pdvector<int_function*>*>::entry>;
template class  dictionary_hash <unsigned, heapItem*>;
template class  dictionary_hash <unsigned long, heapItem*>;
template class  pdvector<dictionary_hash <unsigned, heapItem*>::entry>;
template class  pdvector<dictionary_hash <unsigned long, heapItem*>::entry>;
template class  dictionary_hash <unsigned, int_function*>;
template class  dictionary_hash <unsigned long, int_function*>;
template class  pdvector<dictionary_hash <unsigned, int_function*>::entry>;
template class  pdvector<dictionary_hash <unsigned long, pdstring>::entry>;

template class dictionary_hash<int, BPatch_process *>;
template class pdvector<dictionary_hash <int, BPatch_process *>::entry>;

template class dictionary_hash<const int_function *, BPatch_function *>;
template class pdvector<dictionary_hash <const int_function *, BPatch_function *>::entry>;
#ifndef BPATCH_LIBRARY
template class  dictionary_hash <int_function*, int_function*>;
template class  pdvector<dictionary_hash <int_function*, int_function*>::entry>;
#endif
template class  dictionary_hash <int_function*, BPatch_function*>;
template class  pdvector<dictionary_hash<int_function*, BPatch_function*>::entry>;
template class  dictionary_hash <Address, BPatch_variableExpr*>;
template class  pdvector<dictionary_hash <Address, BPatch_variableExpr*>::entry>;

template class BPatch_Vector<BPatch_frame>;


#ifdef alpha_dec_osf4_0
template class  dictionary_hash <pdstring, int>;
template class  pdvector<dictionary_hash <pdstring, int>::entry>;
#endif

template class  pdvector<dictionary_hash <unsigned long, int_function*>::entry>;
template class  dictionary_hash <unsigned, unsigned>;
template class  pdvector<dictionary_hash <unsigned, unsigned>::entry>;
template class  dictionary_hash <unsigned, unsigned long>;
template class  dictionary_hash <unsigned long, unsigned long>;
template class  pdvector<dictionary_hash <unsigned long, unsigned long>::entry>;
template class  dictionary_hash_iter <unsigned long, unsigned long>;

#if defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ \
 || defined(i386_unknown_solaris2_4) \
 || defined(sparc_sun_solaris2_4) \
 || defined(mips_sgi_irix_6_4) \
 || defined(ia64_unknown_linux2_4)
template class pdvector<pdElfShdr*>;
#endif
