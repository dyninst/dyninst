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
#include "dyninstAPI/src/dyn_lwp.h"
#include "pdutil/h/pdDebugOstream.h"
#include "paradynd/src/init.h"
#include "paradynd/src/pd_process.h"
#include "paradynd/src/pd_thread.h"

void checkProcStatus();  
  
extern unsigned enable_pd_samplevalue_debug;

#if ENABLE_DEBUG_CERR == 1
#define sampleVal_cerr if (enable_pd_samplevalue_debug) cerr
#else
#define sampleVal_cerr if (0) cerr
#endif /* ENABLE_DEBUG_CERR == 1 */

inline unsigned ui_hash_(const unsigned &u) { return u; }

processMetFocusNode::processMetFocusNode(pd_process *p,
                       const string &metname, const Focus &focus_,
		       aggregateOp agg_op, bool arg_dontInsertData)
  : metricVarCodeNode(NULL), aggregator(agg_op, getCurrSamplingRate()),
    parentNode(NULL), aggInfo(NULL), proc_(p), aggOp(agg_op), 
    metric_name(metname), focus(focus_), dontInsertData_(arg_dontInsertData),
    currentlyPaused(false), instrInserted_(false),
    isBeingDeleted_(false)
{ }

processMetFocusNode::processMetFocusNode(const processMetFocusNode &par, 
					 pd_process *childProc) :
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
              instrCodeNode::copyInstrCodeNode(*par.constraintCodeNodes[i], 
					       childProc));
   }

   // I need to determine which threads are being duplicated in the child
   // process.  Under pthreads, my understanding is that only the thread
   // for which the fork call occurs is duplicated in the child.  Under
   // Solaris threads, my understanding is that all threads are duplicated.
   threadMgr::thrIter itr = childProc->thrMgr().begin();
   for(; itr != childProc->thrMgr().end(); itr++) {
      pd_thread *childThr = *itr;
      const threadMetFocusNode *parThrNode = 
         par.getThrNode(childThr->get_tid());
      // If a thread in the child process has a corresponding threadNode
      // in the parent processMetFocusNode, then we need to copy it over
      // We need to use the method where we only copy over thread nodes
      // if the thread is in the child-process because the different thread
      // models have different semantics as far as which threads are created
      // in the child process when a fork occurs.
      if(parThrNode == NULL) {
         cerr << "Warning: in fork handling, couldn't find a thrMetFocusNode "
              << "in the parent process for a thread in the child process\n";
         continue;
      }
      threadMetFocusNode *newThrNode = 
         threadMetFocusNode::copyThreadMetFocusNode(*parThrNode, childProc,
                                                    childThr);
      addThrMetFocusNode(newThrNode);
   }
   
   procStartTime = par.procStartTime;
   
   aggOp = par.aggOp;
   procStartTime = par.procStartTime;
   
   dontInsertData_ = par.dontInsertData_;
   currentlyPaused = par.currentlyPaused;
   
   catchupASTList = par.catchupASTList;
   sideEffectFrameList = par.sideEffectFrameList;
   instrInserted_ = par.instrInserted_;
}

