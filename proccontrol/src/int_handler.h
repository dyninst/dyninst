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

#include "proccontrol/h/Handler.h"
#include "proccontrol/h/Process.h"
#include <map>

#if !defined(INT_HANDLER_H_)
#define INT_HANDLER_H_

using namespace std;
using namespace Dyninst;
using namespace ProcControlAPI;

struct handler_cmp
{
   bool operator()(const Handler* a, const Handler* b)
   {
      return a->getPriority() < b->getPriority();
   }
};

class HandlerPool
{
 public:
   typedef set<Handler *, handler_cmp> HandlerSet_t;
   typedef map<EventType, HandlerSet_t*, eventtype_cmp> HandlerMap_t;

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
   
   Event::ptr curEvent() const;
 private:
   HandlerMap_t handlers;
   void addHandlerInt(EventType etype, Handler *handler);
   void clearEventAsync(Event::ptr ev);
   void addEventToSet(Event::ptr ev, set<Event::ptr> &ev_set) const;
   Event::ptr getRealParent(Event::ptr ev) const;

   std::set<Event::ptr> pending_async_events;
   int_process *proc;
   Event::ptr cur_event;

   static void markProcAsyncPending(HandlerPool *p);
   static void clearProcAsyncPending(HandlerPool *p);
   static std::set<HandlerPool *> procsAsyncPending;
   static Mutex asyncPendingLock;
};

class HandleBootstrap : public Handler
{
 public:
   HandleBootstrap();
   virtual ~HandleBootstrap();

   virtual void getEventTypesHandled(vector<EventType> &etypes);
   virtual handler_ret_t handleEvent(Event::ptr ev);
};

class HandleCrash : public Handler
{
 public:
  HandleCrash();
  ~HandleCrash();

   virtual void getEventTypesHandled(vector<EventType> &etypes);
   virtual handler_ret_t handleEvent(Event::ptr ev);  
};

class HandleForceTerminate : public Handler
{
 public:
  HandleForceTerminate();
  ~HandleForceTerminate();

   virtual void getEventTypesHandled(vector<EventType> &etypes);
   virtual handler_ret_t handleEvent(Event::ptr ev);  
};

class HandleSignal : public Handler
{
 public:
   HandleSignal();
   ~HandleSignal();
   
   virtual void getEventTypesHandled(vector<EventType> &etypes);
   virtual handler_ret_t handleEvent(Event::ptr ev);
};

class HandlePostExit : public Handler
{
 public:
   HandlePostExit();
   ~HandlePostExit();

   virtual void getEventTypesHandled(vector<EventType> &etypes);
   virtual handler_ret_t handleEvent(Event::ptr ev);
};

class HandlePreExit : public Handler
{
 public:
   HandlePreExit();
   ~HandlePreExit();

   virtual void getEventTypesHandled(vector<EventType> &etypes);
   virtual handler_ret_t handleEvent(Event::ptr ev);
};

class HandleThreadCreate : public Handler
{
 public:
   HandleThreadCreate();
   ~HandleThreadCreate();
   
   virtual void getEventTypesHandled(vector<EventType> &etypes);
   virtual handler_ret_t handleEvent(Event::ptr ev);   
};

class HandleThreadDestroy : public Handler
{
 public:
   HandleThreadDestroy();
   ~HandleThreadDestroy();
      
   virtual void getEventTypesHandled(vector<EventType> &etypes);
   virtual handler_ret_t handleEvent(Event::ptr ev);   
};

class HandleThreadStop : public Handler
{
 public:
  HandleThreadStop();
  ~HandleThreadStop();

  virtual void getEventTypesHandled(vector<EventType> &etypes);
  virtual handler_ret_t handleEvent(Event::ptr ev);
};

class HandlePostFork : public Handler
{
  public:
   HandlePostFork();
   ~HandlePostFork();

  virtual void getEventTypesHandled(vector<EventType> &etypes);
  virtual handler_ret_t handleEvent(Event::ptr ev);
};

class HandlePostExec : public Handler
{
  public:
   HandlePostExec();
   ~HandlePostExec();

