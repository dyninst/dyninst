#pragma implementation "Vector.h"
#include "util/h/Vector.h"
#include "visi/h/datagrid.h"

class PhaseInfo;

template class vector<PhaseInfo *>;

// logo stuff:
#include "paradyn/src/UIthread/minmax.C"
template float max(float, float);

#include "util/h/String.h"
#include "util/src/DictionaryLite.C"
#include "pdLogo.h"
template class dictionary_lite<string, pdLogo *>;

template class dictionary_lite<string, pdLogo::logoStruct>;
