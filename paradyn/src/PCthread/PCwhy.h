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

/*
 * PCwhy.h
 * 
 * The hypothesis class and the why axis.
 * 
 * $Log: PCwhy.h,v $
 * Revision 1.12  1996/08/16 21:03:49  tamches
 * updated copyright for release 1.1
 *
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
