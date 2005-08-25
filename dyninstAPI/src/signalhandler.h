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

/* $Id: signalhandler.h,v 1.14 2005/08/25 22:45:56 bernat Exp $
 */

/*
 * This file describes the entry points to the signal handling
 * routines. This is meant to provide a single interface to bother
 * the varied UNIX-style handlers and the NT debug event system.
 * Further platform-specific details can be found in the
 * signalhandler-unix.h and signalhandler-winnt.h files.
 */

#ifndef _SIGNAL_HANDLER_H
#define _SIGNAL_HANDLER_H

#if defined(i386_unknown_nt4_0)
#include "dyninstAPI/src/signalhandler-winnt.h"
#else
#include "dyninstAPI/src/signalhandler-unix.h"
#endif

#include "codeRange.h"

class process;
class dyn_lwp;


// Return code:
// 0: no event
// 1: event fount

// pid: -1 for all processes
// why: Why the process stopped (return parameter)
// what: what caused the stop (return parameter)
// block: block waiting for a signal?
// waitProcs replacement

class signalHandler {
   int handleProcessEventInternal(const procevent &event);

 public:
   signalHandler() { }

   // checks for process events and handles any events that were found
   // Returns whatever it doesn't know how to handle
   pdvector <procevent *> checkForAndHandleProcessEvents(bool block);

   // checkForProcessEvents: check whether there is an event on any process
   // we're debugging. If one is found decode it and return.  Returns true if
   // found events, otherwise false.
   // If timeout is -1, this function will block.
   // Otherwise, it will return after waiting for specified timeout (in ms)
   // timeout will be set to 0 if the function timed out.
   bool checkForProcessEvents(pdvector<procevent *> *events,
                              int wait_arg, int &timeout);

   // handles process events, unlocks locked processes, deletes proc events
   // Returns events it doesn't know what to do with
   pdvector <procevent *> handleProcessEvents(pdvector<procevent *> &foundEvents);
   int handleProcessEvent(const procevent &event);
};

class signal_handler_location : public codeRange {
 public:
    signal_handler_location(Address addr, unsigned size);
    Address get_address_cr() const { return addr_; }
    unsigned get_size_cr() const { return size_; }
    void *getPtrToInstruction(Address addr) const { assert(0); return NULL; }

 private:
    Address addr_;
    unsigned size_;
};

extern signalHandler *global_sh;
signalHandler *getSH();

/////////////////////
// Callbacks.
/////////////////////

// Note: Replace with BPatch callbacks when Paradyn can use them
// Fork entry
typedef void (*forkEntryCallback_t)(process *p, void *data);
// Fork exit: include new pid
typedef void (*forkExitCallback_t)(process *p, void *data, process *child);
// Exec entry: include program argument
typedef void (*execEntryCallback_t)(process *p, void *data, char *arg0);
// Exec exit
typedef void (*execExitCallback_t)(process *p, void *data);
// Exit entry: include exit code
typedef void (*exitEntryCallback_t)(process *p, void *data, int code);


#endif


