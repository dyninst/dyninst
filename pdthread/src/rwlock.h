/*
 * Copyright (c) 1996-2003 Barton P. Miller
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

/************************************************************************
 * rwlock.h: declaration of rwlock class
 * 
 * A rwlock is the reader-writer locking implementation for the
 * thread library's thread table.  It relies on the pthreads 
 * locking primitives.  Some ideas for this code come from Butenhof's
 * _Programming With POSIX Threads_.  (AWL, 1997)
 *
 * NB!  There are no cleanup handlers in this code.  If you use thread
 * cancellation, you deserve what you get.  (Note to libthread
 * developers: DO NOT use thread cancellation -- it is bad and
 * inherently extremely unsafe.)
 *
 * NB!  This code does not check to see whether you have a lock before 
 * you release it.  Be smart.
 *
 * $Id: rwlock.h,v 1.2 2003/10/28 18:30:03 pcroth Exp $
************************************************************************/
#ifndef _thread_src_rwlock_h_
#define _thread_src_rwlock_h_

#include "xplat/h/Monitor.h"

namespace pdthr
{


#define RWLOCK_FAVOR_READERS 0xda7aba5e
#define RWLOCK_FAVOR_WRITERS 0xac71014 

class rwlock {  
public:
    enum locktype {read, write};
    enum preftype {favor_readers, favor_writers};

private:
    static const unsigned int READ_CVID;
    static const unsigned int WRITE_CVID;

    preftype preference;
    
    XPlat::Monitor* monitor;
    int active_readers; 
    int active_writers;
    int waiting_readers;
    int waiting_writers;
    
public:
        /* The default constructor for rwlock creates a table of the
           default size, favoring writers */
    rwlock(preftype favor=favor_writers)
      : preference(favor),
        monitor( new XPlat::Monitor ),
        active_readers(0), active_writers(0), 
        waiting_readers(0), waiting_writers(0)
    { }
    
    int start_reading();
    int start_writing();  
    int stop_reading();
    int stop_writing();
    
    int acquire(rwlock::locktype lt);
    int release(rwlock::locktype lt);
};

typedef class rwlock rwlock_t;

} // namespace pdthr

#endif // !defined(_thread_src_rwlock_h_)
