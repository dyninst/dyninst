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

// templates1.C

#pragma implementation "Dictionary.h"
#include "util/h/Dictionary.h"

#include "util/h/String.h"

#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/dyninstP.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/util.h"
#include "util/h/Object.h"
#include "dyninstAPI/src/sharedobject.h"

template class  dictionary <const instPoint *, trampTemplate *>;
template class  dictionary <const instPoint *, point *>;
template class  dictionary_hash <const instPoint *, trampTemplate *>;
template class  dictionary <instInstance *, instInstance *>;
template class  dictionary_hash <instInstance *, instInstance *>;
template class  dictionary_hash <Address, Symbol*>;
template class  dictionary_hash<const instPoint*, point*>;
template class  dictionary_hash <instPoint*, unsigned>;
template class  dictionary_hash <string, Symbol>;
template class  dictionary_hash <string, internalSym*>;
template class  dictionary_hash <string, module *>;
template class  dictionary_hash <string, pdFunction*>;
template class  dictionary_hash <string, resource*>;
template class  dictionary_hash <string, unsigned>;
template class  dictionary_hash <string, vector<pdFunction*>*>;
template class  dictionary_hash <unsigned, heapItem*>;
template class  dictionary_hash <unsigned, metricDefinitionNode*>;
template class  dictionary_hash <string, metricDefinitionNode*>;
template class  dictionary_hash <unsigned, pdFunction*>;
template class  dictionary_hash <unsigned, resource *>;
template class  dictionary_hash <unsigned, unsigned>;
template class  dictionary <metricDefinitionNode*,metricDefinitionNode*>;
template class  dictionary_hash <metricDefinitionNode*,metricDefinitionNode*>;

template class  dictionary_iter<unsigned int, pdFunction *>;
template class  dictionary_iter<unsigned int, metricDefinitionNode *>;
template class  dictionary_iter<unsigned int, heapItem *>;
template class  dictionary_iter<string, vector<pdFunction *> *>;
template class  dictionary_iter<string, unsigned int>;
template class  dictionary_iter<string, resource *>;
template class  dictionary_iter<string, pdFunction *>;
template class  dictionary_iter<string, module *>;
template class  dictionary_iter<string, internalSym *>;
template class  dictionary_iter<instPoint *, unsigned int>;
template class  dictionary_iter<instPoint *, point *>;
template class  dictionary_iter<const instPoint *, point *>;
template class  dictionary_iter<unsigned int, Symbol *>;


template class  dictionary_hash_iter <Address, Symbol*>;
template class  dictionary_hash_iter <const instPoint*, point*>;
template class  dictionary_hash_iter <instPoint*, unsigned>;
template class  dictionary_hash_iter <string, Symbol>;
template class  dictionary_hash_iter <string, internalSym*>;
template class  dictionary_hash_iter <string, module *>;
template class  dictionary_hash_iter <string, pdFunction*>;
template class  dictionary_hash_iter <string, resource*>;
template class  dictionary_hash_iter <string, unsigned>;
template class  dictionary_hash_iter <string, vector<pdFunction*>*>;
template class  dictionary_hash_iter <unsigned, heapItem*>;
template class  dictionary_hash_iter <unsigned, metricDefinitionNode*>;
template class  dictionary_hash_iter <unsigned, pdFunction*>;
template class  vector<shared_object *> ;

/* ***************************************************************************** */

#ifdef SHM_SAMPLING
#include "fastInferiorHeap.C"
#include "fastInferiorHeapHKs.h"
template class vector<genericHK::trampRange>;

template class fastInferiorHeap<intCounterHK, intCounter>;
template class vector< fastInferiorHeap<intCounterHK, intCounter>::states >;

template class fastInferiorHeap<wallTimerHK, tTimer>;
template class vector< fastInferiorHeap<wallTimerHK, tTimer>::states >;

template class fastInferiorHeap<processTimerHK, tTimer>;
template class vector< fastInferiorHeap<processTimerHK, tTimer>::states >;
#endif

#include "util/src/vectorSet.C"
template class vectorSet<process::inferiorRPCtoDo>;
template class vectorSet<process::inferiorRPCinProgress>;

#include "fastInferiorHeapMgr.h"
template class vector<fastInferiorHeapMgr::oneHeapStats>;
