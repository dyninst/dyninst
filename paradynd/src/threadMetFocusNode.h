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

#ifndef THR_MET_FOCUS_NODE
#define THR_MET_FOCUS_NODE

#include "paradynd/src/metricFocusNode.h"
#include "common/h/Dictionary.h"
#include "paradynd/src/focus.h"

class processMetFocusNode;

template<class ParentType> class parentDataRec {
 public:
  ParentType *parent;
  aggComponent *childNodeAggInfo;
};

class threadMetFocusNode_Val {
  friend class threadMetFocusNode;

  enum { NON_THREAD = -1};
  const string &metric_name;
  const Focus focus;
  vector< parentDataRec<processMetFocusNode> > parentsBuf;
  pdSample cumulativeValue;
  bool allAggInfoInitialized;
  pdThread *pdThr;
  int referenceCount;

 public:
  static string construct_key_name(const string &metricStr, 
				   const string &focusStr) {
    return (metricStr + "-" + focusStr);
  }

  threadMetFocusNode_Val(const string &met, const Focus &f, pdThread *pdthr) : 
    metric_name(met), focus(f), cumulativeValue(pdSample::Zero()), 
    allAggInfoInitialized(false), pdThr(pdthr), referenceCount(0)
  { }
  ~threadMetFocusNode_Val();

  string getKeyName();
  void updateValue(timeStamp, pdSample);
  void updateWithDeltaValue(timeStamp startTime, timeStamp sampleTime, 
			    pdSample value);

  unsigned getThreadID() const;
  unsigned getThreadPos()const ;
  bool instrInserted();
  bool isReadyForUpdates();
  process *proc();
  void recordAsParent(processMetFocusNode *procNode,
		      aggComponent *childAggInfo);
  void removeParent(processMetFocusNode *parentNode);
  bool hasAggInfoBeenInitialized() { return allAggInfoInitialized; }
  void incrementRefCount() { referenceCount++; }
  void decrementRefCount() { referenceCount--; }
  int getRefCount() { return referenceCount; }
};


class threadMetFocusNode : public metricFocusNode {
 private:
  threadMetFocusNode_Val &V;
  processMetFocusNode *parent;

  void updateAllAggInfoInitialized();

 protected:
  static dictionary_hash<string, threadMetFocusNode_Val*> 
                                                    allThrMetFocusNodeVals;
  threadMetFocusNode(threadMetFocusNode_Val *thrValPtr) : V(*thrValPtr),
    parent(NULL) { }

 public:
  static threadMetFocusNode *threadMetFocusNode::newThreadMetFocusNode(
		   const string &metric_name, const Focus &f, pdThread *pdthr);

  virtual ~threadMetFocusNode();
  void recordAsParent(processMetFocusNode *procNode, 
		      aggComponent *childAggInfo);
  void print();
  process *proc();
  bool instrInserted()     { return V.instrInserted(); }
  bool isReadyForUpdates() { return V.isReadyForUpdates(); }
  int getThreadID()  const { return V.getThreadID(); }
  int getThreadPos() const { return V.getThreadPos(); }
  threadMetFocusNode_Val *getValuePtr() { return &V; }
  void initializeForSampling(timeStamp startTime, pdSample initValue);
  void initAggInfoObjects(timeStamp startTime, pdSample initValue);
};



#endif
