/*
 *  Copyright 1993 Jeff Hollingsworth.  All rights reserved.
 *
 */

#ifndef lint
static char Copyright[] = "@(#) Copyright (c) 1993 Jeff Hollingsowrth\
    All rights reserved.";

static char rcsid[] = "@(#) $Header: /home/jaw/CVSROOT_20081103/CVSROOT/core/paradynd/src/resource.C,v 1.14 1995/02/16 08:54:13 markc Exp $";
#endif

/*
 * resource.C - handle resource creation and queries.
 *
 * $Log: resource.C,v $
 * Revision 1.14  1995/02/16 08:54:13  markc
 * Corrected error in comments -- I put a "star slash" in the comment.
 *
 * Revision 1.13  1995/02/16  08:34:43  markc
 * Changed igen interfaces to use strings/vectors rather than char igen-arrays
 * Changed igen interfaces to use bool, not Boolean.
 * Cleaned up symbol table parsing - favor properly labeled symbol table objects
 * Updated binary search for modules
 * Moved machine dependnent ptrace code to architecture specific files.
 * Moved machine dependent code out of class process.
 * Removed almost all compiler warnings.
 * Use "posix" like library to remove compiler warnings
 *
 * Revision 1.12  1994/11/06  09:53:14  jcargill
 * Fixed early paradynd startup problem; resources sent by paradyn were
 * being added incorrectly at the root level.
 *
 * Revision 1.11  1994/11/02  11:16:57  markc
 * REplaced container classes.
 *
 * Revision 1.10  1994/09/22  02:24:46  markc
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

#include "symtab.h"
#include "process.h"
#include "dyninstP.h"
#include "util.h"
#include "comm.h"
#include "util/h/String.h"
#include "util/h/Dictionary.h"
#include <strstream.h>
#include "main.h"

dictionary_hash<string, resource*> allResources(string::hash);

/*
 * handle the notification of resource creation and deletion.
 *
 */

// TODO - this is ugly - move to init
resource rootNode(true);
resource *rootResource = &rootNode;

resourceListRec *getRootResources()
{
    return(rootResource->children);
}

string getResourceName(resource *r)
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

bool addResourceList(resourceListRec *rl, resource *r)
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
    return(true);
}

bool initResourceRoot = false;

resource *newResource(resource *parent, 
		      void *handle,
		      const string abstraction, 
		      const string name,
		      timeStamp creation,
		      const string unique)
{
    int c;
    resource *ret;
    resource **curr;
    static string host;
    static bool init=false;
    // static dictionary_hash<string, bool> name_map(string::hash);

    if (!init) {
      struct utsname un;
      P_uname(&un);
      host = un.nodename;
      init=true;
    }
    assert (!(name == (char*) NULL));

    string unique_string;
    if (unique.length()) 
      unique_string = name + "{" + unique + "}";
    else 
      unique_string = name;

    string res_string;
    if (parent->info.fullName == (char*) NULL) {
      res_string = string("/") + unique_string;
    } else {
      res_string = parent->info.fullName + "/" + unique_string;
    }

    /* first check to see if the resource has already been defined */
    if (parent->children) {
      for (curr=parent->children->elements, c=0; c < parent->children->count; c++) {
	if (unique_string == curr[c]->info.name)
	  return(curr[c]);
      }
    } else {
	parent->children = new resourceListRec;
    }

    ret = new resource(false);
    ret->parent = parent;
    ret->handle = handle;
    
    ret->info.fullName = res_string;
    ret->info.name = unique_string;
    ret->info.abstraction = abstraction;
    ret->info.creation = creation;

    addResourceList(parent->children, ret);
    allResources[ret->info.fullName] = ret;

    // TODO -- use pid here
    tp->resourceInfoCallback(0, parent->info.fullName, unique_string,
			     unique_string, abstraction);
    return(ret);
}

resource *findChildResource(resource *parent, const string name)
{
    int c;
    resource **curr;

    if (!parent || !parent->children) return(NULL);
    for (curr=parent->children->elements, c=0;
	 c < parent->children->count; c++) {
	 if (curr[c]->info.name == name) {
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
        ostrstream os(errorLine, 1024, ios::out);
	os << r->info.fullName << ends;
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
        ostrstream os(errorLine, 1024, ios::out);
	os << rl->elements[i]->info.fullName << ends;
	logLine(errorLine);
	if (i!= rl->count-1) logLine(",");
    }
    logLine(">");
}

/*
 * Convinence function.
 *
 */
bool isResourceDescendent(resource *parent, resource *child)
{
    while (child) {
        if (child == parent) {
            return(true);
        } else {
            child = getResourceParent(child);
        }
    }
    return(false);
}

//
// Find this passed focus if its resource components are valid for paradynd.
//    We treat "unknown" top level resources as a specical case since the
//    meaning is that we are at the top level of refinement of an unsupported
//    resource hierarchy.  In this case, we simply create the resource, and
//    the focus is valid.  Any other resource that can't be found results in
//    an invalid focus.  jkh 1/29/94.
//
resourceListRec *findFocus(const vector<string> &data)
{
    resource *res;
    resourceListRec *rl;

    rl = createResourceList();

    for (unsigned i=0; i < data.size(); i++) {
      bool def = allResources.defines(data[i]);
      if (!def && (P_strchr(data[i].string_of(), '/') ==
		   P_strrchr(data[i].string_of(), '/'))) {
	const char *pl = data[i].string_of() + 1;
	res = newResource(rootResource, NULL, nullString, pl, 0.0, "");
      } else if (!def) {
	return(NULL);
      } else
	res = allResources[data[i]];

      addResourceList(rl, res);
    }
    return(rl);
}

resource *findResource(const string &name)
{
  // TODO does this work?   NOPE!
  if (name == (char*)NULL) {
    return(rootResource);
  } else {
    if (allResources.defines(name))
      return (allResources[name]);
    else
      return (NULL);
  }
}
