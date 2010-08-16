#include "dynutil/h/dyntypes.h"
#include "proccontrol/src/int_process.h"
#include "proccontrol/src/int_handler.h"
#include "proccontrol/src/procpool.h"
#include "proccontrol/src/irpc.h"
#include "proccontrol/h/Event.h"
#include "proccontrol/h/Process.h"
#include "proccontrol/h/PCErrors.h"
#include <assert.h>
#include <stdlib.h>

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
   master_event(Event::ptr())
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
   return false;
}

bool Event::triggersCB() const
{
   HandleCallbacks *cbhandler = HandleCallbacks::getCB();
   if (cbhandler->requiresCB(shared_from_this()))
     return true;
   std::vector<Event::ptr>::const_iterator i = subservient_events.begin();
   for (; i != subservient_events.end(); i++) {
      if ((*i)->triggersCB())
         return true;
   }
   return false;
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
      STR_CASE(ThreadDestroy);
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
      STR_CASE(ChangePCStop);
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


EventBreakpoint::EventBreakpoint(Dyninst::Address addr_, installed_breakpoint *ibp_) :
   Event(EventType(EventType::None, EventType::Breakpoint)),
   ibp(ibp_),
   addr(addr_)
{
}

EventBreakpoint::~EventBreakpoint()
{
}

Dyninst::Address EventBreakpoint::getAddress() const
{
  return addr;
}

bool EventBreakpoint::procStopper() const
{
   return true;
}

void EventBreakpoint::getBreakpoints(std::vector<Breakpoint::const_ptr> &bps) const
{
   if (!ibp)
      return;
   std::set<Breakpoint::ptr>::iterator i;
   for (i = ibp->hl_bps.begin(); i != ibp->hl_bps.end(); i++) {
      bps.push_back(*i);
   }
}

installed_breakpoint *EventBreakpoint::installedbp() const
{
  return ibp;
}

bool EventBreakpoint::suppressCB() const
{
   return ibp->hl_bps.empty();
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

EventBootstrap::EventBootstrap() :
   Event(EventType(EventType::None, EventType::Bootstrap))
{
}

EventBootstrap::~EventBootstrap()
{
}

EventNewThread::EventNewThread(Dyninst::LWP lwp_) :
   Event(EventType(EventType::None, EventType::ThreadCreate)),
   lwp(lwp_)
{
}

EventNewThread::~EventNewThread()
{
}

Dyninst::LWP EventNewThread::getLWP() const
{
   return lwp;
}

Thread::const_ptr EventNewThread::getNewThread() const
{
   int_thread *thr = getProcess()->llproc()->threadPool()->findThreadByLWP(lwp);
   assert(thr);
   return thr->thread();
}

EventThreadDestroy::EventThreadDestroy(EventType::Time time_) :
   Event(EventType(time_, EventType::ThreadDestroy))
{
}

EventThreadDestroy::~EventThreadDestroy()
{
}

EventFork::EventFork(Dyninst::PID pid_) :
   Event(EventType(EventType::Post, EventType::Fork)),
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
   wrapper(wrapper_)
{
}

EventRPC::~EventRPC()
{
   if (wrapper->rpc->getIRPC().lock() == IRPC::ptr()) {
      delete wrapper;
      wrapper = NULL;
   }
}

IRPC::const_ptr EventRPC::getIRPC() const
{ 
   return wrapper->rpc->getIRPC().lock();
}

bool EventRPC::suppressCB() const
{
   return (wrapper->rpc->getIRPC().lock() == IRPC::ptr());
}

rpc_wrapper *EventRPC::getllRPC()
{
   return wrapper;
}

EventSingleStep::EventSingleStep() :
   Event(EventType(EventType::None, EventType::SingleStep))
{
}

EventSingleStep::~EventSingleStep()
{
}

EventBreakpointClear::EventBreakpointClear(installed_breakpoint *bp) :
  Event(EventType(EventType::None, EventType::BreakpointClear)),
  bp_(bp)
{
}

EventBreakpointClear::~EventBreakpointClear()
{
}

installed_breakpoint *EventBreakpointClear::bp()
{
  return bp_;
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

EventChangePCStop::EventChangePCStop() :
   Event(EventType(EventType::None, EventType::ChangePCStop))
{
}

EventChangePCStop::~EventChangePCStop()
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

DEFN_EVENT_CAST2(EventTerminate, Exit, Crash)
DEFN_EVENT_CAST(EventExit, Exit)
DEFN_EVENT_CAST(EventCrash, Crash)
DEFN_EVENT_CAST(EventExec, Exec)
DEFN_EVENT_CAST(EventStop, Stop)
DEFN_EVENT_CAST(EventBreakpoint, Breakpoint)
DEFN_EVENT_CAST(EventNewThread, ThreadCreate)
DEFN_EVENT_CAST(EventThreadDestroy, ThreadDestroy)
DEFN_EVENT_CAST(EventFork, Fork)
DEFN_EVENT_CAST(EventSignal, Signal)
DEFN_EVENT_CAST(EventBootstrap, Bootstrap)
DEFN_EVENT_CAST(EventRPC, RPC)
DEFN_EVENT_CAST(EventSingleStep, SingleStep)
DEFN_EVENT_CAST(EventBreakpointClear, BreakpointClear)
DEFN_EVENT_CAST(EventLibrary, Library)
DEFN_EVENT_CAST(EventChangePCStop, ChangePCStop)
