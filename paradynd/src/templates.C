/*
 * Generate all the templates in one file.
 *
 */

/* 
 * $Log: templates.C,v $
 * Revision 1.1  1994/05/30 19:51:17  hollings
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

typedef List<int>;
typedef List<void *>;
typedef List<processRec *>;
typedef List<dataReqNode *>;
typedef List<instReqNode *>;
typedef List<metricDefinitionNode *>;

typedef HTable<metricDefinitionNode *>;
typedef HTable<_metricRec *>;
typedef HTable<_resourceRec *>;
typedef HTable<_resourceListRec *>;

typedef StringList<int>;
