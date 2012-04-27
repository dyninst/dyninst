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
#if !defined(EVENT_H_)
#define EVENT_H_

#include <string>
#include <set>
#include "dyntypes.h"
#include "EventType.h"
#include "Process.h"
#include "util.h"

#include "dynptr.h"

class installed_breakpoint;
class HandlerPool;
class HandleCallbacks;
class emulated_singlestep;

namespace Dyninst {
namespace ProcControlAPI {

class PC_EXPORT ArchEvent
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
class EventBreakpointRestore;
class EventLibrary;
class EventRPCLaunch;
class EventAsync;
class EventChangePCStop;
class EventPreBootstrap;
class EventDetach;
class EventIntBootstrap;
class EventNop;
class EventThreadDB;

class PC_EXPORT Event : public dyn_enable_shared_from_this<Event>
{
   friend void dyn_checked_delete<Event>(Event *);
   friend void dyn_checked_delete<const Event>(const Event *);
   friend class ::HandlerPool;
   friend class ::int_process;
   friend class ::HandleCallbacks;
 public:
   typedef dyn_shared_ptr<Event> ptr;
   typedef dyn_shared_ptr<const Event> const_ptr;
   typedef dyn_weak_ptr<Event> weak_ptr;

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
   virtual bool userEvent() const;
   virtual void setUserEvent(bool b);
   virtual bool procStopper() const;

   Event::weak_ptr subservientTo() const;
   void addSubservientEvent(Event::ptr ev);

   dyn_shared_ptr<EventTerminate> getEventTerminate();
   dyn_shared_ptr<const EventTerminate> getEventTerminate() const;

   dyn_shared_ptr<EventExit> getEventExit();
   dyn_shared_ptr<const EventExit> getEventExit() const;

   dyn_shared_ptr<EventCrash> getEventCrash();
   dyn_shared_ptr<const EventCrash> getEventCrash() const;

   dyn_shared_ptr<EventForceTerminate> getEventForceTerminate();
   dyn_shared_ptr<const EventForceTerminate> getEventForceTerminate() const;

   dyn_shared_ptr<EventExec> getEventExec();
   dyn_shared_ptr<const EventExec> getEventExec() const;

   dyn_shared_ptr<EventStop> getEventStop();
   dyn_shared_ptr<const EventStop> getEventStop() const;

   dyn_shared_ptr<EventBreakpoint> getEventBreakpoint();
   dyn_shared_ptr<const EventBreakpoint> getEventBreakpoint() const;

   dyn_shared_ptr<EventNewThread> getEventNewThread();
   dyn_shared_ptr<const EventNewThread> getEventNewThread() const;

   dyn_shared_ptr<EventNewUserThread> getEventNewUserThread();
   dyn_shared_ptr<const EventNewUserThread> getEventNewUserThread() const;

   dyn_shared_ptr<EventNewLWP> getEventNewLWP();
   dyn_shared_ptr<const EventNewLWP> getEventNewLWP() const;

   dyn_shared_ptr<EventThreadDestroy> getEventThreadDestroy();
   dyn_shared_ptr<const EventThreadDestroy> getEventThreadDestroy() const;

   dyn_shared_ptr<EventUserThreadDestroy> getEventUserThreadDestroy();
   dyn_shared_ptr<const EventUserThreadDestroy> getEventUserThreadDestroy() const;

   dyn_shared_ptr<EventLWPDestroy> getEventLWPDestroy();
   dyn_shared_ptr<const EventLWPDestroy> getEventLWPDestroy() const;

   dyn_shared_ptr<EventFork> getEventFork();
   dyn_shared_ptr<const EventFork> getEventFork() const;

   dyn_shared_ptr<EventSignal> getEventSignal();
   dyn_shared_ptr<const EventSignal> getEventSignal() const;

   dyn_shared_ptr<EventBootstrap> getEventBootstrap();
   dyn_shared_ptr<const EventBootstrap> getEventBootstrap() const;

