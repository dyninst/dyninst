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

// fastInferiorHeapHKs.C
// Ari Tamches
// contains housekeeping (HK) classes used as the first template input tpe
// to fastInferiorHeap (see fastInferiorHeap.h and .C)

#include "dyninstAPI/src/process.h"
#include "metric.h"
#include "fastInferiorHeapHKs.h"

// Define some static member vrbles:
// In theory, these values are platform-dependent; for now, they're always
// one million since the timer "getTime" routines in rtinst always return usecs.
// But that will surely change one day...e.g. some platforms might start returning
// cycles.
unsigned wallTimerHK::normalize = 1000000;
unsigned processTimerHK::normalize = 1000000;

genericHK &genericHK::operator=(const genericHK &src) {
   if (&src == this)
      return *this; // the usual check for x=x

   mi            = src.mi;
   trampsUsingMe = src.trampsUsingMe;

   return *this;
}

void genericHK::makePendingFree(const vector<Address> &iTrampsUsing) {
   // now we initialize trampsUsingMe.  iTrampsUsing provides us with the starting
   // addr of each such tramp, but we need to look at the old-style inferiorHeap
   // (process.C) to find the tramp length and hence its endAddr.  Yuck.

//   cout << "welcome to genericHK::makePendingFree" << endl;

   trampsUsingMe.resize(iTrampsUsing.size());
      // we may shrink it later if some entries are shown to be unneeded

   unsigned actualNumTramps=0;

   for (unsigned lcv=0; lcv < iTrampsUsing.size(); lcv++) {
      assert(mi);

      const class process &inferiorProc = *(mi->proc());
         // don't ask why 'class' is needed here because I don't know myself.

      const dictionary_hash<unsigned, heapItem*> &heapActivePart =
	inferiorProc.heaps[inferiorProc.splitHeaps ? textHeap : dataHeap].heapActive;
      
      const unsigned trampBaseAddr = iTrampsUsing[lcv];
      heapItem *trampHeapItem;
      // fills in "trampHeapItem" if found:
      if (!heapActivePart.find(trampBaseAddr, trampHeapItem)) {
         // hmmm...the trampoline was deleted, so I guess we don't need to check it
         //        in the future.
	 continue; // next trampoline check
      }

      trampRange tempTrampRange;
      tempTrampRange.startAddr = trampBaseAddr;
      tempTrampRange.endAddr = trampBaseAddr + trampHeapItem->length - 1;

      trampsUsingMe[actualNumTramps++] = tempTrampRange;
   }

   trampsUsingMe.resize(actualNumTramps);
}

bool genericHK::tryGarbageCollect(const vector<unsigned> &PCs) {
   // returns true iff GC succeeded.
   // We may of course assume that this routine is only called if the
   // item in question is in pending-free state.
   // Similar to isFreeOK of process.C...
   // PCs is a list of PC-register values representing a stack trace in the inferior
   // process.  It's OK to garbage collect (and we turn true) if none of the PCs
   // in the stack trace fall within any of the trampolines who are using us.

   for (unsigned pointlcv=0; pointlcv < trampsUsingMe.size(); pointlcv++) {
      const trampRange &theTrampRange = trampsUsingMe[pointlcv];

      // If any of the PCs of the stack trace are within theTrampRange, then it's unsafe
      // to delete us.

      for (unsigned stacklcv=0; stacklcv < PCs.size(); stacklcv++) {
         const unsigned stackPC = PCs[stacklcv];

         // If this PC falls within the range of the trampoline we're currently
         // looking at, then it's unsafe to delete it.
         if (stackPC >= theTrampRange.startAddr &&
             stackPC <= theTrampRange.endAddr)
            return false; // sorry, can't delete
      }
   }

   // GC has succeeded!  Do some cleanup and return true:
   trampsUsingMe.resize(0); // we won't be needing this anymore
   return true;
}

/* ************************************************************************** */

intCounterHK &intCounterHK::operator=(const intCounterHK &src) {
   if (&src == this)
      return *this; // the usual check for x=x

   lowLevelId = src.lowLevelId;
   genericHK::operator=(src);

   return *this;
}

