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

// $Id: fastInferiorHeap.C,v 1.19 2002/04/23 18:58:38 schendel Exp $

#include <sys/types.h>
#include "common/h/Types.h"
#include "common/h/headers.h"
#include "fastInferiorHeap.h"
#include "rtinst/h/rtinst.h" // for time64
#include "pdutil/h/pdDebugOstream.h"

extern pdDebug_ostream sampleVal_cerr;

template <class HK, class RAW>
void fastInferiorHeap<HK, RAW>::set_houseKeeping(unsigned idx, 
						 const HK &iHKValue) 
{
  houseKeeping[idx] = iHKValue; // HK::operator=()
}

template <class HK, class RAW>
void fastInferiorHeap<HK, RAW>::initialize_houseKeeping(unsigned mapsize) 
{
  houseKeeping.resize(0);
  houseKeeping.resize(mapsize);
  for (unsigned lcv=0; lcv < houseKeeping.size(); lcv++) {
    const HK undefinedHK; // default ctor should leave things undefined
    houseKeeping[lcv] = undefinedHK;
  }
}

template <class HK, class RAW>
void fastInferiorHeap<HK, RAW>::initialize_activemap(unsigned mapsize) 
{
  activemap.resize(0);
  activemap.resize(mapsize);
  for (unsigned i=0;i<activemap.size();i++) {
    activemap[i] = varDontSample;
  }
}

template <class HK, class RAW>
void fastInferiorHeap<HK, RAW>::set_activemap(unsigned idx, 
					      sampling_states value) 
{
  if (idx < activemap.size()) {
    activemap[idx] = value;
  }
}

template <class HK, class RAW>
fastInferiorHeap<HK, RAW> &
fastInferiorHeap<HK, RAW>::operator=(const fastInferiorHeap<HK, RAW> &src)
{
  if (&src == this)
      return *this; // the usual check for x=x

  inferiorProcess = src.inferiorProcess;
  baseAddrInApplic = src.baseAddrInApplic;
  baseAddrInParadynd = src.baseAddrInParadynd;
  permanentSamplingSet = src.permanentSamplingSet;
  currentSamplingSet = src.currentSamplingSet;
  return(*this);
}

template <class HK, class RAW>
fastInferiorHeap<HK, RAW>::
fastInferiorHeap(RAW *iBaseAddrInParadynd,
		 process *iInferiorProcess,
		 unsigned mapsize) : 
  inferiorProcess(iInferiorProcess), houseKeeping(mapsize)
{
   baseAddrInApplic   = NULL; // until setBaseAddrInApplic() gets called...
   baseAddrInParadynd = iBaseAddrInParadynd;
   initialize_activemap(mapsize);
   // Note: houseKeeping[] values are uninitialized; each call to alloc() 
   //       initializes one HK (here) and one RAW (in the inferior heap).
}

