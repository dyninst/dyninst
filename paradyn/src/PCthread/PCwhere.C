
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
 * Revision 1.17  1995/06/02 20:50:18  newhall
 * made code compatable with new DM interface
 *
 * Revision 1.16  1995/03/03  18:12:16  krisna
 * the _correct_ prototype for strCompare
 *
 * Revision 1.15  1995/02/16  08:19:25  markc
 * Changed Boolean to bool
 *
 * Revision 1.14  1994/12/21  00:46:36  tamches
 * Minor changes that reduced the number of compiler warnings; e.g.
 * Boolean to bool.  operator<< routines now return their ostream
 * argument properly.
 *
 * Revision 1.13  1994/10/25  22:08:18  hollings
 * changed print member functions to ostream operators.
 *
 * Fixed lots of small issues related to the cost model for the
 * Cost Model paper.
 *
 * Revision 1.12  1994/09/22  01:08:48  markc
 * Changed arg types on strCompare to remove compiler warnings
 *
 * Revision 1.11  1994/08/05  16:04:18  hollings
 * more consistant use of stringHandle vs. char *.
 *
 * Revision 1.10  1994/07/28  22:34:06  krisna
 * proper starting code for PCmain thread
 * stringCompare matches qsort prototype
 * changed infinity() to HUGE_VAL
 *
 * Revision 1.9  1994/07/25  04:47:12  hollings
 * Added histogram to PCmetric so we only use data for minimum interval
 * that all metrics for a current batch of requests has been enabled.
 *
 * added hypothsis to deal with the procedure level data correctly in
 * CPU bound programs.
 *
 * changed inst hypothesis to use observed cost metric not old procedure
 * call based one.
 *
 * Revision 1.8  1994/06/29  02:56:28  hollings
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
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "../DMthread/DMresource.h"
#include "PCwhere.h"
#include "PCglobals.h"

/*
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
*/
focus::focus(resourceList *val)
{
    resource *r;
    int i, limit;

    suppress = false;
    limit = val->getCount();
    resourceHandle r_handle;
    for (i=0; i < limit; i++) {
	if(val->getNth(i,&r_handle)){
	    r = resource::handle_to_resource(r_handle);
	    if (r->getSuppress()) suppress = true;
	}
    }

    data = val;
    name = NULL;
    updateName();
}

int strCompare(const char * const *a, const char * const *b)
{
     return(strcmp(*a, *b));
}

static int stringCompare(const void* p1, const void* p2) {
  return(strCompare((const char* const *) p1, (const char* const *) p2));
}

void focus::updateName()
{
    int i;
    int limit;
    resource *res;
    char temp[256];
    stringHandle *names;

    strcpy(temp, "<");
    if (data) {
	limit = data->getCount();
	names = (stringHandle *) malloc(sizeof(stringHandle) * limit);
	suppress = false;
	for (i=0; i < limit; i++) {
	    resourceHandle r_handle;
	    if(data->getNth(i,&r_handle)){
                res = resource::handle_to_resource(r_handle);
		// TODO check that this conversion is okay
	        names[i] = (void *)res->getFullName();
	        if (res->getSuppress()) suppress = true;
	    }
	}
	/* make sure we get a canocial form by sorting the name */
	qsort((char *) names, limit, sizeof(char *), stringCompare);
	for (i=0; i < limit; i++) {
	    strcat(temp, (char *) names[i]);
	    if (i != limit - 1) strcat(temp, ",");
	}
	free(names);
    }
    strcat(temp, ">");
    name = strSpace.findAndAdd(temp);
}

/*
focus::focus()
{
    data = new resourceList;
    name = NULL;
}
*/

bool focus::operator ==(focus *f)
{
    if (name == f->name)
	return(true);
    else
	return(false);
}

ostream& operator <<(ostream &os, focus& f)
{
    if (f.data)
	os << ((char *) f.name);
    else
	os << "<>";
    return os; // added AT 12/8/94
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
    for ((void) (curr=globalFocus); *curr; ++curr) {
       cout << *(*curr) <<  endl;
    }
}

