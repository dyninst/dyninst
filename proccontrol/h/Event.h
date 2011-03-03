#if !defined(EVENT_H_)
#define EVENT_H_

#include <string>
#include <set>
#include "dyntypes.h"
#include "EventType.h"
#include "Process.h"

#include "dyn_detail/boost/shared_ptr.hpp"
#include "dyn_detail/boost/enable_shared_from_this.hpp"

class installed_breakpoint;
class HandlerPool;

namespace Dyninst {
namespace ProcControlAPI {

class ArchEvent
{
private:
   std::string name;
public:
   ArchEvent(std::string name_ = std::string(""));
   virtual ~ArchEvent();
   std::string getName(); 
};

class Handler;
class EventTerminate;
class EventExit;
class EventCrash;
class EventForceTerminate;
class EventExec;
class EventBreakpoint;
class EventStop;
class EventNewThread;
class EventNewUserThread;
class EventNewLWP;
class EventThreadDestroy;
class EventUserThreadDestroy;
class EventLWPDestroy;
class EventFork;
class EventSignal;
class EventBootstrap;
class EventRPC;
class EventSingleStep;
class EventBreakpointClear;
class EventLibrary;
class EventRPCInternal;
class EventAsync;
class EventChangePCStop;
class EventDetached;
class EventIntBootstrap;

class Event : public dyn_detail::boost::enable_shared_from_this<Event>
{
   friend void dyn_detail::boost::checked_delete<Event>(Event *);
   friend void dyn_detail::boost::checked_delete<const Event>(const Event *);
   friend class ::HandlerPool;
   friend class ::int_process;
 public:
   typedef dyn_detail::boost::shared_ptr<Event> ptr;
   typedef dyn_detail::boost::shared_ptr<const Event> const_ptr;
   typedef dyn_detail::boost::weak_ptr<Event> weak_ptr;

   Event(EventType etype_, Thread::ptr thread_ = Thread::ptr());
   virtual ~Event();

   typedef enum {
      unset,
      async,
      sync_thread,
      sync_process
   } SyncType;

   EventType getEventType() const;
   Thread::const_ptr getThread() const;
   Process::const_ptr getProcess() const;
   SyncType getSyncType() const;
   std::string name() const;

   void setThread(Thread::const_ptr t);
   void setProcess(Process::const_ptr p);
   void setSyncType(SyncType t);

   void setSuppressCB(bool b);
   virtual bool suppressCB() const;
   virtual bool triggersCB() const;
   virtual bool canFastHandle() const;
   virtual bool procStopper() const;
   Event::weak_ptr subservientTo() const;
   void addSubservientEvent(Event::ptr ev);

   dyn_detail::boost::shared_ptr<EventTerminate> getEventTerminate();
   dyn_detail::boost::shared_ptr<const EventTerminate> getEventTerminate() const;

   dyn_detail::boost::shared_ptr<EventExit> getEventExit();
   dyn_detail::boost::shared_ptr<const EventExit> getEventExit() const;

   dyn_detail::boost::shared_ptr<EventCrash> getEventCrash();
   dyn_detail::boost::shared_ptr<const EventCrash> getEventCrash() const;

   dyn_detail::boost::shared_ptr<EventForceTerminate> getEventForceTerminate();
   dyn_detail::boost::shared_ptr<const EventForceTerminate> getEventForceTerminate() const;

   dyn_detail::boost::shared_ptr<EventExec> getEventExec();
   dyn_detail::boost::shared_ptr<const EventExec> getEventExec() const;

   dyn_detail::boost::shared_ptr<EventStop> getEventStop();
   dyn_detail::boost::shared_ptr<const EventStop> getEventStop() const;

   dyn_detail::boost::shared_ptr<EventBreakpoint> getEventBreakpoint();
   dyn_detail::boost::shared_ptr<const EventBreakpoint> getEventBreakpoint() const;

   dyn_detail::boost::shared_ptr<EventNewThread> getEventNewThread();
   dyn_detail::boost::shared_ptr<const EventNewThread> getEventNewThread() const;

   dyn_detail::boost::shared_ptr<EventNewUserThread> getEventNewUserThread();
   dyn_detail::boost::shared_ptr<const EventNewUserThread> getEventNewUserThread() const;

   dyn_detail::boost::shared_ptr<EventNewLWP> getEventNewLWP();
   dyn_detail::boost::shared_ptr<const EventNewLWP> getEventNewLWP() const;

   dyn_detail::boost::shared_ptr<EventThreadDestroy> getEventThreadDestroy();
   dyn_detail::boost::shared_ptr<const EventThreadDestroy> getEventThreadDestroy() const;

