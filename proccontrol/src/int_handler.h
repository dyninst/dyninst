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

#include "Handler.h"
#include "PCProcess.h"
#include <set>
#include <map>
#include <vector>

#if !defined(INT_HANDLER_H_)
#define INT_HANDLER_H_

using namespace Dyninst;
using namespace ProcControlAPI;

struct handler_cmp
{
   bool operator()(const Handler* a, const Handler* b) const
   {
      return a->getPriority() < b->getPriority();
   }
};

class HandlerPool
{
 public:
   typedef std::set<Handler *, handler_cmp> HandlerSet_t;
   typedef std::map<EventType, HandlerSet_t*, eventtype_cmp> HandlerMap_t;

   HandlerPool(int_process *owner_proc);
   ~HandlerPool();

   void addHandler(Handler *handler);
   bool handleEvent(Event::ptr ev);
   Event::ptr handleAsyncEvent(Event::ptr ev);

   void notifyOfPendingAsyncs(const std::set<response::ptr> &asyncs, Event::ptr ev);
   void notifyOfPendingAsyncs(response::ptr async, Event::ptr ev);

   bool isEventAsyncPostponed(Event::ptr ev) const;
   bool hasAsyncEvent() const;

   static bool hasProcAsyncPending();
   void markEventAsyncPending(Event::ptr ev);
   
   void addLateEvent(Event::ptr ev);
   Event::ptr curEvent();

   void setNopAsCurEvent();
   void clearNopAsCurEvent();
 private:
   HandlerMap_t handlers;
   void addHandlerInt(EventType etype, Handler *handler);
   void clearEventAsync(Event::ptr ev);
   void addEventToSet(Event::ptr ev, std::set<Event::ptr> &ev_set) const;
   void collectLateEvents(Event::ptr parent_ev);
   bool hasLateEvents() const;

   bool insertAsyncPendingEvent(Event::ptr ev);
   bool removeAsyncPendingEvent(Event::ptr ev);

   Event::ptr getRealParent(Event::ptr ev) const;

   std::set<Event::ptr> pending_async_events;
   std::set<Event::ptr> late_events;
   int_process *proc;
   Event::ptr cur_event;
   bool nop_cur_event;

   static void markProcAsyncPending(HandlerPool *p);
   static void clearProcAsyncPending(HandlerPool *p);
   static std::set<HandlerPool *> procsAsyncPending;
   static Mutex<false> asyncPendingLock;
};

class HandlePreBootstrap : public Handler
{
 public:
   HandlePreBootstrap();
   virtual ~HandlePreBootstrap();

   virtual void getEventTypesHandled(std::vector<EventType> &etypes);
   virtual handler_ret_t handleEvent(Event::ptr ev);
};


class HandleBootstrap : public Handler
{
 public:
   HandleBootstrap();
   virtual ~HandleBootstrap();

   virtual void getEventTypesHandled(std::vector<EventType> &etypes);
   virtual handler_ret_t handleEvent(Event::ptr ev);
};

class HandleCrash : public Handler
{
 public:
  HandleCrash();
  virtual ~HandleCrash();

  virtual void getEventTypesHandled(std::vector<EventType> &etypes);
  virtual handler_ret_t handleEvent(Event::ptr ev);  
};

class HandleForceTerminate : public Handler
{
 public:
  HandleForceTerminate();
  virtual ~HandleForceTerminate();

  virtual void getEventTypesHandled(std::vector<EventType> &etypes);
  virtual handler_ret_t handleEvent(Event::ptr ev);  
  virtual int getPriority() const;
};

class HandleSignal : public Handler
{
 public:
   HandleSignal();
   virtual ~HandleSignal();
   
   virtual void getEventTypesHandled(std::vector<EventType> &etypes);
   virtual handler_ret_t handleEvent(Event::ptr ev);
};

class HandlePostExit : public Handler
{
 public:
   HandlePostExit();
   virtual ~HandlePostExit();

   virtual void getEventTypesHandled(std::vector<EventType> &etypes);
   virtual handler_ret_t handleEvent(Event::ptr ev);
};

class HandlePostExitCleanup : public Handler
{
  public:
   HandlePostExitCleanup();
   virtual ~HandlePostExitCleanup();

