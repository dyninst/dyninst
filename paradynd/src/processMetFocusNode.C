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

#include "paradynd/src/machineMetFocusNode.h"
#include "paradynd/src/processMetFocusNode.h"
#include "paradynd/src/threadMetFocusNode.h"
#include "paradynd/src/instrCodeNode.h"
#include "paradynd/src/instrDataNode.h"
#include "paradynd/src/dynrpc.h"
#include "dyninstAPI/src/process.h"
#include "pdutil/h/pdDebugOstream.h"
#include "dyninstAPI/src/pdThread.h"
#include "paradynd/src/init.h"

extern pdDebug_ostream sampleVal_cerr;

vector<processMetFocusNode*> processMetFocusNode::allProcNodes;

processMetFocusNode::processMetFocusNode(process *p,
                       const string &metname, const Focus &focus_,
		       aggregateOp agg_op, bool arg_dontInsertData)
  : aggregator(agg_op, getCurrSamplingRate()),
    parentNode(NULL), aggInfo(NULL), proc_(p), aggOp(agg_op), 
    metric_name(metname), focus(focus_), 
    dontInsertData_(arg_dontInsertData), catchupNotDoneYet_(false),
    currentlyPaused(false), insertionAttempts(0), function_not_inserted(NULL),
    isBeingDeleted_(false)
{
  allProcNodes.push_back(this);
}

processMetFocusNode::processMetFocusNode(const processMetFocusNode &par, 
					 process *childProc) :
  aggregator(par.aggOp, getCurrSamplingRate()), // start with fresh aggregator
  // set by recordAsParent(), in propagateToForkedProcess
  parentNode(NULL), aggInfo(NULL), proc_(childProc),
  metric_name(par.metric_name), 
  focus(adjustFocusForPid(par.focus, childProc->getPid()))
{
  metricVarCodeNode = instrCodeNode::copyInstrCodeNode(*par.metricVarCodeNode,
						       childProc);
  for(unsigned i=0; i<par.constraintCodeNodes.size(); i++) {
    constraintCodeNodes.push_back(
     instrCodeNode::copyInstrCodeNode(*par.constraintCodeNodes[i], childProc));
  }

  // I need to determine which threads are being duplicated in the child
  // process.  Under pthreads, my understanding is that only the thread
  // for which the fork call occurs is duplicated in the child.  Under
  // Solaris threads, my understanding is that all threads are duplicated.
  for(unsigned j=0; j<par.thrNodes.size(); j++) {
    assert(par.thrNodes.size() <= childProc->threads.size());
    pdThread *childThr = childProc->getThread(par.thrNodes[j]->getThreadID());
    threadMetFocusNode *newThrNode = 
      threadMetFocusNode::copyThreadMetFocusNode(*par.thrNodes[j], childProc,
						 childThr);
    addPart(newThrNode);
  }

  procStartTime = par.procStartTime;

  aggOp = par.aggOp;
  procStartTime = par.procStartTime;
  
  dontInsertData_ = par.dontInsertData_;
  catchupNotDoneYet_ = par.catchupNotDoneYet_;
  currentlyPaused = par.currentlyPaused;
  
  catchupASTList = par.catchupASTList;
  sideEffectFrameList = par.sideEffectFrameList;
  
  insertionAttempts = par.insertionAttempts;
  function_not_inserted = par.function_not_inserted;
}

void processMetFocusNode::getProcNodes(vector<processMetFocusNode*> *procnodes)
{
  for(unsigned i=0; i<allProcNodes.size(); i++) {
    (*procnodes).push_back(allProcNodes[i]);
  }
}

// optimize this if helpful
void processMetFocusNode::getProcNodes(vector<processMetFocusNode*> *procnodes,
				       int pid)
{
  for(unsigned i=0; i<allProcNodes.size(); i++) {
    processMetFocusNode *curNode = allProcNodes[i];
    if(curNode->proc()->getPid() == pid)
      (*procnodes).push_back(curNode);
  }
}

// optimize this if helpful
void processMetFocusNode::getMT_ProcNodes(
				    vector<processMetFocusNode*> *MT_procnodes)
{
   for(unsigned i=0; i<allProcNodes.size(); i++) {
      processMetFocusNode *curNode = allProcNodes[i];
      if(curNode->proc()->is_multithreaded()) {
	 (*MT_procnodes).push_back(curNode);
      }
   }
}

