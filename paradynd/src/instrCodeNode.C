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

#include "paradynd/src/instrCodeNode.h"
#include "paradynd/src/instrDataNode.h"
#include "paradynd/src/processMetFocusNode.h"
#include "paradynd/src/threadMetFocusNode.h"
#include "paradynd/src/pd_process.h"
#include "dyninstAPI/h/BPatch_point.h"
#ifdef NOTDEF // PDSEP
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/instPoint.h"
#include "dyninstAPI/src/instPoint.h" // for pointFunc
#endif
#include "pdutil/h/pdDebugOstream.h"
#include "paradynd/src/instReqNode.h"
#include "paradynd/src/variableMgr.h"
#include "paradynd/src/debug.h"

pdstring instrCodeNode::collectThreadName("CollectThread");

dictionary_hash<pdstring, instrCodeNode_Val*> 
                           instrCodeNode::allInstrCodeNodeVals(pdstring::hash);


static void cleanupDataInCodeNode(void *temp, miniTrampHandle *);

instrCodeNode *instrCodeNode::newInstrCodeNode(pdstring name_, const Focus &f,
                                   pd_process *proc, bool arg_dontInsertData, 
                                               pdstring hw_cntr_str)
{
  instrCodeNode_Val *nodeVal;
  // it's fine to use a code node with data inserted for a code node
  // that doesn't need data to be inserted
  pdstring key_name = instrCodeNode_Val::construct_key_name(name_, f.getName());
  bool foundIt = allInstrCodeNodeVals.find(key_name, nodeVal);

  if(! foundIt) {
    HwEvent* hw = NULL;
    /* if PAPI isn't available, hw_cntr_str should always be "" */
    if (hw_cntr_str != "") {
#ifdef PAPI
      papiMgr* papi;
      papi = proc->getPapiMgr();
      assert(papi);
      hw = papi->createHwEvent(hw_cntr_str);

      if (hw == NULL) {
	string msg = pdstring("unable to add PAPI hardware event: ") 
	             + hw_cntr_str;
        showErrorCallback(125, msg.c_str());
        return NULL;
      }
#endif
    }

    nodeVal = new instrCodeNode_Val(name_, f, proc, arg_dontInsertData, hw);
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
  pdstring key_name = instrCodeNode_Val::construct_key_name(par.getName(), 
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
{ 
}

instrCodeNode::instrCodeNode(instrCodeNode_Val *val) 
   : V(*val) 
{  
}

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
         pdvector<instrDataNode *> affectedNodes;
         getDataNodes(&affectedNodes);
         for (unsigned i = 0; i < affectedNodes.size(); i++) {
            affectedNodes[i]->incRefCount();
         }
         newInstReq->setAffectedDataNodes(&cleanupDataInCodeNode, this);
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
}

instrCodeNode_Val::~instrCodeNode_Val() {
  for (unsigned i=0; i<instRequests.size(); i++) {
     instRequests[i]->disable(proc()); // calls deleteInst()
  }

  if(sampledDataNode != NULL) {
    sampledDataNode = NULL;
  }
  if(constraintDataNode != NULL) {
    constraintDataNode = NULL;
  }
  
  pdvector<instrDataNode*>::iterator itr = tempCtrDataNodes.end();
  while(itr != tempCtrDataNodes.begin()) {
     itr--;
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

pdstring instrCodeNode_Val::getKeyName() {
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

void instrCodeNode_Val::cleanupMiniTrampHandle(miniTrampHandle *mtHandle) {
   for (int i=instRequests.size()-1; i>=0; i--)
   {
      instReqNode *req = instRequests[i];
      if (req->miniTramp() == mtHandle)
      {
         instRequests.erase(i, i);
         delete req;
      }
   }
}

void instrCodeNode_Val::cleanupDataRefNodes() {
   bool clear_tempctr = true;
   if (constraintDataNode != NULL) {
      if (!constraintDataNode->decRefCount()) 
         constraintDataNode = NULL;
   }
   if (sampledDataNode != NULL) {
      if (!sampledDataNode->decRefCount())
         sampledDataNode = NULL;
   }
   for (unsigned i=0; i < tempCtrDataNodes.size(); i++) {
      if (!tempCtrDataNodes[i] || !tempCtrDataNodes[i]->decRefCount())
         tempCtrDataNodes[i] = NULL;
      else
         clear_tempctr = false;
   }
   if (clear_tempctr)
      tempCtrDataNodes.clear();
}

// A debug function for prepareCatchupInstr
void prepareCatchupInstr_debug(instReqNode &iRN)
{
  BPatch_function *instPoint_fn = const_cast<BPatch_function *>(iRN.Point()->getFunction());
     
  if (instPoint_fn) {
    char buf[2048];
    instPoint_fn->getName(buf, 2048);
    cerr << "instP function: " << buf << " ";
  }

  switch (iRN.When()) {
  case BPatch_callBefore:
    cerr << " callPreInsn for ";
    break;
  case BPatch_callAfter:
    cerr << " callPostInsn for ";
    break;
  }

  if( iRN.Point()->getPointType() == BPatch_entry )
     cerr << " Function Entry " << endl;
  else if( iRN.Point()->getPointType() == BPatch_exit )
     cerr << " FunctionExit " << endl;
  else if( iRN.Point()->getPointType() == BPatch_subroutine )
     cerr << " callSite " << endl;
  else
     cerr << " other" << endl;
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

      if (pd_debug_catchup) {     
         char buf[2048];
         BPatch_function *bpf = const_cast<BPatch_function *>(curInstReq->Point()->getFunction());
         if (! bpf) {
           sprintf(buf, "<bad function> in instReq");
         }
         else 
           bpf->getName(buf, 2048);
         cerr << "    looking at instReq [" << instIter << "], in func: "
              << buf <<endl;
      }

      // If the instRequest was not installed, skip...
      if( (curInstReq->getRInstance() != NULL) &&
          !(curInstReq->getRInstance()->Installed())) {
          if (pd_debug_catchup)   cerr << "Skipped, not installed\n";
          continue; // skip it (case 3 above)
      }
      // If it accesses parameters, skip it...
      if(curInstReq->Snippet()->PDSEP_ast()->accessesParam()) {
          if (pd_debug_catchup)   cerr << "Skipped, accesses parameters\n";
          continue; // Case 2
      }
      
      // Finally, test if it is active in any stack frame. Note: we can
      // get multiple starts this way, which is good. The counter variable
      // in the timer takes care of that.    
      for(unsigned frameIter=0; frameIter < stackWalk.size(); frameIter++)
      {
         Frame &cur_frame = stackWalk[frameIter]->frame;
	 // If this is an entry instru, and we've already triggered once,
	 // don't trigger again
	 if ((curInstReq->Point()->getPointType() == BPatch_locEntry &&
	      stackWalk[frameIter]->handledFunctionEntry) ||
	     (curInstReq->Point()->getPointType() == BPatch_locLoopEntry &&
	      stackWalk[frameIter]->handledLoopEntry))
	   continue;
	 
	 bool triggered = 
	   curInstReq->triggeredInStackFrame(cur_frame,
					     proc());
	 if(triggered) {
	   // Push this instRequest onto the list of ones to execute
	   stackWalk[frameIter]->reqNodes.push_back(curInstReq);
	   if (curInstReq->Point()->getPointType() == BPatch_locEntry)
	     stackWalk[frameIter]->handledFunctionEntry = true;
	   else if (curInstReq->Point()->getPointType() == BPatch_locLoopEntry)
	     stackWalk[frameIter]->handledLoopEntry = true;
	 }
      }
   }
   
   // don't mark catchup as having completed because we don't want to mark
   // this until the processMetFocusNode has completed initiating catchup for
   // all of the threads
}

inst_insert_result_t instrCodeNode::loadInstrIntoApp() {
   if(instrLoaded()) {
      return inst_insert_success;
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
           cerr << "marking " << (void*)this << " " << u1+1 << " / "
                << inst_size << " as deferred\n";
           return inst_insert_deferred;
           break;
        case failure_res:
           //cerr << "instRequest.insertInstr - wasn't successful\n";
           return inst_insert_failure;
           break;
        case success_res:
           //cerr << "instrRequest # " << u1+1 << " / " << inst_size
           //     << "inserted\n";
           // Interesting... it's possible that this minitramp writes to more
           // than one variable (data, constraint, "temp" vector)
           {
              pdvector<instrDataNode *> affectedNodes;
              getDataNodes(&affectedNodes);
              for (unsigned i = 0; i < affectedNodes.size(); i++)
                 affectedNodes[i]->incRefCount();
              instReq->setAffectedDataNodes(&cleanupDataInCodeNode, &V);
              break;
           }
      }
      if (retInst) {
         V.baseTrampInstances += retInst;
      }
   }
   V.instrLoaded_ = true;
   return inst_insert_success;
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
   if (V.sampledDataNode)     
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
         process *llproc = proc()->get_dyn_process()->lowlevel_process();
         curBaseTramp->installReturnInstance(llproc);
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
  dictionary_hash_iter<pdstring, instrCodeNode*> iter =
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

void instrCodeNode::addInst(BPatch_point *point, BPatch_snippet *snip,
			    BPatch_callWhen when, BPatch_snippetOrder o)
{
  if (!point) return;

  //cerr << "codeNode: " << getID() << ", addInst in func: " 
  //     << point->pointFunc()->prettyName() << "\n";

  instReqNode *newInstReqNode = new instReqNode(point, snip, when, o);
  V.instRequests.push_back(newInstReqNode);
}

static void cleanupDataInCodeNode(void *temp, miniTrampHandle *mt)
{
   instrCodeNode_Val *v = (instrCodeNode_Val *) temp;
   v->cleanupDataRefNodes();
   v->cleanupMiniTrampHandle(mt);
}
