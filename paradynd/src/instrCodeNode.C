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
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/instPoint.h"
#include "pdutil/h/pdDebugOstream.h"
#include "paradynd/src/instReqNode.h"
#include "paradynd/src/variableMgr.h"
#include "dyninstAPI/src/pdThread.h"

extern pdDebug_ostream metric_cerr;
extern pdDebug_ostream sampleVal_cerr;

string instrCodeNode::collectThreadName("CollectThread");

dictionary_hash<string, instrCodeNode_Val*> 
                           instrCodeNode::allInstrCodeNodeVals(string::hash);

instrCodeNode_Val::~instrCodeNode_Val() {
  for (unsigned i=0; i<instRequests.size(); i++) {
    instRequests[i].disable(proc()); // calls deleteInst()
  }

  if(sampledDataNode != NULL) {
    //sampledDataNode->disable();
    sampledDataNode = NULL;
  }
  if(constraintDataNode != NULL) {
    //constraintDataNode->disable();
    constraintDataNode = NULL;
  }
  for (int u=(int)tempCtrDataNodes.size()-1; u>=0; u--) {
    //tempCtrDataNodes[u]->disable();
    tempCtrDataNodes.erase(u);
  }
}


vector<instrDataNode *> *instrCodeNode_Val::getDataNodes() { 
  vector<instrDataNode*> *buff = new vector<instrDataNode *>();
  if(constraintDataNode != NULL)  buff->push_back(constraintDataNode);
  for(unsigned i=0; i<tempCtrDataNodes.size(); i++) {
    buff->push_back(tempCtrDataNodes[i]);
  }
  if(sampledDataNode != NULL)  buff->push_back(sampledDataNode);
  return buff;
}

string instrCodeNode_Val::getKeyName() {
  return construct_key_name(name, focus.getName());
}

instrCodeNode::~instrCodeNode() {
  V.decrementRefCount();
  if(V.getRefCount() == 0) {
    allInstrCodeNodeVals.undef(V.getKeyName());
    delete &V;
  }
}

void instrCodeNode::cleanup_drn() {
  //  for (unsigned u=0; u<V.dataNodes.size(); u++) {
    //    dataNodes[u]->cleanup_drn();
  //}
}

