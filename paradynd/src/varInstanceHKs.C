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

// $Id: varInstanceHKs.C,v 1.23 2005/03/07 21:19:07 bernat Exp $
// contains housekeeping (HK) classes used as the first template input tpe
// to fastInferiorHeap (see fastInferiorHeap.h and .C)

#include "paradynd/src/pd_process.h"
#include "paradynd/src/pd_thread.h"
#include "paradynd/src/threadMetFocusNode.h"
#include "paradynd/src/varInstanceHKs.h"
#include "paradynd/src/init.h"
#include "pdutil/h/pdDebugOstream.h"
#include "common/h/int64iostream.h"
#include "paradynd/src/debug.h"

genericHK &genericHK::operator=(const genericHK &src) {
   if (&src == this)
      return *this; // the usual check for x=x

   thrNodeVal            = src.thrNodeVal;

   return *this;
}


/* ************************************************************************* */

const intCounter intCounterHK::initValue = { 0 };

intCounterHK &intCounterHK::operator=(const intCounterHK &src) {
   if (&src == this)
      return *this; // the usual check for x=x

   genericHK::operator=(src);

   return *this;
}

#if defined(rs6000_ibm_aix4_1)
/* sync on powerPC is actually more general than just a memory barrier,
   more like a total execution barrier, but the general use we are concerned
   of here is as a memory barrier 
*/
#define MEMORY_BARRIER     asm volatile ("sync")
#elif defined( arch_ia64 )
#define MEMORY_BARRIER     asm volatile ( "mf" )
#else
#define MEMORY_BARRIER
#endif

bool intCounterHK::perform(const intCounter *dataValue, 
			   pd_process *inferiorProc)
{
   // returns true iff the process succeeded; i.e., if we were able to grab a
   // consistent value for the intCounter and process without any waiting.
   // Otherwise, we return false and don't process and don't wait.

   // If the thread node hasn't yet been assigned to this dataReqNode, skip
   threadMetFocusNode_Val *thrNval = getThrNodeVal();
   if(thrNval == NULL) {
     return true;
   }

   int64_t val;

#if defined(i386_unknown_solaris2_5) || defined(i386_unknown_linux2_0)
   // the 32 bit platforms require a special method load the value atomically
   __asm__ volatile ("fildq %0"  : : "g" (dataValue->value));
   __asm__ volatile ("fistpq %0" : "=m" (val)  );
#elif defined(i386_unknown_nt4_0)
   const rawTime64 *addr = &dataValue->value;
   __asm {
     mov ecx, dword ptr [addr]
     fild qword ptr [ecx]
     fistp val
   }
#elif defined( arch_ia64 )
	/* I haven't the faintest idea why the byte values were being switched around,
	   but it sure seems to break stuff. */
   val = dataValue->value;
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
   curSample.i64 = dataValue->value;
   switchedSample.u32[0] = curSample.u32[1];
   switchedSample.u32[1] = curSample.u32[0];
   val = switchedSample.i64;   
#endif

   // To avoid race condition, don't use 'dataValue' after this point!

   assert(thrNval->proc() == inferiorProc);
   if(! thrNval->isReadyForUpdates()) {
     sample_cerr << "mdn " << thrNval << " isn't ready for updates yet.\n";
     return false;
   }

   timeStamp currWallTime = getWallTime();

   thrNval->updateValue(currWallTime, pdSample(val));
      // the integer version of updateValue() (no int-->float conversion -- good)

   return true;
}

/* ************************************************************************* */

const tTimer wallTimerHK::initValue = { 0, 0, 0,
					0, /* pos */
					0, 0 };

wallTimerHK &wallTimerHK::operator=(const wallTimerHK &src) {
   if (&src == this)
      return *this; // the usual check for x=x

   lastTimeValueUsed = src.lastTimeValueUsed;

   genericHK::operator=(src);

   return *this;
}

void wallTimerHK::initializeAfterFork(rawType *curElem, rawTime64 curRawTime)
{
  // if it's an active timer, than reset the start field
  curElem->start = curRawTime;
  curElem->total = 0;
}

static rawTime64 calcTimeValueToUse(int count, rawTime64 start,
				    rawTime64 total, rawTime64 currentTime) {
  sample_cerr << "calcTimeValueToUse-  count:" << count << ", start: " 
		 << start << ", total: " << total 
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
     cerr << "Count < 0 in time sample" << endl;
      retVal = total;
   }
   else if (currentTime < start) { 
     // Okay, now for the active timer case.  Report the total plus (now -
     // start).  A wrinkle: for some reason, we occasionally see currentTime
     // < start, which should never happen.  In that case, we'll just report
     // total (why is it happening?)
     cerr << "Current time less than start time in time sample" << endl;
     fprintf(stderr, "currentTime: %lld, start: %lld, total: %lld\n",
	     currentTime, start, total);
     retVal = total;
   }
   else {
      retVal = total + (currentTime - start);
   }
   sample_cerr << ", return:" << retVal << "\n";
   return retVal;
}

