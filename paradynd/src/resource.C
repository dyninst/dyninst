/*
 *  Copyright 1993 Jeff Hollingsworth.  All rights reserved.
 *
 */

#ifndef lint
static char Copyright[] = "@(#) Copyright (c) 1993 Jeff Hollingsowrth\
    All rights reserved.";

static char rcsid[] = "@(#) $Header: /home/jaw/CVSROOT_20081103/CVSROOT/core/paradynd/src/resource.C,v 1.10 1994/09/22 02:24:46 markc Exp $";
#endif

/*
 * resource.C - handle resource creation and queries.
 *
 * $Log: resource.C,v $
 * Revision 1.10  1994/09/22 02:24:46  markc
 * cast stringHandles
 *
 * Revision 1.9  1994/08/08  20:13:46  hollings
 * Added suppress instrumentation command.
 *
 * Revision 1.8  1994/07/28  22:40:45  krisna
 * changed definitions/declarations of xalloc functions to conform to alloc.
 *
 * Revision 1.7  1994/06/27  21:28:20  rbi
 * Abstraction-specific resources and mapping info
 *
 * Revision 1.6  1994/06/27  18:57:08  hollings
 * removed printfs.  Now use logLine so it works in the remote case.
 * added internalMetric class.
 * added extra paramter to metric info for aggregation.
 *
 * Revision 1.5  1994/06/02  23:28:00  markc
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

extern "C" {
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
}

#include "symtab.h"
#include "process.h"
#include "dyninstP.h"
#include "util.h"
#include "comm.h"

extern pdRPC *tp;
HTable <resource*> allResources;

/*
 * handle the notification of resource creation and deletion.
 *
 */

resource rootNode(False);

resource *rootResource = &rootNode;

resourceListRec *getRootResources()
{
    return(rootResource->children);
}

stringHandle getResourceName(resource *r)
{
    return(r->info.name);
}

resource *getResourceParent(resource *r)
{
    return(r->parent);
}

resourceListRec *getResourceChildren(resource *r)
{
    return(r->children);
}

int getResourceCount(resourceListRec *rl)
{
    return(rl ? rl->count: 0);
}

resource *getNthResource(resourceListRec *rl, int n)
{
    if (n < rl->count) {
	return(rl->elements[n]);
    } else {
	return(NULL);
    }
}

resourceInfo *getResourceInfo(resource *r)
{
    return(&r->info);
}

resourceListRec *createResourceList()
{
    resourceListRec *ret;
    ret = new resourceListRec;
    return(ret);
}

Boolean addResourceList(resourceListRec *rl, resource *r)
{
    if (rl->count == rl->maxItems) {
	rl->maxItems += 10;
	if (rl->elements) {
            int i;
	    resource **newEl;
            newEl = new resource*[rl->maxItems];        
	    for (i=0; i<(rl->maxItems - 10); ++i)
	      newEl[i] = rl->elements[i];
	    // don't want delete called on elements in this list
	    delete (rl->elements);
	    rl->elements = newEl;
	} else {
	    rl->elements = new resource*[rl->maxItems];
	}
    }
    rl->elements[rl->count] = r;
    rl->count++;
    return(True);
}

Boolean initResourceRoot;

resource *newResource(resource *parent, 
		      void *handle,
		      stringHandle abstraction, 
		      const char *name, 
		      timeStamp creation,
		      Boolean unique)
{
    int c;
    resource *ret;
    resource **curr;
    char tempName[255];
    stringHandle nameHandle;
    char *uName=0;

    if (unique) {
      // ask paradyn for unqiue name.
      uName = tp->getUniqueResource(0, (char*)parent->info.fullName, (char*)name);
      nameHandle = pool.findAndAdd(uName);
      delete uName;
    } else
      nameHandle = pool.findAndAdd(name);

    if (!initResourceRoot) {
	initResourceRoot = True;
	rootNode.info.name = pool.findAndAdd("");
	rootNode.info.fullName = pool.findAndAdd("");
	rootNode.info.creation = 0.0;
	rootNode.parent = NULL;
	rootNode.handle = NULL;
	rootNode.children = NULL;
    }

    /* first check to see if the resource has already been defined */
    if (parent->children) {
	for (curr=parent->children->elements, c=0;
	     c < parent->children->count; c++) {
	     if (curr[c]->info.name == nameHandle) {
		 return(curr[c]);
	     }
	}
    } else {
	parent->children = new resourceListRec;
    }

    ret = new resource;
    ret->parent = parent;
    ret->handle = handle;

    sprintf(tempName, "%s/%s", (char*)parent->info.fullName, (char*)nameHandle);
    ret->info.fullName = pool.findAndAdd(tempName);
    ret->info.name = nameHandle;
    ret->info.abstraction = abstraction;

    ret->info.creation = creation;

    addResourceList(parent->children, ret);
    allResources.add(ret, (void *) ret->info.fullName);

    /* call notification upcall */
    tp->resourceInfoCallback(0, (char*)parent->info.fullName, (char*)ret->info.name, 
 	 (char*)ret->info.name, (char*)ret->info.abstraction);

    return(ret);
}

resource *findChildResource(resource *parent, char *name)
{
    int c;
    stringHandle iName;
    resource **curr;

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

void printResources(resource *r)
{
    int c;
    resource **curr;

    if (r) {
	sprintf(errorLine, "%s", (char*)r->info.fullName);
	logLine(errorLine);
	if (r->children) {
	    for (curr=r->children->elements, c=0;
		 c < r->children->count; c++) {
		 printResources(curr[c]);
	    }
	}
    }
}

void printResourceList(resourceListRec *rl)
{
    int i;

    logLine("<");
    for (i=0; i < rl->count; i++) {
	logLine((char*)rl->elements[i]->info.fullName);
	if (i!= rl->count-1) logLine(",");
    }
    logLine(">");
}

/*
 * Convinence function.
 *
 */
Boolean isResourceDescendent(resource *parent, resource *child)
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
resourceListRec *findFocus(int count, char **data)
{
    int i;
    stringHandle iName;
    resource *res;
    resourceListRec *rl;
    char *iChar;

    rl = createResourceList();
    for (i=0; i < count; i++) {
	iName = pool.findAndAdd(data[i]);
	res = allResources.find(iName);
	if (!res && (strchr((char*)iName, '/') == strrchr((char*)iName, '/'))) {
	    iChar = (char*) iName;
	    iChar++;
	    res = newResource(rootResource, NULL, NULL, iChar, 0.0, FALSE);
	} else if (!res) {
	    return(NULL);
	}
	addResourceList(rl, res);
    }
    return(rl);
}

resource *findResource(const char *name)
{
    stringHandle iName;
    resource *res;

    if (*name == '\0') {
	return(rootResource);
    } else {
	iName = pool.findAndAdd(name);
	res = allResources.find(iName);
	return(res);
    }
}
