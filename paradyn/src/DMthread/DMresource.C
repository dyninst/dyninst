/*
 *  Copyright 1993 Jeff Hollingsworth.  All rights reserved.
 *
 */

#ifndef lint
static char Copyright[] = "@(#) Copyright (c) 1993 Jeff Hollingsowrth\
    All rights reserved.";

static char rcsid[] = "@(#) $Header: /home/jaw/CVSROOT_20081103/CVSROOT/core/paradyn/src/DMthread/DMresource.C,v 1.6 1994/06/14 15:25:03 markc Exp $";
#endif

/*
 * resource.C - handle resource creation and queries.
 * 
 * $Log: DMresource.C,v $
 * Revision 1.6  1994/06/14 15:25:03  markc
 * Added new call (sameRoot) to the resource class.  This call is used to
 * determine if two resources have the same parent but are not in an
 * ancestor-descendant relationship.  Such a relationship implies a conflict
 * in the two foci.
 *
 * Revision 1.5  1994/06/02  16:08:17  hollings
 * fixed duplicate naming problem for printResources.
 *
 * Revision 1.4  1994/05/31  19:11:34  hollings
 * Changes to permit direct access to resources and resourceLists.
 *
 * Revision 1.3  1994/04/18  22:28:33  hollings
 * Changes to create a canonical form of a resource list.
 *
 * Revision 1.2  1994/02/03  23:26:59  hollings
 * Changes to work with g++ version 2.5.2.
 *
 * Revision 1.1  1994/02/02  00:42:35  hollings
 * Changes to the Data manager to reflect the file naming convention and
 * to support the integration of the Performance Consultant.
 *
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dataManager.h"
#include "DMresource.h"

stringPool resource::names;
resource *resource::rootResource = new resource();
HTable<resource*> resource::allResources;

stringPool resourceList::names;

//
// used only to construct root.
//
resource::resource()
{
    name = "";
    fullName = "";
    parent = NULL;
}

resource::resource(resource *p, char *newResource) 
{
    char *iName;
    char tempName[255];
    // perfStreamList perf;

    iName = names.findAndAdd(newResource);

    parent = p;

    sprintf(tempName, "%s/%s", p->fullName, iName);
    fullName = names.findAndAdd(tempName);
    allResources.add(this, (void *) fullName);
    name = iName;

    parent->children.add(this);
}

void resource::print()
{
    printf("%s", fullName);
}

/*
 * Convinence function.
 *
 */
Boolean resource::isDescendent(resource *child)
{
    while (child) {
        if (child == this) {
            return(TRUE);
        } else {
            child = child->parent;
        }
    }
    return(FALSE);
}

/*
 * Do the two resources have the same base?
 * Note, since the there is a common root for all nodes,
 * the test for a common base checks the node below the
 * common root.
 */
Boolean resource::sameRoot(resource *other)
{
  resource *myBase, *otherBase, *temp;

  temp = this;
  while (temp->parent)
    {
      myBase = temp;
      temp = temp->parent;
    }
  temp = other;
  while (temp->parent)
    {
      otherBase = temp;
      temp = temp->parent;
    }

  if (myBase == otherBase)
    return TRUE;
  else
    return FALSE;
}

void resourceList::print()
{
    int i;

    printf("<");
    lock();
    for (i=0; i < count; i++) {
	if (i) printf(",");
	elements[i]->print();
    }
    unlock();
    printf(">");
}

resource *resourceList::find(char *name) 
{
    int i;
    resource *ret;

    name = resource::names.findAndAdd(name);

    lock();
    for (i=0, ret = NULL; i < count; i++) {
	if (elements[i]->name == name) {
	    ret = elements[i];
	    break;
	}
    }
    unlock();
    return(ret);
}

char ** resourceList::convertToStringList() 
{
    int i;
    char **temp;

    lock();
    temp = (char **) malloc(sizeof(char *) * count);
    for (i=0; i < count; i++) {
	temp[i] = elements[i]->fullName;
    }
    unlock();
    return(temp);
}

char *resourceList::getCanonicalName()
{
    int i;
    int total;
    char *tempName;
    char **temp;
    extern int strCompare(char **a, char **b);

    lock();
    if (!fullName) {
	temp = (char **) malloc(sizeof(char *) * count);
	for (i=0; i < count; i++) {
	    temp[i] = elements[i]->fullName;
	}
	qsort(temp, count, sizeof(char *), strCompare);

	total = 2;
	for (i=0; i < count; i++) total += strlen(temp[i])+2;

	tempName = new(char[total]);
	strcpy(tempName, "<");
	for (i=0; i < count; i++) {
	    if (i) strcat(tempName, ",");
	    strcat(tempName, temp[i]);
	}
	strcat(tempName, ">");

	fullName = names.findAndAdd(tempName);
	delete(tempName);
    }
    unlock();
    return(fullName);
}

void printAllResources()
{
    HTable<resource*> curr;

    for (curr=  resource::allResources; *curr; curr++) {
        (*curr)->print();
        printf("\n");
    }
}
