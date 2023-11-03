/*
 * See the dyninst/COPYRIGHT file for copyright information.
 *
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
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

#include "int_thread_db.h"


#include <cassert>
#include <cerrno>
#include <cstdarg>
#include <cstring>
#include <set>
#include <dlfcn.h>
#include <iostream>

#include "common/src/dthread.h"
#include "compiler_annotations.h"
#include "common/h/SymReader.h"
#include "int_event.h"
#include "Mailbox.h"

#include "boost/filesystem.hpp"

using namespace std;

#if defined(cap_thread_db)


void ps_plog(const char *format, ...) DYNINST_PRINTF_ANNOTATION(1, 2);

/*
 * proc_service interface implementation, needed by libthread_db
 */

ps_err_e ps_pglobal_lookup(struct ps_prochandle *handle, const char *objName,
        const char *symName, psaddr_t *symbolAddr)
{
    pthrd_printf("Looking up symbol %s in %s\n", symName, objName);
    return handle->thread_db_proc->getSymbolAddr(objName, symName, symbolAddr);
}

ps_err_e ps_pread(struct ps_prochandle *handle, psaddr_t remote, void *local, size_t size) {
   thread_db_process *llproc = handle->thread_db_proc;
   pthrd_printf("thread_db reading from %#lx to %#lx, size = %d on %d\n",
                (unsigned long)remote, (unsigned long)local, (int)size, llproc->getPid());

   llproc->resps.clear();
   async_ret_t result = llproc->getMemCache()->readMemory(local, (Address) remote, size,
                                                          llproc->resps,
                                                          llproc->triggerThread());


   switch (result) {
      case aret_success:
         llproc->hasAsyncPending = false;
         return PS_OK;
      case aret_async:
         llproc->hasAsyncPending = true;
         pthrd_printf("Incomplete async read in thread_db read\n");
         return PS_ERR;
      case aret_error:
         llproc->hasAsyncPending = false;
         pthrd_printf("Unexpected read error in thread_db read\n");
         return PS_ERR;
   }
   assert(0);
   return PS_ERR;
}

ps_err_e ps_pdread(struct ps_prochandle *handle, psaddr_t remote, void *local, size_t size) {
   return ps_pread(handle, remote, local, size);
}

ps_err_e ps_ptread(struct ps_prochandle *handle, psaddr_t remote, void *local, size_t size) {
   return ps_pread(handle, remote, local, size);
}

ps_err_e ps_pwrite(struct ps_prochandle *handle, psaddr_t remote, const void *local, size_t size) {
    pthrd_printf("thread_db writing to %#lx from %#lx, size = %d on %d\n",
            (unsigned long)remote, (unsigned long)local, (int)size, handle->thread_db_proc->getPid());

    thread_db_process *proc = handle->thread_db_proc;

    async_ret_t result = proc->getMemCache()->writeMemory((Address) remote,
                                                          const_cast<void *>(local),
                                                          size,
                                                          proc->res_resps,
                                                          proc->triggerThread());
    switch (result) {
      case aret_success:
         proc->hasAsyncPending = false;
         return PS_OK;
      case aret_async:
         proc->hasAsyncPending = true;
         pthrd_printf("Incomplete async write in thread_db write\n");
         return PS_ERR;
      case aret_error:
         proc->hasAsyncPending = false;
         pthrd_printf("Unexpected read error in thread_db write\n");
         return PS_ERR;
    }
    assert(0);
    return PS_ERR;
}

ps_err_e ps_pdwrite(struct ps_prochandle *handle, psaddr_t remote, const void *local, size_t size) {
   return ps_pwrite(handle, remote, local, size);
}

ps_err_e ps_ptwrite(struct ps_prochandle *handle, psaddr_t remote, const void *local, size_t size) {
   return ps_pwrite(handle, remote, local, size);
}

ps_err_e ps_linfo(struct ps_prochandle *handle, lwpid_t lwp, void *lwpInfo) {
   if( !handle->thread_db_proc->plat_getLWPInfo(lwp, lwpInfo) ) {
      pthrd_printf("thread_db called ps_linfo, returning error\n");
      return PS_ERR;
   }
   pthrd_printf("thread_db called ps_linfo, returning info\n");
   return PS_OK;
}

ps_err_e ps_lstop(struct ps_prochandle *handle, lwpid_t lwp) {
   int_process *proc = handle->thread_db_proc;
   int_threadPool *tp = proc->threadPool();
   assert(tp);
   int_thread *thr = tp->findThreadByLWP((Dyninst::LWP) lwp);
   if (!thr) {
      perr_printf("ps_lstop is unable to find LWP %d in process %d\n",
                  lwp, proc->getPid());
      return PS_ERR;
   }
   pthrd_printf("ps_lstop on %d/%d\n", proc->getPid(), thr->getLWP());

   if (thr->getInternalState().getState() == int_thread::stopped) {
      return PS_OK;
   }
   else if (thr->getInternalState().getState() != int_thread::running) {
      perr_printf("Error, ps_lstop on thread in bad state\n");
      return PS_ERR;
   }

   thr->getInternalState().setState(int_thread::stopped);

   return PS_OK;
}

ps_err_e ps_lcontinue(struct ps_prochandle *handle, lwpid_t lwp) {
   int_process *proc = handle->thread_db_proc;
   int_threadPool *tp = proc->threadPool();
   assert(tp);
   int_thread *thr = tp->findThreadByLWP((Dyninst::LWP) lwp);
   if (!thr) {
      perr_printf("ps_lcontinue is unable to find LWP %d in process %d\n",
                  lwp, proc->getPid());
      return PS_ERR;
   }
   pthrd_printf("ps_lcontinue on %d/%d\n", proc->getPid(), thr->getLWP());

   if (thr->getInternalState().getState() == int_thread::running) {
      return PS_OK;
   }
   else if (thr->getInternalState().getState() != int_thread::stopped) {
      perr_printf("Error, ps_lcontinue on thread in bad state\n");
      return PS_ERR;
   }

   thr->getInternalState().setState(int_thread::stopped);
   return PS_OK;
}

ps_err_e ps_lgetregs(struct ps_prochandle *handle, lwpid_t lwp, prgregset_t regs) {
   thread_db_process *proc = handle->thread_db_proc;
   int_threadPool *tp = proc->threadPool();
   assert(tp);
   int_thread *llthr = tp->findThreadByLWP((Dyninst::LWP) lwp);
   if (!llthr) {
      perr_printf("ps_lgetregs is unable to find LWP %d in process %d\n",
                  lwp, proc->getPid());
      return PS_ERR;
   }

   thread_db_thread *thr = dynamic_cast<thread_db_thread *>(llthr);

   pthrd_printf("thread_db reading registers on thread %d/%d\n",
                proc->getPid(), thr->getLWP());

   int_registerPool pool;
   async_ret_t result = proc->getMemCache()->getRegisters(llthr, pool);
   if (result == aret_async) {
      pthrd_printf("Async return during get reg\n");
      return PS_ERR;
   }
   if (result == aret_error) {
      pthrd_printf("Error return during get reg\n");
      return PS_ERR;
   }

   bool bresult = thr->plat_convertToSystemRegs(pool, (unsigned char *) regs, true);
   if (!bresult) {
      pthrd_printf("Error convering to system regs\n");
      return PS_ERR;
   }
   return PS_OK;
}

pid_t ps_getpid (struct ps_prochandle *ph)
{
   int pid = ph->thread_db_proc->threaddb_getPid();
   pthrd_printf("thread_db called ps_getpid.  Returning %d\n", pid);
   return pid;
}

void ps_plog(const char *format, ...) {
   pthrd_printf("thread_db called ps_plog\n");
    if( !dyninst_debug_proccontrol ) return;
    if( NULL == format ) return;

    va_list va;
    va_start(va, format);
    vfprintf(pctrl_err_out, format, va);
    va_end(va);
}

#define NA_IMPLEMENTED "This function is not implemented"

ps_err_e ps_lgetfpregs(struct ps_prochandle *, lwpid_t, prfpregset_t *) {
    assert(!NA_IMPLEMENTED);
    return PS_ERR;
}

ps_err_e ps_lsetfpregs(struct ps_prochandle *, lwpid_t, const prfpregset_t *) {
    assert(!NA_IMPLEMENTED);
    return PS_ERR;
}

ps_err_e ps_lsetregs(struct ps_prochandle *, lwpid_t, const prgregset_t) {
    assert(!NA_IMPLEMENTED);
    return PS_ERR;
}

ps_err_e ps_lgetxmmregs (struct ps_prochandle *, lwpid_t, char *) {
    assert(!NA_IMPLEMENTED);
    return PS_ERR;
}

ps_err_e ps_lsetxmmregs (struct ps_prochandle *, lwpid_t, const char *) {
    assert(!NA_IMPLEMENTED);
    return PS_ERR;
}

