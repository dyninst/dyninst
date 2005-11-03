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


// $Id: templates2.C,v 1.57 2005/11/03 05:21:08 jaw Exp $

#if defined(__XLC__) || defined(__xlC__)
#include "common/h/Dictionary.h"
#else
#pragma implementation "Dictionary.h"
#include "common/src/Dictionary.C"
#endif

#include "common/h/String.h"
#include "common/h/Symbol.h"

#if 0
#include "dyninstAPI/src/symtab.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/inst.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/dyninstP.h"
#include "dyninstAPI/src/ast.h"
#include "dyninstAPI/src/util.h"
#include "dyninstAPI/src/Object.h"
#include "dyninstAPI/src/mapped_object.h"
#include "dyninstAPI/src/syscalltrap.h"
#include "dyninstAPI/src/libState.h"

#include "dyninstAPI/src/signalhandler.h"
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


class Symbol;
template class  dictionary_hash_iter <Address, Symbol*>;

//template class  dictionary_hash_iter <instPoint*, unsigned>;
template class  dictionary_hash_iter <pdstring, Symbol>;
class pdmodule;
template class  dictionary_hash_iter <pdstring, pdmodule *>;
class int_function;
template class  dictionary_hash_iter <pdstring, int_function*>;
template class  dictionary_hash_iter <pdstring, pdvector<int_function*>*>;
template class  dictionary_hash_iter <unsigned, int_function*>;
template class pdvector<pdvector<int_function * >* >;

template class  dictionary_hash_iter <pdstring, unsigned>;

class int_variable;
template class  dictionary_hash_iter <pdstring, pdvector<int_variable*>*>;
class image_func;
template class  dictionary_hash_iter <pdstring, pdvector<image_func*>*>;
class image_variable;
template class  dictionary_hash_iter <pdstring, pdvector<image_variable*>*>;

template class  dictionary_hash_iter <unsigned, unsigned>;
class heapItem;
template class  dictionary_hash_iter <unsigned, heapItem*>;
template class  dictionary_hash_iter <unsigned long, int_function*>;

class mapped_object;
template class  pdvector<mapped_object *> ;

class libraryCallback;
template class dictionary_hash<pdstring, libraryCallback *>;
template class pdvector<dictionary_hash <pdstring, libraryCallback *>::entry>;

class syscallTrap;
template class pdvector<syscallTrap *>;

/* ***************************************************************************** */

template class dictionary_hash <pdstring, Symbol>;
template class pdvector<dictionary_hash <pdstring, Symbol>::entry>;

class BPatch_point;
template class pdvector<dictionary_hash<Address, BPatch_point *>::entry>;
template class dictionary_hash<Address, BPatch_point *>;

class BPatch_type;
template class dictionary_hash<pdstring, BPatch_type *>;
template class dictionary_hash<int, BPatch_type *>;
template class dictionary_hash_iter<pdstring, BPatch_type *>;
template class pdvector<dictionary_hash <pdstring, BPatch_type *>::entry>;
template class dictionary_hash_iter<int, BPatch_type *>;
template class pdvector<dictionary_hash <int, BPatch_type *>::entry>;
class BPatch_localVar;
template class dictionary_hash<pdstring, BPatch_localVar *>;
template class dictionary_hash_iter<pdstring, BPatch_localVar *>;
template class pdvector<dictionary_hash <pdstring, BPatch_localVar *>::entry>;
template class pdvector<BPatch_localVar *>;
class BPatch_thread;
template class dictionary_hash<int, BPatch_thread *>;
template class dictionary_hash_iter<int, BPatch_thread *>;
template class pdvector<dictionary_hash <int, BPatch_thread *>::entry>;

class BPatch_field;
template class pdvector<BPatch_field *>;

#include "EventHandler.h"
template class pdvector<EventRecord *>;
template class pdvector<EventRecord>;

template class pdvector<pdpair<pdstring, pdvector<pdstring> *> >;
template class dictionary_hash <pdstring, pdvector<pdstring> *>;
template class pdvector<dictionary_hash <pdstring, pdvector<pdstring> *>::entry>;

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
template class dictionary_hash< pdstring, BPatch_typeCollection * >;
template class pdvector<dictionary_hash <pdstring, BPatch_typeCollection *>::entry>;

/* For use with regexes.  gcc 2.95.3 on alpha needs these to be made explicit. */
template class pdvector< pdpair< pdstring, int_function *> >;
template class pdvector< pdpair< pdstring, pdvector< int_function * > * > >;



#include "signalhandler.h"
#include "BPatch_asyncEventHandler.h"
#include "mailbox.h"
#include "callbacks.h"
#include "debuggerinterface.h"
/* From class BPatch_asyncEventHandler */
template class pdvector<process_record>;
template class pdvector<BPatchSnippetHandle *>;
template class pdvector<BPatch_function *>;
template class pdvector<eventLock::lock_stack_elem>;
template class pdvector<EventGate *>;
template class dictionary_hash< eventType, pdvector< CallbackBase * > >;
template class pdvector<dictionary_hash < eventType, pdvector <CallbackBase *> >::entry>;

template class pdvector<CallbackBase *>;
template class pdvector<BPatchAsyncThreadEventCallback *>;
template class pdvector<AsyncThreadEventCallback *>;
template class pdvector<dyncall_cb_record>;
template class pdvector<thread_event_cb_record>;
template class pdvector<dyncall_cb_record *>;
template class pdvector<void (*)()>;
template class pdvector<void *>;
template class pdvector<eventType>;
template class pdvector< pdpair<unsigned long, const char *> >;
#if defined (os_linux)
template class pdvector<SignalHandler::stopping_proc_rec>;
template class pdvector<dyn_lwp *>;
#endif
#if defined( arch_ia64 )
class int_basicBlock;
#include <list>
template class std::list< int_basicBlock *>;
#endif /* defined( arch_ia64 ) */

template class pdvector< Symbol >;
template class dictionary_hash< pdstring, pdvector< Symbol > >;
template class pdvector<dictionary_hash < pdstring, pdvector <Symbol> >::entry>;

template class dictionary_hash< pdstring, bool >;
template class pdvector< dictionary_hash< pdstring, bool >::entry >;

/* #include "dyninstAPI/src/LineInformation.h" */
/* template LineInformation::SourceLineInternTable; */

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
