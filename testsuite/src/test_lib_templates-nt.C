#pragma warning (disable: 4660)

#include "common/src/Dictionary.C"
#include "common/src/debugOstream.C"
#include "common/src/int64iostream.C"
#include "common/src/String.C"
#include "ParameterDict.h"

template class dictionary_hash<pdstring, Parameter*>;
template class pdvector<dictionary_hash<pdstring, Parameter*>::entry>;

