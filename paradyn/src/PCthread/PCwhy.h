/*
 * Copyright (c) 1993, 1994 Barton P. Miller, Jeff Hollingsworth,
 *     Bruce Irvin, Jon Cargille, Krishna Kunchithapadam, Karen
 *     Karavanic, Tia Newhall, Mark Callaghan.  All rights reserved.
 * 
 * This software is furnished under the condition that it may not be
 * provided or otherwise made available to, or used by, any other
 * person, except as provided for by the terms of applicable license
 * agreements.  No title to or ownership of the software is hereby
 * transferred.  The name of the principals may not be used in any
 * advertising or publicity related to this software without specific,
 * written prior authorization.  Any use of this software must include
 * the above copyright notice.
 *
 */

/*
 * PCwhy.h
 * 
 * The hypothesis class and the why axis.
 * 
 * $Log: PCwhy.h,v $
 * Revision 1.8  1996/02/02 02:07:36  karavan
 * A baby Performance Consultant is born!
 *
 */

#ifndef pc_why_h
#define pc_why_h

#include "PCintern.h"
#include "PCmetric.h"

typedef sampleValue (*thresholdFunction) (const char *tname, focus foo);
typedef void (*explanationFunction)(searchHistoryNode*);
typedef enum {gt, lt} compOperator;
class hypothesis;

class hypothesis {
  friend class experiment;
 public:
  hypothesis (const char *hypothesisName,
	      const char *pcMetricName, 
	      const char *thresholdName, thresholdFunction getThreshold,
	      compOperator compareOp,
	      explanationFunction explanation, bool *success,
	      vector<string*> *plums); 
  hypothesis (const char *hypothesisName,
	      explanationFunction explanation, 
	      bool *success); 
  const char *getThresholdName () {return thresholdNm.string_of();}
  const char *getName() {return name.string_of();}
  PCmetric *getPcMet() {return pcMet;}
  void addChild (hypothesis *child) {kids += child;}
  vector<hypothesis*> *expand();
  bool isVirtual() {return (pcMet == NULL);}
  bool isPruned(resourceHandle);
  bool prunesDefined() {return (pruneList.size() > 0);}
 private:
  string name;
  explanationFunction explain;
  PCmetric *pcMet;
  string thresholdNm;
  thresholdFunction getThreshold;
  compOperator compOp;
  vector <hypothesis*> kids;
  vector<resourceHandle> pruneList;
};

class whyAxis {
 public:
  whyAxis();
  bool addHypothesis(const char *hypothesisName,
		     const char *parentName,
		     const char *pcMetricName, 
		     const char *thresholdName, 
		     thresholdFunction getThreshold,
		     compOperator compareOp,
		     explanationFunction explanation,
		     vector<string*> *plumList); 
  bool addHypothesis(const char *hypothesisName,
		     const char *parentName,
		     explanationFunction explanation); 
  hypothesis *const getRoot () {return root;}
 private:
  dictionary_hash<string, hypothesis*> AllHypotheses;
  hypothesis *root;
};

#endif
