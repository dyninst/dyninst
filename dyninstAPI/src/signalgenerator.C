/*
 * Copyright (c) 1996-2009 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
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

// $Id: signalhandler.C,v 

#include <string>
#include "process.h"
#include "dyn_lwp.h"
#include "dyn_thread.h"
#include "callbacks.h"
#include "function.h"
#include "common/h/stats.h"
#include "signalhandler.h"
#include "signalgenerator.h"
#include "mapped_object.h"
#include "eventgate.h"
//  signal_generator_counter is used to generate identifier strings
//  for signal generator threads.  eg SYNC1, SYNC2, SYNC3

unsigned signal_generator_counter = 0;
eventLock *SignalGeneratorCommon::global_wait_list_lock;
pdvector<EventGate *> *SignalGeneratorCommon::global_wait_list;
global_wait_list_init_t SignalGeneratorCommon::wlinit;

global_wait_list_init_t::global_wait_list_init_t()
{
  SignalGeneratorCommon::global_wait_list = new pdvector<EventGate *>();
  SignalGeneratorCommon::global_wait_list_lock = new eventLock();
}

const char *processRunStateStr(processRunState_t runState) {
    switch (runState) {
    case unsetRequest: return "unset";
    case stopRequest: return "stopped";
    case runRequest: return "run";
    case ignoreRequest: return "ignoring";
    default: assert(0); return "<ASSERT FAILURE>";
    }
    return "<IMPOSSIBLE CASE";
}


/*
 * So here's the deal. We want to basically use this structure:
 *
 * TOP
 *   if process is paused:
 *     wait until it runs.
 *     goto TOP
 *   if process is not paused:
 *     wait for an OS event
 *       ... set process state to paused
 *     decode the event (lowlevel data -> high level representation)
 *     dispatch the event to a signal handler
 *     goto TOP
 *
 * So basically we sit there looping. There are some complications, though. 
 * The structure (with locking) looks like so: (( is taking the global lock, ))
 * releasing it.
 *
 *
 * TOP
 *  ((
 *   if process is paused:
 *     ))
 *     wait until it runs.
 *     goto TOP
 *   if process is not paused:
 *     ))
 *     wait for an OS event
 *     ((
 *     set process state to paused
 *     decode the event (lowlevel data -> high level representation)
 *     dispatch the event to a signal handler
 *     ))
 *     goto TOP
 *
 * But what of UI thread interaction? We don't have a way of handing off
 * control between the signal generator and the signal handler, so the UI
 * thread may try and get in between those. We avoid that by considering the
 * signalgenerator "active" from the decode stage until there are no running
 * signal handlers. Also, the UI thread may race with the return from poll/waitpid
 * (as those are unlocked). To avoid the second, we recheck the process status
 * when poll/waitpid returns, leading to:
 *
 * TOP
 *  ((
 *   if process is paused:
 *     Collect continue events; when neither the UI thread nor any
 *       handler thread is executing, continue the process.
 *     ))
 *     wait until it runs.
 *     goto TOP
 *   if process is not paused:
 *     ))
 *     wait for an OS event
 *     ((
 *     if process is pausing:
 *        signal return of poll to pausing thread
 *     set process state to paused
 *     decode the event (lowlevel data -> high level representation)
 *     dispatch the event to a signal handler
 *     ))
 *     goto TOP
 *
 * Confused yet?
 */

void SignalGeneratorCommon::main() {
    MONITOR_ENTRY();

    addToThreadMap();
    
    startupLock->_Lock(FILE__, __LINE__);
    signal_printf("%s[%d]:  about to do init for %s\n", FILE__, __LINE__, idstr);
    if (!initialize_event_handler()) {
        signal_printf("%s[%d]: initialize event handler failed, %s returning\n", FILE__, __LINE__, idstr);
        _isRunning = false;
        init_ok = false; 
        // For anyone who asks later
        stop_request = true; 

        removeFromThreadMap();

        startupLock->_Broadcast(FILE__, __LINE__);
        startupLock->_Unlock(FILE__, __LINE__);
        return;
    }
    
    init_ok = true;
    signal_printf("%s[%d]:  init success for %s\n", FILE__, __LINE__, idstr);
    
    _isRunning = true;
    startupLock->_Broadcast(FILE__, __LINE__);
    startupLock->_Unlock(FILE__, __LINE__);
    
    signal_printf("%s[%d]:  before main loop for %s\n", __FILE__, __LINE__, idstr);

    eventlock->_Lock(FILE__, __LINE__);
    pdvector<EventRecord> events;
    while (1) {
        // TOP
        signal_printf("%s[%d]: signal generator at top of loop\n", FILE__, __LINE__);
        assert(haveLock());
        
        if (exitRequested() && !requested_wait_until_active) {
            signal_printf("%s[%d]: exit request (loop top)\n", FILE__, __LINE__);
            break;
        }

        // If there is an event to handle, then keep going. 
        if (processIsPaused() || requested_wait_until_active) {
            signal_printf("%s[%d]: process is paused, waiting (loop top)\n", FILE__, __LINE__);
            // waitForActive... used to unlock/relock the global mutex. 
            waitForActivation();

            // Check for process detach/exit...
            signal_printf("%s[%d]: post-activation, process status %s\n",
                          FILE__, __LINE__, 
                          proc ? proc->getStatusAsString().c_str() : "NULL");

            // We blocked; therefore, check to see if the process is gone
            if( exitRequested() ) {
                signal_printf("%s[%d]: exit request (post-waitForActivation)\n", FILE__, __LINE__);
                break;
            }            

            // waitForActiveProcess will return when everyone agrees that the process should
            // run. This means:
            // 1) All signal handlers are either done or waiting in a callback;
            // 2) If there was a pause request from BPatch, then there has been a run request.
            if (continueRequired())  {
                continueProcessInternal();
            }
            // Post cleanup
            // Cleanup...

            asyncRunWhenFinished_ = unsetRequest;
            // Commented out; set by BPatch layer
            //syncRunWhenFinished_ = unsetRequest;
        }

        postSignalHandler();

        // Process not paused
        signal_printf("%s[%d]: Grabbing event\n", FILE__, __LINE__);

        getEvents(events);

        checkActiveProcess();

        if (exitRequested()) {
            signal_printf("%s[%d]: exit request (post-getEvent)\n", FILE__, __LINE__);
            break;
        }

        signal_printf("%s[%d]: decoding event\n", FILE__, __LINE__);
        decodingEvent_ = true;
        // Translate everything in the events_to_handle vector from low-level
        // (OS return) to high-level (Dyninst event)

        decodeEvents(events);

        // Overlapping control areas...
        dispatchingEvent_ = true; decodingEvent_ = false;
        
        for (unsigned i = 0; i < events.size(); i++) {
            signal_printf("%s[%d]: dispatching event %d\n", FILE__, __LINE__, i);
            dispatchEvent(events[i]);
        }
        events.clear();
        dispatchingEvent_ = false;
    }        

    // We're going down...
    signalEvent(evtShutDown);


    while(numBlockedForContinue) {
        waitForContinueLock->_Lock(FILE__, __LINE__);
        eventlock->_Unlock(FILE__, __LINE__);
        waitForContinueLock->_Broadcast(FILE__, __LINE__);
        waitForContinueLock->_Unlock(FILE__, __LINE__);
        eventlock->_Lock(FILE__, __LINE__);
    }

    // Grab exit lock...
    signal_printf("%s[%d]: SG grabbing exit lock\n", FILE__, __LINE__);
    waitForHandlerExitLock->_Lock(FILE__, __LINE__);

    signal_printf("%s[%d]: SG releasing global mutex...\n", FILE__, __LINE__);
    eventlock->_Unlock(FILE__, __LINE__);

    // And set all handlers to exit
    for (unsigned i = 0; i < handlers.size(); i++) {
        // Force exit of signal handler
        handlers[i]->stop_request = true;
        if (handlers[i]->waitingForWakeup_) {
            handlers[i]->waitLock->_Broadcast(FILE__, __LINE__);
        }
    }

    signal_printf("%s[%d]: SG waiting for handlers to exit\n", FILE__, __LINE__);
    while(handlers.size()) {
        waitForHandlerExitLock->_WaitForSignal(FILE__, __LINE__);
    }

    eventlock->_Lock(FILE__, __LINE__);
            

    waitForHandlerExitLock->_Unlock(FILE__, __LINE__);

    // The pointer is owned by the BPatch_process, since it can
    // be deleted by the BPatch user. So we can't delete it; 
    // however, we can (and should) call deleteProcess so that it
    // gets cleaned up
    // Proc may be null if initialization failed; in this case,
    // don't try to clean up.
    if (proc) {
        proc->deleteProcess();
        // Set our process pointer to NULL; DO NOT DELETE!!!
        // DO NOT DELETE
        proc->sh = NULL;
        proc = NULL;
    }

    thread_printf("%s[%d]: removing from thread map\n", FILE__, __LINE__);
    removeFromThreadMap();
    
    _isRunning = false;
    if (eventlock->depth() != 1) {
        fprintf(stderr, "%s[%d]:  WARNING:  global_mutex->depth() is %d, leaving thread %s\n",
                FILE__, __LINE__, global_mutex->depth(),idstr);
        global_mutex->printLockStack();
    }
    global_mutex->_Broadcast(FILE__, __LINE__);
    global_mutex->_Unlock(FILE__, __LINE__);
    
    thread_printf("%s[%d][%s]:  SignalGenerator::main exiting\n", FILE__, __LINE__, idstr);

    MONITOR_EXIT();
}

