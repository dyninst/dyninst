#if defined(__XLC__) || defined(__xlC__)
#pragma implementation("Dictionary.h")
#else
#pragma implementation "Dictionary.h"
#endif

#include "Process_data.h"
#include <vector>

template class std::vector<Process_data>;

// Templates used in test1_30.C
template class std::vector<std::pair< unsigned long, unsigned long > >;
template class std::vector<std::pair< const char *, unsigned int > >;

#include "ParameterDict.h"
#include "common/src/Dictionary.C"
#include "common/h/String.h"
template class dictionary_hash<pdstring, Parameter*>;
template class pdvector<dictionary_hash<pdstring, Parameter*>::entry>;
