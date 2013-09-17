/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

// $Id: timing.C,v 1.26 2007/05/30 19:20:36 legendre Exp $

#include "common/src/Timer.h"
#include "common/src/timing.h"
#include "common/src/Time.h"

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