   virtual void getEventTypesHandled(std::vector<EventType> &etypes);
   virtual handler_ret_t handleEvent(Event::ptr ev);
   virtual int getPriority() const;
};

class HandlePreExit : public Handler
{
 public:
   HandlePreExit();
   virtual ~HandlePreExit();

   virtual void getEventTypesHandled(std::vector<EventType> &etypes);
   virtual handler_ret_t handleEvent(Event::ptr ev);
};

class HandleThreadCreate : public Handler
{
 public:
   HandleThreadCreate();
   virtual ~HandleThreadCreate();
   
   virtual void getEventTypesHandled(std::vector<EventType> &etypes);
   virtual handler_ret_t handleEvent(Event::ptr ev);   
};

class HandleThreadDestroy : public Handler
{
 public:
   HandleThreadDestroy();
   virtual ~HandleThreadDestroy();
      
   virtual void getEventTypesHandled(std::vector<EventType> &etypes);
   virtual handler_ret_t handleEvent(Event::ptr ev);
   virtual int getPriority() const;
};

class HandleThreadCleanup : public Handler
{
  public:
   HandleThreadCleanup();
   virtual ~HandleThreadCleanup();

   virtual void getEventTypesHandled(std::vector<EventType> &etypes);
   virtual handler_ret_t handleEvent(Event::ptr ev);
   virtual int getPriority() const;
};

class HandleThreadStop : public Handler
{
 public:
  HandleThreadStop();
  virtual ~HandleThreadStop();

  virtual int getPriority() const;
  virtual void getEventTypesHandled(std::vector<EventType> &etypes);
  virtual handler_ret_t handleEvent(Event::ptr ev);
};

class HandlePostFork : public Handler
{
  public:
   HandlePostFork();
   virtual ~HandlePostFork();

  virtual void getEventTypesHandled(std::vector<EventType> &etypes);
  virtual handler_ret_t handleEvent(Event::ptr ev);
};

class HandlePostForkCont : public Handler
{
  public:
   HandlePostForkCont();
   virtual ~HandlePostForkCont();

  virtual void getEventTypesHandled(std::vector<EventType> &etypes);
  virtual handler_ret_t handleEvent(Event::ptr ev);
  virtual int getPriority() const;
};

class HandlePostExec : public Handler
{
  public:
   HandlePostExec();
   virtual ~HandlePostExec();

   virtual void getEventTypesHandled(std::vector<EventType> &etypes);
   virtual handler_ret_t handleEvent(Event::ptr ev);
};

class HandleSingleStep : public Handler
{
 public:
  HandleSingleStep();
  virtual ~HandleSingleStep();

  virtual void getEventTypesHandled(std::vector<EventType> &etypes);
  virtual handler_ret_t handleEvent(Event::ptr ev);
};

class HandlePreSyscall : public Handler
{
 public:
  HandlePreSyscall();
  virtual ~HandlePreSyscall();

  virtual void getEventTypesHandled(std::vector<EventType> &etypes);
  virtual handler_ret_t handleEvent(Event::ptr ev);
};

class HandlePostSyscall : public Handler
{
 public:
  HandlePostSyscall();
  virtual ~HandlePostSyscall();

  virtual void getEventTypesHandled(std::vector<EventType> &etypes);
  virtual handler_ret_t handleEvent(Event::ptr ev);
};

class HandleBreakpoint : public Handler
{
 public:
  HandleBreakpoint();
  virtual ~HandleBreakpoint();

  virtual void getEventTypesHandled(std::vector<EventType> &etypes);
  virtual handler_ret_t handleEvent(Event::ptr ev);
};

class HandleBreakpointContinue : public Handler
{
  public:
   HandleBreakpointContinue();
   virtual ~HandleBreakpointContinue();
   
   virtual void getEventTypesHandled(std::vector<EventType> &etypes);
   virtual handler_ret_t handleEvent(Event::ptr ev);
   virtual int getPriority() const;
};

class HandleBreakpointClear : public Handler
{
 public:
  HandleBreakpointClear();
  virtual ~HandleBreakpointClear();

