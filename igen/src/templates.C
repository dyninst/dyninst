
#include "util/h/headers.h"

#pragma implementation "Vector.h"
#include "util/h/Vector.h"

#pragma implementation "Dictionary.h"
#include "util/h/Dictionary.h"

#include "parse.h"

template class vector<arg*>;
template class vector<message_layer*>;
template class vector<string>;
template class vector<Options::el_data>;
template class vector<Options::stl_data>;
template class vector<type_defn *>;

template class dictionary_hash<string, remote_func*>;
template class dictionary_hash<string, type_defn*>;
template class dictionary_hash_iter<string, type_defn*>;
template class dictionary_hash_iter<string, remote_func*>;

