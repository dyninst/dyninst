// templates.C

#include <X11/Xlib.h> // XColor

#include "minmax.C"
template float max(float, float);

#include "Vector.h"
template class vector<unsigned>;
template class vector< vector<double> >;
template class vector< vector<int> >;
template class vector<XColor *>;
template class vector<bool>;
template class vector<double>;
template class vector<int>;

#include "pdLogo.h"
#include "String.h"
#include "util/src/DictionaryLite.C"
template class dictionary_lite<string, pdLogo *>;
template class vector<string>;
template class vector<pdLogo *>;
template class pair<string, pdLogo *>;
template class vector<dictionary_lite<string, pdLogo *>::hash_pair>;
template class vector< vector<dictionary_lite<string, pdLogo *>::hash_pair> >;

template class dictionary_lite<string, pdLogo::logoStruct>;
template class vector<pdLogo::logoStruct>;
template class vector<dictionary_lite<string, pdLogo::logoStruct>::hash_pair>;
template class vector< vector<dictionary_lite<string, pdLogo::logoStruct>::hash_pair> >;
template class pair<string, pdLogo::logoStruct>;
