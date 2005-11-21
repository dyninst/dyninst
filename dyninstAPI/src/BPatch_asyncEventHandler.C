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


#include "util.h"
#include "BPatch_asyncEventHandler.h"
#include "EventHandler.h"
#include "mailbox.h"
#include "BPatch_libInfo.h"
#include <stdio.h>

#if defined (os_windows)
#include <process.h>
#else
#include <signal.h>
#include <pwd.h>
#include <sys/types.h>
//#if defined (os_osf)
//typedef unsigned long socklen_t;
//#ifndef _XOPEN_SOURCE
//#define _XOPEN_SOURCE 500
//#else
//#undef _XOPEN_SOURCE
//#define _XOPEN_SOURCE 500
//#endif
//#ifndef _XOPEN_SOURCE_EXTENDED
//#define _XOPEN_SOURCE_EXTENDED 1
//#endif
//#define _SOCKADDR_LEN
#include <sys/types.h>
#include <sys/socket.h>
//#endif
#endif

#include "BPatch_eventLock.h"
#include "mailbox.h"
#include "callbacks.h"
#include "EventHandler.h"
#include "util.h"
#include "process.h"

extern unsigned long primary_thread_id;

BPatch_asyncEventHandler *global_async_event_handler = NULL;
BPatch_asyncEventHandler *getAsync() 
{
  if (!global_async_event_handler) {
    // BPatch creates and initializes, so just...

    abort();
  }
  return global_async_event_handler;
}
//extern MUTEX_TYPE global_mutex; // see BPatch_eventLock.h
//extern bool mutex_created = false;

void makeThreadDeleteCB(process *p, int index);

//  A wrapper for pthread_create, or its equivalent.

inline THREAD_RETURN  asyncHandlerWrapper(void *h)
{
  ((BPatch_asyncEventHandler * )h)->main();
  DO_THREAD_RETURN;
}

bool BPatch_asyncEventHandler::connectToProcess(BPatch_process *p)
{
  async_printf("%s[%d][%s]:  enter ConnectToProcess %d\n", FILE__, __LINE__,getThreadStr(getExecThreadID()), p->getPid());
  //  All we do here is add the process to the list of connected processes
  //  with a fd equal to -1, indicating the not-yet-connected state.
  //
  //  Then remotely execute code in the mutatee to initiate the connection.
  
  //  make sure that this process is not already known
  for (int i = (int) process_fds.size() -1 ; i >= 0; i--) 
  {
    if ((p == process_fds[i].process) || 
        (p->getPid() == process_fds[i].process->getPid()))
    {
      //  If it is, delete the old record to prepare for the new one.
      //  This case can be encountered in the case of multiple process management
      //  when processes are created and terminated rapidly.
      //fprintf(stderr,"%s[%d]:  duplicate request to connect to process %d\n",
      //      __FILE__, __LINE__, p->getPid());
      process_fds.erase(i,i);
      //return false;
    }
  } 

  //  add process to list
  process_record newp;
  newp.process = p;
  newp.fd = -1;
  process_fds.push_back(newp);

  //  get mutatee to initiate connection

  //  find the runtime library module
#if defined (os_windows)
  //  find the variable to set with the port number to connect to
  BPatch_variableExpr *portVar;
  portVar = p->getImage()->findVariable("connect_port");
  if (!portVar) {
    fprintf(stderr, "%s[%d]:  cannot find var connect_port in rt lib\n",
           __FILE__, __LINE__);
    return false;
  }
  if (!portVar->writeValue((void *) &listen_port, sizeof(listen_port), false)) {
    fprintf(stderr, "%s[%d]:  cannot write var connect_port in rt lib\n",
           __FILE__, __LINE__);
    return false;
  }

  //  find the function that will initiate the connection
  BPatch_Vector<BPatch_function *> funcs;
  if (!p->getImage()->findFunction("DYNINSTasyncConnect", funcs, true, true, true)
      || !funcs.size() ) {
    bpfatal("%s[%d]:  could not find function: DYNINSTasyncConnect\n",
            __FILE__, __LINE__);
    return false;
  }
  if (funcs.size() > 1) {
    bperr("%s[%d]:  found %d varieties of function: DYNINSTasyncConnect\n",
          __FILE__, __LINE__, funcs.size());
  }
#endif

  return true;
}

