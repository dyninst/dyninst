// templates.C

#include <X11/Xlib.h> // XColor
#include <array2d.h>

template class dynamic1dArray<XColor *>;
template class dynamic1dArray<double>;
template class dynamic1dArray<int>;
template class dynamic1dArray<bool>;

template class dynamic2dArray<double>;
template class dynamic2dArray<int>;
template class dynamic2dArray<bool>;

#include "minmax.C"
template float max(float, float);

#include "Vector.h"
template class vector<unsigned>;

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