bool processMetFocusNode::instrLoaded() {
  bool allCompInserted = true;

  vector<instrCodeNode *> codeNodes;
  getAllCodeNodes(&codeNodes);
  if(codeNodes.size() == 0) {
    allCompInserted = false;
  } else {
    for(unsigned i=0; i<codeNodes.size(); i++) {
      instrCodeNode *codenode = codeNodes[i];
      bool result = codenode->instrLoaded();
      if(result == false) {
	allCompInserted = false; 
	break;
      }
    }
  }
  return allCompInserted;
}

bool processMetFocusNode::trampsHookedUp() {
  vector<instrCodeNode *> codeNodes;
  getAllCodeNodes(&codeNodes);

  if(codeNodes.size() == 0)  return false;

  bool hookedUp = true;
  for(unsigned i=0; i<codeNodes.size(); i++) {
    instrCodeNode *codeNode = codeNodes[i];
    if(codeNode->trampsHookedUp() == false) {
      hookedUp = false;
      break;
    }
  }
  return hookedUp;
}

void processMetFocusNode::tryAggregation() {
  sampleInterval aggSample;
  while(aggregator.aggregate(&aggSample) == true) {
    updateWithDeltaValue(aggSample.start, aggSample.end, aggSample.value);
  }
}

// will adjust the value of the sample if the referred to mdn wasn't active
// wholly during the length of the sample
// returns false if this sample should not be "added" to the mdn

// here's an example that shows why this function is relevant:
//
//      18s992ms / 19s192ms     AGG     AGG'  (AGG'-starttime: 19s387ms)
//   (AGG-starttime: 19s592ms)   |    /  |
//                               |  /    |
//      18s992ms / 19s192ms     COMP    COMP'
//                               |       |
//                               |       |
//      un / 19s761ms           PRIM    PRIM'
//
//   AGG is a cpu/process metric-focus, AGG' is a cpu/wholeprog metric-focus
//   added later.  The time before the '/' represents the start-time of the
//   sample, the time after the '/' represents the sample-time (or end-time)
//   of the sample.  The "AGG-starttime:" represents the start time of the
//   mdn set according to mdnContinue.
//   In the above example, the sample coming out of the COMP's aggregator
//   did not occur while the AGG' was active, so this sample should be
//   skipped or not added to AGG'.

bool adjustSampleIfNewMdn(pdSample *sampleVal, timeStamp startTime,
			  timeStamp sampleTime, timeStamp MFstartTime) {
  if(! MFstartTime.isInitialized()) {
    sampleVal_cerr << "  skipping sample since startTime- is uninitialized\n";
    return false;
  }
  
  // the sample did not occur while (but before) the mdn was active
  if(sampleTime < MFstartTime) {
    sampleVal_cerr << "  skipping sample since sample did not occur while"
		   << " the mdn was active\n";
    return false;
  }
  
  // interpolate the value of the sample for the time that the mdn was 
  // active during the length of the sample
  if(!startTime.isInitialized() || startTime >= MFstartTime) {
    // the whole sample occurred while the mdn was active
    // keep the sampleVal just as it is
  } else if(startTime < MFstartTime) {
    // the sample partially occurred while the mdn was active
    // ie. startTime < MFstartTime < sampleTime
    // we want to determine the amount of the sample within the interval
      // that this new mdn was active
    timeLength timeActiveForNewMdn = sampleTime - MFstartTime;
    timeLength sampleLen = sampleTime - startTime;
    assert(sampleLen >= timeActiveForNewMdn);
    double pcntIntvlActiveForNewMdn = timeActiveForNewMdn / sampleLen;
    *sampleVal = pcntIntvlActiveForNewMdn * (*sampleVal);
    sampleVal_cerr << "  using " << pcntIntvlActiveForNewMdn
		   << ", new val = " << *sampleVal << "\n";
  }
  
  return true;
}

void processMetFocusNode::updateWithDeltaValue(timeStamp startTime,
			 timeStamp sampleTime, pdSample value) {
  pdSample valToPass = value;
  bool shouldAddSample = adjustSampleIfNewMdn(&valToPass, startTime, 
				  sampleTime, parentNode->getStartTime());
  if(!shouldAddSample)   return;
  
  assert(aggInfo->isReadyToReceiveSamples());
  assert(sampleTime >= aggInfo->getInitialStartTime());

  sampleVal_cerr << "procNode: " << this << ", updateWithDeltaValue, time: "
		 << sampleTime << ", val: " << valToPass << "\n";

  aggInfo->addSamplePt(sampleTime, valToPass);
  
  parentNode->tryAggregation();
}