  virtual void getEventTypesHandled(std::vector<EventType> &etypes);
  virtual handler_ret_t handleEvent(Event::ptr ev);
};

class HandleBreakpointRestore : public Handler
{
 public:
  HandleBreakpointRestore();
  virtual ~HandleBreakpointRestore();

  virtual void getEventTypesHandled(std::vector<EventType> &etypes);
  virtual handler_ret_t handleEvent(Event::ptr ev);
};

class HandleEmulatedSingleStep : public Handler
{
  public:
   HandleEmulatedSingleStep();
   ~HandleEmulatedSingleStep();

   virtual void getEventTypesHandled(std::vector<EventType> &etypes);
   virtual handler_ret_t handleEvent(Event::ptr ev);
   virtual int getPriority() const;
};

class HandleLibrary : public Handler
{
 public:
   HandleLibrary();
   virtual ~HandleLibrary();

   virtual handler_ret_t handleEvent(Event::ptr ev);
   virtual void getEventTypesHandled(std::vector<EventType> &etypes);
};

class HandleDetach : public Handler
{ 
  public:
   HandleDetach();
   virtual ~HandleDetach();
   
   virtual handler_ret_t handleEvent(Event::ptr ev);
   virtual void getEventTypesHandled(std::vector<EventType> &etypes);
};

class HandleAsync : public Handler
{
  public:
   HandleAsync();
   virtual ~HandleAsync();
   
   virtual handler_ret_t handleEvent(Event::ptr ev);
   virtual void getEventTypesHandled(std::vector<EventType> &etypes);
};

class HandleAsyncIO : public Handler
{
  public:
   HandleAsyncIO();
   virtual ~HandleAsyncIO();
   
   virtual handler_ret_t handleEvent(Event::ptr ev);
   virtual void getEventTypesHandled(std::vector<EventType> &etypes);
};

class HandleAsyncFileRead : public Handler
{
  public:
   HandleAsyncFileRead();
   virtual ~HandleAsyncFileRead();

   virtual handler_ret_t handleEvent(Event::ptr ev);
   virtual void getEventTypesHandled(std::vector<EventType> &etypes);
};

class HandleNop : public Handler
{
  public:
   HandleNop();
   virtual ~HandleNop();
   
   virtual handler_ret_t handleEvent(Event::ptr ev);
   virtual void getEventTypesHandled(std::vector<EventType> &etypes);
};

class HandleCallbacks : public Handler
{
  friend class HandlerPool;
 private:
  typedef std::map<EventType, std::set<Process::cb_func_t>, eventtype_cmp> cbfuncs_t;
  cbfuncs_t cbfuncs;
  std::set<EventType, eventtype_cmp> alleventtypes;
  bool registerCallback_int(EventType ev, Process::cb_func_t func);
  bool removeCallback_int(EventType et);
  bool removeCallback_int(EventType et, Process::cb_func_t func);
  bool handleCBReturn(Process::const_ptr proc, Thread::const_ptr thrd, 
                      Process::cb_action_t ret);
 public:
  HandleCallbacks();
  virtual ~HandleCallbacks();

  static HandleCallbacks *getCB();
  virtual int getPriority() const;
  virtual void getEventTypesHandled(std::vector<EventType> &etypes);
  virtual handler_ret_t handleEvent(Event::ptr ev);
  bool hasCBs(Event::const_ptr ev);
  bool hasCBs(EventType et);
  
  bool registerCallback(EventType ev, Process::cb_func_t func);
  bool removeCallback(EventType et, Process::cb_func_t func);
  bool removeCallback(EventType et);
  bool removeCallback(Process::cb_func_t func);

  Handler::handler_ret_t deliverCallback(Event::ptr ev, const std::set<Process::cb_func_t> &cbset);
  
  bool requiresCB(Event::const_ptr ev);

  static void getRealEvents(EventType ev, std::vector<EventType> &out_evs);
};

class HandlePostponedSyscall : public Handler
{
 public:
  HandlePostponedSyscall();
  virtual ~HandlePostponedSyscall();

  virtual void getEventTypesHandled(std::vector<EventType> &etypes);
  virtual handler_ret_t handleEvent(Event::ptr ev);
};


#endif