bool intCounterHK::perform(const intCounter &dataValue, process *inferiorProc) {
   // returns true iff the process succeeded; i.e., if we were able to grab a
   // consistent value for the intCounter and process without any waiting.  Otherwise,
   // we return false and don't process and don't wait.

   const int val        = dataValue.value;
      // that was the sampling.  Currently, we don't check to make sure that we
      // sampled a consistent value, since we assume that writes to integers
      // are never partial.  Once we extent intCounters to, say, 64 bits, we'll
      // need to be more careful and use a scheme similar to that used by the
      // process and wall timers...

   // To avoid race condition, don't use 'dataValue' after this point!

#ifdef SHM_SAMPLING_DEBUG
   const unsigned id    = dataValue.id.id;
      // okay to read dataValue.id since there's no race condition with it.
   assert(id == this->lowLevelId); // verify our new code is working right
      // eventually, id field can be removed from dataValue, saving space

   extern dictionary_hash<unsigned, metricDefinitionNode*> midToMiMap;
   metricDefinitionNode *theMi;
   if (!midToMiMap.find(id, theMi)) { // fills in "theMi" if found
      // sample not for valid metric instance; no big deal; just drop sample.
      // (But perhaps in the new scheme this can be made an assert failure?)
      cerr << "intCounter sample not for valid metric instance, so dropping" << endl;
      return true; // is this right?
   }
   assert(theMi == this->mi); // verify our new code is working right
      // eventually, id field can be removed from inferior heap; we'll
      // just use this->mi.

   // note: we do _not_ assert that id==mi->getMId(), since mi->getMId() returns
   // an entirely different identifier that has no relation to our 'id'
#endif

   const float valToReport = (float)val;

   assert(mi);
   assert(mi->proc() == inferiorProc);

//   mi->updateValue(wallTime, valToReport);
   mi->updateValue(getCurrWallTime(), valToReport);

   return true;
}

/* ************************************************************************** */

wallTimerHK &wallTimerHK::operator=(const wallTimerHK &src) {
   if (&src == this)
      return *this; // the usual check for x=x

   lowLevelId        = src.lowLevelId;
   lastTimeValueUsed = src.lastTimeValueUsed;

   genericHK::operator=(src);

   return *this;
}

static time64 calcTimeValueToUse(int count, time64 start,
					     time64 total,
					     time64 currentTime) {
   if (count == 0)
      // inactive timer; the easy case; just report total.
      return total;

   if (count < 0)
      // ??? a strange case, shouldn't happen.  If this occurs, it means
      // that an imbalance has occurred w.r.t. startTimer/stopTimer, and
      // that we don't really know if the timer is active or not.
      return total;

   // Okay, now for the active timer case.  Report the total plus (now - start).
   // A wrinkle: for some reason, we occasionally see currentTime < start, which
   // should never happen.  In that case, we'll just report total  (why is it
   // happening?)
   if (currentTime < start)
      return total;
   else
      return total + (currentTime - start);
}

bool wallTimerHK::perform(const tTimer &theTimer, process *) {
   // returns true iff the process succeeded; i.e., if we were able to read
   // a consistent value of the tTimer, and process it.  Otherwise, returns false,
   // doesn't process and doesn't wait.

   // We sample like this:
   // read protector2, read values, read protector1.  If protector values are equal,
   // then process using a copy of the read values.  Otherwise, return false.
   // This algorithm is from [Lamport 77], and allows concurrent reading and writing.
   // If someday there becomes multiple writers, then we'll have to make changes...

   // We used to take in a wall time to use as the time-of-sample.  But we've found
   // that the wall time (when used as fudge factor when sampling an active process
   // timer) must be taken at the same time the sample is taken to avoid incorrectly
   // scaled fudge factors, leading to jagged spikes in the histogram (i.e.
   // incorrect samples).  This is too bad; it would be nice to read the wall time
   // just once per paradynd sample, instead of once per timer per paradynd sample.

   volatile const int prot2 = theTimer.protector2;

   // Do these 4 need to be volatile as well to ensure that they're read
   // between the reading of protector2 and protector1?  Probably not, since those
   // two are volatile; but let's keep an eye on the generated assembly code...
   const time64 start = theTimer.start;
   const time64 total = theTimer.total;
   const int    count = theTimer.counter;
   const time64 currWallTime = getCurrWallTime();

   volatile const int prot1 = theTimer.protector1;

   if (prot1 != prot2)
      // We read a (possibly) inconsistent value for the timer, so we reject it.
      return false;

   /* don't use 'theTimer' after this point! (avoid race condition).  To ensure
      this, we call calcTimeValueToUse(), which doesn't use 'theTimer' */
   time64 timeValueToUse = calcTimeValueToUse(count, start,
							  total, currWallTime);

   // Check for rollback; update lastTimeValueUsed (the two go hand in hand)
   if (timeValueToUse < lastTimeValueUsed) {
      // Timer rollback!  An assert failure.
      const char bell = 07;
      cerr << "wallTimerHK::perform(): wall timer rollback (" << timeValueToUse << "," << lastTimeValueUsed << ")" << bell << endl;
      cerr << "timer was " << (count > 0 ? "active" : "inactive") << endl;

      assert(false);
   }
   else
      lastTimeValueUsed = timeValueToUse;

#ifdef SHM_SAMPLING_DEBUG
   // It's okay to use theTimer.id because it's not susceptible to race conditions
   const unsigned id    = theTimer.id.id;
   assert(id == this->lowLevelId); // verify our new code is working right

   extern dictionary_hash<unsigned, metricDefinitionNode*> midToMiMap;
   metricDefinitionNode *theMi;
   if (!midToMiMap.find(id, theMi)) { // fills in "theMi" if found
      // sample not for valid metric instance; no big deal; just drop sample.
      cout << "NOTE: dropping sample unknown wallTimer id " << id << endl;
      return true; // is this right?
   }
   assert(theMi == this->mi); // verify our new code is working right

   // note: we do _not_ assert that id==mi->getMId(), since mi->getMId() returns
   // an entirely different identifier that has no relation to our 'id'
#endif

   const double valueToReport = (double)timeValueToUse / normalize;
   
//   mi->updateValue(theWallTime, (float)valueToReport);
   mi->updateValue(currWallTime, (float)valueToReport);

   return true;
}

