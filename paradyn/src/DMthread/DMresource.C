/*
 *  Copyright 1993 Jeff Hollingsworth.  All rights reserved.
 *
 */

#ifndef lint
static char Copyright[] = "@(#) Copyright (c) 1993 Jeff Hollingsowrth\
    All rights reserved.";

static char rcsid[] = "@(#) $Header: /home/jaw/CVSROOT_20081103/CVSROOT/core/paradyn/src/DMthread/DMresource.C,v 1.2 1994/02/03 23:26:59 hollings Exp $";
#endif

/*
 * resource.C - handle resource creation and queries.
 * 
 * $Log: DMresource.C,v $
 * Revision 1.2  1994/02/03 23:26:59  hollings
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

#include "dataManager.h"
#include "DMresource.h"

stringPool resource::names;
resource *resource::rootResource = new resource();
HTable<resource*> resource::allResources;


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

void resourceList::print()
{
    int i;

    printf("<");
    for (i=0; i < count; i++) {
	if (i) printf(",");
	elements[i]->print();
    }
    printf(">");
}

resource *resourceList::find(char *name) 
{
    int i;

    name = resource::names.findAndAdd(name);

    for (i=0; i < count; i++) {
	if (elements[i]->name == name) return(elements[i]);
    }
    return(NULL);
}

char ** resourceList::convertToStringList() 
{
    int i;
    char **temp;

    temp = (char **) malloc(sizeof(char *) * count);
    for (i=0; i < count; i++) {
	temp[i] = elements[i]->fullName;
    }
    return(temp);
}

