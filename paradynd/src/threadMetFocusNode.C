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

#include "paradynd/src/threadMetFocusNode.h"
#include "paradynd/src/processMetFocusNode.h"
#include "pdutil/h/pdDebugOstream.h"
#include "dyninstAPI/src/pdThread.h"


extern pdDebug_ostream sampleVal_cerr;


dictionary_hash<string, threadMetFocusNode_Val*> 
             threadMetFocusNode::allThrMetFocusNodeVals(string::hash);


threadMetFocusNode *threadMetFocusNode::newThreadMetFocusNode(
		    const string &metric_name, const Focus &f, pdThread *pdthr)
{
  threadMetFocusNode_Val *nodeVal;
  string key_name = threadMetFocusNode_Val::construct_key_name(metric_name, 
							       f.getName());
  bool foundIt = 
    threadMetFocusNode::allThrMetFocusNodeVals.find(key_name, nodeVal);

  if(! foundIt) {
    nodeVal = new threadMetFocusNode_Val(metric_name, f, pdthr);
    allThrMetFocusNodeVals[key_name] = nodeVal;
  }
  nodeVal->incrementRefCount();
  threadMetFocusNode *thrNode = new threadMetFocusNode(nodeVal);

  /*
  cerr << "newThreadMetFocusNode" << key_name << " (" << (void*)thrNode << ")";
  if(foundIt)
    cerr << ", found one ( " << (void*)nodeVal << "), using it\n";
  else
    cerr << ", not found, creating one (" << (void*)nodeVal << ")\n";
  */

  return thrNode;
}

threadMetFocusNode_Val::~threadMetFocusNode_Val() {
  for(unsigned i=0; i<parentsBuf.size(); i++) {
    processMetFocusNode *parent = parentsBuf[i].parent;
    parent->stopSamplingThr(this);
    removeParent(parent);
  }
}

threadMetFocusNode::~threadMetFocusNode() {
  V.decrementRefCount();

  if(V.getRefCount() == 0) {
    allThrMetFocusNodeVals.undef(V.getKeyName());
    delete &V;
  } else {
    V.removeParent(parent);
  }
}

void threadMetFocusNode::recordAsParent(processMetFocusNode *procNode, 
					aggComponent *childAggInfo) {
  assert(parent == NULL);
  parent = procNode;
  V.recordAsParent(procNode, childAggInfo);
}

void threadMetFocusNode::updateAllAggInfoInitialized() {
  bool allInitialized = true;
  for(unsigned i=0; i<V.parentsBuf.size(); i++) {
    aggComponent *curAggInfo = V.parentsBuf[i].childNodeAggInfo;
    if(! curAggInfo->isReadyToReceiveSamples())
      allInitialized = false;
  }
  V.allAggInfoInitialized = allInitialized;
}

void threadMetFocusNode::print() {
   cerr << "T:" << (void*)this << "\n";
}

bool threadMetFocusNode_Val::instrInserted() {
  bool allParentsInserted = true;
  for(unsigned i=0; i<parentsBuf.size(); i++) {
    processMetFocusNode *parent = parentsBuf[i].parent;
    if(parent->instrInserted() == false)
      allParentsInserted = false;
  }
  return allParentsInserted;
}

unsigned threadMetFocusNode_Val::getThreadID() const { 
  return pdThr->get_tid(); 
}
unsigned threadMetFocusNode_Val::getThreadPos() const { 
  return pdThr->get_pd_pos(); 
}

bool threadMetFocusNode_Val::isReadyForUpdates() {
  if(! instrInserted()) {
    sampleVal_cerr << "mi " << this << " hasn't been inserted\n";
    return false;
  }
  if(! hasAggInfoBeenInitialized()) {
    sampleVal_cerr << "mi " << this 
		   << " hasn't had it's agg info initialized yet\n";
    return false;
  }
  return true;
}