   virtual void getEventTypesHandled(vector<EventType> &etypes);
   virtual handler_ret_t handleEvent(Event::ptr ev);
};

class HandleSingleStep : public Handler
{
 public:
  HandleSingleStep();
  ~HandleSingleStep();

  virtual void getEventTypesHandled(vector<EventType> &etypes);
  virtual handler_ret_t handleEvent(Event::ptr ev);
};

class HandleBreakpoint : public Handler
{
 public:
  HandleBreakpoint();
  ~HandleBreakpoint();

  virtual void getEventTypesHandled(vector<EventType> &etypes);
  virtual handler_ret_t handleEvent(Event::ptr ev);
};

class HandleBreakpointClear : public Handler
{
 public:
  HandleBreakpointClear();
  ~HandleBreakpointClear();

  virtual void getEventTypesHandled(vector<EventType> &etypes);
  virtual handler_ret_t handleEvent(Event::ptr ev);
};

class HandleBreakpointRestore : public Handler
{
 public:
  HandleBreakpointRestore();
  ~HandleBreakpointRestore();

  virtual void getEventTypesHandled(vector<EventType> &etypes);
  virtual handler_ret_t handleEvent(Event::ptr ev);
};

class HandleLibrary : public Handler
{
 public:
   HandleLibrary();
   ~HandleLibrary();

   virtual handler_ret_t handleEvent(Event::ptr ev);
   virtual void getEventTypesHandled(std::vector<EventType> &etypes);
};

class HandleRPCInternal : public Handler
{
  public:
   HandleRPCInternal();
   ~HandleRPCInternal();
   
   virtual handler_ret_t handleEvent(Event::ptr ev);
   virtual void getEventTypesHandled(std::vector<EventType> &etypes);   
};

class HandleDetached : public Handler
{
   HandleDetached();
   ~HandleDetached();
   
   virtual handler_ret_t handleEvent(Event::ptr ev);
   virtual void getEventTypesHandled(std::vector<EventType> &etypes);
};

class HandleAsync : public Handler
{
  public:
   HandleAsync();
   ~HandleAsync();
   
   virtual handler_ret_t handleEvent(Event::ptr ev);
   virtual void getEventTypesHandled(std::vector<EventType> &etypes);
};

class HandleNop : public Handler
{
  public:
   HandleNop();
   ~HandleNop();
   
   virtual handler_ret_t handleEvent(Event::ptr ev);
   virtual void getEventTypesHandled(std::vector<EventType> &etypes);
};

class HandlePrepSingleStep : public Handler
{
  public:
   HandlePrepSingleStep();
   ~HandlePrepSingleStep();

   virtual handler_ret_t handleEvent(Event::ptr ev);
   virtual void getEventTypesHandled(std::vector<EventType> &etypes);
};

class HandleCallbacks : public Handler
{
  friend class HandlerPool;
 private:
  typedef std::map<EventType, set<Process::cb_func_t>, eventtype_cmp> cbfuncs_t;
  cbfuncs_t cbfuncs;
  set<EventType, eventtype_cmp> alleventtypes;
  bool registerCallback_int(EventType ev, Process::cb_func_t func);
  bool removeCallback_int(EventType et);
  bool removeCallback_int(EventType et, Process::cb_func_t func);
  bool handleCBReturn(Process::const_ptr proc, Thread::const_ptr thrd, 
                      Process::cb_action_t ret);
 public:
  HandleCallbacks();
  ~HandleCallbacks();

  static HandleCallbacks *getCB();
  virtual int getPriority() const;
  virtual void getEventTypesHandled(vector<EventType> &etypes);
  virtual handler_ret_t handleEvent(Event::ptr ev);
  bool hasCBs(Event::const_ptr ev);
  
  bool registerCallback(EventType ev, Process::cb_func_t func);
  bool removeCallback(EventType et, Process::cb_func_t func);
  bool removeCallback(EventType et);
  bool removeCallback(Process::cb_func_t func);

  bool deliverCallback(Event::ptr ev, const set<Process::cb_func_t> &cbset);
  
  bool requiresCB(Event::const_ptr ev);

  static void getRealEvents(EventType ev, std::vector<EventType> &out_evs);
};

#endif
