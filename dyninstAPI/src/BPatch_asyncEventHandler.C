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
#include "BPatch_function.h"
#include "BPatch_point.h"
#include "util.h"
#include "process.h"

class BPatch_eventMailbox;
extern BPatch_eventMailbox *event_mailbox;
extern unsigned long primary_thread_id;

//extern MUTEX_TYPE global_mutex; // see BPatch_eventLock.h
//extern bool mutex_created = false;

bool BPatch_eventMailbox::executeUserCallbacks()
{
    //fprintf(stderr, "%s[%d]:  executing %d callbacks\n", __FILE__, __LINE__, cbs.size());
    bool err = false;

    for (unsigned int i = 0; i < cbs.size(); ++i) {
      switch(cbs[i].type) {
        case BPatch_dynamicCallEvent:
        {
          BPatchDynamicCallSiteCallback cb = (BPatchDynamicCallSiteCallback) cbs[i].cb;
          BPatch_point *p = (BPatch_point *) cbs[i].arg1;
          BPatch_function *f = (BPatch_function *) cbs[i].arg2;

          if (!p || !cb || !f) {
            err = true;
            fprintf(stderr, "%s[%d]:  corrupt callback record\n", __FILE__, __LINE__);
          }
          else
            (cb)(p,f);

          break;
        }
        break;
        case BPatch_threadCreateEvent:
        case BPatch_threadDestroyEvent:
        case BPatch_threadStartEvent:
        case BPatch_threadStopEvent:
        {
          BPatchAsyncThreadEventCallback cb = (BPatchAsyncThreadEventCallback) cbs[i].cb;
          BPatch_thread *t = (BPatch_thread *) cbs[i].arg1;
          unsigned long tid = (unsigned long) cbs[i].arg2;

          if (!t || !cb) {
            err = true;
            fprintf(stderr, "%s[%d]:  corrupt callback record\n", __FILE__, __LINE__);
          }
          else
            (cb)(t,tid);

          break;
        }
        case BPatch_userEvent:
        {
          BPatchUserEventCallback cb = (BPatchUserEventCallback) cbs[i].cb;
          void *buf = (void *) cbs[i].arg1;
          unsigned int bufsize = (unsigned int) cbs[i].arg2;

          if (!buf || !cb) {
            err = true;
            fprintf(stderr, "%s[%d]:  corrupt callback record\n", __FILE__, __LINE__);
          }
          else {
            (cb)(buf,bufsize);
            delete [] (int *) buf;
          }

          break;
        }
        case BPatch_errorEvent:
        {
          BPatchErrorCallback cb = (BPatchErrorCallback) cbs[i].cb;
          BPatchErrorLevel lvl = (BPatchErrorLevel)((unsigned long)cbs[i].arg1);
          int number = (unsigned long) cbs[i].arg2;
          const char *params = (const char *) cbs[i].arg3;
          if (!cb) {
            err = true;
            fprintf(stderr, "%s[%d]:  corrupt callback record\n", __FILE__, __LINE__);
          }
          else
            (cb)(lvl, number, &params);

           //  params is allocated upon registration of this cb
          if (params) delete [] (const_cast<char *>(params));
          break;

        }
        case BPatch_dynLibraryEvent:
        {
          BPatchDynLibraryCallback cb = (BPatchDynLibraryCallback) cbs[i].cb;
          BPatch_process *proc = (BPatch_process *) cbs[i].arg1;
          BPatch_module *mod = (BPatch_module *) cbs[i].arg2;
          bool load = (bool) cbs[i].arg3;
          if (!proc || !mod || !cb) {
            err = true;
            fprintf(stderr, "%s[%d]:  corrupt callback record\n", __FILE__, __LINE__);
          }
          else
          {
             assert(proc->threads.size() > 0);
             (cb)(proc->threads[0], mod,load);
          }
          break;

        }
        case BPatch_postForkEvent:
          if (!cbs[i].arg2) {
            err = true;
            fprintf(stderr, "%s[%d]:  corrupt callback record\n", __FILE__, __LINE__);
            break;
          }
        case BPatch_preForkEvent:
        {
          BPatchForkCallback cb = (BPatchForkCallback) cbs[i].cb;
          BPatch_process *parent = (BPatch_process *) cbs[i].arg1;
          BPatch_process *child = (BPatch_process *) cbs[i].arg2;
          if (!parent || !cb) {
            err = true;
            fprintf(stderr, "%s[%d]:  corrupt callback record\n", __FILE__, __LINE__);
          }
          else
          {
             assert(parent->threads.size() > 0);
             assert(child->threads.size() > 0);
             (cb)(parent->threads[0], child->threads[0]);
          }

          break;

        }
        case BPatch_execEvent:
        {
          BPatchExecCallback cb = (BPatchExecCallback) cbs[i].cb;
          BPatch_process *proc = (BPatch_process *) cbs[i].arg1;
          if (!proc || !cb) {
            err = true;
            fprintf(stderr, "%s[%d]:  corrupt callback record\n", __FILE__, __LINE__);
          }
          else
          {
            assert(proc->threads.size() > 0);
            (cb)(proc->threads[0]);
          }

          break;

        }

        case BPatch_exitEvent:
        {
          BPatchExitCallback cb = (BPatchExitCallback) cbs[i].cb;
          BPatch_process *proc = (BPatch_process *) cbs[i].arg1;
          BPatch_exitType exit_type = (BPatch_exitType) ((unsigned long)cbs[i].arg2);
          if (!proc || !cb) {
            err = true;
            fprintf(stderr, "%s[%d]:  corrupt callback record\n", __FILE__, __LINE__);
          }
          else
          {
            assert(proc->threads.size() > 0);
            (cb)(proc->threads[0], exit_type);
          }
          break;

        }
        case BPatch_signalEvent:
        {
          BPatchSignalCallback cb = (BPatchSignalCallback) cbs[i].cb;
          BPatch_process *proc = (BPatch_process *) cbs[i].arg1;
          int signum = (unsigned long) cbs[i].arg2;
          if (!proc || !signum || !cb) {
            err = true;
            fprintf(stderr, "%s[%d]:  corrupt callback record\n", __FILE__, __LINE__);
          }
          else
          {
            assert(proc->threads.size() > 0);
            (cb)(proc->threads[0],signum);
          }
          break;

        }
        case BPatch_oneTimeCodeEvent:
        {
          BPatchOneTimeCodeCallback cb = (BPatchOneTimeCodeCallback) cbs[i].cb;
          BPatch_process *proc = (BPatch_process *) cbs[i].arg1;
          void *userData = cbs[i].arg2;
          void *returnValue = cbs[i].arg3;
          if (!proc || !userData || !returnValue || !cb) {
            err = true;
            fprintf(stderr, "%s[%d]:  corrupt callback record\n", __FILE__, __LINE__);
          }
          else
          {
            assert(proc->threads.size() > 0);
            (cb)(proc->threads[0], userData, returnValue);
          }

          break;

        }
        default:
          fprintf(stderr, "%s[%d]:  invalid callback record\n", __FILE__, __LINE__);
          err = true;
      }
    }
    cbs.clear();
    return !err;
}


bool BPatch_eventMailbox::registerCallback(BPatch_asyncEventType type,
                                           BPatchAsyncThreadEventCallback _cb,
                                           BPatch_process *p, unsigned long tid)
{
    mb_callback_t cb;
    cb.type = type;
    cb.cb = (void *) _cb;
    cb.arg1 = (void *) p;
    cb.arg2 = (void *) tid;
    cbs.push_back(cb);
    return true;
}

bool BPatch_eventMailbox::registerCallback(BPatchUserEventCallback _cb,
                                           void *buf, unsigned int bufsize)
{
    mb_callback_t cb;
    cb.type = BPatch_userEvent;
    cb.cb = (void *) _cb;
    cb.arg1 = (void *) buf;
    cb.arg2 = (void *) bufsize;
    cbs.push_back(cb);
    return true;
}
bool BPatch_eventMailbox::registerCallback(BPatchDynamicCallSiteCallback _cb,
                                           BPatch_point *p, BPatch_function *f)
{
    mb_callback_t cb;
    cb.type = BPatch_dynamicCallEvent;
    cb.cb = (void *) _cb;
    cb.arg1 = (void *) p;
    cb.arg2 = (void *) f;
    cbs.push_back(cb);
    return true;
}

