/*
 * Generate all the templates in one file.
 *
 */

/* 
 * $Log: templates.C,v $
 * Revision 1.3  1994/07/20 23:23:44  hollings
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
#include "symtab.h"
#include "process.h"
#include "inst.h"
#include "instP.h"
#include "dyninstP.h"
#include "metric.h"
#include "ast.h"
#include "util.h"
#include "internalMetrics.h"

typedef List<int>;
typedef List<void *>;
typedef List<processRec *>;
typedef List<dataReqNode *>;
typedef List<instReqNode *>;
typedef List<metricDefinitionNode *>;
typedef List<internalMetric *>;

typedef HTable<metricDefinitionNode *>;
typedef HTable<_metricRec *>;
typedef HTable<_resourceRec *>;
typedef HTable<_resourceListRec *>;

typedef HTable<pointRec *>;

typedef StringList<int>;