processMetFocusNode *processMetFocusNode::newProcessMetFocusNode(
		       process *p, const string &metname,
		       const Focus &component_foc,
		       aggregateOp agg_op, bool arg_dontInsertData)
{
  processMetFocusNode *procNode = new processMetFocusNode(p, metname, 
				    component_foc, agg_op, arg_dontInsertData);
  return procNode;
}

/* This only called by MDN of type COMP_MDN, it meddles with too many
   members of MDN to make it a member of processMetFocusNode
*/
processMetFocusNode* processMetFocusNode::handleExec() {
   // called by handleExec(), below.  See that routine for documentation.
   // "this" is a component mi.

   // If this component mi can be (re-)enabled in the new (post-exec)
   // process, then do so.  Else, remove the component mi from aggregators,
   // etc.  Returns new component mi if successful, NULL otherwise.

   // How can we tell if the mi can be inserted into the "new" (post-exec)
   // process?  A component mi is basically a set of instReqNodes and
   // dataReqNodes.  The latter don't restrict what can be inserted (is this
   // right?); the instReqNodes hold the key -- we should look at the
   // functions (instPoint's) where code (whose contents are in AstNode's)
   // would be inserted.  Now clearly, the instPoint's must be checked -- if
   // any one doesn't exist, then the instReqNode and hence the component mi
   // doesn't belong in the post-exec process.  But what about the AstNode's?
   // Should the code that gets inserted be subject to a similar test?
   // Probably, but we currently don't do it.

   // BUT: Even if a process contains a function in both the pre-exec and
   // post-exec stages, we must assume that the function is IN A DIFFERENT
   // LOCATION IN THE ADDRESS SPACE.  Ick.  So the instPoint's can't be
   // trusted and must be recalculated from scratch.  In that regard, this
   // routine is similar to propagateToNewProcess(), which propagates
   // aggregate mi's to a brand new process (but which doesn't work for
   // processes started via fork or exec).  The lesson learned is to (ick,
   // ick, ick) call mdl_do() all over again.  This gets really confusing
   // when you consider that a component mi can belong to several aggregate
   // mi's (e.g. if we represent cpu time for proc 100 then we can belong to
   // cpu/whole and cpu/proc-100); for which aggregate mi should we run
   // mdl_do?  Any will do, so we can pick arbitrarily (is this right?).

   // QUESTION: What about internal or cost metrics???  They have aggregate
   // and component mi's just like normal metrics, right?  If that's so, then
   // they must be propagated too!  NOT YET IMPLEMENTED!!!
  processMetFocusNode *procNode = NULL;  // just while commented out
  /* // needs to be reimplemented
#if defined(MT_THREAD)
   machineMetFocusNode *machnode = NULL;
   
   for (unsigned u=0; u<aggregators.size(); u++)
      if (aggregators[u]->getMdnType() == AGG_MDN) {
	 machnode = dynamic_cast<machineMetFocusNode*>(aggregators[u]);
	 break;
      }
   
   if (!machnode)                                        // abort if all aggregators are thr_lev's
      return NULL;                                           // could replic thr_lev's agg, not for now
#else
   machineMetFocusNode *machnode = dynamic_cast<machineMetFocusNode*>(
                                                 this->aggregators[0]);
#endif

   const bool internal = !mdl_can_do(const_cast<string&>(getMetName()));
   if (internal)
      return NULL; // NOT YET IMPLEMENTED
   
   // try to propagate the mi note: the following code is mostly stolen from
   // propagateToNewProcess(); blame it for any bugs :)
   
   // Make the unique ID for this metric/focus visible in MDL. (?)
   string vname = "$globalId";
   mdl_env::add(vname, false, MDL_T_INT);
   mdl_env::set(machnode->getMetricID(), vname);
   
   vector<process*> vp(1, this->proc());
   vector< vector<pdThread *> > threadsVec;
#if defined(MT_THREAD)
   threadsVec += this->proc()->threads;
#endif
   cerr << "mdl_do - C\n";
   machineMetFocusNode *tempMachNode = mdl_do(//mid
              const_cast<vector< vector<string> > &>(machnode->getFocus()),
	                      const_cast<string &>(machnode->getMetName()),
	                      const_cast<string &>(machnode->getFullName()),
	                      vp, 
                              true, // fry existing component MI
	                      false);
   if (tempMachNode == NULL)
      return NULL; // failure
   
   assert(tempMachNode->isTopLevelMDN());

   // okay, it looks like we successfully created a new aggregate mi.
   // Of course, we're just interested in the (single) component mi contained
   // within it; it'll replace "this".
   
   vector<metricFocusNode *> comp = tempMachNode->getComponents();
   assert(comp.size() == 1);
   processMetFocusNode *procNode = 
      dynamic_cast<processMetFocusNode*>(comp[0]);
   
#if defined(MT_THREAD)
   unsigned size = procNode->aggregators.size();
   procNode->aggregators.resize(size-1);
   procNode->samples.resize(size-1);
   procNode->comp_flat_names.resize(size-1);
#else
   procNode->aggregators.resize(0);
   procNode->samples.resize(0);
#endif
   
   // For each aggregator, go back and find where "this" was a component mi.
   // When found, replace the ptr to "this" with "procNode".
   unsigned num_aggregators = aggregators.size();
   assert(num_aggregators > 0);
   for (unsigned agglcv=0; agglcv < num_aggregators; agglcv++) {
      metricFocusNode *aggMI = aggregators[agglcv];
      
#if defined(MT_THREAD)
      if (THR_LEV == aggregators[agglcv]->getMdnType())
	 continue;
#endif
      
      bool found=false;
      vector<metricFocusNode *> aggComp = aggMI->getComponents();
      for (unsigned complcv=0; complcv < aggComp.size(); complcv++) {
	 if (aggComp[complcv] == this) {
	    aggComp[complcv] = 
	       static_cast<metricFocusNode*>(procNode);
	    
	    procNode->aggregators += aggMI;
	    procNode->samples     += aggMI->getAggregator().newComponent();
#if defined(MT_THREAD)
	    procNode->comp_flat_names += comp_flat_names[agglcv];
#endif
	    
	    this->samples[agglcv]->requestRemove();
	    
	    found=true;
	    break;
	 }
      }
      assert(found);
   }
   
   // Now let's actually insert the instrumentation:
   if (!internal) {
      // dummy parameters for loadInstrIntoApp 
      pd_Function *func = NULL;
      procNode->loadInstrIntoApp(&func);
      procNode->insertJumpsToTramps();
   }
   
   // And fry "tempMachNode", but make sure "procNode" isn't fried when we
   // do so
   tempMachNode->removeAllComponents();
   delete tempMachNode; // good riddance; you were an ugly hack to begin with
  */   
   
   return procNode;
}

