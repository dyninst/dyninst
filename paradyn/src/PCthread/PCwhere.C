
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
 * $Log: PCwhere.C,v $
 * Revision 1.8  1994/06/29 02:56:28  hollings
 * AFS path changes?  I am not sure why this changed.
 *
 * Revision 1.7  1994/06/22  22:58:26  hollings
 * Compiler warnings and copyrights.
 *
 * Revision 1.6  1994/06/14  17:20:39  markc
 * Modified PCwhere.C to see if the magnification is an ancestor of the
 * current focus.
 *
 * Revision 1.5  1994/06/14  15:33:59  markc
 * Modified moreSpecific test to check for conflicting magnifications, where
 * the magnification has the same base as the current focus, but one is not
 * an ancestor of the other.
 *
 * Revision 1.4  1994/05/31  19:11:42  hollings
 * Changes to permit direct access to resources and resourceLists.
 *
 * Revision 1.3  1994/02/09  22:35:49  hollings
 * fixed pointers refs that pur caught.
 *
 * Revision 1.2  1994/02/03  23:27:05  hollings
 * Changes to work with g++ version 2.5.2.
 *
 * Revision 1.1  1994/02/02  00:38:23  hollings
 * First version of the Performance Consultant using threads.
 *
 * Revision 1.10  1993/12/15  21:03:50  hollings
 * added machine back in into the where hierachy.
 *
 * Revision 1.9  1993/08/05  18:58:43  hollings
 * new includes.
 *
 * Revision 1.8  1993/05/07  20:19:17  hollings
 * Upgrade to use dyninst interface.
 *
 * Revision 1.7  1993/03/23  17:17:06  hollings
 * switch to ANSI string functions.
 *
 * Revision 1.6  1993/02/03  00:06:49  hollings
 * removed execesive friends of focus and focusCell.
 *
 * Revision 1.5  1993/01/28  19:33:28  hollings
 * bzero to memset.
 *
 * Revision 1.4  1992/12/14  19:56:57  hollings
 * added true enable/disable.
 *
 * Revision 1.3  1992/10/23  20:13:37  hollings
 * Working version right after prelim.
 *
 * Revision 1.2  1992/08/24  15:06:33  hollings
 * first cut at automated refinement.
 *
 * Revision 1.1  1992/08/03  20:42:59  hollings
 * Initial revision
 *
 *
 */

#ifndef lint
static char Copyright[] = "@(#) Copyright (c) 1993, 1994 Barton P. Miller, \
  Jeff Hollingsworth, Jon Cargille, Krishna Kunchithapadam, Karen Karavanic,\
  Tia Newhall, Mark Callaghan.  All rights reserved.";

static char rcsid[] = "@(#) $Header: /home/jaw/CVSROOT_20081103/CVSROOT/core/paradyn/src/PCthread/Attic/PCwhere.C,v 1.8 1994/06/29 02:56:28 hollings Exp $";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "../DMthread/DMresource.h"
#include "PCwhere.h"
#include "PCglobals.h"

int focus::operator =(focus f)
{
    int i;
    int limit;
    void *ret;

    data = new resourceList;
    limit = f.data->getCount();
    for (i=0; i < limit; i++) {
	data->add(f.data->getNth(i));
    }
    return((int)ret);
}

focus::focus(resourceList *val)
{

    data = val;
    name = NULL;
    updateName();
}

int strCompare(char **a, char **b)
{
     return(strcmp(*a, *b));
}

void focus::updateName()
{
    int i;
    int limit;
    char **names;
    resource *res;
    char temp[256];

    strcpy(temp, "<");
    if (data) {
	limit = data->getCount();
	names = (char **) malloc(sizeof(char *) * limit);
	for (i=0; i < limit; i++) {
	    res = data->getNth(i);
	    names[i] = res->getFullName();
	}
	/* make sure we get a canocial form by sorting the name */
	qsort((char *) names, limit, sizeof(char *), strCompare);
	for (i=0; i < limit; i++) {
	    strcat(temp, names[i]);
	    if (i != limit - 1) strcat(temp, ",");
	}
	free(names);
    }
    strcat(temp, ">");
    name = strSpace.findAndAdd(temp);
}

