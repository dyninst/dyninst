// templates.C
// for table visi

/*
 * $Log: templates.C,v $
 * Revision 1.4  1995/12/22 22:37:43  tamches
 * 2 new instantiations
 *
 * Revision 1.3  1995/11/29 00:43:56  tamches
 * added lots of templates needed for new pdLogo stuff
 *
 * Revision 1.2  1995/11/20 20:20:44  tamches
 * a new template to support changes to tableVisi.C
 *
 * Revision 1.1  1995/11/04 00:47:44  tamches
 * First version of new table visi
 *
 */

#include "Vector.h"

template class vector<unsigned>;

#include "tvFocus.h"
template class vector<tvFocus>;

#include "tvMetric.h"
template class vector<tvMetric>;

#include "tvCell.h"
template class vector<tvCell>;
template class vector< vector<tvCell> >;

#include "../../../paradyn/src/UIthread/minmax.C"
template unsigned max(unsigned, unsigned);
template float max(float, float);
template bool ipmax(unsigned, unsigned);
template int ipmin(int, int);
template int min(int, int);
template int max(int, int);

#include "util/src/DictionaryLite.C"
#include "paradyn/src/UIthread/pdLogo.h"
template class vector<pdLogo *>;
template class dictionary_lite<string, pdLogo *>;
template class pair<string, pdLogo *>;
template class vector<dictionary_lite<string, pdLogo *>::hash_pair>;
template class vector< vector<dictionary_lite<string,pdLogo*>::hash_pair> >;

template class dictionary_lite<string, pdLogo::logoStruct>;
template class vector<pdLogo::logoStruct>;
template class vector<dictionary_lite<string, pdLogo::logoStruct>::hash_pair>;
template class vector< vector<dictionary_lite<string, pdLogo::logoStruct>::hash_pair> >;
template class pair<string, pdLogo::logoStruct>;

template class vector<string>;
