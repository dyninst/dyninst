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


   // We better not be in the middle of calculations when a fork hits.
   assert(par.catchupASTList.size() == 0);

   instrInserted_ = par.instrInserted_;
}

// Multiplexes across multiple codeNodes; AND operation

bool processMetFocusNode::instrLoaded() 
{
   bool allCompInserted = true;

   pdvector<instrCodeNode *> codeNodes;
   getAllCodeNodes(&codeNodes);
   if (codeNodes.size() == 0) {
      allCompInserted = false;
   } else {
      for(unsigned i=0; i<codeNodes.size(); i++) {
         instrCodeNode *codenode = codeNodes[i];
         bool result = codenode->instrLoaded();
         if (result == false) {
            allCompInserted = false; 
            break;
         }
      }
   }
   return allCompInserted;
}

// Multiplexes across multiple codeNodes; AND operation


bool processMetFocusNode::instrCatchuped() 
{
    pdvector<instrCodeNode *> codeNodes;
    getAllCodeNodes(&codeNodes);
    
    if (codeNodes.size() == 0)  return false;
    
    bool catchupDone = true;
    for (unsigned i=0; i<codeNodes.size(); i++) {
        instrCodeNode *codeNode = codeNodes[i];
        if (!codeNode->instrCatchuped()) {
            catchupDone = false;
            break;
        }
    }
    return catchupDone;
}

threadMetFocusNode *processMetFocusNode::getThrNode(unsigned tid) 
{
   pdvector<threadMetFocusNode *>::iterator iter = thrNodes.begin();
   while(iter != thrNodes.end()) {
      if((*iter)->getThreadID() == tid) {
         return (*iter);
      }
      iter++;
   }
   return NULL;
}

const threadMetFocusNode *processMetFocusNode::getThrNode(unsigned tid) const 
{
   pdvector<threadMetFocusNode *>::const_iterator iter = thrNodes.begin();
   while(iter != thrNodes.end()) {
      if((*iter)->getThreadID() == tid) {
         return (*iter);
      }
      iter++;
   }
   return NULL;
}

void processMetFocusNode::tryAggregation() 
{
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
			  timeStamp sampleTime, timeStamp MFstartTime) 
{
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
			 timeStamp sampleTime, pdSample value) 
{
  pdSample valToPass = value;
  bool shouldAddSample = adjustSampleIfNewMdn(&valToPass, startTime, 
				  sampleTime, parentNode->getStartTime());
  if (!shouldAddSample) return;
  
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
  processMetFocusNode *procNode;
  procNode = new processMetFocusNode(p, metname, 
				    component_foc, agg_op,  
                                    arg_dontInsertData);
  return procNode;
}

int processMetFocusNode::getMetricID()
{ 
  return parentNode->getMetricID();
}

pdvector<int> deferredMetricIDs;

void registerAsDeferred(int metricID) 
{
   for(unsigned i=0; i<deferredMetricIDs.size(); i++) {
      if(metricID == deferredMetricIDs[i]) {
	 return;
      }
   }
   deferredMetricIDs.push_back(metricID);
}

