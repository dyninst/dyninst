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
#if !defined(INT_THREAD_DB_H_)
#define INT_THREAD_DB_H

#include "proccontrol/h/Generator.h"
#include "proccontrol/h/Event.h"
#include "proccontrol/h/Decoder.h"
#include "proccontrol/h/Handler.h"
#include "proccontrol/src/int_process.h"
#include "proccontrol/src/sysv.h"
#include "proccontrol/src/int_handler.h"

#include <thread_db.h>
#include <proc_service.h>

#include <map>
using std::map;
using std::pair;

#include <vector>
using std::vector;

#include <string>
using std::string;

using namespace Dyninst;
using namespace ProcControlAPI;

class thread_db_thread;

class thread_db_process : virtual public int_process
{
public:
    thread_db_process(Dyninst::PID p, std::string e, std::vector<std::string> a, 
            std::vector<std::string> envp, std::map<int, int> f);
    thread_db_process(Dyninst::PID pid_, int_process *p);
    virtual ~thread_db_process();

    bool getEventsAtAddr(Dyninst::Address addr, thread_db_thread *eventThread,
            vector<Event::ptr> &threadEvents);

    /* helper functions for thread_db interactions */

    td_thragent_t *getThreadDBAgent();
    ps_err_e getSymbolAddr(const char *objName, const char *symName, 
            psaddr_t *symbolAddr);
    virtual bool initThreadDB();
    virtual void freeThreadDBAgent();
    virtual bool getPostDestroyEvents(vector<Event::ptr> &events);
    static void addThreadDBHandlers(HandlerPool *hpool);

    /*
     * When creating a static executable or attaching to a new process,
     * thread_db initialization needs to occur immediately after
     * attach or create.
     *
     * When creating dynamic executables, initialization needs to happen
     * when the thread library is loaded.
     */
    virtual bool post_attach();
    virtual bool post_create();

    virtual bool plat_readProcMem(void *local,
            Dyninst::Address remote, size_t size) = 0;
    virtual bool plat_writeProcMem(void *local,
            Dyninst::Address remote, size_t size) = 0;
    virtual bool plat_getLWPInfo(lwpid_t lwp, void *lwpInfo) = 0;
    virtual bool plat_contThread(lwpid_t lwp) = 0;
    virtual bool plat_stopThread(lwpid_t lwp) = 0;
    virtual string getThreadLibName(const char *symName) = 0;
    virtual bool isSupportedThreadLib(const string &libName) = 0;

protected:
    static volatile bool thread_db_initialized;
    static Mutex thread_db_init_lock;

    map<Dyninst::Address, pair<int_breakpoint *, EventType> > addr2Event;
    td_thragent_t *threadAgent;

    struct ps_prochandle *self;
};

/*
 * libthread_db defines this as opaque. We need to implement it.
 */
struct ps_prochandle {
    thread_db_process *thread_db_proc;
};

class thread_db_thread : public int_thread
{
public:
    thread_db_thread(int_process *p, Dyninst::THR_ID t, Dyninst::LWP l);

    thread_db_thread();
    virtual ~thread_db_thread();

    Event::ptr getThreadEvent();
    bool setEventReporting(bool on);

    bool plat_resume();
    bool plat_suspend();
    void markDestroyed();
    bool isDestroyed();

protected:
    // Initialization of the thread handle cannot be performed until 
    // thread_db is loaded and initialized. When creating a process,
    // we need to be able to create an instance of thread_db_thread
    // before thread_db is initialized so we lazily initialize the
    // thread handle
    bool initThreadHandle();

    td_thrhandle_t *threadHandle;

    // Since a thread destroy event happens at a breakpoint, the 
    // breakpoint needs to be cleaned up before the thread can be 
    // removed from the threadPool and deleted.
    bool destroyed;
};

class ThreadDBCreateHandler : public Handler
{
public:
    ThreadDBCreateHandler();
    virtual ~ThreadDBCreateHandler();
    virtual Handler::handler_ret_t handleEvent(Event::ptr ev);
    virtual int getPriority() const;
    void getEventTypesHandled(vector<EventType> &etypes);
};

class ThreadDBLibHandler : public Handler
{
public:
    ThreadDBLibHandler();
    virtual ~ThreadDBLibHandler();
    virtual Handler::handler_ret_t handleEvent(Event::ptr ev);
    virtual int getPriority() const;
    void getEventTypesHandled(std::vector<EventType> &etypes);
};

class ThreadDBDestroyHandler : public Handler
{
public:
    ThreadDBDestroyHandler();
    virtual ~ThreadDBDestroyHandler();
    virtual Handler::handler_ret_t handleEvent(Event::ptr ev);
    virtual int getPriority() const;
    void getEventTypesHandled(std::vector<EventType> &etypes);
};

#endif
