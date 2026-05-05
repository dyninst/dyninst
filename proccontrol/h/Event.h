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
#if !defined(EVENT_H_)
#define EVENT_H_

#include <string>
#include <set>
#include <stddef.h>
#include <vector>
#include "dyntypes.h"
#include "MachSyscall.h"
#include "EventType.h"
#include "PCProcess.h"
#include "util.h"

class HandlerPool;
class HandleCallbacks;

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
class EventWinStopThreadDestroy;
class EventControlAuthority;
class EventAsyncIO;
class EventAsyncRead;
class EventAsyncWrite;
class EventAsyncReadAllRegs;
class EventAsyncSetAllRegs;
class EventAsyncFileRead;
class EventPostponedSyscall;
class EventSyscall;
class EventPreSyscall;
class EventPostSyscall;

class PC_EXPORT Event : public dyncompat::enable_shared_from_this<Event>
{
   friend void dyncompat::checked_delete<Event>(Event *) noexcept;
   friend void dyncompat::checked_delete<const Event>(const Event *) noexcept;
   friend class ::HandlerPool;
   friend class ::int_process;
   friend class ::HandleCallbacks;
 public:
   typedef dyncompat::shared_ptr<Event> ptr;
   typedef dyncompat::shared_ptr<const Event> const_ptr;
   typedef dyncompat::weak_ptr<Event> weak_ptr;

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

   dyncompat::shared_ptr<EventTerminate> getEventTerminate();
   dyncompat::shared_ptr<const EventTerminate> getEventTerminate() const;

   dyncompat::shared_ptr<EventExit> getEventExit();
   dyncompat::shared_ptr<const EventExit> getEventExit() const;

   dyncompat::shared_ptr<EventCrash> getEventCrash();
   dyncompat::shared_ptr<const EventCrash> getEventCrash() const;

   dyncompat::shared_ptr<EventForceTerminate> getEventForceTerminate();
   dyncompat::shared_ptr<const EventForceTerminate> getEventForceTerminate() const;

   dyncompat::shared_ptr<EventExec> getEventExec();
   dyncompat::shared_ptr<const EventExec> getEventExec() const;

   dyncompat::shared_ptr<EventStop> getEventStop();
   dyncompat::shared_ptr<const EventStop> getEventStop() const;

   dyncompat::shared_ptr<EventBreakpoint> getEventBreakpoint();
   dyncompat::shared_ptr<const EventBreakpoint> getEventBreakpoint() const;

   dyncompat::shared_ptr<EventNewThread> getEventNewThread();
   dyncompat::shared_ptr<const EventNewThread> getEventNewThread() const;

   dyncompat::shared_ptr<EventNewUserThread> getEventNewUserThread();
   dyncompat::shared_ptr<const EventNewUserThread> getEventNewUserThread() const;

   dyncompat::shared_ptr<EventNewLWP> getEventNewLWP();
   dyncompat::shared_ptr<const EventNewLWP> getEventNewLWP() const;

   dyncompat::shared_ptr<EventThreadDestroy> getEventThreadDestroy();
   dyncompat::shared_ptr<const EventThreadDestroy> getEventThreadDestroy() const;

   dyncompat::shared_ptr<EventUserThreadDestroy> getEventUserThreadDestroy();
   dyncompat::shared_ptr<const EventUserThreadDestroy> getEventUserThreadDestroy() const;

   dyncompat::shared_ptr<EventLWPDestroy> getEventLWPDestroy();
   dyncompat::shared_ptr<const EventLWPDestroy> getEventLWPDestroy() const;

   dyncompat::shared_ptr<EventFork> getEventFork();
   dyncompat::shared_ptr<const EventFork> getEventFork() const;

   dyncompat::shared_ptr<EventSignal> getEventSignal();
   dyncompat::shared_ptr<const EventSignal> getEventSignal() const;

   dyncompat::shared_ptr<EventBootstrap> getEventBootstrap();
   dyncompat::shared_ptr<const EventBootstrap> getEventBootstrap() const;

   dyncompat::shared_ptr<EventPreBootstrap> getEventPreBootstrap();
   dyncompat::shared_ptr<const EventPreBootstrap> getEventPreBootstrap() const;

   dyncompat::shared_ptr<EventRPC> getEventRPC();
   dyncompat::shared_ptr<const EventRPC> getEventRPC() const;

