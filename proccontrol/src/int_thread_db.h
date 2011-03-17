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

#include "proccontrol/src/int_process.h"

#if defined(cap_thread_db)

#include "proccontrol/h/Generator.h"
#include "proccontrol/h/Event.h"
#include "proccontrol/h/Decoder.h"
#include "proccontrol/h/Handler.h"
#include "proccontrol/src/int_handler.h"

extern "C" {

#if !defined(THREAD_DB_INC)
#include <thread_db.h>
#else
#include THREAD_DB_INC
#endif

#include "proc_service_wrapper.h"
}

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
   friend class thread_db_thread;
public:
    thread_db_process(Dyninst::PID p, std::string e, std::vector<std::string> a, std::vector<std::string> envp, std::map<int, int> f);
    thread_db_process(Dyninst::PID pid_, int_process *p);
    virtual ~thread_db_process();

    bool decodeTdbBreakpoint(EventBreakpoint::ptr bp);
    bool decodeTdbLWPExit(EventLWPDestroy::ptr lwp_ev);
    bool decodeTdbLibLoad(EventLibrary::ptr lib_ev);

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

    virtual bool plat_supportThreadEvents();

    // Platform-dependent functionality (derived classes override)
    virtual bool plat_getLWPInfo(lwpid_t lwp, void *lwpInfo);
    virtual const char *getThreadLibName(const char *symName);
    virtual bool isSupportedThreadLib(string libName);
    int_thread *triggerThread() const;

    bool initThreadWithHandle(td_thrhandle_t *thr, td_thrinfo_t *info);
    
    bool updateTidInfo(vector<Event::ptr> &threadEvents);
    bool needsTidUpdate();

    //The types for thread_db functions we will call
    typedef td_err_e (*td_init_t)(void);
    typedef td_err_e (*td_ta_new_t)(struct ps_prochandle *, td_thragent_t **);
    typedef td_err_e (*td_ta_delete_t)(td_thragent_t *);
    typedef td_err_e (*td_ta_event_addr_t)(const td_thragent_t *, td_event_e, td_notify_t *);
    typedef td_err_e (*td_ta_set_event_t)(const td_thragent_t *, td_thr_events_t *);
    typedef td_err_e (*td_ta_event_getmsg_t)(const td_thragent_t *, td_event_msg_t *);
    typedef td_err_e (*td_ta_map_lwp2thr_t)(const td_thragent_t *, lwpid_t, td_thrhandle_t *);
    typedef td_err_e (*td_thr_get_info_t)(const td_thrhandle_t *, td_thrinfo_t *);
    typedef td_err_e (*td_thr_event_enable_t)(const td_thrhandle_t *, int);
    typedef td_err_e (*td_thr_set_event_t)(const td_thrhandle_t *, td_thr_events_t *);
    typedef td_err_e (*td_thr_event_getmsg_t)(const td_thrhandle_t *, td_event_msg_t *);
    typedef td_err_e (*td_thr_dbsuspend_t)(const td_thrhandle_t *);
    typedef td_err_e (*td_thr_dbresume_t)(const td_thrhandle_t *);

    //Function pointers to the thread_db functions
    static bool loadedThreadDBLibrary();
    static td_init_t p_td_init;
    static td_ta_new_t p_td_ta_new;
    static td_ta_delete_t p_td_ta_delete;
    static td_ta_event_addr_t p_td_ta_event_addr;
    static td_ta_set_event_t p_td_ta_set_event;
    static td_ta_event_getmsg_t p_td_ta_event_getmsg;
    static td_ta_map_lwp2thr_t p_td_ta_map_lwp2thr;
    static td_thr_get_info_t p_td_thr_get_info;
    static td_thr_event_enable_t p_td_thr_event_enable;
    static td_thr_set_event_t p_td_thr_set_event;
    static td_thr_event_getmsg_t p_td_thr_event_getmsg;
    static td_thr_dbsuspend_t p_td_thr_dbsuspend;
    static td_thr_dbresume_t p_td_thr_dbresume;

