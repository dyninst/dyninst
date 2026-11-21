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


struct handler_cmp
{
   bool operator()(const Dyninst::ProcControlAPI::Handler* a, const Dyninst::ProcControlAPI::Handler* b) const
   {
      return a->getPriority() < b->getPriority();
   }
};

class HandlerPool
{
 public:
   typedef std::set<Dyninst::ProcControlAPI::Handler *, handler_cmp> HandlerSet_t;
   typedef std::map<Dyninst::ProcControlAPI::EventType, HandlerSet_t*, Dyninst::ProcControlAPI::eventtype_cmp> HandlerMap_t;

   HandlerPool(int_process *owner_proc);
   ~HandlerPool();

   void addHandler(Dyninst::ProcControlAPI::Handler *handler);
   bool handleEvent(Dyninst::ProcControlAPI::Event::ptr ev);
   Dyninst::ProcControlAPI::Event::ptr handleAsyncEvent(Dyninst::ProcControlAPI::Event::ptr ev);

   void notifyOfPendingAsyncs(const std::set<response::ptr> &asyncs, Dyninst::ProcControlAPI::Event::ptr ev);
   void notifyOfPendingAsyncs(response::ptr async, Dyninst::ProcControlAPI::Event::ptr ev);

   bool isEventAsyncPostponed(Dyninst::ProcControlAPI::Event::ptr ev) const;
   bool hasAsyncEvent() const;

   static bool hasProcAsyncPending();
   void markEventAsyncPending(Dyninst::ProcControlAPI::Event::ptr ev);
   
   void addLateEvent(Dyninst::ProcControlAPI::Event::ptr ev);
   Dyninst::ProcControlAPI::Event::ptr curEvent();

   void setNopAsCurEvent();
   void clearNopAsCurEvent();
 private:
   HandlerMap_t handlers;
   void addHandlerInt(Dyninst::ProcControlAPI::EventType etype, Dyninst::ProcControlAPI::Handler *handler);
   void clearEventAsync(Dyninst::ProcControlAPI::Event::ptr ev);
   void addEventToSet(Dyninst::ProcControlAPI::Event::ptr ev, std::set<Dyninst::ProcControlAPI::Event::ptr> &ev_set) const;
   void collectLateEvents(Dyninst::ProcControlAPI::Event::ptr parent_ev);
   bool hasLateEvents() const;

   bool insertAsyncPendingEvent(Dyninst::ProcControlAPI::Event::ptr ev);
   bool removeAsyncPendingEvent(Dyninst::ProcControlAPI::Event::ptr ev);

   Dyninst::ProcControlAPI::Event::ptr getRealParent(Dyninst::ProcControlAPI::Event::ptr ev) const;

   std::set<Dyninst::ProcControlAPI::Event::ptr> pending_async_events;
   std::set<Dyninst::ProcControlAPI::Event::ptr> late_events;
   int_process *proc;
   Dyninst::ProcControlAPI::Event::ptr cur_event;
   bool nop_cur_event;

   static void markProcAsyncPending(HandlerPool *p);
   static void clearProcAsyncPending(HandlerPool *p);
   static std::set<HandlerPool *> procsAsyncPending;
   static Mutex<false> asyncPendingLock;
};

class HandlePreBootstrap : public Dyninst::ProcControlAPI::Handler
{
 public:
   HandlePreBootstrap();
   virtual ~HandlePreBootstrap();

   virtual void getEventTypesHandled(std::vector<Dyninst::ProcControlAPI::EventType> &etypes);
   virtual handler_ret_t handleEvent(Dyninst::ProcControlAPI::Event::ptr ev);
};


class HandleBootstrap : public Dyninst::ProcControlAPI::Handler
{
 public:
   HandleBootstrap();
   virtual ~HandleBootstrap();

   virtual void getEventTypesHandled(std::vector<Dyninst::ProcControlAPI::EventType> &etypes);
   virtual handler_ret_t handleEvent(Dyninst::ProcControlAPI::Event::ptr ev);
};

class HandleCrash : public Dyninst::ProcControlAPI::Handler
{
 public:
  HandleCrash();
  virtual ~HandleCrash();

  virtual void getEventTypesHandled(std::vector<Dyninst::ProcControlAPI::EventType> &etypes);
  virtual handler_ret_t handleEvent(Dyninst::ProcControlAPI::Event::ptr ev);  
};

