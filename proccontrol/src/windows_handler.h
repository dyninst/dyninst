#if !defined(WINDOWS_H_)
#define WINDOWS_H_


#include "GeneratorWindows.h"
#include "proccontrol/h/Event.h"
#include "proccontrol/h/Decoder.h"
#include "proccontrol/h/Handler.h"
#include "proccontrol/src/int_process.h"
#include "proccontrol/src/x86_process.h"
#include "common/h/dthread.h"
#include <sys/types.h>
#include <vector>
#include <deque>

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
   friend void dyn_detail::boost::checked_delete<WinEventNewThread>(WinEventNewThread *);
   friend void dyn_detail::boost::checked_delete<const WinEventNewThread>(const WinEventNewThread *);
 public:
   typedef dyn_detail::boost::shared_ptr<WinEventNewThread> ptr;
   typedef dyn_detail::boost::shared_ptr<const WinEventNewThread> const_ptr;
	WinEventNewThread(Dyninst::LWP l, HANDLE ht, LPTHREAD_START_ROUTINE ts,
		LPVOID base) : EventNewLWP(l), hthread(ht), thread_start(ts), tls_base(base)
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

#endif // !defined WINDOWS_H_