bool wallTimerHK::perform(const tTimer *theTimer, pd_process *) {
   // returns true iff the process succeeded; i.e., if we were able to read a
   // consistent value of the tTimer, and process it.  Otherwise, returns
   // false, doesn't process and doesn't wait.

   // If the thread obj hasn't yet been assigned to this dataReqNode, skip
   threadMetFocusNode_Val *thrNval = getThrNodeVal();
   if(thrNval == NULL)
     return true;

   // We sample like this: read protector2, read values, read protector1.  If
   // protector values are equal, then process using a copy of the read
   // values.  Otherwise, return false.  This algorithm is from [Lamport 77],
   // and allows concurrent reading and writing.  If someday there becomes
   // multiple writers, then we'll have to make changes...

   // We used to take in a wall time to use as the time-of-sample.  But we've
   // found that the wall time (when used as fudge factor when sampling an
   // active process timer) must be taken at the same time the sample is
   // taken to avoid incorrectly scaled fudge factors, leading to jagged
   // spikes in the histogram (i.e.  incorrect samples).  This is too bad; it
   // would be nice to read the wall time just once per paradynd sample,
   // instead of once per timer per paradynd sample.

   volatile const int prot2 = theTimer->protector2;
   MEMORY_BARRIER;  // restricts processor from doing reads out-of-order
   // Do these 4 need to be volatile as well to ensure that they're read
   // between the reading of protector2 and protector1?  Probably not, since
   // those two are volatile; but let's keep an eye on the generated assembly
   // code...
   volatile const rawTime64 start = theTimer->start;
   volatile const rawTime64 total = theTimer->total;
   volatile const int    count = theTimer->counter;
   const rawTime64 rawCurrWallTime = 
                  getWallTimeMgr().getRawTime(wallTimeMgr_t::LEVEL_BEST);
   MEMORY_BARRIER;
   volatile const int prot1 = theTimer->protector1;

   // Wall time querying needs to occur in the critical section, otherwise...
   // DAEMON                       RTINST
   // read timer 
   // CONTEXT SWITCH               stopTimer (curR = query-wall-time;
   //                                         totalR += curR - start)
   // curD = query-wall-time       
   // totalD = curD - start
   // 
   // SAMPLE-TIMER (count==0 so totalD'=totalR)
   // --------------------------------------------------------------------
   // From above, totalD > totalR, or totalD > totalD' (a rollback)

   timeStamp currWallTime(getWallTimeMgr().units2timeStamp(rawCurrWallTime));

   if (prot1 != prot2)
      // We read a (possibly) inconsistent value for the timer, so we reject it.
      return false;

   /* don't use 'theTimer' after this point! (avoid race condition).  To ensure
      this, we call calcTimeValueToUse(), which doesn't use 'theTimer' */
   rawTime64 rawTimeValueToUse = calcTimeValueToUse(count, start, total, 
						    rawCurrWallTime);
   // this is where conversion from native units to real time units is done
   timeLength timeValueToUse = 
                getWallTimeMgr().units2timeLength(rawTimeValueToUse);
   sample_cerr << "timeValueToUse: " << timeValueToUse
		  << ", lastTime: " << lastTimeValueUsed << "\n";

   // Check for rollback; update lastTimeValueUsed (the two go hand in hand)
   if (timeValueToUse < lastTimeValueUsed) {
      // Timer rollback!  An assert failure.
      const char bell = 07;
      cerr << "wallTimerHK::perform(): wall timer rollback ("
	   << timeValueToUse << "," << lastTimeValueUsed << ")"
	   << bell << endl;
      cerr << "timer was " << (count > 0 ? "active" : "inactive") << endl;
      timeValueToUse = lastTimeValueUsed;

      //assert(false);
   }
   else
      lastTimeValueUsed = timeValueToUse;

   if(! thrNval->isReadyForUpdates()) {
     sample_cerr << "mdn " << thrNval << " isn't ready for updates yet.\n";
     return false;
   }

   thrNval->updateValue(currWallTime, pdSample(timeValueToUse));

   return true;
}

/* ************************************************************************** */

const tTimer processTimerHK::initValue = { 0, 0, 0,
					   0, /* pos */
					   0, 0 };

processTimerHK &processTimerHK::operator=(const processTimerHK &src) {
   if (&src == this)
      return *this; // the usual check for x=x

   lastTimeValueUsed = src.lastTimeValueUsed;

   vTimer = NULL ;
   genericHK::operator=(src);

   return *this;
}


