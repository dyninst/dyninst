/*
 * Copyright (c) 1996-2011 Barton P. Miller
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

#include "dynutil/h/dyntypes.h"
#include "proccontrol/src/int_process.h"
#include "proccontrol/src/int_handler.h"
#include "proccontrol/src/int_event.h"
#include "proccontrol/src/procpool.h"
#include "proccontrol/src/irpc.h"
#include "proccontrol/h/Event.h"
#include "proccontrol/h/Process.h"
#include "proccontrol/h/PCErrors.h"
#include "proccontrol/h/generator.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

ArchEvent::ArchEvent(std::string name_) :
   name(name_)
{
}

ArchEvent::~ArchEvent()
{
}

std::string ArchEvent::getName()
{
   return name;
}

Event::Event(EventType etype_, Thread::ptr thread_) :
   etype(etype_),
   thread(thread_),
   proc(thread ? thread->getProcess() : Process::ptr()),
   stype(unset),
   master_event(Event::ptr()),
   suppress_cb(false),
   user_event(false),
   handling_started(false),
   noted_event(false)
{
}

EventType Event::getEventType() const {
   return etype;
}

Thread::const_ptr Event::getThread() const {
   return thread;
}

Process::const_ptr Event::getProcess() const {
   return proc;
}

void Event::setThread(Thread::const_ptr t) {
   thread = t;
}

void Event::setProcess(Process::const_ptr p) {
	proc = p;
}

void Event::setUserEvent(bool b) {
    user_event = b;
}

bool Event::userEvent() const {
    return user_event;
}

bool Event::canFastHandle() const
{
   return false;
}

void Event::setSyncType(SyncType s)
{
   stype = s;
}

bool Event::suppressCB() const
{
   return suppress_cb;
}

bool Event::triggersCB() const
{
   HandleCallbacks *cbhandler = HandleCallbacks::getCB();
   if (cbhandler->requiresCB(shared_from_this()))
     return true;
   std::vector<Event::ptr>::const_iterator i = subservient_events.begin();
   for (; i != subservient_events.end(); ++i) {
      if ((*i)->triggersCB())
         return true;
   }
   return false;
}

void Event::setSuppressCB(bool b)
{
   suppress_cb = b;
}

Event::SyncType Event::getSyncType() const
{
   return stype;
}

std::string Event::name() const
{
   return etype.name();
}

Event::weak_ptr Event::subservientTo() const
{
   return master_event;
}

void Event::addSubservientEvent(Event::ptr ev)
{
   subservient_events.push_back(ev);
   ev->master_event = shared_from_this();
}

std::string EventType::name() const
{
   std::string prefix;
   if (etime == Pre)
      prefix = "pre-";
   if (etime == Post)
      prefix = "post-";

#define STR_CASE(ET) case ET: return prefix + std::string(#ET)
   switch (ecode) {
      STR_CASE(Error);
      STR_CASE(Unset);
      STR_CASE(Exit);
      STR_CASE(Crash);
      STR_CASE(Fork);
      STR_CASE(Exec);
      STR_CASE(ThreadCreate);
      STR_CASE(UserThreadCreate);
      STR_CASE(LWPCreate);
      STR_CASE(ThreadDestroy);
      STR_CASE(UserThreadDestroy);
      STR_CASE(LWPDestroy);
      STR_CASE(Stop);
      STR_CASE(Signal);
      STR_CASE(LibraryLoad);
      STR_CASE(LibraryUnload);
      STR_CASE(Bootstrap);
      STR_CASE(Breakpoint);
      STR_CASE(RPC);
      STR_CASE(SingleStep);
      STR_CASE(Library);
      STR_CASE(BreakpointClear);
      STR_CASE(BreakpointRestore);
      STR_CASE(RPCLaunch);
      STR_CASE(Async);
      STR_CASE(ChangePCStop);
      STR_CASE(Detach);
      STR_CASE(IntBootstrap);
      STR_CASE(ForceTerminate);
	  STR_CASE(PreBootstrap);
      STR_CASE(Nop);
      STR_CASE(ThreadDB);
      default: return prefix + std::string("Unknown");
   }
}

bool Event::procStopper() const
{
   return false;
}

Event::~Event()
{
}

EventContinue::EventContinue() :
Event(EventType(EventType::None,EventType::Continue))
{
}

EventContinue::~EventContinue()
{
}

EventTerminate::EventTerminate(EventType type_) :
   Event(type_)
{
}

EventTerminate::~EventTerminate()
{
}

EventExit::EventExit(EventType::Time eventtime, int exitcode_) :
   EventTerminate(EventType(eventtime, EventType::Exit)),
   exitcode(exitcode_)
{
}

int EventExit::getExitCode() const
{
   return exitcode;
}

EventExit::~EventExit()
{
}

EventCrash::EventCrash(int termsig_) :
   EventTerminate(EventType(EventType::None, EventType::Crash)),
   termsig(termsig_)
{
}

int EventCrash::getTermSignal() const
{
   return termsig;
}
         
EventCrash::~EventCrash()
{
}

EventForceTerminate::EventForceTerminate(int termsig_) :
    EventTerminate(EventType(EventType::Post, EventType::ForceTerminate)),
    termsig(termsig_)
{
}

int EventForceTerminate::getTermSignal() const
{
   return termsig;
}

EventForceTerminate::~EventForceTerminate()
{
}

EventExec::EventExec(EventType::Time etime_, std::string path) :
   Event(EventType(etime_, EventType::Exec)),
   execpath(path)
{
}

EventExec::~EventExec()
{
}

std::string EventExec::getExecPath() const
{
   return execpath;
}

void EventExec::setExecPath(std::string e)
{
   execpath = e;
}

EventStop::EventStop() :
   Event(EventType(EventType::None, EventType::Stop))
{
}

EventStop::~EventStop()
{
}


EventBreakpoint::EventBreakpoint(int_eventBreakpoint *ibp_) :
   Event(EventType(EventType::None, EventType::Breakpoint)),
   int_bp(ibp_)
{
}

EventBreakpoint::~EventBreakpoint()
{
   if (int_bp) {
      delete int_bp;
      int_bp = NULL;
   }
   
}

Dyninst::Address EventBreakpoint::getAddress() const
{
   return int_bp->addr;
}

void EventBreakpoint::getBreakpoints(std::vector<Breakpoint::const_ptr> &bps) const
{
   if (!int_bp)
      return;
   installed_breakpoint *ibp = int_bp->lookupInstalledBreakpoint();
   std::set<Breakpoint::ptr>::iterator i;
   for (i = ibp->hl_bps.begin(); i != ibp->hl_bps.end(); ++i) {
      bps.push_back(*i);
   }
}

void EventBreakpoint::getBreakpoints(std::vector<Breakpoint::ptr> &bps)
{
   if (!int_bp)
      return;
   installed_breakpoint *ibp = int_bp->lookupInstalledBreakpoint();
   std::set<Breakpoint::ptr>::iterator i;
   for (i = ibp->hl_bps.begin(); i != ibp->hl_bps.end(); i++) {
      bps.push_back(*i);
   }
}

bool EventBreakpoint::suppressCB() const
{
   if (Event::suppressCB()) return true;
   installed_breakpoint *ibp = int_bp->lookupInstalledBreakpoint();
   return ibp->hl_bps.empty();
}

int_eventBreakpoint *EventBreakpoint::getInternal() const
{
   return int_bp;
}

bool EventBreakpoint::procStopper() const
{
   int num_proc_stoppers = 0;
   installed_breakpoint *bp = int_bp->lookupInstalledBreakpoint();
   if (!bp) {
      return false;
   }

   for (installed_breakpoint::iterator i = bp->begin(); i != bp->end(); i++) {
      if (!(*i)->isProcessStopper())
         continue;
      if (isGeneratorThread()) {
         //We can call this during decode--don't set the states then.
         return true;
      }
      num_proc_stoppers++;
   }

   if (!handled_by.empty())
      return false;

   if (!num_proc_stoppers) {
      //The breakpoint is not a proc stopper.
      return false;
   }

   int_process *proc = getProcess()->llproc();
   int_thread *thrd = getThread()->llthrd();
   if (!int_bp->stopped_proc) {
      //Move the internal state of the process to be stopped.
      thrd->getInternalState().desyncStateProc(int_thread::stopped);
      int_bp->stopped_proc = true;
   }
   
   //We return true if the event isn't ready
   return !proc->getProcStopManager().processStoppedTo(int_thread::InternalStateID);
}

EventSignal::EventSignal(int sig_) :
   Event(EventType(EventType::None, EventType::Signal)),
   sig(sig_)
{
}

EventSignal::~EventSignal()
{
}

int EventSignal::getSignal() const
{
   return sig;
}

void EventSignal::clearThreadSignal() const
{
    int_thread *thr = getThread()->llthrd();
    thr->setContSignal(0);
}

void EventSignal::setThreadSignal(int newSignal) const 
{
    int_thread *thr = getThread()->llthrd();
    thr->setContSignal(newSignal);
}

EventBootstrap::EventBootstrap() :
   Event(EventType(EventType::None, EventType::Bootstrap))
{
}

EventBootstrap::~EventBootstrap()
{
}

EventPreBootstrap::EventPreBootstrap() :
   Event(EventType(EventType::None, EventType::PreBootstrap))
{
}

EventPreBootstrap::~EventPreBootstrap()
{
}


EventNewThread::EventNewThread(EventType et) : 
   Event(et)
{
}

EventNewThread::~EventNewThread()
{
}

EventNewUserThread::EventNewUserThread() :
   EventNewThread(EventType(EventType::None, EventType::UserThreadCreate))
{
   iev = new int_eventNewUserThread();
}

EventNewUserThread::~EventNewUserThread()
{
   if (iev) 
      delete iev;
   iev = NULL;
}

Dyninst::LWP EventNewUserThread::getLWP() const
{
   if (iev->lwp != NULL_LWP)
      return iev->lwp;
   return iev->thr ? iev->thr->getLWP() : NULL_LWP;
}

Thread::const_ptr EventNewUserThread::getNewThread() const
{
   if (iev->thr)
      return iev->thr->thread();
   if (iev->lwp == NULL_LWP)
      return Thread::const_ptr();

   iev->thr = getProcess()->llproc()->threadPool()->findThreadByLWP(iev->lwp);
   assert(iev->thr);
   return iev->thr->thread();
}

int_eventNewUserThread *EventNewUserThread::getInternalEvent() const
{
   return iev;
}

EventNewLWP::EventNewLWP(Dyninst::LWP lwp_) :
   EventNewThread(EventType(EventType::None, EventType::LWPCreate)),
   lwp(lwp_)
{
}

EventNewLWP::~EventNewLWP()
{
}

Dyninst::LWP EventNewLWP::getLWP() const
{
   return lwp;
}

Thread::const_ptr EventNewLWP::getNewThread() const
{
   int_thread *thr = getProcess()->llproc()->threadPool()->findThreadByLWP(lwp);
   assert(thr);
   return thr->thread();
}

EventThreadDestroy::EventThreadDestroy(EventType et) :
   Event(et)
{
}

EventThreadDestroy::~EventThreadDestroy()
{
}

EventUserThreadDestroy::EventUserThreadDestroy(EventType::Time time_) :
   EventThreadDestroy(EventType(time_, EventType::UserThreadDestroy))
{
}

EventUserThreadDestroy::~EventUserThreadDestroy()
{
}

EventLWPDestroy::EventLWPDestroy(EventType::Time time_) :
   EventThreadDestroy(EventType(time_, EventType::LWPDestroy))
{
}

EventLWPDestroy::~EventLWPDestroy()
{
}

EventFork::EventFork(EventType::Time time_, Dyninst::PID pid_) :
   Event(EventType(time_, EventType::Fork)),
   pid(pid_)
{
}

EventFork::~EventFork()
{
}

Dyninst::PID EventFork::getPID() const
{
   return pid;
}

Process::const_ptr EventFork::getChildProcess() const
{
   int_process *iproc = ProcPool()->findProcByPid(pid);
   assert(iproc);
   return iproc->proc();
}

EventRPC::EventRPC(rpc_wrapper *wrapper_) :
   Event(EventType(EventType::None, EventType::RPC)),
   wrapper(new rpc_wrapper(wrapper_))
{
   int_rpc = new int_eventRPC();
}

EventRPC::~EventRPC()
{
   memset(wrapper, 0, sizeof(wrapper));
   delete wrapper;
   wrapper = NULL;

   if (int_rpc) {
      delete int_rpc;
      int_rpc = NULL;
   }
}

IRPC::const_ptr EventRPC::getIRPC() const
{ 
   return wrapper->rpc->getIRPC().lock();
}

bool EventRPC::suppressCB() const
{
   if (Event::suppressCB()) return true;
   return (wrapper->rpc->getIRPC().lock() == IRPC::ptr());
}

rpc_wrapper *EventRPC::getllRPC()
{
   return wrapper;
}

int_eventRPC *EventRPC::getInternal() const
{
   return int_rpc;
}

bool EventRPCLaunch::procStopper() const
{
   int_process *proc = getProcess()->llproc();
   int_thread *thrd = getThread()->llthrd();

   if (!handled_by.empty())
      return false;

   int_iRPC::ptr rpc = thrd->nextPostedIRPC();
   assert(rpc);

   if (proc->plat_threadOpsNeedProcStop()) {
      return !proc->getProcStopManager().processStoppedTo(int_thread::IRPCSetupStateID);
   }
   if (rpc->isProcStopRPC()) {
      return !rpc->isRPCPrepped();
   }
   return !proc->getProcStopManager().threadStoppedTo(thrd, int_thread::IRPCSetupStateID);
}

EventRPCLaunch::EventRPCLaunch() :
   Event(EventType(EventType::None, EventType::RPCLaunch))
{
}

EventRPCLaunch::~EventRPCLaunch()
{
}

EventSingleStep::EventSingleStep() :
   Event(EventType(EventType::None, EventType::SingleStep))
{
}

EventSingleStep::~EventSingleStep()
{
}

EventBreakpointClear::EventBreakpointClear() :
  Event(EventType(EventType::None, EventType::BreakpointClear))
{
   int_bpc = new int_eventBreakpointClear();
}

EventBreakpointClear::~EventBreakpointClear()
{
   assert(int_bpc);
   delete int_bpc;
   int_bpc = NULL;
}

int_eventBreakpointClear *EventBreakpointClear::getInternal() const
{
   return int_bpc;
}

bool EventBreakpointClear::procStopper() const
{
   if (!handled_by.empty())
      return false;

   int_process *proc = getProcess()->llproc();
   return !proc->getProcStopManager().processStoppedTo(int_thread::BreakpointStateID);
}

EventBreakpointRestore::EventBreakpointRestore(int_eventBreakpointRestore *iebpr) :
   Event(EventType(EventType::None, EventType::BreakpointRestore)),
   int_bpr(iebpr)
{
}

EventBreakpointRestore::~EventBreakpointRestore()
{
   assert(int_bpr);
   delete int_bpr;
   int_bpr = NULL;
}

int_eventBreakpointRestore *EventBreakpointRestore::getInternal() const
{
   return int_bpr;
}


EventLibrary::EventLibrary(const std::set<Library::ptr> &added_libs_,
                           const std::set<Library::ptr> &rmd_libs_) :
   Event(EventType(EventType::None, EventType::Library)),
   added_libs(added_libs_),
   rmd_libs(rmd_libs_)
{
}

EventLibrary::EventLibrary() :
   Event(EventType(EventType::None, EventType::Library))
{
}

EventLibrary::~EventLibrary()
{
}

void EventLibrary::setLibs(const std::set<Library::ptr> &added_libs_,
                           const std::set<Library::ptr> &rmd_libs_)
{
   added_libs = added_libs_;
   rmd_libs = rmd_libs_;
}

const std::set<Library::ptr> &EventLibrary::libsAdded() const
{
   return added_libs;
}

const std::set<Library::ptr> &EventLibrary::libsRemoved() const
{
   return rmd_libs;
}

EventAsync::EventAsync(int_eventAsync *ievent) :
   Event(EventType(EventType::None, EventType::Async)),
   internal(ievent)
{
}

EventAsync::~EventAsync()
{
   if (internal) {
      delete internal;
      internal = NULL;
   }
}

int_eventAsync *EventAsync::getInternal() const
{
   return internal;
}

EventChangePCStop::EventChangePCStop() :
   Event(EventType(EventType::None, EventType::ChangePCStop))
{
}

EventChangePCStop::~EventChangePCStop()
{
}

EventDetach::EventDetach() :
   Event(EventType(EventType::None, EventType::Detach))
{
   int_detach = new int_eventDetach();
}

EventDetach::~EventDetach()
{
   if (int_detach)
      delete int_detach;
   int_detach = NULL;
}

int_eventDetach *EventDetach::getInternal() const
{
   return int_detach;
}

bool EventDetach::procStopper() const
{
   if (!handled_by.empty())
      return false;

   int_process *proc = getProcess()->llproc();
   return !proc->getProcStopManager().processStoppedTo(int_thread::DetachStateID);
}

EventIntBootstrap::EventIntBootstrap(void *d) :
   Event(EventType(EventType::None, EventType::IntBootstrap)),
   data(d)
{
}

EventIntBootstrap::~EventIntBootstrap()
{
}

void *EventIntBootstrap::getData() const
{
   return data;
}

void EventIntBootstrap::setData(void *d)
{
   data = d;
}

EventNop::EventNop() :
   Event(EventType(EventType::None, EventType::Nop))
{
}

EventNop::~EventNop()
{
}

EventThreadDB::EventThreadDB() :
   Event(EventType(EventType::None, EventType::ThreadDB))
{
   int_etdb = new int_eventThreadDB();
}

EventThreadDB::~EventThreadDB()
{
   delete int_etdb;
   int_etdb = NULL;
}

int_eventThreadDB *EventThreadDB::getInternal() const
{
   return int_etdb;
}

bool EventThreadDB::triggersCB() const
{
   EventType::Time ev_times[] = { EventType::None, EventType::Pre, EventType::Post };
   int ev_types[] = { EventType::UserThreadCreate, EventType::UserThreadDestroy, 
                      EventType::LWPCreate, EventType::LWPDestroy };
   HandleCallbacks *cbhandler = HandleCallbacks::getCB();
   for (unsigned i = 0; i < 3; i++)
      for (unsigned j = 0; j < 4; j++)
         if (cbhandler->hasCBs(EventType(ev_times[i], ev_types[j])))
            return true;
   return false;
}

int_eventBreakpoint::int_eventBreakpoint(Address a, installed_breakpoint *, int_thread *thr) :
   addr(a),
   thrd(thr),
   stopped_proc(false)
{
}

int_eventBreakpoint::~int_eventBreakpoint()
{
}

installed_breakpoint *int_eventBreakpoint::lookupInstalledBreakpoint()
{
   return thrd->llproc()->getBreakpoint(addr);
}

int_eventBreakpointClear::int_eventBreakpointClear() :
   started_bp_suspends(false),
   cached_bp_sets(false),
   set_singlestep(false)
{
}

int_eventBreakpointClear::~int_eventBreakpointClear()
{
}

int_eventBreakpointRestore::int_eventBreakpointRestore(installed_breakpoint *breakpoint_) :
   set_states(false),
   bp(breakpoint_)
{
}

int_eventBreakpointRestore::~int_eventBreakpointRestore()
{
}

int_eventRPC::int_eventRPC()
{
}

int_eventRPC::~int_eventRPC()
{
}

void int_eventRPC::getPendingAsyncs(std::set<response::ptr> &pending)
{
   if (alloc_regresult && !alloc_regresult->isReady()) {
      pending.insert(alloc_regresult);
   }
   if (memrestore_response && !memrestore_response->isReady()) {
      pending.insert(memrestore_response);
   }
   if (regrestore_response && !regrestore_response->isReady()) {
      pending.insert(regrestore_response);
   }
}

int_eventAsync::int_eventAsync(response::ptr r) :
   resp(r)
{
}

int_eventAsync::~int_eventAsync()
{
}

response::ptr int_eventAsync::getResponse() const
{
   return resp;
}

int_eventNewUserThread::int_eventNewUserThread() :
   thr(NULL),
   lwp(NULL_LWP),
   raw_data(NULL),
   needs_update(true)
{   
}

int_eventNewUserThread::~int_eventNewUserThread()
{
   if (raw_data)
      free(raw_data);
}

int_eventThreadDB::int_eventThreadDB() :
   completed_new_evs(false)
{
}

int_eventThreadDB::~int_eventThreadDB()
{
}

int_eventDetach::int_eventDetach() :
   temporary_detach(false),
   removed_bps(false),
   done(false)
{
}

int_eventDetach::~int_eventDetach()
{
}

#define DEFN_EVENT_CAST(NAME, TYPE) \
   NAME::ptr Event::get ## NAME() {  \
     if (etype.code() != EventType::TYPE) return NAME::ptr();  \
     return dyn_detail::boost::static_pointer_cast<NAME>(shared_from_this()); \
   } \
   NAME::const_ptr Event::get ## NAME() const { \
     if (etype.code() != EventType::TYPE) return NAME::const_ptr();  \
     return dyn_detail::boost::static_pointer_cast<const NAME>(shared_from_this()); \
   }

#define DEFN_EVENT_CAST2(NAME, TYPE, TYPE2) \
   NAME::ptr Event::get ## NAME() {  \
     if (etype.code() != EventType::TYPE && etype.code() != EventType::TYPE2) return NAME::ptr(); \
     return dyn_detail::boost::static_pointer_cast<NAME>(shared_from_this()); \
   } \
   NAME::const_ptr Event::get ## NAME() const { \
     if (etype.code() != EventType::TYPE && etype.code() != EventType::TYPE2) return NAME::const_ptr(); \
     return dyn_detail::boost::static_pointer_cast<const NAME>(shared_from_this()); \
   }

#define DEFN_EVENT_CAST3(NAME, TYPE, TYPE2, TYPE3) \
   NAME::ptr Event::get ## NAME() {  \
     if (etype.code() != EventType::TYPE && etype.code() != EventType::TYPE2 && etype.code() != EventType::TYPE3) return NAME::ptr(); \
     return dyn_detail::boost::static_pointer_cast<NAME>(shared_from_this()); \
   } \
   NAME::const_ptr Event::get ## NAME() const { \
     if (etype.code() != EventType::TYPE && etype.code() != EventType::TYPE2 && etype.code() != EventType::TYPE3) return NAME::const_ptr(); \
     return dyn_detail::boost::static_pointer_cast<const NAME>(shared_from_this()); \
   }

DEFN_EVENT_CAST3(EventTerminate, Exit, Crash, ForceTerminate)
DEFN_EVENT_CAST2(EventNewThread, UserThreadCreate, LWPCreate)
DEFN_EVENT_CAST2(EventThreadDestroy, UserThreadDestroy, LWPDestroy)
DEFN_EVENT_CAST(EventExit, Exit)
DEFN_EVENT_CAST(EventCrash, Crash)
DEFN_EVENT_CAST(EventForceTerminate, ForceTerminate)
DEFN_EVENT_CAST(EventExec, Exec)
DEFN_EVENT_CAST(EventStop, Stop)
DEFN_EVENT_CAST(EventBreakpoint, Breakpoint)
DEFN_EVENT_CAST(EventNewUserThread, UserThreadCreate)
DEFN_EVENT_CAST(EventNewLWP, LWPCreate)
DEFN_EVENT_CAST(EventUserThreadDestroy, UserThreadDestroy)
DEFN_EVENT_CAST(EventLWPDestroy, LWPDestroy)
DEFN_EVENT_CAST(EventFork, Fork)
DEFN_EVENT_CAST(EventSignal, Signal)
DEFN_EVENT_CAST(EventBootstrap, Bootstrap)
DEFN_EVENT_CAST(EventRPC, RPC)
DEFN_EVENT_CAST(EventSingleStep, SingleStep)
DEFN_EVENT_CAST(EventBreakpointClear, BreakpointClear)
DEFN_EVENT_CAST(EventBreakpointRestore, BreakpointRestore)
DEFN_EVENT_CAST(EventLibrary, Library)
DEFN_EVENT_CAST(EventRPCLaunch, RPCLaunch)
DEFN_EVENT_CAST(EventAsync, Async)
DEFN_EVENT_CAST(EventChangePCStop, ChangePCStop)
DEFN_EVENT_CAST(EventPreBootstrap, PreBootstrap)
DEFN_EVENT_CAST(EventDetach, Detach)
DEFN_EVENT_CAST(EventIntBootstrap, IntBootstrap)
DEFN_EVENT_CAST(EventNop, Nop);
DEFN_EVENT_CAST(EventThreadDB, ThreadDB)