class HandleForceTerminate : public Dyninst::ProcControlAPI::Handler
{
 public:
  HandleForceTerminate();
  virtual ~HandleForceTerminate();

  virtual void getEventTypesHandled(std::vector<Dyninst::ProcControlAPI::EventType> &etypes);
  virtual handler_ret_t handleEvent(Dyninst::ProcControlAPI::Event::ptr ev);  
  virtual int getPriority() const;
};

class HandleSignal : public Dyninst::ProcControlAPI::Handler
{
 public:
   HandleSignal();
   virtual ~HandleSignal();
   
   virtual void getEventTypesHandled(std::vector<Dyninst::ProcControlAPI::EventType> &etypes);
   virtual handler_ret_t handleEvent(Dyninst::ProcControlAPI::Event::ptr ev);
};

class HandlePostExit : public Dyninst::ProcControlAPI::Handler
{
 public:
   HandlePostExit();
   virtual ~HandlePostExit();

   virtual void getEventTypesHandled(std::vector<Dyninst::ProcControlAPI::EventType> &etypes);
   virtual handler_ret_t handleEvent(Dyninst::ProcControlAPI::Event::ptr ev);
};

class HandlePostExitCleanup : public Dyninst::ProcControlAPI::Handler
{
  public:
   HandlePostExitCleanup();
   virtual ~HandlePostExitCleanup();

   virtual void getEventTypesHandled(std::vector<Dyninst::ProcControlAPI::EventType> &etypes);
   virtual handler_ret_t handleEvent(Dyninst::ProcControlAPI::Event::ptr ev);
   virtual int getPriority() const;
};

class HandlePreExit : public Dyninst::ProcControlAPI::Handler
{
 public:
   HandlePreExit();
   virtual ~HandlePreExit();

   virtual void getEventTypesHandled(std::vector<Dyninst::ProcControlAPI::EventType> &etypes);
   virtual handler_ret_t handleEvent(Dyninst::ProcControlAPI::Event::ptr ev);
};

class HandleThreadCreate : public Dyninst::ProcControlAPI::Handler
{
 public:
   HandleThreadCreate();
   virtual ~HandleThreadCreate();
   
   virtual void getEventTypesHandled(std::vector<Dyninst::ProcControlAPI::EventType> &etypes);
   virtual handler_ret_t handleEvent(Dyninst::ProcControlAPI::Event::ptr ev);   
};

class HandleThreadDestroy : public Dyninst::ProcControlAPI::Handler
{
 public:
   HandleThreadDestroy();
   virtual ~HandleThreadDestroy();
      
   virtual void getEventTypesHandled(std::vector<Dyninst::ProcControlAPI::EventType> &etypes);
   virtual handler_ret_t handleEvent(Dyninst::ProcControlAPI::Event::ptr ev);
   virtual int getPriority() const;
};

class HandleThreadCleanup : public Dyninst::ProcControlAPI::Handler
{
  public:
   HandleThreadCleanup();
   virtual ~HandleThreadCleanup();

   virtual void getEventTypesHandled(std::vector<Dyninst::ProcControlAPI::EventType> &etypes);
   virtual handler_ret_t handleEvent(Dyninst::ProcControlAPI::Event::ptr ev);
   virtual int getPriority() const;
};

class HandleThreadStop : public Dyninst::ProcControlAPI::Handler
{
 public:
  HandleThreadStop();
  virtual ~HandleThreadStop();

  virtual int getPriority() const;
  virtual void getEventTypesHandled(std::vector<Dyninst::ProcControlAPI::EventType> &etypes);
  virtual handler_ret_t handleEvent(Dyninst::ProcControlAPI::Event::ptr ev);
};

class HandlePostFork : public Dyninst::ProcControlAPI::Handler
{
  public:
   HandlePostFork();
   virtual ~HandlePostFork();

  virtual void getEventTypesHandled(std::vector<Dyninst::ProcControlAPI::EventType> &etypes);
  virtual handler_ret_t handleEvent(Dyninst::ProcControlAPI::Event::ptr ev);
};

class HandlePostForkCont : public Dyninst::ProcControlAPI::Handler
{
  public:
   HandlePostForkCont();
   virtual ~HandlePostForkCont();

  virtual void getEventTypesHandled(std::vector<Dyninst::ProcControlAPI::EventType> &etypes);
  virtual handler_ret_t handleEvent(Dyninst::ProcControlAPI::Event::ptr ev);
  virtual int getPriority() const;
};