int processMetFocusNode::getMetricID() { 
  return parentNode->getMetricID();
}

const int MAX_INSERTION_ATTEMPTS_USING_RELOCATION = 1000;
vector<int> deferredMetricIDs;

void registerAsDeferred(int metricID) {
   for(unsigned i=0; i<deferredMetricIDs.size(); i++) {
      if(metricID == deferredMetricIDs[i]) {
	 return;
      }
   }
   deferredMetricIDs.push_back(metricID);
}

instr_insert_result_t processMetFocusNode::insertInstrumentation() {
   if(instrInserted()) {
      return insert_success;
   }
   ++insertionAttempts;
   if(insertionAttempts == MAX_INSERTION_ATTEMPTS_USING_RELOCATION) {
      if(function_not_inserted != NULL)
	 function_not_inserted->setRelocatable(false);
   }

   pauseProcess();

   instr_insert_result_t insert_status = 
      loadInstrIntoApp(&function_not_inserted);
   
   // Instrumentation insertion may have failed. We should handle this case
   // better than an assert. Currently we handle the case where (on x86) we
   // couldn't insert the instrumentation due to relocation failure (deferred
   // instrumentation). If it's deferred, come back later.
  
   if(insert_status == insert_deferred) {
      continueProcess();
      if(hasDeferredInstr()) {
	 registerAsDeferred(getMetricID());
	 return insert_deferred;
      }
   } else if(insert_status == insert_failure) {
      continueProcess();
      string msg = string("Unable to load instrumentation for metric focus ")
			  + getFullName() + " into process with pid " 
			  + string(proc()->getPid());
      showErrorCallback(126, msg);
      return insert_failure;
   }

   insertJumpsToTramps();

   // Now that the timers and counters have been allocated on the heap, and
   // the instrumentation added, we can manually execute instrumentation we
   // may have processed at function entry points and pre-instruction call
   // sites which have already executed.
   doCatchupInstrumentation();

   continueProcess();
   return insert_success;
}

