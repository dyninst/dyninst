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
 * PCwhy.C
 * 
 * The hypothesis class and the why axis.
 * 
 * $Log: PCwhy.C,v $
 * Revision 1.12  1996/02/08 19:52:52  karavan
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
 * Revision 1.11  1996/02/02 02:06:51  karavan
 * A baby Performance Consultant is born!
 *
 */

#include "PCwhy.h"
#include "PCmetric.h"

hypothesis::hypothesis (const char *hypothesisName,
			const char *pcMetricName, 
			const char *indivThresholdName, 
			const char *groupThresholdName, 
			thresholdFunction threshold,
			compOperator compare,
			explanationFunction explanation, bool *success,
			vector<string*> *plumList) 
:name(hypothesisName), explain(explanation), 
 indivThresholdNm(indivThresholdName), groupThresholdNm(groupThresholdName), 
 getThreshold(threshold),
 compOp(compare)
{ 
  string mname = pcMetricName;
  if (! PCmetric::AllPCmetrics.defines(pcMetricName)) { 
    *success = false;
    return;
  }  
  pcMet = PCmetric::AllPCmetrics[mname];
  if (plumList != NULL) {
    resourceHandle *rh;
    for (unsigned i = 0; i < plumList->size(); i++) {
      rh = dataMgr->findResource((*plumList)[i]->string_of());
      if (rh)
	pruneList += *rh;
    }
  }
  *success = true;
}

hypothesis::hypothesis (const char *hypothesisName,
			explanationFunction explanation, 
			bool *success) 
:name(hypothesisName), explain(explanation), 
 pcMet (NULL), indivThresholdNm(NULL), groupThresholdNm(NULL), 
 getThreshold(NULL), compOp(gt)
{
  *success = true;
}

bool 
hypothesis::isPruned(resourceHandle candidate)
{
  for (unsigned i = 0; i < pruneList.size(); i++) {
    if (pruneList[i] == candidate)
      return true;
  }
  return false;
}

vector<hypothesis*> *
hypothesis::expand()
{
  vector<hypothesis*> *result = new vector<hypothesis*> (kids);
  return result;
}

bool 
whyAxis::addHypothesis(const char *hypothesisName,
		       const char *parentName,
		       const char *pcMetricName, 
		       const char *indivThresholdName,
		       const char *groupThresholdName,
		       thresholdFunction getThreshold,
		       compOperator compareOp,
		       explanationFunction explanation,
		       vector<string*> *plumList)
{
  hypothesis *mom;
  if (parentName == NULL) 
    mom = topLevelHypothesis;
  else {
    if (! AllHypotheses.defines(parentName))
      return false;
    mom = AllHypotheses [parentName];
  }
  bool good = true;
  hypothesis *newhypo = new hypothesis (hypothesisName,
			pcMetricName, indivThresholdName, groupThresholdName, 
			getThreshold, compareOp,
			explanation, &good, plumList); 
  if (! (good))
    return false;
  mom->addChild (newhypo);
  AllHypotheses [hypothesisName] = newhypo;
  return true;
}

bool 
whyAxis::addHypothesis(const char *hypothesisName,
		       const char *parentName,
		       explanationFunction explanation)
{
  hypothesis *mom;
  if (parentName == NULL) 
    mom = topLevelHypothesis;
  else {
    if (! AllHypotheses.defines(parentName))
      return false;
    mom = AllHypotheses [parentName];
  }
  bool good = true;
  hypothesis *newhypo = new hypothesis (hypothesisName,
			explanation, &good); 
  if (! (good))
    return false;
  mom->addChild (newhypo);
  AllHypotheses [hypothesisName] = newhypo;
  return true;
}

whyAxis::whyAxis()
     : AllHypotheses (string::hash)
{
  bool good = true;
  string rootName ("topLevelHypothesis");
  root = new hypothesis (rootName.string_of(), NULL, &good);
  AllHypotheses [rootName] = root; 
}
