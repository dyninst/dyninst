/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
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

// $Id: pd_module.C,v


#include "paradynd/src/pd_module.h"
#include "paradynd/src/pd_image.h"
#include "paradynd/src/resource.h"
#include "paradynd/src/comm.h"
#include "dyninstAPI/src/symtab.h" // for ResourceFullName()
#include "dyninstAPI/src/process.h" // for isDynamicCallSite()
#include "dyninstAPI/h/BPatch_flowGraph.h"

extern bool module_is_excluded(BPatch_module *m);
extern bool function_is_excluded(BPatch_function *f, pdstring modname);
extern bool should_report_loops;
extern pdRPC *tp;

dictionary_hash<BPatch_function *, resource *> *pd_module::func_resources;
dictionary_hash<BPatch_basicBlockLoop *, resource *> *pd_module::loop_resources;

//Stupid hash functions that just hash on the ptr value
typedef BPatch_function *bp_func_ptr_t;
static unsigned func_hash(const bp_func_ptr_t &func)
{
   return ((unsigned) func >> 1);
}

//Stupid hash functions that just hash on the ptr value
typedef BPatch_basicBlockLoop *bp_loop_ptr_t;
static unsigned loop_hash(const bp_loop_ptr_t &loop)
{
   return ((unsigned) loop >> 1);
}

#define NAME_LEN 512
pd_module::pd_module(BPatch_module *dmod) : dyn_module(dmod) 
{ 
  char buf[NAME_LEN];
  dmod->getName(buf, NAME_LEN);
  _name = pdstring(buf);

  is_excluded = module_is_excluded(dmod);

  //  need to get included functions filled in here
  all_funcs = dmod->getProcedures();
  if (!all_funcs || !all_funcs->size()) {
    // ok to just get rid of printout if this happens a lot:
     //fprintf(stderr, "%s[%d]: WARNING:  module %s has no functions\n", 
     //      __FILE__, __LINE__, buf);
    return;
  }

  for (unsigned int i = 0; i < all_funcs->size(); ++i) {
    BPatch_function *f = (*all_funcs)[i];
    if (!function_is_excluded(f, _name))
      some_funcs.push_back(f);

  }

  // Paradyn uses names with type strings as primary


  createResources();
  //fprintf(stderr, "%s[%d]:  new pd_module %s: %s, contains %d/%d funcs\n", 
  //        __FILE__, __LINE__,
  //        buf, is_excluded ? "excluded" : "included", all_funcs->size(),
  //        some_funcs.size());
}

pd_module::~pd_module()
{
}

// send message to data manager to specify the entry function for the
//  call graph corresponding to a given image.  r should hold the 
//  FULL resourcename of the entry function (e.g. "/Code/module.c/main")
void CallGraphSetEntryFuncCallback(pdstring exe_name, pdstring r, int tid) {
    tp->CallGraphSetEntryFuncCallback(exe_name, r, tid);
}

void CallGraphAddProgramCallback(pdstring name) {
   tp->CallGraphAddProgramCallback(name);
}

//send message to the data manager, notifying it that all of the statically
//determinable functions have been registered with the call graph. The
//data manager will then be able to create the call graph.
void CallGraphFillDone(pdstring exe_name) {
   tp->CallGraphFillDone(exe_name);
}

//send message to the data manager in order to register a function 
//in the call graph.
void AddCallGraphNodeCallback(pdstring exe_name, pdstring r) {
   tp->AddCallGraphNodeCallback(exe_name, r);
}

//send a message to the data manager in order register a the function
//calls made by a function (whose name is stored in r).
void AddCallGraphStaticChildrenCallback(pdstring exe_name, pdstring r,
                                        const pdvector<pdstring> children) 
{
   tp->AddCallGraphStaticChildrenCallback(exe_name, r, children);
}


// Add nested loops and functions to the call graph
void pd_module::FillInCallGraphNodeNested(pdstring exe_name, 
                                          pdstring func_name,
                                          pdstring parent_name,
                                          BPatch_loopTreeNode *node,
                                          BPatch_process *proc)
{
   // register the name for the parent
   AddCallGraphNodeCallback(exe_name, parent_name);
   
   // collect children nested under this node
   pdvector<pdstring> children;
   
   // add callees
   BPatch_Vector<BPatch_function *> callees;
   node->getCallees(callees, proc);
   for (unsigned i = 0; i < callees.size(); i++) {
      BPatch_function *f = callees[i];
      resource *res = getFunctionResource(f);

      if (res)
         children.push_back(res->full_name());
      
   }
   
   // add nested loops
   for (unsigned j = 0; j < node->children.size(); j++) {
      // loop resource names are not nested, all loop resource names are
      // of the following form /Code/Module/Function/Loop
      children.push_back(func_name+"/"+node->children[j]->name());
   }
   
   // register children for this parent
   AddCallGraphStaticChildrenCallback(exe_name, parent_name, children);
   
   // recurse with each child
   for (unsigned k = 0; k < node->children.size(); k++) {
      // we register resources while traversing the tree of nodes in a dfs
      // so that the call graph is properly nested, while ensuring that the
      // the resource names are correct (loop resource names are not nested)
      FillInCallGraphNodeNested(exe_name, func_name,
                                func_name+"/"+node->children[k]->name(),
                                node->children[k], proc);
   }
}


