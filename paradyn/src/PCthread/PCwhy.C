
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
 * 
 * $Log: PCwhy.C,v $
 * Revision 1.8  1995/06/02 20:50:20  newhall
 * made code compatable with new DM interface
 *
 * Revision 1.7  1995/02/16  08:19:27  markc
 * Changed Boolean to bool
 *
 * Revision 1.6  1994/12/21  00:46:37  tamches
 * Minor changes that reduced the number of compiler warnings; e.g.
 * Boolean to bool.  operator<< routines now return their ostream
 * argument properly.
 *
 * Revision 1.5  1994/10/25  22:08:24  hollings
 * changed print member functions to ostream operators.
 *
 * Fixed lots of small issues related to the cost model for the
 * Cost Model paper.
 *
 * Revision 1.4  1994/09/22  01:11:12  markc
 * Added const to char* in
 *  hypothesis::hypothesis(hypothesis *p, test *t , const char *id, explanationFunction e
 * hypothesis::hypothesis(hypothesis *p, test *t , const char *id)
 * test::test(changeCollectionFunc on, evalFunc eval, const char *id)
 *
 * Revision 1.3  1994/06/29  02:56:30  hollings
 * AFS path changes?  I am not sure why this changed.
 *
 * Revision 1.2  1994/06/22  22:58:27  hollings
 * Compiler warnings and copyrights.
 *
 * Revision 1.1  1994/02/02  00:38:24  hollings
 * First version of the Performance Consultant using threads.
 *
 * Revision 1.7  1993/09/03  19:04:41  hollings
 * fixed totaling of costs.
 *
 * Revision 1.6  1993/08/11  18:52:31  hollings
 * adedd cost function.
 *
 * Revision 1.5  1993/08/05  18:58:58  hollings
 * new includes.
 *
 * Revision 1.4  1993/05/07  20:19:17  hollings
 * Upgrade to use dyninst interface.
 *
 * Revision 1.3  1992/12/14  19:56:57  hollings
 * added true enable/disable.
 *
 * Revision 1.2  1992/08/24  15:06:33  hollings
 * first cut at automated refinement.
 *
 * Revision 1.1  1992/08/03  20:42:59  hollings
 * Initial revision
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "PCwhy.h"
#include "PCwhen.h"

#include "PCmetric.h"
#include "PCglobals.h"

#include <string.h>

ostream& operator <<(ostream &os, test& t)
{
    os << t.name;
}

test::test(changeCollectionFunc on, evalFunc eval, const char *id)
{
    changeCollection = on;
    evaluate = eval;
    name = strdup(id);
}

extern void defaultExplanation(searchHistoryNode *explainee);

hypothesis::hypothesis(hypothesis *p, test *t , const char *id)
{
    explanation = &defaultExplanation;
    children = NULL;
    preCondition = p;
    testObject = t;
    if (id)
      name = strdup(id);
    else 
      name = 0;

    allHypotheses.add(this);
    if (t) {
	allTests.addUnique(t);
    }

    if (preCondition) {
	if (!preCondition->children) 
	    preCondition->children = new(hypothesisList);
	preCondition->children->add(this);
    }
}


hypothesis::hypothesis(hypothesis *p, test *t , const char *id, explanationFunction e)
{
    explanation = e;
    children = NULL;
    preCondition = p;
    testObject = t;
    if (id)
      name = strdup(id);
    else
      name = 0;

    allHypotheses.add(this);
    if (t) {
	allTests.addUnique(t);
    }

    if (preCondition) {
	if (!preCondition->children) 
	    preCondition->children = new(hypothesisList);
	preCondition->children->add(this);
    }
}

float hypothesis::cost()
{
    float total;
    hypothesisList curr;

    if (testObject) {
        return(testObject->changeCollection(getCollectionCost));
    } else {
	total = 0.0;
        for (curr = *children; *curr; curr++) {
            total += (*curr)->cost();
        }
	return(total);
    }
}

ostream& operator <<(ostream &os, hypothesis& hyp)
{
     os << hyp.name;
     return os; // added AT 12/8/94
}

void hypothesis::unLabel()
{
    hypothesisList curr;

    labeled = false;
    if (!children) return;
    for (curr = *children; *curr; curr++) {
        (*curr)->unLabel();
    }
}