   dyncompat::shared_ptr<EventRPCLaunch> getEventRPCLaunch();
   dyncompat::shared_ptr<const EventRPCLaunch> getEventRPCLaunch() const;

   dyncompat::shared_ptr<EventSingleStep> getEventSingleStep();
   dyncompat::shared_ptr<const EventSingleStep> getEventSingleStep() const;

   dyncompat::shared_ptr<EventBreakpointClear> getEventBreakpointClear();
   dyncompat::shared_ptr<const EventBreakpointClear> getEventBreakpointClear() const;

   dyncompat::shared_ptr<EventBreakpointRestore> getEventBreakpointRestore();
   dyncompat::shared_ptr<const EventBreakpointRestore> getEventBreakpointRestore() const;

   dyncompat::shared_ptr<EventLibrary> getEventLibrary();
   dyncompat::shared_ptr<const EventLibrary> getEventLibrary() const;

   dyncompat::shared_ptr<EventAsync> getEventAsync();
   dyncompat::shared_ptr<const EventAsync> getEventAsync() const;

   dyncompat::shared_ptr<EventChangePCStop> getEventChangePCStop();
   dyncompat::shared_ptr<const EventChangePCStop> getEventChangePCStop() const;

   dyncompat::shared_ptr<EventDetach> getEventDetach();
   dyncompat::shared_ptr<const EventDetach> getEventDetach() const;

   dyncompat::shared_ptr<EventIntBootstrap> getEventIntBootstrap();
   dyncompat::shared_ptr<const EventIntBootstrap> getEventIntBootstrap() const;

   dyncompat::shared_ptr<EventNop> getEventNop();
   dyncompat::shared_ptr<const EventNop> getEventNop() const;

   dyncompat::shared_ptr<EventThreadDB> getEventThreadDB();
   dyncompat::shared_ptr<const EventThreadDB> getEventThreadDB() const;

   dyncompat::shared_ptr<EventWinStopThreadDestroy> getEventWinStopThreadDestroy();
   dyncompat::shared_ptr<const EventWinStopThreadDestroy> getEventWinStopThreadDestroy() const;

   dyncompat::shared_ptr<EventControlAuthority> getEventControlAuthority();
   dyncompat::shared_ptr<const EventControlAuthority> getEventControlAuthority() const;

   dyncompat::shared_ptr<EventAsyncIO> getEventAsyncIO();
   dyncompat::shared_ptr<const EventAsyncIO> getEventAsyncIO() const;

   dyncompat::shared_ptr<EventAsyncRead> getEventAsyncRead();
   dyncompat::shared_ptr<const EventAsyncRead> getEventAsyncRead() const;

   dyncompat::shared_ptr<EventAsyncWrite> getEventAsyncWrite();
   dyncompat::shared_ptr<const EventAsyncWrite> getEventAsyncWrite() const;

   dyncompat::shared_ptr<EventAsyncReadAllRegs> getEventAsyncReadAllRegs();
   dyncompat::shared_ptr<const EventAsyncReadAllRegs> getEventAsyncReadAllRegs() const;

   dyncompat::shared_ptr<EventAsyncSetAllRegs> getEventAsyncSetAllRegs();
   dyncompat::shared_ptr<const EventAsyncSetAllRegs> getEventAsyncSetAllRegs() const;

   dyncompat::shared_ptr<EventAsyncFileRead> getEventAsyncFileRead();
   dyncompat::shared_ptr<const EventAsyncFileRead> getEventAsyncFileRead() const;

   dyncompat::shared_ptr<EventPostponedSyscall> getEventPostponedSyscall();
   dyncompat::shared_ptr<const EventPostponedSyscall> getEventPostponedSyscall() const;

   dyncompat::shared_ptr<EventSyscall> getEventSyscall();
   dyncompat::shared_ptr<const EventSyscall> getEventSyscall() const;
   
   dyncompat::shared_ptr<EventPreSyscall> getEventPreSyscall();
   dyncompat::shared_ptr<const EventPreSyscall> getEventPreSyscall() const;
   
   dyncompat::shared_ptr<EventPostSyscall> getEventPostSyscall();
   dyncompat::shared_ptr<const EventPostSyscall> getEventPostSyscall() const;


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
   friend void dyncompat::checked_delete<EventTerminate>(EventTerminate *) noexcept;
   friend void dyncompat::checked_delete<const EventTerminate>(const EventTerminate *) noexcept;
 public:
   typedef dyncompat::shared_ptr<EventTerminate> ptr;
   typedef dyncompat::shared_ptr<const EventTerminate> const_ptr;
   EventTerminate(EventType type_);
   virtual ~EventTerminate();
};

