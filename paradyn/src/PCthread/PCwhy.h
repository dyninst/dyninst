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
 * Revision 1.11  1996/07/22 21:19:50  karavan
 * added new suppress feature to hypothesis definition.
 *
 * Revision 1.10  1996/04/30 06:27:14  karavan
 * change PC pause function so cost-related metric instances aren't disabled
 * if another phase is running.
 *
 * fixed bug in search node activation code.
 *
 * added change to treat activeProcesses metric differently in all PCmetrics
 * in which it is used; checks for refinement along process hierarchy and
 * if there is one, uses value "1" instead of enabling activeProcesses metric.
 *
 * changed costTracker:  we now use min of active Processes and number of
 * cpus, instead of just number of cpus; also now we average only across
 * time intervals rather than cumulative average.
 *
 * Revision 1.9  1996/02/08 19:52:56  karavan
 * changed performance consultant's use of tunable constants:  added 3 new
 * user-level TC's, PC_CPUThreshold, PC_IOThreshold, PC_SyncThreshold, which
 * are used for all hypotheses for the respective categories.  Also added
 * PC_useIndividualThresholds, which switches thresholds back to use hypothesis-
 * specific, rather than categorical, thresholds.
 *
 * Moved all TC initialization to PCconstants.C.
 *
 * Switched over to callbacks for TC value updates.
 *
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
	      const char *pcMetric2Name,
	      const char *indivThresholdName, 
	      const char *groupThresholdName,
	      thresholdFunction getThreshold,
	      compOperator compareOp,
	      explanationFunction explanation, bool *success,
	      vector<string*> *plums,
	      vector<string*> *suppressions); 
  hypothesis (const char *hypothesisName,
	      explanationFunction explanation, 
	      bool *success); 
  const char *getThresholdName () {return indivThresholdNm.string_of();}
  const char *getName() {return name.string_of();}
  PCmetric *getPcMet(bool altFlag);
  void addChild (hypothesis *child) {kids += child;}
  vector<hypothesis*> *expand();
  bool isVirtual() {return (pcMet == NULL);}
  bool isPruned(resourceHandle);
  bool isSuppressed(resourceHandle);
  bool prunesDefined() {return (pruneList.size() > 0);}
 private:
  string name;
  explanationFunction explain;
  PCmetric *pcMet;
  PCmetric *pcMet2;
  string indivThresholdNm;
  string groupThresholdNm;
  thresholdFunction getThreshold;
  compOperator compOp;
  vector <hypothesis*> kids;
  vector<resourceHandle> pruneList;
  vector<resourceHandle> suppressList;
};

class whyAxis {
 public:
  whyAxis();
  bool addHypothesis(const char *hypothesisName,
		     const char *parentName,
		     const char *pcMetricName,
		     const char *pcMetric2Name,
		     const char *indivThresholdName,
		     const char *groupThresholdName,
		     thresholdFunction getThreshold,
		     compOperator compareOp,
		     explanationFunction explanation,
		     vector<string*> *plumList,
		     vector<string*> *suppressions); 
  bool addHypothesis(const char *hypothesisName,
		     const char *parentName,
		     explanationFunction explanation); 
  hypothesis *const getRoot () {return root;}
 private:
  dictionary_hash<string, hypothesis*> AllHypotheses;
  hypothesis *root;
};

#endif
