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

#include "paradynd/src/instrCodeNode.h"
#include "paradynd/src/instrDataNode.h"
#include "paradynd/src/processMetFocusNode.h"
#include "paradynd/src/threadMetFocusNode.h"
#include "paradynd/src/pd_process.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/instPoint.h"
#include "pdutil/h/pdDebugOstream.h"
#include "paradynd/src/instReqNode.h"
#include "paradynd/src/variableMgr.h"

extern unsigned enable_pd_metric_debug;

#if ENABLE_DEBUG_CERR == 1
#define metric_cerr if (enable_pd_metric_debug) cerr
#else
#define metric_cerr if (0) cerr
#endif /* ENABLE_DEBUG_CERR == 1 */

extern unsigned enable_pd_samplevalue_debug;

#if ENABLE_DEBUG_CERR == 1
#define sampleVal_cerr if (enable_pd_samplevalue_debug) cerr
#else
#define sampleVal_cerr if (0) cerr
#endif /* ENABLE_DEBUG_CERR == 1 */

string instrCodeNode::collectThreadName("CollectThread");

dictionary_hash<string, instrCodeNode_Val*> 
                           instrCodeNode::allInstrCodeNodeVals(string::hash);


instrCodeNode *instrCodeNode::newInstrCodeNode(string name_, const Focus &f,
                                   pd_process *proc, bool arg_dontInsertData, 
                                               string hw_cntr_str)
{
  instrCodeNode_Val *nodeVal;
  // it's fine to use a code node with data inserted for a code node
  // that doesn't need data to be inserted
  string key_name = instrCodeNode_Val::construct_key_name(name_, f.getName());
  bool foundIt = allInstrCodeNodeVals.find(key_name, nodeVal);
  //if(foundIt) cerr << "found instrCodeNode " << key_name << " (" << nodeVal
  //		   << "), using it, , arg_proc=" << (void*)proc << "\n";

  if(! foundIt) {
    HwEvent* hw = NULL;
    /* if PAPI isn't available, hw_cntr_str should always be "" */
    if (hw_cntr_str != "") {
#ifdef PAPI
      papiMgr* papi;
      papi = proc->getPapiMgr();
      assert(papi);
      hw = papi->createHwEvent(hw_cntr_str);

      fprintf(stderr, "MRM_DEBUG: done with createHwEvent\n");

      if (hw == NULL) {
	string msg = string("unable to add PAPI hardware event: ") 
	             + hw_cntr_str;
        showErrorCallback(125, msg.c_str());
        return NULL;
      }
#endif
    }

    nodeVal = new instrCodeNode_Val(name_, f, proc, arg_dontInsertData, hw);
    //cerr << "instrCodeNode " << key_name << " (" << nodeVal 
    //     << ") doesn't exist, creating one (proc=" << (void*)proc << ")\n";
    registerCodeNodeVal(nodeVal);
  }

  nodeVal->incrementRefCount();
  instrCodeNode *retNode = new instrCodeNode(nodeVal);
  return retNode;
}

instrCodeNode *instrCodeNode::copyInstrCodeNode(const instrCodeNode &par,
						pd_process *childProc) {
  instrCodeNode_Val *nodeVal;
  Focus adjustedFocus = adjustFocusForPid(par.getFocus(), childProc->getPid());
  string key_name = instrCodeNode_Val::construct_key_name(par.getName(), 
						      adjustedFocus.getName());
  bool foundIt = allInstrCodeNodeVals.find(key_name, nodeVal);
  //if(foundIt) cerr << "found instrCodeNode " << key_name << " (" << nodeVal
  //		   << "), using it, , arg_proc=" << childProc << "\n";

  if(! foundIt) {
    nodeVal = new instrCodeNode_Val(par.V, childProc);
    //cerr << "instrCodeNode " << key_name << " (" << nodeVal 
    //     << ") doesn't exist, creating one (proc=" << childProc << ")\n";
    registerCodeNodeVal(nodeVal);
  }
  nodeVal->incrementRefCount();
  instrCodeNode *retNode = new instrCodeNode(nodeVal);
  return retNode;
}

void instrCodeNode::registerCodeNodeVal(instrCodeNode_Val *nodeVal) {
  // we don't want to save code nodes that don't have data inserted
  // because they might be reused for a code node where we need the
  // data to be inserted
  if(! nodeVal->getDontInsertData())
    allInstrCodeNodeVals[nodeVal->getKeyName()] = nodeVal;
}