ps_err_e ps_pcontinue(struct ps_prochandle *) {
    assert(!NA_IMPLEMENTED);
    return PS_ERR;
}

ps_err_e ps_pdmodel(struct ps_prochandle *, int *) {
    assert(!NA_IMPLEMENTED);
    return PS_ERR;
}

ps_err_e ps_pstop(struct ps_prochandle *) {
    assert(!NA_IMPLEMENTED);
    return PS_ERR;
}

ps_err_e ps_get_thread_area(const struct ps_prochandle *phandle, lwpid_t lwp, int val, psaddr_t *addr)
{
   thread_db_process *tdb_proc = phandle->thread_db_proc;
   thread_db_thread *tdb_thread = dynamic_cast<thread_db_thread *>(tdb_proc->threadPool()->findThreadByLWP(lwp));

   Dyninst::Address daddr = 0;
   bool result = tdb_thread->thrdb_getThreadArea(val, daddr);
   if (addr && result)
      *addr = (psaddr_t) daddr;

   pthrd_printf("thread_db called ps_get_thread_area.  Returning %s\n", result ? "PS_OK" : "PS_ERR");
   return result ? PS_OK : PS_ERR;
}

#if defined(THREAD_DB_STATIC)
#define TDB_BIND(SYM) \
   p_ ## SYM = SYM
