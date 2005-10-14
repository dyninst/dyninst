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


#ifndef __BPATCH_ASYNC_EVENT_HANDLER_H__
#define __BPATCH_ASYNC_EVENT_HANDLER_H__

#if defined (os_osf)
//#ifndef _XOPEN_SOURCE
//#define _XOPEN_SOURCE 500
#include <standards.h>
//#endif
//#ifndef _XOPEN_SOURCE_EXTENDED
//#error
//#endif
#endif

#include <errno.h>
#include <BPatch_eventLock.h>
#include <BPatch.h>
#include <BPatch_process.h>
#include <BPatch_image.h>
#include <BPatch_Vector.h>
#include "common/h/Pair.h"
#include "common/h/Vector.h"
#include "dyninstAPI_RT/h/dyninstAPI_RT.h" // for BPatch_asyncEventType
                                           //  and BPatch_asyncEventRecord
#if defined(os_windows)

#include <winsock2.h>
#define ssize_t int
typedef SOCKET PDSOCKET;
#define DYNINST_ASYNC_PORT 28003
#define CLOSEPDSOCKET(s) closesocket(s)
#define PDSOCKET_ERRNO WSAGetLastError()
#define INVALID_PDSOCKET (INVALID_SOCKET)
#define PDSOCKET_ERROR SOCKET_ERROR
#define SOCKET_TYPE PF_INET
#define THREAD_RETURN void
#define DO_THREAD_RETURN return
#define SOCKLEN_T unsigned int

#else
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
typedef int PDSOCKET;

#define CLOSEPDSOCKET(s) P_close(s)
#define PDSOCKET_ERRNO errno
#define INVALID_PDSOCKET (-1)
#define PDSOCKET_ERROR (-1)
#define SOCKET_TYPE PF_UNIX
#define THREAD_RETURN void *
#define DO_THREAD_RETURN return NULL
#if defined(os_osf)
#define SOCKLEN_T size_t 
#elif defined(os_irix)
#define SOCKLEN_T std::size_t 
#else
#define SOCKLEN_T socklen_t 
#endif
#endif

typedef struct {
  unsigned int pid;
  BPatch_asyncEventType type;
  unsigned int event_fd;
  unsigned int size;
} BPatch_asyncEventRecord;

class BPatch_eventMailbox {
  typedef struct {
    BPatch_asyncEventType type;
    void *cb;
    void *arg1;
    void *arg2;
    void *arg3;
    void *arg4;
    void *arg5;
    void *arg6;
  } mb_callback_t;
  pdvector<mb_callback_t> cbs;

  public:
  BPatch_eventMailbox() {}
  ~BPatch_eventMailbox() {}

  bool executeUserCallbacks();
  bool registerCallback(BPatch_asyncEventType type,
                        BPatch_process *p, dynthread_t tid,
                        int lwp, unsigned index, 
                        unsigned long stack_addr,
                        unsigned long start_pc);
  bool registerCallback(BPatch_process *proc,
                        void *buf, unsigned int bufsize);
  bool registerCallback(BPatchDynamicCallSiteCallback _cb,
                        BPatch_point *p, BPatch_function *f);
  bool executeOrRegisterCallback(BPatchErrorCallback _cb,
                                 BPatchErrorLevel lvl,
                                 int number, const char *params);
  bool executeOrRegisterCallback(BPatch_asyncEventType type,
                                 BPatch_process *p, unsigned index);
  bool executeOrRegisterCallback(BPatchDynLibraryCallback _cb,
                                 BPatch_process * proc,
                                 BPatch_module * mod,
                                 bool load);
  bool executeOrRegisterCallback(BPatchForkCallback _cb,
                                 BPatch_asyncEventType t,
                                 BPatch_process * parent,
                                 BPatch_process * child);
  bool executeOrRegisterCallback(BPatchExecCallback _cb,
                                 BPatch_process * proc);
  bool executeOrRegisterCallback(BPatchExitCallback _cb,
                                 BPatch_process * proc,
                                 BPatch_exitType exit_type);
  bool executeOrRegisterCallback(BPatchSignalCallback _cb,
                                 BPatch_process * proc,
                                 int signum);
  bool executeOrRegisterCallback(BPatchOneTimeCodeCallback _cb,
                                 BPatch_process * proc,
                                 void * user_data,
                                 void * return_value);
  void triggerThreadCallback(BPatch_asyncEventType type,
                             BPatch_process *p, unsigned index);
};

