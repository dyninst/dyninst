/*
 *  Copyright 1993 Jeff Hollingsworth.  All rights reserved.
 *
 */

#ifndef lint
static char Copyright[] = "@(#) Copyright (c) 1993 Jeff Hollingsowrth\
    All rights reserved.";

static char rcsid[] = "@(#) $Header: /home/jaw/CVSROOT_20081103/CVSROOT/core/paradynd/src/resource.C,v 1.5 1994/06/02 23:28:00 markc Exp $";
#endif

/*
 * resource.C - handle resource creation and queries.
 *
 * $Log: resource.C,v $
 * Revision 1.5  1994/06/02 23:28:00  markc
 * Replaced references to igen generated class to a new class derived from
 * this class to implement error handling for igen code.
 *
 * Revision 1.4  1994/05/16  22:31:54  hollings
 * added way to request unique resource name.
 *
 * Revision 1.3  1994/02/24  04:32:36  markc
 * Changed header files to reflect igen changes.  main.C does not look at the number of command line arguments now.
 *
 * Revision 1.2  1994/02/01  18:46:55  hollings
 * Changes for adding perfConsult thread.
 *
 * Revision 1.1  1994/01/27  20:31:41  hollings
 * Iinital version of paradynd speaking dynRPC igend protocol.
 *
 * Revision 1.3  1993/07/13  18:30:02  hollings
 * new include file syntax.
 * expanded tempName to 255 chars for c++ support.
 *
 * Revision 1.2  1993/06/08  20:14:34  hollings
 * state prior to bc net ptrace replacement.
 *
 * Revision 1.1  1993/03/19  22:45:45  hollings
 * Initial revision
 *
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "symtab.h"
#include "process.h"
#include "dyninstP.h"
#include "util.h"
#include "comm.h"

extern pdRPC *tp;
HTable <resource>	allResources;

/*
 * handle the notification of resource creation and deletion.
 *
 */

_resourceRec rootNode(False);

resource rootResource = &rootNode;

resourceList getRootResources()
{
    return(rootResource->children);
}

char *getResourceName(resource r)
{
    return(r->info.name);
}

resource getResourceParent(resource r)
{
    return(r->parent);
}

resourceList getResourceChildren(resource r)
{
    return(r->children);
}

int getResourceCount(resourceList rl)
{
    return(rl ? rl->count: 0);
}

resource getNthResource(resourceList rl, int n)
{
    if (n < rl->count) {
	return(rl->elements[n]);
    } else {
	return(NULL);
    }
}

resourceInfo *getResourceInfo(resource r)
{
    return(&r->info);
}

resourceList createResourceList()
{
    resourceList ret;

    ret = (resourceList) xcalloc(sizeof(struct _resourceListRec), 1);
    return(ret);
}

Boolean addResourceList(resourceList rl, resource r)
{

    if (rl->count == rl->maxItems) {
	rl->maxItems += 10;
	if (rl->elements) {
	    rl->elements = (resource *) 
		xrealloc(rl->elements, sizeof(resource) * rl->maxItems);
	} else {
	    rl->elements = (resource *) xmalloc(sizeof(resource) * rl->maxItems);
	}
    }
    rl->elements[rl->count] = r;
    rl->count++;
    return(True);
}

Boolean initResourceRoot;

resource newResource(resource parent, 
		     void *handle, 
		     char *name, 
		     timeStamp creation,
		     Boolean unique)
{
    int c;
    char *iName;
    resource ret;
    resource *curr;
    char tempName[255];

    if (unique) {
	// ask paradyn for unqiue name.
	name = tp->getUniqueResource(0, parent->info.fullName, name);
    }

    if (!initResourceRoot) {
	initResourceRoot = True;
	rootNode.info.name = "";
	rootNode.info.fullName = "";
	rootNode.info.creation = 0.0;
	rootNode.parent = NULL;
	rootNode.handle = NULL;
	rootNode.children = NULL;
    }

    iName = pool.findAndAdd(name);

    /* first check to see if the resource has already been defined */
    if (parent->children) {
	for (curr=parent->children->elements, c=0;
	     c < parent->children->count; c++) {
	     if (curr[c]->info.name == iName) {
		 return(curr[c]);
	     }
	}
    } else {
	parent->children = (resourceList) 
	    xcalloc(sizeof(struct _resourceListRec), 1);
    }

    ret = new(_resourceRec);
    ret->parent = parent;
    ret->handle = handle;

    sprintf(tempName, "%s/%s", parent->info.fullName, name);
    ret->info.fullName = pool.findAndAdd(tempName);
    ret->info.name = iName;

    ret->info.creation = creation;

    addResourceList(parent->children, ret);
    allResources.add(ret, (void *) ret->info.fullName);

    /* call notification upcall */
    tp->resourceInfoCallback(0, parent->info.fullName, ret->info.name, 
 	 ret->info.name);

    return(ret);
}

resource findChildResource(resource parent, char *name)
{
    int c;
    char *iName;
    resource *curr;

    iName = pool.findAndAdd(name);

    if (!parent || !parent->children) return(NULL);
    for (curr=parent->children->elements, c=0;
	 c < parent->children->count; c++) {
	 if (curr[c]->info.name == iName) {
	     return(curr[c]);
	 }
    }
    return(NULL);
}

void printResources(resource r)
{
    int c;
    resource *curr;

    if (r) {
	printf("%s\n", r->info.fullName);
	if (r->children) {
	    for (curr=r->children->elements, c=0;
		 c < r->children->count; c++) {
		 printResources(curr[c]);
	    }
	}
    }
}

void printResourceList(resourceList rl)
{
    int i;

    printf("<");
    for (i=0; i < rl->count; i++) {
	printf(rl->elements[i]->info.fullName);
	if (i!= rl->count-1) printf(",");
    }
    printf(">");
}

/*
 * Convinence function.
 *
 */
Boolean isResourceDescendent(resource parent, resource child)
{
    while (child) {
        if (child == parent) {
            return(True);
        } else {
            child = getResourceParent(child);
        }
    }
    return(False);
}

//
// Find this passed focus if its resource components are valid for paradynd.
//    We treat "unknown" top level resources as a specical case since the
//    meaning is that we are at the top level of refinement of an unsupported
//    resource hierarchy.  In this case, we simply create the resource, and
//    the focus is valid.  Any other resource that can't be found results in
//    an invalid focus.  jkh 1/29/94.
//
resourceList findFocus(int count, char **data)
{
    int i;
    char *iName;
    resource res;
    resourceList rl;

    rl = createResourceList();
    for (i=0; i < count; i++) {
	iName = pool.findAndAdd(data[i]);
	res = allResources.find(iName);
	if (!res && (strchr(iName, '/') == strrchr(iName, '/'))) {
	    res = newResource(rootResource, NULL, ++iName, 0.0, FALSE);
	} else if (!res) {
	    return(NULL);
	}
	addResourceList(rl, res);
    }
    return(rl);
}

resource findResource(char *name)
{
    char *iName;
    resource res;

    if (*name == '\0') {
	return(rootResource);
    } else {
	iName = pool.findAndAdd(name);
	res = allResources.find(iName);
	return(res);
    }
}