   dyn_detail::boost::shared_ptr<EventUserThreadDestroy> getEventUserThreadDestroy();
   dyn_detail::boost::shared_ptr<const EventUserThreadDestroy> getEventUserThreadDestroy() const;

   dyn_detail::boost::shared_ptr<EventLWPDestroy> getEventLWPDestroy();
   dyn_detail::boost::shared_ptr<const EventLWPDestroy> getEventLWPDestroy() const;

   dyn_detail::boost::shared_ptr<EventFork> getEventFork();
   dyn_detail::boost::shared_ptr<const EventFork> getEventFork() const;

   dyn_detail::boost::shared_ptr<EventSignal> getEventSignal();
   dyn_detail::boost::shared_ptr<const EventSignal> getEventSignal() const;

   dyn_detail::boost::shared_ptr<EventBootstrap> getEventBootstrap();
   dyn_detail::boost::shared_ptr<const EventBootstrap> getEventBootstrap() const;

   dyn_detail::boost::shared_ptr<EventRPC> getEventRPC();
   dyn_detail::boost::shared_ptr<const EventRPC> getEventRPC() const;

   dyn_detail::boost::shared_ptr<EventRPCInternal> getEventRPCInternal();
   dyn_detail::boost::shared_ptr<const EventRPCInternal> getEventRPCInternal() const;

   dyn_detail::boost::shared_ptr<EventSingleStep> getEventSingleStep();
   dyn_detail::boost::shared_ptr<const EventSingleStep> getEventSingleStep() const;

   dyn_detail::boost::shared_ptr<EventBreakpointClear> getEventBreakpointClear();
   dyn_detail::boost::shared_ptr<const EventBreakpointClear> getEventBreakpointClear() const;

   dyn_detail::boost::shared_ptr<EventLibrary> getEventLibrary();
   dyn_detail::boost::shared_ptr<const EventLibrary> getEventLibrary() const;

   dyn_detail::boost::shared_ptr<EventAsync> getEventAsync();
   dyn_detail::boost::shared_ptr<const EventAsync> getEventAsync() const;

   dyn_detail::boost::shared_ptr<EventChangePCStop> getEventChangePCStop();
   dyn_detail::boost::shared_ptr<const EventChangePCStop> getEventChangePCStop() const;

   dyn_detail::boost::shared_ptr<EventDetached> getEventDetached();
   dyn_detail::boost::shared_ptr<const EventDetached> getEventDetached() const;

   dyn_detail::boost::shared_ptr<EventIntBootstrap> getEventIntBootstrap();
   dyn_detail::boost::shared_ptr<const EventIntBootstrap> getEventIntBootstrap() const;
 protected:
   EventType etype;
   Thread::const_ptr thread;
   Process::const_ptr proc;
   SyncType stype;
   std::vector<Event::ptr> subservient_events;
   Event::weak_ptr master_event;
   std::set<Handler *> handled_by;
   bool suppress_cb;
};

class EventTerminate : public Event
{
   friend void dyn_detail::boost::checked_delete<EventTerminate>(EventTerminate *);
   friend void dyn_detail::boost::checked_delete<const EventTerminate>(const EventTerminate *);
 public:
   typedef dyn_detail::boost::shared_ptr<EventTerminate> ptr;
   typedef dyn_detail::boost::shared_ptr<const EventTerminate> const_ptr;
   EventTerminate(EventType type_);
   virtual ~EventTerminate();
};

class EventExit : public EventTerminate
{
   friend void dyn_detail::boost::checked_delete<EventExit>(EventExit *);
   friend void dyn_detail::boost::checked_delete<const EventExit>(const EventExit *);
 private:
   int exitcode;
 public:
   typedef dyn_detail::boost::shared_ptr<EventExit> ptr;
   typedef dyn_detail::boost::shared_ptr<const EventExit> const_ptr;
   int getExitCode() const;
   EventExit(EventType::Time eventtime, int exitcode_);
   virtual ~EventExit();
};

class EventCrash : public EventTerminate
{
   friend void dyn_detail::boost::checked_delete<EventCrash>(EventCrash *);
   friend void dyn_detail::boost::checked_delete<const EventCrash>(const EventCrash *);
 private:
   int termsig;
 public:
   typedef dyn_detail::boost::shared_ptr<EventCrash> ptr;
   typedef dyn_detail::boost::shared_ptr<const EventCrash> const_ptr;
   int getTermSignal() const;
   EventCrash(int termsig);
   virtual ~EventCrash();
};

class EventForceTerminate : public EventTerminate
{
   friend void dyn_detail::boost::checked_delete<EventForceTerminate>(EventForceTerminate *);
   friend void dyn_detail::boost::checked_delete<const EventForceTerminate>(const EventForceTerminate *);
 private:
   int termsig;
 public:
   typedef dyn_detail::boost::shared_ptr<EventForceTerminate> ptr;
   typedef dyn_detail::boost::shared_ptr<const EventForceTerminate> const_ptr;
   int getTermSignal() const;
   EventForceTerminate(int termsig);
   virtual ~EventForceTerminate();
};

class EventExec : public Event
{
   friend void dyn_detail::boost::checked_delete<EventExec>(EventExec *);
   friend void dyn_detail::boost::checked_delete<const EventExec>(const EventExec *);
 private:
   std::string execpath;
 public:
   typedef dyn_detail::boost::shared_ptr<EventExec> ptr;
   typedef dyn_detail::boost::shared_ptr<const EventExec> const_ptr;
   EventExec(EventType::Time etime_, std::string path = std::string(""));
   virtual ~EventExec();

