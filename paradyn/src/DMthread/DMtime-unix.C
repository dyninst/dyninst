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

/*
 * $Id: DMtime-unix.C,v 1.3 2004/03/23 01:12:26 eli Exp $
 * method functions for paradynDaemon and daemonEntry classes
 */
#include "DMinclude.h"
#include "DMtime.h"

#if defined(i386_unknown_solaris2_5) || defined(sparc_sun_solaris2_4)
#include <sys/time.h>
#endif

timeStamp getCurrentTime(void) {
    static double previousTime=0.0;
    double seconds_dbl;

#if defined(i386_unknown_solaris2_5) || defined(sparc_sun_solaris2_4)
    seconds_dbl = (double)gethrtime() / 1000000000.0F;    
    // converts nanoseconds to seconds

    if (seconds_dbl < previousTime)  seconds_dbl = previousTime;
    else  previousTime = seconds_dbl;
#else
    struct timeval tv;
  retry:
    bool aflag;
    aflag=(gettimeofday(&tv, NULL) == 0); // 0 --> success; -1 --> error
    assert(aflag);

    seconds_dbl = tv.tv_sec * 1.0;
    assert(tv.tv_usec < 1000000);
    double useconds_dbl = tv.tv_usec * 1.0;

    seconds_dbl += useconds_dbl / 1000000.0;

    if (seconds_dbl < previousTime) goto retry;
    previousTime = seconds_dbl;
#endif

    return seconds_dbl;
}

