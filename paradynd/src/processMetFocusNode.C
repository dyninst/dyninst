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

#include "paradynd/src/machineMetFocusNode.h"
#include "paradynd/src/processMetFocusNode.h"
#include "paradynd/src/threadMetFocusNode.h"
#include "paradynd/src/instrCodeNode.h"
#include "paradynd/src/instrDataNode.h"
#include "paradynd/src/dynrpc.h"
#include "pdutil/h/pdDebugOstream.h"
#include "paradynd/src/init.h"
#include "paradynd/src/pd_process.h"
#include "paradynd/src/pd_thread.h"
#include "paradynd/src/debug.h"

#include "dyninstAPI/src/dyn_thread.h"
#include "dyninstAPI/src/function.h"

void checkProcStatus();  
  
inline unsigned ui_hash_(const unsigned &u) { return u; }

pdvector<processMetFocusNode *> processMetFocusNode::procNodesToDeleteLater;

processMetFocusNode::processMetFocusNode(pd_process *p,
                       const pdstring &metname, const Focus &focus_,
		       aggregateOp agg_op, bool arg_dontInsertData)
  : metricVarCodeNode(NULL), aggregator(agg_op, getCurrSamplingRate()),
    parentNode(NULL), aggInfo(NULL), proc_(p), aggOp(agg_op), 
    metric_name(metname), focus(focus_), dontInsertData_(arg_dontInsertData),
    runWhenFinished_(false), instrInserted_(false),
    isBeingDeleted_(false)
{ 
  // /* DEBUG */ fprintf(stderr, "new procMetFocus, runWhenFinished_ off\n");
}

processMetFocusNode::processMetFocusNode(const processMetFocusNode &par, 
					 pd_process *childProc) :
   aggregator(par.aggOp, getCurrSamplingRate()), // start with fresh aggregator
   // set by recordAsParent(), in propagateToForkedProcess
   parentNode(NULL), aggInfo(NULL), proc_(childProc),
   metric_name(par.metric_name), 
   focus(adjustFocusForPid(par.focus, childProc->getPid())),
   dontInsertData_(par.dontInsertData_), runWhenFinished_(par.runWhenFinished_),
   isBeingDeleted_(par.isBeingDeleted_)
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
              << "in the parent process for thread " 
	      << childThr->get_tid() 
	      << " in the child process\n";
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
   runWhenFinished_ = par.runWhenFinished_;
   
   catchupASTList = par.catchupASTList;
   instrInserted_ = par.instrInserted_;
}

// Multiplexes across multiple codeNodes; AND operation

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

// Multiplexes across multiple codeNodes; AND operation


bool processMetFocusNode::instrCatchuped() {
    pdvector<instrCodeNode *> codeNodes;
    getAllCodeNodes(&codeNodes);
    
    if(codeNodes.size() == 0)  return false;
    
    bool catchupDone = true;
    for(unsigned i=0; i<codeNodes.size(); i++) {
        instrCodeNode *codeNode = codeNodes[i];
        if(!codeNode->instrCatchuped()) {
            catchupDone = false;
            break;
        }
    }
    return catchupDone;
}

// Multiplexes across multiple codeNodes; AND operation

bool processMetFocusNode::instrLinked() {
    pdvector<instrCodeNode *> codeNodes;
    getAllCodeNodes(&codeNodes);
    
    if(codeNodes.size() == 0)  return false;
    
    bool linked = true;
    for(unsigned i=0; i<codeNodes.size(); i++) {
        instrCodeNode *codeNode = codeNodes[i];
        if(!codeNode->instrLinked()) {
            linked = false;
            break;
        }
    }
    return linked;
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
    sample_cerr << "  skipping sample since startTime- is uninitialized\n";
    return false;
  }
  
  // the sample did not occur while (but before) the mdn was active
  if(sampleTime < MFstartTime) {
    sample_cerr << "  skipping sample since sample did not occur while"
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
    sample_cerr << "  using " << pcntIntvlActiveForNewMdn
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

  sample_cerr << "procNode: " << this << ", updateWithDeltaValue, time: "
		 << sampleTime << ", val: " << valToPass << "\n";

  aggInfo->addSamplePt(sampleTime, valToPass);
  
  parentNode->tryAggregation();
}

