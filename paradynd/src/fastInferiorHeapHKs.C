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

// $Id: fastInferiorHeapHKs.C,v 1.21 2001/11/03 06:08:53 schendel Exp $
// contains housekeeping (HK) classes used as the first template input tpe
// to fastInferiorHeap (see fastInferiorHeap.h and .C)

#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/pdThread.h"
#include "metric.h"
#include "fastInferiorHeapHKs.h"
#include "paradynd/src/init.h"
#include "pdutil/h/pdDebugOstream.h"
#include "common/h/int64iostream.h"

extern pdDebug_ostream sampleVal_cerr;

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

      const dictionary_hash<Address, heapItem*> &heapActivePart =
	inferiorProc.heap.heapActive;

      const Address trampBaseAddr = iTrampsUsing[lcv];
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

bool genericHK::tryGarbageCollect(const vector<Address> &PCs) {
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
         const Address stackPC = PCs[stacklcv];

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

   int64_t val;
#if defined(i386_unknown_solaris2_5) || defined(i386_unknown_linux2_0)
   // the 32 bit platforms require a special method load the value atomically
   __asm__ volatile ("fildq %0"  : : "g" (dataValue.value));
   __asm__ volatile ("fistpq %0" : "=m" (val)  );
#elif defined(i386_unknown_nt4_0)
   const rawTime64 *addr = &dataValue.value;
   __asm {
     mov ecx, dword ptr [addr]
     fild qword ptr [ecx]
     fistp val
   }
#else
   // The 64 bit platforms have 64 bit load instructions.
   // Perhaps this will need to be changed to not switch the byte values
   // when the associated platform code generators get updated to handle
   // 64 bit counters.
   union value_bits {
     int64_t i64;
     unsigned u32[2];  // we're just using for the shifting of bits
   };
   union value_bits curSample, switchedSample;
   curSample.i64 = dataValue.value;
   switchedSample.u32[0] = curSample.u32[1];
   switchedSample.u32[1] = curSample.u32[0];
   val = switchedSample.i64;
#endif

      // that was the sampling.  Currently, we don't check to make sure that we
      // sampled a consistent value, since we assume that writes to integers
      // are never partial.  Once we extent intCounters to, say, 64 bits, we'll
      // need to be more careful and use a scheme similar to that used by the
      // process and wall timers...

   // To avoid race condition, don't use 'dataValue' after this point!

#ifdef SHM_SAMPLING_DEBUG
   const unsigned drnId    = dataValue.id.id;
      // okay to read dataValue.id since there's no race condition with it.
   assert(drnId == this->lowLevelId); // verify our new code is working right
      // eventually, drnId field can be removed from dataValue, saving space

   extern dictionary_hash<unsigned, metricDefinitionNode*> drnIdToMdnMap;
   metricDefinitionNode *theMi;
   if (!drnIdToMdnMap.find(drnId, theMi)) { // fills in "theMi" if found
      // sample not for valid metric instance; no big deal; just drop sample.
      // (But perhaps in the new scheme this can be made an assert failure?)
      cerr << "intCounter sample not for valid metric instance, so dropping" << endl;
      return true; // is this right?
   }
   assert(theMi == this->mi); // verify our new code is working right
      // eventually, id field can be removed from inferior heap; we'll
      // just use this->mi.
#endif
   assert(mi);
   assert(mi->proc() == inferiorProc);

   if(! mi->isReadyForUpdates()) {
     sampleVal_cerr << "mdn " << mi << " isn't ready for updates yet.\n";
     return false;
   }

   timeStamp currWallTime = getWallTime();

   mi->updateValue(currWallTime, pdSample(val));
      // the integer version of updateValue() (no int-->float conversion -- good)

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

static rawTime64 calcTimeValueToUse(int count, rawTime64 start,rawTime64 total,
				    rawTime64 currentTime, int timerId) {
  sampleVal_cerr << "calcTimeValueToUse- tid: " << timerId << ", count:" 
		 << count << ", start: " << start << ", total: " << total 
		 << ", currentTime: " << currentTime;
   rawTime64 retVal = 0;
   if (count == 0) {
      // inactive timer; the easy case; just report total.
      retVal = total;
   }
   else if (count < 0) {
      // ??? a strange case, shouldn't happen.  If this occurs, it means
      // that an imbalance has occurred w.r.t. startTimer/stopTimer, and
      // that we don't really know if the timer is active or not.
      retVal = total;
   }
   else if (currentTime < start) { 
     // Okay, now for the active timer case.  Report the total plus (now -
     // start).  A wrinkle: for some reason, we occasionally see currentTime
     // < start, which should never happen.  In that case, we'll just report
     // total (why is it happening?)
      retVal = total;
   }
   else {
      retVal = total + (currentTime - start);
   }
   sampleVal_cerr << ", return:" << retVal << "\n";
   return retVal;
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
   const rawTime64 start = theTimer.start;
   const rawTime64 total = theTimer.total;
   const int    count = theTimer.counter;
   const rawTime64 rawCurrWallTime = 
                  getWallTimeMgr().getRawTime(wallTimeMgr_t::LEVEL_BEST);
   timeStamp currWallTime(getWallTimeMgr().units2timeStamp(rawCurrWallTime));

   volatile const int prot1 = theTimer.protector1;

   if (prot1 != prot2)
      // We read a (possibly) inconsistent value for the timer, so we reject it.
      return false;

   /* don't use 'theTimer' after this point! (avoid race condition).  To ensure
      this, we call calcTimeValueToUse(), which doesn't use 'theTimer' */
   rawTime64 rawTimeValueToUse = calcTimeValueToUse(count, start, total, 
                                            rawCurrWallTime, theTimer.id.id);
   // this is where conversion from native units to real time units is done
   timeLength timeValueToUse = 
                getWallTimeMgr().units2timeLength(rawTimeValueToUse);
   sampleVal_cerr << "timeValueToUse: " << timeValueToUse << "\n";
   // Check for rollback; update lastTimeValueUsed (the two go hand in hand)
   if (timeValueToUse < lastTimeValueUsed) {
      // Timer rollback!  An assert failure.
      const char bell = 07;
      cerr << "wallTimerHK::perform(): wall timer rollback ("
          << timeValueToUse << "," << lastTimeValueUsed << ")"
          << bell << endl;
      cerr << "timer was " << (count > 0 ? "active" : "inactive") << endl;

      assert(false);
   }
   else
      lastTimeValueUsed = timeValueToUse;

#ifdef SHM_SAMPLING_DEBUG
   // It's okay to use theTimer.id because it's not susceptible to race conditions
   const unsigned drnId    = theTimer.id.id;
   assert(drnId == this->lowLevelId); // verify our new code is working right

   extern dictionary_hash<unsigned, metricDefinitionNode*> drnIdToMdnMap;
   metricDefinitionNode *theMi;
   if (!drnIdToMdnMap.find(drnId, theMi)) { // fills in "theMi" if found
      // sample not for valid metric instance; no big deal; just drop sample.
      cout << "NOTE: dropping sample unknown wallTimer id " << drnId << endl;
      return true; // is this right?
   }
   assert(theMi == this->mi); // verify our new code is working right
#endif

   if(! mi->isReadyForUpdates()) {
     sampleVal_cerr << "mdn " << mi << " isn't ready for updates yet.\n";
     return false;
   }

   mi->updateValue(currWallTime, pdSample(timeValueToUse));

   return true;
}

/* ************************************************************************** */

processTimerHK &processTimerHK::operator=(const processTimerHK &src) {
   if (&src == this)
      return *this; // the usual check for x=x

   lowLevelId        = src.lowLevelId;
   lastTimeValueUsed = src.lastTimeValueUsed;

#if defined(MT_THREAD)
   vTimer = NULL ;
#endif
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
   const rawTime64 total = theTimer.total;
   const rawTime64 start = (count > 0) ? theTimer.start : 0; // not needed if count==0

#if defined(MT_THREAD)
   const tTimer* vt   = (tTimer*) theTimer.vtimer ;
   rawTime64 inferiorCPUtime ;
   if (vt == (tTimer*) -1) {
     inferiorCPUtime = (count>0) ? inferiorProc->getRawCpuTime() : 0;
   } else {
     if (vt) {
       RTINSTsharedData *RTsharedDataInParadynd 
	   =((RTINSTsharedData*) inferiorProc->getRTsharedDataInParadyndSpace()) ; 
       RTINSTsharedData *RTsharedDataInApplic 
	   =((RTINSTsharedData*) inferiorProc->getRTsharedDataInApplicSpace()) ; 
       vTimer = RTsharedDataInParadynd->virtualTimers + 
	   (int)(vt- RTsharedDataInApplic->virtualTimers);
     }
     bool success = true ; // count <=0 should return true
     inferiorCPUtime =(count>0)?pdThread::getInferiorVtime(vTimer,inferiorProc, success):0 ; 
     if (!success)
       return false ;
   }
#else   
   const rawTime64 inferiorCPUtime = (count>0) ? 
                            inferiorProc->getRawCpuTime(/*theTimer.lwp_id*/) : 0;
#endif 

   // This protector read and comparison must happen *after* we obtain the inferior
   // CPU time or thread virtual time, or we have a race condition resulting in lots
   // of annoying timer rollback warnings.  In the long run, our data is still 
   // correct, but individual samples will be bad.
   volatile const int protector1 = theTimer.protector1;
   if (protector1 != protector2)
      return false;

   // Also cheating; see below.
   // the fudge factor is needed only if count > 0.
   const timeStamp currWallTime = getWallTime();
   // This is cheating a little; officially, this call should be inside
   // of the two protector vrbles.  But, it was taking too long...


   /* don't use 'theTimer' after this point! (avoid race condition).  To ensure
      this, we call calcTimeValueToUse() without passing 'theTimer' */
   rawTime64 rawTimeValueToUse = calcTimeValueToUse(count, start, total, 
					     inferiorCPUtime, theTimer.id.id);
   // this is where conversion from native units to real time units is done
   timeLength timeValueToUse=inferiorProc->units2timeLength(rawTimeValueToUse);
   sampleVal_cerr << "raw-total: " << total << ", timeValueToUse: " << timeValueToUse << "\n";
   
   // Check for rollback; update lastTimeValueUsed (the two go hand in hand)
   // cerr <<"lastTimeValueUsed="<< lastTimeValueUsed << endl ;
   if (timeValueToUse < lastTimeValueUsed) {
#if  !defined(MT_THREAD)
   // Timer could roll back in the case that the per-thread
   // virtual timer is reused by another thread, to avoid this to happen, we should
   // reuse them with a least recently used strategy.
   // See DYNINST_hash_delete in RTinst.c
      const char bell = ' '; //07;
      cerr << "processTimerHK::perform(): process timer rollback (" 
	   << timeValueToUse << "," << lastTimeValueUsed << ")" 
	   << bell << endl;
      cerr << "timer was " << (count > 0 ? "active" : "inactive") 
	   << ", id=" << theTimer.id.id 
	   << ", start=" << start
	   << ", total=" << total
	   << ", inferiorCPUtime=" << inferiorCPUtime
	   << endl;

#endif
      timeValueToUse = lastTimeValueUsed;

//      assert(false);
   }
   else
      lastTimeValueUsed = timeValueToUse;

#ifdef SHM_SAMPLING_DEBUG
   const unsigned drnId    = theTimer.id.id;
   assert(drnId == this->lowLevelId); // verify our new code is working right

   extern dictionary_hash<unsigned, metricDefinitionNode*> drnIdToMdnMap;
   metricDefinitionNode *theMi;
   if (!drnIdToMdnMap.find(drnId, theMi)) { // fills in "theMi" if found
      // sample not for valid metric instance; no big deal; just drop sample.
      cerr << "procTimer id " << drnId 
	   << " not found in drnIdToMdnMap so dropping sample of val " 
	   << timeValueToUse << " for mi " 
	   << (void*)mi << " proc pid " << inferiorProc->getPid() << endl;
      assert(0);
      return true; // is this right?
   }
   assert(theMi == this->mi); // verify our new code is working right
#endif

   if(! mi->isReadyForUpdates()) {
     sampleVal_cerr << "mdn " << mi << " isn't ready for updates yet.\n";
     return false;
   }

   mi->updateValue(currWallTime, pdSample(timeValueToUse));

   return true;
}


/* We are expressing the ratio ns/cycle as a numerator and denominator
   because this won't have the problem with double imprecision 
   (happens past integer digit 15, ie. when resultant value is greater
   than 2^49).  Also, this might be faster for some architectures.

   Nanosecond overflow of the long long data type isn't a problem because
   this would take 292 years of ticks until it occured, see below:

      Time until nanosecond overflow of long long data type (2^63):
      ratio to convert from cycles to ns is (numer/denom).
      numer = 1000/gcd(1000,Mhz)
      denom = Mhz /gcd(1000,Mhz)   Mhz is really floor(Mhz/10^6)
      happens at tick:
      tick*(numer/denom) >= 2^63,
      tickoverflow >= 2^63 * (denom/numer)

      happens at time (in ns):
      tickoverflow*(numer/denom)
      2^63 * (denom/numer) * (numer/denom)
      2^63 ns => 292 years

      if the earliest a cpu could record ticks is 1904, then the earliest
      the tick register could cause a nanosecond rollover is 2196...
      even still, I think most tick registers are reset at power off.

   However, we could possibly get overflow at an interim step in our
   calculations when we multiply the numerator times the current tick
   value.  For instance, if the numerator is 1000, and the tick value
   is 2^54 (.57 years), then the result is 1.9 times greater than
   2^63, or an overflow.  So we'll do our divide by our denominator
   first, and then we'll multiply by our numerator.  Yes, we'll loose
   some precision, because we'll cut off the fractional part on the
   divide by the denominator, but the only place where we're
   converting ticks this big is for time stamps, which have high
   granularity and thus a little imprecision at the nanosecond level
   is irrelevant.  We are using these functions to convert the time
   samples to nanoseconds, however, samples are time differences, that
   is the amount of time spent in a location, and thus won't get this
   big for a long time.

   Calculation of time until interim overflow condition is hit and 
   imprecision can result:

      Time until interim overflow condition occurs:
      happens at tick:
      tick*numer = 2^63
      tick = 2^63 / numer
      
      happens at time (in ns):
      tick * (numer/denom)
      (2^63 / numer) * (numer/denom)
      2^63 / denom
      2^63 / (Mhz/gcd(1000,Mhz))

      for instance:
      499 Mhz machine: time = 2^63 / 499 => 30.6 weeks
      999 Mhz machine: time = 2^63 / 999 => 15.3 weeks
      1999 Mhz machine: time = 2^63 /1999=> 7.6 weeks

   So, if someone runs paradyn on a function for an extremely long
   period (ie. 15 weeks), then inaccuracies in the samples could
   develop.  Also, as computers get faster, this could become more of
   a problem.  So we switch methods of doing our conversion when we're
   in the overflow stage.

   In order to do our ns = ticks * numer / denom calculation when
   overflow can occur partway through the calculation, we will change
   the operation to:
     ns = ticks / denom * numer
     Let  IntDiv = floor(ticks/denom)
        = [ IntDiv + ticks%IntDiv/denom ] * numer
        = [ IntDiv * numer  +  (ticks%IntDiv)*numer/denom ]
   This will preserve exactness in our multiply.  It results in
   2 int divs, 2 integer mults, 1 integer modular, 1 integer addition
   Where previously we just had 1 integer mult and 1 integer div.

   Aggregation doesn't affect this consideration of converting the
   primitive time to nanosecond time by use of this fraction value,
   because aggregation is done after the raw time unit has been
   converted to nanoseconds.  Therefore, our time limits until
   precision occurs isn't affected by aggregation.  
*/