bool SignalGeneratorCommon::exitRequested() 
{
    // Commented out for reference: we handle this at
    // dispatch time. 
    //    if (ev.type == evtShutdown) return true;

    if (stop_request)
        return true;

    if (!proc) {
        // This can be called if there is no process; failed fork, for instance.
        return false;
    }

    if (proc->status() == deleted)
        return true;
    if (proc->status() == exited)
        return true;
    if (proc->status() == detached)
        return true;

    return false;
}


void SignalGeneratorCommon::waitForActivation() {

    assert(eventlock->depth() == 1);

    assert(processIsPaused() || requested_wait_until_active);

    while (1) {
        
        // Now, do we want to return or wait for more people?
        
        // bool syncRunWhenFinished_ -- member vrble, set to true if BPatch
        // has requested a run (and false when they stop...)
        
        bool activeHandler = false;
        
        for (unsigned i = 0; i < handlers.size(); i++) {
            if (handlers[i]->processing()) {
                signal_printf("%s[%d]: waitForActivation: handler %s active, returning true\n",
                              FILE__, __LINE__, getThreadStr(handlers[i]->getThreadID()));
                activeHandler = true;
            }
        }

        // bool asyncRunWhenFinished_ - member vrble, set to true if any signal handler
        // requested us to run when it finished up (aka executed process->continueProc)
        
        signal_printf("%s[%d]: waitForActivation loop top: activeProcessSignalled_ %d, syncRun %s, activeHandler %d, asyncRun %s\n",
                      FILE__, __LINE__,
                      activeProcessSignalled_, processRunStateStr(syncRunWhenFinished_),
                      activeHandler, processRunStateStr(asyncRunWhenFinished_));

        // Ordering... activeProcessSignalled activeHandler beats
        // syncRunWhenFinished beats asyncRunWhenFinished.
        if (activeProcessSignalled_) {
            signal_printf("%s[%d]: active process signalled, breaking out of wait loop;\n", 
                          FILE__, __LINE__);
            break;
        }
        else if (activeHandler) {
            signal_printf("%s[%d]: active handler, continuing around;\n", 
                          FILE__, __LINE__);
        }
        else if (syncRunWhenFinished_ == stopRequest) {
            signal_printf("%s[%d]: syncRunWhenFinished is stay stopped, continuing around;\n", 
                          FILE__, __LINE__);
        }
        else if (syncRunWhenFinished_ == runRequest) {
            signal_printf("%s[%d]: syncRunWhenFinished is run, breaking out of wait loop;\n", 
                          FILE__, __LINE__);
            break;
        }
        else if (asyncRunWhenFinished_ == stopRequest) {
            signal_printf("%s[%d]: asyncRunWhenFinished is stay stopped, waiting;\n", 
                          FILE__, __LINE__);
        }
        else if (asyncRunWhenFinished_ == runRequest) {
            signal_printf("%s[%d]: asyncRunWhenFinished is run, breaking;\n", 
                          FILE__, __LINE__);
            break;
        }
        else 
            signal_printf("%s[%d]: default case, waiting.\n", 
                          FILE__, __LINE__);

        // Otherwise, wait for someone to get a round tuit.
        assert(activationLock);
        
        signal_printf("%s[%d]: Grabbing the activationLock...\n", FILE__, __LINE__);
        activationLock->_Lock(FILE__, __LINE__);

        assert(eventlock->depth() == 1);
        eventlock->_Unlock(FILE__, __LINE__);
        
        signal_printf("%s[%d]: released global lock in waitForActivation\n", FILE__, __LINE__);
        
        waitingForActivation_ = true;
        signal_printf("[%s:%d]: waiting for process to be active\n", FILE__, __LINE__);
        activationLock->_WaitForSignal(FILE__, __LINE__);
        signal_printf("[%s:%d]: process received activation request\n", FILE__, __LINE__);
        waitingForActivation_ = false;
        activationLock->_Unlock(FILE__, __LINE__);
        
        // Left in here and disabled - if we say "continue, pause"
        // on the UI thread the generator will be woken up but not 
        // (necessarily) paused. In this case we'll get the lock, 
        // find out that we're paused, and go through it again.
        //assert(!processIsPaused());   
        
        signal_printf("%s[%d]: reacquiring global lock\n", FILE__, __LINE__);
        eventlock->_Lock(FILE__, __LINE__);

        assert(eventlock->depth() == 1);
    }
}

bool SignalGeneratorCommon::continueProcessAsync(int signalToContinueWith, dyn_lwp *lwp) {
    if (exitRequested()) {
        //fprintf(stderr, "%s[%d]:  continueProcessAsync: exit requested, ignoring\n", FILE__, __LINE__);
        // We're going away... so don't do anything
        return true;
    }

    // Fixes an odd case when we receive multiple fork exit events; as soon as
    // we've handled one skip the rest.
    childForkStopAlreadyReceived_ = true;

    // First, let see if there was a signal to continue with. Only one person can send
    // a signal; complain if there are multiples. 

    // Technically, this is one per LWP... but we're not there yet.
    setContinueSig(signalToContinueWith);

    // Grab the activation lock, set asyncRun... and signal the SG if necessary
    assert(activationLock);
    signal_printf("%s[%d]: async continue grabbing activation lock\n", FILE__, __LINE__);
    activationLock->_Lock(FILE__, __LINE__);
    
    if (lwp) {
        signal_printf("%s[%d]: adding lwp %d to continue list...\n",
                      FILE__, __LINE__, lwp->get_lwp_id());
        lwpsToContinue_.push_back(lwp);
    }
    else {
        // Someone wants us to run the whole nine yards
        continueWholeProcess_ = true;
    }

    if (waitingForOS_ && !independentLwpStop_) {
        // We managed to get into waitpid (prolly signalActiveProcess) without
        // continuing the process...
        signal_printf("%s[%d]: Raced with SG %s, in waitpid, going to continue...\n",
                      FILE__, __LINE__, getThreadStr(getThreadID()));
        // Okay, we must be in a "lwps are partially running" situation. Feed in
        // the LWP we were given in continue.
        // If we're in a stop situation we can't continue the process; someone else needs
        // the stop as well.

	bool other_handler = false;
	for (unsigned i = 0; i < handlers.size(); i++) {
		signal_printf("%s[%d]: checking handler %d for activity\n",
			FILE__, __LINE__, i);
		if (getExecThreadID() == handlers[i]->getThreadID()) {
			signal_printf("%s[%d]: checked self\n", 
			FILE__, __LINE__);
			continue;
		}
		if (!handlers[i]->processing()) {
			signal_printf("%s[%d]: handler is not processing\n",
			FILE__, __LINE__);
			continue;
		}
		if (!handlers[i]->events_to_handle.size()) {
			signal_printf("%s[%d]: handler has no events to handle\n",
			FILE__, __LINE__);
			continue;
		}
		if (handlers[i]->events_to_handle[0].lwp == lwp) {
			signal_printf("%s[%d]: handler is waiting for current LWP!\n",
					FILE__, __LINE__);
			other_handler = true;
			continue;
		}
		signal_printf("%s[%d]: handler does not match\n", FILE__, __LINE__);
	}
	
	if (!other_handler) {
        	signal_printf("%s[%d]: continuing process from non-SG thread\n",
                	      FILE__, __LINE__);
        	continueProcessInternal();
	}
	else {
		signal_printf("%s[%d]: other handler waiting, skipping continue\n",
			FILE__, __LINE__);
	}
        signal_printf("%s[%d]: async continue broadcasting...\n", FILE__, __LINE__);
        activationLock->_Broadcast(FILE__, __LINE__);
        activationLock->_Unlock(FILE__, __LINE__);
        return true;
    }

    asyncRunWhenFinished_ = runRequest;

    signal_printf("%s[%d]: async continue broadcasting...\n", FILE__, __LINE__);
    activationLock->_Broadcast(FILE__, __LINE__);
    signal_printf("%s[%d]: async continue releasing activation lock\n", FILE__, __LINE__);
    activationLock->_Unlock(FILE__, __LINE__);

    // Note: this can wake up the SG before the SH is done handling. However, the global
    // lock is still held, and the SG re-acquires that before checking state again. So the 
    // order of events *should* be:
    // SH signals run
    // SH finishes
    // SH releases lock
    // SG acquires lock
    // SG checks and discovers all SHs are done

    return true;
}

// This is a logical, not physical, call. The process is "paused" if one of the
// following is true:
// 
// 1) There has been a BPatch_process pause (stopExecution) without a 
//   more recent continue (continueExecution).
// 2) There is an active signal handler. A signal handler is active if all of the
//   following are true:
//    a) There is an event in the SH queue.
//    b) The SH is not blocked in a callback.

bool SignalGeneratorCommon::processIsPaused() { 

    if (stop_request) return false;

    assert(proc);

    if (!proc->reachedBootstrapState(attached_bs)) {
        signal_printf("[%s:%d]: override processIsPaused; process not attached\n",
                      FILE__, __LINE__);
        return false;
    }

    // Return true if: global process status is paused, and there
    // are _no_ LWPs that are running.                  
    
    dyn_lwp *lwp = proc->query_for_running_lwp();
    
    signal_printf("%s[%d]: process state %s, running lwp %d\n",
                  FILE__, __LINE__,
                  proc->getStatusAsString().c_str(),
                  (lwp != NULL) ? lwp->get_lwp_id() : (unsigned) -1);

    bool isPaused = false;
    if (proc->status() == neonatal) isPaused = false;
    else if (independentLwpStop_ && proc->query_for_running_lwp()) isPaused = false;
    else if (proc->query_for_stopped_lwp() == NULL) isPaused = false;
    else if (proc->status() != running) isPaused = true;

    signal_printf("%s[%d]: decision is %s\n", FILE__, __LINE__, isPaused ? "paused" : "running");

    return isPaused;
}

