/*
 * 
 * $Log: PCwhere.C,v $
 * Revision 1.1  1994/02/02 00:38:23  hollings
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
static char Copyright[] = "@(#) Copyright (c) 1992 Jeff Hollingsowrth\
    All rights reserved.";

static char rcsid[] = "@(#) $Header: /home/jaw/CVSROOT_20081103/CVSROOT/core/paradyn/src/PCthread/Attic/PCwhere.C,v 1.1 1994/02/02 00:38:23 hollings Exp $";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "PCwhere.h"
#include "PCglobals.h"

int focus::operator =(focus f)
{
    int i;
    int limit;
    void *ret;

    data = dataMgr->createResourceList();
    limit = dataMgr->getResourceCount(f.data);
    for (i=0; i < limit; i++) {
	dataMgr->addResourceList(data, dataMgr->getNthResource(f.data, i));
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
    char temp[256];

    strcpy(temp, "<");
    if (data) {
	limit = dataMgr->getResourceCount(data);
	names = (char **) malloc(sizeof(char *) * limit);
	for (i=0; i < limit; i++) {
	    // ??? should this be the full name???
	    names[i] = dataMgr->getResourceName(dataMgr->getNthResource(data, i));
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
    data = dataMgr->createResourceList();
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

    for (curr=globalFocus; *curr; curr++) {
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

    // see if we have done this one before.
    result = findMagnifyCache(param);
    if (result) return(*result);
    result = new(focusList);

    parent = moreSpecific(param);
    // our current focus is more specific.
    if (!parent) {
	result->add(this, (void *) this);
	count = 1;
    } else {
	// for each known focus cell.
	c = dataMgr->getResourceChildren(param);
	oLimit = dataMgr->getResourceCount(c);
	for (count = 0; count < oLimit; count++) {
	    fc = dataMgr->getNthResource(c, count);
	    resList = dataMgr->createResourceList();
	    iLimit = dataMgr->getResourceCount(data);
	    for (inner=0; inner < iLimit; inner++) {
		tc = dataMgr->getNthResource(data, inner);
		if (dataMgr->isResourceDescendent(tc, fc)) {
		    dataMgr->addResourceList(resList, fc);
		} else {
		    dataMgr->addResourceList(resList, tc);
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
    int childCount;
    focus *realFocus;
    focusList returnList;
    resourceList *children;
    resourceList *destList;


    limit = dataMgr->getResourceCount(data);
    for (i=0; i < limit; i++) {
	children = dataMgr->getResourceChildren(dataMgr->getNthResource(data, i));
	childCount = dataMgr->getResourceCount(children);
	for (j=0; j < childCount; j++) {
	    destList = dataMgr->createResourceList();
	    f = new focus(destList);
	    for (k=0; k < limit; k++) {
		if (k == i) {
		    dataMgr->addResourceList(destList, dataMgr->getNthResource(children, j));
		} else {
		    dataMgr->addResourceList(destList, dataMgr->getNthResource(data, k));
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
    newCellList = dataMgr->createResourceList();
    for (i=0; i < dataMgr->getResourceCount(data); i++) {
	curr = dataMgr->getNthResource(data, i);
	if (dataMgr->isResourceDescendent(curr, param) == TRUE) {
	    dataMgr->addResourceList(newCellList, param);
	    found = TRUE;
	} else {
	    dataMgr->addResourceList(newCellList, curr);
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
focus *focus::moreSpecific(resource *parm)
{
    int i;
    focus *nf;
    int limit;
    focus *ret;
    resource *curr;
    Boolean found;
    resourceList *newList;

    assert(data);
    limit = dataMgr->getResourceCount(data);
    newList = dataMgr->createResourceList();
    for (i=0; i < limit; i++) {
	curr = dataMgr->getNthResource(data, i);
	if (dataMgr->isResourceDescendent(curr, parm) == TRUE) {
	    dataMgr->addResourceList(newList, parm);
	    found = TRUE;
	} else {
	    dataMgr->addResourceList(newList, curr);
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