   std::string getExecPath() const;
   void setExecPath(std::string path_);
};

class EventStop : public Event
{
   friend void dyn_detail::boost::checked_delete<EventStop>(EventStop *);
   friend void dyn_detail::boost::checked_delete<const EventStop>(const EventStop *);
 public:
   typedef dyn_detail::boost::shared_ptr<EventStop> ptr;
   typedef dyn_detail::boost::shared_ptr<const EventStop> const_ptr;
   EventStop();
   virtual ~EventStop();
};

class EventNewThread : public Event
{
   friend void dyn_detail::boost::checked_delete<EventNewThread>(EventNewThread *);
   friend void dyn_detail::boost::checked_delete<const EventNewThread>(const EventNewThread *);
 public:
   typedef dyn_detail::boost::shared_ptr<EventNewThread> ptr;
   typedef dyn_detail::boost::shared_ptr<const EventNewThread> const_ptr;
   EventNewThread(EventType et);
   virtual ~EventNewThread();

   virtual Dyninst::LWP getLWP() const = 0;
   virtual Thread::const_ptr getNewThread() const = 0;
};

class int_eventNewUserThread;
class EventNewUserThread : public EventNewThread
{
   friend void dyn_detail::boost::checked_delete<EventNewUserThread>(EventNewUserThread *);
   friend void dyn_detail::boost::checked_delete<const EventNewUserThread>(const EventNewUserThread *);
  private:
   int_eventNewUserThread *iev;
  public:
   typedef dyn_detail::boost::shared_ptr<EventNewUserThread> ptr;
   typedef dyn_detail::boost::shared_ptr<const EventNewUserThread> const_ptr;   

   EventNewUserThread();
   virtual ~EventNewUserThread();
   int_eventNewUserThread *getInternalEvent() const;

   virtual Dyninst::LWP getLWP() const;
   virtual Thread::const_ptr getNewThread() const;
};

class EventNewLWP : public EventNewThread
{
   friend void dyn_detail::boost::checked_delete<EventNewLWP>(EventNewLWP *);
   friend void dyn_detail::boost::checked_delete<const EventNewLWP>(const EventNewLWP *);
  private:
   Dyninst::LWP lwp;
  public:
   typedef dyn_detail::boost::shared_ptr<EventNewLWP> ptr;
   typedef dyn_detail::boost::shared_ptr<const EventNewLWP> const_ptr;   
   EventNewLWP(Dyninst::LWP lwp_);
   virtual ~EventNewLWP();

