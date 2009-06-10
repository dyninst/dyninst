#if defined(__XLC__) || defined(__xlC__)
#pragma implementation("Dictionary.h")
#else
#pragma implementation "Dictionary.h"
#endif

#include <vector>

// Templates used in test1_30.C
template class std::vector<std::pair< unsigned long, unsigned long > >;
template class std::vector<std::pair< const char *, unsigned int > >;

#include "ParameterDict.h"
#include <map>
#include <string>
template class std::map<std::string, Parameter*>;
