// templates.C
// for table visi

/*
 * $Log: templates.C,v $
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
