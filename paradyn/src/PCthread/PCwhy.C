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

// $Id: PCwhy.C,v 1.24 2004/03/23 01:12:28 eli Exp $
// The hypothesis class and the why axis.

#include "PCwhy.h"
#include "PCmetric.h"

hypothesis::hypothesis (const char *hypothesisName,
			const char *pcMetricName, 
			const char *pcMetric2Name,
			const char *indivThresholdName, 
			const char *groupThresholdName, 
			thresholdFunction threshold,
			compOperator compare,
			expandPolicy exPol,
			explanationFunction explanation, bool *success,
			pdvector<pdstring*> *plumList,
			pdvector<pdstring*> *suppressions) 
:name(hypothesisName), explain(explanation), 
 indivThresholdNm(indivThresholdName), groupThresholdNm(groupThresholdName), 
 getThreshold(threshold),
 compOp(compare), exType(exPol)
{ 
  pdstring mname = pcMetricName;
  pdstring mname2 = pcMetric2Name;
  if (! PCmetric::AllPCmetrics.defines(pcMetricName)) { 
    *success = false;
    return;
  }  
  pcMet = PCmetric::AllPCmetrics[mname];
  if (pcMetric2Name != NULL) {
    if (PCmetric::AllPCmetrics.defines(pcMetricName))
      pcMet2 = PCmetric::AllPCmetrics[mname2];
  }
  if (plumList != NULL) {
    // find and store resource handles for all pruned resources
    resourceHandle *rh;
    for (unsigned i = 0; i < plumList->size(); i++) {
      rh = dataMgr->findResource((*plumList)[i]->c_str());
      if (rh) {
	pruneList += *rh;
	delete rh;
      }
    }
  }
  if (suppressions != NULL) {
    // find and store resource handles for all suppressed resources
    resourceHandle *rh;
    for (unsigned i = 0; i < suppressions->size(); i++) {
      rh = dataMgr->findResource((*suppressions)[i]->c_str());
      if (rh) {
	suppressList += *rh;
	delete rh;
      }
    }
  }
  *success = true;
}

hypothesis::hypothesis (const char *hypothesisName,
			explanationFunction explanation, 
			bool *success) 
:name(hypothesisName), explain(explanation), 
 pcMet ((PCmetric *)NULL), indivThresholdNm((const char *)NULL), 
 groupThresholdNm((const char *)NULL), 
 getThreshold((pdRate (*)(const char *, unsigned int))NULL), 
 compOp(gt), exType (whereAndWhy)
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

bool 
hypothesis::isSuppressed(resourceHandle candidate)
{
  for (unsigned i = 0; i < suppressList.size(); i++) {
    if (suppressList[i] == candidate)
      return true;
  }
  return false;
}

pdvector<hypothesis*> *
hypothesis::expand()
{
  pdvector<hypothesis*> *result = new pdvector<hypothesis*> (kids);
  return result;
}

PCmetric *
hypothesis::getPcMet(bool amFlag) 
{
  if (amFlag && pcMet2) 
    return pcMet2;
  else
    return pcMet;
}

bool 
whyAxis::addHypothesis(const char *hypothesisName,
		       const char *parentName,
		       const char *pcMetricName,
		       const char *pcMetric2Name,
		       const char *indivThresholdName,
		       const char *groupThresholdName,
		       thresholdFunction getThreshold,
		       compOperator compareOp,
		       expandPolicy expandPol,
		       explanationFunction explanation,
		       pdvector<pdstring*> *plumList,
		       pdvector<pdstring*> *suppressions)
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
  hypothesis *newhypo = 
    new hypothesis (hypothesisName, pcMetricName, pcMetric2Name, 
		    indivThresholdName, groupThresholdName, 
		    getThreshold, compareOp, expandPol, explanation, 
		    &good, plumList, suppressions); 
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
     : AllHypotheses (pdstring::hash)
{
  bool good = true;
  pdstring rootName ("topLevelHypothesis");
  root = new hypothesis (rootName.c_str(), (explanationFunction)NULL, 
			 &good);
  AllHypotheses [rootName] = root; 
}