//
// Added to allow metrics affecting a function F to be triggered when
//  a program is executing (in a stack frame) under F:
// The basic idea here is similar to Ari's(?) hack to start "whole 
//  program" metrics requested while the program is executing 
//  (see T_dyninstRPC::mdl_instr_stmt::apply in mdl.C).  However,
//  in this case, the manuallyTrigger flag is set based on the 
//  program stack (the call sequence implied by the sequence of PCs
//  in the program stack), and the types of the set of inst points 
//  which the metricFocusNode corresponds to, as opposed to
//  a hacked interpretation of the original MDL statement syntax.
// The basic algorithm is as follows:
//  1. Construct function call sequence leading to the current 
//     stack frame (yields vector of pf_Function hopefully equivalent
//     to yield of "backtrace" functionality in gdb.
//  2. Look at each instReqNode in *this (call it node n):
//     Does n correspond to a function currently on the stack?
//       No -> don't manually trigger n's instrumentation.
//       Yes ->
//         

void processMetFocusNode::doCatchupInstrumentation() {
  if(! catchupNotDoneYet()) {
    return;
  }

  prepareCatchupInstr();

  postCatchupRPCs();

  void checkProcStatus();
  
  // Get them all cleared out
  do {
    bool result = proc_->launchRPCifAppropriate(false, false);
    checkProcStatus();
  } while (proc_->existsRPCreadyToLaunch() ||
	   proc_->existsRPCinProgress());
  catchupNotDoneYet_ = false;
}

bool processMetFocusNode::catchupInstrNeeded() const {
  assert(0 && "Not implemented");
  return false;
}

//
// Added to allow metrics affecting a function F to be triggered when
//  a program is executing (in a stack frame) under F:
// The basic idea here is similar to Ari's(?) hack to start "whole 
//  program" metrics requested while the program is executing 
//  (see T_dyninstRPC::mdl_instr_stmt::apply in mdl.C).  However,
//  in this case, the manuallyTrigger flag is set based on the 
//  program stack (the call sequence implied by the sequence of PCs
//  in the program stack), and the types of the set of inst points 
//  which the metricFocusNode corresponds to, as opposed to
//  a hacked interpretation of the original MDL statement syntax.
// The basic algorithm is as follows:
//  1. Construct function call sequence leading to the current 
//     stack frame (yields vector of pf_Function hopefully equivalent
//     to yield of "backtrace" functionality in gdb.
//  2. Look at each instReqNode in *this (call it node n):
//     Does n correspond to a function currently on the stack?
//       No -> don't manually trigger n's instrumentation.
//       Yes ->
//         

void processMetFocusNode::prepareCatchupInstr() {

  vector< vector<Frame> > allStackWalks;

#if defined(MT_THREAD)
  proc()->walkAllStack(allStackWalks);
  
#else
  // Fake multithread stack walk
  vector<Frame> stackWalk;
  proc()->walkStack(proc()->getActiveFrame(), stackWalk);
  allStackWalks.push_back(stackWalk);
#endif

  assert(catchupASTList.size() == 0); // Make sure there's no catchup sitting around

  // Convert the stack walks into a similar list of catchupReq nodes,
  // which maps 1:1 onto the stack walk and includes a vector of instReqNodes
  // that need to be executed

  // 2d vector -- first by thread, then by individual stack frame
  vector<vector<catchupReq *> > catchupStackWalks;
  for (unsigned i = 0; i < allStackWalks.size(); i++) {
    vector<catchupReq *> catchupWalk;
    for (unsigned j = 0; j < allStackWalks[i].size(); j++)
      catchupWalk.push_back(new catchupReq(allStackWalks[i][j]));
    catchupStackWalks.push_back(catchupWalk);
  }

  // Now go through all associated code nodes, and add appropriate bits
  // to the catchup request list.
  for (unsigned cIter = 0; cIter < constraintCodeNodes.size(); cIter++)
    if (!constraintCodeNodes[cIter]->hasBeenCatchuped()) {
      constraintCodeNodes[cIter]->prepareCatchupInstr(catchupStackWalks);
    }

  // Get the metric code node catchup list
  metricVarCodeNode->prepareCatchupInstr(catchupStackWalks);

  // Note: stacks are delivered to us in bottom-up order (most recent to
  // main()), and we want to execute from main down. So loop through
  // backwards and add to the main list. We can go one thread at a time,
  // because multiple threads will not interfere with each other.

  // Iterate through all threads
  //  cerr << "Looping through " << catchupStackWalks.size() << " stack walks" << endl;
  for (unsigned i1 = 0; i1 < catchupStackWalks.size(); i1++) {
    // Then through each frame in the stack walk
    AstNode *conglomerate = NULL;
    for (int j1 = catchupStackWalks[i1].size()-1;
	 j1 >= 0; j1--) { // Note: backwards iteration
      //      cerr << "Frame " <<i1<<"/"<<j1 << " "  << catchupStackWalks[i1][j1]->frame << endl;
      if (((catchupStackWalks[i1])[j1]->reqNodes).size()) { // Means we have a catchup request
	// What we want to do: build a single big AST then
	// launch it as an RPC. 
	for (unsigned k1 = 0; k1<((catchupStackWalks[i1])[j1]->reqNodes).size();
	     k1++) {
	  AstNode *AST = catchupStackWalks[i1][j1]->reqNodes[k1]->Ast();
	  if (!conglomerate) {
	    conglomerate = AST;
	  }
	  else { // Need to combine with a SequenceNode
	    AstNode *old = conglomerate;
	    conglomerate = new AstNode(conglomerate, AST);
	    removeAst(old);
	  }
	}
	sideEffect_t side;
	side.frame = catchupStackWalks[i1][j1]->frame;
	side.reqNode = catchupStackWalks[i1][j1]->reqNodes[0];
	sideEffectFrameList.push_back(side);
      }
    } // Inner loop
    if (conglomerate) {
      catchup_t catchup;
      catchup.ast = conglomerate;
      catchup.thread = catchupStackWalks[i1][0]->frame.getThread();
      catchup.lwp = catchupStackWalks[i1][0]->frame.getLWP();
      catchupASTList.push_back(catchup);
    }
  }
}