class HandlePostExec : public Dyninst::ProcControlAPI::Handler
{
  public:
   HandlePostExec();
   virtual ~HandlePostExec();

   virtual void getEventTypesHandled(std::vector<Dyninst::ProcControlAPI::EventType> &etypes);
   virtual handler_ret_t handleEvent(Dyninst::ProcControlAPI::Event::ptr ev);
};

class HandleSingleStep : public Dyninst::ProcControlAPI::Handler
{
 public:
  HandleSingleStep();
  virtual ~HandleSingleStep();

  virtual void getEventTypesHandled(std::vector<Dyninst::ProcControlAPI::EventType> &etypes);
  virtual handler_ret_t handleEvent(Dyninst::ProcControlAPI::Event::ptr ev);
};

class HandlePreSyscall : public Dyninst::ProcControlAPI::Handler
{
 public:
  HandlePreSyscall();
  virtual ~HandlePreSyscall();

  virtual void getEventTypesHandled(std::vector<Dyninst::ProcControlAPI::EventType> &etypes);
  virtual handler_ret_t handleEvent(Dyninst::ProcControlAPI::Event::ptr ev);
};

class HandlePostSyscall : public Dyninst::ProcControlAPI::Handler
{
 public:
  HandlePostSyscall();
  virtual ~HandlePostSyscall();

  virtual void getEventTypesHandled(std::vector<Dyninst::ProcControlAPI::EventType> &etypes);
  virtual handler_ret_t handleEvent(Dyninst::ProcControlAPI::Event::ptr ev);
};

class HandleBreakpoint : public Dyninst::ProcControlAPI::Handler
{
 public:
  HandleBreakpoint();
  virtual ~HandleBreakpoint();

  virtual void getEventTypesHandled(std::vector<Dyninst::ProcControlAPI::EventType> &etypes);
  virtual handler_ret_t handleEvent(Dyninst::ProcControlAPI::Event::ptr ev);
};

class HandleBreakpointContinue : public Dyninst::ProcControlAPI::Handler
{
  public:
   HandleBreakpointContinue();
   virtual ~HandleBreakpointContinue();
   
   virtual void getEventTypesHandled(std::vector<Dyninst::ProcControlAPI::EventType> &etypes);
   virtual handler_ret_t handleEvent(Dyninst::ProcControlAPI::Event::ptr ev);
   virtual int getPriority() const;
};

class HandleBreakpointClear : public Dyninst::ProcControlAPI::Handler
{
 public:
  HandleBreakpointClear();
  virtual ~HandleBreakpointClear();

  virtual void getEventTypesHandled(std::vector<Dyninst::ProcControlAPI::EventType> &etypes);
  virtual handler_ret_t handleEvent(Dyninst::ProcControlAPI::Event::ptr ev);
};

class HandleBreakpointRestore : public Dyninst::ProcControlAPI::Handler
{
 public:
  HandleBreakpointRestore();
  virtual ~HandleBreakpointRestore();

  virtual void getEventTypesHandled(std::vector<Dyninst::ProcControlAPI::EventType> &etypes);
  virtual handler_ret_t handleEvent(Dyninst::ProcControlAPI::Event::ptr ev);
};

class HandleEmulatedSingleStep : public Dyninst::ProcControlAPI::Handler
{
  public:
   HandleEmulatedSingleStep();
   ~HandleEmulatedSingleStep();

   virtual void getEventTypesHandled(std::vector<Dyninst::ProcControlAPI::EventType> &etypes);
   virtual handler_ret_t handleEvent(Dyninst::ProcControlAPI::Event::ptr ev);
   virtual int getPriority() const;
};

class HandleLibrary : public Dyninst::ProcControlAPI::Handler
{
 public:
   HandleLibrary();
   virtual ~HandleLibrary();

   virtual handler_ret_t handleEvent(Dyninst::ProcControlAPI::Event::ptr ev);
   virtual void getEventTypesHandled(std::vector<Dyninst::ProcControlAPI::EventType> &etypes);
};

class HandleDetach : public Dyninst::ProcControlAPI::Handler
{ 
  public:
   HandleDetach();
   virtual ~HandleDetach();
   
   virtual handler_ret_t handleEvent(Dyninst::ProcControlAPI::Event::ptr ev);
   virtual void getEventTypesHandled(std::vector<Dyninst::ProcControlAPI::EventType> &etypes);
};

class HandleAsync : public Dyninst::ProcControlAPI::Handler
{
  public:
   HandleAsync();
   virtual ~HandleAsync();
   