bool BPatch_asyncEventHandler::detachFromProcess(BPatch_process *p)
{
  //  find the fd for this process 
  //  (reformat process vector while we're at it)
#if ! defined( cap_async_events )
   return true;
#endif
  int targetfd = -2;
  for (unsigned int i = 0; i < process_fds.size(); ++i) {
    if (process_fds[i].process == p) {
      //fprintf(stderr, "%s[%d]:  removing process %d\n", __FILE__, __LINE__, p->getPid());
      targetfd  = process_fds[i].fd;
      process_fds.erase(i,i);
      break;
    }
  } 

  if (targetfd == -2) {
    //  if we have no record of this process. must already be detached
    //bperr("%s[%d]:  detachFromProcess(%d) could not find process record\n",
    //      __FILE__, __LINE__, p->getPid());
    return true;
  }

  //  if we never managed to fully attach, targetfd might still be -1.
  //  not sure if this could happen, but just return in this case.
  if (targetfd == -1) return true;

  //  get the mutatee to close the comms file desc.

  if (!mutateeDetach(p)) {
    //bperr("%s[%d]:  detachFromProcess(%d) could not clean up mutatee\n",
    //      __FILE__, __LINE__, p->getPid());
  }

  //  close our own file desc for this process.
  P_close(targetfd);

  return true; // true
}

BPatch_asyncEventHandler::BPatch_asyncEventHandler() :
  EventHandler<EventRecord>(BPatch_eventLock::getLock(), "ASYNC",false /*create thread*/)
{
  //  prefer to do socket init in the initialize() function so that we can
  //  return errors.


}
#if defined(os_windows)
static
void
cleanupSockets( void )
{
    WSACleanup();
}
#endif

bool BPatch_asyncEventHandler::initialize()
{

#if defined(os_windows)
  WSADATA data;
  bool wsaok = false;

  // request WinSock 2.0
  if( WSAStartup( MAKEWORD(2,0), &data ) == 0 )
  {
     // verify that the version that was provided is one we can use
     if( (LOBYTE(data.wVersion) == 2) && (HIBYTE(data.wVersion) == 0) )
     {
         wsaok = true;
     }
  }
  assert(wsaok);

  //  set up socket to accept connections from mutatees (on demand)
  sock = P_socket(PF_INET, SOCK_STREAM, 0);
  if (INVALID_PDSOCKET == sock) {
    bperr("%s[%d]:  new socket failed, sock = %d, lasterror = %d\n", __FILE__, __LINE__, (unsigned int) sock, WSAGetLastError());
    return false;
  }

  struct sockaddr_in saddr;
  struct in_addr *inadr;
  struct hostent *hostptr;

  hostptr = gethostbyname("localhost");
  inadr = (struct in_addr *) ((void*) hostptr->h_addr_list[0]);
  memset((void*) &saddr, 0, sizeof(saddr));
  saddr.sin_family = PF_INET;
  saddr.sin_port = htons(0); // ask system to assign
  saddr.sin_addr = *inadr;
  
  const char *path = "windows-socket";
#else
  //  set up socket to accept connections from mutatees (on demand)
  sock = P_socket(SOCKET_TYPE, SOCK_STREAM, 0);
  if (INVALID_PDSOCKET == sock) {
    bperr("%s[%d]:  new socket failed\n", __FILE__, __LINE__);
    return false;
  }

  uid_t euid = geteuid();
  struct passwd *passwd_info = getpwuid(euid);
  assert(passwd_info);
  char path[128];
  snprintf(path, 128, "%s/dyninstAsync.%s.%d", P_tmpdir, 
                 passwd_info->pw_name, (int) getpid());

  struct sockaddr_un saddr;
  saddr.sun_family = AF_UNIX;
  strcpy(saddr.sun_path, path);

  //  make sure this file does not exist already.
  if ( 0 != unlink(path) && (errno != ENOENT)) {
     bperr("%s[%d]:  unlink failed [%d: %s]\n", __FILE__, __LINE__, errno, 
            strerror(errno));
  }
#endif

  //  bind socket to port (windows) or temp file in the /tmp dir (unix)

  if (PDSOCKET_ERROR == bind(sock, (struct sockaddr *) &saddr, 
                             sizeof(saddr))) { 
    bperr("%s[%d]:  bind socket to %s failed\n", __FILE__, __LINE__, path);
    return false;
  }

#if defined(os_windows)
  //  get the port number that was assigned to us
  int length = sizeof(saddr);
  if (PDSOCKET_ERROR == getsockname(sock, (struct sockaddr *) &saddr,
                                    &length)) {
    bperr("%s[%d]:  getsockname failed\n", __FILE__, __LINE__);
    return false;
  }
  listen_port = ntohs (saddr.sin_port);
#endif

  // set socket to listen for connections  
  // (we will explicitly accept in the main event loop)

  if (PDSOCKET_ERROR == listen(sock, 32)) {  //  this is the number of simultaneous connects we can handle
    bperr("%s[%d]:  listen to %s failed\n", __FILE__, __LINE__, path);
    return false;
  }

  //  Finally, create the event handling thread
  if (!createThread()) {
    bperr("%s[%d]:  could not create event handling thread\n", 
          __FILE__, __LINE__);
    return false;
  }

  startup_printf("%s[%d]:  Created async thread\n", __FILE__ , __LINE__);
  return true;
}