template <class HK, class RAW>
fastInferiorHeap<HK, RAW>::fastInferiorHeap(fastInferiorHeap<HK, RAW> *parent,
					    process *newProc,
                                            void *paradynd_attachedAt,
                                            void *appl_attachedAt) : 
  inferiorProcess(newProc)
{
   // this copy-ctor is a fork()/dup()-like routine.  Call after process forks.
   assert(parent!=NULL);
   activemap = parent->activemap;
   houseKeeping = parent->houseKeeping;

   baseAddrInApplic   = (RAW*)appl_attachedAt;
   baseAddrInParadynd = (RAW*)paradynd_attachedAt;

   // Now for the houseKeeping, which is quite tricky on a fork.
   // We (intentionally) leave every entry undefined for now.
   // (using a copy-ctor would be a very bad idea, for then we would end up 
   // sharing mi's with the parent process, which is not at all the correct 
   // thing to do.
   // outside code (forkProcess(); context.C) should soon fill in what we left
   // undefined; for example, the call to metricDefinitionNode::handleFork().)
   // More specifically: on a fork, dataReqNode->dup() is called for everything
   // in the parent.  This virtual fn will end up calling the correct fork-ctor
   // for objects like "samedShmIntCounterReqNode", which will in turn think up
   // a new HK value for a single allocated entry, and call 
   // initializeHKAfterFork() to
   // fill it in. If, for whatevery reason, some allocated datareqnode doesn't
   // have its HK filled in by initializeAfterFork(), then the HK is left 
   // undefined (in particular, its mi will be NULL), so we should get an 
   // assert error rather soon...the next shm sample, in fact.   
   // Furthermore: we initially turn each allocated entry in the new heap into
   // type maybeAllocated.  Why do we do this?
   // Because some of the allocated items of the parent process don't belong in
   // the child process after all...specifically, those with foci narrowed down
   // to a specific process --- the parent process.  In such cases, the new 
   // process shouldn't get a copy of mi. On the other hand, what about foci 
   // which aren't specific to any process, and thus should be copied to the 
   // new heap? Well in such cases, intializeHKAfterFork() will be called...so
   // at that time, we'll turn the item back to allocated.  A bit ugly, 
   // n'est-ce pas?.  --ari

   for (unsigned lcv=0; lcv < houseKeeping.size(); lcv++) {
      // If we wanted, we could do this only for allocated items...but doing 
      // it for every item should be safe (if unnecessary).
      const HK undefinedHK; // default ctor should leave things undefined
      houseKeeping[lcv] = undefinedHK;
   }

   // permanent and curr sampling sets init'd to null on purpose, since they
   // can differ from those of the parent. forkHasCompleted() initializes them.
}

template <class HK, class RAW>
void fastInferiorHeap<HK, RAW>::forkHasCompleted(
                                             const vector<states> &statemap)
{
  for (unsigned lcv=0; lcv < statemap.size(); lcv++) {
    if(statemap[lcv] == varAllocated) {
      const HK &theHK = houseKeeping[lcv];
      theHK.assertWellDefined();
    }
  }
}

template <class HK, class RAW>
fastInferiorHeap<HK, RAW>::~fastInferiorHeap() {
   // destructors for statemap[], houseKeeping[], permanentSamplingSet[], and
   // currentSamplingSet[] are called automatically
}

template <class HK, class RAW>
void fastInferiorHeap<HK, RAW>::handleExec() {
  // here's a quick-and-dirty way to reinitialize the houseKeeping vector 
  //(I think):
  houseKeeping.resize(0);
  houseKeeping.resize(activemap.size());
}

template <class HK, class RAW>
bool fastInferiorHeap<HK, RAW>::doMajorSample(const vector<states> &statemap)
{
   // return true iff a complete sample was made
   //cerr << "    FIH::doMajorSample\n";
   // We used to take in a process (virtual) time as the last param, passing
   // it on to HK::perform().  But we've found that the process time (when
   // used as fudge factor when sampling an active process timer) must be
   // taken at the same time the sample is taken to avoid incorrectly scaled
   // fudge factors, leading to jagged spikes in the histogram
   // (i.e. incorrect samples).  The same applies to wall-time, so we ignore
   // both params.

   currentSamplingSet = permanentSamplingSet;
      // not a fast operation; vector::operator=()

#ifdef SHM_SAMPLING_DEBUG
   // Verify that every item in the sampling set is allocated (i.e. should be
   // sampled)
   for (unsigned lcv=0; lcv < currentSamplingSet.size(); lcv++)
      assert(statemap[currentSamplingSet[lcv]] == varAllocated);
#endif

   // Verify that every allocated statemap item is in the current sampling
   // set, and that the current sampling set is ordered. (A very strong set
   // of asserts.)  (Note: The curr sampling set is sorted now (at the start
   // of a major sample), but it likely won't remain so for long...which is
   // OK.)  It's a bit too expensive to do casually, however, so we ifdef it:
#ifdef SHM_SAMPLING_DEBUG
   unsigned cssIndex=0; // current-sampling-set index
   for (unsigned lcv=0; lcv < statemap.size(); lcv++) {
      if (statemap[lcv] == varAllocated)
	 assert(currentSamplingSet[cssIndex++] == lcv);
   }
   assert(cssIndex == currentSamplingSet.size());
#endif

   const bool completeSuccessfulSample = doMinorSample(statemap);
   //cerr << "    FIH::doMajorSample, result = " << completeSuccessfulSample
   //  	  << "\n";
   return completeSuccessfulSample;
}

