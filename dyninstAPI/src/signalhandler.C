/*
 * Copyright (c) 1996 Barton P. Miller
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

// $Id: signalhandler.C,v 

#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/signalhandler.h"

signalHandler *global_sh = NULL;

signalHandler *getSH() {
   if(global_sh == NULL)
      global_sh = new signalHandler();

   return global_sh;
}

void signalHandler::checkForAndHandleProcessEvents(bool block) {
   pdvector<procevent *> foundEvents;
   bool res = checkForProcessEvents(&foundEvents, -1, block);
   if(res) {
      handleProcessEvents(foundEvents);
   }
}

void signalHandler::handleProcessEvents(pdvector<procevent *> &foundEvents) {
   getSH()->beginEventHandling(foundEvents.size());

   for(unsigned i=0; i<foundEvents.size(); i++) {
      procevent *ev = foundEvents[i];
      if(!handleProcessEventKeepLocked(*ev))
         fprintf(stderr, "handleProcessEvent failed!\n");
      delete ev;
   }

   continueLockedProcesses();
}

int signalHandler::handleProcessEventWithUnlock(const procevent &event) {
   int result = handleProcessEventKeepLocked(event);
   continueLockedProcesses();
   return result;
}

int signalHandler::handleProcessEventKeepLocked(const procevent &event) {
   process *proc = event.proc;
   if(numEventsToProcess > 1)
      proc->lock_continues();
   procs_with_locked_statuses.push_back(proc);
   numEventsProcessed++;
   return handleProcessEventInternal(event);
}

void signalHandler::continueLockedProcesses() {
   if(numEventsProcessed < numEventsToProcess) {
      cerr << "   error in continueLockedProcesses, there are "
           << numEventsToProcess << " events to process\n    at this point, "
           << " but have only processed " << numEventsToProcess << " events.";
      cerr << "  Can't continue process until all events have been handled.";
      assert(false);
   }

   for(unsigned j=0; j<procs_with_locked_statuses.size(); j++) {
      process *cur_proc = procs_with_locked_statuses[j];
      cur_proc->unlock_continues();
   }
   procs_with_locked_statuses.clear();
}