instrCodeNode::instrCodeNode(const instrCodeNode &par, pd_process *childProc)
   : V(* new instrCodeNode_Val(par.V, childProc)) 
{ }

instrCodeNode_Val::instrCodeNode_Val(const instrCodeNode_Val &par,
				     pd_process *childProc) :
  name(par.name), focus(adjustFocusForPid(par.focus, childProc->getPid())),
  hwEvent(par.hwEvent)
{
   if(par.sampledDataNode)
      sampledDataNode = new instrDataNode(*par.sampledDataNode, childProc);
   else sampledDataNode = NULL;

   if(par.constraintDataNode)
      constraintDataNode = new instrDataNode(*par.constraintDataNode, 
                                             childProc);
   else constraintDataNode = NULL;
   
   for(unsigned i=0; i<par.tempCtrDataNodes.size(); i++) {
      tempCtrDataNodes.push_back(new instrDataNode(*(par.tempCtrDataNodes[i]), 
                                                   childProc));
   }

   for(unsigned j=0; j<par.instRequests.size(); j++) {
      instReqNode *newInstReq = new instReqNode(*par.instRequests[j], 
                                                childProc);
      instRequests.push_back(newInstReq);

      if(newInstReq->instrLoaded()) {
         // update the data assocated with the minitramp deletion callback
         pdvector<instrDataNode *> *affectedNodes = 
            new pdvector<instrDataNode *>;
         getDataNodes(affectedNodes);
         for (unsigned i = 0; i < affectedNodes->size(); i++) {
            (*affectedNodes)[i]->incRefCount();
         }
         newInstReq->setAffectedDataNodes(instrDataNode::decRefCountCallback, 
                                          affectedNodes);
      }
   }
   baseTrampInstances = par.baseTrampInstances;
   trampsNeedHookup_ = par.trampsNeedHookup_;
   needsCatchup_ = par.needsCatchup_;
   instrDeferred_ = par.instrDeferred_;
   instrLoaded_ = par.instrLoaded_;
   proc_ = childProc;
   
   dontInsertData_ = par.dontInsertData_;
   referenceCount = 0;  // this node when created, starts out unshared
#if defined(MT_THREAD)
   // remember names of each of its threads (tid + start_func_name)
   thr_names = par.thr_names;
#endif
}

instrCodeNode_Val::~instrCodeNode_Val() {
  for (unsigned i=0; i<instRequests.size(); i++) {
    instRequests[i]->disable(proc()); // calls deleteInst()
  }

  if(sampledDataNode != NULL) {
    //sampledDataNode->disable();
    sampledDataNode = NULL;
  }
  if(constraintDataNode != NULL) {
    //constraintDataNode->disable();
    constraintDataNode = NULL;
  }
  
  pdvector<instrDataNode*>::iterator itr = tempCtrDataNodes.end();
  while(itr != tempCtrDataNodes.begin()) {
     itr--;
     //tempCtrDataNodes[u]->disable();
     tempCtrDataNodes.erase(itr);
  }

  for(unsigned j=0; j<instRequests.size(); j++)
     delete instRequests[j];
}


void instrCodeNode_Val::getDataNodes(pdvector<instrDataNode *> *saveBuf) { 
  if(constraintDataNode != NULL)  (*saveBuf).push_back(constraintDataNode);
  for(unsigned i=0; i<tempCtrDataNodes.size(); i++) {
    (*saveBuf).push_back(tempCtrDataNodes[i]);
  }
  if(sampledDataNode != NULL)  (*saveBuf).push_back(sampledDataNode);
}

string instrCodeNode_Val::getKeyName() {
  return construct_key_name(name, focus.getName());
}

instrCodeNode::~instrCodeNode() {
  V.decrementRefCount();

  if(V.getRefCount() == 0) {
#ifdef PAPI
    if (V.hwEvent != NULL) {
      delete V.hwEvent;
    }
#endif    
    allInstrCodeNodeVals.undef(V.getKeyName());
    delete &V;
  }
}

void instrCodeNode::cleanup_drn() {
  //  for (unsigned u=0; u<V.dataNodes.size(); u++) {
    //    dataNodes[u]->cleanup_drn();
  //}
}

