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

#include "paradynd/src/metricFocusNode.h"
#include "paradynd/src/machineMetFocusNode.h"
#include "paradynd/src/processMetFocusNode.h"
#include "paradynd/src/dynrpc.h"
#include "paradynd/src/comm.h"
#include "dyninstAPI/src/symtab.h"
#include "paradynd/src/internalMetrics.h"
#include "paradynd/src/costmetrics.h"
#include "pdutil/h/pdDebugOstream.h"
#include "paradynd/src/focus.h"
#include "paradynd/src/init.h"
#include "paradynd/src/pd_process.h"

extern pdDebug_ostream sampleVal_cerr;
extern pdRPC *tp;

dictionary_hash<unsigned, machineMetFocusNode*> 
                                machineMetFocusNode::allMachNodes(uiHash);

machineMetFocusNode::machineMetFocusNode(int metricID, 
					 const string& metric_name, 
					 const Focus &foc,
					 vector<processMetFocusNode*>& parts, 
					 aggregateOp agg_op, bool enable_)
  : metricFocusNode(), aggregator(agg_op, getCurrSamplingRate()), 
    id_(metricID), aggOp(agg_op),
    _sentInitialActualValue(false), met_(metric_name), focus_(foc), 
    enable(enable_), is_internal_metric(false), 
    cbi(NULL), isBeingDeleted(false)
{
  allMachNodes[metricID] = this;

  for (unsigned u=0; u<parts.size(); u++) {
    addPart(parts[u]);
  }
}