   dyn_shared_ptr<EventPreBootstrap> getEventPreBootstrap();
   dyn_shared_ptr<const EventPreBootstrap> getEventPreBootstrap() const;

   dyn_shared_ptr<EventRPC> getEventRPC();
   dyn_shared_ptr<const EventRPC> getEventRPC() const;

   dyn_shared_ptr<EventRPCLaunch> getEventRPCLaunch();
   dyn_shared_ptr<const EventRPCLaunch> getEventRPCLaunch() const;

   dyn_shared_ptr<EventSingleStep> getEventSingleStep();
   dyn_shared_ptr<const EventSingleStep> getEventSingleStep() const;

   dyn_shared_ptr<EventBreakpointClear> getEventBreakpointClear();
   dyn_shared_ptr<const EventBreakpointClear> getEventBreakpointClear() const;

   dyn_shared_ptr<EventBreakpointRestore> getEventBreakpointRestore();
   dyn_shared_ptr<const EventBreakpointRestore> getEventBreakpointRestore() const;

   dyn_shared_ptr<EventLibrary> getEventLibrary();
   dyn_shared_ptr<const EventLibrary> getEventLibrary() const;

   dyn_shared_ptr<EventAsync> getEventAsync();
   dyn_shared_ptr<const EventAsync> getEventAsync() const;

   dyn_shared_ptr<EventChangePCStop> getEventChangePCStop();
   dyn_shared_ptr<const EventChangePCStop> getEventChangePCStop() const;

   dyn_shared_ptr<EventDetach> getEventDetach();
   dyn_shared_ptr<const EventDetach> getEventDetach() const;

   dyn_shared_ptr<EventIntBootstrap> getEventIntBootstrap();
   dyn_shared_ptr<const EventIntBootstrap> getEventIntBootstrap() const;

   dyn_shared_ptr<EventNop> getEventNop();
   dyn_shared_ptr<const EventNop> getEventNop() const;

   dyn_shared_ptr<EventThreadDB> getEventThreadDB();
   dyn_shared_ptr<const EventThreadDB> getEventThreadDB() const;

   //Not meant for public consumption
   void setLastError(err_t ec, const char *es);
 protected:
   EventType etype;
   Thread::const_ptr thread;
   Process::const_ptr proc;
   SyncType stype;
   std::vector<Event::ptr> subservient_events;
   Event::weak_ptr master_event;
   std::set<Handler *> handled_by;
   bool suppress_cb;
   bool user_event;
   bool handling_started;
   bool noted_event;
};

template<typename OS>
OS& operator<<(OS& str, Event& e)
{
	str << e.getEventType().name() << " event is ";
	switch(e.getSyncType())
	{
	case Event::async:
		str << "asynchronous ";
		break;
	case Event::sync_thread:
		str << "thread synchronous ";
		break;
	case Event::sync_process:
		str << "process synchronous ";
		break;
	default:
		str << "<UNKNOWN SYNC TYPE> ";
		break;
	}
	str << "on " << (e.getProcess() ? e.getProcess()->getPid() : -1);
	str << "/" << ((e.getThread()) ?  e.getThread()->getLWP() : (Dyninst::LWP) -1);
	str << "\n";
	return str;
}

class PC_EXPORT EventTerminate : public Event
{
   friend void dyn_checked_delete<EventTerminate>(EventTerminate *);
   friend void dyn_checked_delete<const EventTerminate>(const EventTerminate *);
 public:
   typedef dyn_shared_ptr<EventTerminate> ptr;
   typedef dyn_shared_ptr<const EventTerminate> const_ptr;
   EventTerminate(EventType type_);
   virtual ~EventTerminate();
};

class PC_EXPORT EventExit : public EventTerminate
{
   friend void dyn_checked_delete<EventExit>(EventExit *);
   friend void dyn_checked_delete<const EventExit>(const EventExit *);
 private:
   int exitcode;
 public:
   typedef dyn_shared_ptr<EventExit> ptr;
   typedef dyn_shared_ptr<const EventExit> const_ptr;
   int getExitCode() const;
   EventExit(EventType::Time eventtime, int exitcode_);
   virtual ~EventExit();
};

class PC_EXPORT EventCrash : public EventTerminate
{
   friend void dyn_checked_delete<EventCrash>(EventCrash *);
   friend void dyn_checked_delete<const EventCrash>(const EventCrash *);
 private:
   int termsig;
 public:
   typedef dyn_shared_ptr<EventCrash> ptr;
   typedef dyn_shared_ptr<const EventCrash> const_ptr;
   int getTermSignal() const;
   EventCrash(int termsig);
   virtual ~EventCrash();
};

class PC_EXPORT EventForceTerminate : public EventTerminate
{
   friend void dyn_checked_delete<EventForceTerminate>(EventForceTerminate *);
   friend void dyn_checked_delete<const EventForceTerminate>(const EventForceTerminate *);
 private:
   int termsig;
 public:
   typedef dyn_shared_ptr<EventForceTerminate> ptr;
   typedef dyn_shared_ptr<const EventForceTerminate> const_ptr;
   int getTermSignal() const;
   EventForceTerminate(int termsig);
   virtual ~EventForceTerminate();
};

class PC_EXPORT EventExec : public Event
{
   friend void dyn_checked_delete<EventExec>(EventExec *);
   friend void dyn_checked_delete<const EventExec>(const EventExec *);
 private:
   std::string execpath;
 public:
   typedef dyn_shared_ptr<EventExec> ptr;
   typedef dyn_shared_ptr<const EventExec> const_ptr;
   EventExec(EventType::Time etime_, std::string path = std::string(""));
   virtual ~EventExec();