#else
#define TDB_BIND(SYM) \
   do { \
     p_ ## SYM = (SYM ## _t) dlsym(libhandle, #SYM); \
     if (!p_ ## SYM) { \
       const char *errmsg = dlerror();                                       \
       perr_printf("Error looking up %s in threaddb.so: %s\n", #SYM, errmsg); \
       return false; \
     } \
   } while (0)
#endif

#if defined(THREAD_DB_PATH)
#define THREAD_DB_PATH_STR THREAD_DB_PATH
#else
#define THREAD_DB_PATH_STR NULL
#endif

thread_db_process::td_init_t thread_db_process::p_td_init;
thread_db_process::td_ta_new_t thread_db_process::p_td_ta_new;
thread_db_process::td_ta_delete_t thread_db_process::p_td_ta_delete;
thread_db_process::td_ta_event_addr_t thread_db_process::p_td_ta_event_addr;
thread_db_process::td_ta_set_event_t thread_db_process::p_td_ta_set_event;
thread_db_process::td_ta_map_lwp2thr_t thread_db_process::p_td_ta_map_lwp2thr;
thread_db_process::td_ta_event_getmsg_t thread_db_process::p_td_ta_event_getmsg;
thread_db_process::td_thr_get_info_t thread_db_process::p_td_thr_get_info;
thread_db_process::td_thr_event_enable_t thread_db_process::p_td_thr_event_enable;
thread_db_process::td_thr_set_event_t thread_db_process::p_td_thr_set_event;
thread_db_process::td_thr_event_getmsg_t thread_db_process::p_td_thr_event_getmsg;
thread_db_process::td_thr_dbsuspend_t thread_db_process::p_td_thr_dbsuspend;
thread_db_process::td_thr_dbresume_t thread_db_process::p_td_thr_dbresume;
thread_db_process::td_thr_tls_get_addr_t thread_db_process::p_td_thr_tls_get_addr;
thread_db_process::td_thr_tlsbase_t thread_db_process::p_td_thr_tlsbase;

bool thread_db_process::tdb_loaded = false;
bool thread_db_process::tdb_loaded_result = false;

#if !defined(THREAD_DB_STATIC)
static void *dlopenThreadDB(char *path)
{
   std::string filename;
   std::string alt_filename;
   if (path) {
      filename = std::string(path);
      if (*filename.rend() != '/') {
         filename += std::string("/");
      }
      filename += std::string("libthread_db.so");
      alt_filename = std::string("libthread_db.so");
   }
   else {
      filename = std::string("libthread_db.so");
   }

   pthrd_printf("Opening thread_db with %s\n", filename.c_str());
   void *libhandle = dlopen(filename.c_str(), RTLD_LAZY);
   if (!libhandle && !alt_filename.empty()) {
   pthrd_printf("Opening thread_db with %s\n", alt_filename.c_str());
      libhandle = dlopen(alt_filename.c_str(), RTLD_LAZY);
   }
   if (!libhandle) {
      const char *errmsg = dlerror();
      perr_printf("Error loading libthread_db.so: %s\n", errmsg);
      return NULL;
   }
   return libhandle;
}

#else
static void *dlopenThreadDB(char *)
{
   return (void *) 0x1;  //Return anything non-NULL
}
#endif

bool thread_db_process::loadedThreadDBLibrary()
{
   if (tdb_loaded)
      return tdb_loaded_result;
   tdb_loaded = true;

   void *libhandle = dlopenThreadDB(THREAD_DB_PATH_STR);
   if (!libhandle)
      return false;

   TDB_BIND(td_init);
   TDB_BIND(td_ta_new);
   TDB_BIND(td_ta_delete);
   TDB_BIND(td_ta_event_addr);
   TDB_BIND(td_ta_set_event);
   TDB_BIND(td_ta_event_getmsg);
   TDB_BIND(td_ta_map_lwp2thr);
   TDB_BIND(td_thr_get_info);
   TDB_BIND(td_thr_event_enable);
   TDB_BIND(td_thr_set_event);
   TDB_BIND(td_thr_event_getmsg);
   TDB_BIND(td_thr_dbsuspend);
   TDB_BIND(td_thr_dbresume);
   TDB_BIND(td_thr_tls_get_addr);
   TDB_BIND(td_thr_tlsbase);

   pthrd_printf("Successfully loaded thread_db.so library\n");
   tdb_loaded_result = true;
   return true;
}

#ifndef CASE_RETURN_STR
#define CASE_RETURN_STR(x) case x: return #x;
#endif

static const char *tdErr2Str(td_err_e errVal) {
    switch(errVal) {
        CASE_RETURN_STR(TD_ERR)
        CASE_RETURN_STR(TD_OK)
        CASE_RETURN_STR(TD_BADKEY)
        CASE_RETURN_STR(TD_BADPH)
        CASE_RETURN_STR(TD_BADSH)
        CASE_RETURN_STR(TD_BADTA)
        CASE_RETURN_STR(TD_BADTH)
        CASE_RETURN_STR(TD_DBERR)
        CASE_RETURN_STR(TD_MALLOC)
        CASE_RETURN_STR(TD_NOAPLIC)
        CASE_RETURN_STR(TD_NOCAPAB)
        CASE_RETURN_STR(TD_NOEVENT)
        CASE_RETURN_STR(TD_NOFPREGS)
        CASE_RETURN_STR(TD_NOLIBTHREAD)
        CASE_RETURN_STR(TD_NOLWP)
        CASE_RETURN_STR(TD_NOMSG)
        CASE_RETURN_STR(TD_NOSV)
        CASE_RETURN_STR(TD_NOTHR)
        CASE_RETURN_STR(TD_NOTSD)
        CASE_RETURN_STR(TD_NOXREGS)
        CASE_RETURN_STR(TD_PARTIALREG)
        default:
            return "?";
    }
}

Event::ptr thread_db_process::decodeThreadEvent(td_event_msg_t *eventMsg, bool &async)
{
   td_thrinfo_t info;
   async = false;
#if !defined(os_freebsd)
   td_thrhandle_t *handle = const_cast<td_thrhandle_t *>(eventMsg->th_p);
#else
   td_thrhandle_t *handle = (td_thrhandle_t *)(eventMsg->th_p);
#endif
   pthrd_printf("Decoding thread event on %d\n", getPid());
   async_ret_t result = ll_fetchThreadInfo(handle, &info);
   if (result == aret_error) {
      pthrd_printf("Failed to fetch thread info\n");
      return Event::ptr();
   }
   if (result == aret_async) {
      async = true;
      pthrd_printf("Returning async from decodeThreadEvent\n");
      return Event::ptr();
   }
   Dyninst::LWP lwp = (Dyninst::LWP) info.ti_lid;
   int_thread *thr = threadPool()->findThreadByLWP(lwp); //thr may be NULL if OS doesn't support LWP events (BG/P)
   switch(eventMsg->event) {
      case TD_CREATE:
      {
         pthrd_printf("Decoded to user thread create of %d/%d\n", getPid(), lwp);

         EventNewUserThread::ptr new_ev = EventNewUserThread::ptr(new EventNewUserThread());
         new_ev->setProcess(proc());
         new_ev->setThread(thr ? thr->thread() : Thread::ptr());
         new_ev->setSyncType(Event::sync_process);
         int_eventNewUserThread *iev = new_ev->getInternalEvent();

         new_thread_data_t *thrdata = (new_thread_data_t *) malloc(sizeof(new_thread_data_t));
         thrdata->thr_handle = new td_thrhandle_t(*handle);
         thrdata->thr_info = info;
         thrdata->threadHandle_alloced = true;

         iev->raw_data = (void *) thrdata;
         iev->lwp = lwp;

         if (threadPool()->initialThread() == thr)
            initialThreadEventCreated = true;

         return new_ev;
      }
      case TD_DEATH: {
         pthrd_printf("Decoded to user thread death of %d/%d\n", getPid(), lwp);
         if (!thr) {
            perr_printf("Error.  Got thread delete event for unknown LWP\n");
            return Event::ptr();
         }

         EventUserThreadDestroy::ptr new_ev = EventUserThreadDestroy::ptr(new EventUserThreadDestroy(EventType::Pre));
         new_ev->setProcess(proc());
         new_ev->setThread(thr->thread());
         new_ev->setSyncType(Event::sync_process);

         return new_ev;
      }
      default: {
         pthrd_printf("Unimplemented libthread_db event encountered. Skipping for now.\n");
         break;
      }
   }

   return Event::ptr();
}

volatile bool thread_db_process::thread_db_initialized = false;
Mutex<> thread_db_process::thread_db_init_lock;

thread_db_process::thread_db_process(Dyninst::PID p, std::string e, std::vector<std::string> envp, std::vector<std::string> a, std::map<int, int> f) :
  int_process(p, e, a, envp, f),
  int_threadTracking(p, e, a, envp, f),
  thread_db_proc_initialized(false),
  threadAgent(NULL),
  createdThreadAgent(false),
  self(NULL),
  trigger_thread(NULL),
  hasAsyncPending(false),
  initialThreadEventCreated(false),
  setEventSet(false),
  completed_post(false),
  track_threads(ThreadTracking::getDefaultTrackThreads())
{
   if (!loadedThreadDBLibrary())
      return;
   self = new ps_prochandle();
   assert(self);
   self->thread_db_proc = this;
}

thread_db_process::thread_db_process(Dyninst::PID pid_, int_process *p) :
  int_process(pid_, p),
  int_threadTracking(pid_, p),
  thread_db_proc_initialized(false),
  threadAgent(NULL),
  createdThreadAgent(false),
  self(NULL),
  trigger_thread(NULL),
  hasAsyncPending(false),
  initialThreadEventCreated(false),
  setEventSet(false),
  completed_post(false),
  track_threads(ThreadTracking::getDefaultTrackThreads())
{
   if (!loadedThreadDBLibrary())
      return;
   self = new ps_prochandle();
   assert(self);
   self->thread_db_proc = this;
}

thread_db_process::~thread_db_process()
{
    // Free the breakpoints allocated for events
    map<Dyninst::Address, pair<int_breakpoint *, EventType> >::iterator brkptIter;
    for(brkptIter = addr2Event.begin(); brkptIter != addr2Event.end(); ++brkptIter) {
        delete brkptIter->second.first;
    }

    if (self)
       delete self;
}

async_ret_t thread_db_process::initThreadWithHandle(td_thrhandle_t *thr, td_thrinfo_t *info, LWP lwp)
{
   pthrd_printf("initThreadWithHandle on %d/%d\n", getPid(), lwp);

   td_thrinfo_t tinfo;
   if (!info) {
      async_ret_t result = ll_fetchThreadInfo(thr, &tinfo);
      if (result == aret_error) {
         pthrd_printf("Error calling ll_fetchThreadInfo from initThreadWithHandle\n");
         return aret_error;
      }
      if (result == aret_async) {
         pthrd_printf("Returning async from initThreadWithHandle\n");
         return aret_async;
      }
      info = &tinfo;
   }

   if (lwp == NULL_LWP) {
      lwp = (Dyninst::LWP) info->ti_lid;
      pthrd_printf("initThreadWithHandle found thread %d/%d\n", getPid(), lwp);
   }
   thread_db_thread *tdb_thread = dynamic_cast<thread_db_thread *>(threadPool()->findThreadByLWP(lwp));
   if (!tdb_thread) {
      perr_printf("Error.  Thread_db reports thread %d/%d, but couldn't find existing LWP\n",
                  getPid(), lwp);
      return aret_error;
   }
   if (tdb_thread->thread_initialized) {
      return aret_success;
   }
   pthrd_printf("thread_db handling thread create for %d/%d\n", getPid(), lwp);
   tdb_thread->threadHandle = thr;
   tdb_thread->tinfo = *info;
   if (info->ti_tid)
      tdb_thread->tinfo_initialized = true;

   getMemCache()->markToken(token_seteventreporting);
   async_ret_t result = tdb_thread->setEventReporting(true);
   if (result == aret_error) {
      pthrd_printf("Error in setEventReporting for %d/%d\n", getPid(), tdb_thread->getLWP());
      return aret_error;
   }
   if (result == aret_async) {
      pthrd_printf("Async return in setEventReporting for %d/%d\n", getPid(), tdb_thread->getLWP());
      return aret_async;
   }
   getMemCache()->condense();
   tdb_thread->thread_initialized = true;
   return aret_success;
}


async_ret_t thread_db_process::handleThreadAttach(td_thrhandle_t *thr, Dyninst::LWP lwp)
{
   return initThreadWithHandle(thr, NULL, lwp);
}

async_ret_t thread_db_process::initThreadDB() {
    // Q: Why isn't this in the constructor?
    // A: This function depends on the corresponding thread library being loaded
    // and this event occurs some time after process creation.

   if (!track_threads) {
      return aret_success;
   }
    // Make sure thread_db is initialized - only once for all instances
   if( !thread_db_initialized ) {
      pthrd_printf("Initializing thread_db library\n");
      thread_db_init_lock.lock();
      if( !thread_db_initialized ) {
         if (!loadedThreadDBLibrary()) {
            setLastError(err_internal, "libthread_db was not loaded");
            thread_db_init_lock.unlock();
            return aret_error;
         }
         td_err_e errVal;
         if( TD_OK != (errVal = p_td_init()) ) {
            perr_printf("Failed to initialize libthread_db: %s(%d)\n",
                        tdErr2Str(errVal), errVal);
            setLastError(err_internal, "libthread_db initialization failed");
            thread_db_init_lock.unlock();
            return aret_error;
          }
         pthrd_printf("Sucessfully initialized thread_db\n");
         thread_db_initialized = true;
      }
      thread_db_init_lock.unlock();
   }
   if (thread_db_proc_initialized) {
      return aret_success;
   }

   getMemCache()->markToken(token_init);
   // Create the thread agent
   td_err_e errVal;
   if (!createdThreadAgent)
   {
      pthrd_printf("Creating threadAgent\n");
      errVal = p_td_ta_new(self, &threadAgent);
      switch(errVal) {
         case TD_OK:
            pthrd_printf("Retrieved thread agent from thread_db\n");
            break;
         case TD_NOLIBTHREAD:
            pthrd_printf("Debuggee isn't multithreaded at this point, libthread_db not enabled\n");
            return aret_success;
         case TD_ERR:
            if (getMemCache()->hasPendingAsync()) {
               pthrd_printf("Postponing thread_db initialization for async\n");
               return aret_async;
            }
            //FALLTHROUGH
         default:
            perr_printf("Failed to create thread agent: %s(%d)\n",
                        tdErr2Str(errVal), errVal);
            thread_db_proc_initialized = true;
            setLastError(err_internal, "Failed to create libthread_db agent");
            return aret_error;
      }
      createdThreadAgent = true;
   }

   bool hasAsync = false;
   set<pair<td_thrhandle_t *, LWP> > all_handles;
   for (int_threadPool::iterator i = threadPool()->begin(); i != threadPool()->end(); i++) {
      thread_db_thread *tdb_thread = dynamic_cast<thread_db_thread *>(*i);

      if (tdb_thread->threadHandle_alloced) {
         all_handles.insert(pair<td_thrhandle_t *, LWP>(tdb_thread->threadHandle, tdb_thread->getLWP()));
         continue;
      }

      if (!tdb_thread->threadHandle) {
         tdb_thread->threadHandle = new td_thrhandle_t;
         memset(tdb_thread->threadHandle, 0, sizeof(td_thrhandle_t));
      }

      pthrd_printf("lwp2thr on %d/%d\n", getPid(), tdb_thread->getLWP());
      errVal = p_td_ta_map_lwp2thr(getThreadDBAgent(), tdb_thread->getLWP(), tdb_thread->threadHandle);
      if (errVal != TD_OK) {
         if (getMemCache()->hasPendingAsync()) {
            pthrd_printf("Hit async during lwp2thr\n");
            hasAsync = true;
            continue;
         }
         perr_printf("Failed to map LWP %d to thread_db thread: %s(%d)\n",
                     tdb_thread->getLWP(), tdErr2Str(errVal), errVal);
         setLastError(err_internal, "Failed to get thread_db thread handle");
         delete tdb_thread->threadHandle;
         tdb_thread->threadHandle = NULL;
         continue;
      }
      pthrd_printf("Successful lwp2thr on %d/%d\n", getPid(), tdb_thread->getLWP());
      tdb_thread->threadHandle_alloced = true;
      all_handles.insert(pair<td_thrhandle_t *, LWP>(tdb_thread->threadHandle, tdb_thread->getLWP()));
   }
   if (hasAsync) {
      pthrd_printf("Postponing lwp2thr for async\n");
      return aret_async;
   }

   pthrd_printf("handleThreadAttach for %d threads\n", (int) all_handles.size());
   for (set<pair<td_thrhandle_t *, LWP> >::iterator i = all_handles.begin(); i != all_handles.end(); i++)
   {
      async_ret_t result = handleThreadAttach(i->first, i->second);
      if (result == aret_error) {
         perr_printf("Error handling thread_db attach\n");
         return aret_error;
      }
      if (result == aret_async) {
         pthrd_printf("handleThreadAttach returned async in initThreadDB\n");
         return aret_async;
      }
   }

   // Enable all events
   td_thr_events_t eventMask;
#if defined(td_event_fillset)
   //Macro on GNU libc
   td_event_fillset(&eventMask);
#elif defined(os_freebsd)
   //Inline header file function on FreeBSD
   td_event_fillset(&eventMask);
#else
//Need to make td_event_fillset a function pointer if this hits
#error td_event_fillset is not a macro on this platform
#endif

   if (!setEventSet) {
      getMemCache()->markToken(token_setevent);
      errVal = p_td_ta_set_event(threadAgent, &eventMask);
      if( errVal != TD_OK && getMemCache()->hasPendingAsync()) {
         pthrd_printf("Async return from td_ta_set_event in initThreadDB\n");
         return aret_async;
      }
      setEventSet = true;
      getMemCache()->condense();
      if (errVal != TD_OK) {
         perr_printf("Failed to enable events: %s(%d)\n",
                     tdErr2Str(errVal), errVal);
         setLastError(err_internal, "Failed to enable libthread_db events");
         thread_db_proc_initialized = true;
         return aret_error;
      }
   }

    // Determine the addresses for all events
   td_event_e allEvents[] = { TD_CATCHSIG, TD_CONCURRENCY, TD_CREATE,
                              TD_DEATH, TD_IDLE, TD_LOCK_TRY, TD_PREEMPT, TD_PRI_INHERIT,
                              TD_READY, TD_REAP, TD_SLEEP, TD_SWITCHFROM, TD_SWITCHTO,
                              TD_TIMEOUT };

   for(unsigned i = 0; i < (sizeof(allEvents)/sizeof(td_event_e)); ++i) {
      td_notify_t notifyResult;
      errVal = p_td_ta_event_addr(threadAgent, allEvents[i], &notifyResult);

      // This indicates that the event isn't supported
      if( TD_OK != errVal ) continue;

      assert( notifyResult.type == NOTIFY_BPT && "Untested notify type" );

      EventType newEvent;
      switch(allEvents[i]) {
         case TD_CREATE:
            newEvent = EventType(EventType::Post, EventType::ThreadCreate);
            pthrd_printf("Installing breakpoint for thread creation events\n");
            break;
         case TD_DEATH:
            newEvent = EventType(EventType::Post, EventType::ThreadDestroy);
            pthrd_printf("Installing breakpoint for thread destroy events\n");
            break;
         default:
            pthrd_printf("Unimplemented libthread_db event encountered. Skipping for now.\n");
            continue;
      }

      Address addr = (Address) notifyResult.u.bptaddr;
      pthrd_printf("Received address of 0x%lx for breakpoint, checking platform conversion\n",
		   addr);
      if( !plat_convertToBreakpointAddress(addr, triggerThread()) ) {
         perr_printf("Failed to determine breakpoint address\n");
         setLastError(err_internal, "Failed to install new thread_db event breakpoint");
         thread_db_proc_initialized = true;
         return aret_error;
      }
      pthrd_printf("Post-conversion, using address of 0x%lx\n", addr);
#if defined(os_freebsd)
      notifyResult.u.bptaddr = (psaddr_t) addr;
#else
      notifyResult.u.bptaddr = (void *) addr;
#endif
      int_breakpoint *newEventBrkpt = new int_breakpoint(Breakpoint::ptr());
      newEventBrkpt->setProcessStopper(true);
      if( !addBreakpoint(addr, newEventBrkpt))
      {
         perr_printf("Failed to install new event breakpoint\n");
         setLastError(err_internal, "Failed to install new thread_db event breakpoint");
         delete newEventBrkpt;
         thread_db_proc_initialized = true;
         return aret_error;
         }

      pair<map<Dyninst::Address, pair<int_breakpoint *, EventType> >::iterator, bool> insertIter;
      insertIter = addr2Event.insert(make_pair(addr, make_pair(newEventBrkpt, newEvent)));

      assert( insertIter.second && "event breakpoint address not unique" );
   }

   thread_db_proc_initialized = true;
   return aret_success;
}

void thread_db_process::freeThreadDBAgent() {
    // This code cannot be in the destructor because it makes use of
    // the proc_service interface and this makes calls to functions
    // that are pure virtual in this class.
    //
    // A possible, better solution would be to make the functions static
    // but we lose all the convenience of pure virtual functions
    //
    // At any rate, this function should be called from a derived class'
    // destructor for the time being.

    if( thread_db_initialized && threadAgent ) {
        td_err_e errVal = p_td_ta_delete(threadAgent);
        if( TD_OK != errVal ) {
            perr_printf("Failed to delete thread agent: %s(%d)\n",
                    tdErr2Str(errVal), errVal);
        }
        assert( TD_OK == errVal && "Failed to delete thread agent" );
        threadAgent = NULL;
    }
}

const char *thread_db_process::getThreadLibName(const char *)
{
   return "";
}

bool thread_db_process::decodeTdbLWPExit(EventLWPDestroy::ptr lwp_ev)
{
   thread_db_thread *db_thread = dynamic_cast<thread_db_thread *>(lwp_ev->getThread()->llthrd());
   assert(db_thread);

   if (db_thread->destroyed || !db_thread->thread_initialized)
      return false;

   pthrd_printf("Decoded LWP exit without thread exit on %d/%d.  Faking thread exit event\n",
                db_thread->llproc()->getPid(), db_thread->getLWP());

   EventUserThreadDestroy::ptr new_ev = EventUserThreadDestroy::ptr(new EventUserThreadDestroy(EventType::Post));
   new_ev->setProcess(db_thread->llproc()->proc());
   new_ev->setThread(db_thread->thread());
   new_ev->setSyncType(Event::async);
   lwp_ev->addSubservientEvent(new_ev);

   return true;
}

async_ret_t thread_db_process::decodeTdbBreakpoint(EventBreakpoint::ptr bp)
{
    // Decoding thread_db events needs to be a two-step process:
    // 1) Create events depending on the breakpoint address
    //    Don't get events from thread_db as this can write to memory
    //    and threads could currently be running -- introduces some race
    //    conditions where the running threads could be modifying data
    //    structures thread_db is accessing
    //    Just create placeholder events that can later be filled in with
    //    more information
    // 2) Get events from thread_db in the handler for the event, at this
    //    point all threads are stopped and it is safe to make changes to
    //    memory because the parent event is a breakpoint and requires
    //    that all threads are stopped
    Dyninst::Address addr = bp->getAddress();

    // Determine what type of event occurs at the specified address
    map<Dyninst::Address, pair<int_breakpoint *, EventType> >::iterator addrIter;
    addrIter = addr2Event.find(addr);
    if (addrIter == addr2Event.end())
       return aret_error;

    vector<Event::ptr> threadEvents;

    EventType::Code ecode = addrIter->second.second.code();
    pthrd_printf("Address 0x%lx corresponds to a thread %s breakpoint.\n",
                 addr, ecode == EventType::ThreadCreate ? "create" : "destroy");
    switch(ecode) {
       case EventType::ThreadCreate:
       case EventType::ThreadDestroy:
          threadEvents.push_back(EventThreadDB::ptr(new EventThreadDB()));
          break;
       default:
          pthrd_printf("Failed to decode any thread events due to the breakpoint\n");
          return aret_error;
    }

    for (vector<Event::ptr>::iterator i = threadEvents.begin(); i != threadEvents.end(); i++) {
       Event::ptr ev = *i;
       if (!ev->getThread())
          ev->setThread(bp->getThread());
       if (!ev->getProcess())
          ev->setProcess(proc());
       if (ev->getSyncType() == Event::unset)
          ev->setSyncType(Event::sync_process);
       bp->addSubservientEvent(ev);
    }
    bp->setSuppressCB(true);
    return aret_success;
}

td_thragent_t *thread_db_process::getThreadDBAgent() {
    return threadAgent;
}

static string stripLibraryName(const char *libname)
{
   boost::filesystem::path p(libname);
   return p.filename().string();
}

ps_err_e thread_db_process::getSymbolAddr(const char *objName, const char *symName,
        psaddr_t *symbolAddr)
{
    SymReader *objSymReader = NULL;
    int_library *lib = NULL;

    if (plat_isStaticBinary()) {
       // For static executables, we need to search the executable instead of the
       // thread library.
       assert(memory()->libs.size() == 1);
       lib = *memory()->libs.begin();
    }
    else
    {
       // FreeBSD implementation doesn't set objName
       const char *name_c = objName ? objName : getThreadLibName(symName);
       std::string name = stripLibraryName(name_c);

       for (set<int_library *>::iterator i = memory()->libs.begin(); i != memory()->libs.end(); i++) {
          int_library *l = *i;
          if (stripLibraryName(l->getName().c_str()) ==  name) {
             lib = l;
             break;
          }
       }
    }

    if( NULL == lib ) {
       pthrd_printf("Didn't yet find loaded thread library\n");
       return PS_ERR;
    }

    objSymReader = getSymReader()->openSymbolReader(lib->getName());
    if( NULL == objSymReader ) {
        perr_printf("Failed to open symbol reader for %s\n",
                    lib->getName().c_str());
        setLastError(err_internal, "Failed to open executable for symbol reading");
        return PS_ERR;
    }

    Symbol_t lookupSym = objSymReader->getSymbolByName(string(symName));

    if( !objSymReader->isValidSymbol(lookupSym) ) {
       pthrd_printf("thread_db getSymbolAddr(%s, %s) = none\n", objName ? objName : "NULL",
                    symName ? symName : "NULL");
       return PS_NOSYM;
    }

    Address tmp = lib->getAddr() + objSymReader->getSymbolOffset(lookupSym);
    if (getAddressWidth() == 4) {
       tmp &= 0xffffffff;
    }

    *symbolAddr = (psaddr_t) tmp;

    pthrd_printf("thread_db getSymbolAddr(%s, %s) = %p\n", objName ? objName : "NULL",
                 symName ? symName : "NULL", (void *) *symbolAddr);
    return PS_OK;
}


async_ret_t thread_db_process::post_create(std::set<response::ptr> &async_responses)
{
   async_ret_t result;
   if (!completed_post) {
      result = int_process::post_create(async_responses);
      if (result != aret_success)
         return result;
      completed_post = true;
   }

   err_t saved_error = getLastError();
   const char *last_err_msg = getLastErrorMsg();

   getMemCache()->setSyncHandling(true);
   for (;;) {
      result = initThreadDB();
      if (result != aret_async)
         break;
      getMemCache()->getPendingAsyncs(async_responses);
      return aret_async;
   }
   getMemCache()->setSyncHandling(false);

   setLastError(saved_error, last_err_msg);
   return aret_success; //Swallow these errors, thread_db failure does not bring down rest of startup
}

async_ret_t thread_db_process::post_attach(bool wasDetached, set<response::ptr> &aresps) {
   async_ret_t result;
   if (!completed_post) {
      result = int_process::post_attach(wasDetached, aresps);
      if (result != aret_success)
         return result;
      completed_post = true;
   }

   err_t saved_error = getLastError();
   const char *last_err_msg = getLastErrorMsg();

   getMemCache()->setSyncHandling(true);
   for (;;) {
      result = initThreadDB();
      if (result != aret_async)
         break;
      getMemCache()->getPendingAsyncs(aresps);
      return aret_async;
   }
   getMemCache()->setSyncHandling(false);

   setLastError(saved_error, last_err_msg);
   return aret_success; //Swallow these errors, thread_db failure does not bring down rest of startup
}

#if 0
#warning TODO fix detach part in post attach rewrite
bool thread_db_process::post_attach(bool wasDetached) {
    if( !int_process::post_attach(wasDetached) ) return false;

    if( !wasDetached ) {
        return initThreadDB();
    }else{
        // Need to initialize all new threads
        bool success = true;
        td_err_e errVal;
        for (int_threadPool::iterator i = threadPool()->begin(); i != threadPool()->end(); i++) {
           thread_db_thread *tdb_thread = static_cast<thread_db_thread *>(*i);
           if( tdb_thread->thread_initialized ) continue;

           tdb_thread->threadHandle = new td_thrhandle_t;

           errVal = td_ta_map_lwp2thr(getThreadDBAgent(), tdb_thread->getLWP(), tdb_thread->threadHandle);
           if (errVal != TD_OK) {
              perr_printf("Failed to map LWP %d to thread_db thread: %s(%d)\n",
                          tdb_thread->getLWP(), tdErr2Str(errVal), errVal);
              setLastError(err_internal, "Failed to get thread_db thread handle");
              delete tdb_thread->threadHandle;
              tdb_thread->threadHandle = NULL;
              success = false;
              continue;
           }
           tdb_thread->threadHandle_alloced = true;

           if( !handleThreadAttach(tdb_thread->threadHandle) ) {
               perr_printf("Error handling thread_db attach\n");
               success = false;
           }
        }

        return success;
    }
}
#endif

bool thread_db_process::isSupportedThreadLib(string libName) {
   return (libName.find("libpthread") != string::npos);
}

void thread_db_process::addThreadDBHandlers(HandlerPool *hpool) {
   static bool initialized = false;
   static ThreadDBLibHandler *libHandler = NULL;
   static ThreadDBCreateHandler *createHandler = NULL;
   static ThreadDBDestroyHandler *destroyHandler = NULL;
   static ThreadDBDispatchHandler *dispatchHandler = NULL;
   if( !initialized ) {
      libHandler = new ThreadDBLibHandler();
      createHandler = new ThreadDBCreateHandler();
      destroyHandler = new ThreadDBDestroyHandler();
      dispatchHandler = new ThreadDBDispatchHandler();
      initialized = true;
   }
   hpool->addHandler(libHandler);
   hpool->addHandler(createHandler);
   hpool->addHandler(destroyHandler);
   hpool->addHandler(dispatchHandler);
}

bool thread_db_process::plat_getLWPInfo(lwpid_t, void *)
{
   perr_printf("Attempt to use unsupported plat_getLWPInfo\n");
   return false;
}

bool thread_db_process::plat_supportThreadEvents()
{
   if (!loadedThreadDBLibrary()) {
      return false;
   }
   return true;
}

bool thread_db_thread::plat_convertToSystemRegs(const int_registerPool &,
                                                unsigned char *, bool)
{
    return true;
}

int_thread *thread_db_process::triggerThread() const
{
   return trigger_thread;
}

async_ret_t thread_db_process::ll_fetchThreadInfo(td_thrhandle_t *th, td_thrinfo_t *info)
{
   td_err_e result = thread_db_process::p_td_thr_get_info(th, info);
   if (result != TD_OK) {
      if (getMemCache()->hasPendingAsync()) {
         pthrd_printf("Async return from td_thr_get_info in ll_fetchThreadInfo\n");
         return aret_async;
      }
      perr_printf("Error calling td_thr_get_info: %s (%d)\n", tdErr2Str(result), (int) result);
      return aret_error;
   }
   pthrd_printf("Successful ll_fetchThreadInfo for handle %p - tid = %lu, lid = %lu\n", (void*)th, (unsigned long) info->ti_tid, (unsigned long) info->ti_lid);
   return aret_success;
}

ThreadDBDispatchHandler::ThreadDBDispatchHandler() :
   Handler("thread_db Dispatch Handler")
{
}

ThreadDBDispatchHandler::~ThreadDBDispatchHandler()
{
}

int ThreadDBDispatchHandler::getPriority() const
{
   return Handler::PostPlatformPriority;
}

Handler::handler_ret_t ThreadDBDispatchHandler::handleEvent(Event::ptr ev)
{
   /**
    * All we know is that we got a thread_db breakpoint, but we don't
    * know whether that was a thread create/destroy, or any information
    * about those events.  We'll collect that info here, then add
    * UserThreadCreate or UserThreadDestroy events as 'late' events
    * (means they were generated at handle time) to this event.
    **/
   pthrd_printf("At top of ThreadDB Dispatch handler\n");
   EventThreadDB::ptr etdb = ev->getEventThreadDB();
   assert(etdb);
   int_eventThreadDB *int_ev = etdb->getInternal();
   assert(int_ev);

   thread_db_process *proc = dynamic_cast<thread_db_process *>(etdb->getProcess()->llproc());
   assert(proc);

   if (proc->dispatch_event && proc->dispatch_event != etdb) {
      //We don't need to handle a new dispatch event if another is in
      //progress.  We'll drop the second.
      pthrd_printf("Dropping dispatch event, another is in progress\n");
      return ret_success;
   }
   proc->dispatch_event = etdb;

   if (!int_ev->completed_new_evs) {
      async_ret_t result = proc->getEventForThread(int_ev);
      if (result == aret_async) {
         pthrd_printf("getEventForThread returned async\n");
         return ret_async;
      }
      int_ev->completed_new_evs = true;
      if (result == aret_error) {
         pthrd_printf("getEventForThread returned error\n");
         proc->dispatch_event = EventThreadDB::ptr();
         return ret_error;
      }
   }

   thread_db_thread *main_thread = dynamic_cast<thread_db_thread *>(proc->threadPool()->initialThread());
   if (main_thread->tinfo_initialized)
      proc->initialThreadEventCreated = true;

   if (!proc->initialThreadEventCreated) {
      pthrd_printf("Creating thread event for main thread\n");

      if (!main_thread->threadHandle) {
         main_thread->threadHandle = new td_thrhandle_t;
         bzero(&main_thread->threadHandle, sizeof(td_thrhandle_t));
         main_thread->threadHandle_alloced = true;
      }

      int td_result = thread_db_process::p_td_ta_map_lwp2thr(proc->getThreadDBAgent(), main_thread->getLWP(), main_thread->threadHandle);
      if (td_result == TD_ERR && proc->getMemCache()->hasPendingAsync()) {
         pthrd_printf("async return from td_ta_map_lwp2thr while creating event for main thread\n");
         std::set<response::ptr> resps;
         proc->getMemCache()->getPendingAsyncs(resps);
         proc->handlerPool()->notifyOfPendingAsyncs(resps, ev);
         return Handler::ret_async;
      }
      else if (td_result == TD_ERR) {
         perr_printf("Error return from td_ta_map_lwp2thr while creating event for main thread\n");
         proc->dispatch_event = EventThreadDB::ptr();
         return ret_error;
      }

      td_thrinfo_t tinfo;
      bzero(&tinfo, sizeof(td_thrinfo_t));
      async_ret_t result = proc->ll_fetchThreadInfo(main_thread->threadHandle, &tinfo);
      if (result == aret_async) {
         pthrd_printf("Async return during ll_fetchThreadInfo for main thread\n");
         std::set<response::ptr> resps;
         proc->getMemCache()->getPendingAsyncs(resps);
         proc->handlerPool()->notifyOfPendingAsyncs(resps, ev);
         return Handler::ret_async;
      }
      if (result == aret_error) {
         pthrd_printf("Error return during ll_fetchThreadInfo for main thread\n");
         proc->dispatch_event = EventThreadDB::ptr();
         return Handler::ret_error;
      }
      if (tinfo.ti_tid) {
         new_thread_data_t *thrdata = (new_thread_data_t *) malloc(sizeof(new_thread_data_t));
         thrdata->thr_handle = main_thread->threadHandle;
         thrdata->thr_info = tinfo;
         thrdata->threadHandle_alloced = main_thread->threadHandle_alloced;

         EventNewUserThread::ptr new_ev = EventNewUserThread::ptr(new EventNewUserThread());
         new_ev->setProcess(proc->proc());
         new_ev->setThread(main_thread->thread());
         new_ev->setSyncType(Event::sync_process);
         new_ev->getInternalEvent()->thr = main_thread;
         new_ev->getInternalEvent()->lwp = main_thread->getLWP();
         new_ev->getInternalEvent()->raw_data = (void *) thrdata;
         proc->initialThreadEventCreated = true;
         int_ev->new_evs.insert(new_ev);
         pthrd_printf("Success creating event for main thread\n");
      }
      else {
         pthrd_printf("TID info for main thread not ready yet\n");
      }
   }
   pthrd_printf("Got %u events, adding as late events\n", (unsigned int) int_ev->new_evs.size());
   for (set<Event::ptr>::iterator i = int_ev->new_evs.begin(); i != int_ev->new_evs.end(); i++) {
      proc->handlerPool()->addLateEvent(*i);
   }
   proc->dispatch_event = EventThreadDB::ptr();
   return ret_success;
}

void ThreadDBDispatchHandler::getEventTypesHandled(std::vector<EventType> &etypes)
{
   etypes.push_back(EventType(EventType::None, EventType::ThreadDB));
}

ThreadDBLibHandler::ThreadDBLibHandler() :
    Handler("thread_db Library Handler")
{
}

ThreadDBLibHandler::~ThreadDBLibHandler()
{
}

Handler::handler_ret_t ThreadDBLibHandler::handleEvent(Event::ptr ev) {
   if (!thread_db_process::loadedThreadDBLibrary()) {
      pthrd_printf("Failed to load thread_db.  Not running handlers\n");
      return Handler::ret_success;
   }
   EventLibrary::const_ptr libEv = ev->getEventLibrary();
   thread_db_process *proc = dynamic_cast<thread_db_process *>(ev->getProcess()->llproc());

   //Check if we need to clear the library->tls cache on library unload
   const set<Library::ptr> &rmLibs = libEv->libsRemoved();
   set<int_library *> &cached_libs = proc->libs_with_cached_tls_areas;
   for (set<Library::ptr>::const_iterator i = rmLibs.begin(); i != rmLibs.end(); i++) {
      int_library *ll_lib = (*i)->debug();
      if (cached_libs.find(ll_lib) == cached_libs.end())
         continue;
      pthrd_printf("Removing library %s from internal tls cached on unload\n",
                   ll_lib->getName().c_str());
      for (int_threadPool::iterator j = proc->threadPool()->begin(); j != proc->threadPool()->end(); j++) {
         thread_db_thread *thrd = dynamic_cast<thread_db_thread *>(*j);
         if (!thrd)
            continue;
         map<int_library*, Address>::iterator k = thrd->cached_tls_areas.find(ll_lib);
         if (k == thrd->cached_tls_areas.end())
            continue;
         thrd->cached_tls_areas.erase(k);
      }
   }

   //Check if thread library is being loaded, init thread_db if so
   const set<Library::ptr> &addLibs = libEv->libsAdded();
   set<Library::ptr>::iterator libIter;
   for( libIter = addLibs.begin(); libIter != addLibs.end(); ++libIter ) {
      if( ! proc->isSupportedThreadLib((*libIter)->getName()) )
         continue;

      pthrd_printf("Enabling thread_db support for pid %d\n",
                   proc->getPid());
      async_ret_t ret = proc->initThreadDB();

      if (ret == aret_error) {
         pthrd_printf("Failed to initialize thread_db for pid %d\n",
                      proc->getPid());
         return Handler::ret_error;
      }
      else if (ret == aret_success) {
         return Handler::ret_success;
      }
      else if (ret == aret_async) {
         std::set<response::ptr> resps;
         proc->getMemCache()->getPendingAsyncs(resps);
         proc->handlerPool()->notifyOfPendingAsyncs(resps, ev);
         return Handler::ret_async;
      }
   }

   return Handler::ret_success;
}

int ThreadDBLibHandler::getPriority() const {
    return PostPlatformPriority;
}

void ThreadDBLibHandler::getEventTypesHandled(vector<EventType> &etypes) {
    etypes.push_back(EventType(EventType::None, EventType::Library));
}

ThreadDBCreateHandler::ThreadDBCreateHandler() :
    Handler("thread_db New Thread Handler")
{
}

ThreadDBCreateHandler::~ThreadDBCreateHandler()
{
}

int ThreadDBCreateHandler::getPriority() const
{
   //After dispatch handler, which runs at PostPlatformPriority
   return Handler::PostPlatformPriority + 1;
}

Handler::handler_ret_t ThreadDBCreateHandler::handleEvent(Event::ptr ev) {
  pthrd_printf("ThreadDBCreateHandler::handleEvent\n");
   if (!thread_db_process::loadedThreadDBLibrary()) {
      pthrd_printf("Failed to load thread_db.  Not running handlers");
      return Handler::ret_success;
   }

   EventNewUserThread::ptr threadEv = ev->getEventNewUserThread();
   thread_db_process *tdb_proc = dynamic_cast<thread_db_process *>(threadEv->getProcess()->llproc());
   thread_db_thread *tdb_thread = dynamic_cast<thread_db_thread *>(threadEv->getNewThread()->llthrd());

   pthrd_printf("ThreadDBCreateHandler::handleEvent for %d/%d\n", tdb_proc->getPid(), tdb_thread->getLWP());
   if (threadEv->getInternalEvent()->needs_update) {
      pthrd_printf("Updating user thread data for %d/%d in thread_db create handler\n",
                   tdb_proc->getPid(), tdb_thread->getLWP());
      assert(tdb_proc);
      new_thread_data_t *thrdata = (new_thread_data_t *) threadEv->getInternalEvent()->raw_data;

      async_ret_t result = tdb_proc->initThreadWithHandle(thrdata->thr_handle, &thrdata->thr_info, NULL_LWP);
      if (result == aret_error) {
         pthrd_printf("ThreadDBCreateHandler returning error\n");
         return Handler::ret_error;
      }
      if (result == aret_async) {
         pthrd_printf("ThreadDBCreateHandler returning async\n");
         return Handler::ret_async;
      }
      if (thrdata->threadHandle_alloced) tdb_thread->threadHandle_alloced = true;
   }

   return Handler::ret_success;
}

void ThreadDBCreateHandler::getEventTypesHandled(vector<EventType> &etypes) {
   etypes.push_back(EventType(EventType::Any, EventType::UserThreadCreate));
}

ThreadDBDestroyHandler::ThreadDBDestroyHandler() :
   Handler("thread_db Destroy Handler")
{
}

ThreadDBDestroyHandler::~ThreadDBDestroyHandler()
{
}

int ThreadDBDestroyHandler::getPriority() const
{
   //After dispatch handler, which runs at PostPlatformPriority
   return Handler::PostPlatformPriority + 1;
}

Handler::handler_ret_t ThreadDBDestroyHandler::handleEvent(Event::ptr ev) {
   if (!thread_db_process::loadedThreadDBLibrary()) {
      pthrd_printf("Failed to load thread_db.  Not running handlers\n");
      return Handler::ret_success;
   }
   thread_db_process *proc = dynamic_cast<thread_db_process *>(ev->getProcess()->llproc());
   thread_db_thread *thrd = dynamic_cast<thread_db_thread *>(ev->getThread()->llthrd());

   if(thrd) {
      pthrd_printf("Running ThreadDBDestroyHandler on %d/%d\n", proc->getPid(), thrd->getLWP());
      thrd->markDestroyed();
   }

   return Handler::ret_success;
}

void ThreadDBDestroyHandler::getEventTypesHandled(vector<EventType> &etypes) {
    etypes.push_back(EventType(EventType::Any, EventType::UserThreadDestroy));
}

thread_db_thread::thread_db_thread(int_process *p, Dyninst::THR_ID t, Dyninst::LWP l) :
   int_thread(p, t, l),
   threadHandle(NULL),
   destroyed(false),
   tinfo_initialized(false),
   thread_initialized(false),
   threadHandle_alloced(false),
   enabled_event_reporting(false)
{
   memset(&tinfo, 0, sizeof(tinfo));
}

thread_db_thread::~thread_db_thread()
{
   if (threadHandle_alloced)
      delete threadHandle;
}

bool thread_db_thread::initThreadHandle() {
    if( NULL != threadHandle ) return true;

    thread_db_process *lproc = dynamic_cast<thread_db_process *>(llproc());
    if( NULL == lproc->getThreadDBAgent() ) return false;

    threadHandle = new td_thrhandle_t;

    td_err_e errVal = thread_db_process::p_td_ta_map_lwp2thr(lproc->getThreadDBAgent(),
                                          lwp, threadHandle);
    if( TD_OK != errVal ) {
        perr_printf("Failed to map LWP %d to thread_db thread: %s(%d)\n",
                lwp, tdErr2Str(errVal), errVal);
        setLastError(err_internal, "Failed to get thread_db thread handle");
        delete threadHandle;
        threadHandle = NULL;
        return false;
    }
    threadHandle_alloced = true;

    return true;
}

async_ret_t thread_db_process::getEventForThread(int_eventThreadDB *iev) {
   // These specific calls into thread_db can modify the memory of the process
   // and can introduce some race conditions if the platform allows memory reads
   // while some threads are running
   assert( threadPool()->allHandlerStopped() );

   // We need to save thread_db generated events because we need to use the
   // process-level event retrieval call to get thread creation events (at
   // least on some platforms).

   bool local_async = false;
   td_err_e msgErr = TD_OK;

   if (!iev->completed_getmsgs) {
      getMemCache()->markToken(token_getmsg);
      vector<td_event_msg_t> msgs;
      vector<td_thrhandle_t> handles;

      td_event_msg_t evMsg;

      for (;;) {
         msgErr = p_td_ta_event_getmsg(threadAgent, &evMsg);
         if (msgErr != TD_OK) {
            if (getMemCache()->hasPendingAsync()) {
               pthrd_printf("Async return in getEventForThread from td_ta_event_getmsg\n");
               return aret_async;
            }
            else if (msgErr == TD_NOMSG) {
               pthrd_printf("No more messages ready in thread_db\n");
               break;
            }
            else {
               perr_printf("Error reading messages from thread_db\n");
               return aret_error;
            }
         }
         msgs.push_back(evMsg);
         //GLIBC's thread_db returns a pointer to a static variable inside
         // evMsg.  Thus subsequent calls will override the data from prior
         // calls.  Annoying.  Make a copy of the th_p in handles to avoid
         // this problem.
         handles.push_back(*evMsg.th_p);
      }
      pthrd_printf("Received %lu messages from thread_db on %d\n", (unsigned long)msgs.size(), getPid());
      iev->msgs = msgs;
      iev->handles = handles;
      iev->completed_getmsgs = true;
   }

   getMemCache()->condense();

   for (int i=iev->msgs.size()-1; i>=0; i--) {
      td_event_msg_t &evMsg = iev->msgs[i];
      evMsg.th_p = & iev->handles[i];
      Event::ptr newEvent = decodeThreadEvent(&evMsg, local_async);
      if (local_async) {
         pthrd_printf("Async return from decodeThreadEvent\n");
         return aret_async;
      }
      if (newEvent)
         iev->new_evs.insert(newEvent);
      iev->msgs.pop_back();
      iev->handles.pop_back();
   }

   return aret_success;
}

bool thread_db_process::setTrackThreads(bool b, std::set<std::pair<int_breakpoint *, Address> > &bps,
                                                 bool &add_bp)
{
   if (b == track_threads) {
      pthrd_printf("User wants to %s thread_db on %d, which is already done.  Leaving in same state\n",
                   b ? "enable" : "disable", getPid());
      return true;
   }
   track_threads = b;

   std::map<Address, pair<int_breakpoint *, EventType> >::iterator i;
   for (i = addr2Event.begin(); i != addr2Event.end(); i++) {
      Address addr = i->first;
      int_breakpoint *bp = i->second.first;
      bps.insert(make_pair(bp, addr));
   }

   add_bp = b;
   return true;
}

bool thread_db_process::isTrackingThreads()
{
   return track_threads;
}

bool thread_db_process::refreshThreads()
{
   EventThreadDB::ptr ev = EventThreadDB::ptr(new EventThreadDB());
   ev->setSyncType(Event::async);
   ev->setProcess(proc());
   ev->setThread(threadPool()->initialThread()->thread());
   mbox()->enqueue(ev);
   return true;
}

int thread_db_process::threaddb_getPid()
{
   return getPid();
}

async_ret_t thread_db_process::plat_calcTLSAddress(int_thread *thread, int_library *lib, Offset off,
                                                   Address &outaddr, set<response::ptr> &resps_)
{
   thread_db_thread *thrd = dynamic_cast<thread_db_thread *>(thread);
   if (!thrd || !thrd->initThreadHandle()) {
      perr_printf("Thread_db not supported on thread %d/%d\n", getPid(), thread->getLWP());
      setLastError(err_unsupported, "TLS Operations not supported on this thread\n");
      return aret_error;
   }
   bool is_staticbinary = plat_isStaticBinary();

   if ((!is_staticbinary && !p_td_thr_tls_get_addr) ||
       (is_staticbinary && !p_td_thr_tlsbase)) {
      perr_printf("TLS operations not supported in this version of thread_db\n");
      setLastError(err_unsupported, "TLS Operations not supported on this system\n");
      return aret_error;
   }

   map<int_library *, Address>::iterator i = thrd->cached_tls_areas.find(lib);
   if (i != thrd->cached_tls_areas.end()) {
      outaddr = i->second + off;
      return aret_success;
   }

   getMemCache()->setSyncHandling(true);
   void *tls_base = NULL;
   td_err_e err;

   if (!is_staticbinary)
      err = p_td_thr_tls_get_addr(thrd->threadHandle, (void *) lib->mapAddress(),
                                  0, &tls_base);
   else
      err = p_td_thr_tlsbase(thrd->threadHandle, 1, &tls_base);

   if (err != TD_OK && getMemCache()->hasPendingAsync()) {
      pthrd_printf("Async return in plat_calcTLSAddress\n");
      getMemCache()->getPendingAsyncs(resps_);
      return aret_async;
   }
   getMemCache()->setSyncHandling(false);
   if (err != TD_OK) {
      perr_printf("Error [%s] return from td_thr_tls_get_addr from thread_db\n", tdErr2Str(err));
      return aret_error;
   }

   Address tls_base_addr = (Address) tls_base;
   thrd->cached_tls_areas[lib] = tls_base_addr;
   libs_with_cached_tls_areas.insert(lib);
   outaddr = tls_base_addr + off;
   return aret_success;
}

async_ret_t thread_db_thread::setEventReporting(bool on) {
    if( !initThreadHandle() ) return aret_error;
    if (enabled_event_reporting == on) return aret_success;

    pthrd_printf("Enabled thread_db events for LWP %d\n", lwp);
    td_err_e errVal = thread_db_process::p_td_thr_event_enable(threadHandle, (on ? 1 : 0 ));
    if (errVal != TD_OK && llproc()->getMemCache()->hasPendingAsync()) {
       pthrd_printf("td_thr_event_enable returned async in setEventReporting\n");
       return aret_async;
    }
    enabled_event_reporting = on;
    if (errVal != TD_OK) {
       perr_printf("Failed to enable events for LWP %d: %s(%d)\n",
                   lwp, tdErr2Str(errVal), errVal);
       setLastError(err_internal, "Failed to enable thread_db events");
       return aret_error;
    }

    return aret_success;
}

bool thread_db_thread::fetchThreadInfo() {
   if (!thread_db_process::loadedThreadDBLibrary()) {
      perr_printf("Failed to load thread_db.  Not fetching thread data.");
      setLastError(err_unsupported, "thread_db.so not loaded.  User-level thread data unavailable.");
      return false;
   }
   if (!thread_initialized) {
      perr_printf("Attempt to read user thread info of %d/%d before user thread create\n",
                  llproc()->getPid(), getLWP());
      setLastError(err_nouserthrd, "Attempted to read user thread info, but user thread has not been created.");
      return false;
   }
   if (tinfo_initialized) {
      return true;
   }
   if( !initThreadHandle() ) return false;

   pthrd_printf("Calling td_thr_get_info on %d/%d\n", llproc()->getPid(), getLWP());
   thread_db_process *tdb_proc = dynamic_cast<thread_db_process *>(llproc());
   async_ret_t result = tdb_proc->ll_fetchThreadInfo(threadHandle, &tinfo);
   if (result == aret_error) {
      pthrd_printf("Returning error in fetchThreadInfo due to ll_fetchThreadInfo\n");
      return false;
   }
   while (result == aret_async) {
      std::set<response::ptr> resps;
      llproc()->getMemCache()->getPendingAsyncs(resps);
      llproc()->waitForAsyncEvent(resps);
      result = tdb_proc->ll_fetchThreadInfo(threadHandle, &tinfo);
      if (result == aret_error) {
         pthrd_printf("Returning error in fetchThreadInfo due to ll_fetchThreadInfo\n");
         return false;
      }
   }

   if( tinfo.ti_tid ) tinfo_initialized = true;
   return true;
}

void thread_db_thread::markDestroyed() {
    destroyed = true;
}

bool thread_db_thread::isDestroyed() {
    return destroyed;
}

bool thread_db_thread::thrdb_getThreadArea(int, Dyninst::Address &)
{
   assert(0); //Unsupported.  Currently only known to be needed on linux/x86_64
   return false;
}

bool thread_db_thread::haveUserThreadInfo()
{
   pthrd_printf("haveUserThreadInfo (%d/%d): %d\n", (llproc() ? llproc()->getPid() : 0), lwp, thread_initialized);
   return thread_initialized;
}

bool thread_db_thread::getTID(Dyninst::THR_ID &tid_)
{
   if (!fetchThreadInfo()) {
      return false;
   }
#if defined(os_freebsd)
   tid_ = (Dyninst::THR_ID) tinfo.ti_thread;
#else
   tid_ = (Dyninst::THR_ID) tinfo.ti_tid;
#endif
   return true;
}

bool thread_db_thread::getStartFuncAddress(Dyninst::Address &addr)
{
   if (!fetchThreadInfo()) {
      return false;
   }
   addr = (Dyninst::Address) tinfo.ti_startfunc;
   return true;
}

bool thread_db_thread::getStackBase(Dyninst::Address &addr)
{
   if (!fetchThreadInfo()) {
      return false;
   }
   addr = (Dyninst::Address) tinfo.ti_stkbase;
   return true;
}

bool thread_db_thread::getStackSize(unsigned long &size)
{
   if (!fetchThreadInfo()) {
      return false;
   }
   size = (unsigned long) tinfo.ti_stksize;
   return true;
}

bool thread_db_thread::getTLSPtr(Dyninst::Address &addr)
{
   if (!fetchThreadInfo()) {
      return false;
   }
   addr = (Dyninst::Address) tinfo.ti_tls;
   return true;
}

#else

//Empty place holder functions in-case we're built on a machine without libthread_db.so

thread_db_process::thread_db_process(Dyninst::PID p, std::string e, std::vector<std::string> a, std::vector<std::string> envp, std::map<int, int> f) :
	int_process(p, e, a, envp, f)
{
  cerr << "Thread DB process constructor" << endl;
}

thread_db_process::thread_db_process(Dyninst::PID pid_, int_process *p) :
	int_process(pid_, p)
{
}

thread_db_process::~thread_db_process()
{
}

bool thread_db_process::decodeTdbLWPExit(EventLWPDestroy::ptr)
{
   return false;
}

async_ret_t thread_db_process::decodeTdbBreakpoint(EventBreakpoint::ptr)
{
   return aret_error;
}

void thread_db_process::addThreadDBHandlers(HandlerPool *)
{
}

thread_db_thread::thread_db_thread(int_process *p, Dyninst::THR_ID t, Dyninst::LWP l) :
   int_thread(p, t, l)
{
}

thread_db_thread::~thread_db_thread()
{
}

bool thread_db_thread::thrdb_getThreadArea(int, Dyninst::Address &)
{
   assert(0); //Should not be called if there's no thread_db
   return false;
}

bool thread_db_thread::haveUserThreadInfo()
{
   return false;
}

bool thread_db_thread::getTID(Dyninst::THR_ID &)
{
   perr_printf("Error. thread_db not installed on this platform.\n");
   setLastError(err_unsupported, "Cannot perform thread operations without thread_db\n");
   return false;
}

bool thread_db_thread::getStartFuncAddress(Dyninst::Address &)
{
   perr_printf("Error. thread_db not installed on this platform.\n");
   setLastError(err_unsupported, "Cannot perform thread operations without thread_db\n");
   return false;
}

bool thread_db_thread::getStackBase(Dyninst::Address &)
{
   perr_printf("Error. thread_db not installed on this platform.\n");
   setLastError(err_unsupported, "Cannot perform thread operations without thread_db\n");
   return false;
}

bool thread_db_thread::getStackSize(unsigned long &)
{
   perr_printf("Error. thread_db not installed on this platform.\n");
   setLastError(err_unsupported, "Cannot perform thread operations without thread_db\n");
   return false;
}

bool thread_db_thread::getTLSPtr(Dyninst::Address &)
{
   perr_printf("Error. thread_db not installed on this platform.\n");
   setLastError(err_unsupported, "Cannot perform thread operations without thread_db\n");
   return false;
}

bool thread_db_thread::plat_convertToSystemRegs(const int_registerPool &,
                                                unsigned char *, bool)
{
   return true;
}

async_ret_t thread_db_process::post_attach(bool b, set<response::ptr> &s) {
   return int_process::post_attach(b, s);
}

async_ret_t thread_db_process::post_create(std::set<response::ptr> &async_responses) {
   return int_process::post_create(async_responses);
}

bool thread_db_process::plat_getLWPInfo(lwpid_t, void *) {
   return false;
}

const char *thread_db_process::getThreadLibName(const char *)
{
   return "";
}

void thread_db_process::freeThreadDBAgent() {
}

async_ret_t thread_db_process::getEventForThread(int_eventThreadDB *)
{
   return aret_error;
}

bool thread_db_process::isSupportedThreadLib(string) {
   return false;
}

bool thread_db_process::plat_supportThreadEvents() {
   return false;
}

bool thread_db_process::setTrackThreads(bool, std::set<std::pair<int_breakpoint *, Address> > &,
                                                 bool &)
{
   perr_printf("Error. thread_db not installed on this platform.\n");
   setLastError(err_unsupported, "Cannot perform thread operations without thread_db\n");
   return false;
}

bool thread_db_process::isTrackingThreads()
{
   perr_printf("Error. thread_db not installed on this platform.\n");
   setLastError(err_unsupported, "Cannot perform thread operations without thread_db\n");
   return false;
}

ThreadTracking *thread_db_process::threaddb_getThreadTracking()
{
   perr_printf("Error. thread_db not installed on this platform.\n");
   setLastError(err_unsupported, "Cannot perform thread operations without thread_db\n");
   return NULL;
}

#endif
