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

#include "paradynd/src/metric.h"
#include "paradynd/src/machineMetFocusNode.h"
#include "paradynd/src/processMetFocusNode.h"
#include "paradynd/src/dynrpc.h"
#include "paradynd/src/comm.h"
#include "dyninstAPI/src/symtab.h"
#include "paradynd/src/internalMetrics.h"
#include "paradynd/src/costmetrics.h"
#include "pdutil/h/pdDebugOstream.h"

extern pdDebug_ostream sampleVal_cerr;
extern pdRPC *tp;

dictionary_hash<unsigned, machineMetFocusNode*> 
                                machineMetFocusNode::allMachNodes(uiHash);

machineMetFocusNode::machineMetFocusNode(int metricID, 
     const string& metric_name, const vector< vector<string> >& foc,
     const string& flat_name, vector<processMetFocusNode*>& parts,
     aggregateOp agg_op, bool enable_)
   : metricDefinitionNode(),
     aggregator(agg_op, getCurrSamplingRate()),
     id_(metricID), aggOp(agg_op), aggInfoInitialized(false),
     _sentInitialActualValue(false), met_(metric_name), flat_name_(flat_name),
     focus_(foc), enable(enable_)
{
  allMachNodes[metricID] = this;

  for (unsigned u=0; u<parts.size(); u++) {
    addPart(parts[u]);
  }
}

machineMetFocusNode::~machineMetFocusNode() {
  for(unsigned i=0; i<procNodes.size(); i++) {
    delete procNodes[i];
  }

  // I want to improve on how we disable the internal and cost metrics
  unsigned ai_size = internalMetric::allInternalMetrics.size();
  for (unsigned t=0; t<ai_size; t++) {
    internalMetric *theIMetric = internalMetric::allInternalMetrics[t];
    theIMetric->disableByMetricDefinitionNode(this);
  }

  // check for cost metrics
  for (unsigned u=0; u<costMetric::allCostMetrics.size(); u++){
    if (costMetric::allCostMetrics[u]->getNode() == this) {
      costMetric::allCostMetrics[u]->disable();
    }
  }

  allMachNodes.undef(getMetricID());
  if(isEnabled())  endOfDataCollection();
}

void machineMetFocusNode::getMachineNodes(
                                 vector<machineMetFocusNode*> *machNodes)
{
  dictionary_hash_iter<unsigned, machineMetFocusNode*> iter = allMachNodes;

  for (; iter; iter++) {
    machineMetFocusNode *curMachNode = iter.currval();
    (*machNodes).push_back(curMachNode);
  }
}

void machineMetFocusNode::deleteProcNode(processMetFocusNode *procNode) {
  for(int i=(int)procNodes.size()-1; i>=0; i--) {
    if(procNode == procNodes[i]) {
      delete procNodes[i];
      procNodes.erase(i);
    }
  }
  if(procNodes.size() == 0) {
    //cerr << "calling delete on machNode since procNodes.size == 0\n";
    delete this;
  }
}

