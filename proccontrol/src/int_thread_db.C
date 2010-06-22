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

#include <proc_service.h>
#include <thread_db.h>

#include <cassert>
#include <set>
using std::set;

#include "common/h/dthread.h"
#include "int_thread_db.h"

/* 
 * proc_service interface implementation, needed by libthread_db
 */

ps_err_e ps_pglobal_lookup(struct ps_prochandle *handle, const char *objName, 
        const char *symName, psaddr_t *symbolAddr)
{
    return handle->thread_db_proc->getSymbolAddr(objName, symName, symbolAddr);
}

ps_err_e ps_pread(struct ps_prochandle *handle, psaddr_t remote, void *local, size_t size) {
    if( !handle->thread_db_proc->plat_readProcMem(local, (Dyninst::Address)remote, size) ) 
        return PS_ERR;

    return PS_OK;
}

ps_err_e ps_pwrite(struct ps_prochandle *handle, psaddr_t remote, const void *local, size_t size) {
    if( !handle->thread_db_proc->plat_writeProcMem(const_cast<void *>(local), (Dyninst::Address)remote, size) )
        return PS_ERR;

    return PS_OK;
}

ps_err_e ps_linfo(struct ps_prochandle *handle, lwpid_t lwp, void *lwpInfo) {
    if( !handle->thread_db_proc->plat_getLWPInfo(lwp, lwpInfo) )
        return PS_ERR;

    return PS_OK;
}

ps_err_e ps_lstop(struct ps_prochandle *handle, lwpid_t lwp) {
    if( !handle->thread_db_proc->plat_stopThread(lwp) ) {
        return PS_ERR;
    }

    return PS_OK;
}

