// templatesPCV.C
// templates for perf consultant and visis

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

/* ********************************
 * PCthread stuff
 */

#include "paradyn/src/PCthread/PCintern.h"

#include "paradyn/src/PCthread/PCsearch.h"
#include "paradyn/src/PCthread/PCfilter.h"
#include "paradyn/src/PCthread/PCmetric.h"
#include "paradyn/src/PCthread/PCmetricInst.h"
#include "paradyn/src/PCthread/PCwhy.h"
#include "paradyn/src/PCthread/PCshg.h"
#include "paradyn/src/PCthread/PCdata.h"
#include "paradyn/src/PCthread/PCcostServer.h"
#include "UI.thread.h"

template class vector<ff>;
template class vector<mfpair>;
template class vector<costServerRec>;
template class vector<uiSHGrequest>;
template class vector<dataSubscriber*>;
template class vector<PCsearch*>;
template class vector<filter*>;
template class vector<PCMetInfo*>;
template class vector<PCmetric*>;
template class vector<PCMRec>;
template class vector<inPort>;
template class vector<searchHistoryNode*>;
template class vector<hypothesis*>;
template class vector<PCmetricInst*>;
template class pair<PCmetricInst*, filter*>;

template class pair<unsigned, PCsearch*>;
template class dictionary<unsigned, PCsearch*>;
template class vector< pair<unsigned, PCsearch*> >;
template class vector<dictionary_hash<unsigned, PCsearch*>::hash_pair>;
template class vector< vector< dictionary_hash< unsigned, PCsearch*>::hash_pair> >;
template class dictionary_hash<unsigned, PCsearch*>;

#include "util/src/PriorityQueue.C"
template class PriorityQueue<unsigned, searchHistoryNode*>; 
template ostream &operator<<(ostream &, PriorityQueue<unsigned, searchHistoryNode*> &);
template class vector<PriorityQueue<unsigned, searchHistoryNode*>::pair>;

template class dictionary_lite<unsigned, filter*>;
template class vector< dictionary_lite<unsigned, filter*> :: hash_pair >;
template class vector< vector< dictionary_lite<unsigned, filter*>::hash_pair> >;
 
template class pair<unsigned, searchHistoryNode*>;
template class dictionary<unsigned, searchHistoryNode*>;
template class vector< pair<unsigned, searchHistoryNode*> >;
template class vector< dictionary_hash<unsigned, searchHistoryNode*>::hash_pair>;
template class vector< vector< dictionary_hash<unsigned, searchHistoryNode*>::hash_pair> >;
template class dictionary_hash<unsigned, searchHistoryNode*>;

template class pair< metricHandle, dictionary_hash< unsigned, filter*>* >;
template class vector< dictionary_hash< unsigned, filter*>* >;
template class dictionary<metricHandle, dictionary_hash<unsigned, filter*>* >;
template class vector< pair<metricHandle, dictionary_hash<unsigned, filter*>* > >;
template class vector< dictionary_hash< metricHandle, dictionary_hash<unsigned, filter*>* >::hash_pair>;
template class vector< vector< dictionary_hash<metricHandle, dictionary_hash<unsigned, filter*>*>::hash_pair> >;
template class dictionary_hash<metricHandle, dictionary_hash<unsigned, filter*>* >; 

template class pair<string, PCmetric*>;
template class dictionary<string, PCmetric*>;
template class vector< pair<string, PCmetric*> >;
template class vector< dictionary_hash<string, PCmetric*>::hash_pair>;
template class vector< vector< dictionary_hash<string, PCmetric*>::hash_pair> >;
template class dictionary_hash<string, PCmetric*>;

template class pair<unsigned, PCmetricInst*>;
template class dictionary<unsigned, PCmetricInst*>;
template class vector< pair<unsigned, PCmetricInst*> >;
template class vector< dictionary_hash<unsigned, PCmetricInst*>::hash_pair>;
template class vector< vector< dictionary_hash<unsigned, PCmetricInst*>::hash_pair> >;
template class dictionary_hash<unsigned, PCmetricInst*>;

template class pair<string, hypothesis*>;
template class dictionary<string, hypothesis*>;
template class vector< pair< string, hypothesis*> >;
template class vector< dictionary_hash<string, hypothesis*>::hash_pair>;
template class vector< vector< dictionary_hash<string, hypothesis*>::hash_pair> >;
template class dictionary_hash<string, hypothesis*>;

template class pair<focus, vector<searchHistoryNode*>*>;
template class vector<vector<searchHistoryNode*>*>;
template class dictionary<focus, vector<searchHistoryNode*>*>;
template class vector< pair<focus, vector<searchHistoryNode*>*> >;
template class vector< dictionary_hash<focus, vector<searchHistoryNode*>*>::hash_pair>;
template class vector< vector< dictionary_hash<focus, vector<searchHistoryNode*>*>::hash_pair> >;
template class dictionary_hash<focus, vector<searchHistoryNode*>*>;

#include "util/src/CircularBuffer.C"
#include "paradyn/src/PCthread/PCintern.h"
template class circularBuffer<Interval, PCdataQSize>;

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

