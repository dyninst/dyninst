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

/* $Id: util.C,v 1.28 2004/04/02 06:34:15 jaw Exp $
 * util.C - support functions.
 */

#include "common/h/headers.h"
#ifndef BPATCH_LIBRARY
#include "rtinst/h/rtinst.h" // for time64
#endif
#include "dyninstAPI/src/util.h"
#include "common/h/Time.h"

// TIMING code

#if defined(i386_unknown_solaris2_5) || defined(sparc_sun_solaris2_4)
#include <sys/time.h>  // for gethrtime()
#endif


#ifndef BPATCH_LIBRARY
timeStamp *pFirstRecordTime = NULL;

void setFirstRecordTime(const timeStamp &ts) {
  if(pFirstRecordTime != NULL) delete pFirstRecordTime;
  pFirstRecordTime = new timeStamp(ts);
}
bool isInitFirstRecordTime() {
  if(pFirstRecordTime == NULL) return false;
  else return true;
}
const timeStamp &getFirstRecordTime() {
  if(pFirstRecordTime == NULL) assert(0);
  return *pFirstRecordTime;
}
#endif

static unsigned addrHashCommon(Address addr) {
   // inspired by hashs of string class

   register unsigned result = 5381;

   register Address accumulator = addr;
   while (accumulator > 0) {
      // We use 3 bits at a time from the address
      result = (result << 4) + result + (accumulator & 0x07);
      accumulator >>= 3;
   }

   return result;
}

unsigned addrHash(const Address & iaddr) {
   return addrHashCommon(iaddr);
}

unsigned addrHash4(const Address &iaddr) {
   // call when you know that the low 2 bits are 0 (meaning they contribute
   // nothing to an even hash distribution)
   return addrHashCommon(iaddr >> 2);
}

unsigned addrHash16(const Address &iaddr) {
   // call when you know that the low 4 bits are 0 (meaning they contribute
   // nothing to an even hash distribution)
   return addrHashCommon(iaddr >> 4);
}

void
pd_log_perror(const char* msg) {
    sprintf(errorLine, "%s: %s\n", msg, strerror(errno));
    logLine(errorLine);
    // fprintf(stderr, "%s", log_buffer);
}