// True if: the signal generator is decoding (we'd have a lock,
// but hey...) or if there is a running signal handler. Under the
// current design the generation and handling phases are decoupled,
// and it's possible for someone else (say, the UI thread) to nip
// in between dispatch (in the generator) and handling (in the 
// handler). We 'cover' that gap with this function - it basically
// says 'back off' to the UI thread.

bool SignalGeneratorCommon::isActivelyProcessing() {
    if (decodingEvent_) {
        signal_printf("%s[%d]: decoding event, returning active\n",
                      FILE__, __LINE__);
        return true;
    }
    if (dispatchingEvent_) {
        signal_printf("%s[%d]: dispatching event, returning active\n",
                      FILE__, __LINE__);
        return true;
    }

    // Signal handlers have three states: idle (waiting
    // for an event), processing (handling an event), 
    // or waiting (waiting for a callback to complete). 
    // If the handler is processing, we consider it active.
    
    for (unsigned i = 0; i < handlers.size(); i++) {
        if (handlers[i]->processing()) {
            // If we're the same thread (inquiring about ourselves),
            // override...
            if (getExecThreadID() == handlers[i]->getThreadID()) {
                signal_printf("%s[%d]: handler %s processing, same thread, skipping\n",
                              FILE__, __LINE__, getThreadStr(handlers[i]->getThreadID()));
                continue;
            }
            else {
                signal_printf("%s[%d]: handler %s processing, returning active\n",
                              FILE__, __LINE__, getThreadStr(handlers[i]->getThreadID()));
                return true;
            }
        }
    }
    return false;
}
	
// When we're blocked on the OS we release the lock (as 
// we have to), and this means that pretty much anything
// can happen with the process state. Frex, the UI thread
// can come in and attempt a pause on the process. If
// we jump out of the poll/waitpid and start going, this
// can confuse the heck out of the UI thread. So we need to know
// if someone tried to pause the process while we were waiting;
// in this case, we back off and wait for the process to run.

bool SignalGeneratorCommon::pauseAttemptedOnProcess() {
	// Simple approach: if process state is paused (and
	// it wasn't when we went into waitpid), back off.
	return (processPausedDuringOSWait_);
}


// Either returns an event from the pending queue,
// or waits for a new one from the OS.

bool SignalGeneratorCommon::getEvents(pdvector<EventRecord> &events) 
{
    assert(eventlock->depth() > 0);

    // should be empty; we dispatch simultaneously
    assert(events.size() == 0);
    
    if (stop_request) return false;
    assert(proc);
    
    return waitForEventsInternal(events);
}


bool SignalGeneratorCommon::dispatchEvent(EventRecord &ev)
{
    assert(eventlock->depth() > 0);
    
    char buf[128];
    signal_printf("%s[%d]:  dispatching event %s\n", FILE__, __LINE__,
                  ev.sprint_event(buf));

    // Special-case handling...
    switch (ev.type) {
    case evtUndefined:
        fprintf(stderr, "%s[%d]:  CHECK THIS, undefined event\n", FILE__, __LINE__); 
        return false;
        break;
    case evtShutDown:
        // Don't propagate to signal handlers
        stop_request = true;
        // We signal because the UI thread could be waiting for us.
        // fallthrough...
    case evtIgnore:
    case evtRequestedStop:
        signalEvent(ev);
        return true;
        break;
    case evtProcessExit:
        signal_printf("%s[%d]:  preparing to shut down signal gen for process %d\n", FILE__, __LINE__, getPid());
        stop_request = true;
#if defined(os_linux) || defined(os_vxworks)
	// Set process status to exited so we don't try to poke at it
	ev.proc->exiting_ = true;
#endif
        // Do not return - still dispatch
        break;
    default:
        // Do nothing.. handled below.
        break;
    }

    bool ret = assignOrCreateSignalHandler(ev);

    if (ret == false) return false;

    //  special case:  if this event is a process exit event, we want to shut down
    //  the SignalGenerator (there are not going to be any more events to listen for).
    //  The exit event has been passed off to a handler presumably, which will take
    //  care of user notifications, callbacks, etc.    
    
    return ret;
}

bool SignalGeneratorCommon::assignOrCreateSignalHandler(EventRecord &ev) {

    SignalHandler *sh = NULL;
    for (unsigned int i = 0; i < handlers.size(); ++i) {
        if (handlers[i]->assignEvent(ev)) {
            sh = handlers[i];
            break;
        }
    }
        
    bool ret = true;
    if (!sh) {
        int shid = handlers.size();
        char shname[64];
        sprintf(shname, "SH-%d-%d", getPid(), shid);
        signal_printf("%s[%d]:  about to create event handler %s\n", 
                      FILE__, __LINE__, shname);
        sh = newSignalHandler(shname, shid);
        sh->createThread();
        handlers.push_back(sh);
        ret = sh->assignEvent(ev);
        if (!ret)  {
            char buf[128];
            fprintf(stderr, "%s[%d]:  failed to assign event %s to handler\n", 
                    FILE__, __LINE__, ev.sprint_event(buf));
        }
    }

    return ret;
}


bool SignalGeneratorCommon::signalActiveProcess()
{         
  bool ret = true;
  assert(activationLock);
  
  if (waitingForOS_) return true;

  signal_printf("%s[%d]: ************************************************** signalActiveProcess for pid %d\n", FILE__, __LINE__, proc->getPid());
  activationLock->_Lock(FILE__, __LINE__);

  activeProcessSignalled_ = true;
  signal_printf("%s[%d]: signalActiveProcess waking up SignalGenerator\n", FILE__, __LINE__);
  activationLock->_Broadcast(FILE__, __LINE__);
  
  signal_printf("%s[%d]: signalActiveProcess exit, processIsPaused %d\n",
                FILE__, __LINE__, processIsPaused());
  activationLock->_Unlock(FILE__, __LINE__);

  return ret;
}   

bool SignalGeneratorCommon::belayActiveProcess()  {
    activationLock->_Lock(FILE__, __LINE__);
    signal_printf("%s[%d]: belayActiveProcess\n", FILE__, __LINE__);
    activeProcessSignalled_ = false;
    activationLock->_Unlock(FILE__, __LINE__);
    return true;
}

SignalHandler *SignalGenerator::newSignalHandler(char *name, int id)
{
  SignalHandler *sh;
  sh  = new SignalHandler(name, id, this);
  return (SignalHandler *)sh;
}



bool SignalGeneratorCommon::signalEvent(EventRecord &ev)
{

  if (ev.type != evtError) {
    char buf[128];
    signal_printf("%s[%d]:  signalEvent(%s)\n", FILE__, __LINE__, 
                  ev.sprint_event(buf));
  }
  assert(global_mutex->depth());

  signal_printf("%s[%d]: executing callbacks\n", FILE__, __LINE__);
  getMailbox()->executeCallbacks(FILE__, __LINE__);

  if (ev.type == evtProcessStop || ev.type == evtProcessExit) {
     //fprintf(stderr, "%s[%d]:  flagging BPatch status change\n", FILE__, __LINE__);
     SignalHandler::flagBPatchStatusChange();
  }

  signal_printf("%s[%d]: signalling wait list\n", FILE__, __LINE__);
  bool ret = false;
  for (unsigned int i = 0; i <wait_list.size(); ++i) {
      if (wait_list[i]->signalIfMatch(ev)) {
          signal_printf("%s[%d]: signalled the guy at position %d\n", FILE__, __LINE__, i);
          ret = true;
      }
  }

  signal_printf("%s[%d]: signalling global wait list\n", FILE__, __LINE__);
  global_wait_list_lock->_Lock(__FILE__, __LINE__);
  for (unsigned int i = 0; i < global_wait_list->size(); ++i) {
    if ((*global_wait_list)[i]->signalIfMatch(ev)) {
          ret = true;
      }
  }
  global_wait_list_lock->_Unlock(FILE__, __LINE__);


  signal_printf("%s[%d]: acquiring activation lock in signalEvent...\n", FILE__, __LINE__);
  activationLock->_Lock(FILE__, __LINE__);
  if (waitingForActivation_) {
      signal_printf("%s[%d]: generator sleeping, waking up...\n", FILE__, __LINE__);
      activationLock->_Broadcast(FILE__, __LINE__);
  }
  signal_printf("%s[%d]: releasing activation lock in signalEvent...\n", FILE__, __LINE__);
  activationLock->_Unlock(FILE__, __LINE__);
  
#if 0
  if (!ret) 
    signal_printf("%s[%d][%s]:  signalEvent(%s): nobody waiting\n", FILE__, __LINE__, 
                  getThreadStr(getExecThreadID()), eventType2str(ev.type));
#endif
  signal_printf("%s[%d]: signalEvent returning\n", FILE__, __LINE__);
  return ret;
}

bool SignalGeneratorCommon::signalEvent(eventType t)
{
  EventRecord ev;
  ev.type = t;
  ev.proc = proc;
  return signalEvent(ev);
}