focus::focus()
{
    data = new resourceList;
    name = NULL;
}

Boolean focus::operator ==(focus *f)
{
    if (name == f->name)
	return(TRUE);
    else
	return(FALSE);
}

void focus::print() 
{
    if (data)
	printf(name);
    else
	printf("<>");
}

void focus::print(char *ret)
{
    if (data)
	strcpy(ret, name);
    else
	strcpy(ret, "<>");
}

focusList *focus::findMagnifyCache(resource *cell)
{
    focusList *result;

    result = magnifyCache.find(cell);
    return(result);
}

void focus::addMagnifyCache(resource *cell, focusList *result)
{
    magnifyCache.add(result, (void*)cell);
}

focus *focus::findConstrainCache(resource *cell)
{
    focus *result;

    result = constrainCache.find(cell);
    return(result);
}

void focus::addConstrainCache(resource *cell, focus *result)
{
    constrainCache.add(result, (void*)cell);
}

void printCurrentFocus()
{
    focusList curr;

    // ugly cast to void to shut up g++ 2.5.8 - jkh 6/22/94
    for ((void) (curr=globalFocus); *curr; curr++) {
       (*curr)->print();
    }
}

/* ARGSUSED */
focusList focus::magnify(resource *param)
{
    int inner;
    int count;
    int oLimit;
    int iLimit;
    focus *curr;
    focus *ret;
    resource *fc;
    resource *tc;
    focus *parent;
    resourceList *c;
    focusList *result;
    resourceList *resList;
    Boolean conflicts;

    // see if we have done this one before.
    result = findMagnifyCache(param);
    if (result) return(*result);
    result = new(focusList);

    parent = moreSpecific(param, conflicts);
    // our current focus is more specific.
    if (!parent) {
      if (!conflicts) {
	result->add(this, (void *) this);
	count = 1;
      }
      // do nothing if conflicts = TRUE
    } else {
	// for each known focus cell.
	c = param->getChildren();
	oLimit = c->getCount();
	for (count = 0; count < oLimit; count++) {
	    fc = c->getNth(count);
	    resList = new resourceList;
	    iLimit = data->getCount();
	    for (inner=0; inner < iLimit; inner++) {
		tc = data->getNth(inner);
		if (tc->isDescendent(fc)) {
		    resList->add(fc);
		} else {
		    resList->add(tc);
		}
	    }
	    // generate a new name for it.
	    curr = new focus(resList);
	    ret = allFoci.find(curr);
	    if (!ret) {
		allFoci.addUnique(curr);
		result->add(curr, (void *) curr);
	    } else {
		// add to resulting focus.
		result->add(ret, (void *) ret);
		free(curr->data);
		delete(curr);
	    }
	}
    }
    if (count) {
	addMagnifyCache(param, result);
    } else {
	delete(result);
    }
    return(*result);
}

focusList focus::enumerateRefinements()
{
    focus *f;
    int limit;
    int i, j, k;
    resource *res;
    int childCount;
    focus *realFocus;
    focusList returnList;
    resourceList *children;
    resourceList *destList;


    limit = data->getCount();
    for (i=0; i < limit; i++) {
	res = data->getNth(i);
	children = res->getChildren();
	childCount = children->getCount();
	for (j=0; j < childCount; j++) {
	    destList = new resourceList;
	    f = new focus(destList);
	    for (k=0; k < limit; k++) {
		if (k == i) {
		    destList->add(children->getNth(j));
		} else {
		    destList->add(data->getNth(k));
		}
	    }

	    // generate a new name for it.
	    f->updateName();
	    realFocus = allFoci.find(f);
	    if (!realFocus) {
		allFoci.addUnique(f);
		realFocus = f;
	    } else {
		// already been defined.
		free(destList);
		delete(f);
	    }
	    returnList.addUnique(realFocus);
	}
    }
    return(returnList);
}