class PC_EXPORT EventExit : public EventTerminate
{
   friend void dyncompat::checked_delete<EventExit>(EventExit *) noexcept;
   friend void dyncompat::checked_delete<const EventExit>(const EventExit *) noexcept;
 private:
   int exitcode;
 public:
   typedef dyncompat::shared_ptr<EventExit> ptr;
   typedef dyncompat::shared_ptr<const EventExit> const_ptr;
   int getExitCode() const;
   EventExit(EventType::Time eventtime, int exitcode_);
   virtual ~EventExit();
};

class PC_EXPORT EventCrash : public EventTerminate
{
   friend void dyncompat::checked_delete<EventCrash>(EventCrash *) noexcept;
   friend void dyncompat::checked_delete<const EventCrash>(const EventCrash *) noexcept;
 private:
   int termsig;
 public:
   typedef dyncompat::shared_ptr<EventCrash> ptr;
   typedef dyncompat::shared_ptr<const EventCrash> const_ptr;
   int getTermSignal() const;
   EventCrash(int termsig);
   virtual ~EventCrash();
};

class PC_EXPORT EventForceTerminate : public EventTerminate
{
   friend void dyncompat::checked_delete<EventForceTerminate>(EventForceTerminate *) noexcept;
   friend void dyncompat::checked_delete<const EventForceTerminate>(const EventForceTerminate *) noexcept;
 private:
   int termsig;
 public:
   typedef dyncompat::shared_ptr<EventForceTerminate> ptr;
   typedef dyncompat::shared_ptr<const EventForceTerminate> const_ptr;
   int getTermSignal() const;
   EventForceTerminate(int termsig);
   virtual ~EventForceTerminate();
};

class PC_EXPORT EventExec : public Event
{
   friend void dyncompat::checked_delete<EventExec>(EventExec *) noexcept;
   friend void dyncompat::checked_delete<const EventExec>(const EventExec *) noexcept;
 private:
   std::string execpath;
 public:
   typedef dyncompat::shared_ptr<EventExec> ptr;
   typedef dyncompat::shared_ptr<const EventExec> const_ptr;
   EventExec(EventType::Time etime_, std::string path = std::string(""));
   virtual ~EventExec();

   std::string getExecPath() const;
   void setExecPath(std::string path_);
};

class PC_EXPORT EventStop : public Event
{
   friend void dyncompat::checked_delete<EventStop>(EventStop *) noexcept;
   friend void dyncompat::checked_delete<const EventStop>(const EventStop *) noexcept;
 public:
   typedef dyncompat::shared_ptr<EventStop> ptr;
   typedef dyncompat::shared_ptr<const EventStop> const_ptr;
   EventStop();
   virtual ~EventStop();
};

class PC_EXPORT EventNewThread : public Event
{
   friend void dyncompat::checked_delete<EventNewThread>(EventNewThread *) noexcept;
   friend void dyncompat::checked_delete<const EventNewThread>(const EventNewThread *) noexcept;
 public:
   typedef dyncompat::shared_ptr<EventNewThread> ptr;
   typedef dyncompat::shared_ptr<const EventNewThread> const_ptr;
   EventNewThread(EventType et);
   virtual ~EventNewThread();

   virtual Dyninst::LWP getLWP() const = 0;
   virtual Thread::const_ptr getNewThread() const = 0;
};

class int_eventNewUserThread;
class PC_EXPORT EventNewUserThread : public EventNewThread
{
   friend void dyncompat::checked_delete<EventNewUserThread>(EventNewUserThread *) noexcept;
   friend void dyncompat::checked_delete<const EventNewUserThread>(const EventNewUserThread *) noexcept;
  private:
   int_eventNewUserThread *iev;
  public:
   typedef dyncompat::shared_ptr<EventNewUserThread> ptr;
   typedef dyncompat::shared_ptr<const EventNewUserThread> const_ptr;   

   EventNewUserThread();
   virtual ~EventNewUserThread();
   int_eventNewUserThread *getInternalEvent() const;

   virtual Dyninst::LWP getLWP() const;
   virtual Thread::const_ptr getNewThread() const;
};