//  When the fork occurs, we copy the counter and timer instrumentation
//  variables that are in the parent process to the child process.  However,
//  cpu time queried in the child process begins at zero again, so for cpu
//  timers, total fields needs to be reset to zero and start fields need to be
//  reset to the child cpu time at the time of the fork.  Otherwise, errors
//  about time rollbacks will occur.  Because wall time queried in the child
//  process continues to increase (from the last queried time in the parent
//  process), I don't believe these fields are required to be reset, but I'm
//  doing it anyways for simplicity.

void processTimerHK::initializeAfterFork(rawType *curElem, 
					 rawTime64 curRawTime) {
  // if it's an active timer, than reset the start field
  // the process is stopped at this time, so we don't need to do an
  // atomic read (ie. with the protector variables)
  curElem->start = curRawTime;
  curElem->total = 0;
}

bool processTimerHK::perform(const tTimer *theTimer, 
                             pd_process *inferiorProc) {
   // returns true iff the process succeeded; i.e., if we were able to grab
   // the mutex for this tTimer and process without any waiting.  Otherwise,
   // we return false and don't process and don't wait.
   // If the thread obj hasn't yet been assigned to this dataReqNode, skip
   threadMetFocusNode_Val *thrNval = getThrNodeVal();

   if(thrNval == NULL) {
     return true;
   }

   // Timer sampling is trickier than counter sampling.  There are more race
   // conditions and other factors to carefully consider.

   // see the corresponding wall-timer routine for comments on how we
   // guarantee a consistent read value...

   // We used to take in a wall time to use as the time-of-sample, and a
   // process time to use as the current-process-time for use in fudge factor
   // when sampling an active process timer.  But we've found that both
   // values must be taken at the same time the sample is taken to avoid
   // incorrect values, leading to jagged spikes in the histogram
   // (i.e. incorrect samples).  This is too bad; it would be nice to read
   // these times just once per paradynd sample, instead of once per timer
   // per paradynd sample.

   volatile const int protector2 = theTimer->protector2;
   MEMORY_BARRIER;   // restricts processor from doing reads out-of-order
   // Do the following vrbles need to be volatile to ensure that they're read
   // between the reading of protector2 and protector1?  Probably not, but
   // we should keep an eye on the generated assembly to be sure...

   // We always need to use the first 2 vrbles:
   const int    count = theTimer->counter;
   const rawTime64 total = theTimer->total;
   const rawTime64 start = (count > 0) ? theTimer->start : 0; // not needed if count==0

   rawTime64 inferiorCPUtime;
   if(inferiorProc->multithread_capable()) {
      virtualTimer *vt = inferiorProc->getVirtualTimer(theTimer->index);
      assert(vt != NULL);
      assert(thrNval->getThread() != NULL);
      pd_thread *thr = thrNval->getThread();
      assert(thr != NULL);

      if (vt == (virtualTimer*) -1) {
         inferiorCPUtime = (count>0) ? inferiorProc->getRawCpuTime() : 0;
      } else {
         bool success = true ; // count <=0 should return true
         inferiorCPUtime =(count>0)?thr->getInferiorVtime(vt, success) : 0;
         if (!success) {
            return false ;
         }
      }
      if(inferiorCPUtime == -1)  //getRawCpuTime failed (perhaps process ended)
         return false;
   } else {
      inferiorCPUtime = (count>0) ? inferiorProc->getRawCpuTime() : 0;
      if(inferiorCPUtime == -1)  //getRawCpuTime failed (perhaps process ended)
         return false;
   }
   // This protector read and comparison must happen *after* we obtain the
   // inferior CPU time or thread virtual time, or we have a race condition
   // resulting in lots of annoying timer rollback warnings.  In the long
   // run, our data is still correct, but individual samples will be bad.
   MEMORY_BARRIER;
   volatile const int protector1 = theTimer->protector1;
   if (protector1 != protector2) {
      return false;
   }

   // Also cheating; see below.
   // the fudge factor is needed only if count > 0.
   const timeStamp currWallTime = getWallTime();
   // This is cheating a little; officially, this call should be inside
   // of the two protector vrbles.  But, it was taking too long...


   /* don't use 'theTimer' after this point! (avoid race condition).  To ensure
      this, we call calcTimeValueToUse() without passing 'theTimer' */
   rawTime64 rawTimeValueToUse = calcTimeValueToUse(count, start, total, 
                                                    inferiorCPUtime);
   // this is where conversion from native units to real time units is done
   timeLength timeValueToUse=inferiorProc->units2timeLength(rawTimeValueToUse);
   sample_cerr << "raw-total: " << total << ", timeValToUse: " 
		  << timeValueToUse << "\n";

   // Check for rollback; update lastTimeValueUsed (the two go hand in hand)
   // cerr <<"lastTimeValueUsed="<< lastTimeValueUsed << endl ;
   if (timeValueToUse < lastTimeValueUsed) {
      // Timer could roll back if the per-thread virtual timer is reused by
      // another thread, to avoid this to happen, we should reuse them with a
      // least recently used strategy.  See DYNINST_hash_delete in RTinst.c
      //ct_showTrace("daemon os cpu-timer history");
      // extern void vt_showTrace(char *msg);
      // vt_showTrace("daemon vtimer history");
      const char bell = ' '; //07;
      cerr << "processTimerHK::perform(): process timer rollback (" 
	   << timeValueToUse << "," << lastTimeValueUsed << ")" 
	   << bell << endl;
      cerr << "timer was " << (count > 0 ? "active" : "inactive") 
	   << ", start=" << start << ", total=" << total
	   << ", inferiorCPUtime=" << inferiorCPUtime << endl;

      timeValueToUse = lastTimeValueUsed;
      // assert(false);
   }
   else
      lastTimeValueUsed = timeValueToUse;

   if(! thrNval->isReadyForUpdates()) {
     sample_cerr << "mdn " << thrNval << " isn't ready for updates yet.\n";
     return false;
   }

   thrNval->updateValue(currWallTime, pdSample(timeValueToUse));
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



#ifdef PAPI
hwTimerHK &hwTimerHK::operator=(const hwTimerHK &src) {
   if (&src == this)
      return *this; // the usual check for x=x

   lastTimeValueUsed = src.lastTimeValueUsed;

   vTimer = NULL;
   genericHK::operator=(src);

   return *this;
}


bool hwTimerHK::perform(const tHwTimer *theTimer, pd_process *inferiorProc) {


   threadMetFocusNode_Val *thrNval = getThrNodeVal();
   if(thrNval == NULL) {
     return true;
   }

   volatile const int protector2 = theTimer->protector2;
   MEMORY_BARRIER;   // restricts processor from doing reads out-of-order

   const int    count = theTimer->counter;
   const rawTime64 total = theTimer->total;
   const rawTime64 start = theTimer->start;
   rawTime64 valueToUse;

   MEMORY_BARRIER;
   volatile const int protector1 = theTimer->protector1;
   if (protector1 != protector2) {
      return false;
   }

   if (count > 0) {
      int64_t currValue = inferiorProc->getPapiMgr()->getCurrentHwSample(hwEvent->getIndex());
      //fprintf(stderr, "MRM_DEBUG: catchup HW sample   currValue is %lld, start is %lld, hwCntrIndex is %d\n", currValue, start, hwCntrIndex);
      valueToUse = (currValue - start) + total;

      if ( (currValue - start) < 0) {
        fprintf(stderr, "hwTimerHK::perform() rollback\n");
      }
   }
   else {
      valueToUse = total;
   }



   if(! thrNval->isReadyForUpdates()) {
     sample_cerr << "mdn " << thrNval << " isn't ready for updates yet.\n";
     return false;
   }

   timeStamp currWallTime = getWallTime();

   thrNval->updateValue(currWallTime, pdSample(valueToUse));

   return true;
}

const tHwTimer hwTimerHK::initValue = { 0, 0, 0,
                                        0, 0, 0, 0 };

const tHwCounter hwCounterHK::initValue = { 0, 0 };

hwCounterHK &hwCounterHK::operator=(const hwCounterHK &src) {
   if (&src == this)
      return *this; // the usual check for x=x

   genericHK::operator=(src);

   return *this;
}

bool hwCounterHK::perform(const tHwCounter *dataValue, 
								  pd_process *inferiorProc) {
   threadMetFocusNode_Val *thrNval = getThrNodeVal();
   if(thrNval == NULL) {
     return true;
   }

   int64_t val;

   /* ********** MRM fix me for other platforms */
   __asm__ volatile ("fildq %0"  : : "g" (dataValue->value));
   __asm__ volatile ("fistpq %0" : "=m" (val)  );

   assert(thrNval->proc() == inferiorProc);
   if(! thrNval->isReadyForUpdates()) {
     sample_cerr << "mdn " << thrNval << " isn't ready for updates yet.\n";
     return false;
   }

   timeStamp currWallTime = getWallTime();

   thrNval->updateValue(currWallTime, pdSample(val));
      // the integer version of updateValue() (no int-->float conversion -- good)

   return true;
}

void hwTimerHK::initializeAfterFork(rawType *curElem, rawTime64 curRawTime)
{
  // if it's an active timer, than reset the start field
  curElem->start = curRawTime;
  curElem->total = 0;
}
#endif

