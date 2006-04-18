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
+ * incur to third parties resulting from your use of Paradyn.
 */

// $Id: signalhandler.C,v 

#include "process.h"
#include "dyn_lwp.h"
#include "dyn_thread.h"
#include "callbacks.h"
#include "function.h"
#include "stats.h"
#include "signalhandler.h"
#include "signalgenerator.h"
#include "Object.h"
#include "mapped_object.h"
#include "eventgate.h"
//  signal_generator_counter is used to generate identifier strings
//  for signal generator threads.  eg SYNC1, SYNC2, SYNC3

unsigned signal_generator_counter = 0;
eventLock SignalGeneratorCommon::global_wait_list_lock;
pdvector<EventGate *> SignalGeneratorCommon::global_wait_list;

static char *processRunStateStr(processRunState_t runState) {
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

    addToThreadMap();
    
    startupLock->_Lock(FILE__, __LINE__);
    thread_printf("%s[%d]:  about to do init for %s\n", FILE__, __LINE__, idstr);
    if (!initialize_event_handler()) {
        thread_printf("%s[%d]: initialize event handler failed, %s returning\n", FILE__, __LINE__, idstr);
        _isRunning = false;
        init_ok = false; 

        removeFromThreadMap();

        startupLock->_Broadcast(FILE__, __LINE__);
        startupLock->_Unlock(FILE__, __LINE__);
        return;
    }
    
    init_ok = true;;
    thread_printf("%s[%d]:  init success for %s\n", FILE__, __LINE__, idstr);
    
    _isRunning = true;
    startupLock->_Broadcast(FILE__, __LINE__);
    startupLock->_Unlock(FILE__, __LINE__);
    
    thread_printf("%s[%d]:  before main loop for %s\n", __FILE__, __LINE__, idstr);

    eventlock->_Lock(FILE__, __LINE__);
    pdvector<EventRecord> events;
    while (1) {
        // TOP
        signal_printf("%s[%d]: signal generator at top of loop\n", FILE__, __LINE__);
        assert(haveLock());
        
        if (exitRequested()) {
            signal_printf("%s[%d]: exit request (loop top)\n", FILE__, __LINE__);
            break;
        }

        // If there is an event to handle, then keep going. 
        if (processIsPaused() || requested_wait_until_active) {
            signal_printf("%s[%d]: process is paused, waiting (loop top)\n", FILE__, __LINE__);
            // waitForActive... used to unlock/relock the global mutex. 
            waitForActivation();
            
            /* Continuing a process we no longer care about can trigger a
               spinloop around waitpid() (if we're detached, waitpid()
               doesn't block, but just returns ECHILD). */
	        if( exitRequested() ) {
    			signal_printf("%s[%d]: exit request (post-waitForActivation)\n", FILE__, __LINE__);
	    		break;
	    	    }            
            
            // waitForActiveProcess will return when everyone agrees that the process should
            // run. This means:
            // 1) All signal handlers are either done or waiting in a callback;
            // 2) If there was a pause request from BPatch, then there has been a run request.
            if (continueRequired()) 
                continueProcessInternal();
            // Post cleanup
            // Cleanup...

            asyncRunWhenFinished_ = unsetRequest;
            // Commented out; set by BPatch layer
            //syncRunWhenFinished_ = unsetRequest;
            activeProcessSignalled_ = false;
        }

        postSignalHandler();

        // Process not paused
        signal_printf("%s[%d]: Grabbing event\n", FILE__, __LINE__);
        getEvents(events);

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
}

bool SignalGeneratorCommon::exitRequested() {
    // Commented out for reference: we handle this at
    // dispatch time. 
    //    if (ev.type == evtShutdown) return true;

    assert(proc);
    if (proc->status() == deleted)
        return true;
    if (proc->status() == exited)
        return true;
    if (proc->status() == detached)
    	return true;
    if (stop_request)
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
        if (activeHandler) {
            signal_printf("%s[%d]: active handler, continuing around;\n", 
                          FILE__, __LINE__);
        }
        else if (activeProcessSignalled_) {
            signal_printf("%s[%d]: active process signalled, breaking out of wait loop;\n", 
                          FILE__, __LINE__);
            break;
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

bool SignalGeneratorCommon::continueProcessAsync(int signalToContinueWith) {
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
    
    if (!waitingForActivation_) {
        // We got here before the SG...
        signal_printf("%s[%d]: Raced with SG %s, setting asyncRun anyway...\n",
                      FILE__, __LINE__, getThreadStr(getThreadID()));
    }

    if (waitingForOS_) {
        // We managed to get into waitpid (prolly signalActiveProcess) without
        // continuing the process...
        continueProcessInternal();
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
#if 0
    
    //  If we have events left over from the last call of this fn,
    //  just return one.
    if (events_to_handle.size()) {
        //  if per-call ordering is important this should grab events from the front
        //  of events_to_handle.  Guessing that (if possible) multiple events generated
        //  "simultaneously" can be handled in any order, however.
        ev = events_to_handle[events_to_handle.size() - 1];
        events_to_handle.pop_back();
        char buf[128];
        signal_printf("%s[%d][%s]:  waitNextEvent: had existing event %s\n", FILE__, __LINE__,
                      getThreadStr(getExecThreadID()), ev.sprint_event(buf));
        return true;
    }
#endif
    
    assert(proc);
    
    bool ret = waitForEventsInternal(events);

    if (ret == false) return false;

    // Will be set to false when we've assigned the event to a signal handler
    
    return true;
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

  signal_printf("%s[%d]: signalActiveProcess\n", FILE__, __LINE__);
  activationLock->_Lock(FILE__, __LINE__);

  activeProcessSignalled_ = true;
  signal_printf("%s[%d]: signalActiveProcess waking up SignalGenerator\n", FILE__, __LINE__);
  activationLock->_Broadcast(FILE__, __LINE__);
  
  signal_printf("%s[%d]: signalActiveProcess exit, processIsPaused %d\n",
                FILE__, __LINE__, processIsPaused());
  activationLock->_Unlock(FILE__, __LINE__);

  return ret;
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
    signal_printf("%s[%d][%s]:  signalEvent(%s)\n", FILE__, __LINE__, 
                  getThreadStr(getExecThreadID()), ev.sprint_event(buf));
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
          ret = true;
      }
  }

  signal_printf("%s[%d]: signalling global wait list\n", FILE__, __LINE__);
  global_wait_list_lock._Lock(__FILE__, __LINE__);
  for (unsigned int i = 0; i < global_wait_list.size(); ++i) {
      if (global_wait_list[i]->signalIfMatch(ev)) {
          ret = true;
      }
  }
  global_wait_list_lock._Unlock(FILE__, __LINE__);


  
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
  return signalEvent(ev);
}

#if 0
bool SignalGeneratorCommon::unsafeToContinue() {
    // If there are any pending events, then don't continue. Otherwise,
    // obey the continuation hint.

    signal_printf("%s[%d]: checking whether safe to continue...\n",
                  FILE__, __LINE__);

    continueDesired_ = true;

    if (events_to_handle.size()) {
        signal_printf("%s[%d]: %d pending signal generator events, unsafe to continue\n",
                      FILE__, __LINE__, events_to_handle.size());
        return true;
    }    

    for (unsigned i =0; i < handlers.size(); i++) {
        if (handlers[i]->events_to_handle.size()) {
            signal_printf("%s[%d]: handler %s has %d pending signal generator events, unsafe to continue\n",
                          FILE__, __LINE__, getThreadStr(handlers[i]->getThreadID()), handlers[i]->events_to_handle.size());
            return true;
        }
    }
    
    // Reset, we're okay
    continueDesired_ = false;
    return false;
}
#endif

eventType SignalGeneratorCommon::waitForOneOf(pdvector<eventType> &evts)
{
  assert(global_mutex->depth());

  if (getExecThreadID() == getThreadID()) {
    fprintf(stderr, "%s[%d][%s]:   ILLEGAL:  SYNC THREAD waiting on for a signal\n", 
            FILE__, __LINE__, getThreadStr(getExecThreadID()));
    abort();
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
  
  EventGate *eg = new EventGate(global_mutex,evts[0]);
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
       wait_list.erase(i,i);
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

  global_wait_list_lock._Lock(FILE__, __LINE__);
  global_wait_list.push_back(eg);
  global_wait_list_lock._Unlock(FILE__, __LINE__);

  if (global_mutex->depth() > 1)
     signal_printf("%s[%d]:  about to EventGate::wait(), lock depth %d\n", 
                   FILE__, __LINE__, global_mutex->depth());

  EventRecord result = eg->wait();
  
  global_wait_list_lock._Lock(FILE__, __LINE__);
  bool found = false;
  for (int i = global_wait_list.size() -1; i >= 0; i--) {
    if (global_wait_list[i] == eg) {
       found = true;
       global_wait_list.erase(i,i);
       delete eg;
       break;
    } 
  }
  global_wait_list_lock._Unlock(FILE__, __LINE__);
  
  if (!found) {
     fprintf(stderr, "%s[%d]:  BAD NEWS, somehow lost a pointer to eg\n", 
             FILE__, __LINE__);
  }

  return result.type;
}

eventType SignalGeneratorCommon::waitForEvent(eventType evt, process *p, dyn_lwp *lwp,
                                        eventStatusCode_t status)
{
  if (getExecThreadID() == getThreadID()) {
    fprintf(stderr, "%s[%d][%s]:   ILLEGAL:  SYNC THREAD waiting on for a signal: %s\n", 
            FILE__, __LINE__, getThreadStr(getExecThreadID()), eventType2str(evt));
    abort();
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
  EventRecord result = eg->wait();
  
  bool found = false;
  for (int i = wait_list.size() -1; i >= 0; i--) {
    if (wait_list[i] == eg) {
       found = true;
       wait_list.erase(i,i);
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


#if 0
bool SignalGeneratorCommon::activeHandlerForProcess(process *p)
{
    // If the handler is active and running on a different thread from
    // us (as we can get the following:
    //    Handler calls into BPatch
    //    BPatch tries continue
    //    .... continue blocked because of active handler

    if (decodingEvent) {
        fprintf(stderr, "UITHREAD - sync decoding event, backing off\n");
        return true;
    }
    for (unsigned int i = 0; i < handlers.size(); ++i) {
        if (handlers[i]->isActive(p) &&
            !(handlers[i]->getThreadID() == getExecThreadID())) {
            fprintf(stderr, "UITHREAD: active handler, backing off\n");
            return true;
        }
    }
    return false;
}
#endif

bool SignalGeneratorCommon::decodeIfDueToProcessStartup(EventRecord &ev)
{
  bool ret = false;
  char buf[128];
  process *proc = ev.proc;
  bootstrapState_t bootstrapState = proc->getBootstrapState();

  //fprintf(stderr, "%s[%d]:  decodeIfDueToProcessStartup: state: %s\n", FILE__, __LINE__, proc->getBootstrapStateAsString().c_str());
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
#if defined (os_windows)
       if (proc->trapAtEntryPointOfMain(NULL, (Address)ev.info.u.Exception.ExceptionRecord.ExceptionAddress)) {
          ev.type = evtProcessInit; 
          ret = true;
       }
#else
       if (proc->trapAtEntryPointOfMain(ev.lwp)) {
          ev.type = evtProcessInit; 
          ret = true;
       }
#endif
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
     signal_printf("%s[%d]:  decodeIfDueToProcessStartup got %s, status = %s\n",
                   FILE__, __LINE__, ev.sprint_event(buf), 
                   proc->getBootstrapStateAsString().c_str());

  return ret;
}

bool SignalGenerator::attachProcess()
{
  assert(proc);

    proc->creationMechanism_ = process::attached_cm;
    // We're post-main... run the bootstrapState forward

#if !defined(os_windows)
    proc->bootstrapState = initialized_bs;
#else
    // We need to wait for the CREATE_PROCESS debug event.
    // Set to "begun" here, and fix up in the signal loop
    proc->bootstrapState = attached_bs;
#endif

  if (!proc->attach()) {
     proc->set_status( detached);

     startup_printf("%s[%d] attach failing here: thread %s\n", 
                    FILE__, __LINE__, getThreadStr(getExecThreadID()));
     pdstring msg = pdstring("Warning: unable to attach to specified process: ")
                  + pdstring(getPid());
     showErrorCallback(26, msg.c_str());
     return false;
  }

  startup_printf("%s[%d]: attached, getting current process state\n", FILE__, __LINE__);

   // Record what the process was doing when we attached, for possible
   // use later.
#if !defined(os_windows)
   if (proc->isRunning_()) {
       startup_printf("[%d]: process running when attached, pausing...\n", getPid());
       proc->stateWhenAttached_ = running;
       proc->set_status(running);


       //  Now pause the process -- since we are running on the signal handling thread
       //  we cannot use the "normal" pause, which sends a signal and then waits
       //  for the signal handler to receive the trap.
       //  Need to do it all inline.
       if (!proc->stop_(false)) {
          fprintf(stderr, "%s[%d]:  failed to stop process\n", FILE__, __LINE__);
          return false;
       }

       if (!waitForStopInline()) {
         fprintf(stderr, "%s[%d]:  failed to do initial stop of process\n", FILE__, __LINE__);
         return false;
       }
       proc->set_status(stopped);
   } else
#endif
   {
       startup_printf("%s[%d]: attached to previously paused process: %d\n", FILE__, __LINE__, getPid());
       proc->stateWhenAttached_ = stopped;
       //proc->set_status(stopped);
   }

  return true;
}

pdstring SignalGeneratorCommon::createExecPath(pdstring &file, pdstring &dir)
{
  pdstring ret = file;
#if defined (os_windows)
  if (dir.length() > 0) {
      if ( (file.length() < 2)      // file is too short to be a drive specifier
         || !isalpha( file[0] )     // first character in file is not a letter
         || (file[1] != ':') )      // second character in file is not a colon
            ret = dir + "\\" + file;
  }
#else
  if (dir.length() > 0) {
     if (!file.prefixed_by("/") ) {
         // file does not start  with a '/', so it is a relative pathname
         // we modify it to prepend the given directory
         if (dir.suffixed_by("/") ) {
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
SignalGenerator *SignalGeneratorCommon::newSignalGenerator(pdstring file, pdstring dir,
                                                         pdvector<pdstring> *argv,
                                                         pdvector<pdstring> *envp,
                                                         pdstring inputFile,
                                                         pdstring outputFile,
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

SignalGenerator *SignalGeneratorCommon::newSignalGenerator(pdstring file, int pid)
{
  char idstr[16];
  sprintf(idstr, "SYNC%d", signal_generator_counter++);
  return new SignalGenerator(idstr, file, pid);
}

process *SignalGeneratorCommon::newProcess(pdstring file_, pdstring dir, 
                                                     pdvector<pdstring> *argv,
                                                     pdvector<pdstring> *envp,
                                                     int stdin_fd, int stdout_fd, 
                                                     int stderr_fd)
{
   // Verify existence of exec file
   pdstring file = createExecPath(file_, dir);

   if (!OS::executableExists(file)) {
      startup_printf("%s[%d]:  failed to read file %s\n", FILE__, __LINE__, file.c_str());
      pdstring msg = pdstring("Can't read executable file ") + file + (": ") + strerror(errno);
      showErrorCallback(68, msg.c_str());
      return NULL;
   }

   // check for I/O redirection in arg list.
   pdstring inputFile;
   pdstring outputFile;
#if !defined(BPATCH_LIBRARY) || defined(BPATCH_REDIRECT_IO)
   // TODO -- this assumes no more than 1 of each "<", ">"
   for (unsigned i1=0; i1<argv->size(); i1++) {
     if ((*argv)[i1] == "<") {
       inputFile = (*argv)[i1+1];
       for (unsigned j=i1+2, k=i1; j<argv->size(); j++, k++)
         (*argv)[k] = (*argv)[j];
       argv->resize(argv->size()-2);
     }
   }
   for (unsigned i2=0; i2<argv->size(); i2++) {
     if ((*argv)[i2] == ">") {
       outputFile = (*argv)[i2+1];
       for (unsigned j=i2+2, k=i2; j<argv->size(); j++, k++)
         (*argv)[k] = (*argv)[j];
       argv->resize(argv->size()-2);
     }
   }
#endif


  SignalGenerator *sg = newSignalGenerator(file, dir, argv, envp, inputFile, outputFile,
                                           stdin_fd, stdout_fd, stderr_fd);

  if (!sg) {
     fprintf(stderr, "%s[%d]:  failed to create event handler thread for %s\n", 
             FILE__, __LINE__, getThreadStr(getExecThreadID()));
     getMailbox()->executeCallbacks(FILE__, __LINE__);
     return NULL;
  }


  process *theProc = new process(sg);
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


process *SignalGeneratorCommon::newProcess(pdstring &progpath, int pid)
{
  SignalGenerator *sg = newSignalGenerator(progpath, pid);

  if (!sg) {
     fprintf(stderr, "%s[%d]:  failed to create event handler thread for %s\n", 
             FILE__, __LINE__, getThreadStr(getExecThreadID()));
     getMailbox()->executeCallbacks(FILE__, __LINE__);
     return NULL;
  }


  process *theProc = new process(sg);
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
     fprintf(stderr, "%s[%d]:  failed to create event handler thread for %s\n", 
             FILE__, __LINE__, getThreadStr(getExecThreadID()));
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
    continueSig_(-1),
    syncRunWhenFinished_(unsetRequest),
    asyncRunWhenFinished_(unsetRequest),
    activeProcessSignalled_(false),
    independentLwpStop_(false),
    childForkStopAlreadyReceived_(false)
{
    signal_printf("%s[%d]:  new SignalGenerator\n", FILE__, __LINE__);
    assert(eventlock == global_mutex);

    waitlock = new eventLock;
    activationLock = new eventLock;
    waitForContinueLock = new eventLock;
}

bool SignalGeneratorCommon::setupCreated(pdstring file,
                                         pdstring dir,
                                         pdvector <pdstring> *argv,
                                         pdvector <pdstring> *envp,
                                         pdstring inputFile,
                                         pdstring outputFile,
                                         int stdin_fd,
                                         int stdout_fd,
                                         int stderr_fd) {
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

bool SignalGeneratorCommon::setupAttached(pdstring file,
                                          int pid) {
    file_ = file;
    pid_ = pid;
    return true;
}

bool SignalGeneratorCommon::wakeUpThreadForShutDown()
{
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

SignalGeneratorCommon::~SignalGeneratorCommon() 
{
  //killThread();

  for (unsigned int i = 0; i < handlers.size(); ++i) {
    signal_printf("%s[%d]:  destroying handler %s\n", FILE__, __LINE__, 
                  handlers[i]->getName());
    delete handlers[i];
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
       fprintf(stderr, "%s[%d]:  failed to fork a new process for %s\n", FILE__, __LINE__,
               file_.c_str());
       return false;
    }

    proc->createRepresentativeLWP();

    if (!proc->setupCreated(traceLink_)) {
        delete proc;
        proc = NULL;
        return false;
    }

    int status;
    fileDescriptor desc;
    if (!getExecFileDescriptor(file_, getPid(), true, status, desc)) {
        startup_cerr << "Failed to find exec descriptor" << endl;
    ///    cleanupBPatchHandle(theProc->sh->getPid());
      //  processVec.pop_back();
        delete proc;
        proc = NULL;
        return false;
    }
    //HACKSTATUS = status;

    if (!proc->setAOut(desc)) {
        startup_printf("[%s:%u] - Couldn't setAOut\n", FILE__, __LINE__);
       // cleanupBPatchHandle(theProc->sh->getPid());
       // processVec.pop_back();
        delete proc;
        proc = NULL;
        return false;
    }

    
  }
  else if (!proc->getParent()){
    //  attach case (pid != -1 && proc->parent == NULL)
    proc->createRepresentativeLWP();

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
    if (!getExecFileDescriptor(file_, getPid(), false, status, desc)) 
    {
        delete proc;
        proc = NULL;
        return false;
    }

    if (!proc->setAOut(desc)) {
       delete proc;
       proc = NULL;
       return false;
    }

  }
  else { // proc->getParent() is non-NULL, fork case
      signal_printf("%s[%d]: attaching to forked child, creating representative LWP\n",
                    FILE__, __LINE__);

     proc->createRepresentativeLWP();
     // Set the status to stopped - it's stopped at the exit of fork() and we may
     // want to do things to it.
     proc->set_lwp_status(proc->getRepresentativeLWP(), stopped);

     
     if (!attachProcess()) {
         fprintf(stderr, "%s[%d]:  failed to attach to process %d\n", FILE__, __LINE__,
                 getPid());
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

#if 0
bool SignalGeneratorCommon::isBlockedInOS() const {
    waitlock->_Lock(FILE__, __LINE__);
    bool ret = waitingForOS_;
    waitlock->_Unlock(FILE__, __LINE__);
    return ret;
}

bool SignalGeneratorCommon::waitForOSReturn() {
    signal_printf("%s[%d]: thread %s waiting for OS return, grabbing lock...\n",
                  FILE__, __LINE__, getThreadStr(getExecThreadID()));
    waitlock->_Lock(FILE__, __LINE__);

    int lock_depth = eventlock->depth();
    assert(lock_depth > 0);
    
    // We can't iterate over eventlock->depth(); we unlock, someone else locks...
    // TODO: make this an eventLock method: "completely release"
    for (unsigned i = 0; i < lock_depth; i++) {
        eventlock->_Unlock(FILE__, __LINE__);
    }
    
    signal_printf("%s[%d]: released global lock in waitForOS: stack of %d\n", FILE__, __LINE__, lock_depth);

    signal_printf("%s[%d]: thread %s waiting for OS return, waiting for signal...\n",
                  FILE__, __LINE__, getThreadStr(getExecThreadID()));
    waitlock->_WaitForSignal(FILE__, __LINE__);

    signal_printf("%s[%d]: thread %s waiting for OS return, returning...\n",
                  FILE__, __LINE__, getThreadStr(getExecThreadID()));
    waitlock->_Unlock(FILE__, __LINE__);

    signal_printf("%s[%d]: reacquiring global lock (taken %d times)\n", 
                  FILE__, __LINE__, lock_depth);
    while (lock_depth > 0) {
        eventlock->_Lock(FILE__, __LINE__);
        lock_depth--;
    }
    
    assert(!isBlockedInOS());

    return true;
}    

bool SignalGeneratorCommon::signalOSReturn() {
    signal_printf("%s[%d]: signalling return from OS: acquiring lock\n", 
                  FILE__, __LINE__);
    waitlock->_Lock(FILE__, __LINE__);

    signal_printf("%s[%d]: signalling return from OS: broadcast\n",
                  FILE__, __LINE__);
    waitlock->_Broadcast(FILE__, __LINE__);

    signal_printf("%s[%d]: signalling return from OS: unlock\n",
                  FILE__, __LINE__);
    waitlock->_Unlock(FILE__, __LINE__);
    return true;
}    

#endif


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

SignalGenerator::SignalGenerator(char *idstr, pdstring file, pdstring dir,
                                 pdvector<pdstring> *argv,
                                 pdvector<pdstring> *envp,
                                 pdstring inputFile,
                                 pdstring outputFile,
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

// The signalGenerator is waiting for waitForContinueLock to get signalled.
// We'll bounce it, then wait on requestContinueLock... 

bool SignalGeneratorCommon::continueProcessBlocking(int requestedSignal) {
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
    
    if (proc->query_for_stopped_lwp() &&
        waitingForOS_) {
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
        activationLock->_Unlock(FILE__, __LINE__);

        assert(activationLock->depth() == 0);
        return true;
    }

    signal_printf("%s[%d]: grabbed requestContinueLock...\n", FILE__, __LINE__);

    int lock_depth = eventlock->depth();
    assert(lock_depth > 0);
    
    // We can't iterate over eventlock->depth(); we unlock, someone else locks...
    // TODO: make this an eventLock method: "completely release"
    for (unsigned i = 0; i < lock_depth; i++) {
        eventlock->_Unlock(FILE__, __LINE__);
    }

    signal_printf("%s[%d]: continueProcessBlocking: gave up global mutex\n", FILE__, __LINE__);

    signal_printf("%s[%d]: continueProcessBlocking, signalling SG\n", FILE__, __LINE__);
    activationLock->_Broadcast(FILE__, __LINE__);

    signal_printf("%s[%d]: continueProcessBlocking, locking waitForContinue\n", FILE__, __LINE__);
    waitForContinueLock->_Lock(FILE__, __LINE__);
    signal_printf("%s[%d]: continueProcessBlocking, unlocking activationLock\n", FILE__, __LINE__);
    activationLock->_Unlock(FILE__, __LINE__);

    signal_printf("%s[%d]: continueProcessBlocking, waiting...\n", FILE__, __LINE__);
    waitForContinueLock->_WaitForSignal(FILE__, __LINE__);
    
    signal_printf("%s[%d]: continueProcessBlocking, woken up and releasing waitForContinue lock.\n", FILE__, __LINE__);
    waitForContinueLock->_Unlock(FILE__, __LINE__);

    signal_printf("%s[%d]: continueProcessBlocking, process continued, grabbing %d global mutexes\n",
                  FILE__, __LINE__, lock_depth);

    while (lock_depth > 0) {
        eventlock->_Lock(FILE__, __LINE__);
        lock_depth--;
    }

    signal_printf("%s[%d]: continueProcessBlocking, returning\n",
                  FILE__, __LINE__);
    
    assert(activationLock->depth() == 0);
    assert(waitForContinueLock->depth() == 0);
    return true;
}

// Continue when both the UI and all active signal handlers have
// asked for the process to continue.

#if 0
bool SignalGeneratorCommon::waitToContinueProcess() {
    signal_printf("%s[%d]: continueProcess, grabbing continueProcessBlockingLock\n",
                  FILE__, __LINE__);
    requestContinueLock->_Lock(FILE__, __LINE__);

    assert(eventlock->depth() == 1);
    
    eventlock->_Unlock(FILE__, __LINE__);

    while(1) {
        signal_printf("%s[%d]: waiting for someone to wake up the process\n", FILE__, __LINE__);
        requestContinueLock->_WaitForSignal(FILE__, __LINE__);

        int activeHandlers = 0;
#if 0
        for (unsigned i = 0; i < handlers.size(); i++) {
            signal_printf("%s[%d]: checking handler %s, processing %d\n",
                          FILE__, __LINE__, getThreadStr(handlers[i]->getThreadID()), handlers[i]->processing());
            if (handlers[i]->processing())
                activeHandlers++;
        }
#endif
        signal_printf("%s[%d]: continueProcess, activeHandlers %d, UIRequested %d\n",
                      FILE__, __LINE__, activeHandlers, stopsDesired_);

        if ((activeHandlers == 0) && (stopsDesired_ == 0))
            break;
    }

    signal_printf("%s[%d]: exchanging requestContinueLock for eventlock\n",
                  FILE__, __LINE__);
    eventlock->_Lock(FILE__, __LINE__);
    requestContinueLock->_Unlock(FILE__, __LINE__);

    return true;
}
#endif

bool SignalGeneratorCommon::continueRequired() {
    // Do we need to continue the process or just check waitpid/poll?

    // sync run gets priority...
    if (syncRunWhenFinished_ == stopRequest)
        return false;

    if (syncRunWhenFinished_ == runRequest) {
        if (asyncRunWhenFinished_ == stopRequest) {
            fprintf(stderr, "Odd case: BPatch requests run, internals request stop\n");
            return false;
        }
        return true;
    }

    // We're unset or purposefully ignoring. 

    if (asyncRunWhenFinished_ == runRequest) {
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

    bool res = proc->continueProc_(continueSig_);
    continueSig_ = -1;

    if (!res)  {
        fprintf(stderr, "%s[%d]:  continueProc_ failed\n", FILE__, __LINE__);
        showErrorCallback(38, "System error: can't continue process");
        return false;
    }
    signal_printf("%s[%d]: setting global process state to running\n", FILE__, __LINE__);
    
    if (proc->status() != exited)
        proc->set_status(running);
    
    // Now wake up everyone who was waiting for me...
    signal_printf("%s[%d]: waking up everyone who was waiting for continue, locking...\n",
                  FILE__, __LINE__);
    waitForContinueLock->_Lock(FILE__, __LINE__);
    signal_printf("%s[%d]: waking up everyone who was waiting for continue, broadcasting...\n",
                  FILE__, __LINE__);
    waitForContinueLock->_Broadcast(FILE__, __LINE__);
    signal_printf("%s[%d]: waking up everyone who was waiting for continue, unlocking\n",
                  FILE__, __LINE__);
    waitForContinueLock->_Unlock(FILE__, __LINE__);

    return true;
}


void SignalGeneratorCommon::setContinueSig(int signalToContinueWith) {
    if ((continueSig_ != -1) &&
        (signalToContinueWith != continueSig_)) {
        fprintf(stderr, "%s[%d]: WARNING: conflict in signal to continue with: previous %d, new %d\n",
                FILE__, __LINE__, continueSig_,  signalToContinueWith);
    }

    continueSig_ = signalToContinueWith;
}

bool SignalGeneratorCommon::pauseProcessBlocking() {

    signal_printf("%s[%d]: pauseProcessBlocking...\n", FILE__, __LINE__);
    syncRunWhenFinished_ = stopRequest;

    return proc->pause();
}
