/*
 * Copyright (c) 1996 Barton P. Miller
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
#include "dyninstAPI/src/symtab.h"
#include "paradynd/src/comm.h"
#include "dyninstAPI/src/process.h"

#include "dyninstAPI/h/BPatch_flowGraph.h"

extern pdRPC *tp;

pdstring pd_module::fileName() const {
   return get_dyn_module()->fileName();
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


// Called across all modules (in a given image) to define the call
//  graph relationship between functions.
// Must be called AFTER all functions in all modules (in image) are
//  registered as resource (e.g. w/ pdmodule::define())....
void pd_module::FillInCallGraphStatic(process *proc, bool printLoops=false) {
   pdvector<pd_Function *> callees;
   pdvector<pdstring>        callees_as_strings;
   const pdvector<pd_Function *> *mod_funcs = get_dyn_module()->getPD_Functions();
  
   // for each INSTRUMENTABLE function in the module (including excluded 
   //  functions, but NOT uninstrumentable ones)....
  
   for(unsigned f=0; f<(*mod_funcs).size(); f++) {
      pd_Function *pdf = (*mod_funcs)[f];

      if (printLoops) pdf->printLoops(proc);
    
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
         if(callee->FuncResourceSet()){
            pdstring callee_full_name = callee->ResourceFullName();
	
            // if callee->funcResource has been set, then it should have 
            //  been registered as a resource.... 
            resource *callee_as_resource = 
               resource::findResource(callee_full_name);
            assert(callee_as_resource);
            callees_as_strings.push_back(callee_full_name);
         }
      
      }//end for
    
      // register that callee_resources holds list of resource*s 
      //  describing children of resource r....
      pdstring exe_name = proc->getImage()->file();
      AddCallGraphNodeCallback(exe_name, resource_full_name);

      AddCallGraphStaticChildrenCallback(exe_name, resource_full_name,
                                         callees_as_strings);

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



