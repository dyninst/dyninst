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
#include "paradynd/src/instrThrDataNode.h"
#include "paradynd/src/processMetFocusNode.h"
#include "paradynd/src/threadMetFocusNode.h"
#include "dyninstAPI/src/process.h"
#include "paradynd/src/metric.h"
#include "dyninstAPI/src/instP.h"
#include "dyninstAPI/src/instPoint.h"
#include "pdutil/h/pdDebugOstream.h"
#include "paradynd/src/instReqNode.h"


extern pdDebug_ostream metric_cerr;
extern pdDebug_ostream sampleVal_cerr;

string instrCodeNode::collectThreadName("CollectThread");

dictionary_hash<string, instrCodeNode_Val*> 
                           instrCodeNode::allInstrCodeNodeVals(string::hash);

instrCodeNode_Val::~instrCodeNode_Val() {
  vector<addrVecType> pointsToCheck;

  for (unsigned u1=0; u1<instRequests.size(); u1++) {
    addrVecType pointsForThisRequest =
      getAllTrampsAtPoint(instRequests[u1].getInstance());
    pointsToCheck += pointsForThisRequest;
    
    instRequests[u1].disable(pointsForThisRequest); // calls deleteInst()
  }

  // disable components of aggregate metrics
  for (int u=(int)dataNodes.size()-1; u>=0; u--) {
    instrThrDataNode *dataNode = dataNodes[u];
    dataNode->disableAndDelete(pointsToCheck);
    dataNodes.erase(u);
  }
}

instrCodeNode::~instrCodeNode() {
  V.decrementRefCount();
  if(V.getRefCount() == 0) {
    allInstrCodeNodeVals.undef(V.name);
    delete &V;
  }
}

void instrCodeNode::cleanup_drn() {
  //  for (unsigned u=0; u<V.dataNodes.size(); u++) {
    //    dataNodes[u]->cleanup_drn();
  //}
}