protected:
    Event::ptr decodeThreadEvent(td_event_msg_t *eventMsg);
    bool handleThreadAttach(td_thrhandle_t *thr);
    virtual bool plat_convertToBreakpointAddress(psaddr_t &addr);

    static volatile bool thread_db_initialized;
    bool thread_db_proc_initialized;
    static Mutex thread_db_init_lock;

    map<Dyninst::Address, pair<int_breakpoint *, EventType> > addr2Event;
    td_thragent_t *threadAgent;

    struct ps_prochandle *self;
    int_thread *trigger_thread;

    bool needs_tid_update;
    
 private:
    static bool tdb_loaded;
    static bool tdb_loaded_result;
};

/*
 * libthread_db defines this as opaque. We need to implement it.
 */
struct ps_prochandle {
    thread_db_process *thread_db_proc;
};

class thread_db_thread : public int_thread
{
    friend class ThreadDBCreateHandler;
    friend class thread_db_process;
public:
    thread_db_thread(int_process *p, Dyninst::THR_ID t, Dyninst::LWP l);
    virtual ~thread_db_thread();

    Event::ptr getThreadEvent();
    bool setEventReporting(bool on);
    bool fetchThreadInfo();

    bool plat_resume();
    bool plat_suspend();
    void markDestroyed();
    bool isDestroyed();

    // Platform-dependent functionality
    virtual bool thrdb_getThreadArea(int val, Dyninst::Address &addr);
    virtual bool plat_convertToSystemRegs(const int_registerPool &pool, unsigned char *regs);

    virtual bool haveUserThreadInfo();
    virtual bool getTID(Dyninst::THR_ID &tid);
    virtual bool getStartFuncAddress(Dyninst::Address &addr);
    virtual bool getStackBase(Dyninst::Address &addr);
    virtual bool getStackSize(unsigned long &size);
    virtual bool getTLSPtr(Dyninst::Address &addr);
protected:
    // Initialization of the thread handle cannot be performed until 
    // thread_db is loaded and initialized. When creating a process,
    // we need to be able to create an instance of thread_db_thread
    // before thread_db is initialized so we lazily initialize the
    // thread handle
    bool initThreadHandle();
    
    td_thrhandle_t *threadHandle;
    td_thrinfo_t tinfo;

    // Since a thread destroy event happens at a breakpoint, the 
    // breakpoint needs to be cleaned up before the thread can be 
    // removed from the threadPool and deleted.
    bool destroyed;
    bool tinfo_initialized;
    bool thread_initialized;
    bool threadHandle_alloced;
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

typedef struct new_thread_data {
  td_thrhandle_t *thr_handle;
  td_thrinfo_t thr_info;
} new_thread_data_t;

#else

class thread_db_thread : public int_thread
{
  public:
    thread_db_thread(int_process *p, Dyninst::THR_ID t, Dyninst::LWP l);
    virtual ~thread_db_thread();

    virtual bool thrdb_getThreadArea(int val, Dyninst::Address &addr);
    virtual bool haveUserThreadInfo();
    virtual bool getTID(Dyninst::THR_ID &tid);
    virtual bool getStartFuncAddress(Dyninst::Address &addr);
    virtual bool getStackBase(Dyninst::Address &addr);
    virtual bool getStackSize(unsigned long &size);
    virtual bool getTLSPtr(Dyninst::Address &addr);
};


class thread_db_process : virtual public int_process
{
  public:
    thread_db_process(Dyninst::PID p, std::string e, std::vector<std::string> a, std::vector<std::string> envp, std::map<int, int> f);
    thread_db_process(Dyninst::PID pid_, int_process *p);
    virtual ~thread_db_process();

    bool decodeThreadBP(EventBreakpoint::ptr bp);
    static void addThreadDBHandlers(HandlerPool *hpool);
};

#endif

#endif
