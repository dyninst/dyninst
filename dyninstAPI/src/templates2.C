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


// $Id: templates2.C,v 1.68 2006/12/13 21:34:36 jaw Exp $

#if defined(__XLC__) || defined(__xlC__)
#include "common/h/Dictionary.h"
#else
#pragma implementation "Dictionary.h"
#include "common/src/Dictionary.C"
#endif

#include "common/h/String.h"
#include "common/h/Symbol.h"

class pdmodule;
template class  dictionary_hash_iter <pdstring, pdmodule *>;

class image_func;
template class  dictionary_hash_iter <pdstring, pdvector<image_func*>*>;

class mapped_object;
template class  pdvector<mapped_object *> ;

class syscallTrap;
template class pdvector<syscallTrap *>;

/* ***************************************************************************** */

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

#include "EventHandler.h"
template class pdvector<EventRecord>;
template class dictionary_hash<Address, threadmap_t *>;
template class pdvector<dictionary_hash<Address, threadmap_t *>::entry>;

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
class EventGate;
template class pdvector<EventGate *>;
template class dictionary_hash< eventType, pdvector< CallbackBase * > >;
template class pdvector<dictionary_hash < eventType, pdvector <CallbackBase *> >::entry>;

template class pdvector<CallbackBase *>;
template class pdvector<AsyncThreadEventCallback *>;
template class pdvector<void *>;
template class pdvector<eventType>;
template class pdvector<dyn_lwp *>;
template class pdvector<SignalHandler *>;
#if defined( arch_ia64 )
class int_basicBlock;
#include <list>
template class std::list< int_basicBlock *>;
template class dictionary_hash< Address, void * >;
template class pdvector< dictionary_hash< Address, void * >::entry >;
#endif /* defined( arch_ia64 ) */

template class dictionary_hash< pdstring, pdvector< Symbol > >;
template class pdvector<dictionary_hash < pdstring, pdvector <Symbol> >::entry>;

template class dictionary_hash< pdstring, bool >;
template class pdvector< dictionary_hash< pdstring, bool >::entry >;

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
template class dictionary_hash<pdstring, Statistic *>;