instrCodeNode *instrCodeNode::newInstrCodeNode(string name_, process *proc,
					       bool arg_dontInsertData)
{
  instrCodeNode_Val *nodeVal;
  bool foundIt = false;

  // it's fine to use a code node with data inserted for a code node
  // that doesn't need data to be inserted
  foundIt = allInstrCodeNodeVals.find(name_, nodeVal);
  //  if(foundIt) cerr << "found instrCodeNode " << name_ << " (" << nodeVal
  //		   << "), using it, , arg_proc=" << (void*)proc << "\n";

  if(! foundIt) {
    nodeVal = new instrCodeNode_Val(name_, proc);
    //    cerr << "instrCodeNode " << name_ << " (" << nodeVal 
    //	 << ") doesn't exist, creating one (proc=" << (void*)proc << "\n";

    // we don't want to save code nodes that don't have data inserted
    // because they might be reused for a code node where we need the
    // data to be inserted
    if(! arg_dontInsertData)
      allInstrCodeNodeVals[name_] = nodeVal;
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

void instrCodeNode::prepareCatchupInstr(vector<Address> stack_pcs, 
					       int tid)
{
  vector<instReqNode> &instRequests = V.getInstRequests();
  vector<instPoint*> instPts;
  unsigned j, k;
  pd_Function *stack_func;
  instPoint *point;
  Address stack_pc;
  
  string prettyName; // not really a good name
  /* The variable met_ isn't a member of instrCodeNode.  In fact,
     there can't be "one" met_ since instrCodeNode's can be shared.
  if (pd_debug_catchup) {
    prettyName = met_ + string(": <");;

    bool first = true;
    for (unsigned h=0; h<focus_.size(); h++) {
      if (focus_[h].size() > 1) {
	if (!first) prettyName += string(",");
	first = false;
	for (unsigned c=0; c< focus_[h].size(); c++) {
	  prettyName += string("/");
	  prettyName += focus_[h][c];
	}
      }
    }
    prettyName += string(">");
  }
  */

  if( stack_pcs.size() == 0 ) {
    cerr << "WARNING -- prepareCatchupInstr was handed an empty stack" << endl;
  }
  vector<pd_Function *> stack_funcs = proc()->convertPCsToFuncs(stack_pcs);
  proc()->correctStackFuncsForTramps( stack_pcs, stack_funcs );
  bool badReturnInst = false;

  unsigned i = stack_funcs.size();
  //for(i=0;i<stack_funcs.size();i++) {
  if (i!=0)
    do {
      --i;
      stack_func = stack_funcs[i];
      stack_pc = stack_pcs[i];
      if (pd_debug_catchup) {
	if( stack_func != NULL ) {
	  instPoint *ip = findInstPointFromAddress(proc(), stack_pc);
	  if (ip) {// this is an inst point
	    cerr << i << ": " << stack_func->prettyName() << "@" << (void*) ip->iPgetAddress() << "[iP]"
		 << "@" << (void*)stack_pc << endl;
	  } else 
	    cerr << i << ": " << stack_func->prettyName() << "@" << (void*)stack_pc << endl;
	} else
	  cerr << i << ": skipped (unknown function) @" << (void*)stack_pc << endl;
	cerr << "proc() : " << proc() << "\n";
      }
      if (stack_func == NULL) continue;
      instPts.resize(0);
      instPts += const_cast<instPoint*>( stack_func->funcEntry(proc()) );
      instPts += stack_func->funcCalls(proc());
      instPts += stack_func->funcExits(proc());

#if defined(i386_unknown_nt4_0) || defined(i386_unknown_linux2_0) || defined(ia64_unknown_linux2_4) /* Temporary duplication - TLM */
      if (stack_func->isInstalled(proc())) {
        instPts += const_cast<instPoint*>( stack_func->funcEntry(0) );
        instPts += stack_func->funcCalls(0);
        instPts += stack_func->funcExits(0);
      }
#endif

#if !(defined(i386_unknown_nt4_0) \
  || defined(i386_unknown_solaris2_5) \
  || defined(i386_unknown_linux2_0) \
  || defined(ia64_unknown_linux2_4) ) /* Temporary duplication - TLM */
      // If there is a function on the stack with relevant instPoints which we were
      // not able to install return instances for, we don't want to manually trigger
      // anything else further on the stack, as it could cause inconsistencies with
      // metrics which rely on matching pairs of actions. - DAN
      // **NOTE** we don't do this on x86, because we can always immediately insert
      // a 'jump' to base tramp via. trap.  In addition, when we use a trap rather
      // than a branch, the return instance is still NULL, and this code breaks. - DAN

      //
      // If any instrumentation attempt failed for this function, we shouldn't
      // be doing catchup instrumentation for this function. - ZHICHEN
      //
      for(j=1;j<instPts.size()&&!badReturnInst;j++) {
	for(k=0;k<instRequests.size();k++) {
	  if( instPts[j] == instRequests[k].Point() ) {
	    if( instRequests[k].getRInstance() != NULL
		&& !(instRequests[k].getRInstance()->Installed()) 
	      )
	    {
	      if (pd_debug_catchup) {
	        cerr << "AdjustManuallyTrigger -- Bad return instance in "
		     << stack_func->prettyName()
		     << ", not manually triggering for this stack frame." << endl;
	      }
	      badReturnInst = true;
	      break;
	    }
	  }
	}
      }
#endif
      if( badReturnInst )
	continue;
      
      // For all of the inst points in all the functions on the stack...

      for(j=0;j<instPts.size();j++) {
	point = instPts[j];
	for(k=0;k<instRequests.size();k++) {
	  if (point == instRequests[k].Point()) {
	    // If we have an instrumentation request for that point...
	    
 	    if (instRequests[k].Ast()->accessesParam()) {
	      // and it accesses parameters for the function, break (parameters are unknown)
	      //cerr << "access parameters for the function so skipping\n";
	      break;
	    }
	    if (instRequests[k].triggeredInStackFrame(stack_func, stack_pc, proc()))
	      {
		// Otherwise, add it to the list to be manually triggered
		if (pd_debug_catchup) {
		  instReqNode &iRN = instRequests[k];
		  cerr << "--- catch-up needed for "
		       << prettyName << " @ " << stack_func->prettyName() 
		       << " @ " << (void*) stack_pc << endl;
		  prepareCatchupInstr_debug(iRN);
		}
		V.manuallyTriggerNodes += &(instRequests[k]);
		instRequests[k].friesWithThat(tid);
	      }
	  }
	}
      }
    } while (i!=0);

#if defined(MT_THREAD)

  oldCatchUp(tid);

#endif  // not OLD_CATCHUP, but MT_THREAD
}

void instrCodeNode::oldCatchUp(int tid) {
  unsigned k;

  assert(proc()); // proc_ should always be correct for non-aggregates
  const function_base *mainFunc = proc()->getMainFunction();
  assert(mainFunc); // processes should always have mainFunction defined
                    // Instead of asserting we could call prepareCatchupInstr0,
                    // which could handle a pseudo function.

  // The following code is used in the case where the new catchup code is
  // disabled.  It is replicated in the prepareCatchupInstr0 function and
  // at some point in the future could be moved into a single separate
  // function.  This code could also useful in the case where mainFunc is
  // undefined.

  // This code is a kludge which will catch the case where the WHOLE_PROGRAM
  // metrics have not been set to manjually trigger by the above code.  Look
  // at the component_focus for the "Code" element, and see if there is any
  // constraint.  Then, for each InstReqNode in this MetricDefinitionNode
  // which is at the entry point of main, and which has not been added to the
  // manuallyTriggerNodes list, add it to the list.

  /*  Needed to comment out since using component_focus, which isn't 
      a member of instrCodeNode.  This will need to be reimplemented.
      Since instrCodeNode's can be shared, I don't see how there can
      be "one" focus.
  for( j = 0; j < component_focus.size(); ++j )
    if( component_focus[j][0] == "Code" && 
	( component_focus[j].size() == 1 ||
	  ( component_focus[j].size() == 2 && component_focus[j][1] == "" ) ) )
  */      
  vector<instReqNode> &instRequests = V.getInstRequests();
  for( k = 0; k < instRequests.size(); ++k ) {
	if( instRequests[ k ].Point()->iPgetFunction() == mainFunc ) {
	  unsigned dummy;
	  if( !find( V.manuallyTriggerNodes, &(instRequests[k]), dummy ) ) {
#if defined(mips_sgi_irix6_4)
	  if( instRequests[ k ].Point()->type() == IPT_ENTRY )
#elif defined(sparc_sun_solaris2_4) || defined(alpha_dec_osf4_0)
	  if( instRequests[ k ].Point()->ipType == functionEntry )
#elif defined(rs6000_ibm_aix4_1)
	  if( instRequests[ k ].Point()->ipLoc == ipFuncEntry )
#elif defined(i386_unknown_nt4_0) || defined(i386_unknown_solaris2_5) \
      || defined(i386_unknown_linux2_0) || defined(ia64_unknown_linux2_4) /* Temporary duplication - TLM. */
	    if( instRequests[ k ].Point()->iPgetAddress() == mainFunc->addr() )
#else
#error Check for instPoint type == entry not implemented on this platform
#endif
	    {
              if ( pd_debug_catchup ) {
		metric_cerr << "AdjustManuallyTrigger -- "
			    << "(WHOLE_PROGRAM kludge) catch-up needed for "
		  //<< flat_name_   // not a flat_name_ member of instrCodeNode
			    << " @ " << mainFunc->prettyName() << endl;
              }
	      //manuallyTriggerNodes.insert( 0, &(instRequests[k]) );
	      V.manuallyTriggerNodes.push_back( &(instRequests[k]) );
	      instRequests[k].friesWithThat(tid);
	    }
	  }
	}
      }
}

#if defined(MT_THREAD)
void instrCodeNode::prepareCatchupInstr0(int tid) {
  // This code is a kludge which will catch the case where the WHOLE_PROGRAM
  // metrics have not been set to manjually trigger by the above code.  Look
  // at the component_focus for the "Code" element, and see if there is any
  // contraint.  Then, for each InstReqNode in this MetricDefinitionNode
  // which is at the entry point of main, and which has not been added to the
  // manuallyTriggerNodes list, add it to the list.

  oldCatchUp(tid);
}
#endif // MT_THREAD


// Look at the inst point corresponding to *this, and the stack.
// If inst point corresponds a location which is conceptually "over"
// the current execution frame, then set the manuallyTrigger flag
// (of *this) to true, (hopefully) causing the AST corresponding
// to the inst point to be executed (before the process resumes
// execution).
// What does conceptually "over" mean?
// An inst point is "over" the current execution state if that inst
//  point would have had to have been traversed to get to the current
//  execution state, had the inst point existed since before the
//  program began execution.
// In practise, inst points are categorized as function entry, exit, and call
//  site instrumentation.  entry instrumentation is "over" the current 
//  execution frame if the function it is applied to appears anywhere
//  on the stack.  exit instrumentation is never "over" the current 
//  execution frame.  call site instrumentation is "over" the current
//  execution frame if   


void instrCodeNode::manuallyTrigger(int mid) {
  for ( unsigned i=0; i < V.manuallyTriggerNodes.size(); ++i ) {
    if (!V.manuallyTriggerNodes[i]->triggerNow(proc(), mid)) {
      cerr << "manual trigger failed for an inst request" << endl;
    }
  }
  V.manuallyTriggerNodes.resize(0);
}

bool instrCodeNode::loadInstrIntoApp(pd_Function **func) {
  // Loop thru "dataRequests", an array of (ptrs to) dataReqNode: Here we
  // allocate ctrs/timers in the inferior heap but don't stick in any code,
  // except (if appropriate) that we'll instrument the application's
  // alarm-handler when not shm sampling.
  //unsigned size = dataRequests.size();
  //for (u=0; u<size; u++) {
  // the following allocs an object in inferior heap and arranges for it to
  // be alarm sampled, if appropriate.  Note: this is not necessary anymore
  // because we are allocating the space when the constructor for dataReqNode
  // is called. This was done for the dyninstAPI - naim 2/18/97
  //if (!dataRequests[u]->loadInstrIntoApp(proc_, this))
  //  return false; // shouldn't we try to undo what's already put in?
  //}
  
  // Loop thru "instRequests", an array of instReqNode:
  // (Here we insert code instrumentation, tramps, etc. via addInstFunc())
  unsigned int inst_size = V.instRequests.size();
  //cerr << "instrCodeNode id: " << getID() << " attempted insert of " 
  //     << inst_size << " instRequests\n";
  for (unsigned u1=0; u1<inst_size; u1++) {
    // code executed later (prepareCatchupInstr) may also manually trigger 
    // the instrumentation via inferiorRPC.
    returnInstance *retInst=NULL;
    bool deferredFlag = false;
    if (!V.instRequests[u1].loadInstrIntoApp(proc(), retInst, &deferredFlag)) {
      unmarkAsDeferred();
      if (deferredFlag == true) {
	*func = dynamic_cast<pd_Function *>(const_cast<function_base *>(
                            V.instRequests[u1].Point()->iPgetFunction()));
	markAsDeferred();
	//cerr << "marking " << (void*)this << " " << u1+1 << " / "
	//     << inst_size << " as deferred\n";
      }
      else cerr << "instRequest.insertInstr - wasn't successful\n";
      //assert (*func != NULL);
      return false; // shouldn't we try to undo what's already put in?
    } else {
      //cerr << "instrRequest # " << u1+1 << " / " << inst_size 
      //     << " inserted\n";
    }
    if (retInst) {
      V.returnInsts += retInst;
    }
  }
  V.instrLoaded_ = true;
  return true;
}

void instrCodeNode::mapSampledDRNs2ThrNodes(
                         const vector<threadMetFocusNode *> &thrNodes) {
  if(! instrLoaded()) return;

  for(unsigned i=0; i<thrNodes.size(); i++) {
    threadMetFocusNode *curThrNode = thrNodes[i];
    instrThrDataNode *curDataNode = getThrDataNode(curThrNode->getThreadID());
    if(curDataNode) {
      curDataNode->startSampling(curThrNode->getValuePtr());
      //cerr << "setting thrNode to " << (void*)curThrNode 
      //     << " for instrThrDataNode: " << (void*)curDataNode << "\n";
    } else {
      cerr << "Couldn't find a corresponding instrThrDataNode for the "
	   << "thrMetFocusNode which is being sampled\n";
      assert(false);
    }
  }
}

void instrCodeNode::stopSamplingThr(threadMetFocusNode_Val *thrNodeVal) {
  instrThrDataNode *curDataNode = getThrDataNode(thrNodeVal->getThreadID());  
  if(curDataNode) {
    curDataNode->stopSampling();    
  } else {
    cerr << "Warning, couldn't find proper thread_id for stopSampling\n";
  }
}


void instrCodeNode::recordAsParent(processMetFocusNode *procnode) {
  assert(proc() == procnode->proc());
  V.parentNodes.push_back(procnode);
}

bool instrCodeNode::needToWalkStack() {
   for (unsigned u1=0; u1<V.returnInsts.size(); u1++) {
      if (V.returnInsts[u1]->needToWalkStack())
	 return true;
   }
   return false;
}

bool instrCodeNode::hookupJumpsToBaseTramps(vector<Address>& pc) {
   if(baseTrampsHookedUp()) return true;

  vector<returnInstance *> &returnInsts = V.getReturnInsts();
   unsigned rsize = returnInsts.size();
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
      bool installSafe = returnInsts[u] -> checkReturnInstance(pc,index);
#endif
      // if unsafe, index will be set to the first unsafe stack walk ndx
      // (0 being top of stack; i.e. the current pc)
      
      if (!installSafe && index > max_index)
	 max_index = index;
      
      if (installSafe) {
	 returnInsts[u] -> installReturnInstance(proc());
	 delay_elm[u] = false;
      } else {
	 delay_install = true;
	 delay_elm[u] = true;
      }
   }
   
   if (delay_install) {
      // get rid of pathological cases...caused by threaded applications 
      // TODO: this should be fixed to do something smarter
      if(max_index > 0 && max_index+1 >= pc.size()){
	 max_index--;
	//printf("max_index changed: %d\n",max_index);
      }
      if(max_index > 0 && pc[max_index+1] == 0){
	 max_index--;
	 //printf("max_index changed: %d\n",max_index);
      }
      Address pc2 = pc[max_index+1];
      for (u_int i=0; i < rsize; i++) {
	 if (delay_elm[i]) {
	    returnInsts[i]->addToReturnWaitingList(pc2, proc());
	 }
      }
   }
   markBaseTrampsAsHookedUp();
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
   for(unsigned i=0; i<V.dataNodes.size(); i++)
      cerr << "ST:" << (void*)V.dataNodes[i] << "\n";
}

