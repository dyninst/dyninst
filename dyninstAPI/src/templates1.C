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

// $Id: templates1.C,v 1.36 2000/07/27 15:23:23 hollings Exp $

#pragma implementation "Dictionary.h"
#include "common/src/Dictionary.C"

#include "util/h/String.h"

#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/dyninstP.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/util.h"
#include "dyninstAPI/src/Object.h"
#include "dyninstAPI/src/sharedobject.h"

#include "dyninstAPI/src/FunctionExpansionRecord.h"

#if defined(BPATCH_LIBRARY)
#include "dyninstAPI/h/BPatch_Set.h"
#include "dyninstAPI/h/BPatch_type.h"

class BPatch_thread;
class BPatch_field;
class BPatch_variableExpr;
#endif

#if defined(mips_sgi_irix6_4)
#include "dyninstAPI/src/irixDL.h" // dsoEvent_t
template class  vector<pdDsoEvent *>;
template class  vector<pdElfObjInfo *>;
template class  vector<vector<int> >;
template class  vector<pdElfSym *>;
#endif
template class  dictionary_hash <Address, unsigned>;
template class  vector<dictionary_hash <Address, unsigned>::entry>;
template class  dictionary_hash_iter <Address, unsigned>;


template class  dictionary_hash <const instPoint *, trampTemplate *>;
template class  vector<dictionary_hash <const instPoint *, trampTemplate *>::entry>;
template class  dictionary_hash_iter <const instPoint *, trampTemplate *>;

template class  dictionary_hash <instInstance *, instInstance *>;
template class  vector<dictionary_hash <instInstance *, instInstance *>::entry>;
template class  dictionary_hash <Address, Symbol*>;
template class  vector<dictionary_hash <Address, Symbol*>::entry>;
template class  dictionary_hash<const instPoint*, point*>;
template class  vector<dictionary_hash<const instPoint*, point*>::entry>;
template class  dictionary_hash <instPoint*, unsigned>;
template class  vector<dictionary_hash <instPoint*, unsigned>::entry>;
template class  vector<dictionary_hash <instPoint*, unsigned long>::entry>;
template class  dictionary_hash <string, internalSym*>;
template class  vector<dictionary_hash <string, internalSym*>::entry>;
template class  dictionary_hash <string, pdmodule *>;
template class  vector<dictionary_hash <string, pdmodule *>::entry>;
template class  dictionary_hash <string, pd_Function*>;
template class  vector<dictionary_hash <string, pd_Function*>::entry>;
template class  dictionary_hash <string, resource*>;
template class  vector<dictionary_hash <string, resource*>::entry>;
template class  dictionary_hash <string, unsigned>;
template class  vector<dictionary_hash <string, unsigned>::entry>;
template class  dictionary_hash <string, vector<pd_Function*>*>;
template class  vector<dictionary_hash <string, vector<pd_Function*>*>::entry>;
template class  dictionary_hash <unsigned, heapItem*>;
template class  dictionary_hash <unsigned long, heapItem*>;
template class  vector<dictionary_hash <unsigned, heapItem*>::entry>;
template class  vector<dictionary_hash <unsigned long, heapItem*>::entry>;
template class  dictionary_hash <unsigned, metricDefinitionNode*>;
template class  vector<dictionary_hash <unsigned, metricDefinitionNode*>::entry>;
template class  vector<dictionary_hash <unsigned long, metricDefinitionNode*>::entry>;
template class  dictionary_hash <string, metricDefinitionNode*>;
template class  vector<dictionary_hash <string, metricDefinitionNode*>::entry>;
template class  dictionary_hash_iter <string, metricDefinitionNode*>;
template class  dictionary_hash <unsigned, pd_Function*>;
template class  dictionary_hash <unsigned long, pd_Function*>;
template class  vector<dictionary_hash <unsigned, pd_Function*>::entry>;

#ifndef BPATCH_LIBRARY
template class  dictionary_hash <function_base*, function_base*>;
template class  vector<dictionary_hash <function_base*, function_base*>::entry>;
#else
template class  dictionary_hash <function_base*, BPatch_function*>;
template class  vector<dictionary_hash<function_base*, BPatch_function*>::entry>;
template class  dictionary_hash <Address, BPatch_variableExpr*>;
template class  vector<dictionary_hash <Address, BPatch_variableExpr*>::entry>;
#endif

#ifdef alpha_dec_osf4_0
template class  dictionary_hash <string, int>;
template class  vector<dictionary_hash <string, int>::entry>;
#endif