   virtual Dyninst::LWP getLWP() const;
   virtual Thread::const_ptr getNewThread() const;
};

class EventThreadDestroy : public Event
{
   friend void dyn_detail::boost::checked_delete<EventThreadDestroy>(EventThreadDestroy *);
   friend void dyn_detail::boost::checked_delete<const EventThreadDestroy>(const EventThreadDestroy *);
 public:
   typedef dyn_detail::boost::shared_ptr<EventThreadDestroy> ptr;
   typedef dyn_detail::boost::shared_ptr<const EventThreadDestroy> const_ptr;
   EventThreadDestroy(EventType et);
   virtual ~EventThreadDestroy() = 0;
};

class EventUserThreadDestroy : public EventThreadDestroy
{
   friend void dyn_detail::boost::checked_delete<EventUserThreadDestroy>(EventUserThreadDestroy *);
   friend void dyn_detail::boost::checked_delete<const EventUserThreadDestroy>(const EventUserThreadDestroy *);
 public:
   typedef dyn_detail::boost::shared_ptr<EventUserThreadDestroy> ptr;
   typedef dyn_detail::boost::shared_ptr<const EventUserThreadDestroy> const_ptr;
   EventUserThreadDestroy(EventType::Time time_);
   virtual ~EventUserThreadDestroy();
};

class EventLWPDestroy : public EventThreadDestroy
{
   friend void dyn_detail::boost::checked_delete<EventLWPDestroy>(EventLWPDestroy *);
   friend void dyn_detail::boost::checked_delete<const EventLWPDestroy>(const EventLWPDestroy *);
 public:
   typedef dyn_detail::boost::shared_ptr<EventLWPDestroy> ptr;
   typedef dyn_detail::boost::shared_ptr<const EventLWPDestroy> const_ptr;
   EventLWPDestroy(EventType::Time time_);
   virtual ~EventLWPDestroy();
};

class EventFork : public Event
{
   friend void dyn_detail::boost::checked_delete<EventFork>(EventFork *);
   friend void dyn_detail::boost::checked_delete<const EventFork>(const EventFork *);
  private:
   Dyninst::PID pid;
  public:
   typedef dyn_detail::boost::shared_ptr<EventFork> ptr;
   typedef dyn_detail::boost::shared_ptr<const EventFork> const_ptr;
   EventFork(Dyninst::PID pid_);
   virtual ~EventFork();
   Dyninst::PID getPID() const;
   Process::const_ptr getChildProcess() const;
};

class EventSignal : public Event
{
   friend void dyn_detail::boost::checked_delete<EventSignal>(EventSignal *);
   friend void dyn_detail::boost::checked_delete<const EventSignal>(const EventSignal *);
 private:
   int sig;
 public:
   typedef dyn_detail::boost::shared_ptr<EventSignal> ptr;
   typedef dyn_detail::boost::shared_ptr<const EventSignal> const_ptr;
   EventSignal(int sig);
   virtual ~EventSignal();

   int getSignal() const;
   void clearSignal() const;
};

class EventBootstrap : public Event
{
   friend void dyn_detail::boost::checked_delete<EventBootstrap>(EventBootstrap *);
   friend void dyn_detail::boost::checked_delete<const EventBootstrap>(const EventBootstrap *);
 public:
   typedef dyn_detail::boost::shared_ptr<EventBootstrap> ptr;
   typedef dyn_detail::boost::shared_ptr<const EventBootstrap> const_ptr;
   EventBootstrap();
   virtual ~EventBootstrap();
};

class int_eventRPC;
class EventRPC : public Event
{
   friend void dyn_detail::boost::checked_delete<EventRPC>(EventRPC *);
   friend void dyn_detail::boost::checked_delete<const EventRPC>(const EventRPC *);
 private:
   int_eventRPC *int_rpc;
   rpc_wrapper *wrapper;
 public:
   virtual bool suppressCB() const;
   rpc_wrapper *getllRPC();
   typedef dyn_detail::boost::shared_ptr<EventRPC> ptr;
   typedef dyn_detail::boost::shared_ptr<const EventRPC> const_ptr;
   EventRPC(rpc_wrapper *wrapper_);
   virtual ~EventRPC();

   IRPC::const_ptr getIRPC() const;
   int_eventRPC *getInternal() const;
};

class EventRPCInternal : public Event
{
   friend void dyn_detail::boost::checked_delete<EventRPCInternal>(EventRPCInternal *);
   friend void dyn_detail::boost::checked_delete<const EventRPCInternal>(const EventRPCInternal *);
 public:
   typedef dyn_detail::boost::shared_ptr<EventRPCInternal> ptr;
   typedef dyn_detail::boost::shared_ptr<const EventRPCInternal> const_ptr;
   virtual bool suppressCB() const;
   EventRPCInternal();
   virtual ~EventRPCInternal();
};

class EventSingleStep : public Event
{
   friend void dyn_detail::boost::checked_delete<EventSingleStep>(EventSingleStep *);
   friend void dyn_detail::boost::checked_delete<const EventSingleStep>(const EventSingleStep *);
 public:
   typedef dyn_detail::boost::shared_ptr<EventSingleStep> ptr;
   typedef dyn_detail::boost::shared_ptr<const EventSingleStep> const_ptr;
   EventSingleStep();
   virtual ~EventSingleStep();
};

class int_eventBreakpoint;
class EventBreakpoint : public Event
{
   friend void dyn_detail::boost::checked_delete<EventBreakpoint>(EventBreakpoint *);
   friend void dyn_detail::boost::checked_delete<const EventBreakpoint>(const EventBreakpoint *);
 private:
   installed_breakpoint *ibp;
   int_eventBreakpoint *int_bp;
   Dyninst::Address addr;
 public:
   typedef dyn_detail::boost::shared_ptr<EventBreakpoint> ptr;
   typedef dyn_detail::boost::shared_ptr<const EventBreakpoint> const_ptr;
   installed_breakpoint *installedbp() const;
   int_eventBreakpoint *getInternal() const;