instrThrDataNode *instrCodeNode::getThrDataNode(const string &tname) {
  bool findCollectThread = false;

  if(tname == collectThreadName)  findCollectThread = true;
  else                            findCollectThread = false;
  
#if defined(MT_THREAD)
  unsigned numThrNames = V.thr_names.size();
#endif
  for (unsigned i=0; i<V.dataNodes.size(); i++) {
    if(findCollectThread) {
      return V.dataNodes[i];
    } else {
#if defined(MT_THREAD)
      if(i > (numThrNames-1))  break;
      if(V.thr_names[i] == tname) {
	return V.dataNodes[i];
      }
#endif
    }
  }
  return NULL;
}

instrThrDataNode *instrCodeNode::getThrDataNode(int thr_id) {
  for(unsigned i=0; i<V.dataNodes.size(); i++) {
    instrThrDataNode *curThr = V.dataNodes[i];
    if(curThr->getThreadID() == thr_id)
      return curThr;
  }
  // not found
  return NULL;
}

const instrThrDataNode *instrCodeNode::getThrDataNode(int thr_id) const {
  for(unsigned i=0; i<V.dataNodes.size(); i++) {
    const instrThrDataNode *curThr = V.dataNodes[i];
    if(curThr->getThreadID() == thr_id)
      return curThr;
  }
  // not found
  return NULL;
}