   std::string getExecPath() const;
   void setExecPath(std::string path_);
};

class PC_EXPORT EventStop : public Event
{
   friend void dyn_checked_delete<EventStop>(EventStop *);
   friend void dyn_checked_delete<const EventStop>(const EventStop *);
 public:
   typedef dyn_shared_ptr<EventStop> ptr;
   typedef dyn_shared_ptr<const EventStop> const_ptr;
   EventStop();
   virtual ~EventStop();
};

class PC_EXPORT EventNewThread : public Event
{
   friend void dyn_checked_delete<EventNewThread>(EventNewThread *);
   friend void dyn_checked_delete<const EventNewThread>(const EventNewThread *);
 public:
   typedef dyn_shared_ptr<EventNewThread> ptr;
   typedef dyn_shared_ptr<const EventNewThread> const_ptr;
   EventNewThread(EventType et);
   virtual ~EventNewThread();

   virtual Dyninst::LWP getLWP() const = 0;
   virtual Thread::const_ptr getNewThread() const = 0;
};

class int_eventNewUserThread;
class PC_EXPORT EventNewUserThread : public EventNewThread
{
   friend void dyn_checked_delete<EventNewUserThread>(EventNewUserThread *);
   friend void dyn_checked_delete<const EventNewUserThread>(const EventNewUserThread *);
  private:
   int_eventNewUserThread *iev;
  public:
   typedef dyn_shared_ptr<EventNewUserThread> ptr;
   typedef dyn_shared_ptr<const EventNewUserThread> const_ptr;   

   EventNewUserThread();
   virtual ~EventNewUserThread();
   int_eventNewUserThread *getInternalEvent() const;

   virtual Dyninst::LWP getLWP() const;
   virtual Thread::const_ptr getNewThread() const;
};

class PC_EXPORT EventNewLWP : public EventNewThread
{
   friend void dyn_checked_delete<EventNewLWP>(EventNewLWP *);
   friend void dyn_checked_delete<const EventNewLWP>(const EventNewLWP *);
  private:
   Dyninst::LWP lwp;
  public:
   typedef dyn_shared_ptr<EventNewLWP> ptr;
   typedef dyn_shared_ptr<const EventNewLWP> const_ptr;   
   EventNewLWP(Dyninst::LWP lwp_);
   virtual ~EventNewLWP();