eventType SignalGeneratorCommon::waitForOneOf(pdvector<eventType> &evts, dyn_lwp *lwp /* = NULL */)
{
  assert(global_mutex->depth());
  MONITOR_ENTRY();

  if (getExecThreadID() == getThreadID()) {
    fprintf(stderr, "%s[%d][%s]:   ILLEGAL:  SYNC THREAD waiting on for a signal\n", 
            FILE__, __LINE__, getThreadStr(getExecThreadID()));
    assert(0);
  }

  signal_printf("%s[%d]: waitForOneOf... \n", FILE__, __LINE__);

  //  When to set wait_flag ??
  //    (1)  If we are running on an event handler thread
  //    (2)  If we are currently running inside a callback AND an 
  //         event handler is waiting for the completion of this callback
  SignalHandler *sh = findSHWithThreadID(getExecThreadID());
  if (sh) {
      signal_printf("%s[%d]: signal handler waiting, setting wait_flag\n", FILE__, __LINE__);
      sh->wait_flag = true;
  }
  else {
      CallbackBase *cb = NULL;
      if (NULL != (cb = getMailbox()->runningInsideCallback())) {
          signal_printf("%s[%d]: running inside callback... \n",
                        FILE__, __LINE__);
          sh = findSHWaitingForCallback(cb);
          if (sh) {
              signal_printf("%s[%d]: signal handler %s waiting on callback, setting wait_flag\n",
                            FILE__, __LINE__, getThreadStr(sh->getThreadID()));
              sh->wait_flag = true;
          }
      }
  }
  
  EventGate *eg = new EventGate(global_mutex,evts[0], NULL, lwp);
  for (unsigned int i = 1; i < evts.size(); ++i) {
    eg->addEvent(evts[i]);
  }
  wait_list.push_back(eg);

  if (global_mutex->depth() > 1)
    signal_printf("%s[%d]:  about to EventGate::wait(), lock depth %d\n", 
                  FILE__, __LINE__, global_mutex->depth());

  EventRecord result = eg->wait();
  
  bool found = false;
  for (int i = wait_list.size() -1; i >= 0; i--) {
    if (wait_list[i] == eg) {
       found = true;
       VECTOR_ERASE(wait_list,i,i);
       delete eg;
       break;
    } 
  }

  if (!found) {
     fprintf(stderr, "%s[%d]:  BAD NEWS, somehow lost a pointer to eg\n", 
             FILE__, __LINE__);
  }

  if (sh)
    sh->wait_flag = false;

  // Poink the signal generator?

  MONITOR_EXIT();
  return result.type;
}


// Global version of the one above this: only called by threads external to
// the entire signal process (e.g., UI thread) who need to wait cross-process.

// Static method.
eventType SignalGeneratorCommon::globalWaitForOneOf(pdvector<eventType> &evts)
{
  assert(global_mutex->depth());

  EventGate *eg = new EventGate(global_mutex,evts[0]);
  for (unsigned int i = 1; i < evts.size(); ++i) {
      eg->addEvent(evts[i]);
  }

  global_wait_list_lock->_Lock(FILE__, __LINE__);
  global_wait_list->push_back(eg);
  global_wait_list_lock->_Unlock(FILE__, __LINE__);

  if (global_mutex->depth() > 1)
     signal_printf("%s[%d]:  about to EventGate::wait(), lock depth %d\n", 
                   FILE__, __LINE__, global_mutex->depth());

  EventRecord result = eg->wait();
  
  global_wait_list_lock->_Lock(FILE__, __LINE__);
  bool found = false;
  for (int i = global_wait_list->size() -1; i >= 0; i--) {
    if ((*global_wait_list)[i] == eg) {
       found = true;
       VECTOR_ERASE((*global_wait_list),i,i);
       delete eg;
       break;
    } 
  }
  global_wait_list_lock->_Unlock(FILE__, __LINE__);
  
  if (!found) {
     fprintf(stderr, "%s[%d]:  BAD NEWS, somehow lost a pointer to eg\n", 
             FILE__, __LINE__);
  }

  return result.type;
}

eventType SignalGeneratorCommon::waitForEvent(eventType evt, 
					      process *p, dyn_lwp *lwp,
					      eventStatusCode_t status,
					      bool executeCallbacks /* = true */)
{
    MONITOR_ENTRY();

  if (getExecThreadID() == getThreadID()) {
    fprintf(stderr, "%s[%d][%s]:   ILLEGAL:  SYNC THREAD waiting on for a signal: %s\n", 
            FILE__, __LINE__, getThreadStr(getExecThreadID()), eventType2str(evt));
    assert(0);
  }

  signal_printf("%s[%d]:  welcome to waitForEvent(%s)\n", FILE__, __LINE__, eventType2str(evt));


  //  When to set wait_flag ??
  //    (1)  If we are running on an event handler thread
  //    (2)  If we are currently running inside a callback AND a 
  //         signal handler is waiting for the completion of this callback
  SignalHandler *sh = findSHWithThreadID(getExecThreadID());
  if (sh) {
      signal_printf("%s[%d]: signal handler waiting, setting wait_flag\n", FILE__, __LINE__);
      sh->wait_flag = true;
  }
  else {
      CallbackBase *cb = NULL;
      if (NULL != (cb = getMailbox()->runningInsideCallback())) {
          signal_printf("%s[%d]: running inside callback... \n",
                        FILE__, __LINE__);
          sh = findSHWaitingForCallback(cb);
          if (sh) {
              signal_printf("%s[%d]: signal handler %s waiting on callback, setting wait_flag\n",
                            FILE__, __LINE__, getThreadStr(sh->getThreadID()));
              sh->wait_flag = true;
          }
      }
  }

  EventGate *eg = new EventGate(global_mutex,evt,p, lwp, status);
  wait_list.push_back(eg);
  
  if (global_mutex->depth() > 1)
    signal_printf("%s[%d]:  about to EventGate::wait(%s), lock depth %d\n", 
                  FILE__, __LINE__, 
                  eventType2str(evt), global_mutex->depth());
  EventRecord result = eg->wait(executeCallbacks);
  
  bool found = false;
  for (int i = wait_list.size() -1; i >= 0; i--) {
    if (wait_list[i] == eg) {
       found = true;
       VECTOR_ERASE(wait_list,i,i);
       delete eg;
       break;
    } 
  }

  if (!found) {
      fprintf(stderr, "%s[%d]:  BAD NEWS, somehow lost a pointer to eg\n", 
              FILE__, __LINE__);
  }

  if (sh)
    sh->wait_flag = false;

  MONITOR_EXIT();
  return result.type;
}

SignalHandler *SignalGeneratorCommon::findSHWithThreadID(unsigned long tid)
{
  for (unsigned int i = 0; i < handlers.size(); ++i) {
    if (handlers[i]->getThreadID() == tid)
      return handlers[i];
  }
  return NULL;
}

SignalHandler *SignalGeneratorCommon::findSHWaitingForCallback(CallbackBase *cb)
{
  for (unsigned int i = 0; i < handlers.size(); ++i) {
      signal_printf("%s[%d]: comparing handler wait_cb %p with given %p\n",
                    FILE__, __LINE__, handlers[i]->wait_cb, cb);
    if (handlers[i]->wait_cb == cb)
      return handlers[i];
  }
  return NULL;
}

bool SignalGeneratorCommon::decodeIfDueToProcessStartup(EventRecord &ev)
{
  bool ret = false;
  char buf[128];
  process *proc = ev.proc;
  bootstrapState_t bootstrapState = proc->getBootstrapState();

  startup_printf("%s[%d]:  decodeIfDueToProcessStartup: state: %s\n", 
                 FILE__, __LINE__, proc->getBootstrapStateAsString().c_str());

  switch(bootstrapState) {
    case bootstrapped_bs:  
        break;
    case unstarted_bs:     
    case attached_bs:
       if (proc->wasCreatedViaAttach())
          ev.type = evtProcessAttach;
        else 
          ev.type = evtProcessCreate; 
        ret = true;
        break;
    case begun_bs:
    case libcLoaded_bs:
       if (proc->getTraceSysCalls()) {
           ret = proc->decodeStartupSysCalls(ev);
       }
       else if (proc->trapAtEntryPointOfMain(ev.lwp, 
                            (Address) INFO_TO_ADDRESS(ev.info))) {
          ev.type = evtProcessInit; 
          ret = true;
       }
       else {
         fprintf(stderr, "%s[%d]:  begun_bs, but no trap!!!!!\n", FILE__, __LINE__);
       }
       break;
    case loadingRT_bs:
        if (proc->trapDueToDyninstLib(ev.lwp)) {
          ev.type = evtProcessLoadedRT;
          ret = true;
        }
#if 0
  //  windows has more info available here, not just from getActiveFrame -- we
  //  used to use it, but it may not be necessary anymore...
        
        if (proc->dyninstlib_brk_addr &&
            (proc->dyninstlib_brk_addr == (Address)ev.info.u.Exception.ExceptionRecord.ExceptionAddress)) {
            ev.type = evtProcessLoadedRT;
            ret = true;
        }

#endif
        break;
    case initialized_bs:
    case loadedRT_bs:
    default:
        break;
  };
  
  if (ret) 
     startup_printf("%s[%d]:  decodeIfDueToProcessStartup got %s, status = %s\n",
                   FILE__, __LINE__, ev.sprint_event(buf), 
                   proc->getBootstrapStateAsString().c_str());

  return ret;
}

void SignalGeneratorCommon::clearCachedLocations()  {
    sync_event_id_addr = 0;
    sync_event_arg1_addr = 0;
    sync_event_arg2_addr = 0;
    sync_event_arg3_addr = 0;
    sync_event_breakpoint_addr = 0;
}

