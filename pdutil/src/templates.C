
/*
 * Generate code for template classes used by libutil
 *
 * $Log: templates.C,v $
 * Revision 1.1  1994/09/22 03:21:14  markc
 * generate libutil template code
 *
 */

#pragma implementation "list.h"
#include "util/h/list.h"
#include "util/h/tunableConst.h"
#include "util/h/aggregateSample.h"

typedef List<tunableConstant*> l1;
typedef List<sampleInfo*> l2;