/* ************************************************************************** */

processTimerHK &processTimerHK::operator=(const processTimerHK &src) {
   if (&src == this)
      return *this; // the usual check for x=x

   lowLevelId        = src.lowLevelId;
   lastTimeValueUsed = src.lastTimeValueUsed;

   genericHK::operator=(src);

   return *this;
}

bool processTimerHK::perform(const tTimer &theTimer, process *inferiorProc) {
   // returns true iff the process succeeded; i.e., if we were able to grab the
   // mutex for this tTimer and process without any waiting.  Otherwise,
   // we return false and don't process and don't wait.

   // Timer sampling is trickier than counter sampling.  There are more
   // race conditions and other factors to carefully consider.

   // see the corresponding wall-timer routine for comments on how we guarantee
   // a consistent read value...

   // We used to take in a wall time to use as the time-of-sample, and a process
   // time to use as the current-process-time for use in fudge factor when sampling
   // an active process timer.  But we've found that both values must be taken at
   // the same time the sample is taken to avoid incorrect values, leading to
   // jagged spikes in the histogram (i.e. incorrect samples).  This is too bad; it
   // would be nice to read these times just once per paradynd sample, instead of
   // once per timer per paradynd sample.

   volatile const int protector2 = theTimer.protector2;

   // Do the following vrbles need to be volatile to ensure that they're read
   // between the reading of protector2 and protector1?  Probably not, but
   // we should keep an eye on the generated assembly to be sure...

   // We always need to use the first 2 vrbles:
   const int    count = theTimer.counter;
   const time64 total = theTimer.total;
   const time64 start = (count > 0) ? theTimer.start : 0; // not needed if count==0

   const unsigned long long inferiorCPUtime =
                  (count > 0) ? inferiorProc->getInferiorProcessCPUtime() : 0;
      // Also cheating; see below.
      // the fudge factor is needed only if count > 0.
   const time64 theWallTime = getCurrWallTime();
      // This is cheating a little; officially, this call should be inside
      // of the two protector vrbles.  But, it was taking too long...

   volatile const int protector1 = theTimer.protector1;

   if (protector1 != protector2)
      return false;

   /* don't use 'theTimer' after this point! (avoid race condition).  To ensure
      this, we call calcTimeValueToUse() without passing 'theTimer' */
   time64 timeValueToUse = calcTimeValueToUse(count, start,
							  total, inferiorCPUtime);

   // Check for rollback; update lastTimeValueUsed (the two go hand in hand)
   if (timeValueToUse < lastTimeValueUsed) {
      // Timer rollback!  An assert failure.
      // (Unfortunately, since it's happening so often, we must "gracefully"
      // handle it instead of bombing)

//      const char bell = 07;
      const char bell = ' ';

      cerr << "processTimerHK::perform(): process timer rollback (" << timeValueToUse << "," << lastTimeValueUsed << ")" << bell << endl;
      cerr << "timer was " << (count > 0 ? "active" : "inactive") << endl;

      timeValueToUse = lastTimeValueUsed;

//      assert(false);
   }
   else
      lastTimeValueUsed = timeValueToUse;

#ifdef SHM_SAMPLING_DEBUG
   const unsigned id    = theTimer.id.id;

   assert(id == this->lowLevelId); // verify our new code is working right

   extern dictionary_hash<unsigned, metricDefinitionNode*> midToMiMap;
   metricDefinitionNode *theMi;
   if (!midToMiMap.find(id, theMi)) { // fills in "theMi" if found
      // sample not for valid metric instance; no big deal; just drop sample.
      cerr << "procTimer id " << id << " not found in midToMiMap so dropping sample of val " << (double)timeValueToUse / normalize << " for mi " << (void*)mi << " proc pid " << inferiorProc->getPid() << endl;
      return true; // is this right?
   }
   assert(theMi == this->mi); // verify our new code is working right

   // note: we do _not_ assert that id==mi->getMId(), since mi->getMId() returns
   // an entirely different identifier that has no relation to our 'id'
#endif

   const double valueToReport = (double)timeValueToUse / normalize;

   mi->updateValue(theWallTime, (float)valueToReport);

   return true;
}
