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
#if !defined(WINDOWS_H_)
#define WINDOWS_H_


#include "GeneratorWindows.h"
#include "Event.h"
#include "Decoder.h"
#include "Handler.h"
#include "int_process.h"
#include "x86_process.h"
#include "common/src/dthread.h"
#include <sys/types.h>
#include <vector>
#include <deque>
#include "int_handler.h"
#include "int_event.h"


using namespace Dyninst;
using namespace ProcControlAPI;


class ArchEventWindows : public ArchEvent
{
   static std::vector<ArchEventWindows *> pending_events;
public:
   bool findPairedEvent(ArchEventWindows* &parent, ArchEventWindows* &child);
   void postponePairedEvent();

   ArchEventWindows(DEBUG_EVENT e);

   virtual ~ArchEventWindows();
   DEBUG_EVENT evt;
};

class PC_EXPORT WinEventNewThread : public EventNewLWP
{
   friend void boost::checked_delete<WinEventNewThread>(WinEventNewThread *) CHECKED_DELETE_NOEXCEPT;
   friend void boost::checked_delete<const WinEventNewThread>(const WinEventNewThread *) CHECKED_DELETE_NOEXCEPT;
 public:
   typedef boost::shared_ptr<WinEventNewThread> ptr;
   typedef boost::shared_ptr<const WinEventNewThread> const_ptr;
   WinEventNewThread(Dyninst::LWP l, HANDLE ht, LPTHREAD_START_ROUTINE ts, LPVOID base) :
      EventNewLWP(l, (int) int_thread::as_created_attached),
      hthread(ht), thread_start(ts), tls_base(base)
	{}
	virtual ~WinEventNewThread() {}

	HANDLE getHandle() const { return hthread; }
	LPTHREAD_START_ROUTINE getThreadStart() const { return thread_start; }
	LPVOID getTLSBase() const { return tls_base; }
private:
	HANDLE hthread;
	LPTHREAD_START_ROUTINE thread_start;
	LPVOID tls_base;
};

class PC_EXPORT WinEventThreadInfo : public Event
{
   friend void boost::checked_delete<WinEventThreadInfo>(WinEventThreadInfo *) CHECKED_DELETE_NOEXCEPT;
   friend void boost::checked_delete<const WinEventThreadInfo>(const WinEventThreadInfo *) CHECKED_DELETE_NOEXCEPT;
 public:
   typedef boost::shared_ptr<WinEventThreadInfo> ptr;
   typedef boost::shared_ptr<const WinEventThreadInfo> const_ptr;
	WinEventThreadInfo(Dyninst::LWP l, HANDLE ht, LPTHREAD_START_ROUTINE ts,
		LPVOID base) : Event(EventType(EventType::None, EventType::ThreadInfo)), hthread(ht), thread_start(ts), tls_base(base),
		lwp(l)
	{}
	virtual ~WinEventThreadInfo() {}

	HANDLE getHandle() const { return hthread; }
	LPTHREAD_START_ROUTINE getThreadStart() const { return thread_start; }
	LPVOID getTLSBase() const { return tls_base; }
	Dyninst::LWP getLWP() const { return lwp; }
private:
	HANDLE hthread;
	LPTHREAD_START_ROUTINE thread_start;
	LPVOID tls_base;
	Dyninst::LWP lwp;
};

class WindowsHandleSetThreadInfo : public Handler
{
 public:
   WindowsHandleSetThreadInfo();
   virtual ~WindowsHandleSetThreadInfo();
   virtual handler_ret_t handleEvent(Event::ptr ev);
   virtual int getPriority() const;
   void getEventTypesHandled(std::vector<EventType> &etypes);
};



class WindowsHandleNewThr : public Handler
{
 public:
   WindowsHandleNewThr();
   virtual ~WindowsHandleNewThr();
   virtual handler_ret_t handleEvent(Event::ptr ev);
   virtual int getPriority() const;
   void getEventTypesHandled(std::vector<EventType> &etypes);
};

class WindowsHandleLWPDestroy : public Handler
{
 public:
     WindowsHandleLWPDestroy();
     virtual ~WindowsHandleLWPDestroy();
     virtual handler_ret_t handleEvent(Event::ptr ev);
     virtual int getPriority() const;
     void getEventTypesHandled(std::vector<EventType> &etypes);
};

class HandleThreadDestroy;

class WindowsHandleProcessExit : public Handler
{
 public:
     WindowsHandleProcessExit();
     virtual ~WindowsHandleProcessExit();
     virtual handler_ret_t handleEvent(Event::ptr ev);
     virtual int getPriority() const;
     void getEventTypesHandled(std::vector<EventType> &etypes);
private:
	HandleThreadDestroy* do_work;
};


class WinHandleSingleStep : public Handler
{
 public:
     WinHandleSingleStep();
     virtual ~WinHandleSingleStep();
     virtual handler_ret_t handleEvent(Event::ptr ev);
     virtual int getPriority() const;
     void getEventTypesHandled(std::vector<EventType> &etypes);
};

class WinHandleBootstrap : public Handler
{
public:
	WinHandleBootstrap();
	virtual ~WinHandleBootstrap();
	virtual handler_ret_t handleEvent(Event::ptr ev);
	virtual int getPriority() const;
	void getEventTypesHandled(std::vector<EventType> &etypes);
};

class WinHandleContinue : public Handler
{
public:
	WinHandleContinue();
	virtual ~WinHandleContinue();
	virtual handler_ret_t handleEvent(Event::ptr ev);
	virtual int getPriority() const;
	void getEventTypesHandled(std::vector<EventType> &etypes);
};

// Windows stop behavior handlers

class HandleThreadStop;

class WindowsHandleThreadStop : public HandleThreadStop
{
 public:
  WindowsHandleThreadStop();
  virtual ~WindowsHandleThreadStop();

  virtual int getPriority() const;
  virtual void getEventTypesHandled(std::vector<EventType> &etypes);
};


#endif // !defined WINDOWS_H_