typedef struct {
    BPatchDynamicCallSiteCallback cb;
    void *handle;
    BPatch_point *pt;
} dyncall_cb_record;

typedef struct {
    BPatchUserEventCallback cb;
} user_event_cb_record;

typedef struct {
    pdvector<BPatch_function *> *mutatee_side_cbs;
    pdvector<BPatchSnippetHandle *> *handles;
} thread_event_cb_record;

typedef struct {
  BPatch_process *process;
  int fd;
} process_record;

const char *asyncEventType2Str(BPatch_asyncEventType evtype); 

#ifdef DYNINST_CLASS_NAME
#undef DYNINST_CLASS_NAME
#endif
#define DYNINST_CLASS_NAME BPatch_asyncEventHandler

class BPatch_asyncEventHandler : public BPatch_eventLock {
  friend THREAD_RETURN asyncHandlerWrapper(void *);
  friend class BPatch;  // only BPatch constructs & does init
  friend class BPatch_eventMailbox;
  public:
    //  BPatch_asyncEventHandler::connectToProcess()
    //  Tells the async event handler that there is a new process
    //  to listen for.
    bool connectToProcess(BPatch_process *p);

    //  BPatch_asyncEventHandler::detachFromProcess()
    //  stop paying attention to events from specified process
    bool detachFromProcess(BPatch_process *p);

    //  BPatch_asyncEventHandler::registerDynamicCallCallback()
    //  Specify a function to be called when a function is called
    //  at the specified dynamic callsite
    void *registerDynamicCallCallback(BPatchDynamicCallSiteCallback cb,
                                      BPatch_point *pt);

    //  BPatch_asyncEventHandler::removeDynamicCallCallback()
    //  Using handle obtained from registerDynamicCallCallback, stop
    //  monitoring that particular callsite with that particular
    //  callback.
    bool removeDynamicCallCallback(void *handle);

    //  BPatch_asyncEventHandler::registerThreadEventCallback
    //  Specify a function to be called when a thread event of 
    //  <type> occurs.
    bool registerThreadEventCallback(BPatch_asyncEventType type,
                                     BPatchAsyncThreadEventCallback cb);

    //  BPatch_asyncEventHandler::registerThreadEventCallback()
    //  Specify a function to be called in the mutatee when a
    //  the event of <type> occurs.
    bool registerThreadEventCallback(BPatch_process *proc,
                                     BPatch_asyncEventType type,
                                     BPatch_function *cb);

    //  BPatch_asyncEventHandler::removeThreadEventCallback()
    //  
    bool removeThreadEventCallback(BPatch_asyncEventType type,
                                   BPatchAsyncThreadEventCallback cb);

    //  BPatch_asyncEventHandler::removeThreadEventCallback()
    //  
    bool removeThreadEventCallback(BPatch_process *proc,
                                   BPatch_asyncEventType type,
                                   BPatch_function *cb);


    //  BPatch_asyncEventHandler::registerUserEventCallback
    //  Specify a function to be called when a user event  
    bool registerUserEventCallback(BPatchUserEventCallback cb);


    //  BPatch_asyncEventHandler::removeUserEventCallback()
    //  
    bool removeUserEventCallback(BPatchUserEventCallback cb);

    bool startupThread();
    //  BPatch_asyncEventHandler::removeThreadEventCallback()
#if !defined (os_windows)
    //  NOTE:  right now stop()/pause() is expensive in terms of waiting time
    //  (requires select loop to wake up on its own before pause request
    //  is encountered).  Ultimately we should not need this function.
    //  It seems to be a useful debug mechanism however, (ie. something is
    //  not working, let's see if its because the other thread is somehow
    //  interfering ).

    //  BPatch_asyncEventHandler::pause()
    //  pause the event handling thread
    bool pause();

    //  BPatch_asyncEventHandler::unpause()
    //  unpause the event handling thread (after pause)
    bool unpause();

    //  BPatch_asyncEventHandler::stop()
    //  Stop the event handling thread (thread will fully exit)
    bool stop();

    //  BPatch_asyncEventHandler::resume()
    //  Resume the event handling thread (re-create) 
    bool resume();
#endif

