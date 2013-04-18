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

#ifndef __TIMING_H
#define __TIMING_H

#include "common/src/Types.h"  // for getCurrentTimeRaw()
#include "common/src/Time.h"   // for getCurrentTime(), ...


/* Function for general time retrieval.  Use in dyninst, front-end; don't use
   in daemon when desire high res timer level if exists instead (see
   getWallTime in init.h) */
COMMON_EXPORT timeStamp getCurrentTime();

/* returns primitive wall time in microsecond units since 1970, 
   (eg. gettimeofday) used by getCurrentTime() */
COMMON_EXPORT int64_t getRawTime1970();

/* Calculates the cpu cycle rate value.  Needs to be called once,
   before any getCyclesPerSecond() call */
COMMON_EXPORT void initCyclesPerSecond();

/* Returns the cpu cycle rate.  Timeunits represented as units
   (ie. cycles) per nanosecond */
COMMON_EXPORT timeUnit getCyclesPerSecond();

/* Platform dependent, used by initCyclesPerSecond(). */
COMMON_EXPORT double calcCyclesPerSecondOS();

enum { cpsMethodNotAvailable = -1 };

/* A default stab at getting the cycle rate.  Used by calcCyclesPerSecondOS
   if a method which gets a more precise value isn't available. */
COMMON_EXPORT double calcCyclesPerSecond_default();


#endif