bool SignalGeneratorCommon::decodeRTSignal(EventRecord &ev)
{
   // We've received a signal we believe was sent
   // from the runtime library. Check the RT lib's
   // status variable and return it.
   // These should be made constants
   process *proc = ev.proc;
   if (!proc) return false;

   int breakpoint;
   int status;
   Address arg1 = 0x0;
   int zero = 0;

   // First, check breakpoint...
   if (sync_event_breakpoint_addr == 0) {
       std::string status_str ("DYNINST_break_point_event");
       
       pdvector<int_variable *> vars;
       if (!proc->findVarsByAll(status_str, vars)) {
           return false;
       }
       
       if (vars.size() != 1) {
           fprintf(stderr, "%s[%d]:  ERROR, found %ld copies of var %s\n", 
                   FILE__, __LINE__, (long) vars.size(), status_str.c_str());
           return false;
       }

       sync_event_breakpoint_addr = vars[0]->getAddress();
   }

   if (!proc->readDataSpace((void *)sync_event_breakpoint_addr, 
                            sizeof(int),
                            &breakpoint, true)) {
       fprintf(stderr, "%s[%d]:  readDataSpace failed (ev.proc %d, ev.lwp %d)\n", 
               FILE__, __LINE__, ev.proc->getPid(), ev.lwp->get_lwp_id());
       return false;
   }

   switch (breakpoint) {
   case 0:
       return false;
   case 1:
#if ! defined (os_windows)
       // We SIGNUMed it
       if (ev.what != DYNINST_BREAKPOINT_SIGNUM) {
           // False alarm...
           return false;
       }
       break;
   case 2:
       if (ev.what != SIGSTOP) {
           // Again, false alarm...
           return false;
       }
#endif
       break;
   default:
       assert(0);
   }

   // Definitely a breakpoint... set that up and clear the flag
   ev.type = evtProcessStop;

   // Further processing may narrow this down some more.

   // Make sure we don't get this event twice....
   if (!proc->writeDataSpace((void *)sync_event_breakpoint_addr, sizeof(int), &zero)) {
       fprintf(stderr, "%s[%d]:  writeDataSpace failed\n", FILE__, __LINE__);
   }

   if (sync_event_id_addr == 0) {
       std::string status_str ("DYNINST_synch_event_id");
       
       
       pdvector<int_variable *> vars;
       if (!proc->findVarsByAll(status_str, vars)) {
           fprintf(stderr, "%s[%d]:  cannot find var %s\n", 
                   FILE__, __LINE__, status_str.c_str());
           return false;
       }
       
       if (vars.size() != 1) {
           fprintf(stderr, "%s[%d]:  ERROR:  %ld vars matching %s, not 1\n", 
                   FILE__, __LINE__, (long) vars.size(), status_str.c_str());
           return false;
       }

       sync_event_id_addr = vars[0]->getAddress();
   }

   if (!proc->readDataSpace((void *)sync_event_id_addr, sizeof(int),
                            &status, true)) {
       fprintf(stderr, "%s[%d]:  readDataSpace failed\n", FILE__, __LINE__);
       return false;
   }
   
   if (status == DSE_undefined) {
       return false; // Nothing to see here
   }

   // Make sure we don't get this event twice....
   if (!proc->writeDataSpace((void *)sync_event_id_addr, sizeof(int), &zero)) {
       fprintf(stderr, "%s[%d]:  writeDataSpace failed\n", FILE__, __LINE__);
       return false;
   }

   // get runtime library arg1 address
   if (sync_event_arg1_addr == 0) {
       std::string arg_str ("DYNINST_synch_event_arg1");
       
       pdvector<int_variable *> vars;
       if (!proc->findVarsByAll(arg_str, vars)) {
           fprintf(stderr, "%s[%d]:  cannot find var %s\n", 
                   FILE__, __LINE__, arg_str.c_str());
           return false;
       }
       
       if (vars.size() != 1) {
           fprintf(stderr, "%s[%d]:  ERROR:  %ld vars matching %s, not 1\n", 
                   FILE__, __LINE__, (long) vars.size(), arg_str.c_str());
           return false;
       }
       sync_event_arg1_addr = vars[0]->getAddress();
   }
   //read arg1
   if (!proc->readDataSpace((void *)sync_event_arg1_addr, 
                            proc->getAddressWidth(), &arg1, true)) {
       fprintf(stderr, "%s[%d]:  readDataSpace failed\n", FILE__, __LINE__);
       return false;
   }

   // Do platform dependent decoding, sets ev.type and ev.what, adds
   // arg1 to EventRecord
   return decodeRTSignal_NP(ev, arg1, status);

}


bool SignalGenerator::attachProcess()
{
  assert(proc);
  
    proc->creationMechanism_ = process::attached_cm;

    proc->stateWhenAttached_ = stopped;
    if (proc->isRunning_()) {
       //Checks the state in /proc (at least on Linux)
      proc->stateWhenAttached_ = running;
    }

#if defined (os_linux)
   waitpid_mux.registerProcess(this);
#endif
   if (!proc->attach()) 
   {
      proc->set_status(detached);      
      startup_printf("%s[%d] attach failing here: thread %s\n", 
                     FILE__, __LINE__, getThreadStr(getExecThreadID()));
      std::string msg = std::string("Warning: unable to attach to specified process:")
         + utos(getPid());
      showErrorCallback(26, msg.c_str());
      return false;
   }

    // Record what the process was doing when we attached, for possible
    // use later.
    proc->set_status(stopped);
    
    startup_printf("%s[%d]: returning from attachProcess\n", FILE__, __LINE__);
    return true;
}


std::string SignalGeneratorCommon::createExecPath(std::string &file, std::string &dir)
{
  std::string ret = file;
#if defined (os_windows)
  if (dir.length() > 0) {
      if ( (file.length() < 2)      // file is too short to be a drive specifier
         || !isalpha( file[0] )     // first character in file is not a letter
         || (file[1] != ':') )      // second character in file is not a colon
            ret = dir + "\\" + file;
  }
#else
  if (dir.length() > 0) {
     if (!(file[0] == ('/'))) {
         // file does not start  with a '/', so it is a relative pathname
         // we modify it to prepend the given directory
         if (dir[dir.length()-1 ] == ('/') ) {
             // the dir already has a trailing '/', so we can
             // just concatenate them to get an absolute path
             ret =  dir + file;
          }
          else {
             // the dir does not have a trailing '/', so we must
             // add a '/' to get the absolute path
             ret =  dir + "/" + file;
          }
      }
      else {
         // file starts with a '/', so it is an absolute pathname
         // DO NOT prepend the directory, regardless of what the
         // directory variable holds.
         // nothing to do in this case
      }

  }
#endif
  return ret;
}
SignalGenerator *SignalGeneratorCommon::newSignalGenerator(std::string file, std::string dir,
                                                         pdvector<std::string> *argv,
                                                         pdvector<std::string> *envp,
                                                         std::string inputFile,
                                                         std::string outputFile,
                                                         int stdin_fd, int stdout_fd,
                                                         int stderr_fd)
{
  char idstr[16];
  sprintf(idstr, "SYNC%d", signal_generator_counter++);
  return new SignalGenerator(idstr, 
                             file, dir, 
                             argv, envp, inputFile, outputFile,
                             stdin_fd, stdout_fd, stderr_fd);
}

SignalGenerator *SignalGeneratorCommon::newSignalGenerator(std::string file, int pid)
{
  char idstr[32];
  sprintf(idstr, "SG-%d", pid);
  return new SignalGenerator(idstr, file, pid);
}

process *SignalGeneratorCommon::newProcess(std::string file_, std::string dir, 
                                           pdvector<std::string> *argv,
                                           pdvector<std::string> *envp,
                                           int stdin_fd, int stdout_fd, 
                                           int stderr_fd,
                                           BPatch_hybridMode mode)
{
   // Verify existence of exec file
   std::string file = createExecPath(file_, dir);

   if (!OS::executableExists(file)) {
      startup_printf("%s[%d]:  failed to read file %s\n", FILE__, __LINE__, file.c_str());
      std::string msg = std::string("Can't read executable file ") + file + (": ") + strerror(errno);
      showErrorCallback(68, msg.c_str());
      return NULL;
   }

   

   // check for I/O redirection in arg list.
   std::string inputFile;
   std::string outputFile;
   // TODO -- this assumes no more than 1 of each "<", ">"
   // also, do we want this behavior in general, or should there be a switch to enable/disable?
   for (unsigned i1=0; i1<argv->size(); i1++) {
     if ((*argv)[i1] == "\\<") {
        (*argv)[i1] = "<";
     }
     else if ((*argv)[i1] == "<") {
       inputFile = (*argv)[i1+1];
       for (unsigned j=i1+2, k=i1; j<argv->size(); j++, k++)
         (*argv)[k] = (*argv)[j];
       argv->resize(argv->size()-2);
     }
   }
   for (unsigned i2=0; i2<argv->size(); i2++) {
     if ((*argv)[i2] == "\\>") {
        (*argv)[i2] = ">";
     }
     else if ((*argv)[i2] == ">") {
       outputFile = (*argv)[i2+1];
       for (unsigned j=i2+2, k=i2; j<argv->size(); j++, k++)
         (*argv)[k] = (*argv)[j];
       argv->resize(argv->size()-2);
     }
   }


  SignalGenerator *sg = newSignalGenerator(file, dir, argv, envp, inputFile, outputFile,
                                           stdin_fd, stdout_fd, stderr_fd);

  if (!sg) {
     fprintf(stderr, "%s[%d]:  failed to create event handler thread for %s\n", 
             FILE__, __LINE__, getThreadStr(getExecThreadID()));
     getMailbox()->executeCallbacks(FILE__, __LINE__);
     return NULL;
  }


  process *theProc = new process(sg,mode);
  assert(theProc);
  sg->setProcess(theProc);
  //  finally, create the signal handler thread -- this creates the process
  //  from the event handling thread.  We want to do it there because on some platforms
  //  (windows, linux-with-linuxThreads) the only thread that can properly listen for
  //  debug events is the thread that spawned the process. 

  if (!sg->createThread()) {
     delete sg;
     //delete theProc;
     getMailbox()->executeCallbacks(FILE__, __LINE__);
     startup_printf("%s[%d]: failed to create signal generator thread, returning NULL\n",
                    FILE__, __LINE__);
     return NULL;
  }

  assert(-1 != sg->getPid());

  signal_printf("%s[%d]:  started signal listener for new process %d -- %s\n",
                FILE__, __LINE__, sg->getPid(), file.c_str());

  return theProc;
}