// takes an actual value as input, as opposed to a change in sample value
void threadMetFocusNode_Val::updateValue(timeStamp sampleTime, pdSample value)
{
  assert(value >= pdSample::Zero());
 
  sampleVal_cerr << "thrNode::updateValue() - mdn: " << (void*)this
		 << ", value: " << value << ", cumVal: " << cumulativeValue 
		 << "\n";

  if (value < cumulativeValue) {
  // only use delta from last sample.
    value = cumulativeValue;
  }

  value -= cumulativeValue;
  cumulativeValue += value;
  timeStamp uninitializedTime;  // we don't know the start of the interval here
  updateWithDeltaValue(uninitializedTime, sampleTime, value);
}

extern bool adjustSampleIfNewMdn(pdSample *sampleVal, timeStamp startTime,
				 timeStamp sampleTime, timeStamp MFstartTime);

void threadMetFocusNode_Val::updateWithDeltaValue(timeStamp startTime,
			timeStamp sampleTime, pdSample value) {
  for(unsigned i=0; i<parentsBuf.size(); i++) {
    processMetFocusNode *curParent = parentsBuf[i].parent;
    aggComponent *thrNodeAggInfo   = parentsBuf[i].childNodeAggInfo;

    pdSample valToPass = value;
    bool shouldAddSample = adjustSampleIfNewMdn(&valToPass, startTime, 
				  sampleTime, curParent->getStartTime());
    if(!shouldAddSample)   continue;
    //cerr << "thrNode::updateWithDeltaVal- (" << i+1 << ") addSamplePt, tm: " 
    // << sampleTime << ", val: " << valToPass << "\n";
    thrNodeAggInfo->addSamplePt(sampleTime, valToPass);
    curParent->tryAggregation();
  }
}

process *threadMetFocusNode_Val::proc() {
  if(parentsBuf.size() > 0) {
    return parentsBuf[0].parent->proc();
  } else
    return NULL;
}

void threadMetFocusNode_Val::recordAsParent(processMetFocusNode *procNode,
					    aggComponent *childAggInfo) {
  // make sure that any added parent is of the same process
  if(parentsBuf.size() > 0) {
    processMetFocusNode *aParent = parentsBuf[0].parent;
    assert(aParent->proc() == procNode->proc());
  }
  parentDataRec<processMetFocusNode> curRec;
  curRec.parent = procNode;
  curRec.childNodeAggInfo = childAggInfo;
  parentsBuf.push_back(curRec);
  if(! childAggInfo->isReadyToReceiveSamples())
    allAggInfoInitialized = false;
}

void threadMetFocusNode_Val::removeParent(processMetFocusNode *procNode) {
  for(int i=(int)(parentsBuf.size()-1); i>=0; i--) {
    if(parentsBuf[i].parent == procNode) {
      parentsBuf[i].childNodeAggInfo->markAsFinished();
      parentsBuf.erase(i);
    }
  }
}

string threadMetFocusNode_Val::getKeyName() {
  return construct_key_name(metric_name, focus.getName());
}

process *threadMetFocusNode::proc() {
  assert(parent != NULL);
  return parent->proc();
}

// initializes agg info objects between this THR node and parent PROC NODE
void threadMetFocusNode::initAggInfoObjects(timeStamp startTime, 
					    pdSample initValue)
{
  //cerr << "    thrNode (" << (void*)this << ") initAggInfo\n";
   for(unsigned i=0; i<V.parentsBuf.size(); i++) {  
      processMetFocusNode *curParent  = V.parentsBuf[i].parent;    
      if(parent == curParent) {
	 aggComponent *thrNodeAggInfo = V.parentsBuf[i].childNodeAggInfo;
	 if(! thrNodeAggInfo->isInitialized()) {
	   //cerr << "      initializing aggInfo " << (void*)thrNodeAggInfo
	   //<< "\n";
	    thrNodeAggInfo->setInitialStartTime(startTime);
	    thrNodeAggInfo->setInitialActualValue(initValue);
	 }
      }
   }
   updateAllAggInfoInitialized();
   //cerr << "    allAggInfoInitialized = " << V.hasAggInfoBeenInitialized() 
   //     << "\n";
}

void threadMetFocusNode::initializeForSampling(timeStamp startTime, 
					       pdSample initValue)
{
  initAggInfoObjects(startTime, initValue);
  parent->prepareForSampling(this);
}

