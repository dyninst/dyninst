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

// $Id: signalhandler.C,v 

#include "process.h"
#include "dyn_lwp.h"
#include "dyn_thread.h"
#include "callbacks.h"
#include "function.h"
#include "stats.h"
#include "signalhandler.h"
#include "Object.h"
#include "mapped_object.h"
//  signal_generator_counter is used to generate identifier strings
//  for signal generator threads.  eg SYNC1, SYNC2, SYNC3

unsigned signal_generator_counter = 0;
eventLock SignalGeneratorCommon::global_wait_list_lock;
pdvector<EventGate *> SignalGeneratorCommon::global_wait_list;

EventGate::EventGate(eventLock *l, eventType t, process *p, dyn_lwp *lwp,
                     eventStatusCode_t status) :
     lock(l), waiting(false)
{
  //cond = new eventCond(lock);
  EventRecord target_event;
  target_event.type = t;
  target_event.proc = p;
  target_event.lwp = lwp;
  target_event.status = status;
  evts.push_back(target_event);

  if (t != evtProcessExit) {
    EventRecord process_exit_event;
    process_exit_event.type = evtProcessExit;
    process_exit_event.proc = p;
    evts.push_back(process_exit_event);
  }
}

bool EventGate::addEvent(eventType type, process *p)
{
  EventRecord target_event;
  target_event.type = type;
  target_event.proc = p;
  if (type == evtProcessExit) {
    for (unsigned int i = 0; i < evts.size(); ++i) {
      if (evts[i].isTemplateOf(target_event)) {
        //fprintf(stderr, "%s[%d]:  dropping duplicate request to wait for proces exit\n",
         //       FILE__, __LINE__);
        return true;
      }
    }
  }
  evts.push_back(target_event);
  return true;
}

EventGate::~EventGate()
{
  //delete cond;
}

EventRecord &EventGate::wait()
{
  trigger.type = evtUndefined;
  assert(lock->depth());
 
  still_waiting:
  waiting = true;
  getMailbox()->executeCallbacks(FILE__, __LINE__);

  if (trigger.type != evtUndefined) {
    return trigger;
  }
  extern int dyn_debug_signal;
  if (dyn_debug_signal) {
    signal_printf("%s[%d][%s]: waiting for event matching:\n", FILE__, __LINE__,
            getThreadStr(getExecThreadID()));
    for (unsigned int i = 0; i < evts.size(); ++i) {
      char buf[1024];
      signal_printf("\t%s\n", evts[i].sprint_event(buf));
    }
  }

  lock->_WaitForSignal(FILE__, __LINE__);
  waiting = false;
  

  bool triggered = false;
  for (unsigned int i = 0; i < evts.size(); ++i) {
    if (evts[i].isTemplateOf(trigger)) {
      triggered = true;
      break;
    }
  }
  if (!triggered) goto still_waiting;

  return trigger;
}

bool EventGate::signalIfMatch(EventRecord &ev)
{
  lock->_Lock(FILE__, __LINE__);
  if (!waiting) {
    lock->_Unlock(FILE__, __LINE__);
    return false;
  }
  bool ret = false;
  for (unsigned int i = 0; i < evts.size(); ++i) {
    if (evts[i].isTemplateOf(ev)) {
      ret = true;
      trigger = ev;
      lock->_Broadcast(FILE__, __LINE__);
      break;
    }
  }
  lock->_Unlock(FILE__, __LINE__);
  return ret;
}

SignalHandler *SignalGenerator::newSignalHandler(char *name, int id)
{
  SignalHandler *sh;
  sh  = new SignalHandler(name, id, this);
  return (SignalHandler *)sh;
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
   struct stat file_stat;
   int stat_result;

   stat_result = stat(file.c_str(), &file_stat);

   if (stat_result == -1) {
      startup_printf("%s[%d]:  failed to read file %s\n", __FILE__, __LINE__, file.c_str());
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
          FILE__, __LINE__, sg->pid, file.c_str());

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

bool SignalGeneratorCommon::waitNextEvent(EventRecord &ev) 
{
  __LOCK;
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
    __UNLOCK;
    return true;
  }

  assert(proc);

  
  if (proc->status() == deleted || proc->status() == exited) {
      fprintf(stderr, "%s[%d]:  getting ready to shut down event handling thread\n", FILE__, __LINE__);
      stopThreadNextIter();
      ev.type = evtShutDown;
      ev.proc = proc;
      __UNLOCK;
      return true;
  }
  signal_printf("%s[%d]: checking process status %s (%d)\n",
                FILE__, __LINE__, proc->getStatusAsString().c_str(), proc->status());
  //  maybe query_for_running_lwp is sufficient here??
  while (  proc->status() != running
           && proc->status() != neonatal
           && !proc->query_for_running_lwp()) {

      signal_printf("%s[%d]:  waiting for process %d to become active, status: %s\n",
                    FILE__, __LINE__, pid, proc->getStatusAsString().c_str());
      waiting_for_active_process = true;
      __WAIT_FOR_SIGNAL;
      signal_printf("%s[%d]: process %d marked active, continuing on to waitNextEventLocked, status: %s\n",
                    FILE__, __LINE__, pid, proc->getStatusAsString().c_str());
  }
  
  if (stop_request) {
      fprintf(stderr, "%s[%d]:  getting ready to shut down event handling thread 2\n", FILE__, __LINE__);
      ev.type = evtShutDown;
      ev.proc = proc;
      __UNLOCK;
      return true;
  }
  
  waiting_for_active_process = false;
  
  ev.type = evtUndefined;
  bool ret = waitNextEventLocked(ev);
  
  //  shut down events will not make it to the handler thread(s), since this thread will
  //  terminate execution after leaving this function.

  if (ev.type == evtShutDown)
    signalEvent(ev);

  __UNLOCK;
  return ret;
}