void SignalGeneratorCommon::stopSignalGenerator(SignalGenerator *sg)
{
   
  int dur = 0;
  signal_printf("%s[%d]:  waiting for thread to terminate\n", FILE__, __LINE__);

  sg->stopThreadNextIter();
  sg->wakeUpThreadForShutDown();

  while (sg->isRunning() && (dur < 5)) {
    sg->__UNLOCK;
    //  If we wait more than 5 iters here, something is defnitely wrong and
    //  this should be reexamined.
    if (dur++ > 5) {
        fprintf(stderr, "%s[%d]:  sg still running\n", FILE__, __LINE__);
    }
    sleep(1);
    
    sg->__LOCK;
  }

  for (unsigned i = 0; i < sg->handlers.size(); i++) {
      sg->deleteSignalHandler(sg->handlers[i]);
  }

  signal_printf("%s[%d]:  sg has stopped\n", FILE__, __LINE__);
}

void SignalGeneratorCommon::deleteSignalGenerator(SignalGenerator *sg)
{
   
  if (sg->isRunning())
    stopSignalGenerator(sg);
 
   delete (sg);
}


process *SignalGeneratorCommon::newProcess
(std::string &progpath, int pid, BPatch_hybridMode mode)
{
  SignalGenerator *sg = newSignalGenerator(progpath, pid);

  if (!sg) {
     fprintf(stderr, "%s[%d]:  failed to create event handler thread for %s\n", 
             FILE__, __LINE__, getThreadStr(getExecThreadID()));
     getMailbox()->executeCallbacks(FILE__, __LINE__);
     return NULL;
  }


  process *theProc = new process(sg,mode);
  assert(theProc);
  sg->setProcess(theProc);
  //  finally, create the signal handler thread -- this creates the process
  //  from the event handling thread.  We want to do it there because on some platforms
  //  (windows, linux-with-linuxThreads) the only thread that can properly listen for
  //  debug events is the thread that spawned the process. 

  if (!sg->createThread()) {
     signal_printf("%s[%d]:  failed to create event handler thread %s\n", 
                   FILE__, __LINE__, getThreadStr(getExecThreadID()));
     delete sg;
     getMailbox()->executeCallbacks(FILE__, __LINE__);
     return NULL;
  }

  assert(-1 != sg->getPid());
  signal_printf("%s[%d]:  started signal listener for new process %d -- %s\n",
                FILE__, __LINE__, sg->getPid(), progpath.c_str());

  

  return theProc;
}

process * SignalGeneratorCommon::newProcess(process *parent, int pid_, int traceLink)
{
  char *progpath = const_cast<char *>(parent->getAOut()->fullName().c_str());
  assert(progpath);
  SignalGenerator *sg = newSignalGenerator(progpath, pid_);

  if (!sg) {
     fprintf(stderr, "%s[%d]:  failed to create event handler thread for %s\n", 
             FILE__, __LINE__, getThreadStr(getExecThreadID()));
     getMailbox()->executeCallbacks(FILE__, __LINE__);
     return NULL;
  }

  process *theChild = new process(parent, sg, traceLink);
  assert(theChild);
  sg->setProcess(theChild);
  
  if (!sg->createThread()) {
     delete sg;
     getMailbox()->executeCallbacks(FILE__, __LINE__);
     return NULL;
  }

  assert(-1 != sg->getPid());


  return theChild;
}

SignalGeneratorCommon::SignalGeneratorCommon(char *idstr) :
    EventHandler<EventRecord>(BPatch_eventLock::getLock(),
                              idstr,/*start thread?*/ false),
    file_(""),
    dir_(""),
    inputFile_(""),
    outputFile_(""),
    stdin_fd_(-1),
    stdout_fd_(-1),
    stderr_fd_(-1),
    argv_(NULL),
    envp_(NULL),
    pid_(-1),
    traceLink_(-1),
    requested_wait_until_active(false),
    waitingForActivation_(false),
    processPausedDuringOSWait_(false),
    pendingDecode_(false),
    decodingEvent_(false),
    dispatchingEvent_(false),
    waitingForOS_(false),
    continueDesired_(false),
    shuttingDown_(false),
    continueSig_(-1),
    continueWholeProcess_(false),
    continueCompleted_(false),
    numBlockedForContinue(0),
    syncRunWhenFinished_(unsetRequest),
    asyncRunWhenFinished_(unsetRequest),
    activeProcessSignalled_(false),
    independentLwpStop_(0),
    childForkStopAlreadyReceived_(false),
    sync_event_id_addr(0),
    sync_event_arg1_addr(0),
    sync_event_arg2_addr(0),
    sync_event_arg3_addr(0),
    sync_event_breakpoint_addr(0),
    usage_count(0)
{
    signal_printf("%s[%d]:  new SignalGenerator\n", FILE__, __LINE__);
    assert(eventlock == global_mutex);

    waitlock = new eventLock;
    activationLock = new eventLock;
    waitForContinueLock = new eventLock;
    waitForHandlerExitLock = new eventLock;
}

bool SignalGeneratorCommon::setupCreated(std::string file,
                                         std::string dir,
                                         pdvector <std::string> *argv,
                                         pdvector <std::string> *envp,
                                         std::string inputFile,
                                         std::string outputFile,
                                         int stdin_fd,
                                         int stdout_fd,
                                         int stderr_fd) 
{
    file_ = file;
    dir_ = dir;
    argv_ = argv;
    envp_ = envp;
    inputFile_ = inputFile;
    outputFile_ = outputFile;
    stdin_fd_ = stdin_fd;
    stdout_fd_ = stdout_fd;
    stderr_fd_ = stderr_fd;
    return true;
}

bool SignalGeneratorCommon::setupAttached(std::string file,
                                          int pid) {
    file_ = file;
    pid_ = pid;
    return true;
}

bool SignalGeneratorCommon::wakeUpThreadForShutDown()
{
    shuttingDown_ = true;
#if defined (os_windows)
//  DebugBreakProcess(this->proc->processHandle_);
  if (waiting_for_active_process) {
    __BROADCAST;
    return true;
  }
#else
  int sig_to_send = SIGTRAP;
   assert(global_mutex->depth());

  if (waitingForOS_) {
    signal_printf("%s[%d]:  sending SIGTRAP to wake up signal handler\n", FILE__, __LINE__);
    P_kill (getPid(), sig_to_send);
    waitForEvent(evtShutDown, proc);
    signal_printf("%s[%d][%s]:  got shutdown event\n", FILE__, __LINE__, getThreadStr(getExecThreadID()));
  }
  else if (waiting_for_active_process) {
    __BROADCAST;
  }
#endif
  return true;
}

extern void dyninst_yield();

SignalGeneratorCommon::~SignalGeneratorCommon() 
{
  //killThread();
    signal_printf("%s[%d]: Deleting SignalGeneratorCommon, %p\n", FILE__, __LINE__, this);
    
    // Don't delete signal handlers; they get deleted in their own threads
    
    delete waitlock;
    delete activationLock;
    delete waitForContinueLock;
    delete waitForHandlerExitLock;

    while (wait_list.size()) {
        signal_printf("%s[%d]: Waiting for %d wait list to go to 0\n", FILE__, __LINE__, wait_list.size());
        dyninst_yield();
    }
    
}

void SignalGeneratorCommon::deleteSignalHandler(SignalHandler *sh)
{
    EventRecord ev;
    ev.type = evtShutDown;
    sh->stopThreadNextIter();
    sh->assignEvent(ev);
    
    if (getExecThreadID() == getThreadID()) {
      // We're trying to delete ourselves... don't block ;)
      return;
    }

#if 0
    // Don't wait anymore... consider the following:
    // Process exits
    //   SignalGenerator picks this up.
    //   signalHandler gets assigned event
    //   signalHandler registers callback and blocks
    //   UI thread starts running
    //   UI thread deletes the process
    //   ... which gets here
    // 
    // Note that the signalHandler is running (and will be, since it is
    // waiting for the callback to complete). Don't delete it. 
    while (sh->isRunning()) {
      waitForEvent(evtAnyEvent);
    }
#endif

}