void machineMetFocusNode::endOfDataCollection() {
  flush_batch_buffer();
  // trace data streams
  extern dictionary_hash<unsigned, unsigned> traceOn;
  for (dictionary_hash_iter<unsigned,unsigned> iter=traceOn; iter; iter++) {
     unsigned key = iter.currkey();
     unsigned val = iter.currval();

     if (val) {
        extern void batchTraceData(int, int, int, char *);
	extern bool TRACE_BURST_HAS_COMPLETED;
	TRACE_BURST_HAS_COMPLETED = true;
	batchTraceData(0, key, 0, (char *)NULL);
	traceOn[key] = 0;
     }
  }
  // we're not done until this metric doesn't have any metrics
  tp->endOfDataCollection(id_);
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
//  which the metricDefinitionNode corresponds to, as opposed to
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

void machineMetFocusNode::doCatchupInstrumentation() {
  for(unsigned i=0; i<procNodes.size(); i++) {
    procNodes[i]->doCatchupInstrumentation();
  }
}

// if a thread or process is added to the parent metFocusNode,
// the addThread or "addProcess" function will handle initializing
// these nodes aggInfoObjects

void machineMetFocusNode::initializeForSampling(timeStamp startTime, 
						pdSample initValue)
{
  if(hasAggInfoBeenInitialized())
    return;

  //cerr << "machineMF::initAggInfo, startT: " << startTime << "\n";
  // Initialize aggComponents between MACH <-> PROC nodes
  for(unsigned k=0; k<aggregator.numComponents(); k++) {
    aggComponent *curAggInfo = aggregator.getComponent(k);
    //cerr << "  initializing aggComponent: " << (void*)curAggInfo << "\n";
    curAggInfo->setInitialStartTime(startTime);
    curAggInfo->setInitialActualValue(initValue);
  }

  for(unsigned i=0; i<procNodes.size(); i++) {
    // some processMetFocusNodes might be shared and thus already initialized
    // don't reinitialize these
    // eg. already exist:  MACH-WHOLEPROG -> PROC1, PROC2, PROC3
    //     new request:    MACH-PROC1     -> PROC1
    // in the case above, PROC1 already exists and it's aggInfo is already
    // initialized
    if(! procNodes[i]->hasAggInfoBeenInitialized()) {
      //cerr << "  procNode: " << (void*)procNodes[i] 
      //   << " has not been initialized, initializing\n";
      procNodes[i]->initAggInfoObjects(startTime, initValue);
    } else {
      //cerr << "  procNode: " << (void*)procNodes[i] 
      //   << " has already been initialized\n";
    }
  }

  aggInfoInitialized = true;
  machStartTime = startTime;
}


void machineMetFocusNode::updateAllAggInterval(timeLength width) {
  vector<machineMetFocusNode *> machNodes;
  getMachineNodes(&machNodes);

  for(unsigned i=0; i<machNodes.size(); i++) {
    machNodes[i]->updateAggInterval(width);
  }
}

vector<defInst*> instrToDo;

// returns false if the latest attempt to insert failed
void handleDeferredInstr(machineMetFocusNode *machnode, pd_Function *func)
{
  // Check if we have already created the defInst object for 
  // instrumenting the function later 
  bool previouslyDeferred = false;
  // number of functions for which instrumentation was deferred
  int numDeferred = instrToDo.size();
  
  for (int i=0; i < numDeferred; i++) {
    //    cerr << "handling deferred " << i+1 << " of " << numDeferred 
    //	 << ", id: " << instrToDo[i]->id() << "\n";
    if (instrToDo[i]->id() == machnode->getMetricID()) {
      previouslyDeferred = true;
      instrToDo[i]->failedAttempt();
      
      if (instrToDo[i]->numAttempts() == 0) {
	instrToDo[i]->func()->setRelocatable(false);
      }
      
      // latest attempt to insert instrumentation failed.
      break;
    }
  }
  
  // Create defInst object so instrumentation can be inserted later.
  // number of attempts at relocation is 1000
  if (!previouslyDeferred) {
    assert(func != NULL);
    defInst *di = new defInst(machnode->getMetricID(), func, 1000);
	instrToDo.push_back(di);
  }
      
  // Don't delete the metricDefinitionNode because we want to
  // reuse it later. 
}

void machineMetFocusNode::pauseProcesses() {
  assert(currentlyPausedProcs.size() == 0);
  for (unsigned u=0; u<procNodes.size(); u++) {
    process *p = procNodes[u]->proc();

    if (p->status() == running) {
#ifdef DETACH_ON_THE_FLY
      if (p->reattachAndPause())
#else
      if (p->pause())
#endif
	currentlyPausedProcs += p;
    }
  }
}

void machineMetFocusNode::continueProcesses() {
   for (unsigned u=0; u<currentlyPausedProcs.size(); u++) {
#ifdef DETACH_ON_THE_FLY
      currentlyPausedProcs[u]->detachAndContinue();
#else
      currentlyPausedProcs[u]->continueProc();
#endif
   }
   currentlyPausedProcs.resize(0);
}

bool machineMetFocusNode::insertInstrumentation() {
   pd_Function *func = NULL;
   bool inserted = loadInstrIntoApp(&func);
   
   // Instrumentation insertion may have failed. We should handle this case
   // better than an assert. Currently we handle the case where (on x86) we
   // couldn't insert the instrumentation due to relocation failure (deferred
   // instrumentation). If it's deferred, come back later.
  
   if(inserted == false) {
      // instrumentation was deferred
      if (hasDeferredInstr()) {
	handleDeferredInstr(this, func);
	return false;
      } // end deferred handling
      // Instrumentation failed for an unknown reason. Very bad case.
      else {
	delete this;
	return false;
      }
   }

   mapSampledDRNs2ThrNodes();
   insertJumpsToTramps();

   // Now that the timers and counters have been allocated on the heap, and
   // the instrumentation added, we can manually execute instrumentation we
   // may have processed at function entry points and pre-instruction call
   // sites which have already executed.
   doCatchupInstrumentation();
   return true;
}

bool machineMetFocusNode::loadInstrIntoApp(pd_Function **func) {
  // returns true iff successful
  if (instrLoaded()) {
    return true;
  }
  
  bool aCompFailedToInsert = false;
  for (unsigned i=0; i<procNodes.size(); i++) {
    if (! procNodes[i]->loadInstrIntoApp(func)) {
      aCompFailedToInsert = true;
      break;
    }
  }
  if(aCompFailedToInsert) {
    //cerr << "machNode::load, failed insertion, returning false\n";
    return false;
  }
  return true;
}

bool machineMetFocusNode::instrLoaded() {
  if(procNodes.size() == 0)  return false;

  bool allCompInserted = true;
  for(unsigned i=0; i<procNodes.size(); i++) {
    bool result = procNodes[i]->instrLoaded();
    if(result == false) {
      allCompInserted = false; 
      break;
    }
  }
  return allCompInserted;
}


void machineMetFocusNode::mapSampledDRNs2ThrNodes() {
  for(unsigned i=0; i<procNodes.size(); i++) {
    procNodes[i]->mapSampledDRNs2ThrNodes();
  }
}

bool machineMetFocusNode::baseTrampsHookedUp() {
  if(procNodes.size() == 0)  return false;

  bool hookedUp = true;
  for(unsigned i=0; i<procNodes.size(); i++) {
    if(procNodes[i]->baseTrampsHookedUp() == false) {
      hookedUp = false;
      break;
    }
  }
  return hookedUp;
}


bool machineMetFocusNode::insertJumpsToTramps() {
   // Patch up the application to make it jump to the base trampoline(s) of
   // this metric.  (The base trampoline and mini-tramps have already been
   // installed in the inferior heap).  We must first check to see if it's
   // safe to install by doing a stack walk, and determining if anything on
   // it overlaps with any of our desired jumps to base tramps.  The key
   // variable is "returnsInsts", which was created for us when the base
   // tramp(s) were created.  Essentially, it contains the details of how
   // we'll jump to the base tramp (where in the code to patch, how many
   // instructions, the instructions themselves).  Note that it seems this
   // routine is misnamed: it's not instrumentation that needs to be
   // installed (the base & mini tramps are already in place); it's just the
   // last step that is still needed: the jump to the base tramp.  If one or
   // more can't be added, then a TRAP insn is inserted in the closest common
   // safe return point along the stack walk, and some structures are
   // appended to the process' "wait list", which is then checked when a TRAP
   // signal arrives.  At that time, the jump to the base tramp is finally
   // done.  WARNING: It seems to me that such code isn't thread-safe...just
   // because one thread hits the TRAP, there may still be other threads that
   // are unsafe.  It seems to me that we should be doing this check again
   // when a TRAP arrives...but for each thread (right now, there's no stack
   // walk for other threads).  --ari
   if(baseTrampsHookedUp()) {
     return true;
   }

   bool allInserted = true;
   for(unsigned i=0; i<procNodes.size(); i++) {
      bool result = procNodes[i]->insertJumpsToTramps();
      if(result == false)
	allInserted = false;
   }
   return allInserted;
}

void machineMetFocusNode::updateAggInterval(timeLength width) {
  aggregator.changeAggIntervalWidth(width);

  for(unsigned i=0; i<procNodes.size(); i++) {
    procNodes[i]->updateAggInterval(width);
  }
}

timeLength machineMetFocusNode::cost() const {
   timeLength largestCost = timeLength::Zero();   

   for(unsigned i=0; i<procNodes.size(); i++) {
      timeLength procCost = procNodes[i]->cost();
      if(procCost > largestCost) 
	largestCost = procCost;
   }
   return largestCost;
}

void machineMetFocusNode::print() {
   cerr << getFullName() << "\n";
   cerr << "M:" << (void*)this << "\n"; 

   for(unsigned i=0; i<procNodes.size(); i++)
      procNodes[i]->print();
}

void machineMetFocusNode::sendInitialActualValue(pdSample s) {
  double valToSend = static_cast<double>(s.getValue());
  tp->setInitialActualValueFE(getMetricID(), valToSend);
  //cerr << "  sending init act value, id: " << getMetricID() << ", val: "
  //   << valToSend << "\n";
  _sentInitialActualValue = true;
}

void machineMetFocusNode::forwardSimpleValue(timeStamp start, timeStamp end,
					     pdSample value)
{
  // TODO mdc
  sampleVal_cerr << "forwardSimpleValue - st: " << start << "  end: " << end 
		 << "  value: " << value << "\n";

  assert(start >= getFirstRecordTime());
  assert(end > start);

  batchSampleData(getMetName(), id_, start, end, value);
}



void machineMetFocusNode::updateWithDeltaValue(timeStamp startTime,
			 timeStamp sampleTime, pdSample value) {
  assert(startTime.isInitialized());
  sampleVal_cerr << "Batching st: " << startTime << ", end: " << sampleTime 
		 <<", val: " << value << "\n";

  batchSampleData(getMetName(), id_, startTime, sampleTime, value);
  return;
}


void machineMetFocusNode::tryAggregation() {
  sampleInterval aggSample;

  while(aggregator.aggregate(&aggSample) == true) {
    //cerr << "machNode- got an aggregate sample, val: " << aggSample.value 
    // << "\n";
    if(!sentInitialActualValue()) {
      //      cerr << "  sending init act value: " 
      //	   << aggregator.getInitialActualValue() << "\n";
      sendInitialActualValue(aggregator.getInitialActualValue());
    }
    updateWithDeltaValue(aggSample.start, aggSample.end, aggSample.value);
  }
}

bool machineMetFocusNode::hasDeferredInstr() {
  bool hasDeferredComp = false;

  for(unsigned i=0; i<procNodes.size(); i++) {
    if(procNodes[i]->hasDeferredInstr()) {
      hasDeferredComp = true;
      break;
    }
  }
  return hasDeferredComp;
}

void machineMetFocusNode::addPart(processMetFocusNode* procNode)
{
  procNodes.push_back(procNode);
  aggComponent *childAggInfo = aggregator.newComponent();
  procNode->recordAsParent(this, childAggInfo);
}