void processMetFocusNode::postCatchupRPCs()
{
  // Assume the list of catchup requests is 
  // sorted

  if (pd_debug_catchup) {
    cerr << "Posting " << catchupASTList.size() << " catchup requests\n";
    cerr << "Handing " << sideEffectFrameList.size() << " side effects\n";
  }

  for (unsigned i=0; i < catchupASTList.size(); i++) {
    proc_->postRPCtoDo(catchupASTList[i].ast, false,
		       NULL, NULL,
		       getMetricID(),
		       catchupASTList[i].thread,
		       catchupASTList[i].lwp,
		       false);
  }
  catchupASTList.resize(0);
  for (unsigned j = 0; j < sideEffectFrameList.size(); j++) {
    proc_->catchupSideEffect(sideEffectFrameList[j].frame, 
			     sideEffectFrameList[j].reqNode);
  }
  sideEffectFrameList.resize(0);
}

void processMetFocusNode::initializeForSampling(timeStamp startTime, 
						pdSample initValue)
{
  initAggInfoObjects(startTime, initValue);
  prepareForSampling();  
}

void processMetFocusNode::initAggInfoObjects(timeStamp startTime, 
					     pdSample initValue)
{
  //cerr << "  procNode (" << (void*)this << ") initAggInfo\n";
  // Initialize aggComponent between [parent MACHnode] <-> [this PROCnode]

  // some processMetFocusNodes might be shared and thus already initialized
  // don't reinitialize these
  // eg. already exist:  MACH-WHOLEPROG -> PROC1, PROC2, PROC3
  //     new request:    MACH-PROC1     -> PROC1
  // in the case above, PROC1 already exists and it's aggInfo is already
  // initialized
  if(! aggInfo->isInitialized()) {
    //cerr << "    initializing aggInfo " << (void*)aggInfo << "\n";
    aggInfo->setInitialStartTime(startTime);
    aggInfo->setInitialActualValue(initValue);
  }

  procStartTime = startTime;

  // Initialize aggComponent between [this PROCnode] <-> [THRnodes]
  for(unsigned j=0; j<thrNodes.size(); j++) {
    thrNodes[j]->initAggInfoObjects(startTime, initValue);
  }
}

instr_insert_result_t processMetFocusNode::loadInstrIntoApp(pd_Function **func)
{
   if(instrLoaded()) {
      return insert_success;
   }

   vector<instrCodeNode *> codeNodes;
   getAllCodeNodes(&codeNodes);

   // mark remaining prim. components as deferred if we come upon
   // one deferred component
   for (unsigned j=0; j<codeNodes.size(); j++) {
      instrCodeNode *codeNode = codeNodes[j];
      instr_insert_result_t status = codeNode->loadInstrIntoApp(func);
      if(status != insert_success) {
	 return status;
      }
   }

   catchupNotDoneYet_ = true;
   
   return insert_success;
}