machineMetFocusNode::~machineMetFocusNode() {
  if (isBeingDeleted) return;
  isBeingDeleted = true;
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
  delete cbi;
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

// if auto_delete is true, then if function action causes no procNodes
// to remain, then the machineMetFocusNode will be deleted too
void machineMetFocusNode::deleteProcNode(processMetFocusNode *procNode,
                                         bool auto_delete_mach_node) {
   vector<processMetFocusNode*>::iterator itr = procNodes.end();

   while(itr != procNodes.begin()) {
      itr--;
      if(procNode == (*itr)) {
         delete procNode;
         procNodes.erase(itr);
      }
   } 

   if(auto_delete_mach_node == true && procNodes.size() == 0) {
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

void machineMetFocusNode::initializeForSampling(timeStamp startTime, 
						pdSample initValue)
{
  initAggInfoObjects(startTime, initValue);
  prepareForSampling();  
}

// if a thread or process is added to the parent metFocusNode,
// the addThread or "addProcess" function will handle initializing
// these nodes aggInfoObjects
void machineMetFocusNode::initAggInfoObjects(timeStamp startTime, 
					     pdSample initValue)
{
  //cerr << "machNode (" << (void*)this << ") initAggInfo\n";
  for(unsigned i=0; i<procNodes.size(); i++) {
    procNodes[i]->initAggInfoObjects(startTime, initValue);
  }

  if(! machStartTime.isInitialized())
    machStartTime = startTime;
}


void machineMetFocusNode::updateAllAggInterval(timeLength width) {
  vector<machineMetFocusNode *> machNodes;
  getMachineNodes(&machNodes);

  for(unsigned i=0; i<machNodes.size(); i++) {
    machNodes[i]->updateAggInterval(width);
  }
}

instr_insert_result_t machineMetFocusNode::insertInstrumentation() {
   bool deferred = false;

   for(unsigned i=0; i<procNodes.size(); i++) {
      // processMetFocusNode::insertInstrumentation pauses and continues
      // it's process when needed
      instr_insert_result_t status = procNodes[i]->insertInstrumentation();
      if(status == insert_failure) {
	 return insert_failure;
      } else if(status == insert_deferred) {
	 deferred = true;
	 // continue inserting remaining the processMetFocusNodes
      }
   }

   if(deferred == true)  return insert_deferred;

   return insert_success;
}

void machineMetFocusNode::prepareForSampling() {
  for(unsigned i=0; i<procNodes.size(); i++) {
    procNodes[i]->prepareForSampling();
  }
}

bool machineMetFocusNode::instrInserted() {
   bool allInserted = true;
   for(unsigned i=0; i<procNodes.size(); i++) {
      if(! procNodes[i]->instrInserted()) {
	 allInserted = false;
	 break;
      }
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
    if(!sentInitialActualValue()) {
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

void machineMetFocusNode::propagateToNewProcess(pd_process *newProcess) {
  // see if this metric-focus needs to be adjusted for this new process
  if(isInternalMetric()) {
    return;
  }

  const Focus node_focus = getFocus();
  string this_machine = getNetworkName();

  // do the propagation if the focus is all_machines or it is this machine
  // specifically with no other process defined (a process defined in the
  // focus wouldn't be the same process as the new process, since the new 
  // process wouldn't have been around to be selected)
  if(! (node_focus.allMachines() || 
	(node_focus.get_machine() == this_machine && 
	                   !node_focus.process_defined())))  
  {
    return;
  }

  processMetFocusNode *procNode = 
    makeProcessMetFocusNode(node_focus, getMetName(), newProcess, false, 
			    isEnabled());
  if(procNode==NULL)
    return;

  addPart(procNode);

  addCurrentPredictedCost(procNode->cost());

  instr_insert_result_t insert_status = procNode->insertInstrumentation(); 
  if(insert_status == insert_deferred) {
     return ;
  } else if(insert_status == insert_failure) {
     return ;
  }

  // There may be other procNodes in machNode with deferred instrumentation.
  // If this is the case, then the machNode will be marked as !instrInserted.
  if(instrInserted())
     procNode->initializeForSampling(getWallTime(), pdSample::Zero());
}

void machineMetFocusNode::setupProcNodeForForkedProcess(
		      processMetFocusNode *parentProcNode, 
		      pd_process *childProc,
		      vector<processMetFocusNode *> *procNodesToUnfork)
{
  processMetFocusNode *childProcNode = 
    new processMetFocusNode(*parentProcNode, childProc);

  const Focus &node_focus = getFocus();
  string this_machine = getNetworkName();

  // do the propagation if the focus is all_machines or it is this machine
  // specifically with no other process defined (a process defined in the
  // focus wouldn't be the same process as the new process, since the
  // new process wouldn't have been around to be selected)
  if(! (node_focus.allMachines() || 
	(node_focus.get_machine() == this_machine && 
	                     ! node_focus.process_defined()))) 
  {
    // since this metric-focus isn't relevant for this process,
    // remove the (inherited) instrumentation within the child process
    // which is associated with this metric-focus

    // we need to do the unForking (ie. instrumentation deletion) after we've
    // gone through all of the metric-focuses first, because some other
    // metric-focus might need this instrumentation in there.  If we deleted
    // it now, we'd have no way to put it back in (since this is a forked
    // process and we don't have all of the information we have for a freshly
    // created process).
    (*procNodesToUnfork).push_back(childProcNode);
    return;
  }

  addPart(childProcNode);
  addCurrentPredictedCost(childProcNode->cost());

  // don't do "childProcNode->insertInstrumentation()" since already 
  // instrumentation has already been inserted, it was inherited from the
  // parent process

  // don't have to worry about instrumentation insertion being deferred
  // since it's already in (ie. been inherited into) the child process
  childProcNode->initializeForSampling(getWallTime(), pdSample::Zero());
}

void machineMetFocusNode::propagateToForkedProcess(
			   const pd_process *parentProc, pd_process *childProc,
			      vector<processMetFocusNode *> *procNodesToUnfork)
{
  // see if this metric-focus needs to be adjusted for this new process
  if(isInternalMetric()) {
    return;
  }

  // we need to adjust the metric-focuses which will include 
  for(unsigned i=0; i<procNodes.size(); i++) {
    if(procNodes[i]->proc()->getPid() != parentProc->getPid()) continue;
    processMetFocusNode *parentProcNode = procNodes[i];
    setupProcNodeForForkedProcess(parentProcNode, childProc, 
				  procNodesToUnfork);
  }
}

void machineMetFocusNode::adjustForExecedProcess(pd_process *proc) {
   // see if this metric-focus needs to be adjusted for this new process
   if(isInternalMetric()) {
      return;
   }

   const Focus focus = getFocus();

   // we need to adjust the metric-focuses which will include 
   for(unsigned i=0; i<procNodes.size(); i++) {
      processMetFocusNode *procNode = procNodes[i];

      if(procNode->proc()->getPid() != proc->getPid()) continue;
      deleteProcNode(procNode, false);

      // we won't propagate over metric-focuses with a source level focus (or
      // threads) since these may not be defined in the new process
      if(focus.module_defined() || focus.function_defined() || 
         focus.thread_defined())
         continue;

      propagateToNewProcess(proc);
   }
}




