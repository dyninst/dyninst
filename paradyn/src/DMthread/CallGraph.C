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

// $Id: CallGraph.C,v 1.17 2003/10/22 17:57:01 pcroth Exp $

#include "CallGraph.h"
#include "DMdaemon.h"
#include "pdutil/h/mdl.h"
#include "paradyn/src/pdMain/paradyn.h"
#include "common/h/Types.h" // Address

inline unsigned intHash(const int &val) {
  return val;
}

inline unsigned unsignedHash(const unsigned &val) {
  return val;
}

// constructors for static data members in call graph class....
dictionary_hash<int, CallGraph *> CallGraph::directory(intHash);
resource *CallGraph::rootResource = NULL;
int CallGraph::last_id_issued = 0;

// simple function to convert from resource * to unsigned for use w/
//  hashing....
static inline unsigned hash_dummy(resource * const &r) {
    const resource *p = r;
    unsigned u = (unsigned)(Address)p;
    return u;
}

//Returns a pointer to the 6th character of string,
//which is useful for when we want to print out the name
//of a resource minus the standard "/Code/" prefix from
//the code hierarchy.
//
//Yup- this is one ugly function. 
static char *stripCodeFromName(const char *c){
  char *newc = const_cast<char *>(c);
  newc+=sizeof(char)*6;
  return newc;
}

static pdstring stripPathFromExecutableName(pdstring exe_name){
  unsigned i;
  const char *array = exe_name.c_str();
  char *last_slash = const_cast<char *>(array);
  for(i = 0; i < exe_name.length(); i++){
    if(array[i] == '/' || array[i] == '\\')
      last_slash = const_cast<char *>(&array[i]);
  }
  last_slash += sizeof(char);
  return pdstring(last_slash);
}

int CallGraph::name2id(pdstring exe_name){
  unsigned u;
  for(u = 0; u < directory.size(); u++){
    if(directory[u]->getExeAndPathName() == exe_name)
      return directory[u]->getId();
  }
  return -1;
}

CallGraph::CallGraph(pdstring exe_name) : 
    children(hash_dummy), parents(hash_dummy), visited(hash_dummy),
    instrumented_call_sites(hash_dummy), tid_to_start_func(unsignedHash),
    entryFunction(NULL), executableAndPathName(exe_name)
{
  resourceHandle h;
  program_id = last_id_issued;
  last_id_issued++;
  callGraphInitialized = false;
  callGraphDisplayed = false;
  if(!resource::string_to_handle("/Code", &h)){
    cerr << "Problem converting /Code to resource\n";
    assert(0);
  }
  rootResource = resource::handle_to_resource(h);
  assert(rootResource != NULL);
  executableName =stripPathFromExecutableName(executableAndPathName);
  // all call graph should be aware of the root resource, so that they 
  //  know to magnify from it to the entryFunction (specified in 
  //  SetEntryFunc)....
  this->AddResource(rootResource);
  dynamic_call_sites.resize(0);
}

CallGraph::CallGraph(pdstring exe_name, 
		     resource *nroot) : 
  children(hash_dummy), parents(hash_dummy), visited(hash_dummy), 
  instrumented_call_sites(hash_dummy), tid_to_start_func(unsignedHash),
  entryFunction(NULL), executableAndPathName(exe_name)
{
  program_id = last_id_issued;
  last_id_issued++;
  rootResource = nroot;
  callGraphInitialized = false;
  callGraphDisplayed = false;
  executableName =stripPathFromExecutableName(executableAndPathName);
  // all call graphs should be aware of the root resource, so that they 
  //  know to magnify from it to the entryFunction (specified in 
  //  SetEntryFunc)....
  this->AddResource(rootResource);
  dynamic_call_sites.resize(0);
}

CallGraph::~CallGraph() {
  // DO NOT DELETE RESOURCES....
}

void CallGraph::AddProgram(pdstring exe_name){
   CallGraph *cg;
   if(name2id(exe_name) == -1) {
      cg = new CallGraph(exe_name);
      directory[cg->getId()] = cg;
   }
}
void CallGraph::CallGraphFillDone(){
  callGraphInitialized = true;
}
bool CallGraph::AddResource(resource *r) {
    pdvector <resource *> empty;

    // make sure that resource refers to function....
    assert(r == rootResource || r->getMDLType() == MDL_T_PROCEDURE);

    if (children.defines(r) || parents.defines(r)) {
        return false;
    }
    children[r] = empty;
    parents[r] = empty;
    
    return true;
}