inst_insert_result_t processMetFocusNode::insertInstrumentation() 
{
    //fprintf(stderr, "InsertInstrumentation for %p, status done %d, inserted %d, hooked %d, catchuped %d\n", this, instrInserted(), instrLoaded(), instrLinked(), instrCatchuped());

    if (proc()->hasExited()) {
        // This in addition to the below; locking can run callbacks
        return inst_insert_success;
    }

    assert(dontInsertData() == false);  // code doesn't have allocated variables
    if(instrInserted()) {
        return inst_insert_success;
    }

    // We may have already "inserted" the instrumentation; that is, we are sharing
    // with previously existing code. 
    if (instrLoaded()) {
        // If loaded, then catchuped
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
    
    bool res = false;
    bool modified = false;

    if (!instrLoaded())
        res = loadInstrIntoApp(&modified);
    else
        res = true;
    
    if (!res) {
        assert(!modified);
        // We defer and try again later.
        continueProcess();
        // If any of the instrCodeNodes are marked as deferred...
        if(instrDeferred()) {
            registerAsDeferred(getMetricID());
            
            return inst_insert_deferred;
        }
        else {
            return inst_insert_failure;
        }
    }
    
    // Now that the timers and counters have been allocated on the heap, and
    // the instrumentation added, we can manually execute instrumentation we
    // may have inserted that was "missed" because we did not insert at
    // time 0.
    // Note: this must run IMMEDIATELY after inserting the jumps to tramps or
    // our state snapshot is invalid.

    // Catchup must not have been done already...
    assert(!instrCatchuped());

    res = doCatchupInstrumentation();

    // Changes for MT: process will be continued by inferior RPCs or doCatchup...
    // This is because the inferior RPCs may complete after the instrumentation
    // path, and so they must leave the process in a paused state.

    assert(res); // No fallback for failed catchup.
    instrInserted_ = true;
    return inst_insert_success;
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
bool processMetFocusNode::doCatchupInstrumentation() 
{
    // doCatchupInstrumentation is now the primary control
    // of whether a process runs or not. 
    // The process may have been paused when we inserted instrumentation.
    // If so, the value of "runWhenFinished_" is 0 (user paused)
    // If we paused to insert the value of runWhenFinished_ is 1
    // /* DEBUG */ fprintf(stderr, "doCatchup entry, runWhenFinished_ = %d\n", runWhenFinished_);
    assert(!instrCatchuped());
    
    prepareCatchupInstr();
    postCatchupRPCs();
    
    // Get them all cleared out
    // Logic: there are three states (that we care about) for an iRPC
    // 1) Runnable, cool, we're done
    // 2) Waiting for a system call trap. Equivalent to runnable for 
    //    our purposes
    // 3) Waiting for a system call, no trap. Nothing we can do but
    //    wait and pick it up somewhere else.

    if (runWhenFinished_) {
        continueProcess();
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

void processMetFocusNode::prepareCatchupInstr() 
{
    // We want to go through the cachup handles, glue together all
    // catchup requests for a given thread, and lump them into the
    // catchupASTList structure.

    // Input: catchup_handles, a vector of:
    //  BPatch_snippet *snip;
    //  BPatchSnippetHandle *sh; // we don't care...
    //  BPatch_thread *thread
    // 
    
    // The list is sorted:
    //  1) By thread
    //  2) By frame order (from main() to the leaf)
    //  3) By snippet (though this we don't care)

    // We want to build catchup_t's. 
    
    // Those are just threads and snippets. However, we don't care
    // about return values from catchup code, and so we optimize
    // slightly by lumping everything for one thread together into a
    // big vector.
    
    BPatch_thread *currentThread = NULL;
    BPatch_Vector<BPatch_snippet *> currentSnippets; 

    pd_catchup_printf("%s[%d]: %d current catchup handles\n",
                      __FILE__, __LINE__, catchup_handles.size());

    for (unsigned i = 0; i < catchup_handles.size(); i++) {

        pd_catchup_printf("%s[%d]: examining handle %d, thread %p\n",
                          __FILE__, __LINE__, i, catchup_handles[i].thread);

        // If currentThread is NULL, assign it our thread;
        
        // While our thread == currentThread, add us to the
        // currentSnippets vector;
        
        // When the threads are different, wrap a sequenceNode around
        // the currentSnippets, stick it in an AST node, and we're good.
        
        if (currentThread == NULL) 
            currentThread = catchup_handles[i].thread;
        assert(currentThread != NULL);
        if (currentThread == catchup_handles[i].thread) {
            pd_catchup_printf("%s[%d]: adding new snippet to sequence\n",
                              __FILE__, __LINE__);
            currentSnippets.push_back(&catchup_handles[i].snip);
        } else {
            // Wrap up the old one and start over.
            assert(currentSnippets.size());


            pd_catchup_printf("%s[%d]: new thread seen (%p != %p), finishing old\n",
                              __FILE__, __LINE__, currentThread, catchup_handles[i].thread);
            
            catchup_t *newCatchup = new catchup_t(currentSnippets, currentThread);
            catchupASTList.push_back(newCatchup);
            currentSnippets.clear();

            // And prime the pump
            currentSnippets.push_back(&catchup_handles[i].snip);
            currentThread = catchup_handles[i].thread;
        }
        assert(currentThread == catchup_handles[i].thread);
        assert(currentSnippets.size());
    }

    if (currentSnippets.size()) {
        // We need to finish off the last one
        pd_catchup_printf("%s[%d]: cleaning up last thread, creating catchup\n",
                          __FILE__, __LINE__);
        catchup_t *newCatchup = new catchup_t(currentSnippets, currentThread);
        catchupASTList.push_back(newCatchup);
        currentSnippets.clear();
    }

    catchup_handles.clear();
    
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
                 << catchupASTList[i]->thread->getTid() << endl;
            
        }
        
        // We now fire off a pile of asynchronous (AKA don't let us
        // know when they're done) oneTimeCodes. We can run into
        // trouble with this if more instrumentation goes in while
        // we're running oneTimeCodes; however, Dyninst now hides this
        // fact from us (the stack doesn't change until the
        // oneTimeCode is complete). So our life is pretty simple. 

        // For each catchupASTList, post an RPC to do. Nice, huh?

        assert(catchupASTList[i]->thread);

        pd_catchup_printf("%s[%d]: posting oneTimeCodeAsync on thread %p\n",
                          __FILE__, __LINE__, catchupASTList[i]->thread);

        catchupASTList[i]->thread->oneTimeCodeAsync(catchupASTList[i]->snip, NULL, NULL);

        delete catchupASTList[i];
    }
    
    catchupASTList.clear();
    
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

bool processMetFocusNode::loadInstrIntoApp(bool *modified)
{
    //  sanity check...  make sure that we don't have any leftover catchup
    if (catchup_handles.size()) {
       fprintf(stderr, "%s[%d]:  FIXME\n", FILE__, __LINE__);
       return false;
    }

    pdvector<instrCodeNode *> codeNodes;
    getAllCodeNodes(&codeNodes);
    
    //  All instrumentation for all codeNodes must be inserted atomically,
    //  manage it here...
    proc()->get_dyn_process()->beginInsertionSet();

    // If we hit an error stop immediately.
    for (unsigned j=0; j<codeNodes.size(); j++) {
        instrCodeNode *codeNode = codeNodes[j];
        bool res = codeNode->loadInstrIntoApp();
        
        if (!res) {
            fprintf(stderr, "%s[%d]:  WARNING:  possible unsafe finalizeInsertionSet()\n", FILE__, __LINE__);
            proc()->get_dyn_process()->finalizeInsertionSetWithCatchup(true /*true = atomic*/, modified, catchup_handles);
            return false;
        }
    }
    
    bool ret  = proc()->get_dyn_process()->finalizeInsertionSetWithCatchup(true /*true = atomic*/, modified, catchup_handles);
    if (!ret)
        fprintf(stderr, "%s[%d] WARNING:  finalizeInsertionSet failed\n", FILE__, __LINE__);
    return ret;
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
    if(runWhenFinished_ == true)  return;
    if (proc()->isStopped()) return; // at startup, process can be stopped, but not "paused"
    // don't need to pause, and setting runWhenFinished_
    // will result in the process getting started at
    // a bad moment.
    if (proc()->pauseProc()) {
        runWhenFinished_ = true;
    }
}

void processMetFocusNode::continueProcess() {
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
                      thr->get_initial_func_name() + "}";
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

