/*
 * Copyright (c) 1996 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

// $Id: CallGraph.C,v 1.1 1999/05/24 16:55:06 cain Exp $

#include "CallGraph.h"
#include "paradyn/src/met/mdl.h"
#include "dyninstAPI/src/util.h"
#include "paradyn/src/pdMain/paradyn.h"

// constructors for static data members in call graph class....
dictionary_hash<int, CallGraph *> CallGraph::directory(intHash);
resource *CallGraph::rootResource = NULL;

// simple function to convert from resource * to unsigned for use w/
//  hashing....
static inline unsigned int hash_dummy(resource * const &r) {
    const resource *p = r;
    unsigned int u = (unsigned int)p;
    return u;
}

//Returns a pointer to the 6th character of string,
//which is useful for when we want to print out the name
//of a resource minus the standard "/Code/" prefix from
//the code hierarchy.
//
//Yup- this is one ugly function. 
static char *stripCodeFromName(const char *c){
  char *newc = (char *) c;
  newc+=sizeof(char)*6;
  return newc;
}

CallGraph::CallGraph(int program) : 
    children(hash_dummy), parents(hash_dummy)
{
  rootResource = NULL;
  program_id = program;
  callGraphInitialized = false;
}

CallGraph::CallGraph(int program, resource *nroot) : 
    children(hash_dummy), parents(hash_dummy)
{
    rootResource = nroot;
    program_id = program;
    callGraphInitialized = false;
}

CallGraph::~CallGraph() {
  // DO NOT DELETE RESOURCES....
}

bool CallGraph::AddResource(resource *r) {
    vector <resource *> empty;

    // make sure that resource refers to function....
    assert(r == rootResource || r->getType() == MDL_T_PROCEDURE);

    if (children.defines(r) || parents.defines(r)) {
        return false;
    }
    children[r] = empty;
    parents[r] = empty;
    
    return true;
}


int CallGraph::SetChildren(resource *r, const vector <resource *>rchildren) {
    unsigned u;
    resource *rchild;
    
    // assert(r previously seen by call graph)....
    assert(children.defines(r) && parents.defines(r));

    //This assert was removed in order to deal with overloaded function
    //calls. It makes sure that when we add children to a function, 
    //that function doesn't already have children, which is the common case
    //because SetChildren() is called once per function. We run into problems
    //when there are multiple functions with the same name...the call
    //Graph doesn't distinguish between them.
    //    assert(children[r].size() == 0);
    for(u=0;u<rchildren.size();u++) {
      
        rchild = rchildren[u];
        AddResource(rchild);
	children[r] += rchild;
	parents[rchild] += r;
    }

    return (int)u;
}

int CallGraph::SetChild(resource *p, resource *c) {
    // assert (p previously seen by call graph) 
    assert(children.defines(p) && parents.defines(p));
    // assert (p had no previously defined children)
    //assert(children[p].size() == 0);
    AddResource(c);
    
    children[p] += c;
    parents[c] += p;
    return 1;
}

bool CallGraph::AddDynamicallyDeterminedChild(resource *r, resource *c) {
    // dynamically determined children not yet supported....
    assert(false);
    assert(r != NULL && c != NULL);
    return false;
}


CallGraph* CallGraph::GetCallGraph(int program) {
    CallGraph *n;
    resourceHandle h;

    if (directory.defines(program)) {
        return directory[program];
    }

    // would like to throw MemFullErr if n is NULL, but paradyd doesn't
    //  define or use error hierarchies....
    n = new CallGraph(program);
    assert(n);

    directory[program] = n;
    // If rootResource is previously defined,
    //  define now (static data member)....
    // In current version, 1st call graph should be created AFTER function 
    //  resources have been defined.  Thus the assert that rootResource 
    //  is defined.  Probably safe to remove if start creating
    //  call graphs before sreources are defined in paradyn....
    if (rootResource == NULL) {
        assert(resource::string_to_handle("/Code", &h));
        rootResource = resource::handle_to_resource(h);
	assert(rootResource != NULL);
    }

    // all call graph should be aware of the root resource, so that they 
    //  know to magnify from it to the entryFunction (specified in 
    //  SetEntryFunc)....    
    n->AddResource(rootResource);

    return n;
}

void CallGraph::SetEntryFunc(resource *r) {
    assert(r != NULL);
    assert(r->getType() == MDL_T_PROCEDURE);
    
    entryFunction = r;
    // The first request to magnify results in a refinement from /Code.
    // However, /Code is not a function, and is thus not known to the call
    //  graph.  Instead, the request to magnify /Code should return a single
    //  function - the entry function....
    AddResource(entryFunction);
    

    SetChild(rootResource, entryFunction);
}

CallGraph*CallGraph::FindCallGraph(int program) {

    if (directory.defines(program)) {
        return directory[program];
    }
    return NULL;
}

void CallGraph::displayCallGraph(){

  dictionary_hash <resource *, int> callPath(hash_dummy); 
  //callPath is used to avoid cycles in CG
  
  callPath.clear();
  callGraphInitialized = true;
  //add program function to display, which is probably 
  //rooted by the function "main"
  uiMgr->callGraphProgramAddedCB(0,entryFunction->getHandle(), 
		     entryFunction->getName(),
		     stripCodeFromName(entryFunction->getFullName())); 
   
  callPath[entryFunction] = 1;
  addChildrenToDisplay(entryFunction,callPath);
  uiMgr->CGDoneAddingNodesForNow(0);
}

//This function adds all of the children of resource specified by r
//to the callGraphDisplay. It then recursively adds all of the children
//of r to the callGraphDisplay, and their children, ...
void CallGraph::addChildrenToDisplay(resource *parent,
				     dictionary_hash <resource *, int> 
				     callPath){
  
  unsigned i;
  const vector<resource *> &these_children = children[parent];

  for(i =0; i < these_children.size(); i++){
    if(!callPath[these_children[i]]){
      //For this call path, this is the first time that we have seen this
      //function. 
      callPath[these_children[i]] = 1;
      uiMgr->CGaddNode(program_id, parent->getHandle(), 
		       these_children[i]->getHandle(),
		       these_children[i]->getName(),
		       stripCodeFromName(these_children[i]->getFullName()),
		       false);//function is not recursive
      addChildrenToDisplay(these_children[i],callPath);
      callPath.undef(these_children[i]);
    }
    else if(callPath[these_children[i]] == 1){
      //For this call path, this is the second time that we have seen this 
      //function, meaning that this function is recursive.
      callPath[these_children[i]] = 2;
      uiMgr->CGaddNode(program_id, parent->getHandle(), 
		       these_children[i]->getHandle(),
		       these_children[i]->getName(),
		       stripCodeFromName(these_children[i]->getFullName()),
		       true);//function is recursive
    }       
  } 
}

vector <resourceHandle> *CallGraph::getChildren(resource *rh) {
    unsigned i;
    vector <resourceHandle> *ret;

    // rh should have been registsred w/ call graph....
    assert(children.defines(rh));

    // convert children[rh] from vector of resources to vector of 
    //  resource handles....
    ret = new vector<resourceHandle>;
    assert(ret);
    for (i=0;i<children[rh].size();i++) {
        (*ret) += (children[rh])[i]->getHandle();
    }

    return ret;
}
