/*
 * Generate all the templates in one file.
 *
 */

/* 
 * $Log: PCtemplates.C,v $
 * Revision 1.1  1994/05/19 00:00:32  hollings
 * Added tempaltes.
 * Fixed limited number of nodes being evaluated on once.
 * Fixed color coding of nodes.
 *
 *
 */
#pragma implementation  "list.h"

#include "util/h/list.h"

#include "util/h/tunableConst.h"
#include "dataManager.h"
#include "PCshg.h"
#include "PCevalTest.h"
#include "PCglobals.h"
#include "PCauto.h"
#include "PCwhen.h"
#include "PCwhere.h"
#include "PCwhy.h"


typedef HTable<PCmetric *>;
typedef HTable<datum *>;
typedef HTable<focus *>;
// typedef HTable<metricInstance *>;
typedef List<datum *>;
// typedef List<focus *>;
// typedef List<focusList *>;
typedef List<hint *>;
typedef List<hypothesis *>;
// typedef List<metricInstance *>;
typedef List<searchHistoryNode *>;
typedef List<test *>;
typedef List<testResult *>;
typedef List<timeInterval *>;

