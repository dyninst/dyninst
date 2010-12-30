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

#include "common/h/Types.h"

#include "proccontrol/src/int_thread_db.h"

#if defined(cap_thread_db)

#include <cassert>
#include <cerrno>
#include <cstdarg>
#include <cstring>
#include <set>

using namespace std;

#include "common/h/dthread.h"
#include "dynutil/h/SymReader.h"
#include "proccontrol/src/int_event.h"

static bool ll_fetchThreadInfo(td_thrhandle_t *th, td_thrinfo_t *info);

/* 
 * proc_service interface implementation, needed by libthread_db
 */

ps_err_e ps_pglobal_lookup(struct ps_prochandle *handle, const char *objName, 
        const char *symName, psaddr_t *symbolAddr)
{
    return handle->thread_db_proc->getSymbolAddr(objName, symName, symbolAddr);
}

ps_err_e ps_pread(struct ps_prochandle *handle, psaddr_t remote, void *local, size_t size) {
    pthrd_printf("thread_db reading from %#lx to %#lx, size = %d on %d\n",
            (unsigned long)remote, (unsigned long)local, (int)size, handle->thread_db_proc->getPid());

    mem_response::ptr resp = mem_response::createMemResponse((char *) local, size);
    bool result = handle->thread_db_proc->readMem((Dyninst::Address) remote, resp,
                                                  handle->thread_db_proc->triggerThread());
    if (!result) {
       goto err;
    }
    result = int_process::waitForAsyncEvent(resp);
    if (!result || resp->hasError()) {
       goto err;
    }
    
    return PS_OK;
  err:
    pthrd_printf("Failed to read from %#lx to %#lx, size = %d on %d: %s\n",
                 (unsigned long)remote, (unsigned long)local, (int)size, handle->thread_db_proc->getPid(),
                 strerror(errno));
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
    result_response::ptr resp = result_response::createResultResponse();
    bool result = handle->thread_db_proc->writeMem(const_cast<void *>(local), (Dyninst::Address) remote, size, resp, 
                                                   handle->thread_db_proc->triggerThread());
    if (!result) {
       goto err;
    }
    result = int_process::waitForAsyncEvent(resp);
    if (!result || resp->hasError()) {
       goto err;
    }
    
    return PS_OK;
  err:
    pthrd_printf("Failed to write to %#lx from %#lx, size = %d on %d: %s\n",
                 (unsigned long)remote, (unsigned long)local, (int)size, handle->thread_db_proc->getPid(),
                 strerror(errno));
    return PS_ERR;
}

ps_err_e ps_pdwrite(struct ps_prochandle *handle, psaddr_t remote, const void *local, size_t size) {
   return ps_pwrite(handle, remote, local, size);
}

ps_err_e ps_ptwrite(struct ps_prochandle *handle, psaddr_t remote, const void *local, size_t size) {
   return ps_pwrite(handle, remote, local, size);
}

ps_err_e ps_linfo(struct ps_prochandle *handle, lwpid_t lwp, void *lwpInfo) {
    if( !handle->thread_db_proc->plat_getLWPInfo(lwp, lwpInfo) )
        return PS_ERR;

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
   
   if (thr->getInternalState() == int_thread::stopped) {
      return PS_OK;
   }
   else if (thr->getInternalState() != int_thread::running) {
      perr_printf("Error, ps_lstop on thread in bad state\n");
      return PS_ERR;
   }
   
   if( !thr->intStop() ) {
      return PS_ERR;
   }
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
   
   if (thr->getInternalState() == int_thread::running) {
      return PS_OK;
   }
   else if (thr->getInternalState() != int_thread::stopped) {
      perr_printf("Error, ps_lcontinue on thread in bad state\n");
      return PS_ERR;
   }
   
   if( !thr->intCont() ) {
      return PS_ERR;
   }
   return PS_OK;
}