// Check if "mn" and "this" correspond to the same instrumentation?
bool instrCodeNode::condMatch(instrCodeNode *mn,
				   vector<dataReqNode*> &data_tuple1,
				   vector<dataReqNode*> &data_tuple2) {
  vector<dataReqNode *> this_datanodes = getDataRequests();
  vector<dataReqNode *> other_datanodes = mn->getDataRequests();

  vector<instReqNode> this_instRequests = V.getInstRequests();
  vector<instReqNode> other_instRequests = mn->getInstRequests();  

  // Both "this" metricDefinitionNode and the passed in metricDefinitionNode
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
const dataReqNode* instrCodeNode::getFlagDRN(int thrID) const { 
  const instrThrDataNode *curDataNode = getThrDataNode(thrID); 
  const dataReqNode *retDRN = NULL;
  if(curDataNode) {
    retDRN = curDataNode->getConstraintDRN();
  }
  return retDRN;
}

void instrCodeNode::addInst(instPoint *point, AstNode *ast,
				 callWhen when, callOrder o)
{
  if (!point) return;

  instReqNode temp(point, ast, when, o);
  V.instRequests += temp;
}

vector<dataReqNode *> instrCodeNode::getDataRequests()
{
  vector<dataReqNode *> curDataRequests;
  for(unsigned i=0; i<V.dataNodes.size(); i++) {
    curDataRequests += V.dataNodes[i]->getDataRequests();
  }
  return curDataRequests;
}