bool processMetFocusNode::instrLoaded() {
   bool allCompInserted = true;

   pdvector<instrCodeNode *> codeNodes;
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

bool processMetFocusNode::hasBeenCatchuped() {
  pdvector<instrCodeNode *> codeNodes;
  getAllCodeNodes(&codeNodes);

  if(codeNodes.size() == 0)  return false;

  bool hookedUp = true;
  for(unsigned i=0; i<codeNodes.size(); i++) {
    instrCodeNode *codeNode = codeNodes[i];
    if(codeNode->needsCatchup() == true) {
      hookedUp = false;
      break;
    }
  }
  return hookedUp;
}

bool processMetFocusNode::trampsHookedUp() {
  pdvector<instrCodeNode *> codeNodes;
  getAllCodeNodes(&codeNodes);

  if(codeNodes.size() == 0)  return false;

  bool hookedUp = true;
  for(unsigned i=0; i<codeNodes.size(); i++) {
    instrCodeNode *codeNode = codeNodes[i];
    if(codeNode->trampsNeedHookup() == true) {
      hookedUp = false;
      break;
    }
  }
  return hookedUp;
}


threadMetFocusNode *processMetFocusNode::getThrNode(unsigned tid) {
   pdvector<threadMetFocusNode *>::iterator iter = thrNodes.begin();
   while(iter != thrNodes.end()) {
      if((*iter)->getThreadID() == tid) {
         return (*iter);
      }
      iter++;
   }
   return NULL;
}

const threadMetFocusNode *processMetFocusNode::getThrNode(unsigned tid) const {
   pdvector<threadMetFocusNode *>::const_iterator iter = thrNodes.begin();
   while(iter != thrNodes.end()) {
      if((*iter)->getThreadID() == tid) {
         return (*iter);
      }
      iter++;
   }
   return NULL;
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
		       pd_process *p, const string &metname,
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
   
   pdvector<process*> vp(1, this->proc());
   pdvector< pdvector<dyn_thread *> > t_hreadsVec;
#if defined(MT_THREAD)
   t_hreadsVec += this->proc()->t_hreads;
#endif
   cerr << "mdl_do - C\n";
   machineMetFocusNode *tempMachNode = mdl_do(//mid
              const_cast<pdvector< pdvector<string> > &>(machnode->getFocus()),
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
   
   pdvector<metricFocusNode *> comp = tempMachNode->getComponents();
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
      pdvector<metricFocusNode *> aggComp = aggMI->getComponents();
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

pdvector<int> deferredMetricIDs;

void registerAsDeferred(int metricID) {
   for(unsigned i=0; i<deferredMetricIDs.size(); i++) {
      if(metricID == deferredMetricIDs[i]) {
	 return;
      }
   }
   deferredMetricIDs.push_back(metricID);
}

instr_insert_result_t processMetFocusNode::insertInstrumentation() {
    assert(dontInsertData() == false);  // code doesn't have allocated variables
    if(instrInserted()) {
        return insert_success;
    }
   
   if(instrLoaded()) {
       assert(trampsHookedUp());
       assert(hasBeenCatchuped());
       instrInserted_ = true;
       return insert_success;
   }
   
   pauseProcess();

   instr_insert_result_t insert_status = loadInstrIntoApp();
   
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
   // Note: this must run IMMEDIATELY after inserting the jumps to tramps

   doCatchupInstrumentation();

   // Changes for MT: process will be continued by inferior RPCs
   // This is because the inferior RPCs may complete after the instrumentation
   // path, and so they must leave the process in a paused state.
   
   instrInserted_ = true;
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
//       Yes -> run (a copy) of n's instrumentation via inferior RPC
//         
void processMetFocusNode::doCatchupInstrumentation() {
    // doCatchupInstrumentation is now the primary control
    // of whether a process runs or not. 
    // The process may have been paused when we inserted instrumentation.
    // If so, the value of "currentlyPaused" is 0 (user paused)
    // If we paused to insert the value of currentlyPaused is 1

    assert(hasBeenCatchuped() == false);
    
    prepareCatchupInstr();
    bool catchupPosted = postCatchupRPCs();

    if (!catchupPosted) {
        if (currentlyPaused) continueProcess();
        return;
    }
    
    // Get them all cleared out
    // Logic: there are three states (that we care about) for an iRPC
    // 1) Runnable, cool, we're done
    // 2) Waiting for a system call trap. Equivalent to runnable for 
    //    our purposes
    // 3) Waiting for a system call, no trap. Nothing we can do but
    //    wait and pick it up somewhere else.
    proc_->launchRPCs(currentlyPaused);
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

void processMetFocusNode::prepareCatchupInstr(pd_thread *thr) {
   pdvector<Frame> stackWalk;
   thr->walkStack(stackWalk);
   
   // Convert the stack walks into a similar list of catchupReq nodes, which
   // maps 1:1 onto the stack walk and includes a vector of instReqNodes that
   // need to be executed
   Address aixHACKlowestFunc = (Address) -1;
   pdvector<catchupReq *> catchupWalk;
   for (unsigned f=0; f<stackWalk.size(); f++) {
       catchupWalk.push_back(new catchupReq(stackWalk[f]));
       if (aixHACKlowestFunc == -1) {
           aixHACKlowestFunc = stackWalk[f].getPC();
       }
   }
   
   // Now go through all associated code nodes, and add appropriate bits to
   // the catchup request list.
   for (unsigned cIter = 0; cIter < constraintCodeNodes.size(); cIter++)
      constraintCodeNodes[cIter]->prepareCatchupInstr(catchupWalk);
   // Get the metric code node catchup list
   metricVarCodeNode->prepareCatchupInstr(catchupWalk);
   
   // Note: stacks are delivered to us in bottom-up order (most recent to
   // main()), and we want to execute from main down. So loop through
   // backwards and add to the main list. We can go one thread at a time,
   // because multiple threads will not interfere with each other.

   // See comment below for sparc info
#if !defined(sparc_sun_solaris2_4)
   // Then through each frame in the stack walk
   AstNode *conglomerate = NULL;
   Address aixHACK = 0;
   for(int j = catchupWalk.size()-1; j >= 0; j--) { 
      catchupReq *curCReq = catchupWalk[j];
      // Note: backwards iteration
      if ((curCReq->reqNodes).size()) {   // Means we have a catchup request
         // What we want to do: build a single big AST then launch it as
         // an RPC.
         for (unsigned k1 = 0; k1<(curCReq->reqNodes).size(); k1++) {
            AstNode *AST = curCReq->reqNodes[k1]->Ast();
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
         side.frame = curCReq->frame;
         side.reqNode = curCReq->reqNodes[0];
         sideEffectFrameList.push_back(side);
      }
   }

   if (conglomerate) {
      catchup_t catchup;
      //conglomerate->print();
      catchup.ast = conglomerate;
      catchup.thread = catchupWalk[0]->frame.getThread();
      catchup.firstaddr = aixHACKlowestFunc;
      catchupASTList.push_back(catchup);      
   }
#else
   // Sparc is currently having problems with AST sequences, especially
   // if the sequences include several copies of the same AST tree. So 
   // we post as individual iRPCs

   // Then through each frame in the stack walk
   for(int j = catchupWalk.size()-1; j >= 0; j--) { 
       catchupReq *curCReq = catchupWalk[j];
       // Note: backwards iteration
       if ((curCReq->reqNodes).size()) {   // Means we have a catchup request
           // What we want to do: build a single big AST then launch it as
           // an RPC.
           for (unsigned k1 = 0; k1<(curCReq->reqNodes).size(); k1++) {
               AstNode *AST = curCReq->reqNodes[k1]->Ast();
               catchup_t catchup;
               catchup.ast = AST;
               catchup.thread = catchupWalk[0]->frame.getThread();
               // Don't need the AIX hack here
               catchup.firstaddr = 0;
               catchupASTList.push_back(catchup);
           }
           sideEffect_t side;
           side.frame = curCReq->frame;
           side.reqNode = curCReq->reqNodes[0];
           sideEffectFrameList.push_back(side);
       }
   }
#endif
}

// only does catchup on threads which need it
// returns false if no catchup needed
void processMetFocusNode::prepareCatchupInstr() {
   // Run on all threads even if focus only applies to certain threads.
   // Because even if we only want to sample on certain threads still want to
   // set up all threads so instr. variables get written to properly.
   threadMgr::thrIter itr = proc()->thrMgr().begin();
   while(itr != proc()->thrMgr().end()) {
      pd_thread *pdthr = *itr;
      itr++;
      prepareCatchupInstr(pdthr);
   }

   pdvector<instrCodeNode *> allCodeNodes;
   getAllCodeNodes(&allCodeNodes);
   for(unsigned i=0; i<allCodeNodes.size(); i++)
      allCodeNodes[i]->markAsCatchupDone();
}

/*
// This might be a good way to trigger sampling in the future.
// Needs some testing and debugging though.
void processMetFocusNode::catchupRPC_Complete(process *, unsigned rpc_id,
                                              void *data, void *) {
   processMetFocusNode *procNode = static_cast<processMetFocusNode *>(data);

   procNode->cancelPendingRPC(rpc_id);

   if(! procNode->anyPendingRPCs()) {
      cerr << "all pending RPCs have finished, start sampling data nodes\n";
      procNode->prepareForSampling();
   }
}
*/

bool processMetFocusNode::postCatchupRPCs()
{
   // Assume the list of catchup requests is 
   // sorted
   bool catchupPosted = false;
    
   if (catchupASTList.size() == 0) {
       catchupPosted = true;
       return true;
   }

   if (pd_debug_catchup) {
      cerr << "Posting " << catchupASTList.size() << " catchup requests\n";
      cerr << "Handing " << sideEffectFrameList.size() << " side effects\n";
   }
    
   for (unsigned i=0; i < catchupASTList.size(); i++) {
      if (pd_debug_catchup) {
         cerr << "metricID: " << getMetricID() << ", posting ast " << i 
              << " on thread: " 
              << catchupASTList[i].thread->get_tid() << endl;
         
      }
      catchupPosted = true;

      unsigned rpc_id =
         proc_->postRPCtoDo(catchupASTList[i].ast, false, 
                            NULL, NULL,
                            false,  // lowmem parameter
                            catchupASTList[i].thread, NULL,
                            catchupASTList[i].firstaddr);
      rpc_id_buf.push_back(rpc_id);
   }
   
   catchupASTList.resize(0);
   for (unsigned j = 0; j < sideEffectFrameList.size(); j++) {
      proc_->catchupSideEffect(sideEffectFrameList[j].frame, 
                               sideEffectFrameList[j].reqNode);
   }
   sideEffectFrameList.resize(0);
   return catchupPosted;
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

instr_insert_result_t processMetFocusNode::loadInstrIntoApp()
{
   pdvector<instrCodeNode *> codeNodes;
   getAllCodeNodes(&codeNodes);

   // mark remaining prim. components as deferred if we come upon
   // one deferred component
   for (unsigned j=0; j<codeNodes.size(); j++) {
      instrCodeNode *codeNode = codeNodes[j];
      instr_insert_result_t status = codeNode->loadInstrIntoApp();
      if(status != insert_success) {
	 return status;
      }
   }

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

pdvector<const instrDataNode*> processMetFocusNode::getFlagDataNodes() const {
  pdvector<const instrDataNode*> buff;
  for(unsigned i=0; i<constraintCodeNodes.size(); i++) {
    const instrDataNode *dn = constraintCodeNodes[i]->getFlagDataNode();
    buff.push_back(dn);
  }
  return buff;
}

bool processMetFocusNode::needToWalkStack() {
   bool anyNeeded = false;

   pdvector<instrCodeNode *> codeNodes;
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
   assert(trampsHookedUp() == false);

   // pause once for all primitives for this component
   
   // only overwrite 1 instruction on power arch (2 on mips arch)
   // always safe to instrument without stack walk
   bool allInserted = true;

   pdvector<instrCodeNode *> codeNodes;
   getAllCodeNodes(&codeNodes);
   
   // Vector of stack walks, one per thread
   pdvector<pdvector<Frame> > stackWalks; 

   if (needToWalkStack()) {
     proc()->walkStacks(stackWalks);
     // ndx 0 is where the pc is now; ndx 1 is the call site;
     // ndx 2 is the call site's call site, etc...
   }
   // Some platforms overwrite a single instruction, so no need
   // for a stack walk since writing is always safe.

   for (unsigned u2=0; u2<codeNodes.size(); u2++) {
      instrCodeNode *codeNode = codeNodes[u2];
      bool result = codeNode->insertJumpsToTramps(stackWalks);
      if(result == false)
         allInserted = false;
   }
   return allInserted;
}


timeLength processMetFocusNode::cost() const {
   timeLength totCost = timeLength::Zero();   

   pdvector<const instrCodeNode *> codeNodes;
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

   pdvector<instrCodeNode *> codeNodes;
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
}

void processMetFocusNode::pauseProcess() {
  if(currentlyPaused == true)  return;

  if (proc()->status() == running) {
    if (proc()->pause())
    {
      currentlyPaused = true;
    }
  }
}

void processMetFocusNode::continueProcess() {
  if(currentlyPaused) {
      proc()->continueProc();
      currentlyPaused = false;
   }
}

bool processMetFocusNode::hasDeferredInstr() {
  bool hasDeferredComp = false;

  pdvector<instrCodeNode *> codeNodes;
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
void processMetFocusNode::addThrMetFocusNode(threadMetFocusNode* thrNode)
{
  thrNodes.push_back(thrNode);
  aggComponent *childAggInfo = aggregator.newComponent();
  thrNode->recordAsParent(this, childAggInfo);
}

void processMetFocusNode::removeThrMetFocusNode(threadMetFocusNode *thrNode) {
   pdvector<threadMetFocusNode *>::iterator iter = thrNodes.end();
   bool didErase = false;
   while(iter != thrNodes.begin()) {
      threadMetFocusNode *curThrNode = *(--iter);
      if(curThrNode == thrNode) {
         thrNodes.erase(iter);         
         delete curThrNode;   // calls markAsFinished on the aggComponent
         didErase = true;
         break;
      }
   }
   assert(didErase == true);   
}

void processMetFocusNode::setMetricVarCodeNode(instrCodeNode* part) {
  metricVarCodeNode = part;
}

void processMetFocusNode::addConstraintCodeNode(instrCodeNode* part) {
  constraintCodeNodes.push_back(part);
}

void processMetFocusNode::propagateToNewThread(pd_thread *thr)
{
   // we only want to sample this new thread if the selected focus for this
   // processMetFocusNode was for the whole process.  If it's a thread
   // specific focus, then it can't have been the given thread because this
   // propagate function is only called on existing metrics.  If it's not a
   // thread specific focus, then the focus includes the whole process
   // (ie. all of the threads).
   if(getFocus().thread_defined()) return;

   string thrName = string("thr_") + string(thr->get_tid()) + "{" + 
                      thr->get_start_func()->prettyName() + "}";
   Focus focus_with_thr = getFocus();
   focus_with_thr.set_thread(thrName);
   threadMetFocusNode *thrNode = threadMetFocusNode::
      newThreadMetFocusNode(metric_name, focus_with_thr, thr);
   addThrMetFocusNode(thrNode);

   thrNode->initializeForSampling(getWallTime(), pdSample::Zero());
}

void processMetFocusNode::updateForExitedThread(pd_thread *thr)
{
   // we only want to sample this new thread if the selected focus for this
   // processMetFocusNode was for the whole process.  If it's a thread
   // specific focus, then it can't have been the given thread because this
   // propagate function is only called on existing metrics.  If it's not a
   // thread specific focus, then the focus includes the whole process
   // (ie. all of the threads).
   if(getFocus().thread_defined()) {
      return;
   }

   threadMetFocusNode *thrNode = getThrNode(thr);
   assert(thrNode != NULL);
   removeThrMetFocusNode(thrNode);
}

void processMetFocusNode::unFork() {
  delete this;  // will remove instrumentation and data for the procMetFocusNode
                // (the metFocusNode deletion code deals with code and data
                //  sharing appropriately)
}

// returns true if erased, otherwise returns false
bool processMetFocusNode::cancelPendingRPC(unsigned rpc_id) {
   bool erased = false;
   pdvector<unsigned>::iterator iter = rpc_id_buf.end();
   while(iter != rpc_id_buf.begin()) {
      iter--;
      unsigned cur_rpc_id = (*iter);
      if(rpc_id == cur_rpc_id) {
         rpc_id_buf.erase(iter);
         erased = proc_->cancelRPC(cur_rpc_id);
         break;
      }
   }
   return erased;
}

void processMetFocusNode::cancelPendingRPCs() {
   pdvector<unsigned>::iterator iter = rpc_id_buf.end();
   while(iter != rpc_id_buf.begin()) {
      iter--;
      cancelPendingRPC(*iter);
   }
}