void processMetFocusNode::prepareForSampling() {
  metricVarCodeNode->prepareForSampling(thrNodes);
}

void processMetFocusNode::prepareForSampling(threadMetFocusNode *thrNode)
{
  metricVarCodeNode->prepareForSampling(thrNode);
}

void processMetFocusNode::stopSamplingThr(threadMetFocusNode_Val *thrNodeVal) {
  if(dontInsertData()) return;

  assert(metricVarCodeNode != NULL);
  metricVarCodeNode->stopSamplingThr(thrNodeVal);
}

vector<const instrDataNode*> processMetFocusNode::getFlagDataNodes() const {
  vector<const instrDataNode*> buff;
  for(unsigned i=0; i<constraintCodeNodes.size(); i++) {
    const instrDataNode *dn = constraintCodeNodes[i]->getFlagDataNode();
    buff.push_back(dn);
  }
  return buff;
}

bool processMetFocusNode::needToWalkStack() {
   bool anyNeeded = false;

   vector<instrCodeNode *> codeNodes;
   getAllCodeNodes(&codeNodes);

   for(unsigned i=0; i<codeNodes.size(); i++) {
      if(codeNodes[i]->needToWalkStack())  anyNeeded = true;
   }
   return anyNeeded;
}

// Patch up the application to make it jump to the base trampoline(s) of this
// metric.  (The base trampoline and mini-tramps have already been installed
// in the inferior heap).  We must first check to see if it's safe to install
// by doing a stack walk, and determining if anything on it overlaps with any
// of our desired jumps to base tramps.  The key variable is "returnsInsts",
// which was created for us when the base tramp(s) were created.
// Essentially, it contains the details of how we'll jump to the base tramp
// (where in the code to patch, how many instructions, the instructions
// themselves).  Note that it seems this routine is misnamed: it's not
// instrumentation that needs to be installed (the base & mini tramps are
// already in place); it's just the last step that is still needed: the jump
// to the base tramp.  If one or more can't be added, then a TRAP insn is
// inserted in the closest common safe return point along the stack walk, and
// some structures are appended to the process' "wait list", which is then
// checked when a TRAP signal arrives.  At that time, the jump to the base
// tramp is finally done.  WARNING: It seems to me that such code isn't
// thread-safe...just because one thread hits the TRAP, there may still be
// other threads that are unsafe.  It seems to me that we should be doing
// this check again when a TRAP arrives...but for each thread (right now,
// there's no stack walk for other threads).  --ari
bool processMetFocusNode::insertJumpsToTramps() {
   if(trampsHookedUp()) {
      return true;
   }
   // pause once for all primitives for this component
   
   // only overwrite 1 instruction on power arch (2 on mips arch)
   // always safe to instrument without stack walk
   bool allInserted = true;

   vector<instrCodeNode *> codeNodes;
   getAllCodeNodes(&codeNodes);

   if (!needToWalkStack()) {
      // NO stack walk necessary
     vector<Frame> stackWalk;
     for (unsigned u=0; u<codeNodes.size(); u++) {
       instrCodeNode *codeNode = codeNodes[u];
       bool result = codeNode->insertJumpsToTramps(stackWalk);
       if(result == false)
	 allInserted = false;
     }
   }
   else {
      // stack walk necessary, do stack walk only ONCE for all primitives
      // NOTE: walkStack should walk all the threads' staks! It doesn't do
      // that right now... naim 1/28/98

      // The curr_lwp parameter is IGNORED on non-AIX platforms.
      Frame currentFrame = proc()->getActiveFrame();
      vector<Frame> stackWalk;
      proc()->walkStack(currentFrame, stackWalk);

      // ndx 0 is where the pc is now; ndx 1 is the call site;
      // ndx 2 is the call site's call site, etc...
	 
      for (unsigned u2=0; u2<codeNodes.size(); u2++) {
	 instrCodeNode *codeNode = codeNodes[u2];
	 bool result = codeNode->insertJumpsToTramps(stackWalk);
	 if(result == false)
	    allInserted = false;
      }
   } // else of needToWalkStack();

   return allInserted;
}


timeLength processMetFocusNode::cost() const {
   timeLength totCost = timeLength::Zero();   

   vector<const instrCodeNode *> codeNodes;
   getAllCodeNodes(&codeNodes);

   for(unsigned i=0; i<codeNodes.size(); i++) {
      timeLength curCost = codeNodes[i]->cost();
      totCost += curCost;
   }
   return totCost;
}