BPatch_asyncEventHandler::~BPatch_asyncEventHandler()
{
  if (isRunning()) 
    if (!shutDown()) {
      bperr("%s[%d]:  shut down async event handler failed\n", __FILE__, __LINE__);
    }

#if defined (os_windows)
  WSACleanup();
#else
  

  uid_t euid = geteuid();
  struct passwd *passwd_info = getpwuid(euid);
  assert(passwd_info);
  //  clean up any files left over in the /tmp dir
  char path[128];
  snprintf(path, 128, "%s/dyninstAsync.%s.%d", P_tmpdir, 
                passwd_info->pw_name, (int) getpid());
  unlink(path);

#endif
}

bool BPatch_asyncEventHandler::shutDown()
{
  if (!isRunning()) goto close_comms;

#if defined(os_windows)
  shutDownFlag = true;
#else
  int killres;
  killres = pthread_kill(handler_thread, 9);
  if (killres) {
     fprintf(stderr, "%s[%d]:  pthread_kill: %s[%d]\n", __FILE__, __LINE__,
             strerror(killres), killres);
     return false;
  }
  fprintf(stderr, "%s[%d]:  \t\t..... killed.\n", __FILE__, __LINE__);
#endif

  close_comms:

  return true;
}

bool BPatch_asyncEventHandler::waitNextEvent(EventRecord &ev)
{
  //  Since this function is part of the main event loop, __most__ of
  //  it is under lock. This is necessary to protect data in this class
  //  (process-fd mappings for ex) from race conditions.
  // 
  //  The basic lock structure:
  //     Lock
  //       do set up for select
  //     Unlock
  //     select();
  // 
  //     Lock
  //       analyze results of select
  //     Unlock
  //     return
  __LOCK;

  async_printf("%s[%d]: welcome to waitnextEvent\n", FILE__, __LINE__);
  //  keep a static list of events in case we get several simultaneous
  //  events from select()...  just in case.

  if (event_queue.size()) {
    // we already have one (from last call of this func)
    //
    //  this might result in an event reordering, not sure if important
    //   (since we are removing from the end of the list)
    //ev = event_queue[event_queue.size() - 1];
    //event_queue.pop_back();
    ev = event_queue[0];
    event_queue.erase(0,0);
    async_printf("%s[%d]: waitnextEvent, stale event %s from last time\n", 
                 FILE__, __LINE__, eventType2str(ev.type));
    __UNLOCK;
    return true;
  }

  int width = 0;
  fd_set readSet;
  fd_set errSet;

  FD_ZERO(&readSet);
  FD_ZERO(&errSet);

  struct timeval timeout;
  //timeout.tv_usec = 1000*100;
#if defined(os_windows)
  timeout.tv_sec = 20;
#elif defined(arch_ia64)
  timeout.tv_sec = 1;
#elif defined(os_linux)
  timeout.tv_sec = 5;
#else
  timeout.tv_sec = 5;
#endif
  timeout.tv_sec = 10;
  timeout.tv_usec = 100*1000;

  //  start off with a NULL event:
  ev.type = evtNullEvent;

  cleanUpTerminatedProcs();

  //  build the set of fds we want to wait on, one fd per process
  for (unsigned int i = 0; i < process_fds.size(); ++i) {

    if (process_fds[i].fd == -1) continue; // waiting for connect/accept

    FD_SET(process_fds[i].fd, &readSet);
    FD_SET(process_fds[i].fd, &errSet);
    if (process_fds[i].fd > width)
      width = process_fds[i].fd;
  }

  //  Add the (listening) socket to set(s)
  FD_SET(sock, &readSet);
  if (sock > width)
     width = sock;

  // "width" is computed but ignored on Windows NT, where sockets
  // are not represented by nice little file descriptors.

  __UNLOCK;

  if (-1 == P_select(width+1, &readSet, NULL, &errSet, &timeout)) {
    __LOCK;
    if (errno == EBADF) {
      if (!cleanUpTerminatedProcs()) {
        //fprintf(stderr, "%s[%d]:  FIXME:  select got EBADF, but no procs terminated\n",
        //       __FILE__, __LINE__);
        __UNLOCK;
        return false;
      }
      else {
        __UNLOCK;
        return true;  
      }
    }
    bperr("%s[%d]:  select returned -1\n", __FILE__, __LINE__);
    __UNLOCK;
    return false;
  }

  ////////////////////////////////////////
  //  WARNING:  THIS SECTION IS UNLOCKED -- don't access any non local vars here
  ////////////////////////////////////////

  //  See if we have any new connections (accept):
  if (FD_ISSET(sock, &readSet)) {

     struct sockaddr cli_addr;
     SOCKLEN_T clilen = sizeof(cli_addr);
     
     //fprintf(stderr, "%s[%d]:  about to accept\n", __FILE__, __LINE__); 

     int new_fd = P_accept(sock, (struct sockaddr *) &cli_addr, &clilen);
     if (-1 == new_fd) {
       bperr("%s[%d]:  accept failed\n", __FILE__, __LINE__);
       return false;
     }
     else {
       async_printf("%s[%d]:  about to read new connection\n", __FILE__, __LINE__); 
       //  do a (blocking) read so that we can get the pid associated with
       //  this connection.
       EventRecord pid_ev;
       if (! readEvent(new_fd, pid_ev)) {
         fprintf(stderr,"%s[%d]:  readEvent failed due to process termination\n", __FILE__, __LINE__);
         //bperr("%s[%d]:  readEvent failed\n", __FILE__, __LINE__);
         return false;
       }
       else {
         assert(pid_ev.type == evtNewConnection);
         ev = pid_ev;
         async_printf("%s[%d]:  new connection to %d\n", __FILE__, __LINE__, ev.proc->getPid());
         ev.what = new_fd;
       }
     }
  }

  ////////////////////////////////////////
  ////////////////////////////////////////
  ////////////////////////////////////////

  __LOCK;
  //  See if we have any processes reporting events:

  for (unsigned int j = 0; j < process_fds.size(); ++j) {
    if (-1 == process_fds[j].fd) continue;

    //  Possible race here, if mutator removes fd from set, but events
    //  are pending??

    if (FD_ISSET(process_fds[j].fd, &readSet)) { 

      // Read event
      EventRecord new_ev;

      if (! readEvent(process_fds[j].fd, new_ev)) { 
        //  This read can fail if the mutatee has exited.  Just note that this
        //  fd is no longer valid, and keep quiet.
        //if (process_fds[j].process->isTerminated()) {
        async_printf("%s[%d]:  read event failed\n", FILE__, __LINE__);
        if (1) {
          //  remove this process/fd from our vector
          async_printf("%s[%d]:  readEvent failed due to process termination\n", __FILE__, __LINE__);
          for (unsigned int k = j+1; k < process_fds.size(); ++k) {
            process_fds[j] = process_fds[k];
          }
          process_fds.pop_back();
          // and decrement counter so we examine this element (j) again
          j--;
        }
        else
          bperr("%s[%d]:  readEvent failed\n", __FILE__, __LINE__);
      }
      else { // ok

        if (ev.type != evtNullEvent) {
          ev = new_ev;
          ev.what = process_fds[j].fd;
        }
        else {
          // Queue up events if we got more than one.
          //  NOT SAFE???
          if (new_ev.type != evtNullEvent) {
            EventRecord qev = new_ev;
            qev.what = process_fds[j].fd;
            event_queue.push_back(qev);
          }
        }

      }
      
    }
  }
#if 0
  fprintf(stderr, "%s[%d]:  leaving waitNextEvent: events in queue:\n", __FILE__, __LINE__);
  for (unsigned int r = 0; r < event_queue.size(); ++r) {
    fprintf(stderr, "\t%s\n", eventType2str(event_queue[r].type));
  }
#endif
  async_printf("%s[%d]: leaving waitnextEvent\n",  FILE__, __LINE__);

  __UNLOCK;
  return true;
}