// A debug function for prepareCatchupInstr
void prepareCatchupInstr_debug(instReqNode &iRN)
{
  //
  //
  pd_Function *instPoint_fn = dynamic_cast<pd_Function *>
    (const_cast<function_base *>
     (iRN.Point()->iPgetFunction()));
  if (instPoint_fn) {
    pdvector<string> name = instPoint_fn->prettyNameVector();
    if (name.size())
      cerr << "instP function: " << name[0] << " ";
  }

  switch (iRN.When()) {
  case callPreInsn:
    cerr << " callPreInsn for ";
    break;
  case callPostInsn:
    cerr << " callPostInsn for ";
    break;
  }
#if defined(mips_sgi_irix6_4)
  if( iRN.Point()->type() == IPT_ENTRY )
    cerr << " FunctionEntry " << endl;
  else if (iRN.Point()->type() == IPT_EXIT )
    cerr << " FunctionExit " << endl;
  else if (iRN.Point()->type() == IPT_CALL )
    cerr << " callSite " << endl;
  
#elif defined(sparc_sun_solaris2_4) || defined(alpha_dec_osf4_0)
  if( iRN.Point()->ipType == functionEntry )
    cerr << " Function Entry " << endl;
  else if( iRN.Point()->ipType == functionExit )
    cerr << " FunctionExit " << endl;
  else if( iRN.Point()->ipType == callSite )
    cerr << " callSite " << endl;
  
#elif defined(rs6000_ibm_aix4_1)
  if( iRN.Point()->ipLoc == ipFuncEntry )
    cerr << " FunctionEntry " << endl;
  if( iRN.Point()->ipLoc == ipFuncReturn )
    cerr << " FunctionExit " << endl;
  if( iRN.Point()->ipLoc == ipFuncCallPoint )
    cerr << " callSite " << endl;
#elif defined(i386_unknown_nt4_0) || defined(i386_unknown_solaris2_5) || defined(i386_unknown_linux2_0) || defined(ia64_unknown_linux2_4) /* Temporary duplication - TLM */
  if( iRN.Point()->iPgetAddress() == iRN.Point()->iPgetFunction()->addr() )
    cerr << " FunctionEntry " << endl;
  else if ( iRN.Point()->insnAtPoint().isCall() ) 
    cerr << " calSite " << endl;
  else
    cerr << " FunctionExit " << endl;
#else
#error Check for instPoint type == entry not implemented on this platform
#endif
  
//
//
}

void instrCodeNode::prepareCatchupInstr(pdvector<catchupReq *> &stackWalk)
{
   if(needsCatchup()==false) return;

   // Okay, we have a stack frame and a list of instPoints (instRequests). We
   // want to get a list of the instRequests that are on the stack. So, we
   // loop through the instrumentation requests (the instPoints), checking to
   // see if:
   //   1) the instPoint would have been triggered (triggeredInStackFrame)
   //   2) the instPoint takes no arguments from the function it is in
   //      (knowledge that we no longer have)
   //   3) That the instrumentation was actually put in and not deferred
   // If these three requirements pass, the instPoint and the associated
   // frame are stored for later manual triggering.
   // Note: we repeat this for each stack walk we have (The above for loop)
   
   // Note: the matchup is probably sparse, and so this is extremely
   // inefficient O(#threads * #instrequests * #of frames on stack), where
   // realistically it should be O(#threads * #instrequests).
   
   // We loop through the inst requests first, since an inst request might
   // not be suitable for catchup. If we don't like an inst request, we can
   // skip it once and for all.
   for (unsigned instIter = 0; instIter < V.instRequests.size(); instIter++)
   {
      instReqNode *curInstReq = V.instRequests[instIter];
      //prepareCatchupInstr_debug(V.instRequests[instIter]);

      // If the instRequest was not installed, skip...
      if( (curInstReq->getRInstance() != NULL) &&
          !(curInstReq->getRInstance()->Installed())) {
          if (pd_debug_catchup)   cerr << "Skipped, not installed\n";
          continue; // skip it (case 3 above)
      }
      // If it accesses parameters, skip it...
      if(curInstReq->Ast()->accessesParam()) {
          if (pd_debug_catchup)   cerr << "Skipped, accesses parameters\n";
          continue; // Case 2
      }
      
      // Finally, test if it is active in any stack frame. Note: we can
      // get multiple starts this way, which is good. The counter variable
      // in the timer takes care of that.    
      for(unsigned frameIter=0; frameIter < stackWalk.size(); frameIter++)
      {
          Frame thisFrame = stackWalk[frameIter]->frame;
          bool triggered = 
          curInstReq->triggeredInStackFrame(stackWalk[frameIter]->frame,
                                            stackWalk[frameIter]->func,
                                            proc());
          if(triggered) {
              // Push this instRequest onto the list of ones to execute
              stackWalk[frameIter]->reqNodes.push_back(curInstReq);
          }
      }
   }
   
   // don't mark catchup as having completed because we don't want to mark
   // this until the processMetFocusNode has completed initiating catchup for
   // all of the threads

   // if MTHREAD
   // Not sure what the following did: figure it out and set it up.
   //oldCatchUp(tid);
}