bool SignalGeneratorCommon::initialize_event_handler()
{
  assert(proc);

  //  This is the init function for the event handler.  It is called before the main
  //  event handing loop.  

  //  In the case of the signal handler, here we call either forkNewProcess() or
  //  attachProcess() if the process already exists.

  if (getPid() == -1) {
      if (!forkNewProcess()) {
          return false;
      }

      // We have a PID, override the name of this thread
      char newIdStr[32];
      sprintf(newIdStr, "SG-%d", getPid());
      setName(newIdStr);
      
      proc->createRepresentativeLWP();
      
      if (!proc->setupCreated(traceLink_)) {
          signal_printf("%s[%d]: Failed to do basic process setup\n", FILE__, __LINE__);
          fprintf(stderr,"%s[%d]: Failed to do basic process setup\n", FILE__, __LINE__);
#if !defined(os_windows)
          P_kill(getPid(), SIGKILL);
#endif
          delete proc;
          proc = NULL;
          return false;
      }

      int status;
      fileDescriptor desc;
      startup_printf("%s[%d]:  about to getExecFileDescriptor\n", FILE__, __LINE__);
      if (!getExecFileDescriptor(file_, getPid(), true, status, desc)) {
          signal_printf("%s[%d]: Failed to find exec descriptor\n", FILE__, __LINE__);
          fprintf(stderr,"%s[%d]: Failed to find exec descriptor\n", FILE__, __LINE__);
#if !defined(os_windows)
          P_kill(getPid(), SIGKILL);
#endif
          ///    cleanupBPatchHandle(theProc->sh->getPid());
          //  processVec.pop_back();
          delete proc;
          proc = NULL;
          return false;
      }

      startup_printf("%s[%d]:  about to setAOut\n", FILE__, __LINE__);
      if (!proc->setAOut(desc)) {
          fprintf(stderr, "%s[%d] - Couldn't setAOut\n", FILE__, __LINE__);
          sleep(1);
          startup_printf("%s[%d] - Couldn't setAOut\n", FILE__, __LINE__);
#if !defined(os_windows)
          P_kill(getPid(), SIGKILL);
#if defined (os_linux)
          int waitpid_status = 0;
          waitpid(getPid(), &waitpid_status, 0);
#endif
#endif
          // cleanupBPatchHandle(theProc->sh->getPid());
          // processVec.pop_back();
          delete proc;
          proc = NULL;
          return false;
      }
      startup_printf("%s[%d]:  after setAOut\n", FILE__, __LINE__);
      //HACKSTATUS = status;      
  }
  else if (!proc->getParent()){
      //  attach case (pid != -1 && proc->parent == NULL)
      proc->createRepresentativeLWP();
      
      startup_printf("%s[%d]:  about to attachProcess\n", FILE__, __LINE__);
      if (!attachProcess()) {
          delete proc;
          proc = NULL;
          return false;
      }
      
#if defined(os_windows)
    int status = (int)INVALID_HANDLE_VALUE;    // indicates we need to obtain a valid handle
#else
    int status = getPid();
#endif // defined(i386_unknown_nt4_0)

    fileDescriptor desc;
      startup_printf("%s[%d]:  about to getExecFileDesc\n", FILE__, __LINE__);
    if (!getExecFileDescriptor(file_, getPid(), false, status, desc)) 
    {
        delete proc;
        proc = NULL;
        return false;
    }

      startup_printf("%s[%d]:  about to setAOut\n", FILE__, __LINE__);
    if (!proc->setAOut(desc)) {
       delete proc;
       proc = NULL;
       return false;
    }

#if !defined(os_windows)
    if (proc->hasPassedMain()) {
       proc->setBootstrapState(initialized_bs);
    }
#endif
  }
  else { // proc->getParent() is non-NULL, fork case
      signal_printf("%s[%d]: attaching to forked child, creating representative LWP\n",
                    FILE__, __LINE__);

     proc->createRepresentativeLWP();
     // Set the status to stopped - it's stopped at the exit of fork() and we may
     // want to do things to it.
     proc->set_lwp_status(proc->getRepresentativeLWP(), stopped);

     
     if (!attachProcess()) {
         delete proc;
         proc = NULL;
         return false;
     }
     
     if (!proc->setupFork()) {
         fprintf(stderr, "%s[%d]:  failed to setupFork\n", FILE__, __LINE__);
         delete proc;
         proc = NULL;
         return false;
     }


  }

  return true;
}


////////////////////////////////////////////
// Unused functions
////////////////////////////////////////////

bool SignalGeneratorCommon::waitNextEvent(EventRecord &) {
    assert(0);
    return true;
}

bool SignalGeneratorCommon::handleEvent(EventRecord &) {
    assert(0);
    return true;
}

// The signalGenerator is waiting for waitForContinueLock to get signalled.
// We'll bounce it, then wait on requestContinueLock... 

bool SignalGeneratorCommon::continueProcessBlocking(int requestedSignal, dyn_lwp *lwp /* = NULL */) 
{
    if (exitRequested()) {
        // We're going away... so don't do anything
        return true;
    }

    // Fixes an odd case when we receive multiple fork exit events; as soon as
    // we've handled one skip the rest.
    childForkStopAlreadyReceived_ = true;
    // Ask (politely) the signal generator to continue us...

    signal_printf("%s[%d]: requestContinue entry, locking...\n", FILE__, __LINE__);
    activationLock->_Lock(FILE__, __LINE__);

    setContinueSig(requestedSignal);

    // We could be in waitpid for a particular thread (yay IndependentLwpControl...),
    // in which case we just kick _everyone_ up.

    // Set runRequest to true
    syncRunWhenFinished_ = runRequest;
    
    // We can run this from a signal handler thread, which deadlocks - the sig handler
    // is waiting for the generator to run the process, and the generator thinks the handler
    // is active. One specific case of this is exec; we call proc->finishExec from the handler
    // and that calls loadDyninstLib. 
    // So if we're in a handler, then set the wait_flag.
    int waitingHandler = -1; // 0 or positive if we match...
    for (unsigned i = 0; i < handlers.size(); i++) {
        if (handlers[i]->getThreadID() == getExecThreadID()) {
            signal_printf("%s[%d]: continueProcessBlocking called on handler, setting wait_flag\n",
                          FILE__, __LINE__);
            waitingHandler = i;
            assert(handlers[i]->wait_flag == false);
            handlers[i]->wait_flag = true;
            break;
        }
    }


    if (lwp) {
        signal_printf("%s[%d]: adding lwp %d to continue list...\n",
                      FILE__, __LINE__, lwp->get_lwp_id());
        lwpsToContinue_.push_back(lwp);
    }
    else {
        // Someone wants us to run the whole nine yards
        continueWholeProcess_ = true;
    }


    if (waitingForOS_ && !independentLwpStop_) {
        // Make sure that all active signal handlers kick off...
        while (isActivelyProcessing()) {
            signal_printf("%s[%d]: continueProcessBlocking waiting for signal handlers\n",
                          FILE__, __LINE__);
            activationLock->_Unlock(FILE__, __LINE__);
            waitForEvent(evtAnyEvent);
            activationLock->_Lock(FILE__, __LINE__);
        }
            
        signal_printf("%s[%d]: Blocking continue and already in waitpid; overriding and continuing\n",
                      FILE__, __LINE__);
        // Just continue ze sucker
        continueProcessInternal();
        assert(activationLock->depth() == 1);
        activationLock->_Unlock(FILE__, __LINE__);

        if (waitingHandler != -1) {
            signal_printf("%s[%d]: continueProcessBlocking on handler returning, resetting wait_flag\n",
                          FILE__, __LINE__);
            handlers[waitingHandler]->wait_flag = false;
        }
        return true;
    }

    signal_printf("%s[%d]: grabbed requestContinueLock...\n", FILE__, __LINE__);

    int lock_depth = eventlock->depth();
    assert(lock_depth > 0);
    
    // We can't iterate over eventlock->depth(); we unlock, someone else locks...
    // TODO: make this an eventLock method: "completely release"
    for (int i = 0; i < lock_depth; i++) {
        eventlock->_Unlock(FILE__, __LINE__);
    }

    signal_printf("%s[%d]: continueProcessBlocking: gave up global mutex\n", FILE__, __LINE__);

    signal_printf("%s[%d]: continueProcessBlocking, signalling SG\n", FILE__, __LINE__);
    activationLock->_Broadcast(FILE__, __LINE__);

    signal_printf("%s[%d]: continueProcessBlocking, locking waitForContinue\n", FILE__, __LINE__);
    waitForContinueLock->_Lock(FILE__, __LINE__);
    signal_printf("%s[%d]: continueProcessBlocking, unlocking activationLock\n", FILE__, __LINE__);

    assert(activationLock->depth() == 1);
    activationLock->_Unlock(FILE__, __LINE__);

    numBlockedForContinue++;
    do {
        signal_printf("%s[%d]: continueProcessBlocking, waiting...\n", FILE__, __LINE__);
        //getMailbox()->executeCallbacks(FILE__, __LINE__);
        waitForContinueLock->_WaitForSignal(FILE__, __LINE__);
    } while (!continueCompleted_);
    
    numBlockedForContinue--;
    if (!numBlockedForContinue) {
        //Everyone's excepted the continue.  Reset the continueCompleted flag
        continueCompleted_ = false;
    }
    
    signal_printf("%s[%d]: continueProcessBlocking, woken up and releasing waitForContinue lock.\n", FILE__, __LINE__);

    assert(waitForContinueLock->depth() == 1);
    waitForContinueLock->_Unlock(FILE__, __LINE__);

    signal_printf("%s[%d]: continueProcessBlocking, process continued, grabbing %d global mutexes\n",
                  FILE__, __LINE__, lock_depth);

    while (lock_depth > 0) {
        eventlock->_Lock(FILE__, __LINE__);
        lock_depth--;
    }

    signal_printf("%s[%d]: continueProcessBlocking, returning\n",
                  FILE__, __LINE__);

    if (waitingHandler != -1) {
        signal_printf("%s[%d]: continueProcessBlocking on handler returning, resetting wait_flag\n",
                      FILE__, __LINE__);
        handlers[waitingHandler]->wait_flag = false;
    }
    
    return true;
}