/* ARGSUSED */
focus *focus::constrain(resource *param)
{
    int i;
    focus *nf;
    focus *ret;
    Boolean found;
    resource *curr;
    resourceList *newCellList;

    // see if we have done this one before.
    ret = findConstrainCache(param);
    if (ret) return(ret);

    found = FALSE;
    ret = NULL;
    newCellList = new resourceList;
    for (i=0; i < data->getCount(); i++) {
	curr = data->getNth(i);
	if (curr->isDescendent(param) == TRUE) {
	    newCellList->add(param);
	    found = TRUE;
	} else {
	    newCellList->add(curr);
	}
    }
    if (found) {
	nf = new focus(newCellList);
	ret = allFoci.find(nf);
	addConstrainCache(param, ret);
	delete(nf);
	return(ret);
    } else {
	addConstrainCache(param, this);
	return(this);
    }
}

//
// given a focus and a resource, see if the resource is more specific 
//   (more magnified) than the corresponding resource in the passed focus.
// If it is return the more specific focus, otherwise return null.
//
focus *focus::moreSpecific(resource *parm, Boolean &conflicts)
{
    int i;
    focus *nf;
    int limit;
    focus *ret;
    resource *curr;
    Boolean found;
    resourceList *newList;

    assert(data);
    conflicts = FALSE;
    limit = data->getCount();
    newList = new resourceList;
    found = FALSE;
    for (i=0; i < limit; i++) {
	curr = data->getNth(i);
	if (curr->isDescendent(parm) == TRUE) {
	  newList->add(parm);
	  found = TRUE;
	} else if (parm->isDescendent(curr) == TRUE) {
          // the current focus is more specific - it will be reused
	  found = FALSE;
	  break;
	} else if (strcmp(curr->getName(), parm->getName()) &&
		   (curr->sameRoot(parm) == TRUE)) {
	  // if the resource and param have the same root, but
	  // one is not a descendant of the other,
	  // and they are not the same, then this
	  // magnification conflicts with the current focus
	    conflicts = TRUE;
	    found = FALSE;
	    break;
	} else {
	    newList->add(curr);
	}
    }
    if (found) {
	nf = new focus(newList);
	ret = allFoci.find(nf);
	if (ret) {
	    free(newList);
	    delete(nf);
	    return(ret);
	} else {
	    allFoci.addUnique(nf);
	    return(nf);
	}
    } else {
	// not more specific.
	free(newList);
	return(NULL);
    }
}

void initResources()
{
    resource *root;

    root = dataMgr->getRootResource();
    SyncObject = dataMgr->newResource(context, root, "SyncObject");
    Procedures = dataMgr->newResource(context, root, "Procedure");
    Processes = dataMgr->newResource(context, root, "Process");
    Machines = dataMgr->newResource(context, root, "Machine");
    Locks = dataMgr->newResource(context, SyncObject, "SpinLock");
    Barriers = dataMgr->newResource(context, SyncObject, "Barrier");
    Semaphores = dataMgr->newResource(context, SyncObject, "Semaphore");
    MsgTags = dataMgr->newResource(context, SyncObject, "MsgTag");

    whereAxis = new focus(dataMgr->getRootResources());
    globalFocus.addUnique(whereAxis);
    allFoci.addUnique(whereAxis);

}

// common string pool
stringPool strSpace;

//
// root resources
//
resource *SyncObject;
resource *Procedures;
resource *Processes;
resource *Machines;

//
// Some common focus classes that tests wish to magnify on.
//   We try to avoid these where possible because it violates the 
//   orthogonality of the why and where axes.
//
resource *Locks;
resource *Barriers;
resource *Semaphores;
resource *MsgTags;

//
// Looking at the entire global view.
//
focus *whereAxis;

//
// THE current focus.
//
focus *currentFocus;

//
// current points of focus.
//
focusList globalFocus;
focusList allFoci;
