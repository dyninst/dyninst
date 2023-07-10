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
#if !defined(INT_THREAD_DB_H_)
#define INT_THREAD_DB_H_

#if defined(os_windows)
#error "Thread_db should not be included on Windows"
#endif

#include "int_process.h"

#if defined(cap_thread_db)

#include "Generator.h"
#include "Event.h"
#include "Decoder.h"
#include "Handler.h"
#include "int_handler.h"
#include "processplat.h"

extern "C" {

#if !defined(THREAD_DB_INC)
#include <thread_db.h>
#else
#include THREAD_DB_INC
#endif

#include "proc_service_wrapper.h"
}

#include <stddef.h>
#include <utility>
#include <map>
#include <set>
#include <vector>
#include <string>
#include <deque>

using namespace Dyninst;
using namespace ProcControlAPI;

class thread_db_thread;

class thread_db_process : public int_threadTracking
{
   friend class thread_db_thread;
   friend class ThreadDBDispatchHandler;
   friend class ThreadDBLibHandler;

   friend ps_err_e ps_pread(struct ps_prochandle *, psaddr_t, void *, size_t);
   friend ps_err_e ps_pwrite(struct ps_prochandle *, psaddr_t, const void *, size_t);

public:
    thread_db_process(Dyninst::PID p, std::string e, std::vector<std::string> a, std::vector<std::string> envp, std::map<int, int> f);
    thread_db_process(Dyninst::PID pid_, int_process *p);
    virtual ~thread_db_process();

    async_ret_t decodeTdbBreakpoint(EventBreakpoint::ptr bp);
    bool decodeTdbLWPExit(EventLWPDestroy::ptr lwp_ev);

    /* helper functions for thread_db interactions */

    td_thragent_t *getThreadDBAgent();
    ps_err_e getSymbolAddr(const char *objName, const char *symName, 
                           psaddr_t *symbolAddr);
    async_ret_t initThreadDB();

    virtual void freeThreadDBAgent();
    virtual async_ret_t getEventForThread(int_eventThreadDB *iev);
    static void addThreadDBHandlers(HandlerPool *hpool);

    /*
     * When creating a static executable or attaching to a new process,
     * thread_db initialization needs to occur immediately after
     * attach or create.
     *
     * When creating dynamic executables, initialization needs to happen
     * when the thread library is loaded.
     */
    virtual async_ret_t post_attach(bool wasDetached, std::set<response::ptr> &aresps);
    virtual async_ret_t post_create(std::set<response::ptr> &async_responses);

    virtual bool plat_supportThreadEvents();

    // Platform-dependent functionality (derived classes override)
    virtual bool plat_getLWPInfo(lwpid_t lwp, void *lwpInfo);
    virtual const char *getThreadLibName(const char *symName);
    virtual bool isSupportedThreadLib(std::string libName);
    int_thread *triggerThread() const;
    async_ret_t initThreadWithHandle(td_thrhandle_t *thr, td_thrinfo_t *info, Dyninst::LWP lwp);

    virtual bool setTrackThreads(bool b, std::set<std::pair<int_breakpoint *, Address> > &bps,
                                          bool &add_bp);
    virtual bool isTrackingThreads();
    virtual bool refreshThreads();
    virtual int threaddb_getPid();

    async_ret_t plat_calcTLSAddress(int_thread *thread, int_library *lib, Offset off,
                                    Address &outaddr, std::set<response::ptr> &resps);
    
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
    typedef td_err_e (*td_thr_tls_get_addr_t)(const td_thrhandle_t *, void *map_address,
                                              size_t offset, void **address);
    typedef td_err_e (*td_thr_tlsbase_t)(const td_thrhandle_t *, unsigned long modid,
                                         void **address);

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
    static td_thr_tls_get_addr_t p_td_thr_tls_get_addr;
    static td_thr_tlsbase_t p_td_thr_tlsbase;

protected:
    Event::ptr decodeThreadEvent(td_event_msg_t *eventMsg, bool &async);
    async_ret_t handleThreadAttach(td_thrhandle_t *thr, Dyninst::LWP lwp);
    // plat_convertToBreakpointAddress moved to int_process so we avoid
    // diamond inheritance undefined behavior

    static volatile bool thread_db_initialized;
    bool thread_db_proc_initialized;
    static Mutex<> thread_db_init_lock;

    std::map<Dyninst::Address, std::pair<int_breakpoint *, EventType> > addr2Event;
    td_thragent_t *threadAgent;
    bool createdThreadAgent;