//  threadExitWrapper exists to ensure that callbacks are called before
//  the thread is deleted.  Maybe there's a better way....

void threadDeleteWrapper(BPatch_process *p, BPatch_thread *t)
{
  p->deleteBPThread(t);
}

void threadExitWrapper(BPatch_process *p, BPatch_thread *t, 
                       pdvector<AsyncThreadEventCallback *> *cbs_ptr)
{

  pdvector<AsyncThreadEventCallback *> &cbs = *cbs_ptr;
  for (unsigned int i = 0; i < cbs.size(); ++i) {
    AsyncThreadEventCallback &cb = * ((AsyncThreadEventCallback *) cbs[i]);
    cb(p,t);
  }
  threadDeleteWrapper(p,t);
}

bool BPatch_asyncEventHandler::handleEventLocked(EventRecord &ev)
{
   if ((ev.type != evtNewConnection) && (ev.type != evtNullEvent))
     async_printf("%s[%d]:  inside handleEvent, got %s\n", 
           __FILE__, __LINE__, eventType2str(ev.type));

   int event_fd = -1;
   BPatch_process *appProc = NULL;
   unsigned int j;
   //  Go through our process list and find the appropriate record

   for (j = 0; j < process_fds.size(); ++j) {
      if (!process_fds[j].process) {
        fprintf(stderr, "%s[%d]:  invalid process record!\n", __FILE__, __LINE__);
        continue;
      }
      int process_pid = process_fds[j].process->getPid();
      if (process_pid == ev.proc->getPid()) {
         event_fd = process_fds[j].fd;
         appProc = process_fds[j].process; 
         break;
      }
   }
   

   if (!appProc) {
     if (ev.type == evtNullEvent) return true; 
     //fprintf(stderr, "%s[%d]:  ERROR:  Got event %s for pid %d, but no proc, out of %d procs\n",
      //     __FILE__, __LINE__, eventType2str(ev.type), ev.proc->getPid(),process_fds.size());
     //for (unsigned int k = 0; k < process_fds.size(); ++k) {
     //  fprintf(stderr, "\thave process %d\n", process_fds[k].process->getPid());
    // }
     //  This can happen if we received a connect packet before the BPatch_process has
     //  been created.  Shove it on the front of the queue.
     pdvector<EventRecord> temp;
     for (unsigned int i = 0; i < event_queue.size(); ++i) {
       temp.push_back(event_queue[i]);
     }
     event_queue.push_back(ev);
     for (unsigned int i = 0; i < temp.size(); ++i) {
       event_queue.push_back(temp[i]);
     }
     
     return true;
   }

   async_printf("%s[%d]:  handling event type %s\n", FILE__, __LINE__,
                eventType2str(ev.type));

   switch(ev.type) {
     case evtNullEvent:
       return true;
     case evtNewConnection: 
     {
       //  add this fd to the pair.
       //  this fd will then be watched by select for new events.

//       assert(event_fd == -1);
       if (event_fd != -1)
         fprintf(stderr, "%s[%d]:  WARNING:  event fd for process %d is %d (not -1)\n",
                 FILE__, __LINE__, process_fds[j].process->getPid(), event_fd);

       process_fds[j].fd = ev.what;

       async_printf("%s[%d]:  after handling new connection, we have\n", __FILE__, __LINE__);
       for (unsigned int t = 0; t < process_fds.size(); ++t) {
          async_printf("\tpid = %d, fd = %d\n", process_fds[t].process->getPid(), process_fds[t].fd);
       }
       return true;
     }

     case evtShutDown:
       return false;

     case evtThreadCreate:
     {
        //  Read details of new thread from fd 
        BPatch_newThreadEventRecord call_rec;
        if (!readEvent(ev.fd/*fd*/, (void *) &call_rec, 
                       sizeof(BPatch_newThreadEventRecord))) {
           bperr("%s[%d]:  failed to read thread event call record\n",
                 __FILE__, __LINE__);
           return false;
        }

       BPatch_process *p = (BPatch_process *) appProc;
       unsigned long start_pc = (unsigned long) call_rec.start_pc;
       unsigned long stack_addr = (unsigned long) call_rec.stack_addr;
       bool thread_exists = (p->getThreadByIndex(call_rec.index) != NULL);

       //Create the new BPatch_thread object
       async_printf("%s[%d]:  before createOrUpdateBPThread: start_pc = %p," \
               "addr = %p, tid = %lu\n", 
               FILE__, __LINE__, (void *) start_pc, (void *) stack_addr, call_rec.tid);
       BPatch_thread *newthr = p->createOrUpdateBPThread( call_rec.lwp, 
                                                          (dynthread_t) call_rec.tid,
                                                          call_rec.index, stack_addr,
                                                          start_pc);

       assert(newthr);

       if (thread_exists) {
         return true;
       }

       pdvector<CallbackBase *> cbs;
       getCBManager()->dispenseCallbacksMatching(ev.type, cbs);
       for (unsigned int i = 0; i < cbs.size(); ++i) {
         AsyncThreadEventCallback &cb = * ((AsyncThreadEventCallback *) cbs[i]);
         async_printf("%s[%d]:  before executing thread create callback\n", FILE__, __LINE__);
         cb(p, newthr);
       }

       BPatch::bpatch->mutateeStatusChange = true;
       getSH()->signalEvent(evtThreadCreate);
       return true;
     }
     case evtThreadExit: 
     {
        BPatch_deleteThreadEventRecord rec;
        if (!readEvent(ev.fd, (void *) &rec, 
                       sizeof(BPatch_deleteThreadEventRecord))) {
           bperr("%s[%d]:  failed to read thread event call record\n",
                 __FILE__, __LINE__);
           return false;
        }

       unsigned index = (unsigned) rec.index;
       BPatch_thread *appThread = appProc->getThreadByIndex(index);

       //  this is a bit nasty:  since we need to ensure that the callbacks are 
       //  called before the thread is deleted, we use a special callback function,
       //  threadExitWrapper, specified above, which guarantees serialization.
       pdvector<CallbackBase *> cbs;
       pdvector<AsyncThreadEventCallback *> *cbs_copy = new pdvector<AsyncThreadEventCallback *>;
       getCBManager()->dispenseCallbacksMatching(ev.type, cbs);
       for (unsigned int i = 0; i < cbs.size(); ++i)
          cbs_copy->push_back((AsyncThreadEventCallback *)cbs[i]); 

       InternalThreadExitCallback *cb_ptr = new InternalThreadExitCallback(threadExitWrapper);
       InternalThreadExitCallback &cb = *cb_ptr;
       cb(appProc, appThread, cbs_copy); 

       BPatch::bpatch->mutateeStatusChange = true;
       getSH()->signalEvent(evtThreadExit);
       return true;
     }
     case evtDynamicCall:
     {
       //  Read auxilliary packet with dyn call info

       BPatch_dynamicCallRecord call_rec;
       if (!readEvent(ev.fd, (void *) &call_rec, 
                      sizeof(BPatch_dynamicCallRecord))) {
          bperr("%s[%d]:  failed to read dynamic call record\n",
                __FILE__, __LINE__);
          return false;
       }

       Address callsite_addr = (Address) call_rec.call_site_addr;
       Address func_addr = (Address) call_rec.call_target;

       //  find the record(s) for the pt that triggered this event
       //

       pdvector<dyncall_cb_record *> pts;
       for (unsigned int i = 0; i < dyn_pts.size(); ++i) {
         if (dyn_pts[i].pt->getAddress() == (void *) callsite_addr) {
            pts.push_back(&(dyn_pts[i]));
         }
       }

       if (!pts.size()) {
           bperr("%s[%d]:  failed to find async call point %p\n Valid Addrs:\n",
                 __FILE__, __LINE__, callsite_addr);
           for (unsigned int r = 0; r < dyn_pts.size(); ++r) {
             bperr("\t%p\n", (void *) dyn_pts[r].pt->getAddress());
           } 
           return false;
       }

       //  found the record(s), now find the function that was called
       int_function *f = appProc->llproc->findFuncByAddr(func_addr);
       if (!f) {
           bperr("%s[%d]:  failed to find BPatch_function\n",
                 __FILE__, __LINE__);
          return false;
       }

       //  find the BPatch_function...

       if (!appProc->func_map->defines(f)) {
           bperr("%s[%d]:  failed to find BPatch_function\n",
                 __FILE__, __LINE__);
           return false;
       }

       BPatch_function *bpf = appProc->func_map->get(f);

       if (!bpf) {
           bperr("%s[%d]:  failed to find BPatch_function\n",
                 __FILE__, __LINE__);
           return false;
       }

       //  call the callback(s) and we're done:
       //  actually, just register callback in mailbox
       //  (to be executed on primary thread) and we're done.
       for (unsigned int j = 0; j < pts.size(); ++j) {
         //assert(pts[j]->cb);
         //BPatchDynamicCallSiteCallback cb = pts[j]->cb;
         BPatch_point *pt = pts[j]->pt;
         pdvector<CallbackBase *> cbs;
         getCBManager()->dispenseCallbacksMatching(evtDynamicCall, cbs);
         for (unsigned int i = 0; i < cbs.size(); ++i) {
           DynamicCallsiteCallback &cb = * ((DynamicCallsiteCallback *) cbs[i]);
           cb(pt, bpf);
         }

       }

       return true;
     }
     case evtUserEvent:
     {
#if !defined (os_windows)
       assert(ev.info > 0);
       int *userbuf = new int[ev.info];

       //  Read auxilliary packet with user specifiedbuffer
       if (!readEvent(ev.what, (void *) userbuf, ev.info)) {
          bperr("%s[%d]:  failed to read user specified data\n",
                __FILE__, __LINE__);
          delete [] userbuf;
          return false;
       }
       
        pdvector<CallbackBase *> cbs;
        getCBManager()->dispenseCallbacksMatching(evtUserEvent, cbs);
        for (unsigned int i = 0; i < cbs.size(); ++i) {
          UserEventCallback &cb = *((UserEventCallback *) cbs[i]);
          cb(appProc, userbuf, ev.info);
        }

        delete [] userbuf;
#endif
       return true;
     } 
     default:
       bperr("%s[%d]:  request to handle unsupported event: %s\n", 
             __FILE__, __LINE__, eventType2str(ev.type));
       return false;
       break;
      
   }
   return true;
}