class int_eventNewLWP;
class PC_EXPORT EventNewLWP : public EventNewThread
{
   friend void dyncompat::checked_delete<EventNewLWP>(EventNewLWP *) noexcept;
   friend void dyncompat::checked_delete<const EventNewLWP>(const EventNewLWP *) noexcept;
  private:
   int_eventNewLWP *iev;
   Dyninst::LWP lwp;
  public:
   int_eventNewLWP *getInternalEvent();
   typedef dyncompat::shared_ptr<EventNewLWP> ptr;
   typedef dyncompat::shared_ptr<const EventNewLWP> const_ptr;   
   EventNewLWP(Dyninst::LWP lwp_, int status = 0);
   virtual ~EventNewLWP();

   virtual Dyninst::LWP getLWP() const;
   virtual Thread::const_ptr getNewThread() const;
};

class PC_EXPORT EventThreadDestroy : public Event
{
   friend void dyncompat::checked_delete<EventThreadDestroy>(EventThreadDestroy *) noexcept;
   friend void dyncompat::checked_delete<const EventThreadDestroy>(const EventThreadDestroy *) noexcept;
 public:
   typedef dyncompat::shared_ptr<EventThreadDestroy> ptr;
   typedef dyncompat::shared_ptr<const EventThreadDestroy> const_ptr;
   EventThreadDestroy(EventType et);
   virtual ~EventThreadDestroy() = 0;
};

class PC_EXPORT EventUserThreadDestroy : public EventThreadDestroy
{
   friend void dyncompat::checked_delete<EventUserThreadDestroy>(EventUserThreadDestroy *) noexcept;
   friend void dyncompat::checked_delete<const EventUserThreadDestroy>(const EventUserThreadDestroy *) noexcept;
 public:
   typedef dyncompat::shared_ptr<EventUserThreadDestroy> ptr;
   typedef dyncompat::shared_ptr<const EventUserThreadDestroy> const_ptr;
   EventUserThreadDestroy(EventType::Time time_);
   virtual ~EventUserThreadDestroy();
};

class PC_EXPORT EventLWPDestroy : public EventThreadDestroy
{
   friend void dyncompat::checked_delete<EventLWPDestroy>(EventLWPDestroy *) noexcept;
   friend void dyncompat::checked_delete<const EventLWPDestroy>(const EventLWPDestroy *) noexcept;
 public:
   typedef dyncompat::shared_ptr<EventLWPDestroy> ptr;
   typedef dyncompat::shared_ptr<const EventLWPDestroy> const_ptr;
   EventLWPDestroy(EventType::Time time_);
   virtual ~EventLWPDestroy();
};

class PC_EXPORT EventFork : public Event
{
   friend void dyncompat::checked_delete<EventFork>(EventFork *) noexcept;
   friend void dyncompat::checked_delete<const EventFork>(const EventFork *) noexcept;
  private:
   Dyninst::PID pid;
  public:
   typedef dyncompat::shared_ptr<EventFork> ptr;
   typedef dyncompat::shared_ptr<const EventFork> const_ptr;
   EventFork(EventType::Time time_, Dyninst::PID pid_);
   virtual ~EventFork();
   Dyninst::PID getPID() const;
   Process::const_ptr getChildProcess() const;
};

class PC_EXPORT EventSignal : public Event
{
public:
   // causes of signal. unknown refers to all non-access violations.
   // this is needed for defensve mode.
   enum Cause { Unknown, ReadViolation, WriteViolation, ExecuteViolation };
   friend void dyncompat::checked_delete<EventSignal>(EventSignal *) noexcept;
   friend void dyncompat::checked_delete<const EventSignal>(const EventSignal *) noexcept;
 private:
   int sig;
   // address that caused the signal (if any), the cause, and
   // whether this is a first-cause exception (windows access violations).
   Address addr;
   Cause cause;
   bool first;
 public:
   typedef dyncompat::shared_ptr<EventSignal> ptr;
   typedef dyncompat::shared_ptr<const EventSignal> const_ptr;
   EventSignal(int sig);
   EventSignal(int s, Address a, Cause c, bool f) : Event(EventType(EventType::None, EventType::Signal)), 
       sig(s), addr(a), cause(c), first(f) { }
   virtual ~EventSignal();

   int getSignal() const;
   void setThreadSignal(int newSignal) const;
   void clearThreadSignal() const;