   virtual Dyninst::LWP getLWP() const;
   virtual Thread::const_ptr getNewThread() const;
};

class PC_EXPORT EventThreadDestroy : public Event
{
   friend void dyn_checked_delete<EventThreadDestroy>(EventThreadDestroy *);
   friend void dyn_checked_delete<const EventThreadDestroy>(const EventThreadDestroy *);
 public:
   typedef dyn_shared_ptr<EventThreadDestroy> ptr;
   typedef dyn_shared_ptr<const EventThreadDestroy> const_ptr;
   EventThreadDestroy(EventType et);
   virtual ~EventThreadDestroy() = 0;
};

class PC_EXPORT EventUserThreadDestroy : public EventThreadDestroy
{
   friend void dyn_checked_delete<EventUserThreadDestroy>(EventUserThreadDestroy *);
   friend void dyn_checked_delete<const EventUserThreadDestroy>(const EventUserThreadDestroy *);
 public:
   typedef dyn_shared_ptr<EventUserThreadDestroy> ptr;
   typedef dyn_shared_ptr<const EventUserThreadDestroy> const_ptr;
   EventUserThreadDestroy(EventType::Time time_);
   virtual ~EventUserThreadDestroy();
};

class PC_EXPORT EventLWPDestroy : public EventThreadDestroy
{
   friend void dyn_checked_delete<EventLWPDestroy>(EventLWPDestroy *);
   friend void dyn_checked_delete<const EventLWPDestroy>(const EventLWPDestroy *);
 public:
   typedef dyn_shared_ptr<EventLWPDestroy> ptr;
   typedef dyn_shared_ptr<const EventLWPDestroy> const_ptr;
   EventLWPDestroy(EventType::Time time_);
   virtual ~EventLWPDestroy();
};

class PC_EXPORT EventFork : public Event
{
   friend void dyn_checked_delete<EventFork>(EventFork *);
   friend void dyn_checked_delete<const EventFork>(const EventFork *);
  private:
   Dyninst::PID pid;
  public:
   typedef dyn_shared_ptr<EventFork> ptr;
   typedef dyn_shared_ptr<const EventFork> const_ptr;
   EventFork(EventType::Time time_, Dyninst::PID pid_);
   virtual ~EventFork();
   Dyninst::PID getPID() const;
   Process::const_ptr getChildProcess() const;
};

class PC_EXPORT EventSignal : public Event
{
   friend void dyn_checked_delete<EventSignal>(EventSignal *);
   friend void dyn_checked_delete<const EventSignal>(const EventSignal *);
 private:
   int sig;
 public:
   typedef dyn_shared_ptr<EventSignal> ptr;
   typedef dyn_shared_ptr<const EventSignal> const_ptr;
   EventSignal(int sig);
   virtual ~EventSignal();

   int getSignal() const;
   void setThreadSignal(int newSignal) const;
   void clearThreadSignal() const;
};

class PC_EXPORT EventBootstrap : public Event
{
   friend void dyn_checked_delete<EventBootstrap>(EventBootstrap *);
   friend void dyn_checked_delete<const EventBootstrap>(const EventBootstrap *);
 public:
   typedef dyn_shared_ptr<EventBootstrap> ptr;
   typedef dyn_shared_ptr<const EventBootstrap> const_ptr;
   EventBootstrap();
   virtual ~EventBootstrap();
};

class PC_EXPORT EventPreBootstrap : public Event
{
   friend void dyn_checked_delete<EventPreBootstrap>(EventPreBootstrap *);
   friend void dyn_checked_delete<const EventPreBootstrap>(const EventPreBootstrap *);
 public:
   typedef dyn_shared_ptr<EventPreBootstrap> ptr;
   typedef dyn_shared_ptr<const EventPreBootstrap> const_ptr;
   EventPreBootstrap();
   virtual ~EventPreBootstrap();
};


class int_eventRPC;
class PC_EXPORT EventRPC : public Event
{
   friend void dyn_checked_delete<EventRPC>(EventRPC *);
   friend void dyn_checked_delete<const EventRPC>(const EventRPC *);
 private:
   int_eventRPC *int_rpc;
   rpc_wrapper *wrapper;
 public:
   virtual bool suppressCB() const;
   rpc_wrapper *getllRPC();
   typedef dyn_shared_ptr<EventRPC> ptr;
   typedef dyn_shared_ptr<const EventRPC> const_ptr;
   EventRPC(rpc_wrapper *wrapper_);
   virtual ~EventRPC();