template <class HK, class RAW>
bool fastInferiorHeap<HK, RAW>::doMinorSample(const vector<states> &statemap) 
{
  // returns true if all variables in this FIH that should be sampled 
  // were sampled;  samples variables as defined in currentSamplingSet
  //cerr << "    FIH::doMinorSample, samplingSetSize: " 
  //     << currentSamplingSet.size() << "\n";
  for(int i=(int)(currentSamplingSet.size()-1); i>=0; i--) {
    const unsigned varIndex = currentSamplingSet[i];
    assert(statemap[varIndex] == varAllocated);
    assert(activemap[varIndex] == varDoSample);
    
    RAW *shmVar = baseAddrInParadynd + varIndex;  // ptr arith
    bool success = houseKeeping[varIndex].perform(*shmVar, inferiorProcess);
    if(success) {
      //cerr << "      var " << varIndex << " sampled\n";
      currentSamplingSet.erase(static_cast<unsigned>(i));
    }
  }
  bool everythingSampledSuccessfully = (currentSamplingSet.size() == 0);
  //cerr << "    FIH::doMinorSample, everythingSampledSuccessfully: "
  //     << everythingSampledSuccessfully << "\n";
  return everythingSampledSuccessfully;
}

template <class HK, class RAW>
void fastInferiorHeap<HK, RAW>::reconstructPermanentSamplingSet(
					    const vector<states> &statemap)
{
   permanentSamplingSet.resize(0);
   assert(activemap.size() == statemap.size());

   for (unsigned lcv=0; lcv < statemap.size(); lcv++)
     if (statemap[lcv] == varAllocated && activemap[lcv] == varDoSample) {
         permanentSamplingSet.push_back(lcv);
     }
}

template <class HK, class RAW>
void fastInferiorHeap<HK, RAW>::makePendingFree(unsigned ndx,
					   const vector<Address> &trampsUsing) 
{
  houseKeeping[ndx].makePendingFree(trampsUsing, inferiorProcess);
  activemap[ndx] = varDontSample;
}

template <class HK, class RAW>
bool fastInferiorHeap<HK, RAW>::tryGarbageCollect(const vector<Frame> &stackWalk,
						  unsigned ndx)
{
  assert(ndx<houseKeeping.size());
  return(houseKeeping[ndx].tryGarbageCollect(stackWalk));    
}

template <class HK, class RAW>
void fastInferiorHeap<HK, RAW>::initializeHKAfterFork(unsigned allocatedIndex,
					    const HK &/*iHouseKeepingValue*/)
{
  // pass in statemap and use that instead of activemap
  // in order to determine whether the housekeeping should be copied
  assert(activemap.size() == houseKeeping.size());
  assert(allocatedIndex<activemap.size());
  //  if(activemap[allocatedIndex] == varActive)  --bhs
  //    houseKeeping[allocatedIndex] = iHouseKeepingValue; // HK::operator=()
}

template <class HK, class RAW>
void fastInferiorHeap<HK, RAW>::addToPermanentSamplingSet(unsigned lcv)
{     
  if(activemap[lcv] == varDoSample)
    permanentSamplingSet.push_back(lcv);
}

template <class HK, class RAW>
void fastInferiorHeap<HK, RAW>::removeFromCurrentSamplingSet(
							    unsigned varIndex)
{  
  for(int i=(int)currentSamplingSet.size()-1; i>=0; i--) {
    if(currentSamplingSet[i] == varIndex) {
      currentSamplingSet.erase(static_cast<unsigned>(i));
      break;
    }
  }
}