   // used to get information about windows access violations.
   Address getAddress() const { return addr; }
   Cause getCause() const { return cause; }
   bool isFirst() const { return first; }
};

class PC_EXPORT EventBootstrap : public Event
{
   friend void dyncompat::checked_delete<EventBootstrap>(EventBootstrap *) noexcept;
   friend void dyncompat::checked_delete<const EventBootstrap>(const EventBootstrap *) noexcept;
 public:
   typedef dyncompat::shared_ptr<EventBootstrap> ptr;
   typedef dyncompat::shared_ptr<const EventBootstrap> const_ptr;
   EventBootstrap();
   virtual ~EventBootstrap();
};

class PC_EXPORT EventPreBootstrap : public Event
{
   friend void dyncompat::checked_delete<EventPreBootstrap>(EventPreBootstrap *) noexcept;
   friend void dyncompat::checked_delete<const EventPreBootstrap>(const EventPreBootstrap *) noexcept;
 public:
   typedef dyncompat::shared_ptr<EventPreBootstrap> ptr;
   typedef dyncompat::shared_ptr<const EventPreBootstrap> const_ptr;
   EventPreBootstrap();
   virtual ~EventPreBootstrap();
};


class int_eventRPC;
class PC_EXPORT EventRPC : public Event
{
   friend void dyncompat::checked_delete<EventRPC>(EventRPC *) noexcept;
   friend void dyncompat::checked_delete<const EventRPC>(const EventRPC *) noexcept;
 private:
   int_eventRPC *int_rpc;
   rpc_wrapper *wrapper;
 public:
   virtual bool suppressCB() const;
   rpc_wrapper *getllRPC();
   typedef dyncompat::shared_ptr<EventRPC> ptr;
   typedef dyncompat::shared_ptr<const EventRPC> const_ptr;
   EventRPC(rpc_wrapper *wrapper_);
   virtual ~EventRPC();

   IRPC::const_ptr getIRPC() const;
   int_eventRPC *getInternal() const;
};

class PC_EXPORT EventRPCLaunch : public Event
{
   friend void dyncompat::checked_delete<EventRPCLaunch>(EventRPCLaunch *) noexcept;
   friend void dyncompat::checked_delete<const EventRPCLaunch>(const EventRPCLaunch *) noexcept;
 public:
   typedef dyncompat::shared_ptr<EventRPCLaunch> ptr;
   typedef dyncompat::shared_ptr<const EventRPCLaunch> const_ptr;
   virtual bool procStopper() const;
   EventRPCLaunch();
   virtual ~EventRPCLaunch();
};

class PC_EXPORT EventSingleStep : public Event
{
   friend void dyncompat::checked_delete<EventSingleStep>(EventSingleStep *) noexcept;
   friend void dyncompat::checked_delete<const EventSingleStep>(const EventSingleStep *) noexcept;
 public:
   typedef dyncompat::shared_ptr<EventSingleStep> ptr;
   typedef dyncompat::shared_ptr<const EventSingleStep> const_ptr;
   EventSingleStep();
   virtual ~EventSingleStep();
};

class PC_EXPORT EventSyscall : public Event
{
   friend void dyncompat::checked_delete<EventSyscall>(EventSyscall *) noexcept;
   friend void dyncompat::checked_delete<const EventSyscall>(const EventSyscall *) noexcept;

   friend MachSyscall makeFromEvent(const EventSyscall *);

 public:
   typedef dyncompat::shared_ptr<EventSyscall> ptr;
   typedef dyncompat::shared_ptr<const EventSyscall> const_ptr;
   EventSyscall(EventType type_);
   virtual ~EventSyscall();

   Dyninst::Address getAddress() const;
   MachSyscall getSyscall() const;
   
 private:
    long getSyscallNumber() const;
};

class PC_EXPORT EventPreSyscall : public EventSyscall
{
   friend void dyncompat::checked_delete<EventPreSyscall>(EventPreSyscall *) noexcept;
   friend void dyncompat::checked_delete<const EventPreSyscall>(const EventPreSyscall *) noexcept;

   friend MachSyscall makeFromEvent(const EventPreSyscall *);

 public:
   typedef dyncompat::shared_ptr<EventPreSyscall> ptr;
   typedef dyncompat::shared_ptr<const EventPreSyscall> const_ptr;
   EventPreSyscall();
   virtual ~EventPreSyscall();
};

