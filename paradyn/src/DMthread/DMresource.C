/*
 *  Copyright 1993 Jeff Hollingsworth.  All rights reserved.
 *
 */

#ifndef lint
static char Copyright[] = "@(#) Copyright (c) 1993 Jeff Hollingsowrth\
    All rights reserved.";

static char rcsid[] = "@(#) $Header: /home/jaw/CVSROOT_20081103/CVSROOT/core/paradyn/src/DMthread/DMresource.C,v 1.20 1995/03/02 04:23:21 krisna Exp $";
#endif

/*
 * resource.C - handle resource creation and queries.
 * 
 * $Log: DMresource.C,v $
 * Revision 1.20  1995/03/02 04:23:21  krisna
 * warning and bug fixes.
 *
 * Revision 1.19  1995/02/16  08:17:30  markc
 * Changed Boolean to bool
 * Added function to convert char* lists to vector<string>
 *
 * Revision 1.18  1995/01/26  17:58:24  jcargill
 * Changed igen-generated include files to new naming convention; fixed
 * some bugs compiling with gcc-2.6.3.
 *
 * Revision 1.17  1994/11/07  08:24:40  jcargill
 * Added ability to suppress search on children of a resource, rather than
 * the resource itself.
 *
 * Revision 1.16  1994/11/04  08:46:00  jcargill
 * Made suppressSearch flag be inherited from parent resource.  Solves the
 * problem of having to wait for processes to be defined to suppress them.
 *
 * Revision 1.15  1994/09/30  21:17:44  newhall
 * changed convertToStringList method function return value from
 * stringHandle * to char**
 *
 * Revision 1.14  1994/09/30  19:17:51  rbi
 * Abstraction interface change.
 *
 * Revision 1.13  1994/09/22  00:57:16  markc
 * Entered stringHandles into stringPool rather than assigning from const char *
 * Added casts to remove compiler warnings
 *
 * Revision 1.12  1994/08/05  16:04:00  hollings
 * more consistant use of stringHandle vs. char *.
 *
 * Revision 1.11  1994/07/28  22:31:09  krisna
 * include <rpc/types.h>
 * stringCompare to match qsort prototype
 * proper prorotypes for starting DMmain
 *
 * Revision 1.10  1994/07/26  20:03:06  hollings
 * added suppressSearch.
 *
 * Revision 1.9  1994/07/14  23:45:31  hollings
 * Changed printf of resource to be TCL list like.
 *
 * Revision 1.8  1994/06/27  21:23:31  rbi
 * Abstraction-specific resources and mapping info
 *
 * Revision 1.7  1994/06/17  00:11:55  hollings
 * Fixed off by one error in string canonical string name code.
 *
 * Revision 1.6  1994/06/14  15:25:03  markc
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

#include "dataManager.thread.h"
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
    name = names.findAndAdd("");
    fullName = names.findAndAdd("");
    parent = NULL;
    suppressSearch = FALSE;
    suppressChildSearch = FALSE;
    abstr = NULL;            // We cannot give it a real abstraction
                             // because abstractions have not been initialized
                             // yet.  This is UGLY.  We need a better way
                             // to initialize globals than relying on 
                             // their constructors.
}

resource::resource(resource *p, char *newResource, const char *a) 
{
    stringHandle iName;
    char tempName[255];
    // perfStreamList perf;

    iName = names.findAndAdd(newResource);

    parent = p;

    sprintf(tempName, "%s/%s", (char*)p->fullName, (char*)iName);
    fullName = names.findAndAdd(tempName);
    allResources.add(this, (void *) fullName);
    name = iName;

    abstr = AMfind(a);

    suppressChildSearch = FALSE;
    suppressSearch = p->getSuppressChildren(); // check for suppress of
					       // parent's children
    if (!suppressSearch) 
      suppressSearch = p->getSuppress(); // inherit search suppression from
					 // parent resource
    parent->children.add(this);
}

void resource::print()
{
    if (parent) {
	parent->print();
    }
    printf("%s ", (char*)name);
}

/*
 * Convinence function.
 *
 */
bool resource::isDescendent(resource *child)
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
bool resource::sameRoot(resource *other)
{
  resource *myBase=0, *otherBase=0, *temp;

  temp = this;
  while (temp->parent) {
    myBase = temp;
    temp = temp->parent;
  }
  temp = other;
  while (temp->parent) {
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

    printf("{");
    lock();
    for (i=0; i < count; i++) {
	if (i) printf(" ");
	printf("{");
	elements[i]->print();
	printf("}");
    }
    unlock();
    printf("}");
}

resource *resourceList::find(const char *name) 
{
    int i;
    resource *ret;
    stringHandle iName;

    iName = resource::names.findAndAdd(name);

    lock();
    for (i=0, ret = NULL; i < count; i++) {
	if (elements[i]->name == iName) {
	    ret = elements[i];
	    break;
	}
    }
    unlock();
    return(ret);
}

bool resourceList::convertToStringList(vector<string> &vs) {
    lock();
    for (unsigned i=0; i < count; i++)
      vs += (char *)elements[i]->fullName;
    unlock();
    return true;
}

char **resourceList::convertToStringList() 
{
    int i;
    char **temp;

    lock();
    temp = (char **) malloc(sizeof(char *) * count);
    for (i=0; i < count; i++) {
	temp[i] = (char *)elements[i]->fullName;
    }
    unlock();
    return(temp);
}

static int stringCompare(const void* p1, const void* p2) {
    extern int strCompare(const char* const * a, const char* const * b);
    return (strCompare((const char* const *) p1, (const char* const *) p2));
}

stringHandle resourceList::getCanonicalName()
{
    int i;
    int total;
    char *tempName;
    stringHandle *temp;

    lock();
    if (!fullName) {
	temp = (stringHandle *) malloc(sizeof(stringHandle) * count);
	for (i=0; i < count; i++) {
	    temp[i] = elements[i]->fullName;
	}
	qsort(temp, count, sizeof(char *), stringCompare);

	total = 3;
	for (i=0; i < count; i++) total += strlen((char *) temp[i])+2;

	tempName = new(char[total]);
	strcpy(tempName, "<");
	for (i=0; i < count; i++) {
	    if (i) strcat(tempName, ",");
	    strcat(tempName, (char *) temp[i]);
	}
	strcat(tempName, ">");

	fullName = names.findAndAdd(tempName);
	delete(tempName);
	delete(temp);
    }
    unlock();
    return(fullName);
}

void printAllResources()
{
    HTable<resource*> curr;

    for (curr=  resource::allResources; *curr; ++curr) {
	printf("{");
        (*curr)->print();
	printf("}");
        printf("\n");
    }
}
