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
 * rwlock.c: implementation of reader-writer lock classes.
 *
 * $Id: rwlock.C,v 1.2 2003/10/28 18:30:03 pcroth Exp $
************************************************************************/
#include <stdlib.h>
#include <assert.h>
#include "rwlock.h"
#include "xplat/h/Monitor.h"

namespace pdthr
{

const unsigned int rwlock::READ_CVID = 0;
const unsigned int rwlock::WRITE_CVID = 1;


int rwlock::start_reading() { 
    int status = 0, doblock=0;

    status = monitor->Lock();
    if (status != 0) 
    {
        return status;
    }
    doblock = active_writers || (preference == rwlock::favor_writers && waiting_writers);
    
    if (doblock) {
        waiting_readers++;
        while(doblock) {
            status = monitor->WaitOnCondition( READ_CVID );
            doblock = active_writers || (preference == rwlock::favor_writers && waiting_writers);
            if(status != 0)
            {
                break;
            }
        }
        waiting_readers--;
    }
    
    if(status == 0)
    {
        active_readers++;
    }
    monitor->Unlock();

    return status;
}

int rwlock::start_writing() {
    int status = 0;

    status = monitor->Lock();
    if (status != 0) 
    {
        return status;
    }
    
    if (active_writers || active_readers > 0) {
        waiting_writers++;
        while(active_writers || active_readers > 0) {
            status = monitor->WaitOnCondition( WRITE_CVID );
            if(status != 0)
            {
                break;
            }
        }
        waiting_writers--;
    }
    if(status == 0)
    {
        active_writers = 1;
    }
    monitor->Unlock();

    return status;
}

int rwlock::stop_reading() {
    int status = 0, status2 = 0;

    status = monitor->Lock();
    if (status != 0)
    {
        return status;
    }
    active_readers--;
    if (active_readers == 0 && waiting_writers > 0)
    {
        status = monitor->SignalCondition( WRITE_CVID );
    }
    status2 = monitor->Unlock();

    return (status || status2);
}

int rwlock::stop_writing() {
    int status = 0;

    status = monitor->Lock();
    if (status != 0)
    {
        return status;
    }
    active_writers = 0;
    if (preference == rwlock::favor_writers) {
        if (waiting_writers > 0) {
            monitor->SignalCondition( WRITE_CVID );
        } else if (waiting_readers > 0) {
            monitor->BroadcastCondition( READ_CVID );
        }
    } else {
            /* favoring readers */
        if (waiting_readers > 0) {
            monitor->BroadcastCondition( READ_CVID );
        } else if (waiting_writers > 0) {
            monitor->SignalCondition( WRITE_CVID );
        }
    }
    
    status = monitor->Unlock();

    return status;
}

int rwlock::acquire(rwlock::locktype lt) { 
    switch(lt) {
        case rwlock::read: return start_reading();
        case rwlock::write: return start_writing();
    }
    return 0;
}

int rwlock::release(rwlock::locktype lt) { 
    switch(lt) {
        case rwlock::read: return stop_reading();
        case rwlock::write: return stop_writing();
    }
    return 0;
}

} // namespace pdthr

