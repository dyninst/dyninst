//
// templates for DMthread, VISIthread, VMthread, and PCthread excluding
// igen templates
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


template class vector<cpContext*>;
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
template class stack<T_dyninstRPC::buf_struct *>;
template class stack<T_visi::buf_struct *>;

/* ********************************
 * PCthread stuff
 */
#include "paradyn/src/PCthread/PCshg.h"
#include "paradyn/src/PCthread/PCevalTest.h"
#include "paradyn/src/PCthread/PCglobals.h"
#include "paradyn/src/PCthread/PCauto.h"
#include "paradyn/src/PCthread/PCwhen.h"
#include "paradyn/src/PCthread/PCwhere.h"
#include "paradyn/src/PCthread/PCwhy.h"

template class List<focus *>;
template class ListItem<focus *>;
template class List<focusList *>;
template class ListItem<focusList *>;
template class List<metricInstance *>;
template class ListItem<metricInstance *>;

template class HTable<PCmetric *>;
template class List<PCmetric *>;
template class ListItem<PCmetric *>;
template class HTable<datum *>;
template class HTable<focus *>;

template class List<datum *>;
template class ListItem<datum *>;
template class List<hint *>;
template class ListItem<hint *>;
template class List<hypothesis *>;
template class ListItem<hypothesis *>;
template class List<searchHistoryNode *>;
template class ListItem<searchHistoryNode *>;
template class List<test *>;
template class ListItem<test *>;
template class List<testResult *>;
template class ListItem<testResult *>;
template class List<timeInterval *>;
template class ListItem<timeInterval *>;


/* ************************************
 * VMthread stuff
 */
#include "paradyn/src/VMthread/VMtypes.h"
#include "VM.thread.h"

template class vector<VMactiveStruct *>;
template class vector<VMvisisStruct *>;
template class vector<VM_visiInfo>;

/* ******************************
 * VISIthread stuff
  */
template class vector<metricInstInfo *>;