class PC_EXPORT EventPostSyscall : public EventSyscall
{
   friend void dyncompat::checked_delete<EventPostSyscall>(EventPostSyscall *) noexcept;
   friend void dyncompat::checked_delete<const EventPostSyscall>(const EventPostSyscall *) noexcept;

   friend MachSyscall makeFromEvent(const EventPostSyscall *);

 public:
   typedef dyncompat::shared_ptr<EventPostSyscall> ptr;
   typedef dyncompat::shared_ptr<const EventPostSyscall> const_ptr;
   EventPostSyscall();
   virtual ~EventPostSyscall();

   long getReturnValue() const;
};

class int_eventBreakpoint;
class PC_EXPORT EventBreakpoint : public Event
{
   friend void dyncompat::checked_delete<EventBreakpoint>(EventBreakpoint *) noexcept;
   friend void dyncompat::checked_delete<const EventBreakpoint>(const EventBreakpoint *) noexcept;
 private:
   int_eventBreakpoint *int_bp;
 public:
   typedef dyncompat::shared_ptr<EventBreakpoint> ptr;
   typedef dyncompat::shared_ptr<const EventBreakpoint> const_ptr;
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
   friend void dyncompat::checked_delete<EventBreakpointClear>(EventBreakpointClear *) noexcept;
   friend void dyncompat::checked_delete<const EventBreakpointClear>(const EventBreakpointClear *) noexcept;
  private:
   int_eventBreakpointClear *int_bpc;
  public:
   typedef dyncompat::shared_ptr<EventBreakpointClear> ptr;
   typedef dyncompat::shared_ptr<const EventBreakpointClear> const_ptr;
   EventBreakpointClear();
   virtual ~EventBreakpointClear();
   
   int_eventBreakpointClear *getInternal() const;
   virtual bool procStopper() const;
};

class int_eventBreakpointRestore;
class EventBreakpointRestore : public Event
{
   friend void dyncompat::checked_delete<EventBreakpointRestore>(EventBreakpointRestore *) noexcept;
   friend void dyncompat::checked_delete<const EventBreakpointRestore>(const EventBreakpointRestore *) noexcept;
  private:
   int_eventBreakpointRestore *int_bpr;
  public:
   typedef dyncompat::shared_ptr<EventBreakpointRestore> ptr;
   typedef dyncompat::shared_ptr<const EventBreakpointRestore> const_ptr;

   EventBreakpointRestore(int_eventBreakpointRestore *iebpr);
   virtual ~EventBreakpointRestore();

   int_eventBreakpointRestore *getInternal() const;
};

class PC_EXPORT EventLibrary : public Event
{
   friend void dyncompat::checked_delete<EventLibrary>(EventLibrary *) noexcept;
   friend void dyncompat::checked_delete<const EventLibrary>(const EventLibrary *) noexcept;
 private:
   std::set<Library::ptr> added_libs;
   std::set<Library::ptr> rmd_libs;
 public:
   typedef dyncompat::shared_ptr<EventLibrary> ptr;
   typedef dyncompat::shared_ptr<const EventLibrary> const_ptr;
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
   friend void dyncompat::checked_delete<EventAsync>(EventAsync *) noexcept;
   friend void dyncompat::checked_delete<const EventAsync>(const EventAsync *) noexcept;
   
  private:
   int_eventAsync *internal;
  public:
   typedef dyncompat::shared_ptr<EventAsync> ptr;
   typedef dyncompat::shared_ptr<const EventAsync> const_ptr;

   EventAsync(int_eventAsync *ievent);
   virtual ~EventAsync();
   int_eventAsync *getInternal() const;
};

class PC_EXPORT EventChangePCStop : public Event
{
   friend void dyncompat::checked_delete<EventChangePCStop>(EventChangePCStop *) noexcept;
   friend void dyncompat::checked_delete<const EventChangePCStop>(const EventChangePCStop *) noexcept;
 public:
   typedef dyncompat::shared_ptr<EventChangePCStop> ptr;
   typedef dyncompat::shared_ptr<const EventChangePCStop> const_ptr;
   EventChangePCStop();
   virtual ~EventChangePCStop();
};

class int_eventDetach;
class PC_EXPORT EventDetach : public Event
{
   friend void dyncompat::checked_delete<EventDetach>(EventDetach *) noexcept;
   friend void dyncompat::checked_delete<const EventDetach>(const EventDetach *) noexcept;
   int_eventDetach *int_detach;
 public:
   typedef dyncompat::shared_ptr<EventDetach> ptr;
   typedef dyncompat::shared_ptr<const EventDetach> const_ptr;