   EventBreakpoint(Dyninst::Address addr, installed_breakpoint *ibp_);
   virtual ~EventBreakpoint();

   Dyninst::Address getAddress() const;
   void getBreakpoints(std::vector<Breakpoint::const_ptr> &bps) const;
   virtual bool suppressCB() const;
   virtual bool procStopper() const;
};


class int_eventBreakpointClear;
class EventBreakpointClear : public Event
{
   friend void dyn_detail::boost::checked_delete<EventBreakpointClear>(EventBreakpointClear *);
   friend void dyn_detail::boost::checked_delete<const EventBreakpointClear>(const EventBreakpointClear *);
  private:
   installed_breakpoint *bp_;
   int_eventBreakpointClear *int_bpc;
  public:
   typedef dyn_detail::boost::shared_ptr<EventBreakpointClear> ptr;
   typedef dyn_detail::boost::shared_ptr<const EventBreakpointClear> const_ptr;
   EventBreakpointClear(installed_breakpoint *bp);
   virtual ~EventBreakpointClear();
   
   installed_breakpoint *bp() const;
   int_eventBreakpointClear *getInternal() const;
};

class EventLibrary : public Event
{
   friend void dyn_detail::boost::checked_delete<EventLibrary>(EventLibrary *);
   friend void dyn_detail::boost::checked_delete<const EventLibrary>(const EventLibrary *);
 private:
   std::set<Library::ptr> added_libs;
   std::set<Library::ptr> rmd_libs;
 public:
   typedef dyn_detail::boost::shared_ptr<EventLibrary> ptr;
   typedef dyn_detail::boost::shared_ptr<const EventLibrary> const_ptr;
   EventLibrary();
   EventLibrary(const std::set<Library::ptr> &added_libs_,
                const std::set<Library::ptr> &rmd_libs_);
   virtual ~EventLibrary();

   void setLibs(const std::set<Library::ptr> &added_libs_,
                const std::set<Library::ptr> &rmd_libs_);
   const std::set<Library::ptr> &libsAdded() const;
   const std::set<Library::ptr> &libsRemoved() const;
};

class int_eventAsync;
class EventAsync : public Event
{
   friend void dyn_detail::boost::checked_delete<EventAsync>(EventAsync *);
   friend void dyn_detail::boost::checked_delete<const EventAsync>(const EventAsync *);
   
  private:
   int_eventAsync *internal;
  public:
   typedef dyn_detail::boost::shared_ptr<EventAsync> ptr;
   typedef dyn_detail::boost::shared_ptr<const EventAsync> const_ptr;

   EventAsync(int_eventAsync *ievent);
   virtual ~EventAsync();

   int_eventAsync *getInternal() const;
};

class EventChangePCStop : public Event
{
   friend void dyn_detail::boost::checked_delete<EventChangePCStop>(EventChangePCStop *);
   friend void dyn_detail::boost::checked_delete<const EventChangePCStop>(const EventChangePCStop *);
 public:
   typedef dyn_detail::boost::shared_ptr<EventChangePCStop> ptr;
   typedef dyn_detail::boost::shared_ptr<const EventChangePCStop> const_ptr;
   EventChangePCStop();
   ~EventChangePCStop();
};

class EventDetached : public Event
{
   friend void dyn_detail::boost::checked_delete<EventDetached>(EventDetached *);
   friend void dyn_detail::boost::checked_delete<const EventDetached>(const EventDetached *);
 public:
   typedef dyn_detail::boost::shared_ptr<EventDetached> ptr;
   typedef dyn_detail::boost::shared_ptr<const EventDetached> const_ptr;
   EventDetached();
   ~EventDetached();
};

class EventIntBootstrap : public Event
{
   friend void dyn_detail::boost::checked_delete<EventIntBootstrap>(EventIntBootstrap *);
   friend void dyn_detail::boost::checked_delete<const EventIntBootstrap>(const EventIntBootstrap *);
   
   void *data;
 public:
   typedef dyn_detail::boost::shared_ptr<EventIntBootstrap> ptr;
   typedef dyn_detail::boost::shared_ptr<const EventIntBootstrap> const_ptr;
   EventIntBootstrap(void *d = NULL);
   ~EventIntBootstrap();

   void *getData() const;
   void setData(void *v);
};

}
}
#endif