bool SignalGeneratorCommon::continueRequired() 
{
    // Do we need to continue the process or just check waitpid/poll?

  if (independentLwpStop_) {
    // We're trying to stop an LWP, impolite to continue...
      signal_printf("%s[%d]: independent LWP stop on, not continuing...\n",
                    FILE__, __LINE__);
    return false;
  }

  for (unsigned i = 0; i < handlers.size(); i++) {
      if (handlers[i]->processing()) {
          signal_printf("%s[%d]: continueRequired: handler %s active, returning false\n",
                        FILE__, __LINE__, getThreadStr(handlers[i]->getThreadID()));
          // Active handler == no run-y.
          return false;
      }
  }

    // sync run gets priority...
  if (syncRunWhenFinished_ == stopRequest) {
      signal_printf("%s[%d]: syncRunWhenFinished = stop, not continuing...\n",
                    FILE__, __LINE__);
      return false;
  }

    if (syncRunWhenFinished_ == runRequest) {
        if (asyncRunWhenFinished_ == stopRequest) {
            fprintf(stderr, "Odd case: BPatch requests run, internals request stop\n");
            return false;
        }
        signal_printf("%s[%d]: syncRunWhenFinished = run, continuing...\n",
                    FILE__, __LINE__);
        return true;
    }

    // We're unset or purposefully ignoring. 

    if (asyncRunWhenFinished_ == runRequest) {
      signal_printf("%s[%d]: asyncRunWhenFinished = run, continuing...\n",
                    FILE__, __LINE__);
        return true;
    }
    else if (asyncRunWhenFinished_ == stopRequest) {
        return false;
    }
    assert(asyncRunWhenFinished_ == unsetRequest);

    // Both are unset... this might be an assert case.
    return false;
}


bool SignalGeneratorCommon::continueProcessInternal() {
    signal_printf("%s[%d]: continuing process...\n", FILE__, __LINE__);

    bool res = true;

    // We can get the following sequence of events:
    // UI: grab activationLock
    // UI: release eventLock
    // SG: grab eventLock
    // SG: continue process
    // SG: signal waitForContinueLock
    // SG: release eventLock
    // UI: wait for waitForContinueLock
    // ... so signal happens before wait. To prevent this,
    // we acquire the activationLock here. 

    activationLock->_Lock(FILE__, __LINE__);

    if ((lwpsToContinue_.size() != 0) &&
        process::IndependentLwpControl() &&
        !continueWholeProcess_) {
        for (unsigned i = 0; i < lwpsToContinue_.size(); i++) {
            signal_printf("%s[%d]: Continuing lwp %d\n", FILE__, __LINE__, lwpsToContinue_[i]->get_lwp_id());
            if (!lwpsToContinue_[i]->continueLWP(continueSig_))
                res = false;
        }
    }
    else  {
        signal_printf("%s[%d]: Process continue: %d lwps, %d independent, %d continueWholeProcess\n", FILE__, __LINE__,
                      lwpsToContinue_.size(), process::IndependentLwpControl(), continueWholeProcess_);
        res = proc->continueProc_(continueSig_);
        if (res && proc->status() != exited) {
            proc->set_status(running);
        }
    }

    lwpsToContinue_.clear();
    continueWholeProcess_ = false;
    
    continueSig_ = -1;

    signal_printf("%s[%d]: setting global process state to running\n", FILE__, __LINE__);
        
    // Now wake up everyone who was waiting for me...
    signal_printf("%s[%d]: waking up everyone who was waiting for continue, locking...\n",
                  FILE__, __LINE__);
    waitForContinueLock->_Lock(FILE__, __LINE__);
    activationLock->_Unlock(FILE__, __LINE__);
    signal_printf("%s[%d]: waking up everyone who was waiting for continue, broadcasting...\n",
                  FILE__, __LINE__);
    continueCompleted_ = true;
    waitForContinueLock->_Broadcast(FILE__, __LINE__);
    signal_printf("%s[%d]: waking up everyone who was waiting for continue, unlocking\n",
                  FILE__, __LINE__);
    waitForContinueLock->_Unlock(FILE__, __LINE__);

    if (!res)  {
        fprintf(stderr, "%s[%d]:  continueProc_ failed\n", FILE__, __LINE__);
        showErrorCallback(38, "System error: can't continue process");
        return false;
    }


    return true;
}


void SignalGeneratorCommon::setContinueSig(int signalToContinueWith) 
{
    if ((continueSig_ != -1) &&
        (signalToContinueWith != continueSig_)) {
       signal_printf("%s[%d]: WARNING: conflict in signal to continue with: previous %d, new %d\n",
                FILE__, __LINE__, continueSig_,  signalToContinueWith);
    }

    continueSig_ = signalToContinueWith;
}

bool SignalGeneratorCommon::pauseProcessBlocking() 
{
    if (exitRequested()) {
        // We're going away... so don't do anything
        return true;
    }

    signal_printf("%s[%d]: pauseProcessBlocking...\n", FILE__, __LINE__);
    syncRunWhenFinished_ = stopRequest;

    return proc->pause();
}

processRunState_t SignalGeneratorCommon::overrideSyncContinueState(processRunState_t newState) {
    signal_printf("%s[%d]: Overriding sync continue state, old %s, new %s\n",
                  FILE__, __LINE__, processRunStateStr(syncRunWhenFinished_),
                  processRunStateStr(newState));

    processRunState_t current = syncRunWhenFinished_;
    syncRunWhenFinished_ = newState;
    return current;
}

processRunState_t SignalGeneratorCommon::overrideAsyncContinueState(processRunState_t newState) {
    signal_printf("%s[%d]: Overriding async continue state, old %s, new %s\n",
                  FILE__, __LINE__, processRunStateStr(syncRunWhenFinished_),
                  processRunStateStr(newState));

    processRunState_t current = asyncRunWhenFinished_;
    asyncRunWhenFinished_ = newState;
    return current;
}

SignalGenerator::SignalGenerator(char *idstr, std::string file, std::string dir,
                                 pdvector<std::string> *argv,
                                 pdvector<std::string> *envp,
                                 std::string inputFile,
                                 std::string outputFile,
                                 int stdin_fd, int stdout_fd,
                                 int stderr_fd) :
   SignalGeneratorCommon(idstr),
   waiting_for_stop(false)
{
    setupCreated(file, dir, 
                 argv, envp, 
                 inputFile, outputFile,
                 stdin_fd, stdout_fd, stderr_fd);
}

void SignalGeneratorCommon::markProcessStop() { 
    independentLwpStop_++; 
    signal_printf("%s[%d]: markProcessStop => %d\n", FILE__, __LINE__, independentLwpStop_);
}

void SignalGeneratorCommon::unmarkProcessStop() { 
    independentLwpStop_--; 
    assert(independentLwpStop_ >= 0); 
    signal_printf("%s[%d]: unmarkProcessStop => %d\n", FILE__, __LINE__, independentLwpStop_);
}

void SignalGeneratorCommon::pingIfContinueBlocked() {
   waitForContinueLock->_Lock(FILE__, __LINE__);
   if (numBlockedForContinue) {
      waitForContinueLock->_Broadcast(FILE__, __LINE__);     
   }
   waitForContinueLock->_Unlock(FILE__, __LINE__);
}

void SignalGeneratorCommon::MONITOR_ENTRY() {
    // Should we lock?

    usage_count++;
}

void SignalGeneratorCommon::MONITOR_EXIT() {
    assert(usage_count > 0);
    
    usage_count--;

    if (usage_count == 0) {
        signal_printf("%s[%d]: Last user of signalGenerator exiting, cleaning up...\n",
                      FILE__, __LINE__);
        
        delete this;
    }
}

bool SignalGeneratorCommon::checkActiveProcess()
{
   if (!activeProcessSignalled_)
      return true;
#if !defined(os_linux)
   // independent lwp control... going into waitpid too often is okay on linux,
   // but really a bad idea on AIX and Solaris (as you'll get the last thing again, and
   // again, and again, and again...)
   activeProcessSignalled_ = false;
   return true;
#endif

   bool found_running_sh = false;
   for (unsigned i=0; i<handlers.size(); i++) {
      SignalHandler *sh = handlers[i];
      if (!sh->idle()) {
         signal_printf("[%s]%u: checkActiveProcess - %s is not idle\n",
                       FILE__, __LINE__, sh->getName());
         found_running_sh = true;
         break;
      }
      else {
         signal_printf("[%s]%u: checkActiveProcess - %s is idle\n", 
                       FILE__, __LINE__, sh->getName());
      }
   }
   if (!found_running_sh) {
      signal_printf("[%s]%u: All SH are idle, belaying\n", FILE__, __LINE__);
      belayActiveProcess();
      return true;
   }
   
   if (proc->threads.size() == 1) {
      signal_printf("[%s]%u: Only one thread in mutatee, belaying.\n",
                    FILE__, __LINE__);
      belayActiveProcess();
      return true;
   }
   /*   
   bool all_stopped = true;
   for (unsigned i=0; i<proc->threads.size(); i++) {
      if (proc->threads[i]->get_lwp()->status() != stopped) {
         all_stopped = false;
         break;
      }
   }
   if (all_stopped) {
      signal_printf("[%s]%u: All threads are stopped, belaying.\n",
                    FILE__, __LINE__);
      belayActiveProcess();
      return true;
   }
*/
   return true;
}