bool BPatch_eventMailbox::executeOrRegisterCallback(BPatchErrorCallback _cb,
                                                    BPatchErrorLevel lvl, 
                                                    int number, 
                                                    const char *params)
{
    unsigned long tid = BPatch::bpatch->threadID();
    if (tid == primary_thread_id) {
      (_cb)(lvl, number, &params);
      return true;
    }

    char *buf = new char[strlen(params) + 1];
    strcpy(buf, params);

    mb_callback_t cb;
    cb.type = BPatch_errorEvent;
    cb.cb = (void *) _cb;
    cb.arg1 = (void *) lvl;
    cb.arg2 = (void *) number;
    cb.arg3 = (void *) buf;
    cbs.push_back(cb);
    return true;
}
bool BPatch_eventMailbox::executeOrRegisterCallback(BPatchDynLibraryCallback _cb,
                                                    BPatch_process *proc,
                                                    BPatch_module * mod,
                                                    bool load)
{
    unsigned long tid = BPatch::bpatch->threadID();
    if (tid == primary_thread_id) {
      assert(proc->threads.size() > 0);
      (_cb)(proc->threads[0], mod,load);
      return true;
    }

    mb_callback_t cb;
    cb.type = BPatch_dynLibraryEvent;
    cb.cb = (void *) _cb;
    cb.arg1 = (void *) proc;
    cb.arg2 = (void *) mod;
    cb.arg3 = (void *) load;
    cbs.push_back(cb);
    return true;
}
bool BPatch_eventMailbox::executeOrRegisterCallback(BPatchForkCallback _cb,
                                                    BPatch_asyncEventType t,
                                                    BPatch_process * parent,
                                                    BPatch_process * child)
{
    unsigned long tid = BPatch::bpatch->threadID();
    if (tid == primary_thread_id) {
      assert(parent->threads.size() > 0);
      if (child)
      {
         assert(child->threads.size() > 0);
         assert(parent->threads.size() > 0);
         (_cb)(parent->threads[0], child->threads[0]);
      }
      else
      {
         assert(parent->threads.size() > 0);
         (_cb)(parent->threads[0], NULL);
      }
      return true;
    }

    assert ( (t==BPatch_preForkEvent) || (t==BPatch_postForkEvent));
    mb_callback_t cb;
    cb.type = t;
    cb.cb = (void *) _cb;
    cb.arg1 = (void *) parent;
    cb.arg2 = (void *) child;
    cbs.push_back(cb);
    return true;
}

bool BPatch_eventMailbox::executeOrRegisterCallback(BPatchExecCallback _cb,
                                                    BPatch_process * proc)
{
    unsigned long tid = BPatch::bpatch->threadID();
    if (tid == primary_thread_id) {
      assert(proc->threads.size() > 0);
      (_cb)(proc->threads[0]);
      return true;
    }

    mb_callback_t cb;
    cb.type = BPatch_execEvent;
    cb.cb = (void *) _cb;
    cb.arg1 = (void *) proc;
    cbs.push_back(cb);
    return true;
}

bool BPatch_eventMailbox::executeOrRegisterCallback(BPatchExitCallback _cb,
                                                    BPatch_process * proc,
                                                    BPatch_exitType exit_type)
{
    unsigned long tid = BPatch::bpatch->threadID();
    if (tid == primary_thread_id) {
      assert(proc->threads.size() > 0);
      (_cb)(proc->threads[0], exit_type);
      return true;
    }

    mb_callback_t cb;
    cb.type = BPatch_exitEvent;
    cb.cb = (void *) _cb;
    cb.arg1 = (void *) proc;
    cb.arg2 = (void *) exit_type;
    cbs.push_back(cb);
    return true;
}

bool BPatch_eventMailbox::executeOrRegisterCallback(BPatchSignalCallback _cb,
                                                    BPatch_process * proc,
                                                    int signum)
{
    unsigned long tid = BPatch::bpatch->threadID();
    if (tid == primary_thread_id) {
      assert(proc->threads.size() > 0);
      (_cb)(proc->threads[0], signum);
      return true;
    }

    mb_callback_t cb;
    cb.type = BPatch_signalEvent;
    cb.cb = (void *) _cb;
    cb.arg1 = (void *) proc;
    cb.arg2 = (void *) signum;
    cbs.push_back(cb);
    return true;
}

bool BPatch_eventMailbox::executeOrRegisterCallback(BPatchOneTimeCodeCallback _cb,
                                                    BPatch_process * proc,
                                                    void * user_data,
                                                    void * return_value)
{
    unsigned long tid = BPatch::bpatch->threadID();
    if (tid == primary_thread_id) {
      assert(proc->threads.size() > 0);
      (_cb)(proc->threads[0], user_data, return_value);
      return true;
    }
    mb_callback_t cb;
    cb.type = BPatch_oneTimeCodeEvent;
    cb.cb = (void *) _cb;
    cb.arg1 = (void *) proc;
    cb.arg2 = (void *) user_data;
    cb.arg3 = (void *) return_value;
    cbs.push_back(cb);
    return true;
}




ThreadLibrary::ThreadLibrary(BPatch_process *proc, const char *libName) :
   threadModule(NULL),
   dyninst_rt(NULL),
   DYNINSTasyncThreadCreate(NULL),
   DYNINSTasyncThreadDestroy(NULL),
   DYNINSTasyncThreadStart(NULL),
   DYNINSTasyncThreadStop(NULL)
{
  const char *tmp_libname = libName;

  BPatch_image *appImage = proc->getImage();
  threadModule = appImage->findModuleInt(libName);
  if (!threadModule) {
    // exact match not found, try substrings (so we can specify libpthread
    // when we want a match on either libpthread.so, or libpthread.so.0, .a, etc)
    threadModule = appImage->findModuleInt(libName, true /* substring match */);
  }
  if (!threadModule) {
    //fprintf(stderr, "%s[%d]:  Thread module %s not found, assuming single threaded\n",
    //       __FILE__, __LINE__, libName);

#ifdef NOTDEF
    char buf[512];
    fprintf(stderr, "Available modules:\n");
    BPatch_Vector<BPatch_module *> *mods = appImage->getModules();
    for (unsigned int i = 0; i < mods->size(); ++i) {
      (*mods)[i]->getName(buf, 512);
      fprintf(stderr, "\t%s\n", buf);
    }
#endif
    tmp_libname = "<unavailable thread module>";
  }

  libname = new char [strlen(tmp_libname)+1];
  strcpy(libname, tmp_libname);

  //  find the dyninst RT Library
  const char *rtname = proc->llproc->dyninstRT_name.c_str();
  assert(rtname);

#if defined(os_windows)
  const char *short_rtname = strrchr(rtname, '\\') + 1; /*ignore slash*/
#else
  const char *short_rtname = strrchr(rtname, '/') + 1; /*ignore slash*/
#endif

  dyninst_rt = appImage->findModuleInt(short_rtname ? short_rtname : rtname);
  if (!dyninst_rt) {
    bpfatal("%s[%d]:  Cannot find dyninst rt lib: %s\n", __FILE__, __LINE__,
            short_rtname ? short_rtname : rtname);
    return;
  }

  // find the thread reporting functions in the RT lib
  BPatch_Vector<BPatch_function *> funcs;
  char *funcName;

  ////////
  funcName = "DYNINSTasyncThreadCreate";
  dyninst_rt->findFunction(funcName, funcs);
  if (!funcs.size()) {
    bperr("%s[%d]:  cannot find %s\n", __FILE__, __LINE__, funcName);
  }
  else {
    if (funcs.size() > 1) 
      bperr("%s[%d]:  WARN:  weird:  found %d %s\n", __FILE__, __LINE__,
            funcs.size(), funcName);
    DYNINSTasyncThreadCreate = funcs[0];
    funcs.clear();
  }
  
  ////////
  funcName = "DYNINSTasyncThreadDestroy";
  dyninst_rt->findFunction(funcName, funcs);
  if (!funcs.size()) {
    bperr("%s[%d]:  cannot find %s\n", __FILE__, __LINE__, funcName);
  }
  else {
    if (funcs.size() > 1)
      bperr("%s[%d]:  WARN:  weird:  found %d %s\n", __FILE__, __LINE__,
            funcs.size(), funcName);
    DYNINSTasyncThreadDestroy = funcs[0];
    funcs.clear();
  }

  ////////
  funcName = "DYNINSTasyncThreadStart";
  dyninst_rt->findFunction(funcName, funcs);
  if (!funcs.size()) {
    bperr("%s[%d]:  cannot find %s\n", __FILE__, __LINE__, funcName);
  }
  else {
    if (funcs.size() > 1)
      bperr("%s[%d]:  WARN:  weird:  found %d %s\n", __FILE__, __LINE__,
            funcs.size(), funcName);
    DYNINSTasyncThreadStart = funcs[0];
    funcs.clear();
  }

  ////////
  funcName = "DYNINSTasyncThreadStop";
  dyninst_rt->findFunction(funcName, funcs);
  if (!funcs.size()) {
    bperr("%s[%d]:  cannot find %s\n", __FILE__, __LINE__, funcName);
  }
  else {
    if (funcs.size() > 1)
      bperr("%s[%d]:  WARN:  weird:  found %d %s\n", __FILE__, __LINE__,
            funcs.size(), funcName);
    DYNINSTasyncThreadStop = funcs[0];
    funcs.clear();
  }

}