// Called across all modules (in a given image) to define the call
//  graph relationship between functions.
// Must be called AFTER all functions in all modules (in image) are
//  registered as resource (e.g. w/ pdmodule::define())....
void pd_module::FillInCallGraphStatic(pd_process *proc) 
{
   const BPatch_Vector<BPatch_function *> *mod_funcs = 
      get_dyn_module()->getProcedures();
  
   // for each INSTRUMENTABLE function in the module (including excluded 
   //  functions, but NOT uninstrumentable ones)....
   for(unsigned f=0; f<(*mod_funcs).size(); f++) {
      BPatch_function *bpf = (*mod_funcs)[f];
      resource *res = getFunctionResource(bpf);
      if (!res) 
          continue;

      // register that callee_resources holds list of resource*s 
      //  describing children of resource r....
      pdstring exe_name = proc->getImage()->get_file();

      pdstring resource_full_name = res->full_name();
      BPatch_loopTreeNode *root = bpf->getCFG()->getLoopTree();
      FillInCallGraphNodeNested(exe_name, resource_full_name,
                                resource_full_name, root, 
                                proc->get_dyn_process());

      //Locate the dynamic call sites within the function, and notify 
      //the front end as to their existence
      if (bpf->getCFG()->containsDynamicCallsites())
         tp->CallGraphAddDynamicCallSiteCallback(exe_name, resource_full_name);
   }
}

void pd_module::createResources()
{
   char name[NAME_LEN];
   char typedName[NAME_LEN];
   mod_resource = resource::newResource(moduleRoot, this,
                                        nullString,
                                        _name,
                                        timeStamp::ts1970(),
                                        pdstring(),
                                        ModuleResourceType,
                                        MDL_T_MODULE,
                                        false);

   if (!func_resources)
      func_resources = 
         new dictionary_hash<BPatch_function*, resource*>(func_hash);
   if (!loop_resources)
      loop_resources = 
         new dictionary_hash<BPatch_basicBlockLoop*, resource*>(loop_hash);

   //Create resources for each function
   for (unsigned i=0; i<all_funcs->size(); i++)
   {
      BPatch_function *func = (*all_funcs)[i];
      if (!(*all_funcs)[i]->isInstrumentable())
         continue;
      
      func->getName(name, NAME_LEN);

      // If there are multiple matches for this name, grab the typed name...
      BPatch_Vector<BPatch_function *> name_check_vec;
      dyn_module->findFunction(name, name_check_vec, false, false, false, true);
      if (name_check_vec.size() > 1) {
          func->getTypedName(typedName, NAME_LEN);
          if (strlen(typedName))
              strncpy(name, typedName, NAME_LEN);
      }

      resource *res = resource::newResource(mod_resource, func,
                                            nullString,
                                            name,
                                            timeStamp::ts1970(),
                                            nullString,
                                            FunctionResourceType,
                                            MDL_T_PROCEDURE,
                                            false);
      (*func_resources)[func] = res;
      
      //Create resources for each loop
      BPatch_flowGraph *cfg = func->getCFG();
      BPatch_loopTreeNode *outer_loop = cfg->getLoopTree();
      createLoopResources(outer_loop, res);
   }

   resource::send_now();
}

void pd_module::createLoopResources(BPatch_loopTreeNode *loop_tree, 
                                    resource *parent)
{
   resource *res = parent;
   if (loop_tree->loop)
   {
      res = resource::newResource(parent, loop_tree->loop,
                                  nullString,
                                  pdstring(loop_tree->name()),
                                  timeStamp::ts1970(),
                                  nullString,
                                  LoopResourceType,
                                  MDL_T_LOOP,
                                  false);
      (*loop_resources)[loop_tree->loop] = res;
   }

   for (unsigned i=0; i<loop_tree->children.size(); i++)
   {
      // loop resource objects are nested under their parent function rather
      // than each other. using 'res' instead of 'parent' would cause 
      // the resource hierarchy to have loops nested under each other.
      createLoopResources(loop_tree->children[i], parent);
   }
}

resource *pd_module::getFunctionResource(BPatch_function *f)
{
   if (!func_resources->defines(f))
      return NULL;
   return (*func_resources)[f];

}

resource *pd_module::getLoopResource(BPatch_basicBlockLoop *l)
{
   if (!loop_resources->defines(l))
      return NULL;
   return (*loop_resources)[l];
}
