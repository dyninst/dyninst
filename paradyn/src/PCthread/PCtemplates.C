
/*
 * Copyright (c) 1993, 1994 Barton P. Miller, Jeff Hollingsworth,
 *     Bruce Irvin, Jon Cargille, Krishna Kunchithapadam, Karen
 *     Karavanic, Tia Newhall, Mark Callaghan.  All rights reserved.
 * 
 * This software is furnished under the condition that it may not be
 * provided or otherwise made available to, or used by, any other
 * person, except as provided for by the terms of applicable license
 * agreements.  No title to or ownership of the software is hereby
 * transferred.  The name of the principals may not be used in any
 * advertising or publicity related to this software without specific,
 * written prior authorization.  Any use of this software must include
 * the above copyright notice.
 *
 */

/*
 * Generate all the templates in one file.
 *
 */

/* 
 * $Log: PCtemplates.C,v $
 * Revision 1.2  1994/06/22 22:58:24  hollings
 * Compiler warnings and copyrights.
 *
 * Revision 1.1  1994/05/19  00:00:32  hollings
 * Added tempaltes.
 * Fixed limited number of nodes being evaluated on once.
 * Fixed color coding of nodes.
 *
 *
 */

#ifndef lint
static char Copyright[] = "@(#) Copyright (c) 1993, 1994 Barton P. Miller, \
  Jeff Hollingsworth, Jon Cargille, Krishna Kunchithapadam, Karen Karavanic,\
  Tia Newhall, Mark Callaghan.  All rights reserved.";

static char rcsid[] = "@(#) $Header: /home/jaw/CVSROOT_20081103/CVSROOT/core/paradyn/src/PCthread/Attic/PCtemplates.C,v 1.2 1994/06/22 22:58:24 hollings Exp $";
#endif

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