    struct ps_prochandle *self;
    int_thread *trigger_thread;

    std::deque<Event::ptr> savedEvents;

    std::set<mem_response::ptr> resps;
    std::set<result_response::ptr> res_resps;
    EventThreadDB::ptr dispatch_event;

    bool hasAsyncPending;
    bool initialThreadEventCreated;
    bool setEventSet;
    bool completed_post;
    bool track_threads;
private:
    static bool tdb_loaded;
    static bool tdb_loaded_result;

    std::set<int_library *> libs_with_cached_tls_areas;

    async_ret_t ll_fetchThreadInfo(td_thrhandle_t *th, td_thrinfo_t *info);
};

/*
 * libthread_db defines this as opaque. We need to implement it.
 */
struct ps_prochandle {
    thread_db_process *thread_db_proc;
};

class thread_db_thread : virtual public int_thread
{
    friend class ThreadDBCreateHandler;
    friend class ThreadDBDispatchHandler;
    friend class thread_db_process;
   friend class ThreadDBLibHandler;
public:
    thread_db_thread(int_process *p, Dyninst::THR_ID t, Dyninst::LWP l);
    virtual ~thread_db_thread();

    async_ret_t setEventReporting(bool on);
    bool fetchThreadInfo();

    void markDestroyed();
    bool isDestroyed();

    // Platform-dependent functionality
    virtual bool thrdb_getThreadArea(int val, Dyninst::Address &addr);
    virtual bool plat_convertToSystemRegs(const int_registerPool &pool, unsigned char *regs, bool gprs_only = false);

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
    bool enabled_event_reporting;
  private:
    std::map<int_library *, Address> cached_tls_areas;
};

class ThreadDBDispatchHandler : public Handler
{
  public:
   ThreadDBDispatchHandler();
   virtual ~ThreadDBDispatchHandler();
   virtual Handler::handler_ret_t handleEvent(Event::ptr ev);
   void getEventTypesHandled(std::vector<EventType> &etypes);
   virtual int getPriority() const;
};

class ThreadDBCreateHandler : public Handler
{
public:
   ThreadDBCreateHandler();
   virtual ~ThreadDBCreateHandler();
   virtual Handler::handler_ret_t handleEvent(Event::ptr ev);
   void getEventTypesHandled(std::vector<EventType> &etypes);
   virtual int getPriority() const;
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
   void getEventTypesHandled(std::vector<EventType> &etypes);
   virtual int getPriority() const;
};

typedef struct new_thread_data {
  td_thrhandle_t *thr_handle;
  td_thrinfo_t thr_info;
  bool threadHandle_alloced;
} new_thread_data_t;

#else

#include <string>

#if defined(os_linux)
#include <sys/procfs.h> // lwpid_t
#endif

class thread_db_thread : virtual public int_thread
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

    virtual bool plat_convertToSystemRegs(const int_registerPool &, unsigned char *, bool);

};


class thread_db_process : virtual public int_process
{
  public:
    thread_db_process(Dyninst::PID p, std::string e, std::vector<std::string> a, std::vector<std::string> envp, std::map<int, int> f);
    thread_db_process(Dyninst::PID pid_, int_process *p);
    virtual ~thread_db_process();

    bool decodeTdbLWPExit(EventLWPDestroy::ptr lwp_ev);
    async_ret_t decodeTdbBreakpoint(EventBreakpoint::ptr bp);

    bool decodeThreadBP(EventBreakpoint::ptr bp);
    static void addThreadDBHandlers(HandlerPool *hpool);

    virtual async_ret_t post_attach(bool wasDetached, std::set<response::ptr> &);
    virtual async_ret_t post_create(std::set<response::ptr> &);
    virtual bool plat_getLWPInfo(lwpid_t, void *);
    const char *getThreadLibName(const char *);
    void freeThreadDBAgent();
    async_ret_t getEventForThread(int_eventThreadDB *);
    bool isSupportedThreadLib(std::string);
    bool plat_supportThreadEvents();
    bool threaddb_setTrackThreads(bool, std::set<std::pair<int_breakpoint *, Address> > &,
                                  bool &);
    bool threaddb_isTrackingThreads();
    ThreadTracking *threaddb_getThreadTracking() ;
    bool setTrackThreads(bool b, std::set<std::pair<int_breakpoint *, Address> > &bps,
		    bool &add_bp);
    bool isTrackingThreads();
};
#endif

#endif