   EventDetach();
   virtual ~EventDetach();
   int_eventDetach *getInternal() const;
   virtual bool procStopper() const;
};

class PC_EXPORT EventIntBootstrap : public Event
{
   friend void dyncompat::checked_delete<EventIntBootstrap>(EventIntBootstrap *) noexcept;
   friend void dyncompat::checked_delete<const EventIntBootstrap>(const EventIntBootstrap *) noexcept;
   
   void *data;
 public:
   typedef dyncompat::shared_ptr<EventIntBootstrap> ptr;
   typedef dyncompat::shared_ptr<const EventIntBootstrap> const_ptr;
   EventIntBootstrap(void *d = NULL);
   virtual ~EventIntBootstrap();

   void *getData() const;
   void setData(void *v);
};

class PC_EXPORT EventNop : public Event
{
   friend void dyncompat::checked_delete<EventNop>(EventNop *) noexcept;
   friend void dyncompat::checked_delete<const EventNop>(const EventNop *) noexcept;
 public:
   typedef dyncompat::shared_ptr<EventNop> ptr;
   typedef dyncompat::shared_ptr<const EventNop> const_ptr;
   EventNop();
   virtual ~EventNop();
};

class int_eventThreadDB;
class PC_EXPORT EventThreadDB : public Event
{
   friend void dyncompat::checked_delete<EventThreadDB>(EventThreadDB *) noexcept;
   friend void dyncompat::checked_delete<const EventThreadDB>(const EventThreadDB *) noexcept;
   int_eventThreadDB *int_etdb;
  public:
   typedef dyncompat::shared_ptr<EventThreadDB> ptr;
   typedef dyncompat::shared_ptr<const EventThreadDB> const_ptr;
   int_eventThreadDB *getInternal() const;

   EventThreadDB();
   virtual ~EventThreadDB();

   virtual bool triggersCB() const;
};

class PC_EXPORT EventWinStopThreadDestroy : public EventThreadDestroy
{
   friend void dyncompat::checked_delete<EventWinStopThreadDestroy>(EventWinStopThreadDestroy *) noexcept;
   friend void dyncompat::checked_delete<const EventWinStopThreadDestroy>(const EventWinStopThreadDestroy *) noexcept;
 public:
   typedef dyncompat::shared_ptr<EventWinStopThreadDestroy> ptr;
   typedef dyncompat::shared_ptr<const EventWinStopThreadDestroy> const_ptr;
   EventWinStopThreadDestroy(EventType::Time time_);
   virtual ~EventWinStopThreadDestroy();
};

class int_eventControlAuthority;
class PC_EXPORT EventControlAuthority : public Event
{
   friend void dyncompat::checked_delete<EventControlAuthority>(EventControlAuthority *) noexcept;
   friend void dyncompat::checked_delete<const EventControlAuthority>(const EventControlAuthority *) noexcept;
   int_eventControlAuthority *iev;
  public:
   typedef dyncompat::shared_ptr<EventControlAuthority> ptr;
   typedef dyncompat::shared_ptr<const EventControlAuthority> const_ptr;
   int_eventControlAuthority *getInternalEvent() const;

   EventControlAuthority(EventType::Time t, int_eventControlAuthority *iev_);
   virtual ~EventControlAuthority();
   virtual bool procStopper() const;

   std::string otherToolName() const;
   unsigned int otherToolID() const;
   int otherToolPriority() const;
   
   typedef enum {
      ControlUnset,
      ControlLost,
      ControlGained,
      ControlNoChange
   } Trigger;
   Trigger eventTrigger() const;
};

class int_eventAsyncIO;
class PC_EXPORT EventAsyncIO : public Event {
   friend void dyncompat::checked_delete<EventAsyncIO>(EventAsyncIO *) noexcept;
   friend void dyncompat::checked_delete<const EventAsyncIO>(const EventAsyncIO *) noexcept;
  protected:
   int_eventAsyncIO *iev;
  public:
   typedef dyncompat::shared_ptr<EventAsyncIO> ptr;
   typedef dyncompat::shared_ptr<const EventAsyncIO> const_ptr;
   int_eventAsyncIO *getInternalEvent() const;

   EventAsyncIO(EventType et, int_eventAsyncIO *iev_);
   ~EventAsyncIO();