ThreadLibrary::~ThreadLibrary()
{
  delete [] libname;
}

bool ThreadLibrary::hasCapability(BPatch_asyncEventType t)
{
  if (!threadModule) return false;

  if (t == BPatch_userEvent) return true;

  BPatch_Vector<BPatch_function *> *funcs = funcsForType(t);
  if (!funcs) return false;

  return (funcs->size() > 0);
}

bool ThreadLibrary::addThreadEventFunction(BPatch_asyncEventType t, const char *name)
{
  if (!threadModule) {
    fprintf(stderr, "%s[%d]:  cannot add thread event %s to nonexistant threads lib\n",
            __FILE__, __LINE__, asyncEventType2Str(t));
    return false;
  }

  BPatch_Vector<BPatch_function *> *funcs = funcsForType(t);
  if (!funcs) {
  }

  BPatch_Vector<BPatch_function *> hits;
  if (!threadModule->findFunction(name, hits, false,false, true) || !hits.size()) {
    fprintf(stderr, "%s[%d]:  no matches for %s found in module %s\n",
            __FILE__, __LINE__, name, libname);
    return false;
  }
  //  as usual, warn if we got more than one hit.
  if (hits.size() > 1) {
    fprintf(stderr, "%s[%d]:  %d matches for %s found in module %s\n",
            __FILE__, __LINE__, hits.size(), name, libname);
  }

  //  add function(s) to event-specific vector
  for (unsigned int i = 0; i < hits.size(); ++i) {
    funcs->push_back(hits[i]);
  }

  //fprintf(stderr, "%s[%d]:  installed capability %s for Thread Lib %s\n",
  //       __FILE__, __LINE__, asyncEventType2Str(t), libname);
  return true;
}

unsigned int ThreadLibrary::numberOfFuncsForEvent(BPatch_asyncEventType t)
{
  BPatch_Vector<BPatch_function *> *funcs = funcsForType(t);
  if (!funcs) return 0;
  return funcs->size();
}

BPatch_function *ThreadLibrary::funcForEvent(BPatch_asyncEventType t, int index)
{
  BPatch_Vector<BPatch_function *> *funcs = funcsForType(t);
  if (!funcs) return NULL;
  if (index > (int)(funcs->size() - 1)) return NULL;
  return (*funcs)[index];
}

BPatch_function *ThreadLibrary::getDYNINSTreportFunc(BPatch_asyncEventType t)
{
  switch(t) {
  case BPatch_threadCreateEvent:  return DYNINSTasyncThreadCreate;
  case BPatch_threadDestroyEvent: return DYNINSTasyncThreadDestroy;
  case BPatch_threadStartEvent:   return DYNINSTasyncThreadStart;
  case BPatch_threadStopEvent:    return DYNINSTasyncThreadStop;
  default: break;
  }
  fprintf(stderr, "%s[%d]:  invalid type requested: %s\n",
          __FILE__, __LINE__, asyncEventType2Str(t));
  return NULL;
}
BPatch_Vector<BPatch_function *> *ThreadLibrary::funcsForType(BPatch_asyncEventType t)
{
  switch(t) {
  case BPatch_threadCreateEvent:  return &threadCreateFuncs;
  case BPatch_threadDestroyEvent: return &threadDestroyFuncs;
  case BPatch_threadStartEvent:   return &threadStartFuncs;
  case BPatch_threadStopEvent:    return &threadStopFuncs;
  default: break;
  }
  fprintf(stderr, "%s[%d]:  invalid type requested: %s\n",
          __FILE__, __LINE__, asyncEventType2Str(t));
  return NULL;
}

//  A wrapper for pthread_create, or its equivalent.

inline THREAD_RETURN  asyncHandlerWrapper(void *h)
{
  ((BPatch_asyncEventHandler * )h)->main();
  DO_THREAD_RETURN;
}

bool BPatch_asyncEventHandler::connectToProcess(BPatch_process *p)
{
  //fprintf(stderr, "%s[%d]:  enter ConnectToProcess %d\n", __FILE__, __LINE__,p->getPid());
  //  All we do here is add the process to the list of connected processes
  //  with a fd equal to -1, indicating the not-yet-connected state.
  //
  //  Then remotely execute code in the mutatee to initiate the connection.
  
  //  make sure that this process is not already known
  for (int i = (int) process_fds.size() -1 ; i >= 0; i--) {
    if ((p == process_fds[i].process) || (p->getPid() == process_fds[i].process->getPid())){
      //  If it is, delete the old record to prepare for the new one.
      //  This case can be encountered in the case of multiple process management
      //  when processes are created and terminated rapidly.
      //fprintf(stderr,"%s[%d]:  duplicate request to connect to process %d\n",
      //      __FILE__, __LINE__, p->getPid());
      ThreadLibrary *tlib = process_fds[i].threadlib;
      if (tlib) delete tlib;
      process_fds.erase(i,i);
      //return false;
    }
  } 

  ThreadLibrary *threadLib = newThreadLibrary(p);

  //  instrument thread reporting functions, as available:

  if (threadLib->hasCapability(BPatch_threadCreateEvent)) {
    BPatch_function *reportCreate;
    reportCreate = threadLib->getDYNINSTreportFunc(BPatch_threadCreateEvent);
    reportThreadCreateHandle = instrumentThreadEvent(p, threadLib, 
                                                     BPatch_threadCreateEvent, 
                                                     reportCreate);
  }

  if (threadLib->hasCapability(BPatch_threadDestroyEvent)) {
    BPatch_function *reportDestroy;
    reportDestroy = threadLib->getDYNINSTreportFunc(BPatch_threadDestroyEvent);
    reportThreadDestroyHandle = instrumentThreadEvent(p, threadLib,
                                                      BPatch_threadDestroyEvent,
                                                      reportDestroy);
  }

  if (threadLib->hasCapability(BPatch_threadStartEvent)) {
    BPatch_function *reportStart;
    reportStart = threadLib->getDYNINSTreportFunc(BPatch_threadStartEvent);
    reportThreadStartHandle = instrumentThreadEvent(p, threadLib,
                                                    BPatch_threadStartEvent, 
                                                    reportStart);
  }

  if (threadLib->hasCapability(BPatch_threadStopEvent)) {
    BPatch_function *reportStop;
    reportStop = threadLib->getDYNINSTreportFunc(BPatch_threadStopEvent);
    reportThreadStopHandle = instrumentThreadEvent(p, threadLib,
                                                   BPatch_threadStopEvent, 
                                                   reportStop);
  }

  //  add process to list
  process_record newp;
  newp.process = p;
  newp.fd = -1;
  newp.threadlib = threadLib;
  process_fds.push_back(newp);

  //  get mutatee to initiate connection

  //  find the runtime library module
  BPatch_module *dyninstLib = threadLib->getDyninstRT();
  assert(dyninstLib);
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
#endif

  //  find the function that will initiate the connection
  BPatch_Vector<BPatch_function *> funcs;
  if (!dyninstLib->findFunction("DYNINSTasyncConnect", funcs, true, true, true)
      || ! funcs.size() ) {
    bpfatal("%s[%d]:  could not find function: DYNINSTasyncConnect\n",
            __FILE__, __LINE__);
    return false;
  }
  if (funcs.size() > 1) {
    bperr("%s[%d]:  found %d varieties of function: DYNINSTasyncConnect\n",
          __FILE__, __LINE__, funcs.size());
  }

  //  The (int) argument to this function is our pid
  BPatch_Vector<BPatch_snippet *> args;
#if !defined(os_windows)
  args.push_back(new BPatch_constExpr(getpid()));
#endif
  BPatch_funcCallExpr connectcall(*funcs[0], args);
 
#if !defined (os_osf) && !defined (os_windows)
  //  Run the connect call as oneTimeCode
  if (!p->oneTimeCode(connectcall)) {
    fprintf(stderr,"%s[%d]:  failed to connect mutatee to async handler\n", __FILE__, __LINE__); 
    return false;
  }
#endif
  
  //fprintf(stderr, "%s[%d]:  leaveConnectToProcess %d\n", __FILE__, __LINE__,p->getPid());
  return true;
}