/* ARGSUSED */
focusList focus::magnify(resource *param)
{
    // see if we have done this one before.
    focusList *result = findMagnifyCache(param);
    if (result) return(*result);
    result = new(focusList);

    bool conflicts;
    focus *parent = moreSpecific(param, conflicts);
    // our current focus is more specific.
    unsigned count = 0;
    if (!parent) {
        if (!conflicts) {
	    result->add(this, (void *) this);
	    count = 1;
        }
        // do nothing if conflicts = true
    } else {
	// for each known focus cell.
	resourceHandle r_handle;
	vector<resourceHandle> *c_handles = param->getChildren();
	for (count = 0; count < c_handles->size(); count++) {
	    resource *fc = resource::handle_to_resource((*c_handles)[count]);
	    if (fc) {
	        string *name = new string;
		vector<resourceHandle> *handles = new vector<resourceHandle>;
	        for (unsigned inner=0; inner < data->getCount(); inner++) {
		    if(data->getNth(inner,&r_handle)){
		        resource *tc = resource::handle_to_resource(r_handle); 
		        if(tc->isDescendent(fc->getHandle())){
                            *name += fc->getName();
			    *handles += (*c_handles)[count];
			    if(inner < (data->getCount() - 1))
			        *name += ",";
		        }
		        else{
			    *name += tc->getName();
			    *handles += r_handle; 
			    if(inner < (data->getCount() - 1))
			        *name += ",";
                        }
		    }
	        }
	        // create a new resource List if one doesn't already exist
		resourceList *resList;
	        if(!(resList = resourceList::findRL(name->string_of()))){
	            resList = resourceList::getFocus(
			       resourceList::getResourceList(*handles));
	        }

	        // generate a new name for it.
	        focus *curr = new focus(resList);
	        focus *ret = allFoci.find(curr);
	        if (!ret) {
		    allFoci.addUnique(curr);
		    result->add(curr, (void *) curr);
	        } else {
		    // add to resulting focus.
		    result->add(ret, (void *) ret);
		    free(curr->data);
		    delete(curr);
	        }
	        delete name;
		delete handles;
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
    focusList returnList;
    resourceHandle r_handle;
    resourceHandle next_rh;
    for(unsigned i=0; i < data->getCount(); i++){
        if(data->getNth(i, &r_handle)){
	    resource *res = resource::handle_to_resource(r_handle); 
            vector<resourceHandle> *c_handles = res->getChildren();

	    for(unsigned j=0; j < c_handles->size(); j++){
	        string *name = new string;
		vector<resourceHandle> *handles = new vector<resourceHandle>;
                for(unsigned k=0; k < data->getCount(); k++){
		    resource *next_res = NULL;
                    if(k==i){
                       next_res = 
			    resource::handle_to_resource((*c_handles)[j]); 
		    }
		    else {
                       if(data->getNth(k,&next_rh)){
			   next_res = resource::handle_to_resource(next_rh);
		       } 
		    }
		    if(next_res){
                        *name += next_res->getName();
			*handles += next_res->getHandle();
			if(k < (data->getCount() - 1))
			    *name += ",";
		    }
		}
		// find this resourceList or create new one
		resourceList *destList = NULL;
		if(!(destList = resourceList::findRL(name->string_of()))){ 
		    destList = resourceList::getFocus(
				    resourceList::getResourceList(*handles));
		}
	        focus *f = new focus(destList);    
	        // generate a new name for it.
		f->updateName();
		focus *realFocus = allFoci.find(f);
		if (!realFocus) {
                    allFoci.addUnique(f);
		    realFocus = f;
		}
		else{
		    // already been defined
                    delete(f);
		}
		returnList.addUnique(realFocus);
	        delete name;
		delete handles;
	    }
	}
    }
    return(returnList);
}

/* ARGSUSED */
focus *focus::constrain(resource *param)
{
    // see if we have done this one before.
    focus *ret = findConstrainCache(param);
    if (ret) return(ret);

    bool found = false;
    ret = NULL;
    resourceHandle r_handle;
    string *name = new string;
    vector<resourceHandle> *handles = new vector<resourceHandle>;
    for (unsigned i=0; i < data->getCount(); i++) {
        if (data->getNth(i,&r_handle)) {
            resource *curr = resource::handle_to_resource(r_handle);
	    if(curr){
                if(curr->isDescendent(param->getHandle())){
                    *name += param->getName();
		    *handles += param->getHandle();
                    found = true;
		}
		else {
                    *name += curr->getName();
		    *handles += curr->getHandle();
		}
		if(i < data->getCount())
		    *name += ",";
	    }
	}
    }
    if (found) {
        // check to see if a resourceList with this name already exists
        resourceList *newCellList;
        if(!(newCellList = resourceList::findRL(name->string_of()))){
	    newCellList = resourceList::getFocus(
				resourceList::getResourceList(*handles));
        }
	focus *nf = new focus(newCellList);
	ret = allFoci.find(nf);
	addConstrainCache(param, ret);
	delete(nf);
	delete(name);
	return(ret);
    } else {
	addConstrainCache(param, this);
	return(this);
	delete(name);
    }
}

//
// given a focus and a resource, see if the resource is more specific 
//   (more magnified) than the corresponding resource in the passed focus.
// If it is return the more specific focus, otherwise return null.
//
focus *focus::moreSpecific(resource *parm, bool &conflicts)
{
    assert(data);
    conflicts = false;
    bool found = false;
    resourceHandle r_handle;
    string *name = new string;
    vector<resourceHandle> *handles = new vector<resourceHandle>;
    for (unsigned i=0; i < data->getCount(); i++) {
	if(data->getNth(i,&r_handle)){
	    resource *curr = resource::handle_to_resource(r_handle);
	    if(curr){
                if(curr->isDescendent(parm->getHandle())){
		    *name += parm->getName();
		    *handles += parm->getHandle();
		    if(i < data->getCount())  
			*name += ",";
		    found = true;
		}
		else if(parm->isDescendent(curr->getHandle())){
		// the current focus is more specific - it will be reused
		found = false;
		break;
	        } else if (strcmp((char *) curr->getName(), 
		           (char *) parm->getName()) && 
		           (curr->sameRoot(parm->getHandle()))) {
	            // if the resource and param have the same root, but
	            // one is not a descendant of the other,
	            // and they are not the same, then this
	            // magnification conflicts with the current focus
	            conflicts = true;
	            found = false;
	            break;
	        } else {
		    *name += curr->getName();
		    *handles += curr->getHandle();
		    if(i < data->getCount())  
			*name += ",";
                }
	    }    
	}
    }
    if (found) {
	// check to see if a resourceList with this name already exists
	resourceList *newList;
	if(!(newList = resourceList::findRL(name->string_of()))){
	    newList = resourceList::getFocus(
				resourceList::getResourceList(*handles)); 
	}
	focus *nf = new focus(newList);
	focus *ret = allFoci.find(nf);
	if (ret) {
	    delete(nf);
	    delete name;
	    delete handles;
	    return(ret);
	} else {
	    allFoci.addUnique(nf);
	    delete name;
	    delete handles;
	    return(nf);
	}
    } else {
	// not more specific.
	delete name;
	delete handles;
	return(NULL);
    }
}

void initResources()
{
    resourceHandle *h = dataMgr->getRootResource();
    if(!h) return;

    resourceHandle handle = *h; 
    resourceHandle temp = dataMgr->newResource(handle, "SyncObject");
    SyncObject = resource::handle_to_resource(temp);
    temp = dataMgr->newResource(handle, "Procedure");
    Procedures = resource::handle_to_resource(temp);
    temp = dataMgr->newResource(handle, "Process");
    Processes = resource::handle_to_resource(temp);
    temp = dataMgr->newResource(handle, "Machine");
    Machines = resource::handle_to_resource(temp);

    handle = SyncObject->getHandle(); 
    temp = dataMgr->newResource(handle,"SpinLock");
    Locks = resource::handle_to_resource(temp);
    temp = dataMgr->newResource(handle, "Barrier");
    Barriers = resource::handle_to_resource(temp);
    temp = dataMgr->newResource(handle, "Semaphore");
    Semaphores = resource::handle_to_resource(temp);
    temp = dataMgr->newResource(handle,"MsgTag");
    MsgTags = resource::handle_to_resource(temp);

    // whereAxis = new focus(dataMgr->getRootResources());
    vector<resourceHandle> *children = resource::rootResource->getChildren();
    // get handle for existing rl or creates a new one
    resourceListHandle rl_handle = resourceList::getResourceList(*children);
    resourceList *rl = resourceList::getFocus(rl_handle);
    whereAxis = new focus(rl);

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
