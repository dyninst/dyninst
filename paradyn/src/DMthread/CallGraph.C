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

// $Id: CallGraph.C,v 1.3 1999/07/26 21:47:33 cain Exp $

#include "CallGraph.h"
#include "paradyn/src/met/mdl.h"
#include "dyninstAPI/src/util.h"
#include "paradyn/src/pdMain/paradyn.h"
// constructors for static data members in call graph class....
dictionary_hash<int, CallGraph *> CallGraph::directory(intHash);
resource *CallGraph::rootResource = NULL;
int CallGraph::last_id_issued = 0;

// simple function to convert from resource * to unsigned for use w/
//  hashing....
static inline unsigned hash_dummy(resource * const &r) {
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

static string stripPathFromExecutableName(string exe_name){
  unsigned i;
  const char *array = exe_name.string_of();
  char *last_slash = const_cast<char *>(array);
  for(i = 0; i < exe_name.length(); i++){
    if(array[i] == '/' || array[i] == '\\')
      last_slash = const_cast<char *>(&array[i]);
  }
  last_slash += sizeof(char);
  return string(last_slash);
}

int CallGraph::name2id(string exe_name){
  unsigned u;
  for(u = 0; u < directory.size(); u++){
    if(directory[u]->getExeAndPathName() == exe_name)
      return directory[u]->getId();
  }
  return -1;
}

CallGraph::CallGraph(string exe_name) : 
    children(hash_dummy), parents(hash_dummy), visited(hash_dummy),
    executableAndPathName(exe_name)
{
  resourceHandle h;
  program_id = last_id_issued;
  last_id_issued++;
  callGraphInitialized = false;
  assert(resource::string_to_handle("/Code", &h));
  rootResource = resource::handle_to_resource(h);
  assert(rootResource != NULL);
  executableName =stripPathFromExecutableName(executableAndPathName);
  // all call graph should be aware of the root resource, so that they 
  //  know to magnify from it to the entryFunction (specified in 
  //  SetEntryFunc)....
  this->AddResource(rootResource);
}

CallGraph::CallGraph(string exe_name, 
		     resource *nroot) : 
  children(hash_dummy), parents(hash_dummy), visited(hash_dummy),
  executableAndPathName(exe_name)
{
  program_id = last_id_issued;
  last_id_issued++;
  rootResource = nroot;
  callGraphInitialized = false;
  executableName =stripPathFromExecutableName(executableAndPathName);
  // all call graph should be aware of the root resource, so that they 
  //  know to magnify from it to the entryFunction (specified in 
  //  SetEntryFunc)....
  this->AddResource(rootResource); 
}

CallGraph::~CallGraph() {
  // DO NOT DELETE RESOURCES....
}

void CallGraph::AddProgram(string exe_name){
  CallGraph *cg;
  if(name2id(exe_name) == -1){
    cg = new CallGraph(exe_name);
    directory[cg->getId()] = cg;
  }
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
    unsigned u,i;
    resource *rchild;
    bool already_added;
    // assert(r previously seen by call graph)....
    assert(children.defines(r) && parents.defines(r));

    for(u=0;u<rchildren.size();u++) {
      rchild = rchildren[u];
      already_added = false;
      for(i=0; i < children[r].size(); i++)
	if(children[r][i] == rchild){
	  already_added = true;
	  break;
	}
      if(!already_added){
	AddResource(rchild);
	children[r] += rchild;
	parents[rchild] += r;
      }
    }

    return (int)u;
}

int CallGraph::SetChild(resource *p, resource *c) {
    // assert (p previously seen by call graph) 
    assert(children.defines(p) && parents.defines(p));
    // assert (p had no previously defined children)

    //Don't add duplicate children to this resource
    if(children[p].size() != 0)
      return 0;
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

CallGraph *CallGraph::FindCallGraph(string exe_name) {
  int pid = name2id(exe_name);
  if(pid == -1)
    return NULL;
  if (directory.defines(pid)) {
    return directory[pid];
  }
  //If name2id returns a valid pd, we should always be able to find a CG
  assert(false);
  return NULL;
}

CallGraph *CallGraph::FindCallGraph(){
  assert(directory.size() == 1);
  return directory[0];
}

void CallGraph::displayCallGraph(){

  dictionary_hash <resource *, int> callPath(hash_dummy); 
  //callPath is used to avoid cycles in CG
  callPath.clear();
  callGraphInitialized = true;
  //add program function to display, which is probably 
  //rooted by the function "main"

  uiMgr->callGraphProgramAddedCB(0, entryFunction->getHandle(), 
				 executableName.string_of(),
				 entryFunction->getName(),
			     stripCodeFromName(entryFunction->getFullName())); 
  
  callPath[entryFunction] = 1;
  addChildrenToDisplay(entryFunction,callPath);
  uiMgr->CGDoneAddingNodesForNow(0);
}

//Add Displays for all of the CallGraphs known to the DM
void CallGraph::displayAllCallGraphs(){
  unsigned u;
  vector<CallGraph*> cgs = directory.values();
  for(u = 0; u < directory.size(); u++)
    cgs[u]->displayCallGraph();
}

//This function adds all of the children of resource specified by r
//to the callGraphDisplay. It then recursively adds all of the children
//of r to the callGraphDisplay, and their children, ...
void CallGraph::addChildrenToDisplay(resource *parent,
				     dictionary_hash <resource *, int> 
				     callPath){
  
  unsigned i;
  const vector<resource *> &these_children = children[parent];
  visited[parent] = 1;
  
  for(i =0; i < these_children.size(); i++){
    if(!callPath[these_children[i]] && !visited[these_children[i]]){
      //For this call path, this is the first time that we have seen this
      //function. 
      callPath[these_children[i]] = 1;
      
      uiMgr->CGaddNode(program_id, parent->getHandle(), 
		       these_children[i]->getHandle(),
		       these_children[i]->getName(),
		       stripCodeFromName(these_children[i]->getFullName()),
		       false,//function is not recursive
		       false); //function is not a shadow node
      addChildrenToDisplay(these_children[i], callPath);
      callPath.undef(these_children[i]);
    }
    else if(callPath[these_children[i]] == 1){//function is recursive
      //For this call path, this is the second time that we have seen this 
      //function, meaning that this function is recursive.
      uiMgr->CGaddNode(program_id, parent->getHandle(), 
		       these_children[i]->getHandle(),
		       these_children[i]->getName(),
		       stripCodeFromName(these_children[i]->getFullName()),
		       true,//function is recursive
		       true);//function is a shadown node
    }
    else if(visited[these_children[i]]){
      uiMgr->CGaddNode(program_id, parent->getHandle(), 
		       these_children[i]->getHandle(),
		       these_children[i]->getName(),
		       stripCodeFromName(these_children[i]->getFullName()),
		       false,//function is not recursive
		       true);//function is a shadown node
    }
    else {
      //we should never get here
      assert(false);
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
















