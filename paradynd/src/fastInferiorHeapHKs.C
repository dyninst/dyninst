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

#include "paradynd/src/process.h"
#include "util/h/spinMutex_cintf.h"
#include "fastInferiorHeapHKs.h"

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

bool intCounterHK::perform(intCounter &dataValue, unsigned long long wallTime,
			   unsigned long long) {
   // (we would like to make dataValue a const ref but can't since we acquire
   //  a lock.  new c++ keyword mutable should come in handy here.)
   // returns true iff the process succeeded; i.e., if we were able to grab the
   // mutex for this intCounter and process without any waiting.  Otherwise,
   // we return false and don't process and don't wait.

   if (!spinMutex_tryToGrab(&dataValue.theSpinner))
      return false;

   const int val        = dataValue.value;
   const unsigned id    = dataValue.id.id;

   assert(id == this->lowLevelId); // verify our new code is working right
      // eventually, id field can be removed from dataValue, saving space

   extern dictionary_hash<unsigned, metricDefinitionNode*> midToMiMap;
   metricDefinitionNode *theMi;
   if (!midToMiMap.find(id, theMi)) { // fills in "theMi" if found
      // sample not for valid metric instance; no big deal; just drop sample.
      // (But perhaps in the new scheme this can be made an assert failure?)
      spinMutex_release(&dataValue.theSpinner);
      return true; // is this right?
   }
   assert(theMi == this->mi); // verify our new code is working right
      // eventually, id field can be removed from inferior heap; we'll
      // just use this->mi.

   const float valToReport = (float)val;
   mi->updateValue(wallTime, valToReport);

   spinMutex_release(&dataValue.theSpinner);
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

bool wallTimerHK::perform(tTimer &theTimer, unsigned long long theWallTime,
			  unsigned long long) {
   // (const tTimer & would be nice but it'll have to wait for the mutable keyword
   // to be implemented since we write in order to obtain the lock)
   // returns true iff the process succeeded; i.e., if we were able to grab the
   // mutex for this tTimer and process without any waiting.  Otherwise,
   // we return false and don't process and don't wait.

   // Timer sampling is trickier than counter sampling.  There are more
   // race conditions and other factors to carefully consider.

   if (!spinMutex_tryToGrab(&theTimer.theSpinner))
      return false;

   assert(theTimer.type == wallTime);
   unsigned long long timeValueToUse;

   if (theTimer.counter == 0) {
      // the timer is not currently running (active), so simply report "total"
      timeValueToUse = theTimer.total;
   }
   else if (theTimer.counter > 1) {
      // the timer is (very) active.  Add (now-start) to "total" for the reported value.

      //assert(theWallTime >= theTimer.start);
      if (theWallTime < theTimer.start)
	 // yes, on occasion, this happens (theWallTime being a smidge less than
	 // than the timer's start time).  I'm not sure why this happens, and it
	 // probably shouldn't.  It may be related to theTimer.start being set by
	 // the application and 'theWallTime' being read by paradynd.  In any event,
	 // it seems we need to make this check. --ari
	 timeValueToUse = theTimer.total;
      else
	 // the normal case
	 timeValueToUse = theTimer.total + (theWallTime - theTimer.start);
   }
   else if (theTimer.mutex == 1) {
      // counter==1 and mutex==1 --> snapshot has been updated but total hasn't been yet
      // So, report snapshot instead of "total"; the latter may be corrupted (half-
      // updated).
      timeValueToUse = theTimer.snapShot;
   }
   else {
      // counter==1 and mutex==0 --> total hasn't been updated yet; snapShot may
      // be corrupted (half-updated).  So, use "total" with adjustment (add now-start),
      // like the case where counter > 1

      //assert(theWallTime >= theTimer.start);
      if (theWallTime < theTimer.start)
	 // yes, on occasion, this happens (theWallTime being a smidge less than
	 // than the timer's start time).  I'm not sure why this happens, and it
	 // probably shouldn't.  It may be related to theTimer.start being set by
	 // the application and 'theWallTime' being read by paradynd.  In any event,
	 // it seems we need to make this check. --ari
	 timeValueToUse = theTimer.total;
      else
	 // the normal case
	 timeValueToUse = theTimer.total + (theWallTime - theTimer.start);
   }

   // Check for rollback; update lastTimeValueUsed (the two go hand in hand)
   if (timeValueToUse < lastTimeValueUsed) {
      // Timer rollback!  An assert failure.
      const char bell = 07;
      cerr << "wallTimerHK::perform(): wall timer rollback (" << timeValueToUse << "," << lastTimeValueUsed << ")" << bell << endl;
      cerr << "timer was " << (theTimer.counter > 0 ? "active" : "inactive") << endl;

      assert(false);
   }
   else
      lastTimeValueUsed = timeValueToUse;

   const unsigned id    = theTimer.id.id;
   assert(id == this->lowLevelId); // verify our new code is working right

   extern dictionary_hash<unsigned, metricDefinitionNode*> midToMiMap;
   metricDefinitionNode *theMi;
   if (!midToMiMap.find(id, theMi)) { // fills in "theMi" if found
      // sample not for valid metric instance; no big deal; just drop sample.
      cout << "NOTE: dropping sample unknown wallTimer id " << id << endl;
      spinMutex_release(&theTimer.theSpinner);
      return true; // is this right?
   }
   assert(theMi == this->mi); // verify our new code is working right

   assert(theTimer.normalize == 1000000);
      // ...and since it's always the same, the field itself can be removed, right?

   const double valueToReport = (double)timeValueToUse / theTimer.normalize;
   
   mi->updateValue(theWallTime, (float)valueToReport);

   spinMutex_release(&theTimer.theSpinner);
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

bool processTimerHK::perform(tTimer &theTimer, unsigned long long theWallTime,
			     unsigned long long inferiorProcCPUtime) {
   // (const tTimer & would be nice but it'll have to wait for the mutable keyword
   // to be implemented since we write in order to obtain the lock)
   // returns true iff the process succeeded; i.e., if we were able to grab the
   // mutex for this tTimer and process without any waiting.  Otherwise,
   // we return false and don't process and don't wait.

   // Timer sampling is trickier than counter sampling.  There are more
   // race conditions and other factors to carefully consider.

   if (!spinMutex_tryToGrab(&theTimer.theSpinner))
      return false;

   assert(theTimer.type == processTime);
   unsigned long long timeValueToUse;

   // Locks are needed because the following (i.e. the whole if-then-else stmt)
   // needs to be done atomically w.r.t. the application.
   // (Alternatively, if we could somehow lock down the bus and atomically
   // read a copy of the entire timer structure, then we could work off of
   // the copy with no locks.  But such an atomic copy operation is probably
   // impossible.)

   const int theTimerCounter = theTimer.counter;
   const int theTimerMutex = theTimer.mutex;
   const time64 theTimerTotal = theTimer.total;
   const time64 theTimerSnapShot = theTimer.snapShot;

   if (theTimerCounter == 0) {
      // the timer is not currently running (active), so simply report "total"
      timeValueToUse = theTimerTotal;
   }
   else if (theTimerCounter > 1) {
      // the timer is (very) active.  Add (now-start) to "total" for the reported value.
      timeValueToUse = theTimerTotal + (inferiorProcCPUtime - theTimer.start);
   }
   else if (theTimerMutex == 1) {
      // counter==1 and mutex==1 --> snapshot has been updated but total hasn't been yet
      // So, report snapshot instead of "total"; the latter may be corrupted (half-
      // updated).
      timeValueToUse = theTimerSnapShot;
   }
   else {
      // counter==1 and mutex==0 --> total hasn't been updated yet; snapShot may
      // be corrupted (half-updated).  So, use "total" with adjustment (add now-start),
      // like the case where counter > 1.

      // WARNING of race condition: when we (paradynd) read counter and mutex,
      // the above conditions may have been true...but by now, with the process
      // running in parallel, it's probably no longer true!  The result: a reported
      // value that's too large, causing rollback in the near future (probably the
      // next call to this routine).

      // So what do we do in this case, since locks aren't yet implemented?
      // If we knew that we were in the middle of a DYNINSTstopProcessTimer(),
      // then we could just return false.

      // But we're not always w/in DYNINSTstopProcessTimer.  For example,
      // activate cpu/whole program, and this condition will be true always,
      // with no DYNINSTstopProcessTimer in sight.

      // First, some simple options.
      // 1) If snapShot and total differ then we must be within
      //    DYNINSTstopProcessTimer, and thus we can return false.
      // 2) If mutex is 1 then we must be within DYNINSTstopProcessTimer,
      //    and thus we can return false.
      if (theTimerSnapShot != theTimerTotal)
	 return false;
      if (theTimerMutex)
	 return false; // using snapShot would probably work here
      if (theTimer.snapShot != theTimer.total) // read fresh values
	 return false;
      if (theTimer.mutex)
	 return false;

      // Otherwise, we'll return total + (now-start), which is the "correct" thing to do.
      timeValueToUse = theTimerTotal + (inferiorProcCPUtime - theTimer.start);
   }

   // Check for rollback; update lastTimeValueUsed (the two go hand in hand)
   if (timeValueToUse < lastTimeValueUsed) {
      // Timer rollback!  An assert failure.
      // (Unfortunately, since it's happening so often, we must "gracefully"
      // handle it instead of bombing)
      timeValueToUse = lastTimeValueUsed;

//      const char bell = 07;
//      cerr << "processTimerHK::perform(): process timer rollback (" << timeValueToUse << "," << lastTimeValueUsed << ")" << bell << endl;
//      cerr << "timer was " << (theTimerCounter > 0 ? "active" : "inactive") << endl;
//
//      assert(false);
   }
   else
      lastTimeValueUsed = timeValueToUse;

   const unsigned id    = theTimer.id.id;

   assert(id == this->lowLevelId); // verify our new code is working right

   extern dictionary_hash<unsigned, metricDefinitionNode*> midToMiMap;
   metricDefinitionNode *theMi;
   if (!midToMiMap.find(id, theMi)) { // fills in "theMi" if found
      // sample not for valid metric instance; no big deal; just drop sample.
      spinMutex_release(&theTimer.theSpinner);
      return true; // is this right?
   }
   assert(theMi == this->mi); // verify our new code is working right

   assert(theTimer.normalize == 1000000);
      // ...and since it's always the same, the field itself can be removed, right?

   const double valueToReport = (double)timeValueToUse / theTimer.normalize;

   mi->updateValue(theWallTime, (float)valueToReport);

   spinMutex_release(&theTimer.theSpinner);
   return true;
}