   IRPC::const_ptr getIRPC() const;
   int_eventRPC *getInternal() const;
};

class PC_EXPORT EventRPCLaunch : public Event
{
   friend void dyn_checked_delete<EventRPCLaunch>(EventRPCLaunch *);
   friend void dyn_checked_delete<const EventRPCLaunch>(const EventRPCLaunch *);
 public:
   typedef dyn_shared_ptr<EventRPCLaunch> ptr;
   typedef dyn_shared_ptr<const EventRPCLaunch> const_ptr;
   virtual bool procStopper() const;
   EventRPCLaunch();
   virtual ~EventRPCLaunch();
};

class PC_EXPORT EventSingleStep : public Event
{
   friend void dyn_checked_delete<EventSingleStep>(EventSingleStep *);
   friend void dyn_checked_delete<const EventSingleStep>(const EventSingleStep *);
 public:
   typedef dyn_shared_ptr<EventSingleStep> ptr;
   typedef dyn_shared_ptr<const EventSingleStep> const_ptr;
   EventSingleStep();
   virtual ~EventSingleStep();
};

class int_eventBreakpoint;
class PC_EXPORT EventBreakpoint : public Event
{
   friend void dyn_checked_delete<EventBreakpoint>(EventBreakpoint *);
   friend void dyn_checked_delete<const EventBreakpoint>(const EventBreakpoint *);
 private:
   int_eventBreakpoint *int_bp;
 public:
   typedef dyn_shared_ptr<EventBreakpoint> ptr;
   typedef dyn_shared_ptr<const EventBreakpoint> const_ptr;
   int_eventBreakpoint *getInternal() const;

   EventBreakpoint(int_eventBreakpoint *ibp);
   virtual ~EventBreakpoint();

   Dyninst::Address getAddress() const;
   void getBreakpoints(std::vector<Breakpoint::const_ptr> &bps) const;
   void getBreakpoints(std::vector<Breakpoint::ptr> &bps);
   virtual bool suppressCB() const;
   virtual bool procStopper() const;
};


class int_eventBreakpointClear;
class PC_EXPORT EventBreakpointClear : public Event
{
   friend void dyn_checked_delete<EventBreakpointClear>(EventBreakpointClear *);
   friend void dyn_checked_delete<const EventBreakpointClear>(const EventBreakpointClear *);
  private:
   int_eventBreakpointClear *int_bpc;
  public:
   typedef dyn_shared_ptr<EventBreakpointClear> ptr;
   typedef dyn_shared_ptr<const EventBreakpointClear> const_ptr;
   EventBreakpointClear();
   virtual ~EventBreakpointClear();
   
   int_eventBreakpointClear *getInternal() const;
   virtual bool procStopper() const;
};

class int_eventBreakpointRestore;
class EventBreakpointRestore : public Event
{
   friend void dyn_checked_delete<EventBreakpointRestore>(EventBreakpointRestore *);
   friend void dyn_checked_delete<const EventBreakpointRestore>(const EventBreakpointRestore *);
  private:
   int_eventBreakpointRestore *int_bpr;
  public:
   typedef dyn_shared_ptr<EventBreakpointRestore> ptr;
   typedef dyn_shared_ptr<const EventBreakpointRestore> const_ptr;

