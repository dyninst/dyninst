// templates1.C

#pragma implementation "Dictionary.h"
#include "util/h/Dictionary.h"

#include "util/h/String.h"

#include "rtinst/h/rtinst.h"
#include "rtinst/h/trace.h"
#include "util/h/aggregateSample.h"
#include "symtab.h"
#include "process.h"
#include "inst.h"
#include "instP.h"
#include "dyninstP.h"
#include "metric.h"
#include "ast.h"
#include "util.h"
#include "internalMetrics.h"
#include "util/h/Object.h"

template class  dictionary <instInstance *, instInstance *>;
template class  dictionary_hash <instInstance *, instInstance *>;
template class  dictionary_hash <Address, Symbol*>;
template class  dictionary_hash <instPoint*, point*>;
template class  dictionary_hash <instPoint*, unsigned>;
template class  dictionary_hash <string, Symbol>;
template class  dictionary_hash <string, internalSym*>;
template class  dictionary_hash <string, module *>;
template class  dictionary_hash <string, pdFunction*>;
template class  dictionary_hash <string, resource*>;
template class  dictionary_hash <string, unsigned>;
template class  dictionary_hash <string, vector<pdFunction*>*>;
template class  dictionary_hash <unsigned, cpSample*>;
template class  dictionary_hash <unsigned, heapItem*>;
template class  dictionary_hash <unsigned, metricDefinitionNode*>;
template class  dictionary_hash <unsigned, pdFunction*>;
template class  dictionary_hash <unsigned, resource *>;
template class  dictionary_hash <unsigned, unsigned>;
template class  dictionary_hash <unsigned, vector<mdl_type_desc> >;

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
template class  dictionary_iter<unsigned int, Symbol *>;


template class  dictionary_hash_iter <Address, Symbol*>;
template class  dictionary_hash_iter <instPoint*, point*>;
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

#ifdef paradyndCM5_blizzard
//=============================added by zxu for sampleNodes
#include "util/h/sys.h"
#include "rtinst/h/trace.h"
#include "../../paradyndCM5_blizzard/src/sample_nodes.h"

//template class vector<unsigned> ;
template class vector<time64> ;
//template class vector<sampleValue> ;
template class vector<stamped_sample> ;
template class vector<per_node_buffer> ;
template class dictionary_hash <unsigned, sampleVec *>  ;
template class dictionary_hash <unsigned, traceHeaderVec *>  ;
template class dictionary_hash_iter<unsigned, sampleVec *> ;
template class dictionary_hash_iter<unsigned, per_mid_buffer *> ;
template class dictionary_hash<unsigned, per_mid_buffer *>;
#endif


