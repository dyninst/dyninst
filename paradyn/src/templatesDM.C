//
// templates for DMthread, excluding igen templates
//

#pragma implementation "list.h"
#include "util/h/list.h"
#include "util/h/String.h"
#pragma implementation "Vector.h"
#include "util/h/Vector.h"
#pragma implementation "Queue.h"
#include "util/h/Queue.h"
#pragma implementation "BufferPool.h"
#include "paradyn/src/DMthread/BufferPool.h"
#pragma implementation "Dictionary.h"
#include "util/h/Dictionary.h"
#pragma implementation "DictionaryLite.h"
#include "util/src/DictionaryLite.C"
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
template class vector<sampleInfo*>;
template class vector<bool>;
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
template class dictionary<string,metric*>;
template class dictionary_hash<string,metric*>;
template class pair<string, metric*>;
template class vector< pair< string, metric*> >;
template class vector< dictionary_hash<string, metric *> :: hash_pair >;
template class vector< vector< dictionary_hash<string, metric *> :: hash_pair > >;
template class vector<dataValueType>;
template class BufferPool<dataValueType>;


template class dictionary<string, resource*>;
template class dictionary_hash<string, resource*>;
template class pair<string, resource*>;
template class vector< pair<string, resource*> >;
template class vector< dictionary_hash<string, resource*> :: hash_pair >;
template class vector< vector< dictionary_hash<string, resource*> :: hash_pair > >;

template class dictionary<unsigned int, cpContext*>;
template class dictionary_hash<unsigned int, cpContext*>;
template class pair<unsigned int, cpContext*>;
template class vector< pair<unsigned int, cpContext*> >;
template class vector< dictionary_hash<unsigned int, cpContext*> :: hash_pair >;
template class vector< vector< dictionary_hash<unsigned int, cpContext*> :: hash_pair > >;
template class vector<cpContext *>;

template class dictionary<string, resourceList*>;
template class dictionary_hash<string, resourceList*>;
template class pair<string, resourceList*>;
template class vector< pair<string, resourceList*> >;
template class vector< dictionary_hash<string, resourceList*> :: hash_pair >;
template class vector< vector< dictionary_hash<string, resourceList*> :: hash_pair > >;

template class dictionary<string, abstraction*>;
template class dictionary_hash<string, abstraction*>;
template class vector< dictionary_hash<string, abstraction*> :: hash_pair >;
template class vector< vector< dictionary_hash<string, abstraction*> :: hash_pair > >;
template class pair<string, abstraction*>;
template class vector< pair<string, abstraction*> >;
template class vector< vector< pair<string, abstraction*> > >;

template class vector<performanceStream *>;
template class pair< perfStreamHandle, performanceStream* >;
template class vector< pair< perfStreamHandle, performanceStream* > >;
template class dictionary<perfStreamHandle,performanceStream*>;
template class dictionary_hash<perfStreamHandle,performanceStream*>;
template class dictionary_hash_iter<perfStreamHandle,performanceStream*>;
template class vector< vector< dictionary_hash<perfStreamHandle, performanceStream *> :: hash_pair > >;
template class vector< dictionary_hash<perfStreamHandle, performanceStream *> :: hash_pair >;
template class dictionary_iter<unsigned, performanceStream *>;

template class dictionary<metricInstanceHandle,metricInstance*>;
template class pair<metricInstanceHandle, metricInstance*>;
template class vector< pair<metricInstanceHandle, metricInstance*> >; 
template class dictionary_hash<metricInstanceHandle,metricInstance*>;
template class dictionary_hash_iter<metricInstanceHandle,metricInstance*>;
template class vector< dictionary_hash<unsigned, metricInstance *> :: hash_pair >;
template class vector< vector< dictionary_hash<unsigned, metricInstance *> :: hash_pair > >;
template class dictionary_iter<unsigned, metricInstance *>;

template class List<sampleInfo*>;
template class ListItem<sampleInfo*>;

template class vector<T_dyninstRPC::batch_buffer_entry>;
template bool_t T_dyninstRPC_P_xdr_stl(XDR *, vector<T_dyninstRPC::batch_buffer_entry> *, int (*)(XDR *, T_dyninstRPC::batch_buffer_entry *), T_dyninstRPC::batch_buffer_entry *) ;

