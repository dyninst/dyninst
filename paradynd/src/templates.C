/*
 * Generate all the templates in one file.
 *
 */

/* 
 * $Log: templates.C,v $
 * Revision 1.8  1994/09/22 02:27:18  markc
 * Gave type names to typedefs
 *
 * Revision 1.7  1994/09/20  18:18:33  hollings
 * added code to use actual clock speed for cost model numbers.
 *
 * Revision 1.6  1994/08/08  20:13:48  hollings
 * Added suppress instrumentation command.
 *
 * Revision 1.5  1994/08/02  18:25:09  hollings
 * fixed modules to use list template for lists of functions.
 *
 * Revision 1.4  1994/07/26  20:02:09  hollings
 * fixed heap allocation to use hash tables.
 *
 * Revision 1.3  1994/07/20  23:23:44  hollings
 * added insn generated metric.
 *
 * Revision 1.2  1994/06/27  18:57:17  hollings
 * removed printfs.  Now use logLine so it works in the remote case.
 * added internalMetric class.
 * added extra paramter to metric info for aggregation.
 *
 * Revision 1.1  1994/05/30  19:51:17  hollings
 * added code to support external templates.
 *
 *
 */
#pragma implementation  "list.h"

#include "util/h/list.h"

#include "rtinst/h/rtinst.h"
#include "rtinst/h/trace.h"
#include "util/h/aggregateSample.h"
#include "util/h/tunableConst.h"
#include "symtab.h"
#include "process.h"
#include "inst.h"
#include "instP.h"
#include "dyninstP.h"
#include "metric.h"
#include "ast.h"
#include "util.h"
#include "internalMetrics.h"

typedef List<int> d1;
typedef List<void *> d2;
typedef List<process *> d3;
typedef List<dataReqNode *> d4;
typedef List<instReqNode *> d5;
typedef List<metricDefinitionNode *> d6;
typedef List<internalMetric *> d7;
typedef List<libraryFunc*> d7a;

typedef HTable<metricDefinitionNode *> d8;
typedef HTable<metric*> d9;
typedef HTable<resource*> d10;
typedef HTable<resourceListRec*> d11;
typedef HTable<point*> d12;

typedef List<heapItem *> d13;
typedef HTable<heapItem *> d14;

typedef StringList<int> d15;

typedef List<pdFunction *> d16;