bool BPatch_asyncEventHandler::mutateeDetach(BPatch_process *p)
{
  //  find the function that will initiate the disconnection
  BPatch_Vector<BPatch_function *> funcs;
  if (!p->getImage()->findFunction("DYNINSTasyncDisconnect", funcs)
      || ! funcs.size() ) {
    bpfatal("%s[%d]:  could not find function: DYNINSTasyncDisconnect\n",
            __FILE__, __LINE__);
    return false;
  }
  if (funcs.size() > 1) {
    bperr("%s[%d]:  found %d varieties of function: DYNINSTasyncDisconnect\n",
          __FILE__, __LINE__, funcs.size());
  }

  //  The (int) argument to this function is our pid
  BPatch_Vector<BPatch_snippet *> args;
  args.push_back(new BPatch_constExpr(getpid()));
  BPatch_funcCallExpr disconnectcall(*funcs[0], args);

  //  Run the connect call as oneTimeCode
  if (!p->oneTimeCodeInt(disconnectcall)) {
    bpfatal("%s[%d]:  failed to disconnect mutatee to async handler\n", 
            __FILE__, __LINE__);
    return false;
  }

  return true;
}

bool BPatch_asyncEventHandler::cleanUpTerminatedProcs()
{
  bool ret = false;
  //  iterate from end of vector in case we need to use erase()
  for (int i = (int) process_fds.size() -1; i >= 0; i--) {
    if (process_fds[i].process->llproc->status() == exited) {
    //  fprintf(stderr, "%s[%d]:  Process %d has terminated, cleaning up\n", __FILE__, __LINE__, process_fds[i].process->getPid());
      process_fds.erase(i,i);
      ret = true;
    }
  }
  return ret;
}

