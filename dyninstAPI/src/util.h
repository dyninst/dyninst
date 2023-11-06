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

// $Id: util.h,v 1.41 2008/06/19 22:13:43 jaw Exp $

#ifndef UTIL_H
#define UTIL_H

#ifndef FILE__
#include <string.h>
#define FILE__ strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__
#endif

#include <string>
#include "common/src/headers.h"
#include "common/src/stats.h"

extern void printDyninstStats();
extern CntStatistic insnGenerated;
extern CntStatistic totalMiniTramps;
extern CntStatistic trampBytes;
extern CntStatistic ptraceOps;
extern CntStatistic ptraceOtherOps;
extern CntStatistic ptraceBytes;
extern CntStatistic pointsUsed;

bool waitForFileToExist(char *fname, int timeout_seconds);
int openFileWhenNotBusy(char *fname, int flags, int mode, int timeout_seconds);

inline unsigned uiHash(const unsigned &val) {
  return val;
}

inline unsigned CThash(const unsigned &val) {
  return val % 1048573;
}

unsigned ptrHash4(void *ptr);
unsigned ptrHash16(void *ptr);

inline unsigned intHash(const int &val) {
  return val;
}

void
dyninst_log_perror(const char* msg);

#endif /* UTIL_H */