int CallGraph::SetChildren(resource *r, const pdvector <resource *>rchildren) {
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

int CallGraph::AddChild(resource *parent, resource *child) {
    // assert (p previously seen by call graph) 
    assert(children.defines(parent) && parents.defines(parent));
    AddResource(child);
    children[parent] += child;
    parents[child] += parent;
    return 1;
}

bool CallGraph::AddDynamicallyDeterminedChild(resource *p, resource *c) {
  // dynamically determined children not yet supported....
  assert(p != NULL && c != NULL);
  assert(children.defines(p) && parents.defines(p));
  assert(callGraphInitialized);
  
  unsigned u;
  bool already_added = false;
  //Todo: need to add code here in order to determine whether or not
  //a dynamic function is recursive
  //  cerr << "****Dynamic child added in Front-end CallGraph! Parent = " << 
  //p->getName() << " child = " << c->getName() << endl;
  
  //Determine whether or not this dynamic child has already been added 
  //to the call graph.
  for(u = 0; u < children[p].size(); u++)
    if(children[p][u] == c){
      already_added = true;
      break;
    }
  
  if(!already_added){
    AddResource(c);
    children[p] += c;
    parents[c] += p;

    //If the new child is the descendent of an uninstrumentable function, 
    //then it's parent probably isn't in the call graph display, so we 
    //avoid adding the child to the display
    dictionary_hash <resource *, int> have_visited(hash_dummy);
    if(!isDescendent(p, rootResource, have_visited))
      return false;
    
    //TODO!!!: need to determine whether or not the dynamic function is
    // a recursive call.
    if(callGraphDisplayed){
      if(visited[c]){
	uiMgr->CGaddNode(program_id, p->getHandle(), 
			 c->getHandle(),c->getName(),
			 stripCodeFromName(c->getFullName()),
			 false, //function is not recursive
			 true); //function is a shadow
      }
      else{
	uiMgr->CGaddNode(program_id, p->getHandle(), 
			 c->getHandle(),c->getName(),
			 stripCodeFromName(c->getFullName()),
			 false, //function is not recursive
			 false); //function is not a shadow
	dictionary_hash <resource *, int> callPath(hash_dummy); 
	//callPath is used to avoid cycles in CG
	callPath.clear();
	callPath[c] = 1;
	addChildrenToDisplay(c,callPath); 
      }
      uiMgr->CGDoneAddingNodesForNow(program_id);
    }
  }
  return true;
}

void CallGraph::AddDynamicCallSite(resource *parent){
  assert(children.defines(parent) && parents.defines(parent));
  dynamic_call_sites+= parent;
}

void CallGraph::SetEntryFunc(resource *r, unsigned tid) {
    assert(r != NULL);
    assert(r->getMDLType() == MDL_T_PROCEDURE);
    
    if(entryFunction == NULL)
       entryFunction = r;
    // The first request to magnify results in a refinement from /Code.
    // However, /Code is not a function, and is thus not known to the call
    //  graph.  Instead, the request to magnify /Code should return a single
    //  function - the entry function....
    AddResource(r);

    if(! isStartFunction(r->getHandle()))  // add if it's not already there
       AddChild(rootResource, r);

    if(tid_to_start_func.defines(tid)) {
       assert(tid_to_start_func[tid] = r->getHandle());
    }
    tid_to_start_func[tid] = r->getHandle();
}

bool CallGraph::isStartFunction(resourceHandle rh) {
   pdvector<resourceHandle> *entryFuncs = getChildren(rootResource);
   for(unsigned i=0; i<entryFuncs->size(); i++) {
      if((*entryFuncs)[i] == rh) return true;
   }
   delete entryFuncs;
   return false;
}

CallGraph *CallGraph::FindCallGraph(pdstring exe_name) {
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
   static bool warned_about_multiple_cg = false;
   if(directory.size() > 1) {
      if(! warned_about_multiple_cg) {
         cerr << "Warning: search of multiple programs with different call"
              << "graphs" << endl;
         cerr << "is not fully supported yet. Attempting to continue." << endl;
         warned_about_multiple_cg = true;
      }
   }
   return directory[0];
}

void CallGraph::displayCallGraph(){

  dictionary_hash <resource *, int> callPath(hash_dummy); 
  //callPath is used to avoid cycles in CG
  callPath.clear();
  callGraphDisplayed = true;
  //add program function to display, which is probably 
  //rooted by the function "main"

  uiMgr->callGraphProgramAddedCB(0, entryFunction->getHandle(), 
				 executableName.c_str(),
				 entryFunction->getName(),
			     stripCodeFromName(entryFunction->getFullName())); 
  
  callPath[entryFunction] = 1;
  addChildrenToDisplay(entryFunction,callPath);
  uiMgr->CGDoneAddingNodesForNow(0);
}

//Add Displays for all of the CallGraphs known to the DM
void CallGraph::displayAllCallGraphs(){
  unsigned u;
  pdvector<CallGraph*> cgs = directory.values();
  for(u = 0; u < directory.size(); u++)
    cgs[u]->displayCallGraph();
}

//This function adds all of the children of resource specified by parent
//to the callGraphDisplay. It then recursively adds all of the children
//of parent to the callGraphDisplay, and their children, ...
void CallGraph::addChildrenToDisplay(resource *parent,
				     dictionary_hash <resource *, int> 
				     callPath){
  
  unsigned i;
  const pdvector<resource *> &these_children = children[parent];
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

pdvector <resourceHandle> *CallGraph::getChildren(resource *rh) {
    unsigned i;
    pdvector <resourceHandle> *ret;
    // rh should have been registsred w/ call graph....
    assert(children.defines(rh));

    // convert children[rh] from pdvector of resources to pdvector of 
    //  resource handles....
    ret = new pdvector<resourceHandle>;
    assert(ret);

    for (i=0;i<children[rh].size();i++) {
      (*ret) += (children[rh])[i]->getHandle();
    }

    for(i = 0; i < dynamic_call_sites.size(); i++)
      if(dynamic_call_sites[i] == rh && !instrumented_call_sites.defines(rh)){
	instrumented_call_sites[rh] = 1;
	paradynDaemon::AllMonitorDynamicCallSites(rh->getFullName());
      }
    return ret;
}

bool CallGraph::isDescendentOfAny(resource *child, const resource *parent){
  static bool warned_about_multiple_cg = false;
  dictionary_hash <resource *, int> have_visited(hash_dummy);

  if(directory.size() > 1) {
     if(! warned_about_multiple_cg) {
        cerr << "Warning: search of multiple programs with different call"
             << "graphs" << endl;
        cerr << "is not fully supported yet. Attempting to continue." << endl;
        warned_about_multiple_cg = true;
     }
  }

  CallGraph *cg = directory[0];
  if(cg->isDescendent(child, parent,have_visited))
    return true;
  return false;
}

/*if child is a descendent of parent, return true, else false*/
bool CallGraph::isDescendent(resource *child, const resource *parent,
			     dictionary_hash <resource *, int>& have_visited) {
  if(child == parent)
    return true;
  else {
    unsigned i;
    pdvector<resource *>my_parents = parents[child];
    for(i = 0; i < my_parents.size(); i++){
      if(!have_visited[my_parents[i]]){
	have_visited[my_parents[i]] = 1;
	if(isDescendent(my_parents[i], parent, have_visited)){
	  return true;
	}
      }
    }
  }
  return false;
}

//Adds monitoring instrumentation to all functions that contain dynamic
//call sites.
void CallGraph::determineDynamicCallees(){
  unsigned i;
  for(i =0; i < dynamic_call_sites.size(); i++)
    paradynDaemon::AllMonitorDynamicCallSites(dynamic_call_sites[i]->getFullName());
}

void CallGraph::determineAllDynamicCallees(){
  CallGraph *cg = directory[0];
  //If there were more than one call graph in the directory, 
  //we'd stick a loop in here.
  cg->determineDynamicCallees();
}