void processMetFocusNode::print() {
   cerr << "P:" << (void*)this << "\n";
   for(unsigned i=0; i<thrNodes.size(); i++)
      thrNodes[i]->print();
   cerr << "mfinstr nodes\n";

   vector<instrCodeNode *> codeNodes;
   getAllCodeNodes(&codeNodes);

   for(unsigned j=0; j<codeNodes.size(); j++)
      codeNodes[j]->print();
}


processMetFocusNode::~processMetFocusNode() {
  isBeingDeleted_ = true;
  // NULL possible if processMetFocusNode created for fork and then deleted
  // right away (ie. unforked) because not needed.  See
  // machineMetFocusNode::setupProcNodeForForkedProcess.
  if(aggInfo!=NULL)  
    aggInfo->markAsFinished();

  // thrNode destruction needs to occur before the destruction of the
  // codeNodes, since this will turn off sampling in the data nodes,
  // which can be deleted when the code nodes are deleted
  for(unsigned i=0; i<thrNodes.size(); i++)
    delete thrNodes[i];

  delete metricVarCodeNode;
  metricVarCodeNode = NULL;
  for(unsigned j=0; j<constraintCodeNodes.size(); j++) {
    delete constraintCodeNodes[j];
  }

  for(int k=(int)allProcNodes.size()-1; k>=0; k--) {
    if(allProcNodes[k] == this)  allProcNodes.erase(k);
  }
}

void processMetFocusNode::pauseProcess() {
  if(currentlyPaused == true)  return;

  if (proc()->status() == running) {
#ifdef DETACH_ON_THE_FLY
    if (proc()->reattachAndPause())
#else
    if (proc()->pause())
#endif
    {
      currentlyPaused = true;
    }
  }
}

void processMetFocusNode::continueProcess() {
  if(currentlyPaused) {
#ifdef DETACH_ON_THE_FLY
      proc()->detachAndContinue();
#else
      proc()->continueProc();
#endif
      currentlyPaused = false;
   }
}

bool processMetFocusNode::hasDeferredInstr() {
  bool hasDeferredComp = false;

  vector<instrCodeNode *> codeNodes;
  getAllCodeNodes(&codeNodes);

  for(unsigned i=0; i<codeNodes.size(); i++) {
    if(codeNodes[i]->hasDeferredInstr()) {
      hasDeferredComp = true;
      break;
    }
  }
  return hasDeferredComp;
}

void processMetFocusNode::recordAsParent(machineMetFocusNode *machNode,
					 aggComponent *aggInfo_) {
  assert(parentNode==NULL && aggInfo==NULL);
  parentNode = machNode;
  aggInfo = aggInfo_;
}

// thrNode's parent is recorded in thrNode during construction of thrNode
void processMetFocusNode::addPart(threadMetFocusNode* thrNode)
{
  thrNodes.push_back(thrNode);
  aggComponent *childAggInfo = aggregator.newComponent();
  thrNode->recordAsParent(this, childAggInfo);
}

void processMetFocusNode::setMetricVarCodeNode(instrCodeNode* part) {
  metricVarCodeNode = part;
}

void processMetFocusNode::addConstraintCodeNode(instrCodeNode* part) {
  constraintCodeNodes.push_back(part);
}

void processMetFocusNode::propagateToNewThread(pdThread *thr)
{
   // we only want to sample this new thread if the selected focus for this
   // processMetFocusNode was for the whole process.  If it's not a thread
   // specific focus, then the focus includes the whole process (ie. all of
   // the threads)
   if(getFocus().thread_defined()) return;

   string thrName = string("thr_") + string(thr->get_tid()) + "{" + 
                      thr->get_start_func()->prettyName() + "}";
   Focus focus_with_thr = getFocus();
   focus_with_thr.set_thread(thrName);
   threadMetFocusNode *thrNode = threadMetFocusNode::
      newThreadMetFocusNode(metric_name, focus_with_thr, thr);
   addPart(thrNode);
   
   thrNode->initializeForSampling(getWallTime(), pdSample::Zero());
}

void processMetFocusNode::deleteThread(pdThread * /*thr*/)
{
   // need to implement
}

void processMetFocusNode::unFork() {
  delete this;  // will remove instrumentation and data for the procMetFocusNode
                // (the metFocusNode deletion code deals with code and data
                //  sharing appropriately)
}