eventType rt2EventType(rtBPatch_asyncEventType t)
{       
  switch(t) {
    case rtBPatch_nullEvent: return evtNullEvent;
    case rtBPatch_internalShutDownEvent: return evtShutDown;
    case rtBPatch_newConnectionEvent: return evtNewConnection;
    case rtBPatch_threadCreateEvent: return evtThreadCreate;
    case rtBPatch_threadDestroyEvent: return evtThreadExit;
    case rtBPatch_dynamicCallEvent: return evtDynamicCall;
    case rtBPatch_userEvent: return evtUserEvent;
    default:
    fprintf(stderr, "%s[%d], invalid conversion\n", __FILE__, __LINE__);
  }
  return evtUndefined;
}         


bool BPatch_asyncEventHandler::readEvent(PDSOCKET fd, EventRecord &ev)
{
#if defined (os_windows)
   assert(0);
   return false;
#else 
  rtBPatch_asyncEventRecord rt_ev;
  if (!readEvent(fd, &rt_ev, sizeof(rtBPatch_asyncEventRecord))) {
    async_printf("%s[%d]:  read failed\n", FILE__, __LINE__);
    return false;
  }
  ev.proc = process::findProcess(rt_ev.pid);
  assert (ev.proc);
  ev.what = rt_ev.event_fd;
  ev.fd = fd;
  ev.type = rt2EventType(rt_ev.type);
  ev.info = rt_ev.size;
  async_printf("%s[%d]: read event, proc = %d, fd = %d\n", FILE__, __LINE__,
          ev.proc->getPid(), ev.fd);
  return true;
#endif
}