bool BPatch_asyncEventHandler::detachFromProcess(BPatch_process *p)
{
  //  find the fd for this process 
  //  (reformat process vector while we're at it)
#if defined(os_osf) || defined(os_windows) || defined(os_irix) || defined(arch_ia64)
   return true;
#endif
  int targetfd = -2;
  ThreadLibrary *threadlib = NULL;
  for (unsigned int i = 0; i < process_fds.size(); ++i) {
    if (process_fds[i].process == p) {
      //fprintf(stderr, "%s[%d]:  removing process %d\n", __FILE__, __LINE__, p->getPid());
      targetfd  = process_fds[i].fd;
      threadlib = process_fds[i].threadlib;
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

  //  remove thread instrumentation, if exists
  if (reportThreadCreateHandle) 
    if (!p->deleteSnippet(reportThreadCreateHandle))
       bperr("%s[%d]:  detachFromProcess(%d) failed to remove instru\n",
             __FILE__, __LINE__, p->getPid()); 
  if (reportThreadDestroyHandle)
    if (!p->deleteSnippet(reportThreadDestroyHandle))
       bperr("%s[%d]:  detachFromProcess(%d) failed to remove instru\n",
             __FILE__, __LINE__, p->getPid());
  if (reportThreadStartHandle)
    if (!p->deleteSnippet(reportThreadStartHandle))
       bperr("%s[%d]:  detachFromProcess(%d) failed to remove instru\n",
             __FILE__, __LINE__, p->getPid());
  if (reportThreadStopHandle)
    if (!p->deleteSnippet(reportThreadStopHandle))
       bperr("%s[%d]:  detachFromProcess(%d) failed to remove instru\n",
             __FILE__, __LINE__, p->getPid());

  //  if we never managed to fully attach, targetfd might still be -1.
  //  not sure if this could happen, but just return in this case.
  if (targetfd == -1) return true;

  //  get the mutatee to close the comms file desc.

  if (!mutateeDetach(p)) {
    //bperr("%s[%d]:  detachFromProcess(%d) could not clean up mutatee\n",
    //      __FILE__, __LINE__, p->getPid());
  }

  if (threadlib) delete threadlib;

  //  close our own file desc for this process.
  close(targetfd);

  return true; // true
}

void *BPatch_asyncEventHandler::registerDynamicCallCallback(
        BPatchDynamicCallSiteCallback cb, 
        BPatch_point *pt)
{
  BPatch_process *process = pt->proc;
  ThreadLibrary *threadLib = NULL;
  for (unsigned int i = 0; i < process_fds.size(); ++i) {
    if (process_fds[i].process == process) {
      threadLib = process_fds[i].threadlib;
      break;
    }
  }
  assert(threadLib);
  BPatch_module *dyninstLib = threadLib->getDyninstRT();

  //  find the function that will report the function call
  BPatch_Vector<BPatch_function *> funcs;
  if (!dyninstLib->findFunction("DYNINSTasyncDynFuncCall", funcs)
      || ! funcs.size() ) {
    bpfatal("%s[%d]:  could not find function: DYNINSTasyncDynFuncCall\n",
            __FILE__, __LINE__);
    return NULL;
  }
  if (funcs.size() > 1) {
    bperr("%s[%d]:  found %d varieties of function: DYNINSTasyncDynFuncCall\n",
          __FILE__, __LINE__, funcs.size());
  }

  void *handle = pt->monitorCalls(funcs[0]);
  if (!handle) {
    bperr("%s[%d]:  registerDynamicCallCallback, could not monitor site\n",
          __FILE__, __LINE__);
    return NULL;
  }

  if (!isRunning) {
    if (!createThread()) {
      fprintf(stderr, "%s[%d]:  failed to create thread\n", __FILE__, __LINE__);
      return false;
    }
  }

  dyncall_cb_record new_rec;
  new_rec.pt = pt;
  new_rec.cb = cb;
  new_rec.handle = handle;
  dyn_pts.push_back(new_rec);

  return handle;
}

bool BPatch_asyncEventHandler::removeDynamicCallCallback(void *handle)
{
  void *target = NULL;
  BPatch_point *pt = NULL;

  //  find monitoring request corresp. to <handle>
  //  If we have a lot of points to monitor, this could be slow.
  //  Consider better data struct for dyn_pts ?

  for (unsigned int i = 0; i < dyn_pts.size(); ++i) {
    if (dyn_pts[i].handle == handle) {
      target = dyn_pts[i].handle;
      pt = dyn_pts[i].pt;
      dyn_pts.erase(i,i);
      break;
    }
  }

  if (!target) {
    bperr("%s[%d]:  unregisterDynamicCallCallback, handle not found\n",
          __FILE__, __LINE__);
    return false;
  }

  //  <handle> found, so we can proceed with inst removal.

  assert(pt);
  if (! pt->stopMonitoring(handle)) {
    bperr("%s[%d]:  unregisterDynamicCallCallback, could not remove instrumentation\n",
          __FILE__, __LINE__);
    return false;
  }
  
  return true;
}

pdvector<thread_event_cb_record> *
BPatch_asyncEventHandler::getCBsForType(BPatch_asyncEventType t)
{
  switch(t) {
    case BPatch_threadCreateEvent:  return &thread_create_cbs;
    case BPatch_threadDestroyEvent: return &thread_destroy_cbs;
    case BPatch_threadStartEvent:   return &thread_start_cbs;
    case BPatch_threadStopEvent:    return &thread_stop_cbs;
    default: break;
  };
  bperr("%s[%d]:  bad event type %s, cannot register callback\n",
        __FILE__, __LINE__, asyncEventType2Str(t));
  return NULL;
}

bool BPatch_asyncEventHandler::registerThreadEventCallback(BPatch_process *proc,
         BPatch_asyncEventType type, BPatchAsyncThreadEventCallback cb)
{
  if (!isRunning) {
    if (!createThread()) {
      fprintf(stderr, "%s[%d]:  failed to create thread\n", __FILE__, __LINE__);
      return false;
    }
  }
  ThreadLibrary *threadLib = NULL;
  pdvector<thread_event_cb_record> *event_cbs = getCBsForType(type);
  if (!event_cbs) return false;

  //  find the ThreadLib for this thread
  for (unsigned int k = 0; k < process_fds.size(); ++k) {
    if (process_fds[k].process == proc) {
      threadLib = process_fds[k].threadlib;
      break;
    }
  }

  if (!threadLib) {
    bperr("%s[%d]:  instrument thread event of type %s, cannot find thread lib\n",
        __FILE__, __LINE__,asyncEventType2Str(type));
    return false;
  }

  if (!threadLib->hasCapability(type)) {
    bperr("%s[%d]:  instrument thread event of type %s, not supported for lib %s\n",
        __FILE__, __LINE__,asyncEventType2Str(type), threadLib->getLibName());
    return false;
  }

  //  find out if we already have (some) callback(s) for this thread
  thread_event_cb_record thread_event_rec = {0,0,0,0};

  for (unsigned int i = 0; i < event_cbs->size(); ++i) {
    if ((*event_cbs)[i].proc == proc) {
      thread_event_rec = (*event_cbs)[i];       
      break;
    } 
  }

  if (thread_event_rec.proc && thread_event_rec.cbs) {
    //  already have callbacks for this thread, just add the new one
    pdvector<BPatchAsyncThreadEventCallback> *cbs = thread_event_rec.cbs;
    assert(cbs);
    cbs->push_back(cb); 
    return true;
  }
  
  //fprintf(stderr, "%s[%d]:  allocating new callbacks for event %s\n", 
  //       __FILE__, __LINE__, asyncEventType2Str(type));
  //  don't have any yet, need to alloc a new callback vector
  thread_event_rec.proc = proc;
  thread_event_rec.cbs = new pdvector<BPatchAsyncThreadEventCallback>;
  thread_event_rec.cbs->push_back(cb);
  event_cbs->push_back(thread_event_rec);
  return true;
}

bool BPatch_asyncEventHandler::registerThreadEventCallback(BPatch_process *proc,
                             BPatch_asyncEventType type, BPatch_function *cb)
{
  ThreadLibrary *threadLib = NULL;
  pdvector<thread_event_cb_record> *event_cbs = getCBsForType(type);
  if (!event_cbs) return false;

  //  find the ThreadLib for this thread
  for (unsigned int k = 0; k < process_fds.size(); ++k) {
    if (process_fds[k].process == proc) {
      threadLib = process_fds[k].threadlib;
      break;
    }
  }

  if (!threadLib) {
    bperr("%s[%d]:  instrument thread event of type %s, cannot find thread lib\n",
        __FILE__, __LINE__,asyncEventType2Str(type));
    return false;
  }

  if (!threadLib->hasCapability(type)) {
    bperr("%s[%d]:  instrument thread event of type %s, not supported for lib %s\n",
        __FILE__, __LINE__,asyncEventType2Str(type), threadLib->getLibName());
    return false;
  }

  //  find out if we already have (some) callback(s) for this thread
  thread_event_cb_record thread_event_rec = {0,0,0,0};

  for (unsigned int i = 0; i < event_cbs->size(); ++i) {
    if ((*event_cbs)[i].proc == proc) {
      thread_event_rec = (*event_cbs)[i];
      break;
    }
  }

  BPatchSnippetHandle *handle = NULL;
  if (NULL == (handle = instrumentThreadEvent(proc, threadLib, type, cb))){
      bperr("%s[%d]:  cannot instrument thread event of type %s\n",
        __FILE__, __LINE__,asyncEventType2Str(type));
      return false;
  }
  
  //  callback inserted, keep track of BPatch_function and handle, for possible removal 

  if (thread_event_rec.proc && thread_event_rec.mutatee_side_cbs) {
    //  already have callbacks for this thread, just add the new one
    pdvector<BPatch_function *> *cbs = thread_event_rec.mutatee_side_cbs;
    pdvector<BPatchSnippetHandle *> *handles = thread_event_rec.handles;

    assert(handles);
    assert(cbs->size() == handles->size());
    cbs->push_back(cb);
    handles->push_back(handle);
    return true;
  }

  //  don't have any yet, need to alloc a new callback vector
  thread_event_rec.proc = proc;
  thread_event_rec.mutatee_side_cbs = new pdvector<BPatch_function *>;
  thread_event_rec.mutatee_side_cbs->push_back(cb);

  //  and a new handle vector
  assert(NULL == thread_event_rec.handles);
  thread_event_rec.handles = new pdvector<BPatchSnippetHandle *>;
  thread_event_rec.handles->push_back(handle);

  return true;
}

bool BPatch_asyncEventHandler::removeThreadEventCallback(BPatch_process *proc,
                                                         BPatch_asyncEventType type,
                                                         BPatchAsyncThreadEventCallback cb)
{
  pdvector<thread_event_cb_record> *event_cbs = getCBsForType(type);
  if (!event_cbs) return false;

  thread_event_cb_record thread_event_rec = {0,0,0,0};

  //  find the callbacks for this thread
  for (unsigned int i = 0; i < event_cbs->size(); ++i) {
    if ((*event_cbs)[i].proc == proc) {
      thread_event_rec = (*event_cbs)[i];
      break;
    }
  }

  if (thread_event_rec.proc && thread_event_rec.cbs) {
    pdvector<BPatchAsyncThreadEventCallback> *cbs = thread_event_rec.cbs;

    for (unsigned int j = 0; j < cbs->size(); ++j) {
      if (cb == (*cbs)[j]) {
        cbs->erase(j,j);

        //  if we remove the last callback record, clean up the vector too.
        if (!cbs->size()) delete cbs;
        thread_event_rec.cbs = NULL;
        return true;
      }
    }
  }

  bperr("%s[%d]:  cannot remove thread event of type %s.\n",
        __FILE__, __LINE__,asyncEventType2Str(type));
  return false;
}

bool BPatch_asyncEventHandler::removeThreadEventCallback(BPatch_process *proc,
                                                         BPatch_asyncEventType type,
                                                         BPatch_function *cb)
{
  pdvector<thread_event_cb_record> *event_cbs = getCBsForType(type);
  if (!event_cbs) return false;

  thread_event_cb_record thread_event_rec = {0,0,0,0};

  //  find the callbacks for this thread
  for (unsigned int i = 0; i < event_cbs->size(); ++i) {
    if ((*event_cbs)[i].proc == proc) {
      thread_event_rec = (*event_cbs)[i];
      break;
    }
  }

  if (thread_event_rec.proc && thread_event_rec.mutatee_side_cbs) {

    pdvector<BPatch_function *> *cbs = thread_event_rec.mutatee_side_cbs;
    pdvector<BPatchSnippetHandle *> *handles = thread_event_rec.handles;

    assert(handles);

    for (unsigned int j = 0; j < cbs->size(); ++j) {
      if (cb == (*cbs)[j]) {
        BPatchSnippetHandle *handle = (*handles)[j];
        cbs->erase(j,j);
        handles->erase(j,j);
        assert(cbs->size() == handles->size());

        //  if we remove the last callback record, clean up the vector too.
        if (!cbs->size()) {
          delete cbs;
          thread_event_rec.mutatee_side_cbs = NULL;
        }
        if (!handles->size()) {
          delete handles;
          thread_event_rec.handles = NULL;
        }

        //  remove the instrumentation:
        if (!proc->deleteSnippet(handle)) {
            bperr("%s[%d]:  failed to remove thread event instrumentation of type %s.\n",
            __FILE__, __LINE__,asyncEventType2Str(type));
            return false;
        }

        return true;
      }
    }
  }

  bperr("%s[%d]:  cannot remove thread event of type %s.  No record.\n",
        __FILE__, __LINE__,asyncEventType2Str(type));
  return false;

}


bool BPatch_asyncEventHandler::registerUserEventCallback(BPatch_process *proc,
         BPatchUserEventCallback cb)
{
  if (!isRunning) {
    if (!createThread()) {
      fprintf(stderr, "%s[%d]:  failed to create thread\n", __FILE__, __LINE__);
      return false;
    }
  }

  //  find out if we already have (some) callback(s) for this thread

  for (unsigned int i = 0; i < user_event_cbs.size(); ++i) {
    if (user_event_cbs[i].proc == proc) {
      //  already have an entry for this process, just replace the cb
      user_event_cbs[i].cb = cb;       
      return true;
    } 
  }

  user_event_cb_record user_event_rec;
  user_event_rec.proc = proc;
  user_event_rec.cb = cb;
  user_event_cbs.push_back(user_event_rec);

  return true;
}

bool BPatch_asyncEventHandler::removeUserEventCallback(BPatch_process *proc,
         BPatchUserEventCallback cb)
{
  for (unsigned int i = 0; i < user_event_cbs.size(); ++i) {
    if ((user_event_cbs[i].proc == proc) && (user_event_cbs[i].cb == cb)){
      user_event_cbs.erase(i,i);
      return true;
    } 
  }

  fprintf(stderr, "%s[%d]:  could not remove user event callback, nonexistent\n",
          __FILE__, __LINE__);

  return false;
}

BPatch_asyncEventHandler::BPatch_asyncEventHandler() :
#if !defined (os_windows)
  paused(false),
  stop_req(false),
#endif
  shutDownFlag(false),
  reportThreadCreateHandle(NULL),
  reportThreadDestroyHandle(NULL),
  reportThreadStartHandle(NULL),
  reportThreadStopHandle(NULL),
  isRunning(false)
{
  //  prefer to do socket init in the initialize() function so that we can
  //  return errors.

#if !defined(os_windows)
  //  init pause mutex/cond
  int err = 0;
  pthread_mutexattr_t attr;
  err = pthread_mutexattr_init(&attr);
  assert (!err);

  err = pthread_mutex_init(&pause_mutex, &attr);
  assert (!err);

  err = pthread_cond_init(&pause_cond, NULL);
  assert (!err);
#endif

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
  char path[64];
  sprintf(path, "%s/dyninstAsync.%s.%d", P_tmpdir, 
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

#ifdef NOTDEF
  //  Finally, create the event handling thread
  if (!createThread()) {
    bperr("%s[%d]:  could not create event handling thread\n", 
          __FILE__, __LINE__);
    return false;
  }
#endif
  isRunning = false;
  //fprintf(stderr, "%s[%d]:  Created async thread\n", __FILE__ , __LINE__);
  return true;
}

BPatch_asyncEventHandler::~BPatch_asyncEventHandler()
{
  if (isRunning) 
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
  char path[64];
  sprintf(path, "%s/dyninstAsync.%s.%d", P_tmpdir, 
                passwd_info->pw_name, (int) getpid());
  unlink(path);

  pthread_mutex_destroy(&pause_mutex);
  pthread_cond_destroy(&pause_cond);
#endif
}

bool BPatch_asyncEventHandler::createThread()
{
//fprintf(stderr, "%s[%d]:  welcome to createThread()\n", __FILE__, __LINE__);
#if defined(os_windows)
  fprintf(stderr, "%s[%d]:  about to start thread\n", __FILE__, __LINE__);
  handler_thread = _beginthread(asyncHandlerWrapper, 0, (void *) this);
  if (-1L == handler_thread) {
    bperr("%s[%d]:  _beginthread(...) failed\n", __FILE__, __LINE__); 
    fprintf(stderr,"%s[%d]:  _beginthread(...) failed\n", __FILE__, __LINE__); 
    return false;
  }
  fprintf(stderr, "%s[%d]:  started thread\n", __FILE__, __LINE__);
  isRunning = true;
  return true;
#else  // Unixes

  int err = 0;
  pthread_attr_t handler_thread_attr;

  err = pthread_attr_init(&handler_thread_attr);
  if (err) {
    bperr("%s[%d]:  could not init async handler thread attributes: %s, %d\n",
          __FILE__, __LINE__, strerror(err), err);
    return false;
  }

#if defined (os_solaris)
  err = pthread_attr_setdetachstate(&handler_thread_attr, PTHREAD_CREATE_DETACHED);
  if (err) {
    bperr("%s[%d]:  could not set async handler thread attrixibcutes: %s, %d\n",
          __FILE__, __LINE__, strerror(err), err);
    return false;
  }
#endif
  try {
  err = pthread_create(&handler_thread, &handler_thread_attr, 
                       &asyncHandlerWrapper, (void *) this);
  if (err) {
    bperr("%s[%d]:  could not start async handler thread: %s, %d\n",
          __FILE__, __LINE__, strerror(err), err);
    fprintf(stderr,"%s[%d]:  could not start async handler thread: %s, %d\n",
          __FILE__, __LINE__, strerror(err), err);
    return false;
  }
  } catch(...) {
    assert(0);
  }

  isRunning = true;

  err = pthread_attr_destroy(&handler_thread_attr);
  if (err) {
    bperr("%s[%d]:  could not destroy async handler attr: %s, %d\n",
          __FILE__, __LINE__, strerror(err), err);
    return false; 
  }

#if defined (arch_ia64)
   sigset_t sigs;
   sigaddset(&sigs, SIGTRAP);
   sigaddset(&sigs, SIGILL);
   sigaddset(&sigs, SIGSEGV);
   sigaddset(&sigs, SIGCHLD);
   sigaddset(&sigs, SIGALRM);
   int ret = pthread_sigmask(SIG_UNBLOCK, &sigs, NULL);
   if (ret) {
     fprintf(stderr, "%s[%d]:  error setting thread sigmask: %s\%d\n",
            __FILE__, __LINE__, strerror(errno), errno);
   }
#endif

  return true;

#endif
}

#if !defined (os_windows)
void BPatch_asyncEventHandler::handlePauseRequest()
{
  int pauseerr = 0;
  pauseerr = pthread_mutex_lock(&pause_mutex);
  assert (!pauseerr);
  if (paused) {
    fprintf(stderr, "%s[%d]:  Thr: Getting ready to pause\n", __FILE__, __LINE__);
    //  request to pause thread has been issued, wait for signal
    pauseerr = pthread_cond_signal(&pause_cond);
    pauseerr = pthread_cond_wait(&pause_cond, &pause_mutex);
    assert (!pauseerr);
    fprintf(stderr, "%s[%d]:  Thr: unpaused\n", __FILE__, __LINE__);
    //  pause is over when signal received.
    assert (!paused);
  }

  pauseerr = pthread_mutex_unlock(&pause_mutex);
  assert (!pauseerr);
}
void BPatch_asyncEventHandler::handleStopRequest()
{
  int stoperr = 0;
  stoperr = pthread_mutex_lock(&pause_mutex);
  assert (!stoperr);
  if (stop_req) {
    fprintf(stderr, "%s[%d]:  Thr: Getting ready to stop\n", __FILE__, __LINE__);
    isRunning = false;
    stoperr = pthread_mutex_unlock(&pause_mutex);
    pthread_exit(NULL);
  }
  stoperr = pthread_mutex_unlock(&pause_mutex);
  assert (!stoperr);
}
#endif


void BPatch_asyncEventHandler::main()
{
#if defined (arch_ia64)
   sigset_t sigs;
   sigaddset(&sigs, SIGTRAP);
   sigaddset(&sigs, SIGILL);
   sigaddset(&sigs, SIGSEGV);
   sigaddset(&sigs, SIGCHLD);
   sigaddset(&sigs, SIGALRM);
   int ret = pthread_sigmask(SIG_UNBLOCK, &sigs, NULL);
   if (ret) {
     fprintf(stderr, "%s[%d]:  error setting thread sigmask: %s\%d\n",
            __FILE__, __LINE__, strerror(errno), errno);
   } 
#endif
   BPatch_asyncEventRecord ev;
   //fprintf(stderr, "%s[%d]:  BPatch_asyncEventHandler::main(), thread id = %lu\n", 
   //        __FILE__, __LINE__, (unsigned long) pthread_self());

   while (1) {
#if !defined (os_windows)
     handlePauseRequest();
     handleStopRequest();
#endif

     if (waitNextEvent(ev)) {
       if (shutDownFlag) goto done; // want to flush here?
       if (ev.type == BPatch_nullEvent) continue;
       handleEvent(ev); // This is locked.
     }
     else {
       //  comms problem? what to do?
       bperr("%s[%d]:  waitNextEvent returned false, async handler dying...\n",
              __FILE__, __LINE__);
       goto done;
     } 
   } //  main loop


done:

   isRunning = false;
   return;
}

bool BPatch_asyncEventHandler::shutDown()
{
  if (!isRunning) goto close_comms;

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
  isRunning = false;
#endif

  close_comms:

  return true;
}

#if !defined (os_windows)
bool BPatch_asyncEventHandler::pause()
{
  int err = 0;
  bool ret = false;
  err = pthread_mutex_lock(&pause_mutex); 
  assert (!err);

  if ((paused) || (!isRunning)) {
    err = pthread_mutex_unlock(&pause_mutex); 
    assert (!err);
    return true;
  }

  //  set paused to true, even though we are not paused yet.
  paused = true;

  //  wait for thread to see that it should pause itself
  err = pthread_cond_wait(&pause_cond, &pause_mutex);
  assert (!err);

  ret = paused; 

  err = pthread_mutex_unlock(&pause_mutex);
  assert (!err);

  return ret;
}

bool BPatch_asyncEventHandler::unpause()
{
  int err = 0;
  err = pthread_mutex_lock(&pause_mutex);
  assert (!err);

  if (!paused) {
    err = pthread_mutex_unlock(&pause_mutex);
    assert (!err);
    return true;
  }

  //  set paused to false
  paused = false;

  //  and signal the thread (which is waiting on pause_cond)
  err = pthread_mutex_unlock(&pause_mutex);
  assert (!err);

  err = pthread_cond_signal(&pause_cond);
  assert (!err);

  return true;
}
bool BPatch_asyncEventHandler::stop()
{
  int err = 0;
  err = pthread_mutex_lock(&pause_mutex);
  assert (!err);
  if (!isRunning) {
    err = pthread_mutex_unlock(&pause_mutex);
    assert (!err);
    return true;
  }
  stop_req = true;
  err = pthread_mutex_unlock(&pause_mutex);
  assert (!err);

  err = pthread_join(handler_thread, NULL);
  assert (!err);
  return (!isRunning);
}
bool BPatch_asyncEventHandler::resume()
{
  int err = 0;
  err = pthread_mutex_lock(&pause_mutex);
  assert (!err);
  if (isRunning) {
    err = pthread_mutex_unlock(&pause_mutex);
    assert (!err);
    return true;
  }

  return createThread();
}
#endif
bool BPatch_asyncEventHandler::waitNextEvent(BPatch_asyncEventRecord &ev)
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

  //  keep a static list of events in case we get several simultaneous
  //  events from select()...  just in case.

  static pdvector<BPatch_asyncEventRecord> event_queue;

  if (event_queue.size()) {
    // we already have one (from last call of this func)
    //
    //  this might result in an event reordering, not sure if important
    //   (since we are removing from the end of the list)
    //ev = event_queue[event_queue.size() - 1];
    //event_queue.pop_back();
    ev = event_queue[0];
    event_queue.erase(0,0);
    __UNLOCK;
    return true;
  }

  int width = 0;
  fd_set readSet;
  fd_set errSet;

  FD_ZERO(&readSet);
  FD_ZERO(&errSet);

  struct timeval timeout;
  //timeout.tv_sec = 0;
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
  timeout.tv_usec = 100;

  //  start off with a NULL event:
  ev.type = BPatch_nullEvent;

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
       //fprintf(stderr, "%s[%d]:  about to read new connection\n", __FILE__, __LINE__); 
       //  do a (blocking) read so that we can get the pid associated with
       //  this connection.
       BPatch_asyncEventRecord pid_ev;
       if (! readEvent(new_fd, pid_ev)) {
         //fprintf(stderr,"%s[%d]:  readEvent failed due to process termination\n", __FILE__, __LINE__);
         //bperr("%s[%d]:  readEvent failed\n", __FILE__, __LINE__);
         return false;
       }
       else {
         assert(pid_ev.type == BPatch_newConnectionEvent);
         ev = pid_ev;
         //fprintf(stderr, "%s[%d]:  new connection to %d\n", __FILE__, __LINE__, ev.pid);
         ev.event_fd = new_fd;
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
      BPatch_asyncEventRecord new_ev;

      if (! readEvent(process_fds[j].fd, new_ev)) { 
        //  This read can fail if the mutatee has exited.  Just note that this
        //  fd is no longer valid, and keep quiet.
        //if (process_fds[j].process->isTerminated()) {
        if (1) {
          //  remove this process/fd from our vector
          //fprintf(stderr,"%s[%d]:  readEvent failed due to process termination\n", __FILE__, __LINE__);
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

        if (ev.type != BPatch_nullEvent) {
          ev = new_ev;
          ev.event_fd = process_fds[j].fd;
        }
        else {
          // Queue up events if we got more than one.
          //  NOT SAFE???
          if (new_ev.type != BPatch_nullEvent) {
            BPatch_asyncEventRecord qev = new_ev;
            qev.event_fd = process_fds[j].fd;
            event_queue.push_back(qev);
          }
        }

      }
      
    }
  }
#ifdef NOTDEF
  fprintf(stderr, "%s[%d]:  leaving waitNextEvent: events in queue:\n", __FILE__, __LINE__);
  for (unsigned int r = 0; r < event_queue.size(); ++r) {
    fprintf(stderr, "\t%s\n", asyncEventType2Str(event_queue[r].type));
  }
#endif

  __UNLOCK;
  return true;
}

bool BPatch_asyncEventHandler::handleEventLocked(BPatch_asyncEventRecord &ev)
{
   //fprintf(stderr, "%s[%d]:  inside handleEvent, got %s\n", 
   //        __FILE__, __LINE__, asyncEventType2Str(ev.type));

   int event_fd = -1;
   BPatch_process *appProc = NULL;
   ThreadLibrary *threadLibrary = NULL;
   unsigned int j;
   //  Go through our process list and find the appropriate record

   for (j = 0; j < process_fds.size(); ++j) {
      if (!process_fds[j].process) {
        fprintf(stderr, "%s[%d]:  invalid process record!\n", __FILE__, __LINE__);
        continue;
      }
      unsigned int process_pid = process_fds[j].process->getPid();
      if (process_pid == ev.pid) {
         event_fd = process_fds[j].fd;
         appProc = process_fds[j].process; 
         threadLibrary = process_fds[j].threadlib; 
         break;
      }
   }
   

   if (!appProc) {
     if (ev.type == BPatch_nullEvent) return true; 
     //fprintf(stderr, "%s[%d]:  ERROR:  Got event %s for pid %d, but no proc, out of %d procs\n",
      //     __FILE__, __LINE__, asyncEventType2Str(ev.type), ev.pid,process_fds.size());
     //for (unsigned int k = 0; k < process_fds.size(); ++k) {
     //  fprintf(stderr, "\thave process %d\n", process_fds[k].process->getPid());
    // }
     //  This can happen if the process has died and has already been cleaned out by
     //  cleanUpTerminatedProcs -- might want to keep a list of terminated procs (removed)
     //  to do more comprehensive error checking here.
     return false;
   }

   switch(ev.type) {
     case BPatch_nullEvent:
       return true;
     case BPatch_newConnectionEvent: 
     {
       //  add this fd to the pair.
       //  this fd will then be watched by select for new events.

       assert(event_fd == -1);
       process_fds[j].fd = ev.event_fd;

#ifdef NOTDEF
       fprintf(stderr, "%s[%d]:  after handling new connection, we have\n", __FILE__, __LINE__);
       for (unsigned int t = 0; t < process_fds.size(); ++t) {
          fprintf(stderr, "\tpid = %d, fd = %d\n", process_fds[t].process->getPid(), process_fds[t].fd);
       }
#endif
       return true;
     }

     case BPatch_internalShutDownEvent:
       return false;

     case BPatch_threadCreateEvent:
     case BPatch_threadStartEvent:
     case BPatch_threadStopEvent:
     case BPatch_threadDestroyEvent:
     {
       //  Read auxilliary packet with dyn call info

       BPatch_threadEventRecord call_rec;
       if (!readEvent(ev.event_fd, (void *) &call_rec, sizeof(BPatch_threadEventRecord))) {
          bperr("%s[%d]:  failed to read thread event call record\n",
                __FILE__, __LINE__);
          return false;
       }

       //  BPatch_threadEventRecord contains specific info on this thread

       unsigned long tid = call_rec.tid;

       //  find the callback list for this thread

       thread_event_cb_record *rec = NULL;
       pdvector<thread_event_cb_record> *event_cbs = getCBsForType(ev.type);
       if (!event_cbs) return false;

       for (unsigned int i = 0; i < event_cbs->size(); ++i) {
         if ((*event_cbs)[i].proc == appProc) {
           rec = &( (*event_cbs)[i] );
           break;
         }
       }  

       if (!rec) {
         // no record for this thread
         //
         //fprintf(stderr, "%s[%d]:  FIXME ?? \n", __FILE__, __LINE__);
         //fprintf(stderr, "%s[%d]:  event_cbs.size() == %d\n", __FILE__, __LINE__, event_cbs->size());
         //   for (unsigned int i = 0; i < event_cbs->size(); ++i) {
         //     fprintf(stderr, "\t CB for thread %p, target = %p\n", (*event_cbs)[i].thread,appThread);
         //   }

         return false;
       }

       pdvector<BPatchAsyncThreadEventCallback> *cbs = rec->cbs;
       if (!cbs) {
         // no callbacks for this event on this thread
         //
         return false;
       }

       if (!cbs->size()) {
         // no callbacks for this event on this thread
         //
         return false;
       }

       // call all cbs in the list
       //  (actually just register them in the mailbox,
       //   to be called on the primary thread).
       for (unsigned int j = 0; j < cbs->size(); ++j) {
         BPatchAsyncThreadEventCallback cb = (*cbs)[j];
         event_mailbox->registerCallback(ev.type, cb, appProc, tid);
       }
       return true;
     }

     case BPatch_dynamicCallEvent:
     {
       //  Read auxilliary packet with dyn call info

       BPatch_dynamicCallRecord call_rec;
       if (!readEvent(ev.event_fd, (void *) &call_rec, sizeof(BPatch_dynamicCallRecord))) {
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
         assert(pts[j]->cb);
         BPatchDynamicCallSiteCallback cb = pts[j]->cb;
         BPatch_point *pt = pts[j]->pt;
         event_mailbox->registerCallback(cb, pt, bpf);
       }

       return true;
     }
     case BPatch_userEvent:
     {
       //  Read auxilliary packet with user specifiedbuffer
       assert(ev.size > 0);
       int *userbuf = new int[ev.size];

       if (!readEvent(ev.event_fd, (void *) userbuf, ev.size)) {
          bperr("%s[%d]:  failed to read user specified data\n",
                __FILE__, __LINE__);
          delete [] userbuf;
          return false;
       }
       
       
       //  find the callback for this process
       unsigned int i = 0;
       bool foundit = false;
       for (i = 0; i < user_event_cbs.size(); ++i) {
         if (user_event_cbs[i].proc == appProc) {
            foundit = true;
            break;
         }
       }
       if (!foundit)
         fprintf(stderr, "%s[%d]:  Got user message, but no callback provided\n",
                 __FILE__, __LINE__);
       else
         event_mailbox->registerCallback(user_event_cbs[i].cb, userbuf, ev.size);
       return true;
     } 
     default:
       bperr("%s[%d]:  request to handle unsupported event: %s\n", 
             __FILE__, __LINE__, asyncEventType2Str(ev.type));
       return false;
       break;

   }
   return true;
}

bool BPatch_asyncEventHandler::mutateeDetach(BPatch_process *p)
{
  BPatch_module *dyninstLib = NULL;
  for (unsigned int i = 0; i < process_fds.size(); ++i) {
    if (process_fds[i].process == p) {
      dyninstLib = process_fds[i].threadlib->getDyninstRT();
      break;
    }
  }
    //  use oneTimeCode to call a function in the mutatee to handle
    //  closing of the comms socket.
  if (!dyninstLib) return false;

  //  find the function that will initiate the disconnection
  BPatch_Vector<BPatch_function *> funcs;
  if (!dyninstLib->findFunction("DYNINSTasyncDisconnect", funcs)
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
  if (!p->oneTimeCode(disconnectcall)) {
    bpfatal("%s[%d]:  failed to disconnect mutatee to async handler\n", 
            __FILE__, __LINE__);
    return false;
  }

  return true;

}

BPatchSnippetHandle *
BPatch_asyncEventHandler::instrumentThreadEvent(BPatch_process *process,
                                                ThreadLibrary *threadLib,
                                                BPatch_asyncEventType t,
                                                BPatch_function *f)
{
  if (!f) return NULL;
  BPatchSnippetHandle *ret = NULL;
  BPatch_callWhen when = BPatch_callBefore;

  if (!threadLib->hasCapability(t)) {
      bperr("%s[%d]:  thread event type %s not supported for thread lib %s\n",
            __FILE__, __LINE__, asyncEventType2Str(t), threadLib->getLibName());
      return NULL;
  }

  BPatch_Vector<BPatch_point *> pts;
  unsigned int numFuncs = threadLib->numberOfFuncsForEvent(t);

  //  find entry point(s) for target inst funcs

  for (unsigned int i = 0; i < numFuncs; ++i) {
    BPatch_function *instruFunc = threadLib->funcForEvent(t,i);

    // find entry point

    BPatch_Vector<BPatch_point *> *entryPoints = instruFunc->findPoint(BPatch_entry);
    if (!entryPoints || !entryPoints->size()) {
      char buf[1024];
      bperr("%s[%d]:  no entry points for function %s in module %s\n",
            __FILE__, __LINE__, instruFunc->getName(buf, 1024), threadLib->getLibName());
      return NULL;
    }
    if (entryPoints->size() > 1) {
      char buf[1024];
      bperr("%s[%d]:  WARN: %d entry points for function %s in module %s\n",
            __FILE__, __LINE__, entryPoints->size(), instruFunc->getName(buf, 1024), 
            threadLib->getLibName());
    }

    // construct function call and insert

    pts.push_back((*entryPoints)[0]);
  }

  assert (pts.size()); 

  //  generate function call snippet...

  BPatch_Vector<BPatch_snippet *> args;
  BPatch_funcCallExpr funcCall(*f, args);

  //  ...  and insert at all the interesting points we found.

  ret = process->insertSnippet(funcCall, pts, when, BPatch_lastSnippet);

  if (!ret) {
    bperr("%s[%d]:  failed to insert instrumentation\n",
          __FILE__, __LINE__);
  }

  return ret;
}

ThreadLibrary *BPatch_asyncEventHandler::newThreadLibrary(BPatch_process *proc)
{
  ThreadLibrary *tlib = new ThreadLibrary(proc, THREAD_LIB_NAME);
  if (!tlib->exists()) return tlib;
#ifdef BPATCH_LIBRARY
#if defined(os_windows)
#ifdef NOTDEF
  tlib->addThreadEventFunction(BPatch_threadCreateEvent, "_beginthread");
  tlib->addThreadEventFunction(BPatch_threadCreateEvent, "_beginthreadex");
  tlib->addThreadEventFunction(BPatch_threadDestroyEvent, "_endthread");
  tlib->addThreadEventFunction(BPatch_threadDestroyEvent, "_endthreadex");
#endif
#elif defined(os_linux)
  tlib->addThreadEventFunction(BPatch_threadCreateEvent, "start_thread");
  tlib->addThreadEventFunction(BPatch_threadDestroyEvent, "pthread_exit");
  tlib->addThreadEventFunction(BPatch_threadStartEvent, "_longjmp");
  tlib->addThreadEventFunction(BPatch_threadStopEvent, "_usched_swtch");
#elif defined(os_solaris)
  tlib->addThreadEventFunction(BPatch_threadCreateEvent, "_thrp_create");
  tlib->addThreadEventFunction(BPatch_threadDestroyEvent, "_thr_exit_common");
#elif defined(os_aix)
  tlib->addThreadEventFunction(BPatch_threadCreateEvent, "_pthread_body");
  tlib->addThreadEventFunction(BPatch_threadDestroyEvent, "pthread_exit");
  tlib->addThreadEventFunction(BPatch_threadStartEvent, "_longjmp");
  tlib->addThreadEventFunction(BPatch_threadStopEvent, "_usched_swtch");
#elif defined(os_osf)
#ifdef NOTDEF
  tlib->addThreadEventFunction(BPatch_threadCreateEvent, "_pthread_body");
  tlib->addThreadEventFunction(BPatch_threadDestroyEvent, "pthread_exit");
  tlib->addThreadEventFunction(BPatch_threadStartEvent, "_longjmp");
  tlib->addThreadEventFunction(BPatch_threadStopEvent, "_usched_swtch");
#endif
#elif defined(os_irix)
  tlib->addThreadEventFunction(BPatch_threadCreateEvent, "_pthread_body");
  tlib->addThreadEventFunction(BPatch_threadDestroyEvent, "pthread_exit");
  tlib->addThreadEventFunction(BPatch_threadStartEvent, "_longjmp");
  tlib->addThreadEventFunction(BPatch_threadStopEvent, "_usched_swtch");
#endif
#endif // BPATCH_LIBRARY
  return tlib;
}

bool BPatch_asyncEventHandler::cleanUpTerminatedProcs()
{
  bool ret = false;
  unsigned int j;
  //  iterate from end of vector in case we need to use erase()
  for (int i = (int) process_fds.size() -1; i >= 0; i--) {
    if (process_fds[i].process->isTerminated()) {
    //  fprintf(stderr, "%s[%d]:  Process %d has terminated, cleaning up\n", __FILE__, __LINE__, process_fds[i].process->getPid());
      delete (process_fds[i].threadlib);
      pdvector<thread_event_cb_record> *cbs = getCBsForType(BPatch_threadCreateEvent);
      for (j = 0; j < cbs->size(); ++j) {
        if ((*cbs)[j].proc == process_fds[i].process) {
          if ((*cbs)[j].cbs) delete (*cbs)[j].cbs;
          if ((*cbs)[j].mutatee_side_cbs) delete (*cbs)[j].mutatee_side_cbs;
          if ((*cbs)[j].handles) delete (*cbs)[j].handles;
        }
      }
      cbs = getCBsForType(BPatch_threadDestroyEvent);
      for (j = 0; j < cbs->size(); ++j) {
        if ((*cbs)[j].proc == process_fds[i].process) {
          if ((*cbs)[j].cbs) delete (*cbs)[j].cbs;
          if ((*cbs)[j].mutatee_side_cbs) delete (*cbs)[j].mutatee_side_cbs;
          if ((*cbs)[j].handles) delete (*cbs)[j].handles;
        }
      }
      cbs = getCBsForType(BPatch_threadStartEvent);
      for (j = 0; j < cbs->size(); ++j) {
        if ((*cbs)[j].proc == process_fds[i].process) {
          if ((*cbs)[j].cbs) delete (*cbs)[j].cbs;
          if ((*cbs)[j].mutatee_side_cbs) delete (*cbs)[j].mutatee_side_cbs;
          if ((*cbs)[j].handles) delete (*cbs)[j].handles;
        }
      }
      cbs = getCBsForType(BPatch_threadStopEvent);
      for (j = 0; j < cbs->size(); ++j) {
        if ((*cbs)[j].proc == process_fds[i].process) {
          if ((*cbs)[j].cbs) delete (*cbs)[j].cbs;
          if ((*cbs)[j].mutatee_side_cbs) delete (*cbs)[j].mutatee_side_cbs;
          if ((*cbs)[j].handles) delete (*cbs)[j].handles;
        }
      }

      process_fds.erase(i,i);
      ret = true;
    }
  }
  return ret;
}

BPatch_asyncEventType rt2EventType(rtBPatch_asyncEventType t)
{
#define RT_CASE_CONV(x) case rt##x: return x
  switch(t) {
    RT_CASE_CONV(BPatch_nullEvent);
    RT_CASE_CONV(BPatch_internalShutDownEvent);
    RT_CASE_CONV(BPatch_newConnectionEvent);
    RT_CASE_CONV(BPatch_threadCreateEvent);
    RT_CASE_CONV(BPatch_threadDestroyEvent);
    RT_CASE_CONV(BPatch_threadStartEvent);
    RT_CASE_CONV(BPatch_threadStopEvent);
    RT_CASE_CONV(BPatch_dynamicCallEvent);
    RT_CASE_CONV(BPatch_userEvent);
  }
  fprintf(stderr, "%s[%d], invalid conversion\n", __FILE__, __LINE__);
  return BPatch_nullEvent;
}

bool BPatch_asyncEventHandler::readEvent(PDSOCKET fd, BPatch_asyncEventRecord &ev)
{
  rtBPatch_asyncEventRecord rt_ev;
  if (!readEvent(fd, &rt_ev, sizeof(rtBPatch_asyncEventRecord)))
    return false;
  ev.pid = rt_ev.pid;
  ev.event_fd = rt_ev.event_fd;
  ev.type = rt2EventType(rt_ev.type);
  ev.size = rt_ev.size;
  return true;
}

#if defined(os_windows)
//#ifdef NOTDEF
bool BPatch_asyncEventHandler::readEvent(PDSOCKET fd, void *ev, ssize_t sz)
{
  fprintf(stderr, "%s[%d] about to recv\n", __FILE__, __LINE__);
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

#if defined(os_windows)
  bytes_read = _read(fd, ev , sz);
#else
  bytes_read = read(fd, ev , sz);
#endif

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
  CASE_RETURN_STR(BPatch_threadStartEvent);
  CASE_RETURN_STR(BPatch_threadStopEvent);
  CASE_RETURN_STR(BPatch_threadDestroyEvent);
  CASE_RETURN_STR(BPatch_dynamicCallEvent);
  default:
  return "BadEventType";
  }
}