processMetFocusNode *processMetFocusNode::newProcessMetFocusNode(
		       pd_process *p, const pdstring &metname,
		       const Focus &component_foc,
		       aggregateOp agg_op, bool arg_dontInsertData)
{
  processMetFocusNode *procNode = new processMetFocusNode(p, metname, 
				    component_foc, agg_op, arg_dontInsertData);
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

inst_insert_result_t processMetFocusNode::insertInstrumentation() {
    //fprintf(stderr, "InsertInstrumentation for %p, status done %d, inserted %d, hooked %d, catchuped %d\n", this, instrInserted(), instrLoaded(), instrLinked(), instrCatchuped());
    assert(dontInsertData() == false);  // code doesn't have allocated variables
    if(instrInserted()) {
        return inst_insert_success;
    }

    // We may have already "inserted" the instrumentation; that is, we are sharing
    // with previously existing code. 
    if (instrLoaded() && instrLinked()) {
        // If linked, then catchuped
        assert(instrCatchuped());
        instrInserted_ = true;
        return inst_insert_success;
    }

    // There are three steps we take:
    // 1) Load instrumentation
    // 2) Link instrumentation ("hook up tramps")
    // 3) Do catchup
    // If #2 is done, assert #3 as they must be atomic. However, there can be
    // arbitrary time between #1 and #2.

    // Since instrInserted did not return true, there is work to do. Pause the
    // process for correctness.

    // This is our pauseProcess, it tracks whether the process was paused
    // when instrumentation was inserted.
    pauseProcess();
    if(proc()->hasExited()) {
        // though technically we failed to insert instrumentation, from the
        // perspective of the machineMetFocusNode, it succeeded.
        return inst_insert_success;
    }
    
    bool res;

    if (!instrLoaded())
        res = loadInstrIntoApp();
    else
        res = true;
    
    if (!res) {
        continueProcess();
        pdstring msg = pdstring("Unable to load instrumentation for metric focus ")
            + getFullName() + " into process with pid " 
            + pdstring(proc()->getPid());
        showErrorCallback(126, msg);
        return inst_insert_failure;
    }

    // We've loaded instrumentation, now try to hook it up.
    // We know we're not hooked up because otherwise instrInserted
    // would be true, above.

    // We take a stack walk to assist in both inserting jumps (which needs to
    // know PCs) and in catchup. It's more efficient to do it once.

    assert(!instrLinked());

    pdvector <pdvector <Frame> > stackWalks;
    proc()->walkStacks(stackWalks);

    if (pd_debug_catchup) {
        for (unsigned thr_iter = 0; thr_iter < stackWalks.size(); thr_iter++) {
            pdvector<Frame> &stackWalk = stackWalks[thr_iter];
            
            fprintf(stderr, "Dumping stack walk for thread %d\n",
                    stackWalk[0].getThread()->get_tid());
            for (unsigned a = 0; a < stackWalk.size(); a++) {
                cerr << stackWalk[a] << endl;
            }
            fprintf(stderr, "------------------\n");
        }
    }

    // Before we insert jumps we run a set of "instrumentation fixups". 
    // This code attempts to move the PC in each frame to an equivalent
    // location inside instrumentation (if there is a correspondence). 
    // It's sort of like catchup, but for the PC instead of instrumentation.
    // This may modify the stack walk.

    // We may need to modify certain pieces of the process state to ensure
    // that instrumentation runs properly. Two known cases:
    // 1) If the PC is inside a basic block that is newly instrumented,
    //    move the PC into the equivalent location in the multitramp.
    //    This includes frames on the stack if the return addr is inside
    //    a basic block.
    // 2) Possible: move the PC from an "old" multitramp to the new one (?)
    // TODO: we need to make this work.
    // Assertion case: if we modify PCs, we can inserted jumps to tramps.

    bool modified = doInstrumentationFixup(stackWalks);
    assert(res);
    

    res = insertJumpsToTramps(stackWalks);
    
    if (!res) {
        assert(!modified);
        // We defer and try again later.
        continueProcess();
        // If any of the instrCodeNodes are marked as deferred...
        if(instrDeferred()) {
            registerAsDeferred(getMetricID());
            return inst_insert_deferred;
        }
        else
            return inst_insert_failure;
    }
    
    // Now that the timers and counters have been allocated on the heap, and
    // the instrumentation added, we can manually execute instrumentation we
    // may have inserted that was "missed" because we did not insert at
    // time 0.
    // Note: this must run IMMEDIATELY after inserting the jumps to tramps or
    // our state snapshot is invalid.

    // Catchup must not have been done already...
    assert(!instrCatchuped());

    res = doCatchupInstrumentation(stackWalks);

    // Changes for MT: process will be continued by inferior RPCs or doCatchup...
    // This is because the inferior RPCs may complete after the instrumentation
    // path, and so they must leave the process in a paused state.

    assert(res); // No fallback for failed catchup.
    instrInserted_ = true;
    return inst_insert_success;
}

// We may need to modify certain pieces of the process state to ensure
// that instrumentation runs properly. Two known cases:
// 1) If an active call site (call site on the stack) is instrumented,
//    we need to modify the return address to be in instrumentation
//    and not at the original return addr.
// 2) AIX only: if we instrument with an entry/exit pair, modify the
//    return address of the function to point into the exit tramp rather
//    than the return addr. 
//    Note that #2 overwrites #1; but if we perform the fixes in this order
//    then everything works.

bool processMetFocusNode::doInstrumentationFixup(pdvector<pdvector<Frame> >&stackWalks) {
    bool modified = false;
    // These return true if anything was modified

    for (unsigned cIter = 0; cIter < constraintCodeNodes.size(); cIter++) {
        pdvector<instReqNode *> instRequests = constraintCodeNodes[cIter]->getInstRequests();
        for (unsigned iIter = 0; iIter < instRequests.size(); iIter++) {
            for (unsigned thrIter = 0; thrIter < stackWalks.size(); thrIter++) {
                for (unsigned sIter = 0; sIter < stackWalks[thrIter].size(); sIter++) {
                    modified |= proc_->catchupSideEffect(stackWalks[thrIter][sIter],
                                                         instRequests[iIter]);
                }
            }
        }
    }
    pdvector<instReqNode *> instRequests2 = metricVarCodeNode->getInstRequests();
    for (unsigned iIter2 = 0; iIter2 < instRequests2.size(); iIter2++) {
        for (unsigned thrIter2 = 0; thrIter2 < stackWalks.size(); thrIter2++) {
            for (unsigned sIter2 = 0; sIter2 < stackWalks[thrIter2].size(); sIter2++) {
                modified |= proc_->catchupSideEffect(stackWalks[thrIter2][sIter2],
                                                     instRequests2[iIter2]);
            }
        }
    }
    return modified;
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
bool processMetFocusNode::doCatchupInstrumentation(pdvector<pdvector<Frame> >&stackWalks) {
    // doCatchupInstrumentation is now the primary control
    // of whether a process runs or not. 
    // The process may have been paused when we inserted instrumentation.
    // If so, the value of "runWhenFinished_" is 0 (user paused)
    // If we paused to insert the value of runWhenFinished_ is 1
    // /* DEBUG */ fprintf(stderr, "doCatchup entry, runWhenFinished_ = %d\n", runWhenFinished_);
    assert(!instrCatchuped());
    
    prepareCatchupInstr(stackWalks);
    bool catchupPosted = postCatchupRPCs();
    
    if (!catchupPosted) {
        // Nothing to do here...
        if (runWhenFinished_) continueProcess();
        return true;
    }
    
    // Get them all cleared out
    // Logic: there are three states (that we care about) for an iRPC
    // 1) Runnable, cool, we're done
    // 2) Waiting for a system call trap. Equivalent to runnable for 
    //    our purposes
    // 3) Waiting for a system call, no trap. Nothing we can do but
    //    wait and pick it up somewhere else.
    if (!proc_->launchRPCs(runWhenFinished_)) {
        if (runWhenFinished_) 
            continueProcess();
        return true;
    }
    // runWhenFinished_ now becomes the state to leave the process in when we're
    // done with catchup. For a little while, we'll be out of sync with the 
    // actual process. This is annoying, but necessary, since we can't effectively
    // do synchronous catchup RPCs.
    return true;
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

void processMetFocusNode::prepareCatchupInstr(pdvector<pdvector<Frame> > &stackWalks) {
    for (unsigned thr_iter = 0; thr_iter < stackWalks.size(); thr_iter++) {
        pdvector<Frame> &stackWalk = stackWalks[thr_iter];
        
        
        // Convert the stack walks into a similar list of catchupReq nodes, which
        // maps 1:1 onto the stack walk and includes a vector of instReqNodes that
        // need to be executed
        pdvector<catchupReq *> catchupWalk;
        for (unsigned f=0; f<stackWalk.size(); f++) {
            catchupWalk.push_back(new catchupReq(stackWalk[f]));
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
        
        // can't do "conglomerate rpcs" until bug #369 "AST sequence node
        // generation" is fixed
#if defined(CONGLOMERATE_RPCS)
        // Then through each frame in the stack walk
        AstNode *conglomerate = NULL;
        for(int j = catchupWalk.size()-1; j >= 0; j--) { 
            catchupReq *curCReq = catchupWalk[j];
            // Note: backwards iteration
            if ((curCReq->reqNodes).size()) {   // Means we have a catchup request
                // What we want to do: build a single big AST then launch it as
                // an RPC.
                for (unsigned k1 = 0; k1<(curCReq->reqNodes).size(); k1++) {
                    AstNode *AST = curCReq->reqNodes[k1]->Snippet()->PDSEP_ast();
                    if (!conglomerate) {
                        conglomerate = AST;
                    }
                    else { // Need to combine with a SequenceNode
                        AstNode *old = conglomerate;
                        conglomerate = new AstNode(conglomerate, AST);
                        removeAst(old);
                    }
                }
            }
        }
        
        if (conglomerate) {
            catchup_t catchup;
            //conglomerate->print();
            catchup.ast = conglomerate;
            catchup.thread = catchupWalk[0]->frame.getThread();
            catchupASTList.push_back(catchup);      
        }
#else
        // Sparc (and linux) is currently having problems with AST sequences,
        // especially if the sequences include several copies of the same AST
        // tree. So we post as individual iRPCs
        
        // Then through each frame in the stack walk
        for(int j = catchupWalk.size()-1; j >= 0; j--) { 
            catchupReq *curCReq = catchupWalk[j];
            // Note: backwards iteration
            if ((curCReq->reqNodes).size()) {   // Means we have a catchup request
                // What we want to do: build a single big AST then launch it as
                // an RPC.
                for (unsigned k1 = 0; k1<(curCReq->reqNodes).size(); k1++) {
                    AstNode *AST = curCReq->reqNodes[k1]->Snippet()->PDSEP_ast();
                    catchup_t catchup;
                    catchup.ast = AST;
                    catchup.thread = catchupWalk[0]->frame.getThread();
                    catchupASTList.push_back(catchup);
                }
            }
        }
#endif
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
   if (catchupASTList.size() == 0) {
       return false;
   }

   if (pd_debug_catchup) {
      cerr << "Posting " << catchupASTList.size() << " catchup requests\n";
   }
    
   for (unsigned i=0; i < catchupASTList.size(); i++) {
      if (pd_debug_catchup) {
         cerr << "metricID: " << getMetricID() << ", posting ast " << i 
              << " on thread: " 
              << catchupASTList[i].thread->get_tid() << endl;
         
      }

      unsigned rpc_id =
         proc_->postRPCtoDo(catchupASTList[i].ast, false, 
                            NULL, NULL,
                            false,  // lowmem parameter
                            catchupASTList[i].thread, NULL);
      rpc_id_buf.push_back(rpc_id);
   }
   
   catchupASTList.resize(0);

   return true;
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

bool processMetFocusNode::loadInstrIntoApp()
{
    pdvector<instrCodeNode *> codeNodes;
    getAllCodeNodes(&codeNodes);
    
    // If we hit an error stop immediately.
    for (unsigned j=0; j<codeNodes.size(); j++) {
        instrCodeNode *codeNode = codeNodes[j];
        bool res = codeNode->loadInstrIntoApp();
        
        if (!res) {
            return false;
        }
    }
    
    return true;
}

void processMetFocusNode::removeDataNodes()
{
   pdvector<instrCodeNode *> codeNodes;
   getAllCodeNodes(&codeNodes);

   for (unsigned i=0; i<codeNodes.size(); i++) 
   {
      instrCodeNode *codeNode = codeNodes[i];
      codeNode->setSampledDataNode(NULL);
      codeNode->setConstraintDataNode(NULL);
   }
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

// 11JUN05 - we no longer have a separate "return instance" structure. Instead,
// each instPoint has a "linkInst" call that is made to link in instrumentation,
// and the necessary info is stored there. There is also a checkInst call
// that returns whether it is safe to put in instrumentation.

bool processMetFocusNode::insertJumpsToTramps(pdvector<pdvector<Frame > >&stackWalks) {
    assert(!instrLinked());

   // pause once for all primitives for this component
   
   // only overwrite 1 instruction on power arch (2 on mips arch)
   // always safe to instrument without stack walk
   bool allInserted = true;

   pdvector<instrCodeNode *> codeNodes;
   getAllCodeNodes(&codeNodes);
   
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

void processMetFocusNode::removeProcNodesToDeleteLater() {
   for(unsigned i=0; i<procNodesToDeleteLater.size(); i++) {
      processMetFocusNode *procNode = procNodesToDeleteLater[i];
      delete procNode;
   }
}

void processMetFocusNode::pauseProcess() {
  // /* DEBUG */ fprintf(stderr, "procMetFoc, pauseProc %d\n", runWhenFinished_);
  if(runWhenFinished_ == true)  return;
  if (proc()->isStopped()) return; // at startup, process can be stopped, but not "paused"
                                 // don't need to pause, and setting runWhenFinished_
                                 // will result in the process getting started at
                                 // a bad moment.
  if (proc()->pause())
  {
    runWhenFinished_ = true;
  }

}

void processMetFocusNode::continueProcess() {
    // /* DEBUG */ fprintf(stderr, "procMetFoc, paused %d\n", runWhenFinished_);
    if(runWhenFinished_) {
        proc()->continueProc();
        runWhenFinished_ = false;
    }
}

bool processMetFocusNode::instrDeferred() {
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

   pdstring thrName = pdstring("thr_") + pdstring(thr->get_tid()) + "{" + 
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
         erased = proc_->get_dyn_process()->PDSEP_process()->getRpcMgr()->cancelRPC(cur_rpc_id);
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
