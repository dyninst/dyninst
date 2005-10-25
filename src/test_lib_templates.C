
#include "ParameterDict.h"
#include <map>
#include <string>

template class std::map<std::string, Parameter*>;

#include "Process_data.h"
#include <vector>

template class std::vector<Process_data>;

// Templates used in test1_30.C
template class std::vector<std::pair< unsigned long, unsigned long > >;
template class std::vector<std::pair< const char *, unsigned int > >;