#if defined(os_windows)
bool BPatch_asyncEventHandler::readEvent(PDSOCKET fd, void *ev, ssize_t sz)
{
  ssize_t bytes_read = 0;

  bytes_read = recv( fd, (char *)ev, sz, 0 );

  if ( PDSOCKET_ERROR == bytes_read ) {
    fprintf(stderr, "%s[%d]:  read failed: %s:%d\n", __FILE__, __LINE__,
            strerror(errno), errno);
    return false;
  }

  if (0 == bytes_read) {
    //  fd closed on other end (most likely)
    //bperr("%s[%d]:  cannot read, fd is closed\n", __FILE__, __LINE__);
    return false;
  }

  if (bytes_read != sz) {
    bperr("%s[%d]:  read wrong number of bytes!\n", __FILE__, __LINE__);
    bperr("FIXME:  Need better logic to handle incomplete reads\n");
    return false;
  }

  fprintf(stderr, "%s[%d] done recv\n", __FILE__, __LINE__);
  return true;
}

#else
bool BPatch_asyncEventHandler::readEvent(PDSOCKET fd, void *ev, ssize_t sz)
{
  ssize_t bytes_read = 0;
try_again:
  bytes_read = read(fd, ev, sz);

  if ( (ssize_t)-1 == bytes_read ) {
    if (errno == EAGAIN || errno == EINTR) 
       goto try_again;

    fprintf(stderr, "%s[%d]:  read failed: %s:%d\n", __FILE__, __LINE__,
            strerror(errno), errno);
    return false;
  }

  if (0 == bytes_read) {
    //  fd closed on other end (most likely)
    //bperr("%s[%d]:  cannot read, fd is closed\n", __FILE__, __LINE__);
    return false;
  }
  if (bytes_read != sz) {
    bperr("%s[%d]:  read wrong number of bytes! %d, not %d\n", 
          __FILE__, __LINE__, bytes_read, sz);
    bperr("FIXME:  Need better logic to handle incomplete reads\n");
    return false;
  }

  return true;
}
#endif


#ifndef CASE_RETURN_STR
#define CASE_RETURN_STR(x) case x: return #x
#endif

const char *asyncEventType2Str(BPatch_asyncEventType ev) {
  switch(ev) {
  CASE_RETURN_STR(BPatch_nullEvent);
  CASE_RETURN_STR(BPatch_newConnectionEvent);
  CASE_RETURN_STR(BPatch_internalShutDownEvent);
  CASE_RETURN_STR(BPatch_threadCreateEvent);
  CASE_RETURN_STR(BPatch_threadDestroyEvent);
  CASE_RETURN_STR(BPatch_dynamicCallEvent);
  default:
  return "BadEventType";
  }
}

bool BPatch_asyncEventHandler::startupThread()
{
  if (!isRunning()) {
    if (!createThread()) {
      fprintf(stderr, "%s[%d]:  failed to create thread\n", __FILE__, __LINE__);
      return false;
    }
  }
  return true;
}
