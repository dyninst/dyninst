/*
 * 
 * $Log: PCwhy.C,v $
 * Revision 1.1  1994/02/02 00:38:24  hollings
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

void test::print()
{
    printf("%s", name);
}

test::test(changeCollectionFunc on, evalFunc eval, char *id)
{
    changeCollection = on;
    evaluate = eval;
    name = id;
}

extern void defaultExplanation(searchHistoryNode *explainee);

hypothesis::hypothesis(hypothesis *p, test *t , char *id)
{
    explanation = &defaultExplanation;
    children = NULL;
    preCondition = p;
    testObject = t;
    name = id;

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


hypothesis::hypothesis(hypothesis *p, test *t , char *id, explanationFunction e)
{
    explanation = e;
    children = NULL;
    preCondition = p;
    testObject = t;
    name = id;

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

void hypothesis::print(int level)
{
    int i;
    hypothesisList curr;

    for (i=0; i < level; i++) printf("    ");
    printf("%s", name);

    if (level >= 0) {
	level++;
	printf("\n");
	if (children) {
	    for (curr = *children; *curr; curr++) {
		(*curr)->print(level);
	    }
	}
    }
}

void hypothesis::unLabel()
{
    hypothesisList curr;

    labeled = FALSE;
    if (!children) return;
    for (curr = *children; *curr; curr++) {
        (*curr)->unLabel();
    }
}