pid_t ps_getpid (struct ps_prochandle *ph)
{
   return ph->thread_db_proc->getPid();
}

void	 ps_plog(const char *format, ...) {
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

ps_err_e ps_lgetregs(struct ps_prochandle *, lwpid_t, prgregset_t) {
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
   thread_db_thread *tdb_thread = static_cast<thread_db_thread *>(tdb_proc->threadPool()->findThreadByLWP(lwp));

   Dyninst::Address daddr = 0;
   bool result = tdb_thread->thrdb_getThreadArea(val, daddr);
   if (addr && result)
      *addr = (psaddr_t) daddr;
      
   return result ? PS_OK : PS_ERR;
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

Event::ptr thread_db_process::decodeThreadEvent(td_event_msg_t *eventMsg)
{
   td_thrinfo_t info;
   bool result = ll_fetchThreadInfo(const_cast<td_thrhandle_t *>(eventMsg->th_p), &info);
   if (!result) {
      pthrd_printf("Failed to fetch thread info\n");
      return Event::ptr();
   }
   Dyninst::LWP lwp = (Dyninst::LWP) info.ti_lid;
   switch(eventMsg->event) {
      case TD_CREATE:
      {
         pthrd_printf("Decoded to user thread create of %d/%d\n", getPid(), lwp);
#if defined(os_freebsd)
         td_err_e errVal;
         if( TD_OK != (errVal = td_thr_dbsuspend((const td_thrhandle_t *)eventMsg->th_p)) ) {
            perr_printf("Failed suspend new thread via thread_db: %s(%d)\n",
                        tdErr2Str(errVal), errVal);
            return Event::ptr();
         }
#endif
         EventNewUserThread::ptr new_ev = EventNewUserThread::ptr(new EventNewUserThread());
         int_eventNewUserThread *iev = new_ev->getInternalEvent();
         
         new_thread_data_t *thrdata = (new_thread_data_t *) malloc(sizeof(new_thread_data_t));
         thrdata->thr_handle = const_cast<td_thrhandle_t *>(eventMsg->th_p);
         thrdata->thr_info = info;

         iev->raw_data = (void *) thrdata;
         iev->lwp = lwp;

         return new_ev;
      }
      case TD_DEATH: {
         pthrd_printf("Decoded to user thread death of %d/%d\n", getPid(), lwp);
         int_thread *thr = threadPool()->findThreadByLWP(lwp);
         if (!thr) {
            perr_printf("Error.  Got thread delete event for unknown LWP\n");
            return Event::ptr();
         }

         EventUserThreadDestroy::ptr new_ev = EventUserThreadDestroy::ptr(new EventUserThreadDestroy(EventType::Pre));
         new_ev->setProcess(proc());
         new_ev->setThread(thr->thread());

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
Mutex thread_db_process::thread_db_init_lock;

thread_db_process::thread_db_process(Dyninst::PID p, std::string e, std::vector<std::string> a, std::map<int, int> f) :
  int_process(p, e, a, f),
  thread_db_proc_initialized(false),
  threadAgent(NULL),
  trigger_thread(NULL),
  needs_tid_update(false)
{
  self = new ps_prochandle();
  assert(self);
  self->thread_db_proc = this;
}

thread_db_process::thread_db_process(Dyninst::PID pid_, int_process *p) :
  int_process(pid_, p), 
  thread_db_proc_initialized(false),
  threadAgent(NULL),
  trigger_thread(NULL),
  needs_tid_update(false)  
{
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

    delete self;
}

bool thread_db_process::needsTidUpdate()
{
   return needs_tid_update;
}
 
bool thread_db_process::updateTidInfo(vector<Event::ptr> &threadEvents)
{
   if (!needsTidUpdate())
      return true;
   bool had_error = false;
   
   pthrd_printf("Trying to update TID information for %d\n", getPid());
   for (int_threadPool::iterator i = threadPool()->begin(); i != threadPool()->end(); i++) {
      thread_db_thread *thr = static_cast<thread_db_thread *>(*i);
      if (thr->tinfo_initialized || !thr->thread_initialized)
         continue;
      pthrd_printf("%d/%d needs a TID info update\n", getPid(), thr->getLWP());
      
      if (!thr->threadHandle) {
         thr->threadHandle = new td_thrhandle_t;
         thr->threadHandle_alloced = true;
      }
      td_err_e errVal = td_ta_map_lwp2thr(getThreadDBAgent(), thr->getLWP(), thr->threadHandle);
      if (errVal != TD_OK) {
         perr_printf("Error. Failed to get new thread handle\n");
         continue;
      }

      bool result = ll_fetchThreadInfo(thr->threadHandle, &thr->tinfo);
      if (!result) {
         perr_printf("%d/%d had an error while updating TID info\n", getPid(), thr->getLWP());
         had_error = true;
      }

      if (!thr->tinfo.ti_tid) {
         pthrd_printf("%d/%d still does not have valid TID info (lwp = %u)\n", getPid(), thr->getLWP(), (unsigned) thr->tinfo.ti_lid);
         had_error = true;
         continue;
      }
      pthrd_printf("%d/%d successfully updated TID info\n", getPid(), thr->getLWP());

      EventNewUserThread::ptr new_ev = EventNewUserThread::ptr(new EventNewUserThread());
      int_eventNewUserThread *iev = new_ev->getInternalEvent();
      iev->needs_update = false;
      iev->lwp = thr->getLWP();
      threadEvents.push_back(new_ev);

      pthrd_printf("Decoding new user thread to report updated TID info\n");
   }
   if (!had_error)
      needs_tid_update = false;
   return true;
}

bool thread_db_process::initThreadWithHandle(td_thrhandle_t *thr, td_thrinfo_t *info)
{
   td_thrinfo_t tinfo;
   if (!info) {
      bool result = ll_fetchThreadInfo(thr, &tinfo);
      if (!result) {
         return false;
      }
      info = &tinfo;
   }

   Dyninst::LWP lwp = (Dyninst::LWP) info->ti_lid;
   thread_db_thread *tdb_thread = static_cast<thread_db_thread *>(threadPool()->findThreadByLWP(lwp));
   if (!tdb_thread) { 
      perr_printf("Error.  Thread_db reports thread %d/%d, but couldn't find existing LWP\n",
                  getPid(), lwp);
      return false;
   }
   pthrd_printf("thread_db handling thread create for %d/%d\n", getPid(), lwp);
   tdb_thread->threadHandle = thr;
   tdb_thread->tinfo = *info;
   if (info->ti_tid)
      tdb_thread->tinfo_initialized = true;
   else {
      //On linux the TID of the initial thread isn't available at startup time.  
      // We'll mark this and try to get the TID the next time a thread_db event
      // comes along.
      needs_tid_update = true;
   }
   tdb_thread->thread_initialized = true;

   tdb_thread->setEventReporting(true);

   return true;
}


bool thread_db_process::handleThreadAttach(td_thrhandle_t *thr)
{
#if defined(os_freebsd)
   td_err_e errVal = td_thr_dbsuspend(handle);
   if( TD_OK != errVal ) return false;
#endif

   return initThreadWithHandle(thr, NULL);
}

bool thread_db_process::initThreadDB() {
    // Q: Why isn't this in the constructor? 
    // A: This function depends on the corresponding thread library being loaded
    // and this event occurs some time after process creation.

    // Make sure thread_db is initialized - only once for all instances
    if( !thread_db_initialized ) {
        thread_db_init_lock.lock();
        if( !thread_db_initialized ) {
            td_err_e errVal;
            if( TD_OK != (errVal = td_init()) ) {
                perr_printf("Failed to initialize libthread_db: %s(%d)\n",
                        tdErr2Str(errVal), errVal);
                setLastError(err_internal, "libthread_db initialization failed");
                return false;
            }
            pthrd_printf("Sucessfully initialized thread_db\n");
            thread_db_initialized = true;
        }
        thread_db_init_lock.unlock();
    }
    if (thread_db_proc_initialized) {
       return true;
    }

    // Create the thread agent
    td_err_e errVal = td_ta_new(self, &threadAgent);
    switch(errVal) {
        case TD_OK:
            pthrd_printf("Retrieved thread agent from thread_db\n");
            thread_db_proc_initialized = true;
            break;
        case TD_NOLIBTHREAD:
            pthrd_printf("Debuggee isn't multithreaded at this point, libthread_db not enabled\n");
            return true;
        default:
            perr_printf("Failed to create thread agent: %s(%d)\n",
                    tdErr2Str(errVal), errVal);
            thread_db_proc_initialized = true;
            setLastError(err_internal, "Failed to create libthread_db agent");
            return false;
    }

    set<td_thrhandle_t *> all_handles;
    for (int_threadPool::iterator i = threadPool()->begin(); i != threadPool()->end(); i++) {
       thread_db_thread *tdb_thread = static_cast<thread_db_thread *>(*i);
       tdb_thread->threadHandle = new td_thrhandle_t;

       errVal = td_ta_map_lwp2thr(getThreadDBAgent(), tdb_thread->getLWP(), tdb_thread->threadHandle);
       if (errVal != TD_OK) {
          perr_printf("Failed to map LWP %d to thread_db thread: %s(%d)\n",
                      tdb_thread->getLWP(), tdErr2Str(errVal), errVal);
          setLastError(err_internal, "Failed to get thread_db thread handle");
          delete tdb_thread->threadHandle;
          tdb_thread->threadHandle = NULL;
       }
       tdb_thread->threadHandle_alloced = true;

       all_handles.insert(tdb_thread->threadHandle);
    }

    for (set<td_thrhandle_t *>::iterator i = all_handles.begin(); i != all_handles.end(); i++)
    {
       bool result = handleThreadAttach(*i);
       if (!result) {
          perr_printf("Error handling thread_db attach\n");
       }
    }

    // Enable all events
    td_thr_events_t eventMask;
    td_event_fillset(&eventMask);

    errVal = td_ta_set_event(threadAgent, &eventMask);
    if( TD_OK != errVal ) {
        perr_printf("Failed to enable events: %s(%d)\n",
                tdErr2Str(errVal), errVal);
        setLastError(err_internal, "Failed to enable libthread_db events");
        return false;
    }

    // Determine the addresses for all events
    td_event_e allEvents[] = { TD_CATCHSIG, TD_CONCURRENCY, TD_CREATE,
        TD_DEATH, TD_IDLE, TD_LOCK_TRY, TD_PREEMPT, TD_PRI_INHERIT,
        TD_READY, TD_REAP, TD_SLEEP, TD_SWITCHFROM, TD_SWITCHTO,
        TD_TIMEOUT };

    for(unsigned i = 0; i < (sizeof(allEvents)/sizeof(td_event_e)); ++i) {
        td_notify_t notifyResult;
        errVal = td_ta_event_addr(threadAgent, allEvents[i], &notifyResult);

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

        int_breakpoint *newEventBrkpt = new int_breakpoint(Breakpoint::ptr());
        if( !addBreakpoint((Dyninst::Address)notifyResult.u.bptaddr,
                    newEventBrkpt))
        {
            perr_printf("Failed to install new event breakpoint\n");
            setLastError(err_internal, "Failed to install new thread_db event breakpoint");
            delete newEventBrkpt;

            return false;
        }

        pair<map<Dyninst::Address, pair<int_breakpoint *, EventType> >::iterator, bool> insertIter;
        insertIter = addr2Event.insert(make_pair((Dyninst::Address)notifyResult.u.bptaddr,
                    make_pair(newEventBrkpt, newEvent)));

        assert( insertIter.second && "event breakpoint address not unique" );
    }

    return true;
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
        td_err_e errVal = td_ta_delete(threadAgent);
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
   thread_db_thread *db_thread = static_cast<thread_db_thread *>(lwp_ev->getThread()->llthrd());
   assert(db_thread);
   
   if (db_thread->destroyed || !db_thread->thread_initialized)
      return false;

   pthrd_printf("Decoded LWP exit without thread exit on %d/%d.  Faking thread exit event\n",
                db_thread->llproc()->getPid(), db_thread->getLWP());
   
   EventUserThreadDestroy::ptr new_ev = EventUserThreadDestroy::ptr(new EventUserThreadDestroy(EventType::Pre));
   new_ev->setProcess(db_thread->llproc()->proc());
   new_ev->setThread(db_thread->thread());
   lwp_ev->addSubservientEvent(new_ev);
   return true;
}

bool thread_db_process::decodeTdbLibLoad(EventLibrary::ptr lib_ev)
{
   if (!needsTidUpdate())
      return false;
   
   pthrd_printf("Trying to update TID data upon library load event on %d\n", getPid());

   vector<Event::ptr> threadEvents;
   trigger_thread = lib_ev->getThread()->llthrd();
   updateTidInfo(threadEvents);
   
   if (threadEvents.empty())
      return false;

   pthrd_printf("Decoding thread create with library load\n");
   for (vector<Event::ptr>::iterator i = threadEvents.begin(); i != threadEvents.end(); i++) {
      Event::ptr ev = *i;
      if (!ev->getThread()) 
         ev->setThread(lib_ev->getThread());
      if (!ev->getProcess()) 
         ev->setProcess(proc());
      lib_ev->addSubservientEvent(ev);
   }

   return true;
}

bool thread_db_process::decodeTdbBreakpoint(EventBreakpoint::ptr bp)
{
    Dyninst::Address addr = bp->getAddress();
    
    // Determine what type of event occurs at the specified address
    map<Dyninst::Address, pair<int_breakpoint *, EventType> >::iterator addrIter;
    addrIter = addr2Event.find(addr);
    if( addrIter == addr2Event.end() ) 
       return false;

    vector<Event::ptr> threadEvents;
    trigger_thread = bp->getThread()->llthrd();

    if (needsTidUpdate())
       updateTidInfo(threadEvents);

    EventType::Code ecode = addrIter->second.second.code();
    switch(ecode) {
        case EventType::ThreadCreate: 
        case EventType::ThreadDestroy:
        {
            pthrd_printf("Address 0x%lx corresponds to a thread %s event.\n",
                         addr, ecode == EventType::ThreadCreate ? "create" : "destroy");
            // Need to ask via the thread_db agent for creation events. This
            // could result in getting information about other events.  All of
            // these events need to be handled.
            td_event_msg_t threadMsg;
            td_err_e msgErr = TD_OK;
            while(msgErr == TD_OK) {
                msgErr = td_ta_event_getmsg(threadAgent, &threadMsg);
                if( msgErr == TD_OK ) {
                    Event::ptr threadEvent = decodeThreadEvent(&threadMsg);
                    if( threadEvent ) {
                        threadEvents.push_back(threadEvent);
                    }
                }
            }

            if( msgErr != TD_NOMSG ) {
                perr_printf("Failed to retrieve thread event: %s(%d)\n",
                        tdErr2Str(msgErr), msgErr);
            }
            break;
        }
        default:
            pthrd_printf("Unimplemented libthread_db event encountered. Skipping for now.\n");
            break;
    }
    trigger_thread = NULL;

    if (threadEvents.empty())
       return false;

    for (vector<Event::ptr>::iterator i = threadEvents.begin(); i != threadEvents.end(); i++) {
       Event::ptr ev = *i;
       if (!ev->getThread()) 
          ev->setThread(bp->getThread());
       if (!ev->getProcess()) 
          ev->setProcess(proc());
       bp->addSubservientEvent(ev);
    }
    bp->setSuppressCB(true);
    return true;
}

td_thragent_t *thread_db_process::getThreadDBAgent() {
    return threadAgent;
}

static string stripLibraryName(const char *libname)
{
   const char *filename_c = strrchr(libname, '/');
   if (!filename_c)
      filename_c = strrchr(libname, '\\');
   if (!filename_c) 
      filename_c = libname;
   else 
      filename_c++;
   
   const char *lesser_ext = NULL;
   const char *dot_ext = strchr(filename_c, '.');
   if (dot_ext)
      lesser_ext = dot_ext;
   const char *dash_ext = strchr(filename_c, '-');
   if (dash_ext && (!lesser_ext || dash_ext < lesser_ext))
      lesser_ext = dash_ext;

   if (!lesser_ext) {
      return std::string(filename_c);
   }
   return std::string(filename_c, lesser_ext - filename_c);
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
          if (strstr(l->getName().c_str(), name.c_str())) {
             lib = l;
             break;
          }
       }
    }

    if( NULL == lib ) {
       perr_printf("Failed to find loaded library\n");
       setLastError(err_internal, "Failed to find loaded library");
       return PS_ERR;
    }

    objSymReader = plat_defaultSymReader()->openSymbolReader(lib->getName());
    if( NULL == objSymReader ) {
        perr_printf("Failed to open symbol reader for %s\n",
		    lib->getName().c_str());
        setLastError(err_internal, "Failed to open executable for symbol reading");
        return PS_ERR;
    }

    Symbol_t lookupSym = objSymReader->getSymbolByName(string(symName));

    if( !objSymReader->isValidSymbol(lookupSym) ) {
        return PS_NOSYM;
    }

    *symbolAddr = (psaddr_t) (lib->getAddr() + 
			      objSymReader->getSymbolOffset(lookupSym));

    return PS_OK;
}

bool thread_db_process::post_create() {
    if( !int_process::post_create() ) return false;

    return initThreadDB();
}

bool thread_db_process::post_attach() {
    if( !int_process::post_attach() ) return false;

    return initThreadDB();
}

bool thread_db_process::getPostDestroyEvents(vector<Event::ptr> &events) { 
   unsigned oldSize = events.size();

    int_threadPool::iterator i;
    for(i = threadPool()->begin(); i != threadPool()->end(); ++i) {
        thread_db_thread *tmpThread = static_cast<thread_db_thread *>(*i);
        if( tmpThread->isDestroyed() ) {
            pthrd_printf("Generating post-ThreadDestroy for %d/%d\n",
                         getPid(), tmpThread->getLWP());

            Event::ptr destroyEvent = Event::ptr(new EventUserThreadDestroy(EventType::Post));
            destroyEvent->setThread(tmpThread->thread());
            destroyEvent->setProcess(proc());
            events.push_back(destroyEvent);
        }
    }

    return events.size() != oldSize;
}

bool thread_db_process::isSupportedThreadLib(string libName) {
   return (libName.find("libpthread") != string::npos);
}

void thread_db_process::addThreadDBHandlers(HandlerPool *hpool) {
    static bool initialized = false;
    static ThreadDBLibHandler *libHandler = NULL;
    static ThreadDBCreateHandler *createHandler = NULL;
    static ThreadDBDestroyHandler *destroyHandler = NULL;
    if( !initialized ) {
        libHandler = new ThreadDBLibHandler();
        createHandler = new ThreadDBCreateHandler();
        destroyHandler = new ThreadDBDestroyHandler();
        initialized = true;
    }
    hpool->addHandler(libHandler);
    hpool->addHandler(createHandler);
    hpool->addHandler(destroyHandler);
}

bool thread_db_process::plat_getLWPInfo(lwpid_t, void *) 
{
   perr_printf("Attempt to use unsupported plat_getLWPInfo\n");
   return false;
}

int_thread *thread_db_process::triggerThread() const
{
   return trigger_thread;
}

ThreadDBLibHandler::ThreadDBLibHandler() :
    Handler("thread_db Library Handler")
{
}

ThreadDBLibHandler::~ThreadDBLibHandler() 
{
}

Handler::handler_ret_t ThreadDBLibHandler::handleEvent(Event::ptr ev) {
    EventLibrary::const_ptr libEv = ev->getEventLibrary();

    thread_db_process *proc = dynamic_cast<thread_db_process *>(ev->getProcess()->llproc());

    const set<Library::ptr> &addLibs = libEv->libsAdded();

    set<Library::ptr>::iterator libIter;
    for( libIter = addLibs.begin(); libIter != addLibs.end(); ++libIter ) {
        if( ! proc->isSupportedThreadLib((*libIter)->getName()) )
           continue;

        pthrd_printf("Enabling thread_db support for pid %d\n",
                     proc->getPid());
        if( !proc->initThreadDB() ) {
           pthrd_printf("Failed to initialize thread_db for pid %d\n",
                        proc->getPid());
           return Handler::ret_error;
        }
        break;
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

Handler::handler_ret_t ThreadDBCreateHandler::handleEvent(Event::ptr ev) {
    EventNewUserThread::ptr threadEv = ev->getEventNewUserThread();
    if (threadEv->getInternalEvent()->needs_update) {
       pthrd_printf("Updating user thread data for %d/%d in thread_db create handler\n",
                    threadEv->getProcess()->llproc()->getPid(),
                    threadEv->getNewThread()->llthrd()->getLWP());
       new_thread_data_t *thrdata = (new_thread_data_t *) threadEv->getInternalEvent()->raw_data;
       thread_db_process *tdb_proc = dynamic_cast<thread_db_process *>(ev->getProcess()->llproc());
       assert(tdb_proc);
       pthrd_printf("thread_db user create handler for %d/%d\n", tdb_proc->getPid(), threadEv->getLWP());
       
       bool result = tdb_proc->initThreadWithHandle(thrdata->thr_handle, &thrdata->thr_info);
       return result ? Handler::ret_success : Handler::ret_error;
    }

    return Handler::ret_success;

}

int ThreadDBCreateHandler::getPriority() const {
    return PostPlatformPriority;
}

void ThreadDBCreateHandler::getEventTypesHandled(vector<EventType> &etypes) {
    etypes.push_back(EventType(EventType::None, EventType::UserThreadCreate));
}

ThreadDBDestroyHandler::ThreadDBDestroyHandler() :
    Handler("thread_db Destroy Handler")
{
}

ThreadDBDestroyHandler::~ThreadDBDestroyHandler()
{
}

Handler::handler_ret_t ThreadDBDestroyHandler::handleEvent(Event::ptr ev) {
    thread_db_thread *thrd = static_cast<thread_db_thread *>(ev->getThread()->llthrd());
    if( ev->getEventType().time() == EventType::Pre) {
        pthrd_printf("Marking LWP %d destroyed\n", thrd->getLWP());
        thrd->markDestroyed();
    }else if( ev->getEventType().time() == EventType::Post) {
        // TODO this needs to be reworked -- it isn't quite right
        // Need to make sure that the thread actually finishes and is cleaned up
        // by the OS
        thrd->plat_resume();
    }

    return Handler::ret_success;
}

int ThreadDBDestroyHandler::getPriority() const {
    return PrePlatformPriority;
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
   threadHandle_alloced(false)
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

    td_err_e errVal = td_ta_map_lwp2thr(lproc->getThreadDBAgent(),
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

Event::ptr thread_db_thread::getThreadEvent() {
    if( !initThreadHandle() ) return Event::ptr();
    thread_db_process *dbproc = dynamic_cast<thread_db_process *>(llproc());

    td_event_msg_t eventMsg;

    td_err_e errVal = td_thr_event_getmsg(threadHandle, &eventMsg);

    if( TD_NOMSG == errVal ) {
        pthrd_printf("No message available for LWP %d via thread_db\n", lwp);
        return Event::ptr();
    }

    if( TD_OK != errVal ) {
        perr_printf("Failed to get thread event message: %s(%d)\n",
                tdErr2Str(errVal), errVal);
        setLastError(err_internal, "Failed to get thread event message");
        return Event::ptr();
    }

    return dbproc->decodeThreadEvent(&eventMsg);
}

bool thread_db_thread::setEventReporting(bool on) {
    if( !initThreadHandle() ) return false;

    pthrd_printf("Enabled thread_db events for LWP %d\n", lwp);
    td_err_e errVal = td_thr_event_enable(threadHandle, (on ? 1 : 0 ));
    if( TD_OK != errVal ) {
        perr_printf("Failed to enable events for LWP %d: %s(%d)\n",
                lwp, tdErr2Str(errVal), errVal);
        setLastError(err_internal, "Failed to enable thread_db events");
        return false;
    }

    return true;
}

static bool ll_fetchThreadInfo(td_thrhandle_t *th, td_thrinfo_t *info)
{
   td_err_e result = td_thr_get_info(th, info);
   if (result != TD_OK) {
      perr_printf("Error calling td_thr_get_info: %s (%d)\n", tdErr2Str(result), (int) result);
      return false;      
   }
   return true;
}

bool thread_db_thread::fetchThreadInfo() {
   if (!thread_initialized) {
      perr_printf("Attempt to read user thread info of %d/%d before user thread create\n",
                  llproc()->getPid(), getLWP());
      setLastError(err_nouserthrd, "Attempted to read user thread info, but user thread has not been created.");
      return false;
   }
   if( !initThreadHandle() ) return false;
   if (tinfo_initialized) {
      return true;
   }

   pthrd_printf("Calling td_thr_get_info on %d/%d\n", llproc()->getPid(), getLWP());
   bool result = ll_fetchThreadInfo(threadHandle, &tinfo);
   if (!result) {
      return false;
   }
   tinfo_initialized = true;
   return true;
}

bool thread_db_thread::plat_resume() {
    if( !initThreadHandle() ) return false;

    td_err_e errVal = td_thr_dbresume(threadHandle);

    if( TD_OK != errVal ) {
        perr_printf("Failed to resume %d/%d: %s(%d)\n",
                llproc()->getPid(), lwp, tdErr2Str(errVal), errVal);
        setLastError(err_internal, "Failed to resume LWP");
        return false;
    }

    return true;
}

bool thread_db_thread::plat_suspend() {
    if( !initThreadHandle() ) return false;

    td_err_e errVal = td_thr_dbsuspend(threadHandle);

    if( TD_OK != errVal ) {
        perr_printf("Failed to suspend %d/%d: %s(%d)\n",
                llproc()->getPid(), lwp, tdErr2Str(errVal), errVal);
        setLastError(err_internal, "Failed to suspend LWP");
        return false;
    }

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
   return thread_initialized;
}

bool thread_db_thread::getTID(Dyninst::THR_ID &tid)
{
   if (!fetchThreadInfo()) {
      return false;
   }
   tid = (Dyninst::THR_ID) tinfo.ti_tid;
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

//Empty place holder functions in-case we're built on a machine wihtout libthread_db.so

thread_db_process::thread_db_process(Dyninst::PID p, std::string e, std::vector<std::string> a, std::map<int, int> f) : 
   int_process(p, e, a, f)
{
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

bool thread_db_process::decodeTdbBreakpoint(EventBreakpoint::ptr)
{
   return false;
}

bool thread_db_process::decodeTdbLibLoad(EventLibrary::ptr)
{
   return false;
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

#endif
