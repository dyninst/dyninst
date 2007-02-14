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

// $Id: timing.C,v 1.25 2007/02/14 23:03:29 legendre Exp $

#include "common/h/Timer.h"
#include "common/h/timing.h"
#include "common/h/Time.h"

/* time retrieval function definitions */

timeStamp getCurrentTime() {
  return timeStamp(getRawTime1970(), timeUnit::us(), timeBase::b1970());
}

#if !defined(os_windows)

// returns us since 1970
int64_t getRawTime1970() {
  struct timeval tv;
  if (-1 == gettimeofday(&tv, NULL)) {
    perror("getCurrentTime gettimeofday()");
    return 0;
  }
  int64_t result = tv.tv_sec;
  result *= 1000000;
  result += tv.tv_usec;
  return result;
}
#endif


// Shows the time until the auxillary fraction conversion algorithm
// will be used in order to get around internal rollover 
// (see util/src/Time.C).  Auxiliary algorithm requires an additional
// 64bit integer div, mult, modulas, addition.  Conversion efficiency
// to this degree most likely isn't relevant.
//
//         Timing statistics for a 1999 MHz machine
// fract numer    MHz precision     time until aux alg   time err/year
//    1000        1 MHz             7.6 weeks            2.2 hrs
//   10000        100,000 Hz        5.3 days             13 minutes
//  100000         10,000 Hz        12 hrs               1.3 minutes   *
// 1000000          1,000 Hz        1.2 hrs              8 seconds
//
//         Timing statistics for a  199 MHz machine
// fract numer    MHz precision     time until aux alg   time err/year
//    1000        1 MHz             7.6 weeks            22 hrs
//   10000        100,000 Hz        5.3 days             2.2 hrs
//  100000         10,000 Hz        12 hrs               13 minutes    *
// 1000000          1,000 Hz        1.2 hrs              1.3 minutes
//
// * currently using
// 10,000 Hz precision, 12 hrs time until aux alg, seems like a good compromise

// access only through getCyclesPerSecond()
timeUnit *pCyclesPerSecond = NULL;

void initCyclesPerSecond() {
  double cpsHz = calcCyclesPerSecondOS();
  double cpsTTHz = cpsHz / 10000.0;
  // round it
  cpsTTHz = cpsTTHz + .5;
  int64_t tenThousHz = static_cast<int64_t>(cpsTTHz);
  // in case of multiple calls
  if(pCyclesPerSecond != NULL) delete pCyclesPerSecond;
  pCyclesPerSecond = new timeUnit(fraction(100000, tenThousHz));
}

timeUnit getCyclesPerSecond() {
  if(pCyclesPerSecond == NULL) { 
    cerr << "getCyclesPerSecond(): cycles per second hasn't been initialized\n";
    assert(0);
  }
  return (*pCyclesPerSecond);
}