process *SignalGeneratorCommon::newProcess(pdstring &progpath, int pid_)
{
  SignalGenerator *sg = newSignalGenerator(progpath, pid_);

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
          FILE__, __LINE__, pid_, progpath.c_str());

  

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

SignalGeneratorCommon::SignalGeneratorCommon(char *idstr,pdstring file_, pdstring dir_,
                                 pdvector<pdstring> *argv_,
                                 pdvector<pdstring> *envp_,
                                 pdstring inputFile_,
                                 pdstring outputFile_,
                                 int stdin_fd_, int stdout_fd_,
                                 int stderr_fd_) :
                 EventHandler<EventRecord>(BPatch_eventLock::getLock(),
                                           idstr,/*start thread?*/ false),
                 file(file_),
                 dir(dir_),
                 inputFile(inputFile_),
                 outputFile(outputFile_),
                 stdin_fd(stdin_fd_),
                 stdout_fd(stdout_fd_),
                 stderr_fd(stderr_fd_),
                 argv(argv_),
                 envp(envp_),
                 pid(-1),
                 traceLink(-1),
                 waiting_for_event(false)
{
  signal_printf("%s[%d]:  new SignalGenerator\n", FILE__, __LINE__);
  assert(eventlock == global_mutex);
}

SignalGeneratorCommon::SignalGeneratorCommon(char *idstr, pdstring file_,
                                 int pid_) :
                 EventHandler<EventRecord>(BPatch_eventLock::getLock(),
                                           idstr,/*start thread?*/ false),
                 file(file_),
                 pid(pid_),
                 traceLink(-1),
                 waiting_for_event(false)
{
  signal_printf("%s[%d]:  new SignalGenerator\n", FILE__, __LINE__);
  assert(eventlock == global_mutex);
}

