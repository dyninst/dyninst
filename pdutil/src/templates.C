
/*
 * Generate code for template classes used by libutil
 *
 * templates.C,v
 * Revision 1.2  1995/06/02  21:00:09  newhall
 * added a NaN value generator
 * fixed memory leaks in Histogram class
 * added newValue member with a vector<sampleInfo *> to class sampleInfo
 *
 * Revision 1.1  1994/09/22  03:21:14  markc
 * generate libutil template code
 *
 */

#pragma implementation "list.h"
#include "util/h/Vector.h"
#include "util/h/String.h"
#include "util/h/list.h"
#include "util/h/tunableConst.h"
#include "util/h/aggregateSample.h"
#include "util/h/vector.h"

typedef List<tunableConstant*> l1;
typedef List<sampleInfo*> l2;
typedef vector<sampleInfo*> l3;
