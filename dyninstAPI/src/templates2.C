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

// $Id: templates2.C,v 1.25 2003/07/15 22:44:47 schendel Exp $

#pragma implementation "Dictionary.h"
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
#include "dyninstAPI/src/syscalltrap.h"
#include "dyninstAPI/src/libState.h"

#include "dyninstAPI/src/FunctionExpansionRecord.h"

#if defined(BPATCH_LIBRARY)
#include "dyninstAPI/src/LineInformation.h"
#include "dyninstAPI/h/BPatch_Set.h"
#include "dyninstAPI/h/BPatch_type.h"

class BPatch_thread;
class BPatch_field;
class BPatch_variableExpr;


#ifndef OLD_LINE_INFO
template class  dictionary_hash <pdstring, FunctionInfo *>;
template class  pdvector< dictionary_hash <pdstring, FunctionInfo * >::entry >;
template class  dictionary_hash_iter <pdstring, FunctionInfo *>;

template class  dictionary_hash <pdstring, FileLineInformation *>;
template class  pdvector< dictionary_hash <pdstring, FileLineInformation * >::entry >;
template class  dictionary_hash_iter <pdstring, FileLineInformation *>;

template class pdvector<tuple *>;
template class std::map<unsigned short, pdvector<tuple *> *, std::less<unsigned short> >;
template class std::map<Address, pdvector<tuple *> *, std::less<Address> >;
#endif
#endif

#ifndef alpha_dec_osf4_0
// ld on Alpha complains about the vector<pdstring> class being
// multiply defined with the following line in.  Perhaps
// it automatically generates vector<pdstring> when it sees
// pair<pdstring, vector<pdstring> > in the line after.
//
// Ray Chen 6/18/2002
template class pdvector<pdstring>;
#endif
template class pdpair<pdstring, pdvector<pdstring> >;

template class pdvector<pdpair<pdstring, pdvector<pdstring> > >;
template class pdvector<pdvector<pd_Function * >* >;

template class  dictionary_hash_iter <Address, Symbol*>;
template class  dictionary_hash_iter <instPoint*, unsigned>;
template class  dictionary_hash_iter <pdstring, Symbol>;
template class  dictionary_hash_iter <pdstring, internalSym*>;
template class  dictionary_hash_iter <pdstring, pdmodule *>;
template class  dictionary_hash_iter <pdstring, pd_Function*>;
template class  dictionary_hash_iter <pdstring, unsigned>;
template class  dictionary_hash_iter <pdstring, pdvector<pd_Function*>*>;
template class  dictionary_hash_iter <unsigned, unsigned>;
template class  dictionary_hash_iter <unsigned, heapItem*>;
template class  dictionary_hash_iter <unsigned, pd_Function*>;
template class  dictionary_hash_iter <unsigned long, pd_Function*>;
template class  pdvector<shared_object *> ;


template class dictionary_hash<pdstring, libraryCallback *>;
template class pdvector<dictionary_hash <pdstring, libraryCallback *>::entry>;

template class pdvector<syscallTrap *>;

/* ***************************************************************************** */

#ifdef SHM_SAMPLING
#include "varInstanceHKs.h"
template class pdvector<intCounterHK*>;
template class pdvector<wallTimerHK*>;
template class pdvector<processTimerHK*>;
#ifdef PAPI
template class pdvector<hwTimerHK*>;
template class pdvector<hwCounterHK*>;
#endif

#ifndef USES_NATIVE_CC
#include "varInstance.C"
#endif
template class varInstance<intCounterHK>;
template class varInstance<wallTimerHK>;
template class varInstance<processTimerHK>;
#ifdef PAPI
template class varInstance<hwTimerHK>;
template class varInstance<hwCounterHK>;
#endif
template class pdvector<baseVarInstance *>;

#ifndef USES_NATIVE_CC
#include "varTable.C"
#endif
template class varTable<intCounterHK>;
template class varTable<wallTimerHK>;
template class varTable<processTimerHK>;
#ifdef PAPI
template class varTable<hwTimerHK>;
template class varTable<hwCounterHK>;
#endif
template class pdvector<baseVarTable *>;
#endif

#ifdef BPATCH_LIBRARY
template class dictionary_hash <pdstring, Symbol>;
template class pdvector<dictionary_hash <pdstring, Symbol>::entry>;
template class pdvector<dictionary_hash<Address, BPatch_point *>::entry>;

template class dictionary_hash<pdstring, BPatch_type *>;
template class dictionary_hash<int, BPatch_type *>;
template class dictionary_hash<pdstring, BPatch_localVar *>;
template class dictionary_hash<int, BPatch_thread *>;
template class dictionary_hash<Address, BPatch_point *>;

template class dictionary_hash_iter<pdstring, BPatch_type *>;
template class pdvector<dictionary_hash <pdstring, BPatch_type *>::entry>;
template class dictionary_hash_iter<int, BPatch_type *>;
template class pdvector<dictionary_hash <int, BPatch_type *>::entry>;
template class dictionary_hash_iter<pdstring, BPatch_localVar *>;
template class pdvector<dictionary_hash <pdstring, BPatch_localVar *>::entry>;
template class dictionary_hash_iter<int, BPatch_thread *>;
template class pdvector<dictionary_hash <int, BPatch_thread *>::entry>;

template class pdvector<BPatch_localVar *>;
template class pdvector<BPatch_field *>;
#endif

template class pdvector<pdpair<pdstring, pdvector<pdstring> *> >;
template class dictionary_hash <pdstring, pdvector<pdstring> *>;
template class pdvector<dictionary_hash <pdstring, pdvector<pdstring> *>::entry>;

template class pdvector<inferiorRPCtoDo>;
template class pdvector<inferiorRPCinProgress>;

template class pdvector<dictionary_hash <unsigned int, Address>::entry>;

#include "dyninstAPI/src/installed_miniTramps_list.h"
template class dictionary_hash<const instPoint *, installed_miniTramps_list*>;
template class pdvector<dictionary_hash <const instPoint *, installed_miniTramps_list*>::entry>;

#if defined(BPATCH_LIBRARY)

class BPatch_basicBlock;

template class  dictionary_hash<Address,BPatch_basicBlock*>;
template class  pdvector<dictionary_hash<Address,BPatch_basicBlock*>::entry>;

#endif

#ifndef BPATCH_LIBRARY
class defInst;
template class pdvector<defInst *>;
#endif

#if defined( USES_DWARF_DEBUG )
#include <stack>
template class std::stack< long int >;
#include <map>
template class std::map< unsigned int, char * >;
#endif
