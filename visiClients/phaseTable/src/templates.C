/*
 * $Log: templates.C,v $
 * Revision 1.2  1996/01/17 18:32:00  newhall
 * changes due to new visiLib
 *
 * Revision 1.1  1995/12/15 22:01:55  tamches
 * first version of phaseTable
 *
 */

#pragma implementation "Vector.h"
#include "util/h/Vector.h"
#include "visi/h/visiTypes.h"

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