   bool hadError() const;
   void *getOpaqueVal() const;
};

class PC_EXPORT EventAsyncRead : public EventAsyncIO {
   friend void dyncompat::checked_delete<EventAsyncRead>(EventAsyncRead *) noexcept;
   friend void dyncompat::checked_delete<const EventAsyncRead>(const EventAsyncRead *) noexcept;
  public:
   typedef dyncompat::shared_ptr<EventAsyncRead> ptr;
   typedef dyncompat::shared_ptr<const EventAsyncRead> const_ptr;
   
   EventAsyncRead(int_eventAsyncIO *iev_);
   ~EventAsyncRead();

   void *getMemory() const;
   size_t getSize() const;
   Dyninst::Address getAddress() const;
};

class PC_EXPORT EventAsyncWrite : public EventAsyncIO {
   friend void dyncompat::checked_delete<EventAsyncWrite>(EventAsyncWrite *) noexcept;
   friend void dyncompat::checked_delete<const EventAsyncWrite>(const EventAsyncWrite *) noexcept;
  public:
   typedef dyncompat::shared_ptr<EventAsyncWrite> ptr;
   typedef dyncompat::shared_ptr<const EventAsyncWrite> const_ptr;
   
   EventAsyncWrite(int_eventAsyncIO *iev_);
   ~EventAsyncWrite();

   size_t getSize() const;
   Dyninst::Address getAddress() const;
};

class PC_EXPORT EventAsyncReadAllRegs : public EventAsyncIO {
   friend void dyncompat::checked_delete<EventAsyncReadAllRegs>(EventAsyncReadAllRegs *) noexcept;
   friend void dyncompat::checked_delete<const EventAsyncReadAllRegs>(const EventAsyncReadAllRegs *) noexcept;
  public:
   typedef dyncompat::shared_ptr<EventAsyncReadAllRegs> ptr;
   typedef dyncompat::shared_ptr<const EventAsyncReadAllRegs> const_ptr;
   
   EventAsyncReadAllRegs(int_eventAsyncIO *iev_);
   ~EventAsyncReadAllRegs();

   const RegisterPool &getRegisters() const;
};

class PC_EXPORT EventAsyncSetAllRegs : public EventAsyncIO {
   friend void dyncompat::checked_delete<EventAsyncSetAllRegs>(EventAsyncSetAllRegs *) noexcept;
   friend void dyncompat::checked_delete<const EventAsyncSetAllRegs>(const EventAsyncSetAllRegs *) noexcept;
  public:
   typedef dyncompat::shared_ptr<EventAsyncSetAllRegs> ptr;
   typedef dyncompat::shared_ptr<const EventAsyncSetAllRegs> const_ptr;
   
   EventAsyncSetAllRegs(int_eventAsyncIO *iev_);
   ~EventAsyncSetAllRegs();
};

class int_eventAsyncFileRead;
class PC_EXPORT EventAsyncFileRead : public Event {
   friend void dyncompat::checked_delete<EventAsyncFileRead>(EventAsyncFileRead *) noexcept;
   friend void dyncompat::checked_delete<const EventAsyncFileRead>(const EventAsyncFileRead *) noexcept;
   int_eventAsyncFileRead *iev;
  public:
   typedef dyncompat::shared_ptr<EventAsyncFileRead> ptr;
   typedef dyncompat::shared_ptr<const EventAsyncFileRead> const_ptr;
   int_eventAsyncFileRead *getInternal();
   
   EventAsyncFileRead(int_eventAsyncFileRead *iev_);
   ~EventAsyncFileRead();
   
   std::string getFilename() const;
   size_t getReadSize() const;
   Dyninst::Offset getReadOffset() const;

   void *getBuffer() const;
   size_t getBufferSize() const;

   bool isEOF() const;
   int errorCode() const;
};

class EventPostponedSyscall : public Event
{
   friend void dyncompat::checked_delete<EventPostponedSyscall>(EventPostponedSyscall *) noexcept;
   friend void dyncompat::checked_delete<const EventPostponedSyscall>(const EventPostponedSyscall *) noexcept;
  public:
   typedef dyncompat::shared_ptr<EventPostponedSyscall> ptr;
   typedef dyncompat::shared_ptr<const EventPostponedSyscall> const_ptr;

   EventPostponedSyscall();
   virtual ~EventPostponedSyscall();
};

}
}

#endif