  private: 
    BPatch_asyncEventHandler();
    bool initialize();  //  want to catch init errors, so we do most init here
    virtual ~BPatch_asyncEventHandler();

    //  BPatch_asyncEventHandler::createThread()
    //  platform indep. way of starting the thread.
    bool createThread();

    //  BPatch_asyncEventHandler::main()
    //  The "main" function for the async event thread.
    void main();

    //  BPatch_asyncEventHandler::shutDown()
    //  Sets a flag that the async thread will check during its next iteration.
    //  When set, the mutatee will shut itself down.
    bool shutDown();

#if !defined (os_windows)
    //  a mutex/cond for doing simple comms between main thread
    //  and async thread.  Used in pause/stop _only_.  Possibly not
    //  a good idea to begin with.
    pthread_mutex_t pause_mutex;
    bool paused;
    bool stop_req;
    pthread_cond_t pause_cond;

    //  BPatch_asyncEventHandler::handlePauseRequest()
    //  This only exists to keep the main event loop uncluttered
    void handlePauseRequest();

    //  BPatch_asyncEventHandler::handleStopRequest()
    //  This only exists to keep the main event loop uncluttered
    void handleStopRequest();
#endif

    //  BPatch_asyncEventHandler::waitNextEvent()
    //  Wait for the next event to come from a mutatee.  Essentially 
    //  a big call to select().
    bool waitNextEvent(BPatch_asyncEventRecord &ev);

    //  BPatch_asyncEventHandler::handleEvent()
    //  called after waitNextEvent, obtains global lock and handles event.
    //  Since event handling needs to be a locked operation (esp. if it 
    //  requires accessing lower level dyninst data structures), this is
    //  where it should be done.
    bool handleEvent(BPatch_asyncEventRecord &ev)
         { LOCK_FUNCTION(bool, handleEventLocked, (ev)); }
    bool handleEventLocked(BPatch_asyncEventRecord &ev);

    //  BPatch_asyncEventHandler::readEvent()
    //  Reads from the async fd connection to the mutatee
    static bool readEvent(PDSOCKET fd, void *ev, ssize_t sz);
    static bool readEvent(PDSOCKET fd, BPatch_asyncEventRecord &ev);

    //  BPatch_asyncEventHandler::mutateeDetach()
    //  use oneTimeCode to call a function in the mutatee to handle
    //  closing of the comms socket.

    bool mutateeDetach(BPatch_process *p);

    //  BPatch_asyncEventHandler::cleanUpTerminatedProcs()
    //  clean up any references to terminated processes in our lists
    //  (including any user specified callbacks).
    bool cleanUpTerminatedProcs();
    //  BPatch_asyncEventHandler::instrumentThreadEvent
    //  Associates a function in the thread library with a BPatch_asyncEventType
    BPatchSnippetHandle *instrumentThreadEvent(BPatch_process *process,
                                               BPatch_asyncEventType t,
                                               BPatch_function *f);

    //  These vars are only modified as part of init (before/while threads are
    //  created) so we do not need to worry about locking them:
    PDSOCKET sock;
    bool shutDownFlag;
#if defined (os_windows)
    unsigned int listen_port;
    unsigned long handler_thread;
#else
    pthread_t handler_thread;
#endif

    //  The rest:  Data in this class that is not exclusively set during init
    //   will have to be locked.  
    pdvector<process_record> process_fds;

    //  dyn_pts holds BPatch_point and callback info for dynamic call events.
    //  XXX -- since this is linearly searched for each dynamic call event
    //  it could get prohibitively slow in cases where there are a lot of
    //  requests to monitor dynamic calls.  Maybe a (somewhat more complex)
    //  data structure is necessary?
    pdvector<dyncall_cb_record> dyn_pts;

    pdvector<BPatchAsyncThreadEventCallback> *getCBsForType(BPatch_asyncEventType t);
    pdvector<BPatchAsyncThreadEventCallback> thread_create_cbs;
    pdvector<BPatchAsyncThreadEventCallback> thread_destroy_cbs;
    pdvector<user_event_cb_record> user_event_cbs;

    BPatchSnippetHandle *reportThreadCreateHandle;
    BPatchSnippetHandle *reportThreadDestroyHandle;
    bool isRunning;
};



#endif // __BPATCH_EVENT_HANDLER_H__
