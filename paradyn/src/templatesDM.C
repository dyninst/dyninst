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

//
// $Id: templatesDM.C,v 1.21 2001/08/23 14:43:34 schendel Exp $
// templates for DMthread, excluding igen templates
//

#pragma implementation "list.h"
#include "common/h/list.h"
#include "common/h/String.h"

#pragma implementation "Vector.h"
#include "common/h/Vector.h"

#pragma implementation "BufferPool.h"
#include "paradyn/src/DMthread/BufferPool.h"

#pragma implementation "Dictionary.h"
#include "common/src/Dictionary.C"

#pragma implementation "dyninstRPC.xdr.h"
#include "dyninstRPC.xdr.h"
#pragma implementation "visi.xdr.h"
#include "visi.xdr.h"

/*
 * Handy ready-to-use templates for the templates


  ****** dictionary_hash (no iter) *********
template class pair<>;
template class dictionary<>;
template class vector< pair<> >;
template class vector< dictionary_hash<>::hash_pair>;
template class vector< vector< dictionary_hash<>::hash_pair> >;
template class dictionary_hash<>;

  ****** dictionary_hash (iter) ************
template class dictionary<>;
template class pair<>;
template class vector< pair<> >; 
template class dictionary_hash<>;
template class dictionary_hash_iter<>;
template class vector< dictionary_hash<> :: hash_pair >;
template class vector< vector< dictionary_hash<> :: hash_pair > >;

  ******* priority queue ********************
template class PriorityQueue<>; 
template ostream &operator<<(ostream &, PriorityQueue<> &);
template class vector<PriorityQueue<>::pair>;

*/

/* *********************************   
 * DMthread stuff
 */

#include "dataManager.thread.h"
#include "dyninstRPC.xdr.CLNT.h"
class cpContext;
#include "paradyn/src/DMthread/DMdaemon.h"
#include "paradyn/src/DMthread/DMmetric.h"
#include "paradyn/src/DMthread/DMresource.h"
#include "paradyn/src/DMthread/DMperfstream.h"
#include "paradyn/src/DMthread/DMinclude.h"
#include "paradyn/src/DMthread/DMabstractions.h"
#include "paradyn/src/DMthread/DVbufferpool.h"


template class vector<unsigned>;
template class vector< vector<unsigned> >;
template class vector<int>;
template class vector< vector<string> >;
template class vector<phaseInfo *>;
template class vector<daemonEntry*>;
template class vector<paradynDaemon*>;
template class vector<executable*>;
template class vector<component*>;
template class vector<aggComponent*>;
template class vector<bool>;
template class vector< vector<bool> >;
template class vector<metric_focus_pair>;
template class vector<metricInstInfo>;
template class vector<met_name_id>;
template class vector<metric*>;
template class vector<resource*>;
template class vector<resourceList*>;
template class vector<abstraction*>;
template class vector<metricInstance*>;
template class vector<ArchiveType *>;
template class vector<rlNameIdType>;

template class dictionary_hash<string,metric*>;
template class vector<dictionary_hash<string,metric*>::entry>;

//template class pair<string, metric*>;

template class vector<dataValueType>;
template class BufferPool<dataValueType>;
template class vector<predCostType*>;
template class vector<DM_enableType*>;
template class vector<metricRLType>;

// trace data streams
template class vector<traceDataValueType>;
template class BufferPool<traceDataValueType>;


template class dictionary_hash<string, resource*>;
template class vector<dictionary_hash<string, resource*>::entry>;

template class dictionary_hash<unsigned, resource*>;
template class vector<dictionary_hash<unsigned, resource*>::entry>;
template class dictionary_hash_iter<unsigned, resource*>;


template class dictionary_hash<unsigned int, cpContext*>;
template class vector<dictionary_hash<unsigned int, cpContext*>::entry>;

template class vector<cpContext *>;

template class dictionary_hash<string, resourceList*>;
template class vector<dictionary_hash<string, resourceList*>::entry>;

template class dictionary_hash<string, abstraction*>;
template class vector<dictionary_hash<string, abstraction*>::entry>;

template class vector<performanceStream *>;
template class dictionary_hash<perfStreamHandle,performanceStream*>;
template class vector<dictionary_hash<perfStreamHandle,performanceStream*>::entry>;
template class dictionary_hash_iter<perfStreamHandle,performanceStream*>;

template class dictionary_hash<metricInstanceHandle,metricInstance*>;
template class vector<dictionary_hash<metricInstanceHandle,metricInstance*>::entry>;
template class dictionary_hash_iter<metricInstanceHandle,metricInstance*>;

template class vector<perfStreamEntry>;

//Blizzard
template class dictionary_hash<string,unsigned>;
template class vector<dictionary_hash<string,unsigned>::entry>;
template class dictionary_hash_iter<string,unsigned>;
//

template class List<aggComponent*>;
template class ListItem<aggComponent*>;

template class vector<T_dyninstRPC::batch_buffer_entry>;
template bool_t T_dyninstRPC_P_xdr_stl(XDR *, vector<T_dyninstRPC::batch_buffer_entry> *, int (*)(XDR *, T_dyninstRPC::batch_buffer_entry *), T_dyninstRPC::batch_buffer_entry *) ;

// trace data streams
template class vector<T_dyninstRPC::trace_batch_buffer_entry>;
template bool_t T_dyninstRPC_P_xdr_stl(XDR *, vector<T_dyninstRPC::trace_batch_buffer_entry> *, int (*)(XDR *, T_dyninstRPC::trace_batch_buffer_entry *), T_dyninstRPC::trace_batch_buffer_entry *) ;


template class vector<T_dyninstRPC::resourceInfoCallbackStruct>;

class CallGraph;
template class dictionary_hash<int, CallGraph *>;
template class vector<CallGraph *>;
template class vector<dictionary_hash<int, CallGraph*>::entry>;

template class dictionary_hash <resource *, vector<resource *> >;
template class vector< vector<resource *> >;
template class vector<dictionary_hash<resource *, vector<resource*> >::entry>;

template class dictionary_hash<resource *, int>;
template class vector<dictionary_hash<resource *, int>::entry>;

#if defined(rs6000_ibm_aix4_1)
#include "common/h/Symbol.h"
template class vector<Symbol>;
template class dictionary_hash<string, Symbol>;
template class vector<dictionary_hash<string, Symbol>::entry>;
#endif


template class vector<paradynDaemon::MPICHWrapperInfo>;

