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
#include "paradynd/src/resource.h"
#include "paradynd/src/comm.h"
#include "dyninstAPI/src/symtab.h" // for ResourceFullName()
#include "dyninstAPI/src/process.h" // for isDynamicCallSite()

#include "dyninstAPI/h/BPatch_flowGraph.h"
extern bool module_is_excluded(BPatch_module *m);
extern bool function_is_excluded(BPatch_function *f, pdstring modname);
extern pdRPC *tp;

pd_module::pd_module(BPatch_module *dmod) : dyn_module(dmod) 
{ 
  char buf[512];
  dmod->getName(buf, 512);
  _name = pdstring(buf);

  is_excluded = module_is_excluded(dmod);

  //  need to get included functions filled in here
  all_funcs = dmod->getProcedures();
  if (!all_funcs || !all_funcs->size()) {
    // ok to just get rid of printout if this happens a lot:
    fprintf(stderr, "%s[%d]:  WARNING:  module %s has no functions\n", 
           __FILE__, __LINE__, buf);
    return;
  }

  for (unsigned int i = 0; i < all_funcs->size(); ++i) {
    BPatch_function *f = (*all_funcs)[i];
    if (!function_is_excluded(f, _name))
      some_funcs.push_back(f);
  }
  
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
void FillInCallGraphNodeNested(pdstring exe_name, 
			       pdstring func_name,
			       pdstring parent_name,
			       BPatch_loopTreeNode *node)
{
    // register the name for the parent
    AddCallGraphNodeCallback(exe_name, parent_name);
				
    // collect children nested under this node
    pdvector<pdstring> children;

    // add callees
    for (unsigned i = 0; i < node->callees.size(); i++) {
       pd_Function *f = static_cast<pd_Function *>(node->callees[i]);
        assert(f != NULL);

        if (f->FuncResourceSet())
            children.push_back(f->ResourceFullName());
//         else
//             fprintf(stderr,"Func %s has no resource set!\n", 
//                     f->prettyName().c_str());
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
                                  node->children[k]);
    }
}


// Called across all modules (in a given image) to define the call
//  graph relationship between functions.
// Must be called AFTER all functions in all modules (in image) are
//  registered as resource (e.g. w/ pdmodule::define())....
void pd_module::FillInCallGraphStatic(process *proc) {
   pdvector<pd_Function *> callees;
   pdvector<pdstring>        callees_as_strings;
   const BPatch_Vector<BPatch_function *> *mod_funcs = get_dyn_module()->getProcedures();
  
   // for each INSTRUMENTABLE function in the module (including excluded 
   //  functions, but NOT uninstrumentable ones)....
  
   for(unsigned f=0; f<(*mod_funcs).size(); f++) {
      pd_Function *pdf = (pd_Function *) (*mod_funcs)[f]->PDSEP_pdf();

      if (!pdf->FuncResourceSet()) {
          //fprintf(stderr,"Func resource not set for %s\n",pdf->prettyName().c_str());
          continue;
      }
      
      callees_as_strings.resize(0);
    
      // Translate from function name to resource *.
      // Note that this probably is NOT the correct translation, as 
      //  function names are not necessarily unique, but the paradyn
      //  code assumes that they are in several places (e.g. 
      //  resource::findResource)....
      pdstring resource_full_name = pdf->ResourceFullName();
      resource *r = resource::findResource(resource_full_name);
      // functions registered under pretty name....
      assert(r != NULL);
    
      // get list of statically determined call destinations from pdf,
      //  using the process info to help fill in calls througb PLT
      //  entries....
      pdf->getStaticCallees(proc, callees); 
    
      // and convert them into a list of resources....
      for(unsigned g=0; g<callees.size(); g++) {
         pd_Function *callee = callees[g];
         assert(callee);
      
         //if the funcResource is not set, then the function must be
         //uninstrumentable, so we don't want to notify the front end
         //of its existence
         if (callee->FuncResourceSet()){
            pdstring callee_full_name = callee->ResourceFullName();
	
            // if callee->funcResource has been set, then it should have 
            //  been registered as a resource.... 
            resource *callee_as_resource = 
               resource::findResource(callee_full_name);
            assert(callee_as_resource);
            callees_as_strings.push_back(callee_full_name);
         }
         else {
             // fprintf(stderr,"CALLEE no res %s\n", callee->().c_str());

         }
      
      }//end for
    
      // register that callee_resources holds list of resource*s 
      //  describing children of resource r....
      BPatch_image *appImage = (BPatch_image* ) get_dyn_module()->getObjParent();
      char buf[1024];
      appImage->getProgramFileName(buf, 1024);
      pdstring exe_name = pdstring(buf);

      if (getenv("PARADYN_LOOPS") != NULL) {
	  //if (true) {
          // add nested loops and function calls
          BPatch_loopTreeNode *root = pdf->getLoopTree(proc);
          FillInCallGraphNodeNested(exe_name, resource_full_name,
                                    resource_full_name, root);
      }
      else {
          AddCallGraphNodeCallback(exe_name, resource_full_name);
          AddCallGraphStaticChildrenCallback(exe_name, resource_full_name,
                                             callees_as_strings);
      }

      //Locate the dynamic call sites within the function, and notify 
      //the front end as to their existence
      pdvector<instPoint*> callPoints  = pdf->funcCalls(proc);
      for(unsigned i = 0; i < callPoints.size(); i++){
         if(proc->isDynamicCallSite(callPoints[i])){
            tp->CallGraphAddDynamicCallSiteCallback(exe_name,
                                                    resource_full_name);
            break;
         }
      }
   }
}



