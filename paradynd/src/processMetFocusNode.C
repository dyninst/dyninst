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
#include "paradynd/src/instrThrDataNode.h"
#include "paradynd/src/dynrpc.h"
#include "dyninstAPI/src/process.h"
#include "pdutil/h/pdDebugOstream.h"
#include "dyninstAPI/src/pdThread.h"

extern pdDebug_ostream sampleVal_cerr;

vector<processMetFocusNode*> processMetFocusNode::allProcNodes;

processMetFocusNode::processMetFocusNode(process *p,
                       const vector< vector<string> >& component_foc,
		       aggregateOp agg_op, bool arg_dontInsertData)
  : aggregator(agg_op, getCurrSamplingRate()),
    parentNode(NULL), aggInfo(NULL), proc_(p), aggOp(agg_op), 
    aggInfoInitialized(false), component_focus(component_foc), 
    dontInsertData_(arg_dontInsertData), catchupNotDoneYet_(false)
{
  allProcNodes.push_back(this);
}

void processMetFocusNode::getProcNodes(vector<processMetFocusNode*> *procnodes)
{
  for(unsigned i=0; i<allProcNodes.size(); i++) {
    (*procnodes).push_back(allProcNodes[i]);
  }
}

void processMetFocusNode::getProcNodes(vector<processMetFocusNode*> *procnodes,
				       int pid)
{
  for(unsigned i=0; i<allProcNodes.size(); i++) {
    processMetFocusNode *curNode = allProcNodes[i];
    if(curNode->proc()->getPid() == pid)
      (*procnodes).push_back(curNode);
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

bool processMetFocusNode::baseTrampsHookedUp() {
  vector<instrCodeNode *> codeNodes;
  getAllCodeNodes(&codeNodes);

  if(codeNodes.size() == 0)  return false;

  bool hookedUp = true;
  for(unsigned i=0; i<codeNodes.size(); i++) {
    instrCodeNode *codeNode = codeNodes[i];
    if(codeNode->baseTrampsHookedUp() == false) {
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

  aggInfo->addSamplePt(sampleTime, valToPass);
  
  parentNode->tryAggregation();
}

processMetFocusNode *processMetFocusNode::newProcessMetFocusNode(
                       process *p, 
		       const vector< vector<string> >& component_foc,
		       aggregateOp agg_op, bool arg_dontInsertData)
{
  processMetFocusNode *procNode = 
    new processMetFocusNode(p, component_foc, agg_op, arg_dontInsertData);
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
	                      vp, threadsVec,
                              true, // fry existing component MI
	                      false);
   if (tempMachNode == NULL)
      return NULL; // failure
   
   assert(tempMachNode->isTopLevelMDN());

   // okay, it looks like we successfully created a new aggregate mi.
   // Of course, we're just interested in the (single) component mi contained
   // within it; it'll replace "this".
   
   vector<metricDefinitionNode *> comp = tempMachNode->getComponents();
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
      metricDefinitionNode *aggMI = aggregators[agglcv];
      
#if defined(MT_THREAD)
      if (THR_LEV == aggregators[agglcv]->getMdnType())
	 continue;
#endif
      
      bool found=false;
      vector<metricDefinitionNode *> aggComp = aggMI->getComponents();
      for (unsigned complcv=0; complcv < aggComp.size(); complcv++) {
	 if (aggComp[complcv] == this) {
	    aggComp[complcv] = 
	       static_cast<metricDefinitionNode*>(procNode);
	    
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

void processMetFocusNode::doCatchupInstrumentation() {
  if(! catchupNotDoneYet()) {
    return;
  }

  prepareCatchupInstr();
  vector<instrCodeNode *> codeNodes;
  getAllCodeNodes(&codeNodes);
  
  for(unsigned i=0; i<codeNodes.size(); i++) {
    if(codeNodes[i]->catchupInstrNeeded()) {
      codeNodes[i]->manuallyTrigger(getMetricID());  
    }
  }
}

bool processMetFocusNode::catchupInstrNeeded() const {
  bool needed = false;
  vector<const instrCodeNode *> codeNodes;
  getAllCodeNodes(&codeNodes);

  for(unsigned i=0; i<codeNodes.size(); i++) {
    if(codeNodes[i]->catchupInstrNeeded()) {
      needed = true;
      break;
    }
  }
  return needed;
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

void processMetFocusNode::prepareCatchupInstr() {
  vector< vector<Frame> > allStackWalks;

#if defined(MT_THREAD)
  proc()->walkAllStack(allStackWalks);
  
#else
  vector<Frame> stackWalk;
  proc()->walkStack(proc()->getActiveFrame(), stackWalk);
  allStackWalks.push_back(stackWalk);
#endif
  metricVarCodeNode->prepareCatchupInstr(allStackWalks);
}

void processMetFocusNode::initAggInfoObjects(timeStamp startTime, 
					     pdSample initValue)
{
  //cerr << "  processMF::initAggInfo, startT: " << startTime << "\n";

  // Initialize aggComponents between PROC <-> THR nodes
  for(unsigned k=0; k<aggregator.numComponents(); k++) {
    aggComponent *curAggInfo = aggregator.getComponent(k);
    //cerr << "    initializing aggComponent: " << (void*)curAggInfo << "\n";
    curAggInfo->setInitialStartTime(startTime);
    curAggInfo->setInitialActualValue(initValue);
  }

  aggInfoInitialized = true;
  procStartTime = startTime;

  for(unsigned j=0; j<thrNodes.size(); j++) {
    thrNodes[j]->updateAllAggInfoInitialized();
  }
}

bool processMetFocusNode::loadInstrIntoApp(pd_Function **func) {
   if(instrLoaded())
      return true;

   vector<instrCodeNode *> codeNodes;
   getAllCodeNodes(&codeNodes);

   // mark remaining prim. components as deferred if we come upon
   // one deferred component
   bool aCompWasDeferred = false;
   for (unsigned j=0; j<codeNodes.size(); j++) {
      instrCodeNode *codeNode = codeNodes[j];
      if (!codeNode->loadInstrIntoApp(func)) {
	 if(codeNode->hasDeferredInstr()) {
	    aCompWasDeferred = true;
	    codeNode->markAsDeferred();
	    break;
	 }
      }
   }
   if(aCompWasDeferred) {
      return false;
   }
   
   catchupNotDoneYet_ = true;
   return true;
}

void processMetFocusNode::mapSampledDRNs2ThrNodes() {
  metricVarCodeNode->mapSampledDRNs2ThrNodes(thrNodes);
}

void processMetFocusNode::stopSamplingThr(threadMetFocusNode_Val *thrNodeVal) {
  if(dontInsertData()) return;

  assert(metricVarCodeNode != NULL);
  metricVarCodeNode->stopSamplingThr(thrNodeVal);
}

vector<const dataReqNode*> processMetFocusNode::getFlagDRNs(int thr_id) const {
  vector<const dataReqNode*> buff;
  for(unsigned i=0; i<constraintCodeNodes.size(); i++) {
    const dataReqNode *drn = constraintCodeNodes[i]->getFlagDRN(thr_id);
    buff.push_back(drn);
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

bool processMetFocusNode::insertJumpsToTramps() {
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
  aggInfo->markAsFinished();

  // thrNode destruction needs to occur after the destruction of the codeNodes
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
  
#if defined(MT_THREAD)
  if (proc()) {
    unsigned tSize = proc()->allMIComponentsWithThreads.size();
    for (unsigned u=0; u<tSize; u++) 
      if (proc()->allMIComponentsWithThreads[u] == this) {
	proc()->allMIComponentsWithThreads[u] = proc()->allMIComponentsWithThreads[tSize-1];
	proc()->allMIComponentsWithThreads.resize(tSize-1);
	break ;
      }
  }
#endif
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
  part->recordAsParent(this);
}

void processMetFocusNode::addConstraintCodeNode(instrCodeNode* part) {
  constraintCodeNodes.push_back(part);
  part->recordAsParent(this);
}

#if defined(MT_THREAD)
void processMetFocusNode::addThread(pdThread *thr)
{
  // ----------  This function needs a major rewrite  -----------------
  int tid;
  assert(thr);
  //assert(mdn_type_ == COMP_MDN) ; -- bhs
  tid = thr->get_tid();

  cerr << "+++++ adding thread " << tid << " to component w/ pid: "
       << proc()->getPid() << "\n";

  /*  
#if defined(TEST_DEL_DEBUG)
  sprintf(errorLine,"+++++ adding thread %d to component %s",tid,flat_name_);
  cerr << errorLine << endl ;
#endif
  */

  string pretty_name = string(thr->get_start_func()->prettyName().string_of()) ;
  string thrName = string("thr_") + tid + string("{") + pretty_name + string("}");

  instrThrDataNode *dNode = metricVarCodeNode->getThrDataNode(thrName);
  
  if (dNode) {
    cerr << "+++ metric already exist in "  << parentNode->getFullName() 
	 << "::addThread, " << endl;
    // << component_flat_name_thr.string_of() << endl ;
    return;
  }

  // component hasn't been defined previously. If it has, then we will have
  // reused it - naim

  // use stuff memorized in COMP_MDN: type_thr, temp_ctr_thr, flag_cons_thr and base_use_thr

  if(base_use_thr == NULL) {
    // allocate constraints that is used as flags
    unsigned flag_size = flag_cons_thr.size(); // could be zero
    // for flags only
    assert(flag_size == constraintCodeNodes.size());

    if (flag_size > 0) { 
      //unsigned thr_size = components[flag_size]->components.size();
      for (unsigned fs=0; fs<flag_size; fs++) {
	 instrCodeNode *consCodeNode = constraintCodeNodes[fs];

	// the following tests if flag prims have already been updated
	// assume one thread is added at a time and at that time all related metrics are updated
	// if (thr_size != cons_prim->components.size()) {
	// assert(thr_size+1 == cons_prim->components.size());
	// continue;
	// }
	if (consCodeNode->getThrDataNode(thrName)) {
	  // assert(thr_size+1 == cons_prim->components.size());
	  continue;
	}

	// if (!(flag_cons_thr[fs]->replace()))
	/*  This needs to be revamped
	string cons_met_thr(cons_prim->getMetName());
	vector< vector<string> > cons_component_focus_thr(cons_prim->getComponentFocus());
	vector< vector<string> > cons_focus_thr(cons_prim->getFocus());

	for (unsigned i=0;i<cons_component_focus_thr.size();i++) {
	  if (cons_component_focus_thr[i][0] == "Machine")
	    cons_component_focus_thr[i] += thrName;
	}
	for (unsigned j=0;j<cons_focus_thr.size();j++) {
	  if (cons_focus_thr[j][0] == "Machine")
	    cons_focus_thr[j] += thrName;
	}
	string cons_flat_name_thr = metricAndCanonFocus2FlatName(cons_met_thr,
                                                     cons_component_focus_thr);
	*/

	/*
	indivThreadMetFocusNode *thr_mn = 
	  new indivThreadMetFocusNode(proc(), getAggOp());
	assert(thr_mn);

	string tmp_tname(thrName);
	consCodeNode->addThrName(tmp_tname);
	consCodeNode->addPart(static_cast<metricDefinitionNode*>(thr_mn));

	thr_mn->setInstalled(true);
	*/
	// --bhs
	//	thr_mn->addSampledIntCounter(thr, 0, dontInsertData_thr, true) ; // should be false?
      }
    }
  }

  // for metric only (or base_use)
  // if base_use_thr != NULL;  NEED TO CHECK if base_use_thr[?]->replace() ??
  // add to components[components.size()-1]
  /*
  string met_thr(metric_prim->getMetName());
  vector< vector<string> > component_focus_thr(
        				    metric_prim->getComponentFocus());
  vector< vector<string> > focus_thr(metric_prim->getFocus());

  for (unsigned i=0;i<component_focus_thr.size();i++) {
    if (component_focus_thr[i][0] == "Machine")
      component_focus_thr[i] += thrName;
  }
  for (unsigned j=0;j<focus_thr.size();j++) {
    if (focus_thr[j][0] == "Machine")
      focus_thr[j] += thrName;
  }
  string component_flat_name_thr = metricAndCanonFocus2FlatName(met_thr,component_focus_thr);
*/
  indivInstrThrDataNode *dataNode =new indivInstrThrDataNode(metricVarCodeNode,
							      false, thr);

  string tmp_tname(thrName);
  //metric_p->addThrName(tmp_tname);
  //metric_prim->addDataNode(dataNode);

  //dataNode->setInstalled(true);

  // Create the timer, counter for this thread
  extern dataReqNode *create_data_object(unsigned, instrThrDataNode *,
					 pdThread *, dataInstHandle *);
  dataInstHandle handle;
  create_data_object(type_thr, dataNode, thr, &handle);

  // Create the temporary counters - are these useful
  for (unsigned tc=0; tc<temp_ctr_thr.size(); tc++) {
    // "true" means that we are going to create a sampled int counter but
    // we are *not* going to sample it, because it is just a temporary
    // counter - naim 4/22/97
    // By default, the last parameter is false - naim 4/23/97
    dataInstHandle handle;
    dataNode->createTemporaryCounter(thr, 0, &handle);
  }

  //if(! thrNode->hasAggInfoBeenInitialized())
  //thrNode->initAggInfoObjects(getWallTime(), pdSample::Zero());

  // FIXME: want to start catchup here for whole program

  if (catchupInstrNeeded()) {
    process *theProc = proc();
    assert(theProc);
    
    bool needToContinue = (theProc->status_ == running);
    bool ok;
    if (needToContinue) {
#ifdef DETACH_ON_THE_FLY
      ok = theProc->reattachAndPause();
#else
      ok = theProc->pause();
#endif
    }

    //manuallyTrigger();  --bhs

    if (needToContinue) {
      // the continue will trigger our code
#ifdef DETACH_ON_THE_FLY
      ok = theProc->detachAndContinue();
#else
      ok = theProc->continueProc();
#endif
    }
  }

}

void processMetFocusNode::deleteThread(pdThread *thr)
{
  /*  This needs a rewrite
  assert(mdn_type_ == COMP_MDN);
  int tid;
  assert(thr);
  tid = thr->get_tid();

  for (unsigned u=0; u<components.size(); u++) {
    metricDefinitionNode * prim = components[u];
    assert(prim->getMdnType() == PRIM_MDN);

    unsigned tsize = prim->getComponents().size();
    assert(tsize == prim->getThrNames().size());

    string pretty_name = string(thr->get_start_func()->prettyName().string_of()) ;
    string thrName = string("thr_") + tid + string("{") + pretty_name + string("}");
    instrCodeNode *prim_alt = dynamic_cast<instrCodeNode*>(prim);
    instrThrDataNode *thr_mi = prim_alt->getThrDataNode(thrName);

    if (thr_mi) {
      assert(thr_mi->getMdnType() == THR_LEV);

      thr_mi->removeThisInstance();  // removeThisInstance
      // delete thr_mi
    }
  }
  */  
#if defined(TEST_DEL_DEBUG)
  sprintf(errorLine,"----- deleting thread %d to pid %d\n",tid,
	  proc()->getPid());
  logLine(errorLine);
#endif

}

#endif //MT_THREAD