   virtual handler_ret_t handleEvent(Dyninst::ProcControlAPI::Event::ptr ev);
   virtual void getEventTypesHandled(std::vector<Dyninst::ProcControlAPI::EventType> &etypes);
};

class HandleAsyncIO : public Dyninst::ProcControlAPI::Handler
{
  public:
   HandleAsyncIO();
   virtual ~HandleAsyncIO();
   
   virtual handler_ret_t handleEvent(Dyninst::ProcControlAPI::Event::ptr ev);
   virtual void getEventTypesHandled(std::vector<Dyninst::ProcControlAPI::EventType> &etypes);
};

class HandleAsyncFileRead : public Dyninst::ProcControlAPI::Handler
{
  public:
   HandleAsyncFileRead();
   virtual ~HandleAsyncFileRead();

   virtual handler_ret_t handleEvent(Dyninst::ProcControlAPI::Event::ptr ev);
   virtual void getEventTypesHandled(std::vector<Dyninst::ProcControlAPI::EventType> &etypes);
};

class HandleNop : public Dyninst::ProcControlAPI::Handler
{
  public:
   HandleNop();
   virtual ~HandleNop();
   
   virtual handler_ret_t handleEvent(Dyninst::ProcControlAPI::Event::ptr ev);
   virtual void getEventTypesHandled(std::vector<Dyninst::ProcControlAPI::EventType> &etypes);
};

class HandleCallbacks : public Dyninst::ProcControlAPI::Handler
{
  friend class HandlerPool;
 private:
  typedef std::map<Dyninst::ProcControlAPI::EventType, std::set<Dyninst::ProcControlAPI::Process::cb_func_t>, Dyninst::ProcControlAPI::eventtype_cmp> cbfuncs_t;
  cbfuncs_t cbfuncs;
  std::set<Dyninst::ProcControlAPI::EventType, Dyninst::ProcControlAPI::eventtype_cmp> alleventtypes;
  bool registerCallback_int(Dyninst::ProcControlAPI::EventType ev, Dyninst::ProcControlAPI::Process::cb_func_t func);
  bool removeCallback_int(Dyninst::ProcControlAPI::EventType et);
  bool removeCallback_int(Dyninst::ProcControlAPI::EventType et, Dyninst::ProcControlAPI::Process::cb_func_t func);
  bool handleCBReturn(Dyninst::ProcControlAPI::Process::const_ptr proc, Dyninst::ProcControlAPI::Thread::const_ptr thrd, 
                      Dyninst::ProcControlAPI::Process::cb_action_t ret);
 public:
  HandleCallbacks();
  virtual ~HandleCallbacks();

  static HandleCallbacks *getCB();
  virtual int getPriority() const;
  virtual void getEventTypesHandled(std::vector<Dyninst::ProcControlAPI::EventType> &etypes);
  virtual handler_ret_t handleEvent(Dyninst::ProcControlAPI::Event::ptr ev);
  bool hasCBs(Dyninst::ProcControlAPI::Event::const_ptr ev);
  bool hasCBs(Dyninst::ProcControlAPI::EventType et);
  
  bool registerCallback(Dyninst::ProcControlAPI::EventType ev, Dyninst::ProcControlAPI::Process::cb_func_t func);
  bool removeCallback(Dyninst::ProcControlAPI::EventType et, Dyninst::ProcControlAPI::Process::cb_func_t func);
  bool removeCallback(Dyninst::ProcControlAPI::EventType et);
  bool removeCallback(Dyninst::ProcControlAPI::Process::cb_func_t func);

  Dyninst::ProcControlAPI::Handler::handler_ret_t deliverCallback(Dyninst::ProcControlAPI::Event::ptr ev, const std::set<Dyninst::ProcControlAPI::Process::cb_func_t> &cbset);
  
  bool requiresCB(Dyninst::ProcControlAPI::Event::const_ptr ev);

  static void getRealEvents(Dyninst::ProcControlAPI::EventType ev, std::vector<Dyninst::ProcControlAPI::EventType> &out_evs);
};

class HandlePostponedSyscall : public Dyninst::ProcControlAPI::Handler
{
 public:
  HandlePostponedSyscall();
  virtual ~HandlePostponedSyscall();

  virtual void getEventTypesHandled(std::vector<Dyninst::ProcControlAPI::EventType> &etypes);
  virtual handler_ret_t handleEvent(Dyninst::ProcControlAPI::Event::ptr ev);
};


#endif