instr_insert_result_t instrCodeNode::loadInstrIntoApp() {
   if(instrLoaded()) {
      return insert_success;
   }

   V.trampsNeedHookup_ = true;
   V.needsCatchup_ = true;
  
   // Loop thru "instRequests", an array of instReqNode:
   // (Here we insert code instrumentation, tramps, etc. via addInstFunc())
   unsigned int inst_size = V.instRequests.size();
   //cerr << "instrCodeNode id: " << getID() << " attempted insert of " 
   //     << inst_size << " instRequests\n";
   for(unsigned u1=0; u1<inst_size; u1++) {
      // code executed later (prepareCatchupInstr) may also manually trigger 
      // the instrumentation via inferiorRPC.
      returnInstance *retInst=NULL;
      instReqNode *instReq = V.instRequests[u1];
      if(instReq->instrLoaded())  continue;

      loadMiniTramp_result res = instReq->loadInstrIntoApp(proc(), retInst);
      
      unmarkAsDeferred();
      switch(res) {
        case deferred_res:
           markAsDeferred();
           // cerr << "marking " << (void*)this << " " << u1+1 << " / "
           //      << inst_size << " as deferred\n";
           return insert_deferred;
           break;
        case failure_res:
           //cerr << "instRequest.insertInstr - wasn't successful\n";
           return insert_failure;
           break;
        case success_res:
           // cerr << "instrRequest # " << u1+1 << " / " << inst_size
           //      << "inserted\n";
           // Interesting... it's possible that this minitramp writes to more
           // than one variable (data, constraint, "temp" vector)
           {
              pdvector<instrDataNode *> *affectedNodes = 
                 new pdvector<instrDataNode *>;
              getDataNodes(affectedNodes);
              for (unsigned i = 0; i < affectedNodes->size(); i++)
                 (*affectedNodes)[i]->incRefCount();
              instReq->setAffectedDataNodes(instrDataNode::decRefCountCallback,
                                            affectedNodes);
              break;
           }
      }
      if (retInst) {
         V.baseTrampInstances += retInst;
      }
   }
   V.instrLoaded_ = true;
   return insert_success;
}

void instrCodeNode::prepareForSampling(
                                  const pdvector<threadMetFocusNode *> &thrNodes)
{
  if(! instrLoaded()) return;

  for(unsigned i=0; i<thrNodes.size(); i++) {
    threadMetFocusNode *curThrNode = thrNodes[i];
    V.sampledDataNode->prepareForSampling(curThrNode->getThreadIndex(), 
                                          curThrNode->getValuePtr());
  }

#ifdef PAPI
  if (V.hwEvent != NULL) {
    V.hwEvent->enable();
  }
#endif

}

void instrCodeNode::prepareForSampling(threadMetFocusNode *thrNode) {
  V.sampledDataNode->prepareForSampling(thrNode->getThreadIndex(), 
					thrNode->getValuePtr());
}

void instrCodeNode::stopSamplingThr(threadMetFocusNode_Val *thrNodeVal) {
  V.sampledDataNode->stopSampling(thrNodeVal->getThreadIndex());
}

bool instrCodeNode::needToWalkStack() {
   for (unsigned u1=0; u1<V.baseTrampInstances.size(); u1++) {
      if (V.baseTrampInstances[u1]->needToWalkStack())
	 return true;
   }
   return false;
}

bool instrCodeNode::insertJumpsToTramps(pdvector<pdvector<Frame> > &stackWalks) {
   if(trampsNeedHookup()==false) return true;

   for(unsigned k=0; k<V.instRequests.size(); k++) {
      V.instRequests[k]->hookupJumps(proc());
   }
   
   pdvector<returnInstance *> &baseTrampInstances = V.getBaseTrampInstances();
   unsigned rsize = baseTrampInstances.size();
   bool delay_install = false; // true if some instr. needs to be delayed 
   pdvector<bool> delay_elm(rsize); // wch instr. to delay
   // for each inst point walk the stack to determine if it can be
   // inserted now (it can if it is not currently on the stack)
   // If some can not be inserted, then find the first safe point on
   // the stack where all can be inserted, and set a break point  

   for (unsigned u=0; u<rsize; u++) {
      returnInstance *curBaseTramp = baseTrampInstances[u];

      // checkReturnInstance lives in the inst-<arch>.C files
      bool installSafe = curBaseTramp->checkReturnInstance(stackWalks);

      if (installSafe) {
         curBaseTramp->installReturnInstance(proc()->get_dyn_process());
         delay_elm[u] = false;
      } else {
         delay_install = true;
         delay_elm[u] = true;
      }
   }
   markTrampsAsHookedUp();
   return true;
}