ps_err_e ps_lcontinue(struct ps_prochandle *handle, lwpid_t lwp) {
    if( !handle->thread_db_proc->plat_contThread(lwp) ) {
        return PS_ERR;
    }

    return PS_OK;
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

static
Event::ptr decodeThreadEvent(td_event_msg_t *eventMsg) {
    td_thrinfo_t threadInfo;
    td_err_e errVal;
    if( TD_OK != (errVal = td_thr_get_info(eventMsg->th_p, &threadInfo)) ) {
        perr_printf("Failed to get thread event info from event msg: %s(%d)\n",
                tdErr2Str(errVal), errVal);
        return Event::ptr();
    }

    switch(eventMsg->event) {
        case TD_CREATE:
            return Event::ptr(new EventNewThread((Dyninst::LWP)threadInfo.ti_lid));
            break;
        case TD_DEATH:
            return Event::ptr(new EventThreadDestroy(EventType::Pre));
            break;
        default:
            pthrd_printf("Unimplemented libthread_db event encountered. Skipping for now.\n");
            break;
    }

    return Event::ptr();
}

volatile bool thread_db_process::thread_db_initialized = false;

thread_db_process::thread_db_process(Dyninst::PID p, std::string e, std::vector<std::string> a)
    : sysv_process(p, e, a), threadAgent(NULL)
{
    self = new ps_prochandle();
    self->thread_db_proc = this;
    assert(self);
}

thread_db_process::thread_db_process(Dyninst::PID pid_, int_process *p) 
    : sysv_process(pid_, p), threadAgent(NULL)
{
    self = new ps_prochandle();
    self->thread_db_proc = this;
    assert(self);
}

thread_db_process::~thread_db_process() 
{
    delete self;

    if( thread_db_initialized && threadAgent ) {
        td_err_e errVal = td_ta_delete(threadAgent);
        if( TD_OK != errVal ) {
            perr_printf("Failed to delete thread agent: %s(%d)\n",
                    tdErr2Str(errVal), errVal);
        }
        assert( TD_OK == errVal && "Failed to delete thread agent" );
    }

    // Free the breakpoints allocated for events
    map<Dyninst::Address, pair<int_breakpoint *, EventType> >::iterator brkptIter;
    for(brkptIter = addr2Event.begin(); brkptIter != addr2Event.end(); ++brkptIter) {
        delete brkptIter->second.first;
    }

    // Close all the symbol readers used
    map<string, pair<LoadedLib *, SymReader *> >::iterator symReaderIter;
    for(symReaderIter = symReaders.begin(); symReaderIter != symReaders.end(); 
            ++symReaderIter)
    {
        symreader_factory->closeSymbolReader(symReaderIter->second.second);
    }
}

bool thread_db_process::initThreadDB() {
    // Q: Why isn't this in the constructor? 
    // A: This function depends on the corresponding thread library being loaded
    // and this event occurs some time after process creation.

    // Make sure thread_db is initialized
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

    // Create the thread agent
    td_err_e errVal = td_ta_new(self, &threadAgent);
    switch(errVal) {
        case TD_OK:
            break;
        case TD_NOLIBTHREAD:
            pthrd_printf("Debuggee isn't multithreaded at this point, libthread_db not enabled\n");
            return true;
        default:
            perr_printf("Failed to create thread agent: %s(%d)\n",
                    tdErr2Str(errVal), errVal);
            setLastError(err_internal, "Failed to create libthread_db agent");
            return false;
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

bool thread_db_process::getEventsAtAddr(Dyninst::Address addr, 
        thread_db_thread *eventThread, vector<Event::ptr> &threadEvents) 
{
    unsigned oldSize = threadEvents.size();

    // Determine what type event occurs at the specified address
    map<Dyninst::Address, pair<int_breakpoint *, EventType> >::iterator addrIter;
    addrIter = addr2Event.find(addr);
    if( addrIter == addr2Event.end() ) return false;

    switch(addrIter->second.second.code()) {
        case EventType::ThreadCreate: 
        {
            pthrd_printf("Address 0x%lx corresponds to a thread create event.\n",
                    addr);
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
        case EventType::ThreadDestroy:
        {
            pthrd_printf("Address 0x%lx corresponds to a thread destroy event.\n",
                    addr);
            assert(eventThread);
            Event::ptr threadEvent = eventThread->getThreadEvent();
            if( threadEvent ) {
                threadEvents.push_back(threadEvent);
            }else{
                perr_printf("Failed to retrieve thread event for LWP %d\n",
                        eventThread->getLWP());
            }
            break;
        }
        default:
            pthrd_printf("Unimplemented libthread_db event encountered. Skipping for now.\n");
            break;
    }

    return oldSize != threadEvents.size();
}

td_thragent_t *thread_db_process::getThreadDBAgent() {
    return threadAgent;
}

ps_err_e thread_db_process::getSymbolAddr(const char *objName, const char *symName,
        psaddr_t *symbolAddr)
{
    SymReader *objSymReader = NULL;
    LoadedLib *lib = NULL;

    // For static executables, we need to search the executable instead of the
    // thread library. 

    // For static executables, breakpoint_addr isn't set
    if( !breakpoint_addr ) {
        lib = translator->getExecutable();
        if( NULL == lib ) {
            perr_printf("Failed to get loaded version of executable\n");
            setLastError(err_internal, "Failed to get loaded version of executable");
            return PS_ERR;
        }

        map<string, pair<LoadedLib *, SymReader *> >::iterator symReaderIter;
        symReaderIter = symReaders.find(lib->getName());
        if( symReaderIter == symReaders.end() ) {
            objSymReader = symreader_factory->openSymbolReader(lib->getName());
            if( NULL == objSymReader ) {
                perr_printf("Failed to open symbol reader for %s\n",
                        lib->getName().c_str());
                setLastError(err_internal, "Failed to open executable for symbol reading");
                return PS_ERR;
            }
            symReaders.insert(make_pair(lib->getName(), make_pair(lib, objSymReader)));
        }else{
            objSymReader = symReaderIter->second.second;
        }
    }else{
        // FreeBSD implementation doesn't set objName
        string objNameStr;
        if( NULL == objName ) {
            objNameStr = getThreadLibName(symName);
        }else{
            objNameStr = objName;
        }

        map<string, pair<LoadedLib *, SymReader *> >::iterator symReaderIter;
        symReaderIter = symReaders.find(objNameStr);
        if( symReaderIter == symReaders.end() ) {
            vector<LoadedLib *> libs;
            if( !translator->getLibs(libs) ) {
                perr_printf("Failed to retrieve loaded libraries\n");
                setLastError(err_internal, "Failed to retrieve loaded libraries");
                return PS_ERR;
            }

            vector<LoadedLib *>::iterator loadedLibIter;
            for(loadedLibIter = libs.begin(); loadedLibIter != libs.end();
                    ++loadedLibIter)
            {
                if( (*loadedLibIter)->getName().find(objNameStr) != string::npos ) {
                    lib = (*loadedLibIter);
                    break;
                }
            }

            if( NULL == lib ) {
                perr_printf("Failed to find loaded library for %s\n", objNameStr.c_str());
                setLastError(err_internal, "Failed to find loaded library");
                return PS_ERR;
            }

            objSymReader = symreader_factory->openSymbolReader(lib->getName());

            if( NULL == objSymReader ) {
                perr_printf("Failed to open symbol reader for %s\n", objNameStr.c_str());
                setLastError(err_internal, "Failed to open library for symbol reading");
                return PS_ERR;
            }

            symReaders.insert(make_pair(objNameStr, make_pair(lib, objSymReader)));
        }else{
            lib = symReaderIter->second.first;
            objSymReader = symReaderIter->second.second;
        }
    }

    Symbol_t lookupSym = objSymReader->getSymbolByName(string(symName));

    if( !objSymReader->isValidSymbol(lookupSym) ) {
        return PS_NOSYM;
    }

    *symbolAddr = (psaddr_t)lib->offToAddress(
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

void thread_db_process::addThreadDBHandlers(HandlerPool *hpool) {
    static bool initialized = false;
    static ThreadDBLibHandler *libHandler = NULL;
    static ThreadDBHandleNewThr *thrHandler = NULL;
    if( !initialized ) {
        libHandler = new ThreadDBLibHandler();
        thrHandler = new ThreadDBHandleNewThr();
        initialized = true;
    }
    hpool->addHandler(libHandler);
    hpool->addHandler(thrHandler);
}

ThreadDBLibHandler::ThreadDBLibHandler() :
    Handler("thread_db Library Handler")
{
}

ThreadDBLibHandler::~ThreadDBLibHandler() 
{
}

bool ThreadDBLibHandler::handleEvent(Event::ptr ev) {
    EventLibrary::const_ptr libEv = ev->getEventLibrary();

    thread_db_process *proc = static_cast<thread_db_process *>(ev->getProcess()->llproc());

    const set<Library::ptr> &addLibs = libEv->libsAdded();

    set<Library::ptr>::iterator libIter;
    for( libIter = addLibs.begin(); libIter != addLibs.end(); ++libIter ) {
        if( proc->isSupportedThreadLib((*libIter)->getName()) ) {
            pthrd_printf("Enabling thread_db support for pid %d\n",
                    proc->getPid());
            if( !proc->initThreadDB() ) {
                pthrd_printf("Failed to initialize thread_db for pid %d\n",
                        proc->getPid());
                return false;
            }
            break;
        }
    }

    return true;
}

int ThreadDBLibHandler::getPriority() const {
    return PostPlatformPriority;
}

void ThreadDBLibHandler::getEventTypesHandled(vector<EventType> &etypes) {
    etypes.push_back(EventType(EventType::None, EventType::Library));
}

ThreadDBHandleNewThr::ThreadDBHandleNewThr() :
    Handler("thread_db New Thread Handler")
{
}

ThreadDBHandleNewThr::~ThreadDBHandleNewThr() 
{
}

bool ThreadDBHandleNewThr::handleEvent(Event::ptr ev) {
    EventNewThread::const_ptr threadEv = ev->getEventNewThread();

    thread_db_thread *thread = static_cast<thread_db_thread *>(threadEv->getNewThread()->llthrd());

    assert(thread && "Thread was not initialized before calling this handler");

    // Explicitly ignore errors here
    thread->setEventReporting(true);

    return true;
}

int ThreadDBHandleNewThr::getPriority() const {
    return PostPlatformPriority;
}

void ThreadDBHandleNewThr::getEventTypesHandled(vector<EventType> &etypes) {
    etypes.push_back(EventType(EventType::None, EventType::ThreadCreate));
}

thread_db_thread::thread_db_thread(int_process *p, Dyninst::THR_ID t, Dyninst::LWP l)
    : int_thread(p, t, l)
{
}

thread_db_thread::~thread_db_thread() 
{
}

Event::ptr thread_db_thread::getThreadEvent() {
    thread_db_process *lproc = static_cast<thread_db_process *>(proc_);
    td_thrhandle_t threadHandle;

    // Get the thread handle
    td_err_e errVal = td_ta_map_lwp2thr(lproc->getThreadDBAgent(),
            lwp, &threadHandle);
    if( TD_OK != errVal ) {
        perr_printf("Failed to map lwp to thread_db thread: %s(%d)\n",
                tdErr2Str(errVal), errVal);
        setLastError(err_internal, "Failed to get thread_db thread handle");
        return Event::ptr();
    }

    td_event_msg_t eventMsg;

    errVal = td_thr_event_getmsg(&threadHandle, &eventMsg);

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

    return decodeThreadEvent(&eventMsg);
}

bool thread_db_thread::setEventReporting(bool on) {
    thread_db_process *lproc = static_cast<thread_db_process *>(proc_);
    td_thrhandle_t threadHandle;

    // Get the thread handle
    td_err_e errVal = td_ta_map_lwp2thr(lproc->getThreadDBAgent(),
            lwp, &threadHandle);
    if( TD_OK != errVal ) {
        perr_printf("Failed to map lwp to thread_db thread: %s(%d)\n",
                tdErr2Str(errVal), errVal);
        setLastError(err_internal, "Failed to get thread_db thread handle");
        return false;
    }

    errVal = td_thr_event_enable(&threadHandle, (on ? 1 : 0 ));

    if( TD_OK != errVal ) {
        perr_printf("Failed to enable events for LWP %d: %s(%d)\n",
                lwp, tdErr2Str(errVal), errVal);
        setLastError(err_internal, "Failed to enable thread_db events");
        return false;
    }

    pthrd_printf("Enabled thread_db events for LWP %d\n", lwp);

    return true;
}