bool SignalGeneratorCommon::wakeUpThreadForShutDown()
{
#if defined (os_windows)
//  DebugBreakProcess(this->proc->processHandle_);
  if (waiting_for_active_process) {
    signalEvent(evtShutDown);
    __BROADCAST;
    return true;
  }
#else
  int sig_to_send = SIGTRAP;
   assert(global_mutex->depth());

  if (waiting_for_event) {
    signal_printf("%s[%d]:  sending SIGTRAP to wake up signal handler\n", FILE__, __LINE__);
    P_kill (pid, sig_to_send);
    waitForEvent(evtShutDown, proc);
    signal_printf("%s[%d][%s]:  got shutdown event\n", FILE__, __LINE__, getThreadStr(getExecThreadID()));
  }
  else if (waiting_for_active_process) {
    signalEvent(evtShutDown);
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
    
    while (sh->isRunning()) {
        waitForEvent(evtAnyEvent);
    }
}

bool SignalGeneratorCommon::initialize_event_handler()
{
  assert(proc);

  //  This is the init function for the event handler.  It is called before the main
  //  event handing loop.  

  //  In the case of the signal handler, here we call either forkNewProcess() or
  //  attachProcess() if the process already exists.

  if (pid == -1) {
    if (!forkNewProcess()) {
       fprintf(stderr, "%s[%d]:  failed to fork a new process for %s\n", FILE__, __LINE__,
               file.c_str());
       return false;
    }

    proc->createRepresentativeLWP();

    if (!proc->setupCreated(traceLink)) {
        delete proc;
        proc = NULL;
        return false;
    }

    int status;
    fileDescriptor desc;
    if (!getExecFileDescriptor(file, getPid(), true, status, desc)) {
        startup_cerr << "Failed to find exec descriptor" << endl;
    ///    cleanupBPatchHandle(theProc->sh->getPid());
      //  processVec.pop_back();
        delete proc;
        proc = NULL;
        return false;
    }
    //HACKSTATUS = status;

    if (!proc->setAOut(desc)) {
        startup_printf("[%s:%u] - Couldn't setAOut\n", __FILE__, __LINE__);
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
    int status = pid;
#endif // defined(i386_unknown_nt4_0)

    fileDescriptor desc;
    if (!getExecFileDescriptor(file, status, false, status, desc)) 
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
     proc->createRepresentativeLWP();
     
     if (!attachProcess()) {
         fprintf(stderr, "%s[%d]:  failed to attach to process %d\n", FILE__, __LINE__,
                 pid);
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

bool SignalGeneratorCommon::handleEventLocked(EventRecord &ev)
{
  char buf[128];
  signal_printf("%s[%d]:  dispatching event %s\n", FILE__, __LINE__,
                ev.sprint_event(buf));

  if (ev.type == evtUndefined) {
    fprintf(stderr, "%s[%d]:  CHECK THIS, undefined event\n", FILE__, __LINE__); 
    return true;
  }

  SignalHandler *sh = NULL;
  for (unsigned int i = 0; i < handlers.size(); ++i) {
    if (handlers[i]->assignEvent(ev)) {
      sh = handlers[i];
      break;
    }
  }

#if 0
  /// This code flat doesn't work. As written, it leads to an assert
  //when freed memory is accessed.  Replacing the "delete handlers[i]"
  //with deleteSignalHandler(handlers[i]) threw an assertion because
  //the sync thread was blocking.

  if ((sh) && (handlers.size() > HANDLER_TRIM_THRESH)) {
    for (int i = handlers.size() - 1; i >= 0; i--) {
      if (!handlers[i]->idle()) break;
      if (handlers[i] != sh) {
        signal_printf("%s[%d]:  trimming idle signal handler %s\n", FILE__, __LINE__,
                      handlers[i]->getName());
        delete handlers[i];
        handlers.erase(i,i);
      }
    }
  }
#endif

  if (handlers.size() > MAX_HANDLERS) {
     fprintf(stderr, "%s[%d]:  FATAL:  Something is horribly wrong.\n", FILE__, __LINE__);
     fprintf(stderr, "\thave %d signal handlers, max is %d\n", 
             handlers.size(), MAX_HANDLERS);
     abort();
  }

  bool ret = true;
  if (!sh) {
    int shid = handlers.size();
    char shname[64];
    sprintf(shname, "SH-%d-%d", pid, shid);
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

  //  special case:  if this event is a process exit event, we want to shut down
  //  the SignalGenerator (there are not going to be any more events to listen for).
  //  The exit event has been passed off to a handler presumably, which will take
  //  care of user notifications, callbacks, etc.

  if (ev.type == evtProcessExit) {
    signal_printf("%s[%d]:  preparing to shut down signal gen for process %d\n", FILE__, __LINE__, pid);
    stopThreadNextIter();
  }

  return ret;
}

bool SignalGeneratorCommon::signalActiveProcess()
{         
  bool ret = true;
  signal_printf("%s[%d]: signalActiveProcess\n", FILE__, __LINE__);
  if (waiting_for_active_process) {
    signal_printf("%s[%d]: signalActiveProcess waking up SignalGenerator\n", FILE__, __LINE__);
    ret = __BROADCAST;
  }
  else {
    signal_printf("%s[%d]: signalActiveProcess, SignalGenerator already awake\n", FILE__, __LINE__);
  }

  return ret;
}   

bool SignalGeneratorCommon::signalEvent(EventRecord &ev)
{
  if (ev.type != evtError) {
    char buf[128];
    signal_printf("%s[%d][%s]:  signalEvent(%s)\n", FILE__, __LINE__, 
                  getThreadStr(getExecThreadID()), ev.sprint_event(buf));
  }
  assert(global_mutex->depth());

  getMailbox()->executeCallbacks(FILE__, __LINE__);

  if (ev.type == evtProcessStop || ev.type == evtProcessExit) {
     //fprintf(stderr, "%s[%d]:  flagging BPatch status change\n", FILE__, __LINE__);
     SignalHandler::flagBPatchStatusChange();
  }

  bool ret = false;
  for (unsigned int i = 0; i <wait_list.size(); ++i) {
    if (wait_list[i]->signalIfMatch(ev)) {
      ret = true;
    }
  }

  global_wait_list_lock._Lock(__FILE__, __LINE__);
  for (unsigned int i = 0; i < global_wait_list.size(); ++i) {
      if (global_wait_list[i]->signalIfMatch(ev)) {
          ret = true;
      }
  }
  global_wait_list_lock._Unlock(__FILE__, __LINE__);


  
#if 0
  if (!ret) 
    signal_printf("%s[%d][%s]:  signalEvent(%s): nobody waiting\n", FILE__, __LINE__, 
                  getThreadStr(getExecThreadID()), eventType2str(ev.type));
#endif
  return ret;
}

bool SignalGeneratorCommon::signalEvent(eventType t)
{
  EventRecord ev;
  ev.type = t;
  return signalEvent(ev);
}

eventType SignalGeneratorCommon::waitForOneOf(pdvector<eventType> &evts)
{
  assert(global_mutex->depth());

  if (getExecThreadID() == getThreadID()) {
    fprintf(stderr, "%s[%d][%s]:   ILLEGAL:  SYNC THREAD waiting on for a signal\n", 
            FILE__, __LINE__, getThreadStr(getExecThreadID()));
    abort();
  }

  //  When to set wait_flag ??
  //    (1)  If we are running on an event handler thread
  //    (2)  If we are currently running inside a callback AND an 
  //         event handler is waiting for the completion of this callback
  SignalHandler *sh = findSHWithThreadID(getExecThreadID());
  if (sh)
    sh->wait_flag = true;
  else {
    CallbackBase *cb = NULL;
    if (NULL != (cb = getMailbox()->runningInsideCallback())) {
      sh = findSHWaitingForCallback(cb);
      if (sh)
        sh->wait_flag = true;
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

  global_wait_list_lock._Lock(__FILE__, __LINE__);
  global_wait_list.push_back(eg);
  global_wait_list_lock._Unlock(__FILE__, __LINE__);

  if (global_mutex->depth() > 1)
     signal_printf("%s[%d]:  about to EventGate::wait(), lock depth %d\n", 
                   FILE__, __LINE__, global_mutex->depth());

  EventRecord result = eg->wait();
  
  global_wait_list_lock._Lock(__FILE__, __LINE__);
  bool found = false;
  for (int i = global_wait_list.size() -1; i >= 0; i--) {
    if (global_wait_list[i] == eg) {
       found = true;
       global_wait_list.erase(i,i);
       delete eg;
       break;
    } 
  }
  global_wait_list_lock._Unlock(__FILE__, __LINE__);
  
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

#if 0
  fprintf(stderr, "%s[%d]:  welcome to waitForEvent(%s)\n", FILE__, __LINE__, eventType2str(evt));
#endif

  //  When to set wait_flag ??
  //    (1)  If we are running on an event handler thread
  //    (2)  If we are currently running inside a callback AND a 
  //         signal handler is waiting for the completion of this callback
  SignalHandler *sh = findSHWithThreadID(getExecThreadID());
  if (sh)
    sh->wait_flag = true;
  else {
    CallbackBase *cb = NULL;
    if (NULL != (cb = getMailbox()->runningInsideCallback())) {
      sh = findSHWaitingForCallback(cb);
      if (sh)
        sh->wait_flag = true;
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
    if (handlers[i]->wait_cb == cb)
      return handlers[i];
  }
  return NULL;
}

bool SignalHandler::isActive(process *p) {
    // Cases:
    if (idle_flag) return false;

    if (wait_cb) {
        // Experiment: release "lock" if we're waiting
        // for a callback to complete.
        return false;
    }

    return p == active_proc;
}

bool SignalGeneratorCommon::activeHandlerForProcess(process *p)
{
    // If the handler is active and running on a different thread from
    // us (as we can get the following:
    //    Handler calls into BPatch
    //    BPatch tries continue
    //    .... continue blocked because of active handler

  for (unsigned int i = 0; i < handlers.size(); ++i) {
      if (handlers[i]->isActive(p) &&
          !(handlers[i]->getThreadID() == getExecThreadID()))
          return true;
  }
  return false;
}

bool SignalGeneratorCommon::anyActiveHandlers()
{
  for (unsigned int i = 0; i < handlers.size(); ++i) {
    if (!handlers[i]->idle())
      return true;
  }
  return false;
}

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

SignalHandler::~SignalHandler()
{
   signal_printf("%s[%d]:  welcome to ~SignalHandler\n", FILE__, __LINE__);
   if (idle_flag || wait_flag) {
     // maybe a bit heavy handed here....
     stopThreadNextIter();
     //killThread();
   }else {
     signal_printf("%s[%d]:  waiting for idle before killing thread %s\n", 
             FILE__, __LINE__, getName());
     int timeout = 2000 /*ms*/, time_elapsed = 0;
     while (!idle_flag && !wait_flag) {
       struct timeval slp;
       slp.tv_sec = 0;
       slp.tv_usec = 10 /*ms*/ *1000;
       select(0, NULL, NULL, NULL, &slp);
       time_elapsed +=10;
       if (time_elapsed >= timeout) {
         fprintf(stderr, "%s[%d]:  cannot kill thread %s, did not become idle\n", FILE__, __LINE__, getName());
         break;
       }
     }
     if (idle_flag || wait_flag) {
       stopThreadNextIter();
     //  killThread();
     }
   }
}


bool SignalHandler::handleSingleStep(EventRecord &ev) 
{
   if (!ev.lwp->isSingleStepping()) {
     fprintf(stderr, "%s[%d]:  unexpected step event\n", FILE__, __LINE__);
   }
   ev.lwp->setSingleStepping(false);
   sg->signalEvent(evtDebugStep);
   return true;
}

bool SignalHandler::handleProcessStop(EventRecord &ev)
{
   process *proc = ev.proc;
   bool retval = false;

#if defined(os_linux)
      // Linux uses SIGSTOPs for process control.  If the SIGSTOP
      // came during a process::pause (which we would know because
      // suppressEventConts() is set) then we'll handle the signal.
      // If it comes at another time we'll assume it came from something
      // like a Dyninst Breakpoint and not handle it.      
      if (!ev.lwp) {
         fprintf(stderr, "%s[%d]:  no lwp for SIGSTOP handling (needed)\n", FILE__, __LINE__);
         return false;
      }
      proc->set_lwp_status(ev.lwp, stopped);
#else
      signal_printf("%s[%d]:  unhandled SIGSTOP for pid %d, process will stay paused\n",
             FILE__, __LINE__, proc->getPid());
#endif
      retval = true;

   bool exists = false;
   BPatch_process *bproc = BPatch::bpatch->getProcessByPid(proc->getPid(), &exists);
   if (bproc) {
       setBPatchProcessSignal(bproc, ev.what);
       bproc->isVisiblyStopped = true;
   }

   // Unlike other signals, don't forward this to the process. It's stopped
   // already, and forwarding a "stop" does odd things on platforms
   // which use ptrace. PT_CONTINUE and SIGSTOP don't mix
   return retval;
}

bool SignalHandler::forwardSigToProcess(EventRecord &ev) 
{
    process *proc = ev.proc;

    // Pass the signal along to the child
    bool res;
    if(process::IndependentLwpControl()) {
       res = ev.lwp->continueLWP(ev.what);
    } else {
       res = proc->continueProc(ev.what);
    }
    if (res == false) {
        fprintf(stderr, "%s[%d]:  Couldn't forward signal %d to process %d\n",
                FILE__, __LINE__, ev.what, proc->getPid());
        logLine("error  in forwarding  signal\n");
        showErrorCallback(38, "Error  in forwarding  signal");
        return false;
    } 

    return true;
}

bool SignalHandler::handleProcessExit(EventRecord &ev) 
{
  bool ret = false;
  process *proc = ev.proc;

  if (ev.status == statusNormal) {
      sprintf(errorLine, "Process %d has terminated with code 0x%x\n",
              proc->getPid(), (int) ev.what);
      statusLine(errorLine);
#if defined(os_windows)
      //  on the unixes we do this at syscall exit()
      proc->triggerNormalExitCallback(ev.what);
#endif
      ret = proc->handleProcessExit();
   } else if (ev.status == statusSignalled) {
      sprintf(errorLine, "process %d has terminated on signal %d\n",
              proc->getPid(), (int) ev.what);
      logLine(errorLine);
      statusLine(errorLine);
      printDyninstStats();
      proc->triggerSignalExitCallback(ev.what);
      ret = proc->handleProcessExit();
    } else {
      sprintf(errorLine, "process %d has terminated for unknown reason\n",
              proc->getPid());
      logLine(errorLine);
      ret = proc->handleProcessExit();
      //ret = true; //  maybe this should be false?  (this case is an error)
    }

  handleProcessExitPlat(ev);

  flagBPatchStatusChange();
  return ret;
}

bool SignalHandler::handleCritical(EventRecord &ev) 
{
   process *proc = ev.proc;
   assert(proc);

   signal_printf("Process %d dying on signal %d\n", proc->getPid(), ev.what);

    {
#ifndef mips_unknown_ce2_11 //ccw 6 feb 2001 : 29 mar 2001
        // Should walk stacks for other threads as well
        pdvector<pdvector<Frame> > stackWalks;
        proc->walkStacks(stackWalks);
        for (unsigned walk_iter = 0; walk_iter < stackWalks.size(); walk_iter++) {
            fprintf(stderr, "%s[%d]:  Registers for pid %d, lwpid %d\n", FILE__, __LINE__,
                    stackWalks[walk_iter][0].getLWP()->proc()->getPid(), 
                    stackWalks[walk_iter][0].getLWP()->get_lwp_id());
            stackWalks[walk_iter][0].getLWP()->dumpRegisters();
            fprintf(stderr, "%s[%d]:  Stack for pid %d, lwpid %d\n", FILE__, __LINE__,
                    stackWalks[walk_iter][0].getLWP()->proc()->getPid(), 
                    stackWalks[walk_iter][0].getLWP()->get_lwp_id());
            for( unsigned i = 0; i < stackWalks[walk_iter].size(); i++ )
            {
                //int_function* f = proc->findFuncByAddr( stackWalks[walk_iter][i].getPC() );
                //const char* szFuncName = (f != NULL) ? f->prettyName().c_str() : "<unknown>";
                //fprintf( stderr, "%08x: %s\n", stackWalks[walk_iter][i].getPC(), szFuncName );
                cerr << stackWalks[walk_iter][i] << endl;
            }
        }
        
#endif
    }

    int sleep_counter = SLEEP_ON_MUTATEE_CRASH;
    while (dyn_debug_signal && (sleep_counter > 0)) {
       signal_printf("Critical signal received, spinning to allow debugger to attach\n");
       sleep(10);
       sleep_counter -= 10;
    }

    if (CAN_DUMP_CORE)
      proc->dumpImage("imagefile");
    else
      proc->dumpMemory((void *)ev.address, 32);

    fprintf(stderr, "Forwarding signal to process\n");
    return forwardSigToProcess(ev);
}

bool SignalHandler::handleEvent(EventRecord &ev)
{
  global_mutex->_Lock(FILE__, __LINE__);
  bool ret = handleEventLocked(ev); 
  if (!events_to_handle.size())
    idle_flag = true;
  active_proc = NULL;
  global_mutex->_Unlock(FILE__, __LINE__);

  return ret;
}

bool SignalHandler::handleForkEntry(EventRecord &ev)
{
     signal_printf("Welcome to FORK ENTRY for process %d\n",
                   ev.proc->getPid());
     return ev.proc->handleForkEntry();
}

bool SignalHandler::handleLwpExit(EventRecord &ev)
{
   signal_printf("%s[%d]:  welcome to handleLwpExit\n", FILE__, __LINE__);
   process *proc = ev.proc;
   dyn_lwp *lwp = ev.lwp;
   dyn_thread *thr = NULL;
   //Find the exiting thread
   for (unsigned i=0; i<proc->threads.size(); i++) {
       if (proc->threads[i]->get_lwp()->get_lwp_id() == lwp->get_lwp_id()) {
           thr = proc->threads[i];
           break;
       }
   }
   if (!thr)
   {
       fprintf(stderr, "No matching thread!\n");
       proc->continueProc();
       return false;
   }

   ev.type = evtThreadExit;

   if (proc->IndependentLwpControl())
      proc->set_lwp_status(ev.lwp, exited);

   BPatch::bpatch->registerThreadExit(proc, thr->get_tid());

   flagBPatchStatusChange();
   return true;
}
bool SignalHandler::handleSyscallEntry(EventRecord &ev)
{
    signal_printf("%s[%d]:  welcome to handleSyscallEntry\n", FILE__, __LINE__);
    process *proc = ev.proc;
    bool ret = false;
    switch ((procSyscall_t)ev.what) {
      case procSysFork:
          ret = handleForkEntry(ev);
          break;
      case procSysExec:
         ret = handleExecEntry(ev);
         break;
      case procSysExit:
          signal_printf("%s[%d]:  handleSyscallEntry exit(%d)\n", FILE__, __LINE__, ev.what);
          proc->triggerNormalExitCallback(INFO_TO_EXIT_CODE(ev.info));
          ret = true;
          break;
      case procLwpExit:
         assert(0);
         //  this case should be hijacked during event decoding and mapped onto
         //  the evtThreadExit event type.

         break;
      default:
      // Check process for any other syscall
      // we may have trapped on entry to?
      ret = false;
      break;
    }
    return ret;
}

bool SignalHandler::handleForkExit(EventRecord &ev)
{
     signal_printf("Welcome to FORK EXIT for process %d\n",
                   ev.proc->getPid());

     process *proc = ev.proc;
     // Fork handler time
     extern pdvector<process*> processVec;
     int childPid = INFO_TO_PID(ev.info);

     if (childPid == getpid()) {
         // this is a special case where the normal createProcess code
         // has created this process, but the attach routine runs soon
         // enough that the child (of the mutator) gets a fork exit
         // event.  We don't care about this event, so we just continue
         // the process - jkh 1/31/00
         return true;
     } else if (childPid > 0) {

         unsigned int i;
         for (i=0; i < processVec.size(); i++) {
             if (processVec[i] &&
                 (processVec[i]->getPid() == childPid)) break;
         }
         if (i== processVec.size()) {
             // this is a new child, register it with dyninst
             // Note: we need to wait for the child process to be created.

             sleep(1);

             // For now, we sleep (apparently), but the better solution is to
             // loop waiting for the child to be created and then attach to it.
             // We have seen the following order:
             // Parent exits fork
             // We get notification -- but no child yet.
             // Child is created
             // This seems to be OS dependent on who goes first - parent or child.

             // We leave the parent paused until the child is finished,
             // so that we can be sure to copy everything correctly.

             process *theChild = ev.proc->sh->newProcess(proc, (int) childPid, -1);
             if (!theChild)
               return false;
   
             proc->handleForkExit(theChild);
        }
     }
     else {
         // Child signalGenerator may execute this guy ; leave it untouched.

         // If we've already received the stop (AKA childForkStopAlreadyReceived
         // is true), then we're getting double-signalled due to odd Linux behavior.
         // Continue the process.
         // If not, then set to true and leave paused.

         if (proc->childForkStopAlreadyReceived_) {
             proc->continueProc();
         }
     }
    return true;
}
// the alwaysdosomething argument is to maintain some strange old code
bool SignalHandler::handleExecExit(EventRecord &ev)
{
    process *proc = ev.proc;
    proc->nextTrapIsExec = false;
    if ( (int) INFO_TO_PID(ev.info) == -1) {
        // Failed exec, do nothing
        proc->continueProc();
    }

    proc->execFilePath = proc->tryToFindExecutable(proc->execPathArg, proc->getPid());
    // As of Solaris 2.8, we get multiple exec signals per exec.
    // My best guess is that the daemon reads the trap into the
    // kernel as an exec call, since the process is paused
    // and PR_SYSEXIT is set. We want to ignore all traps 
    // but the last one.

    // We also see an exec as the first signal in a process we create. 
    // That's because it... wait for it... execed!
    if (!proc->reachedBootstrapState(begun_bs)) {
        return handleProcessCreate(ev);
    }


   int status = 0;
   // False: not waitin' for a signal (theoretically, we already got
    // it when we attached)
    fileDescriptor desc;
    if (!proc->sh->getExecFileDescriptor(proc->execFilePath,
                                   proc->getPid(),
                                   false,
                                   status,
                                   desc)) {
        cerr << "Failed to find exec descriptor" << endl;
        proc->continueProc();
    }

    // Unlike fork, handleExecExit doesn't do all processing required.
    // We finish up when the trap at main() is reached.
    proc->handleExecExit(desc);

    return true;
}


bool SignalHandler::handleSyscallExit(EventRecord &ev)
{
    process *proc = ev.proc;
    bool runProcess = false;

    signal_printf( "%s[%d]:  welcome to handleSyscallExit\n", FILE__, __LINE__);

    runProcess = ev.lwp->handleSyscallTrap(ev);

    // Fall through no matter what since some syscalls have their
    // own handlers.
    switch((procSyscall_t) ev.what) {
    case procSysFork:
        signal_printf("%s[%d]:  Fork Exit\n", FILE__, __LINE__);
        handleForkExit(ev);
        runProcess = false; // Fork handler runs process
        break;
    case procSysExec:
        signal_printf("%s[%d]:  Exec Exit\n", FILE__, __LINE__);
        handleExecExit(ev);
        runProcess = false; // As with fork, above
        break;
    case procSysLoad:
        signal_printf("%s[%d]:  Load Exit\n", FILE__, __LINE__);
        handleLoadLibrary(ev);
        runProcess = true;
        break;
    default:
        // We stopped on a system call and didn't particularly care,
        // or handled it above but left ourselves paused. 
        break;
    }

    if (runProcess)
        proc->continueProc();

    return true;
}

bool SignalHandler::handleEventLocked(EventRecord &ev)
{
  signal_printf("%s[%d]:  got event: %s\n", FILE__, __LINE__, eventType2str(ev.type));

  process *proc = ev.proc;
  bool ret = false;
  Frame activeFrame;
  assert(proc);
  
  // One big switch statement
  switch(ev.type) {
     // First the platform-independent stuff
     // (/proc and waitpid)
     case evtProcessExit:
        ret = handleProcessExit(ev);
        break;
     case evtProcessCreate:
        ret = handleProcessCreate(ev);
        break;
     case evtThreadCreate:
        ret = handleThreadCreate(ev);
        break;
     case evtThreadExit:
        ret = handleLwpExit(ev);
#if defined(os_windows)
        proc->continueProc();
#endif
        break;
     case evtProcessAttach:
        proc->setBootstrapState(initialized_bs);
        ret = true;
        break;
     case evtProcessInit:
        proc->handleTrapAtEntryPointOfMain(ev.lwp);
        proc->setBootstrapState(initialized_bs);
        // If we were execing, we now know we finished
        if (proc->execing()) {
           proc->finishExec();
        }
        ret = true;
        break;
     case evtProcessLoadedRT:
     {
        pdstring buffer = pdstring("PID=") + pdstring(proc->getPid());
        buffer += pdstring(", loaded dyninst library");
        statusLine(buffer.c_str());
        startup_cerr << "trapDueToDyninstLib returned true, trying to handle\n";
        proc->loadDYNINSTlibCleanup(ev.lwp);
        proc->setBootstrapState(loadedRT_bs);
        //getSH()->signalEvent(evtProcessLoadedRT);
        ret = true;
        break;
     }
     case evtInstPointTrap:
         // Linux inst via traps
         ev.lwp->changePC(proc->trampTrapMapping[ev.address], NULL);
         proc->continueProc();
         ret = true;
         break;
     case evtLoadLibrary:
     case evtUnloadLibrary:
        ret = handleLoadLibrary(ev);
        proc->continueProc();
        break;
     case evtPreFork:
         // If we ever want to callback this guy, put it here.
         ret = true;
        proc->continueProc();
        break;
     case evtSignalled:
     {
        ret = forwardSigToProcess(ev);
        break;
     }
     case evtProcessStop:
     {
         ret = handleProcessStop(ev);
         if (!ret) {
             fprintf(stderr, "%s[%d]:  handleProcessStop failed\n", FILE__, __LINE__);
         }
         break;
     }
        // Now the /proc only
        // AIX clones some of these (because of fork/exec/load notification)
     case evtRPCSignal:
       ret = proc->getRpcMgr()->handleRPCEvent(ev);
       break;
     case evtSyscallEntry:
        ret = handleSyscallEntry(ev);
        if (!ret)
            cerr << "handleSyscallEntry failed!" << endl;
        break;
     case evtSyscallExit:
        ret = handleSyscallExit(ev);
        if (!ret)
            fprintf(stderr, "%s[%d]: handlesyscallExit failed! ", __FILE__, __LINE__); ;
        break;
     case evtSuspended:
       proc->continueProc();   // ignoring this signal
       ret = true;
       flagBPatchStatusChange();
       break;
     case evtDebugStep:
         handleSingleStep(ev);
         ret = 1;
         break;
     case evtUndefined:
        // Do nothing
         cerr << "Undefined event!" << endl;
        break;
     case evtCritical:
         ret = handleCritical(ev);
         break;
     case evtRequestedStop:
         ret = true;
         break;
     case evtTimeout:
     case evtThreadDetect:
         proc->continueProc();         
        ret = true;
        break;
     case evtNullEvent:
         ret = true;
         break;
     default:
        fprintf(stderr, "%s[%d]:  cannot handle signal %s\n", FILE__, __LINE__, eventType2str(ev.type));
        assert(0 && "Undefined");
   }

   sg->signalEvent(ev);

   if (ret == false) {
      //  if ret is false, complain, but return true anyways, since the handler threads
      //  should be shut down by the SignalGenerator.
      char buf[128];
      fprintf(stderr, "%s[%d]:  failed to handle event %s\n", FILE__, __LINE__,
              ev.sprint_event(buf));
      ret = true;
   }

   return ret;
}

bool SignalHandler::idle()
{
  bool ret;
  _Lock(FILE__, __LINE__);
  ret = idle_flag;
  _Unlock(FILE__, __LINE__);
  return ret;
}

bool SignalHandler::waiting()
{
  bool ret;
  _Lock(FILE__, __LINE__);
  ret = wait_flag;
  _Unlock(FILE__, __LINE__);
  return ret;
}

bool SignalHandler::assignEvent(EventRecord &ev) 
{
  char buf[128];
  bool ret = false;
  assert(global_mutex->depth());

  //  after we get the lock, the handler thread should be either idle, or waiting
  //  for some event.  

  while (!idle_flag) {
    if ((wait_flag) && (ev.type != evtShutDown)) {
      signal_printf("%s[%d]:  cannot assign event %s to %s, while it is waiting\n", 
                    FILE__, __LINE__, ev.sprint_event(buf), getName());
      return false;
    }
    signal_printf("%s[%d]:  shoving event %s into the queue for %s\n",
         FILE__, __LINE__, ev.sprint_event(buf), getName());
    events_to_handle.push_back(ev);
    if (ev.type == evtProcessExit) 
      sg->signalEvent(ev);
    else 
      _Broadcast(FILE__, __LINE__);
    return true;
  }

  if (idle_flag) {
    // handler thread is not doing anything, assign away...
    signal_printf("%s[%d]:  assigning event %s to %s\n", FILE__, __LINE__,
                  ev.sprint_event(buf), getName());
    events_to_handle.push_back(ev);
    ret = true;
    idle_flag = false;
    _Broadcast(FILE__, __LINE__);
  } 
 // else {
 //   char buf[128];
 //   fprintf(stderr, "%s[%d]:  WEIRD, tried to assign %s to busy handler\n",
 //          FILE__, __LINE__, ev.sprint_event(buf));
 // } 

  return ret;
}

bool SignalHandler::waitNextEvent(EventRecord &ev)
{
  bool ret;
  _Lock(FILE__, __LINE__);
  while (idle_flag) {
    signal_printf("%s[%d]:  handler %s waiting for something to do\n", 
                  FILE__, __LINE__, getName());
#if 0
    fprintf(stderr, "%s[%d]:  sg->waitingForActiveProcess = %s\n", FILE__, __LINE__, sg->waitingForActiveProcess() ? "true" : "false");
    fprintf(stderr, "%s[%d]:  sg->anyActiveHandlers() = %s\n", FILE__, __LINE__, sg->anyActiveHandlers() ? "true" : "false");
#endif

    // Commented out 20FEB06 - Bernat
    // This just makes us busy-wait; we wake up the signal generator,
    // which wakes us up, ...
    //if (!sg->anyActiveHandlers() && sg->waitingForActiveProcess()) {
    //sg->signalEvent(evtProcessStop);
    //}


    // Waiting for someone to ping our lock (internal thread)
    _WaitForSignal(FILE__, __LINE__);

    // Someone wants us to go away....
    if (stop_request) {
        signal_printf("%s[%d]:  sg got stop request... no more handling\n", FILE__, __LINE__);
       ev.type = evtShutDown;
       
       // And bounce it back to the signalGenerator
       sg->signalEvent(ev);
       _Unlock(FILE__, __LINE__);
      return true;
    }
    signal_printf("%s[%d]:  handler %s has been signalled: got event = %s\n", 
                  FILE__, __LINE__, getName(), idle_flag ? "false" : "true");
  }

  // Take the top thing off our queue and handle it. 
  ret = true;
  ev = events_to_handle[0];
  events_to_handle.erase(0,0);
  active_proc = ev.proc;

  if (ev.type == evtUndefined) {
    fprintf(stderr, "%s[%d]:  got evtUndefined for next event!\n", FILE__, __LINE__);
    ret = false;
  }

  _Unlock(FILE__, __LINE__);
  if (!ret) abort();
  return ret;
}

signal_handler_location::signal_handler_location(Address addr, unsigned size) :
    addr_(addr),
    size_(size) {}

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
                    __FILE__, __LINE__, getThreadStr(getExecThreadID()));
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