timeLength instrCodeNode::cost() const {
  timeLength ret = timeLength::Zero();
  
  for (unsigned i=0; i<V.instRequests.size(); i++) {
    ret += V.instRequests[i]->cost(proc());
  }
  return ret;
}

void instrCodeNode::print() {
   cerr << "S:" << (void*)this << "\n";
}

/*
// Check if "mn" and "this" correspond to the same instrumentation?
bool instrCodeNode::condMatch(instrCodeNode *mn,
				   pdvector<dataReqNode*> &data_tuple1,
				   pdvector<dataReqNode*> &data_tuple2) {
  pdvector<dataReqNode *> this_datanodes = getDataRequests();
  pdvector<dataReqNode *> other_datanodes = mn->getDataRequests();

  pdvector<instReqNode> this_instRequests = V.getInstRequests();
  pdvector<instReqNode> other_instRequests = mn->getInstRequests();  

  // Both "this" metricFocusNode and the passed in metricFocusNode
  // "mn" have the same number of dataRequestNodes 
  if ((this_datanodes.size() != other_datanodes.size()) ||
      (this_instRequests.size() != other_instRequests.size())) {
    return false;
  }

  unsigned instreqs_size = this_instRequests.size();

  // need to return this match?
  // pdvector<dataReqNode*> data_tuple1, data_tuple2; // initialization?

  // Check that instRequestNodes in "mn" and "this" are the same. 
  for (unsigned i=0; i<instreqs_size; i++) {

    // what to do with "callWhen when" and "callOrder order"??
    bool match_flag = (this_instRequests[i].Point())->match(
					       other_instRequests[i].Point());
    if (!match_flag) {
      return false;
    }
    
    match_flag = (this_instRequests[i].Ast())->condMatch(
                       other_instRequests[i].Ast(), data_tuple1, data_tuple2,
		       this_datanodes, other_datanodes);
    if (!match_flag) {
      return false;
    }
  }

  return true;
}
*/

/*
// incremental code generation optimization
// check if match BEFORE add into allMIPrimitives
instrCodeNode* instrCodeNode::matchInMIPrimitives() {
  // note the two loops; we can't safely combine into one since the second
  // loop modifies the dictionary.
  pdvector<instrCodeNode*> allprims;
  dictionary_hash_iter<string, instrCodeNode*> iter =
                                                getIter_sampleMetFocusBuf();
  for (; iter; iter++) {
    allprims += iter.currval();
  }
  
  for (unsigned i=0; i < allprims.size(); i++) {
    instrCodeNode* primitiveMI = allprims[i];

    if ((primitiveMI->proc() != proc())) {
	//||  (primitiveMI->metStyle() != style_)) {  -- bhs
      continue;
    }

    // what about? -- not meaningful to primitive mdn at all!
    //   met_: met_.., internalMetric::isInternalMetric(aggMI->getMetName())
    //   focus_, component_focus, flat_name_
    //   id_, metric_name_, originalCost_? aggOp should be default
    
    // A NEW PROBLEM TO DO:
    //   component mdn needs to remember all names of its primitive mdns

    pdvector<dataReqNode*> data_tuple1;
    pdvector<dataReqNode*> data_tuple2;
    bool match_flag = condMatch(primitiveMI, data_tuple1, data_tuple2);
    if (match_flag) {
      return primitiveMI;
    }
  }
  
  return NULL;
}
//#endif
*/

// assume constraint variable is always the first in primitive
const instrDataNode *instrCodeNode::getFlagDataNode() const { 
  return V.constraintDataNode;
}

void instrCodeNode::addInst(instPoint *point, AstNode *ast,
			    callWhen when, callOrder o)
{
  if (!point) return;

  //cerr << "codeNode: " << getID() << ", addInst in func: " 
  //     << point->iPgetFunction()->prettyName() << "\n";

  instReqNode *newInstReqNode = new instReqNode(point, ast, when, o);
  V.instRequests.push_back(newInstReqNode);
}