template class  vector<dictionary_hash <unsigned long, pd_Function*>::entry>;
template class  dictionary_hash <unsigned, resource *>;
template class  vector<dictionary_hash <unsigned long, resource *>::entry>;
template class  dictionary_hash <unsigned, unsigned>;
template class  vector<dictionary_hash <unsigned, unsigned>::entry>;
template class  dictionary_hash <unsigned, unsigned long>;
template class  dictionary_hash <unsigned long, unsigned long>;
template class  vector<dictionary_hash <unsigned long, unsigned long>::entry>;
template class  dictionary_hash_iter <unsigned long, unsigned long>;
template class  dictionary_hash <metricDefinitionNode*,metricDefinitionNode*>;
template class  vector<dictionary_hash <metricDefinitionNode*,metricDefinitionNode*>::entry>;

template class  dictionary_hash_iter <Address, Symbol*>;
template class  dictionary_hash_iter <const instPoint*, point*>;
template class  dictionary_hash_iter <instPoint*, unsigned>;
template class  dictionary_hash_iter <string, Symbol>;
template class  dictionary_hash_iter <string, internalSym*>;
template class  dictionary_hash_iter <string, pdmodule *>;
template class  dictionary_hash_iter <string, pd_Function*>;
template class  dictionary_hash_iter <string, resource*>;
template class  dictionary_hash_iter <string, unsigned>;
template class  dictionary_hash_iter <string, vector<pd_Function*>*>;
template class  dictionary_hash_iter <unsigned, unsigned>;
template class  dictionary_hash_iter <unsigned, heapItem*>;
template class  dictionary_hash_iter <unsigned, metricDefinitionNode*>;
template class  dictionary_hash_iter <unsigned long, metricDefinitionNode*>;
template class  dictionary_hash_iter <unsigned, pd_Function*>;
template class  dictionary_hash_iter <unsigned long, pd_Function*>;
template class  vector<shared_object *> ;


/* ***************************************************************************** */

#ifdef SHM_SAMPLING
#include "fastInferiorHeap.C"
#include "fastInferiorHeapHKs.h"

template class vector<genericHK::trampRange>;
template class vector<states>;

template class fastInferiorHeap<intCounterHK, intCounter>;
template class fastInferiorHeap<wallTimerHK, tTimer>;
template class fastInferiorHeap<processTimerHK, tTimer>;
template class vector< fastInferiorHeap<intCounterHK, intCounter> >;
template class vector< fastInferiorHeap<wallTimerHK, tTimer> >;
template class vector< fastInferiorHeap<processTimerHK, tTimer> >;

#ifndef USES_NATIVE_CC
#include "baseTable.C"
#endif
template class baseTable<intCounterHK, intCounter>;
template class baseTable<wallTimerHK, tTimer>;
template class baseTable<processTimerHK, tTimer>;

#ifndef USES_NATIVE_CC
#include "superVector.C"
#endif
template class superVector<intCounterHK, intCounter>;
template class superVector<wallTimerHK, tTimer>;
template class superVector<processTimerHK, tTimer>;
template class vector<superVector<intCounterHK, intCounter> *>;
template class vector<superVector<wallTimerHK, tTimer> *>;
template class vector<superVector<processTimerHK, tTimer> *>;
#endif

#include "common/src/vectorSet.C"
template class vectorSet<process::inferiorRPCtoDo>;
template class vectorSet<process::inferiorRPCinProgress>;

#ifdef SHM_SAMPLING
#include "fastInferiorHeapMgr.h"
template class vector<fastInferiorHeapMgr::oneHeapStats>;
#endif

#ifdef BPATCH_LIBRARY
template class dictionary_hash <string, Symbol>;
template class vector<dictionary_hash <string, Symbol>::entry>;
template class vector<dictionary_hash<Address, BPatch_point *>::entry>;

template class dictionary_hash<string, BPatch_type *>;
template class dictionary_hash<int, BPatch_type *>;
template class dictionary_hash<string, BPatch_localVar *>;
template class dictionary_hash<int, BPatch_thread *>;
template class dictionary_hash<Address, BPatch_point *>;

template class dictionary_hash_iter<string, BPatch_type *>;
template class vector<dictionary_hash <string, BPatch_type *>::entry>;
template class dictionary_hash_iter<int, BPatch_type *>;
template class vector<dictionary_hash <int, BPatch_type *>::entry>;
template class dictionary_hash_iter<string, BPatch_localVar *>;
template class vector<dictionary_hash <string, BPatch_localVar *>::entry>;
template class dictionary_hash_iter<int, BPatch_thread *>;
template class vector<dictionary_hash <int, BPatch_thread *>::entry>;

template class vector<BPatch_localVar *>;
template class vector<BPatch_field *>;
#endif

template class dictionary_hash <string, vector<string>*>;
template class vector<dictionary_hash <string, vector<string>*>::entry>;

template class vector<process::inferiorRPCtoDo>;
template class vector<process::inferiorRPCinProgress>;

template class vector<dictionary_hash <unsigned int, Address>::entry>;


#if defined(BPATCH_LIBRARY)

class BPatch_basicBlock;

template class  dictionary_hash<Address,BPatch_basicBlock*>;
template class  vector<dictionary_hash<Address,BPatch_basicBlock*>::entry>;

#endif