instrCodeNode *instrCodeNode::newInstrCodeNode(string name_, const Focus &f,
				        process *proc, bool arg_dontInsertData)
{
  instrCodeNode_Val *nodeVal;
  bool foundIt = false;

  // it's fine to use a code node with data inserted for a code node
  // that doesn't need data to be inserted
  string key_name = instrCodeNode_Val::construct_key_name(name_, f.getName());
  foundIt = allInstrCodeNodeVals.find(key_name, nodeVal);
  //if(foundIt) cerr << "found instrCodeNode " << key_name << " (" << nodeVal
  //		   << "), using it, , arg_proc=" << (void*)proc << "\n";

  if(! foundIt) {
    nodeVal = new instrCodeNode_Val(name_, f, proc);
    //cerr << "instrCodeNode " << key_name << " (" << nodeVal 
    //     << ") doesn't exist, creating one (proc=" << (void*)proc << ")\n";

    // we don't want to save code nodes that don't have data inserted
    // because they might be reused for a code node where we need the
    // data to be inserted
    if(! arg_dontInsertData)
      allInstrCodeNodeVals[key_name] = nodeVal;
  }
  nodeVal->incrementRefCount();
  instrCodeNode *retNode = new instrCodeNode(nodeVal);
  return retNode;
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
    vector<string> name = instPoint_fn->prettyNameVector();
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

void instrCodeNode::prepareCatchupInstr(vector<vector<catchupReq *> > &allStackWalks)
{
  V.hasBeenCatchuped_ = true;
  // Okay, we have a list of stack frames (allStackWalks[stackIter]), 
  // and a list of instPoints (instRequests). We want to get a list of
  // the instRequests that are on the stack. So, we loop through the 
  // instrumentation requests (the instPoints), checking to see if:
  //   1) the instPoint would have been triggered (triggeredInStackFrame)
  //   2) the instPoint takes no arguments from the function it is in
  //      (knowledge that we no longer have)
  //   3) That the instrumentation was actually put in and not deferred
  // If these three requirements pass, the instPoint and the associated
  // frame are stored for later manual triggering.
  // Note: we repeat this for each stack walk we have (The above for loop)
  
  // Note: the matchup is probably sparse, and so this is extremely inefficient
  // O(#threads*#instrequests*#of frames on stack), where realistically it should 
  // be O(#threads*#instrequests). 

  // We loop through the inst requests first, since an inst request might
  // not be suitable for catchup. If we don't like an inst request, we can skip
  // it once and for all.

  for (unsigned instIter = 0; instIter < V.instRequests.size(); instIter++) {
    //prepareCatchupInstr_debug(V.instRequests[instIter]);
    // If the instRequest was not installed, skip...
    if ( (V.instRequests[instIter].getRInstance() != NULL) &&
	 !(V.instRequests[instIter].getRInstance()->Installed())) {
      if (pd_debug_catchup) {
	cerr << "Skipped, not installed" << endl;
      }
      continue; // skip it (case 3 above)
    }
    // If it accesses parameters, skip it...
    if (V.instRequests[instIter].Ast()->accessesParam()) {
      if (pd_debug_catchup) {
	cerr << "Skipped, accesses parameters" << endl;
      }
      continue; // Case 2
    }
    // Finally, test if it is active in any stack frame. Note: we can
    // get multiple starts this way, which is good. The counter variable
    // in the timer takes care of that.
    
    for (unsigned stackIter = 0; stackIter < allStackWalks.size(); stackIter++) {    
      for (int frameIter = 0; frameIter < allStackWalks[stackIter].size(); frameIter++) {
	Frame thisFrame = (allStackWalks[stackIter])[frameIter]->frame;
      
	bool triggered = V.instRequests[instIter].triggeredInStackFrame(thisFrame, proc());
	if (triggered) {
	  // Push this instRequest onto the list of ones to execute
	  allStackWalks[stackIter][frameIter]->reqNodes.push_back(&(V.instRequests[instIter]));
	} // If we want catchup
      } // loop through instrumentation requests
    } // loop through a stack walk.
  } // loop through all stack walks
  // if MTHREAD
  // Not sure what the following did: figure it out and set it up.
  //oldCatchUp(tid);
  
}

bool instrCodeNode::loadInstrIntoApp(pd_Function **func) {
   // Loop thru "instRequests", an array of instReqNode:
   // (Here we insert code instrumentation, tramps, etc. via addInstFunc())
   unsigned int inst_size = V.instRequests.size();
   //cerr << "instrCodeNode id: " << getID() << " attempted insert of " 
   //     << inst_size << " instRequests\n";
   for (unsigned u1=0; u1<inst_size; u1++) {
      // code executed later (prepareCatchupInstr) may also manually trigger 
      // the instrumentation via inferiorRPC.
      returnInstance *retInst=NULL;
      instReqNode *instReq = &V.instRequests[u1];

      instInstance *mtInst;
      loadMiniTramp_result res = instReq->loadInstrIntoApp(proc(), retInst,
							   &mtInst);

      unmarkAsDeferred();
      switch(res) {
	case deferred_res:
	   *func = dynamic_cast<pd_Function *>(const_cast<function_base *>(
                            instReq->Point()->iPgetFunction()));
	   markAsDeferred();
	   //cerr << "marking " << (void*)this << " " << u1+1 << " / "
	   //     << inst_size << " as deferred\n";
	   //cerr << "deferred on function " << (*func)->prettyName() << "\n";
	   return false;
	   break;
	case failure_res:
	  //cerr << "instRequest.insertInstr - wasn't successful\n";
	   //assert (*func != NULL);
	   return false; // shouldn't we try to undo what's already put in?
	   break;
	case success_res:
	  //cerr << "instrRequest # " << u1+1 << " / " << inst_size
	  //     << "inserted\n";
	   // Interesting... it's possible that this minitramp writes to more
	   // than one variable (data, constraint, "temp" vector)
	   if(mtInst) {
	      vector <instrDataNode *> *affectedNodes = V.getDataNodes();
	      for (unsigned i = 0; i < affectedNodes->size(); i++)
		 (*affectedNodes)[i]->incRefCount();
	      mtInst->registerCallback(instrDataNode::decRefCountCallback, 
				       (void *)affectedNodes);
	   }
	   break;
      }
      if (retInst) {
	 V.baseTrampInstances += retInst;
      }
   }
   V.instrLoaded_ = true;
   return true;
}

void instrCodeNode::prepareForSampling(
                                  const vector<threadMetFocusNode *> &thrNodes)
{
  if(! instrLoaded()) return;

  for(unsigned i=0; i<thrNodes.size(); i++) {
    threadMetFocusNode *curThrNode = thrNodes[i];
    V.sampledDataNode->prepareForSampling(curThrNode->getThreadPos(), 
					  curThrNode->getValuePtr());
  }
}

void instrCodeNode::prepareForSampling(threadMetFocusNode *thrNode) {
  V.sampledDataNode->prepareForSampling(thrNode->getThreadPos(), 
					thrNode->getValuePtr());
}

void instrCodeNode::stopSamplingThr(threadMetFocusNode_Val *thrNodeVal) {
  V.sampledDataNode->stopSampling(thrNodeVal->getThreadPos());
}

bool instrCodeNode::needToWalkStack() {
   for (unsigned u1=0; u1<V.baseTrampInstances.size(); u1++) {
      if (V.baseTrampInstances[u1]->needToWalkStack())
	 return true;
   }
   return false;
}

bool instrCodeNode::insertJumpsToTramps(vector<Frame> stackWalk) {
   if(trampsHookedUp()) return true;

   for(unsigned k=0; k<V.instRequests.size(); k++) {
      V.instRequests[k].hookupJumps(proc());
   }
   
   vector<returnInstance *> &baseTrampInstances = V.getBaseTrampInstances();
   unsigned rsize = baseTrampInstances.size();
   u_int max_index = 0;  // first frame where it is safe to install instr
   bool delay_install = false; // true if some instr. needs to be delayed 
   vector<bool> delay_elm(rsize); // wch instr. to delay
   // for each inst point walk the stack to determine if it can be
   // inserted now (it can if it is not currently on the stack)
   // If some can not be inserted, then find the first safe point on
   // the stack where all can be inserted, and set a break point  

   for (unsigned u=0; u<rsize; u++) {
      u_int index = 0;
#if defined(MT_THREAD) 
      bool installSafe = true; 
#else
      // only overwrite 1 instruction on power arch (2 on mips arch)
      // always safe to instrument without stack walk
      // pc is empty for those didn't do a stack walk, will return safe.
      bool installSafe = baseTrampInstances[u]->checkReturnInstance(stackWalk,index);
#endif
      // if unsafe, index will be set to the first unsafe stack walk ndx
      // (0 being top of stack; i.e. the current pc)
      
      if (!installSafe && index > max_index)
	 max_index = index;
      
      if (installSafe) {
	 baseTrampInstances[u]->installReturnInstance(proc());
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
    ret += V.instRequests[i].cost(proc());
  }
  return ret;
}

void instrCodeNode::print() {
   cerr << "S:" << (void*)this << "\n";
}

/*
// Check if "mn" and "this" correspond to the same instrumentation?
bool instrCodeNode::condMatch(instrCodeNode *mn,
				   vector<dataReqNode*> &data_tuple1,
				   vector<dataReqNode*> &data_tuple2) {
  vector<dataReqNode *> this_datanodes = getDataRequests();
  vector<dataReqNode *> other_datanodes = mn->getDataRequests();

  vector<instReqNode> this_instRequests = V.getInstRequests();
  vector<instReqNode> other_instRequests = mn->getInstRequests();  

  // Both "this" metricFocusNode and the passed in metricFocusNode
  // "mn" have the same number of dataRequestNodes 
  if ((this_datanodes.size() != other_datanodes.size()) ||
      (this_instRequests.size() != other_instRequests.size())) {
    return false;
  }

  unsigned instreqs_size = this_instRequests.size();

  // need to return this match?
  // vector<dataReqNode*> data_tuple1, data_tuple2; // initialization?

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
  vector<instrCodeNode*> allprims;
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

    vector<dataReqNode*> data_tuple1;
    vector<dataReqNode*> data_tuple2;
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

  instReqNode temp(point, ast, when, o);
  V.instRequests += temp;
}