   EventBreakpointRestore(int_eventBreakpointRestore *iebpr);
   virtual ~EventBreakpointRestore();

   int_eventBreakpointRestore *getInternal() const;
};

class PC_EXPORT EventLibrary : public Event
{
   friend void dyn_checked_delete<EventLibrary>(EventLibrary *);
   friend void dyn_checked_delete<const EventLibrary>(const EventLibrary *);
 private:
   std::set<Library::ptr> added_libs;
   std::set<Library::ptr> rmd_libs;
 public:
   typedef dyn_shared_ptr<EventLibrary> ptr;
   typedef dyn_shared_ptr<const EventLibrary> const_ptr;
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
class PC_EXPORT EventAsync : public Event
{
   friend void dyn_checked_delete<EventAsync>(EventAsync *);
   friend void dyn_checked_delete<const EventAsync>(const EventAsync *);
   
  private:
   int_eventAsync *internal;
  public:
   typedef dyn_shared_ptr<EventAsync> ptr;
   typedef dyn_shared_ptr<const EventAsync> const_ptr;

   EventAsync(int_eventAsync *ievent);
   virtual ~EventAsync();
   int_eventAsync *getInternal() const;
};

class PC_EXPORT EventChangePCStop : public Event
{
   friend void dyn_checked_delete<EventChangePCStop>(EventChangePCStop *);
   friend void dyn_checked_delete<const EventChangePCStop>(const EventChangePCStop *);
 public:
   typedef dyn_shared_ptr<EventChangePCStop> ptr;
   typedef dyn_shared_ptr<const EventChangePCStop> const_ptr;
   EventChangePCStop();
   virtual ~EventChangePCStop();
};

class int_eventDetach;
class PC_EXPORT EventDetach : public Event
{
   friend void dyn_checked_delete<EventDetach>(EventDetach *);
   friend void dyn_checked_delete<const EventDetach>(const EventDetach *);
   int_eventDetach *int_detach;
 public:
   typedef dyn_shared_ptr<EventDetach> ptr;
   typedef dyn_shared_ptr<const EventDetach> const_ptr;

   EventDetach();
   virtual ~EventDetach();
   int_eventDetach *getInternal() const;
   virtual bool procStopper() const;
};

class PC_EXPORT EventIntBootstrap : public Event
{
   friend void dyn_checked_delete<EventIntBootstrap>(EventIntBootstrap *);
   friend void dyn_checked_delete<const EventIntBootstrap>(const EventIntBootstrap *);
   
   void *data;
 public:
   typedef dyn_shared_ptr<EventIntBootstrap> ptr;
   typedef dyn_shared_ptr<const EventIntBootstrap> const_ptr;
   EventIntBootstrap(void *d = NULL);
   virtual ~EventIntBootstrap();

   void *getData() const;
   void setData(void *v);
};

class PC_EXPORT EventNop : public Event
{
   friend void dyn_checked_delete<EventNop>(EventNop *);
   friend void dyn_checked_delete<const EventNop>(const EventNop *);
 public:
   typedef dyn_shared_ptr<EventNop> ptr;
   typedef dyn_shared_ptr<const EventNop> const_ptr;
   EventNop();
   virtual ~EventNop();
};

class int_eventThreadDB;
class PC_EXPORT EventThreadDB : public Event
{
   friend void dyn_checked_delete<EventThreadDB>(EventThreadDB *);
   friend void dyn_checked_delete<const EventThreadDB>(const EventThreadDB *);
   int_eventThreadDB *int_etdb;
  public:
   typedef dyn_shared_ptr<EventThreadDB> ptr;
   typedef dyn_shared_ptr<const EventThreadDB> const_ptr;
   int_eventThreadDB *getInternal() const;

   EventThreadDB();
   virtual ~EventThreadDB();

   virtual bool triggersCB() const;
};

}
}
#endif
