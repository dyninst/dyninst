
/*
 * Part 2 of template code generation
 * This is split up to because compiling this file thrashes on a 32M sparc
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
#include "util/h/Object.h"

typedef List<metricDefinitionNode*> o2;

