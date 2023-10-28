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
#include "PCErrors.h"
#include "PCProcess.h"
#include "Mailbox.h"

#include "int_process.h"
#include "int_handler.h"
#include "procpool.h"
#include "irpc.h"
#include "response.h"
#include "int_event.h"
#include "processplat.h"
#include "registers/MachRegister.h"

#if defined(os_windows)
#include "windows_process.h"
#endif

#include <iostream>

using namespace Dyninst;
using namespace std;

#include <assert.h>
#include <cstring>

Handler::Handler(std::string name_) :
   name(name_)
{
}

Handler::~Handler()
{
}

int Handler::getPriority() const
{
   return DefaultPriority;
}

Event::ptr Handler::convertEventForCB(Event::ptr /*orig*/)
{
   return Event::ptr();
}

std::string Handler::getName() const
{
   return name;
}

HandlerPool::HandlerPool(int_process *p) :
   proc(p),
   nop_cur_event(false)
{
}

HandlerPool::~HandlerPool()
{
   for (HandlerMap_t::iterator i = handlers.begin(); i != handlers.end(); i++) {
      delete (*i).second;
   }
   //Do not delete actual Handler* objects.
   handlers.clear();
}

static bool print_add_handler = true;
void HandlerPool::addHandlerInt(EventType etype, Handler *handler)
{
   if (print_add_handler)
      pthrd_printf("Handler %s will handle event %s\n", handler->getName().c_str(), 
                   etype.name().c_str());
   assert(etype.time() != EventType::Any);
   HandlerMap_t::iterator i = handlers.find(etype);
   HandlerSet_t *theset = NULL;
   if (i == handlers.end()) {
      theset = new HandlerSet_t();
      handlers[etype] = theset;
   }
   else {
      theset = (*i).second;
   }   
   theset->insert(handler);

   HandleCallbacks *cb = HandleCallbacks::getCB();
   if (cb != handler) {
      cb->alleventtypes.insert(etype);
      if (!(etype.code() >= EventType::InternalEvents && etype.code() < EventType::MaxProcCtrlEvent))
         addHandlerInt(etype, cb);
   }
}

void HandlerPool::addHandler(Handler *handler)
{
   std::vector<EventType> etypes;
   handler->getEventTypesHandled(etypes);

   for (std::vector<EventType>::iterator i = etypes.begin(); i != etypes.end(); i++)
   {
      if ((*i).time() == EventType::Any) {
         addHandlerInt(EventType(EventType::Pre, (*i).code()), handler);
         addHandlerInt(EventType(EventType::Post, (*i).code()), handler);
         addHandlerInt(EventType(EventType::None, (*i).code()), handler);
      }
      else {
         addHandlerInt(*i, handler);
      }
   }
}

struct eh_cmp_func
{
   bool operator()(const pair<Event::ptr, Handler*> &a,
                   const pair<Event::ptr, Handler*> &b) const
   {
      //Async events go first
      if (a.first->getEventType().code() != EventType::Async ||
          b.first->getEventType().code() != EventType::Async)
      {
         if (a.first->getEventType().code() == EventType::Async)
            return true;
         if (b.first->getEventType().code() == EventType::Async)
            return false;
      }

      //Others are run via handler priority
      if (a.second->getPriority() != b.second->getPriority())
         return a.second->getPriority() < b.second->getPriority();

      //Hard-coded rule.  UserThreadDestroy always comes before LWPDestroy
      if (a.first->getEventType().code() == EventType::LWPDestroy &&
          b.first->getEventType().code() == EventType::UserThreadDestroy)
         return false;
      if (b.first->getEventType().code() == EventType::LWPDestroy &&
          a.first->getEventType().code() == EventType::UserThreadDestroy)
         return true;
      
      //Subservient events run latter in handler tie
      if (a.first->subservientTo().lock() == b.first)
         return false;
      if (b.first->subservientTo().lock() == a.first)
         return true;

      //Events are equal in order--just choose a consistent ordering at this point
      eventtype_cmp cmp;
      if (cmp(a.first->getEventType(), b.first->getEventType()))
         return true;
      if (cmp(b.first->getEventType(), a.first->getEventType()))
         return false;
      return a.first < b.first;
   }
};

Event::ptr HandlerPool::curEvent()
{
   if (!cur_event && nop_cur_event) {
      //Lazily create a NOP event as the current event.
      EventNop::ptr nop_event = EventNop::ptr(new EventNop());
      nop_event->setProcess(proc->proc());
      nop_event->setThread(proc->threadPool()->initialThread()->thread());
      nop_event->setSyncType(Event::async);
      cur_event = nop_event;
   }

   return cur_event;
}

void HandlerPool::setNopAsCurEvent()
{
   assert(!cur_event);
   nop_cur_event = true;
}

void HandlerPool::clearNopAsCurEvent()
{
   nop_cur_event = false;
   cur_event = Event::ptr();
}

void HandlerPool::addLateEvent(Event::ptr ev)
{
   late_events.insert(ev);
}

bool HandlerPool::hasLateEvents() const
{
   return !late_events.empty();
}

void HandlerPool::collectLateEvents(Event::ptr parent_ev)
{
   for (set<Event::ptr>::iterator i = late_events.begin(); i != late_events.end(); i++) {
      parent_ev->addSubservientEvent(*i);
   }
   late_events.clear();
}

Event::ptr HandlerPool::getRealParent(Event::ptr ev) const
{
   Event::ptr master_ev = ev;
   for (;;) {
      Event::ptr parent = master_ev->subservientTo().lock();
      if (!parent || parent->getEventType().code() == EventType::Async)
         break;
      master_ev = parent;
   }
   return master_ev;
}

void HandlerPool::notifyOfPendingAsyncs(const std::set<response::ptr> &asyncs, Event::ptr ev)
{
   Event::ptr master_ev = getRealParent(ev);
      
   for (set<response::ptr>::const_iterator i = asyncs.begin(); i != asyncs.end(); i++) {
      (*i)->setEvent(master_ev);
   }
}

void HandlerPool::notifyOfPendingAsyncs(response::ptr async, Event::ptr ev)
{
   async->setEvent(getRealParent(ev));
}

bool HandlerPool::isEventAsyncPostponed(Event::ptr ev) const
{
   return (pending_async_events.find(ev) != pending_async_events.end());
}

bool HandlerPool::hasAsyncEvent() const
{
   return !pending_async_events.empty();
}

void HandlerPool::markEventAsyncPending(Event::ptr ev)
{
   set<Event::ptr>::iterator i = pending_async_events.find(ev);
   if (i != pending_async_events.end()) {
      pthrd_printf("Async event %s on %d/%d has already been marked, leaving alone.\n",
                   ev->name().c_str(), ev->getProcess()->llproc()->getPid(), 
                   ev->getThread()->llthrd()->getLWP());
      return;
   }
   bool was_empty = insertAsyncPendingEvent(ev);
   pthrd_printf("Event %s async marked.  pending_async_size = %d, (was_empty = %s)\n", 
                ev->name().c_str(),
                (int) pending_async_events.size(),
                was_empty ? "true" : "false");
   for (set<Event::ptr>::iterator j = pending_async_events.begin(); j != pending_async_events.end(); j++) {
      pthrd_printf("\tEvent %s (%p)\n", (*j)->name().c_str(), (void*)(*j).get());
   }

   if (was_empty) {
      markProcAsyncPending(this);
   }
}

void HandlerPool::clearEventAsync(Event::ptr ev)
{
   bool result = removeAsyncPendingEvent(ev);
   if (!result)
      return;
   pthrd_printf("Erasing event %s (%p) from list\n", ev->name().c_str(), (void*)ev.get());
   for (set<Event::ptr>::iterator j = pending_async_events.begin(); j != pending_async_events.end(); j++) {
      pthrd_printf("\tEvent %s (%p)\n", (*j)->name().c_str(), (void*)(*j).get());
   }

   if (pending_async_events.empty()) {
      clearProcAsyncPending(this);
   }
}

bool HandlerPool::insertAsyncPendingEvent(Event::ptr ev)
{
   bool was_empty = pending_async_events.empty();
   pair<set<Event::ptr>::iterator, bool> ret = pending_async_events.insert(ev);
   if (ret.second) {
      //This is a new async event.
      proc->asyncEventCount().inc();

      /**
       * Desync the async state for the thread.  This keeps the process or thread
       * stopped while we handle the async event.
       **/
      if (ev->getSyncType() == Event::sync_thread) {
         int_thread *thr = ev->getThread()->llthrd();
         pthrd_printf("Desync'ing async thread state of %d/%d\n", proc->getPid(), thr->getLWP());
         thr->getAsyncState().desyncState(int_thread::ditto);
      }
      else {
         pthrd_printf("Desync'ing async process state of %d\n", proc->getPid());
         proc->threadPool()->initialThread()->getAsyncState().desyncStateProc(int_thread::ditto);
      }
   }
   return was_empty;
}

bool HandlerPool::removeAsyncPendingEvent(Event::ptr ev)
{
   set<Event::ptr>::iterator i = pending_async_events.find(ev);
   if (i == pending_async_events.end()) {
      return false;
   }
   pending_async_events.erase(ev);
   proc->asyncEventCount().dec();

   /**
    * Undo the previous desync.  After the last restore operation, then thread/process
    * should be able to run again.
    **/
   if (ev->getSyncType() == Event::sync_thread) {
      int_thread *thr = ev->getThread()->llthrd();
      pthrd_printf("Restoring'ing async thread state of %d/%d\n", proc->getPid(), thr->getLWP());
      thr->getAsyncState().restoreState();
   }
   else {
      pthrd_printf("Restoring'ing async process state of %d\n", proc->getPid());
      proc->threadPool()->initialThread()->getAsyncState().restoreStateProc();
   }

   return true;
}

void HandlerPool::addEventToSet(Event::ptr ev, set<Event::ptr> &ev_set) const
{
   ev_set.insert(ev);
   for (vector<Event::ptr>::iterator i = ev->subservient_events.begin(); 
        i != ev->subservient_events.end(); i++)
   {
      addEventToSet(*i, ev_set);
   }
}


bool HandlerPool::handleEvent(Event::ptr orig_ev)
{
   Event::ptr cb_replacement_ev = Event::ptr();

   /**
    * An event and its subservient events are a set of events that
    * are processed at the same time.  As an example, on SysV systems
    * a Library event is triggered by a breakpoint.  We want to do both the
    * breakpoint handling and the library handling at the same time,
    * but don't want to seperate the handling of these events.  In this
    * example the Library event is subservient to the Breakpoint event.
    *
    * We'll take the event and all its subservient set and run handlers
    * for all of them.
    **/
   set<Event::ptr> all_events;
   addEventToSet(orig_ev, all_events);

   typedef set<pair<Event::ptr, Handler *>, eh_cmp_func > ev_hndler_set_t;
   ev_hndler_set_t events_and_handlers;
   for (set<Event::ptr>::iterator i = all_events.begin(); i != all_events.end(); i++)
   {
      Event::ptr ev = *i;
      EventType etype = ev->getEventType();
      HandlerMap_t::iterator j = handlers.find(etype);
      if (j == handlers.end()) {
         perr_printf("Event %s has no handlers registered\n", etype.name().c_str());
         continue;
      }
      HandlerSet_t *hset = j->second;
      for (HandlerSet_t::iterator k = hset->begin(); k != hset->end(); k++)
      {
         Handler *hnd = *k;
         bool already_handled = (ev->handled_by.find(hnd) != ev->handled_by.end());
         if (already_handled) {
            pthrd_printf("Have event %s on %s (already handled)\n", 
                         ev->name().c_str(), hnd->getName().c_str());
            continue;
         }
		 pthrd_printf("Event %s added to handle list with handler %s\n",
			 ev->name().c_str(), hnd->getName().c_str());
         events_and_handlers.insert(pair<Event::ptr, Handler*>(ev, hnd));
      }
   }

   /**
    * We should have all events and handlers properly sorted into the
    * events_and_handlers set in the order we want to run them in.
    * Now it's finally time to run the handlers.
    **/
   bool handled_something = false;
   bool had_error = true;

   ev_hndler_set_t::iterator i;

   if (dyninst_debug_proccontrol) {
      for (i = events_and_handlers.begin(); i != events_and_handlers.end(); i++) {
         pthrd_printf("Have event %s on %s\n", i->first->name().c_str(), i->second->getName().c_str());
      }
   }

   for (i = events_and_handlers.begin(); i != events_and_handlers.end(); i++)
   {
      handled_something = true;
      Event::ptr event = i->first;
      Handler *handler = i->second;
      EventType etype = event->getEventType();

      if (handler->getPriority() != Handler::CallbackPriority) {
         //We don't want the callback handler getting async events
         // from user operations in the callback.
         cur_event = getRealParent(event);
      }

      pthrd_printf("Handling event '%s' with handler '%s'\n", etype.name().c_str(),
                   handler->getName().c_str());
      Handler::handler_ret_t result = handler->handleEvent(event);

      cur_event = Event::ptr();
      if (result == Handler::ret_async) {
         pthrd_printf("Handler %s did not complete due to pending asyncs\n",
                      handler->getName().c_str());
         markEventAsyncPending(event);
         return true;
      }
      if (result == Handler::ret_cbdelay) {
         pthrd_printf("Handler %s delaying for user thread\n",
                      handler->getName().c_str());
         mbox()->enqueue_user(orig_ev);
         return true;
      }
      if (result == Handler::ret_again) {
         pthrd_printf("Handler %s throwing event again\n",
                      handler->getName().c_str());
         mbox()->enqueue(orig_ev);
         return true;
      }
      event->handled_by.insert(handler);
      if (result == Handler::ret_error) {
         pthrd_printf("Error handling event %s with %s\n", etype.name().c_str(),
                      handler->getName().c_str());
         had_error = true;
      }
      
      if (hasLateEvents()) {
         /**
          * A handler can choose to add new events to the current set by
          * calling HandlerPool::addLateEvent(...).  We'll check for late
          * events after each handle call, add them as new subservient events,
          * and then recursively call handleEvents.  The recursive call shouldn't
          * re-trigger already-run handlers due to the 'handled_by' mechanism.
          **/
         pthrd_printf("Handler added late events.  Recursively calling handleEvent\n");
         collectLateEvents(event);
         return handleEvent(orig_ev);
      }
   }

   for (auto event : all_events) {
      clearEventAsync(event); //nop if ev wasn't async
   }

   return !had_error && handled_something;
}

std::set<HandlerPool *> HandlerPool::procsAsyncPending;
Mutex<> HandlerPool::asyncPendingLock;

void HandlerPool::markProcAsyncPending(HandlerPool *p)
{
   asyncPendingLock.lock();
   assert(procsAsyncPending.find(p) == procsAsyncPending.end());
   procsAsyncPending.insert(p);
   asyncPendingLock.unlock();
}

void HandlerPool::clearProcAsyncPending(HandlerPool *p)
{
   asyncPendingLock.lock();
   std::set<HandlerPool *>::iterator i = procsAsyncPending.find(p);
   assert(i != procsAsyncPending.end());
   procsAsyncPending.erase(i);
   asyncPendingLock.unlock();
}

bool HandlerPool::hasProcAsyncPending()
{
   asyncPendingLock.lock();
   bool result = !procsAsyncPending.empty();
   if (result) {
      pthrd_printf("Pending async proc: %d\n", (*procsAsyncPending.begin())->proc->getPid());
   }
   asyncPendingLock.unlock();
   return result;
}

HandlePreBootstrap::HandlePreBootstrap() :
Handler(std::string("Pre-bootstrap"))
{
}
HandlePreBootstrap::~HandlePreBootstrap()
{
}

void HandlePreBootstrap::getEventTypesHandled(std::vector<EventType> &etypes)
{
	etypes.push_back(EventType(EventType::None, EventType::PreBootstrap));
}

Handler::handler_ret_t HandlePreBootstrap::handleEvent(Event::ptr)
{
	//int_process* p = ev->getProcess()->llproc();
	//p->setForceGeneratorBlock(true);
	return ret_success;
}


HandleBootstrap::HandleBootstrap() :
   Handler(std::string("Bootstrap"))
{
}

HandleBootstrap::~HandleBootstrap()
{
}

void HandleBootstrap::getEventTypesHandled(std::vector<EventType> &etypes)
{
   etypes.push_back(EventType(EventType::None, EventType::Bootstrap));
}

Handler::handler_ret_t HandleBootstrap::handleEvent(Event::ptr ev)
{
   int_process *proc = ev->getProcess()->llproc();
   int_thread *thrd = ev->getThread()->llthrd();

   assert(proc);
   pthrd_printf("Handling bootstrap for %d\n", proc->getPid());

   if (proc->getState() == int_process::running) {
      if (thrd->getUserState().getState() == int_thread::neonatal_intermediate) {
         //Bootstrapping a thread on an already running process
         int_thread *initial_thread = proc->threadPool()->initialThread();
         int_thread::State it_user_state = initial_thread->getUserState().getState();
         thrd->getUserState().setState(it_user_state);
      }
      return ret_success;
   }

   thrd->getUserState().setState(int_thread::stopped);
   
   bool all_bootstrapped = true;
   int_threadPool *tp = proc->threadPool();
   for (int_threadPool::iterator i = tp->begin(); i != tp->end(); i++) {
      int_thread *thr = *i;
      if (thr->getHandlerState().getState() == int_thread::neonatal_intermediate) {
         pthrd_printf("Thread %d is not yet bootstrapped\n", thr->getLWP());
         all_bootstrapped = false;
         break;
      }
   }

   if (all_bootstrapped) {
      pthrd_printf("All threads are bootstrapped, marking process bootstrapped\n");
      proc->setState(int_process::running);
   }

   return ret_success;
}

HandleSignal::HandleSignal() :
   Handler(std::string("Signal"))
{
}

HandleSignal::~HandleSignal()
{
}

void HandleSignal::getEventTypesHandled(std::vector<EventType> &etypes)
{
   etypes.push_back(EventType(EventType::None, EventType::Signal));
}

Handler::handler_ret_t HandleSignal::handleEvent(Event::ptr ev)
{
   int_thread *thrd = ev->getThread()->llthrd();
   int_process *proc = ev->getProcess()->llproc();
   
   EventSignal *sigev = static_cast<EventSignal *>(ev.get());
   int signal_no = sigev->getSignal();
   thrd->setContSignal(signal_no);

   int_signalMask *sigproc = proc->getSignalMask();
   if (sigproc) {
      if (!sigproc->allowSignal(signal_no)) {
         pthrd_printf("Not giving callback on signal because its not in the SignalMask\n");
         ev->setSuppressCB(true);
      }
   }

   return ret_success;
}

HandlePostExit::HandlePostExit() :
   Handler("Post Exit")
{
}

HandlePostExit::~HandlePostExit()
{
}

void HandlePostExit::getEventTypesHandled(std::vector<EventType> &etypes)
{
   etypes.push_back(EventType(EventType::Post, EventType::Exit));
}

Handler::handler_ret_t HandlePostExit::handleEvent(Event::ptr ev)
{
   int_process *proc = ev->getProcess()->llproc();
   assert(proc);
   if(ev->getThread()) {
	   int_thread *thrd = ev->getThread()->llthrd();
	   assert(thrd);
	   pthrd_printf("Handling post-exit for process %d on thread %d\n",
		   proc->getPid(), thrd->getLWP());
   } else {
	   pthrd_printf("Handling post-exit for process %d, all threads gone\n",
		   proc->getPid());
   }

   EventExit *event = static_cast<EventExit *>(ev.get());
   
   ProcPool()->condvar()->lock();

   proc->setState(int_process::exited);
   ProcPool()->rmProcess(proc);
   if(proc->wasForcedTerminated())
   {
	   proc->getStartupTeardownProcs().dec();
	   ev->setSuppressCB(true);
   }
   else
   {
	   proc->setExitCode(event->getExitCode());
   }
   ProcPool()->condvar()->broadcast();
   ProcPool()->condvar()->unlock();

   return ret_success;
}

HandlePostExitCleanup::HandlePostExitCleanup() :
   Handler("Post Exit Cleanup")
{
}

HandlePostExitCleanup::~HandlePostExitCleanup()
{
}

void HandlePostExitCleanup::getEventTypesHandled(std::vector<EventType> &etypes)
{
   etypes.push_back(EventType(EventType::Post, EventType::Exit));
   etypes.push_back(EventType(EventType::None, EventType::Crash));
}

Handler::handler_ret_t HandlePostExitCleanup::handleEvent(Event::ptr ev)
{
   int_process *proc = ev->getProcess()->llproc();
   int_thread *thrd = ev->getThread() ? ev->getThread()->llthrd() : NULL;
   assert(proc);

#if defined(os_windows)
   // On Windows, this is the only callback we get, so delay setting exited
   // until cleanup
   proc->setState(int_process::exited);
#endif
//   assert(thrd);
   pthrd_printf("Handling post-exit/crash cleanup for process %d on thread %d\n",
	   proc->getPid(), thrd ? thrd->getLWP() : (Dyninst::LWP)(-1));

   for (int_threadPool::iterator i = proc->threadPool()->begin(); i != proc->threadPool()->end(); ++i) {
      (*i)->setPendingStop(false);
   }

   proc->setForceGeneratorBlock(false);

   if (int_process::in_waitHandleProc == proc) {
      pthrd_printf("Postponing delete due to being in waitAndHandleForProc\n");
   } else {
      delete proc;
   }

   return ret_success;
}

int HandlePostExitCleanup::getPriority() const
{
   return Handler::PostCallbackPriority;
}

HandleCrash::HandleCrash() :
   Handler("Crash")
{
}

HandleCrash::~HandleCrash()
{
}

void HandleCrash::getEventTypesHandled(std::vector<EventType> &etypes)
{
   etypes.push_back(EventType(EventType::None, EventType::Crash));
}

Handler::handler_ret_t HandleCrash::handleEvent(Event::ptr ev)
{
   pthrd_printf("Handling crash for process %d on thread %d\n",
                ev->getProcess()->getPid(), ev->getThread()->getLWP());
//                evproc->getPid(), thrd->getLWP());   
   int_process *proc = ev->getProcess()->llproc();
   int_thread *thrd = ev->getThread()->llthrd();
   assert(proc);
   assert(thrd);
   if( !proc || !thrd) return ret_error;
   
   EventCrash *event = static_cast<EventCrash *>(ev.get());

   if (proc->wasForcedTerminated()) {
      pthrd_printf("Crash was due to process::terminate, not reporting\n");
      event->setSuppressCB(true);
   }
   else {
      proc->setCrashSignal(event->getTermSignal());
   }
   
   ProcPool()->condvar()->lock();

   proc->setState(int_process::exited);
   ProcPool()->rmProcess(proc);

   ProcPool()->condvar()->broadcast();
   ProcPool()->condvar()->unlock();

   return ret_success;
}

HandleForceTerminate::HandleForceTerminate() :
    Handler("ForceTerminate")
{
}

HandleForceTerminate::~HandleForceTerminate()
{
}

void HandleForceTerminate::getEventTypesHandled(std::vector<EventType> &etypes)
{
    etypes.push_back(EventType(EventType::Post, EventType::ForceTerminate));
}


Handler::handler_ret_t HandleForceTerminate::handleEvent(Event::ptr ev) {
   int_process *proc = ev->getProcess()->llproc();
   Thread::const_ptr t = ev->getThread();
   int_thread* thrd = NULL;
   if(t)
      thrd = t->llthrd();

   assert(proc);
   // assert(thrd);
   pthrd_printf("Handling force terminate for process %d on thread %d\n",
	   proc->getPid(), thrd ? thrd->getLWP() : (Dyninst::LWP)(-1));

   ProcPool()->condvar()->lock();

   proc->setState(int_process::exited);
   for (int_threadPool::iterator iter = proc->threadPool()->begin(); 
        iter != proc->threadPool()->end(); ++iter) {
      ProcPool()->addDeadThread((*iter)->getLWP());
   }


   ProcPool()->rmProcess(proc);

   ProcPool()->condvar()->broadcast();
   ProcPool()->condvar()->unlock();

   proc->getStartupTeardownProcs().dec();

   if (int_process::in_waitHandleProc == proc) {
      pthrd_printf("Postponing delete due to being in waitAndHandleForProc\n");
   } else {
      delete proc;
   }

   return ret_success;
}

int HandleForceTerminate::getPriority() const {
   return PostCallbackPriority;
}

HandlePreExit::HandlePreExit() :
   Handler("Pre Exit")
{
}

HandlePreExit::~HandlePreExit()
{
}

void HandlePreExit::getEventTypesHandled(std::vector<EventType> &etypes)
{
   etypes.push_back(EventType(EventType::Pre, EventType::Exit));
}

Handler::handler_ret_t HandlePreExit::handleEvent(Event::ptr ev)
{
   int_process *proc = ev->getProcess()->llproc();
   int_thread *thread = ev->getThread()->llthrd();
   pthrd_printf("Handling pre-exit for process %d on thread %d\n",
                proc->getPid(), thread->getLWP());
   // Sometimes when the mutator attempts to stop the mutatee,
   // the mutatee exited before the stop is delivered to the mutatee.
   // In such case, let the mutator not wait for the exited thread.
   // Otherwise, the mutator can be stuck in an infinite loop waiting
   // for a SIGSTOP that will never come 
   if (thread->hasPendingStop())
      thread->setPendingStop(false);
   thread->setExiting(true);
   if (proc->wasForcedTerminated()) {
      //Linux sometimes throws an extraneous exit after
      // calling ptrace(PTRACE_KILL, ...).  It's not a real exit
      pthrd_printf("Proc pre-exit was due to process::terminate, not reporting\n");
      ev->setSuppressCB(true);
      if (ev->getSyncType() == Event::sync_thread)
         thread->getExitingState().setState(int_thread::running);
      else
         thread->getExitingState().setStateProc(int_thread::running);
      
   }

   return ret_success;
}

HandleThreadCreate::HandleThreadCreate() :
   Handler("Thread Create")
{
}

HandleThreadCreate::~HandleThreadCreate()
{
}

void HandleThreadCreate::getEventTypesHandled(std::vector<EventType> &etypes)
{
   etypes.push_back(EventType(EventType::None, EventType::ThreadCreate));
   etypes.push_back(EventType(EventType::None, EventType::UserThreadCreate));
   etypes.push_back(EventType(EventType::None, EventType::LWPCreate));
}

Handler::handler_ret_t HandleThreadCreate::handleEvent(Event::ptr ev)
{
   int_process *proc = ev->getProcess()->llproc();
   Thread::const_ptr hl_thrd = ev->getThread();
   int_thread* thrd = NULL;
   if(hl_thrd) thrd = hl_thrd->llthrd();
   EventNewThread *threadev = static_cast<EventNewThread *>(ev.get());

   pthrd_printf("Handle thread create for %d/%d with new thread %d\n",
	   proc->getPid(), thrd ? thrd->getLWP() : (Dyninst::LWP)(-1), threadev->getLWP());

   if (thrd && thrd->getPostponedSyscallState().isDesynced())
      thrd->getPostponedSyscallState().restoreState();

   if (ev->getEventType().code() == EventType::UserThreadCreate) {
      //If we support both user and LWP thread creation, and we're doing a user
      // creation, then the Thread object may already exist.  Do nothing.
      int_thread *thr = proc->threadPool()->findThreadByLWP(threadev->getLWP());
      if (thr) {
         pthrd_printf("Thread object already exists, ThreadCreate handler doing nothing\n");
         return ret_success;
      }
   }
   ProcPool()->condvar()->lock();
   int_thread::attach_status_t astatus = int_thread::as_unknown;
   if (ev->getEventType().code() == EventType::LWPCreate) {
      EventNewLWP::ptr lwp_create = ev->getEventNewLWP();
      astatus = lwp_create->getInternalEvent()->attach_status;
   }
   int_thread *newthr = int_thread::createThread(proc, NULL_THR_ID, threadev->getLWP(), false, astatus);

   newthr->getGeneratorState().setState(int_thread::stopped);
   newthr->getHandlerState().setState(int_thread::stopped);

   if (!thrd) {
      //This happens on BG/P with user thread events.
      pthrd_printf("Setting new event to have occured on new thread\n");
      ev->setThread(newthr->thread());
      thrd = newthr;
   }

   int_thread *inherit_from = (thrd == newthr) ? proc->threadPool()->initialThread() : thrd;
   newthr->getUserState().setState(inherit_from->getUserState().getState());
   
   pthrd_printf("Initializing new thread states to match rest of process for %d/%d\n",
                proc->getPid(), newthr->getLWP());
   map<int, int> &states = proc->getProcDesyncdStates();
   for (map<int, int>::iterator i = states.begin(); i != states.end(); i++) {
      if (!i->second)
         continue;
      int_thread::StateTracker &statet = newthr->getStateByID(i->first);
      int_thread::State ns = proc->threadPool()->initialThread()->getStateByID(i->first).getState();
      if (statet.getID() == int_thread::BreakpointResumeStateID) {
         //Special case, new threads always go to stopped for breakpoint resume.
         pthrd_printf(".... setting state to stopped for BreakpointResumeStateID\n");
         ns = int_thread::stopped;
      }
      for (int j = 0; j < i->second; j++) {
         pthrd_printf("desyncing state for %d\n", statet.debugthr()->getLWP());
         statet.desyncState(ns);
      }
   }
   pthrd_printf("finished initializing thread %d/%d\n",
                proc->getPid(), newthr->getLWP());

   ProcPool()->condvar()->broadcast();
   ProcPool()->condvar()->unlock();

   return ret_success;
}

HandleThreadDestroy::HandleThreadDestroy() :
   Handler("Thread Destroy")
{
}

HandleThreadDestroy::~HandleThreadDestroy()
{
}

void HandleThreadDestroy::getEventTypesHandled(std::vector<EventType> &etypes)
{
   etypes.push_back(EventType(EventType::Any, EventType::UserThreadDestroy));
   etypes.push_back(EventType(EventType::Any, EventType::LWPDestroy));
   etypes.push_back(EventType(EventType::Any, EventType::WinStopThreadDestroy));
}

int HandleThreadDestroy::getPriority() const
{
   return Handler::PostPlatformPriority + 2; //After thread_db destroy handler
}

Handler::handler_ret_t HandleThreadDestroy::handleEvent(Event::ptr ev)
{
   int_thread *thrd = ev->getThread()->llthrd();

   /* The internal thread can be NULL if we receive multiple ThreadDestroy events
    * for the same thread. This can happen when handling "ghost" threads.
    */
   if(!thrd) {
	   ev->setSuppressCB(true);
	   pthrd_printf("Encountered an already-destroyed thread\n");
	   return ret_success;
   }

   if (!thrd->isUser()) {
      ev->setSuppressCB(true);
   }

   int_process *proc = ev->getProcess()->llproc();

   if (ev->getEventType().time() == EventType::Pre && proc->plat_supportLWPPostDestroy()) {
      pthrd_printf("Handling pre-thread destroy for %d\n", thrd->getLWP());
      return ret_success;
   }

   if (ev->getEventType().code() == EventType::UserThreadDestroy &&
       (proc->plat_supportLWPPostDestroy() || proc->plat_supportLWPPreDestroy())) 
   {
      //This is a user thread delete, but we still have an upcoming LWP 
      // delete.  Don't actually do anything yet.
      pthrd_printf("Handling user thread... postponing clean of %d/%d until LWP delete\n", 
                   proc->getPid(), thrd->getLWP());
      return ret_success;
   }

   pthrd_printf("Handling post-thread destroy for %d/%d\n", proc->getPid(), thrd->getLWP());

   if (proc->wasForcedTerminated()) {
      //Linux sometimes throws an extraneous thread terminate after
      // calling ptrace(PTRACE_KILL, ...).  It's not a real thread terminate.
      pthrd_printf("Thread terminate was due to process::terminate, not reporting\n");
      ev->setSuppressCB(true);
   }

   return ret_success;
}

HandleThreadCleanup::HandleThreadCleanup() :
   Handler("Thread Cleanup")
{
}

HandleThreadCleanup::~HandleThreadCleanup()
{
}

void HandleThreadCleanup::getEventTypesHandled(vector<EventType> &etypes)
{
   etypes.push_back(EventType(EventType::Any, EventType::UserThreadDestroy));
   etypes.push_back(EventType(EventType::Any, EventType::LWPDestroy));
   etypes.push_back(EventType(EventType::Any, EventType::WinStopThreadDestroy));
}

int HandleThreadCleanup::getPriority() const
{
   return Handler::PostCallbackPriority + 1;
}


Handler::handler_ret_t HandleThreadCleanup::handleEvent(Event::ptr ev)
{
   /**
    * This is a seperate handler so that the cleanup happens after any
    * user callback.
    **/
   int_process *proc = ev->getProcess()->llproc();
   if(!proc) {
	   pthrd_printf("Process for thread cleanup event is NULL. We have no work we can do.\n");
	   return ret_success;
   }
   if ((ev->getEventType().code() == EventType::UserThreadDestroy) &&
       (proc->plat_supportLWPPreDestroy() || proc->plat_supportLWPPostDestroy()))
   {
      //Have a user thread destroy, but platform supports LWP destroys.  Will clean with a latter event.
      pthrd_printf("Nothing to do in HandleThreadCleanup\n");
      return ret_success;
   }
   if ((ev->getEventType().code() == EventType::LWPDestroy && ev->getEventType().time() == EventType::Pre) &&
       proc->plat_supportLWPPostDestroy())
   {
      //Have a pre-lwp destroy, but platform supports post-lwp destroy.  Will clean with a latter event.
      pthrd_printf("Nothing to do in HandleThreadCleanup\n");
      return ret_success;
   }
   bool should_delete = true;
   if ((ev->getEventType().code() == EventType::UserThreadDestroy && 
        !proc->plat_supportLWPPreDestroy() && !proc->plat_supportLWPPostDestroy()))
   {
      //Have a user thread destroy, and platform doesn't support LWP destroys.  Will partially clean,
      // but leave thread object thread throws any events before it deletes.
      should_delete = false;
   }


   int_thread *thrd = ev->getThread()->llthrd();
   if(!thrd) {
	   pthrd_printf("Thread for thread cleanup event is NULL. We have no work we can do.\n");
	   return ret_success;
   }
   pthrd_printf("Cleaning thread %d/%d from HandleThreadCleanup handler.\n", 
                proc->getPid(), thrd->getLWP());
   int_thread::cleanFromHandler(thrd, should_delete);
   return ret_success;
}

HandleThreadStop::HandleThreadStop() :
   Handler(std::string("Thread Stop"))
{
}

HandleThreadStop::~HandleThreadStop()
{
}

int HandleThreadStop::getPriority() const
{
	return DefaultPriority;
}

void HandleThreadStop::getEventTypesHandled(std::vector<EventType> &etypes)
{
   etypes.push_back(EventType(EventType::None, EventType::Stop));
}

Handler::handler_ret_t HandleThreadStop::handleEvent(Event::ptr ev)
{
   int_process *proc = ev->getProcess()->llproc();

   if (ev->getSyncType() == Event::sync_process) {
      pthrd_printf("Handling process stop for %d\n", proc->getPid());
      bool found_pending_stop = false;
#if defined(os_windows)
	  windows_process *wproc = dynamic_cast<windows_process *>(proc);
	  if (wproc && wproc->pendingDebugBreak()) {
		  found_pending_stop = true;
		  wproc->clearPendingDebugBreak();
	  }
#endif

      for (int_threadPool::iterator i = proc->threadPool()->begin(); i != proc->threadPool()->end(); i++) {
         int_thread *thrd = *i;
         pthrd_printf("Handling process stop for %d/%d\n", proc->getPid(), thrd->getLWP());
         if (thrd->hasPendingStop()) {
            found_pending_stop = true;
            thrd->setPendingStop(false);
         }
      }
      assert(found_pending_stop);
   }
   else {
      int_thread *thrd = ev->getThread()->llthrd();
      pthrd_printf("Handling thread stop for %d/%d\n", proc->getPid(), thrd->getLWP());
      assert(thrd->hasPendingStop());
      thrd->setPendingStop(false);
   }
   return ret_success;
}


HandlePostFork::HandlePostFork() :
   Handler("Post Fork")
{
}

HandlePostFork::~HandlePostFork()
{
}

void HandlePostFork::getEventTypesHandled(std::vector<EventType> &etypes)
{
   etypes.push_back(EventType(EventType::Post, EventType::Fork));
}

Handler::handler_ret_t HandlePostFork::handleEvent(Event::ptr ev)
{
   EventFork *efork = static_cast<EventFork *>(ev.get());
   Dyninst::PID child_pid = efork->getPID();
   int_process *parent_proc = ev->getProcess()->llproc();
   pthrd_printf("Handling fork for parent %d to child %d\n",
                parent_proc->getPid(), child_pid);

   int_process *child_proc = ProcPool()->findProcByPid(child_pid);
   if( child_proc == NULL ) {
       child_proc = int_process::createProcess(child_pid, parent_proc);
   }

   int_followFork *fork_proc = parent_proc->getFollowFork();
   if (fork_proc->fork_isTracking() == FollowFork::DisableBreakpointsDetach) {
      //Silence this event.  Child will be detached.
      ev->setSuppressCB(true);
   }

   int_thread *thrd = ev->getThread()->llthrd();
   if (thrd && thrd->getPostponedSyscallState().isDesynced())
      thrd->getPostponedSyscallState().restoreState();

   assert(child_proc);
   return child_proc->forked() ? ret_success : ret_error;
}

HandlePostForkCont::HandlePostForkCont() :
   Handler("Post Fork Continue")
{
}

HandlePostForkCont::~HandlePostForkCont()
{
}

void HandlePostForkCont::getEventTypesHandled(std::vector<EventType> &etypes)
{
   etypes.push_back(EventType(EventType::Post, EventType::Fork));
}

Handler::handler_ret_t HandlePostForkCont::handleEvent(Event::ptr ev)
{
   EventFork *efork = static_cast<EventFork *>(ev.get());
   Dyninst::PID child_pid = efork->getPID();
   int_process *child_proc = ProcPool()->findProcByPid(child_pid);
   int_process *parent_proc = ev->getProcess()->llproc();
   pthrd_printf("Handling post-fork continue for child %d\n", child_pid);
   assert(child_proc);

   int_followFork *fork_proc = parent_proc->getFollowFork();
   if (fork_proc->fork_isTracking() == FollowFork::DisableBreakpointsDetach) {
      child_proc->throwDetachEvent(false, false);
   }
   else {
      //We need syncRunState to run for the new process to continue it.
      // do so by throwing a NOP event on the new process.
      child_proc->throwNopEvent();
   }
   return ret_success;
}

int HandlePostForkCont::getPriority() const {
   return PostCallbackPriority;
}

HandlePostExec::HandlePostExec() :
   Handler("Post Exec")
{
}

HandlePostExec::~HandlePostExec()
{
}

void HandlePostExec::getEventTypesHandled(std::vector<EventType> &etypes)
{
   etypes.push_back(EventType(EventType::Post, EventType::Exec));
}

Handler::handler_ret_t HandlePostExec::handleEvent(Event::ptr ev)
{
   EventExec *eexec = static_cast<EventExec *>(ev.get());
   int_process *proc = ev->getProcess()->llproc();
   pthrd_printf("Handling exec for process %d\n",
                proc->getPid());

   bool result = proc->execed();
   if (!result) 
      return ret_error;
   
   eexec->setExecPath(proc->getExecutable());
   eexec->setThread(proc->threadPool()->initialThread()->thread());
   return ret_success;
}

HandleSingleStep::HandleSingleStep() :
   Handler("Single Step")
{
}

HandleSingleStep::~HandleSingleStep()
{
}

void HandleSingleStep::getEventTypesHandled(vector<EventType> &etypes)
{
   etypes.push_back(EventType(EventType::None, EventType::SingleStep));
}

Handler::handler_ret_t HandleSingleStep::handleEvent(Event::ptr ev)
{
   pthrd_printf("Handling event single step on %d/%d\n", 
                ev->getProcess()->llproc()->getPid(), 
                ev->getThread()->llthrd()->getLWP());
   return ret_success;
}

HandlePreSyscall::HandlePreSyscall() :
    Handler("Pre Syscall")
{
}

HandlePreSyscall::~HandlePreSyscall()
{
}

void HandlePreSyscall::getEventTypesHandled(vector<EventType> &etypes)
{
   etypes.push_back(EventType(EventType::Pre, EventType::PreSyscall));
}

Handler::handler_ret_t HandlePreSyscall::handleEvent(Event::ptr ev)
{
   pthrd_printf("Handling event pre-syscall on %d/%d\n", 
                ev->getProcess()->llproc()->getPid(), 
                ev->getThread()->llthrd()->getLWP());
   return ret_success;
}

HandlePostSyscall::HandlePostSyscall() :
    Handler("Post Syscall")
{
}

HandlePostSyscall::~HandlePostSyscall()
{
}

void HandlePostSyscall::getEventTypesHandled(vector<EventType> &etypes)
{
   etypes.push_back(EventType(EventType::Post, EventType::PostSyscall));
}

Handler::handler_ret_t HandlePostSyscall::handleEvent(Event::ptr ev)
{
   pthrd_printf("Handling event post-syscall on %d/%d\n", 
                ev->getProcess()->llproc()->getPid(), 
                ev->getThread()->llthrd()->getLWP());
   return ret_success;
}

/**
 * This handler is triggered when a breakpoint is first hit (e.g., on
 * the SIGTRAP signal.  It's main purpose is to prepare the thread state
 * before the user callback
 **/
HandleBreakpoint::HandleBreakpoint() :
   Handler("Breakpoint")
{
}

HandleBreakpoint::~HandleBreakpoint()
{
}

void HandleBreakpoint::getEventTypesHandled(vector<EventType> &etypes)
{
   etypes.push_back(EventType(EventType::None, EventType::Breakpoint));
}

Handler::handler_ret_t HandleBreakpoint::handleEvent(Event::ptr ev)
{
   int_process *proc = ev->getProcess()->llproc();
   int_thread *thrd = ev->getThread()->llthrd();

   EventBreakpoint *ebp = static_cast<EventBreakpoint *>(ev.get());
   pthrd_printf("Handling breakpoint at %lx\n", ebp->getAddress());
   int_eventBreakpoint *int_ebp = ebp->getInternal();
   bp_instance *breakpoint = int_ebp->lookupInstalledBreakpoint();

   if (!breakpoint) {
      //Likely someone cleaned the breakpoint.
      pthrd_printf("No breakpoint installed at location of BP hit.\n");
      return ret_success;
   }
   sw_breakpoint *swbp = breakpoint->swBP();

   /**
    * Move the PC if it's a control transfer breakpoint, or if the
    * current architecture advances the PC forward upon breakpoint
    * hit (x86 or x86_64).
    **/
   bool changePC = false;
   Address changePCTo = 0x0;
   MachRegister pcreg = MachRegister::getPC(proc->getTargetArch());
   int_breakpoint *transferbp = breakpoint->getCtrlTransferBP(thrd);

   if (transferbp) {
      changePC = true;
      if (transferbp->isOffsetTransfer()) {
         Address cur_addr = breakpoint->getAddr();
         signed long offset = (signed long) transferbp->toAddr();
         changePCTo = cur_addr + offset;
      }
      else {
         changePCTo = transferbp->toAddr();
      }
      pthrd_printf("Breakpoint has control transfer.  Moving PC to %lx\n", changePCTo);
   }
   else if (swbp && proc->plat_breakpointAdvancesPC()) {
      changePC = true;
      changePCTo = breakpoint->getAddr();
      pthrd_printf("Breakpoint shifted PC.  Moving PC to %lx\n", changePCTo);
   }

   if (changePC && !int_ebp->pc_regset) {
      int_ebp->pc_regset = result_response::createResultResponse();
      bool ok = thrd->setRegister(pcreg, changePCTo, int_ebp->pc_regset);
      if(!ok)
      {
         pthrd_printf("Error setting pc register on breakpoint\n");
         ev->setLastError(err_internal, "Could not set pc register upon breakpoint\n");
         return ret_error;
      }
   }
   if (int_ebp->pc_regset && int_ebp->pc_regset->hasError()) {
      pthrd_printf("Error setting pc register on breakpoint\n");
      ev->setLastError(err_internal, "Could not set pc register upon breakpoint\n");
      return ret_error;
   }
   if (int_ebp->pc_regset && !int_ebp->pc_regset->isReady()) {
      //We're probably waiting for async to finish.
      proc->handlerPool()->notifyOfPendingAsyncs(int_ebp->pc_regset, ev);
      pthrd_printf("Returning async from BP handler while setting PC\n");
      return ret_async;
   }

   /**
    * Check if any of the breakpoints should trigger a callback
    **/
   vector<Breakpoint::ptr> hl_bps;
   ebp->getBreakpoints(hl_bps);
   int_ebp->cb_bps.clear();
   for (vector<Breakpoint::ptr>::iterator i = hl_bps.begin(); i != hl_bps.end(); i++) {
      int_breakpoint *bp = (*i)->llbp();
      if (bp->isOneTimeBreakpoint() && bp->isOneTimeBreakpointHit())
         continue;
      if (bp->isThreadSpecific() && !bp->isThreadSpecificTo(ev->getThread()))
         continue;
      int_ebp->cb_bps.insert(*i);
   }
   pthrd_printf("In breakpoint handler, %ld breakpoints for user callback\n",
                (long int) int_ebp->cb_bps.size());
   if (int_ebp->cb_bps.empty())
      ebp->setSuppressCB(true);

   /**
    * Handle onetime breakpoints.  These are essentially auto-deleted
    * the first time they're hit.  onetime breakpoints that are thread
    * specific are only auto-deleted if they are hit by their thread.
    **/
   for (bp_instance::iterator i = breakpoint->begin(); i != breakpoint->end(); i++) {
      int_breakpoint *bp = *i;
      if (!bp->isOneTimeBreakpoint())
         continue;
      if (bp->isThreadSpecific() && !bp->isThreadSpecificTo(ev->getThread()))
         continue;
      if (bp->isOneTimeBreakpointHit()) {
         pthrd_printf("One time breakpoint was hit twice, should have CB dropped.\n");
         continue;
      }
      pthrd_printf("Marking oneTimeBreakpoint as hit\n");
      bp->markOneTimeHit();
   }

   /**
    * Breakpoints show up as permanent stops to the user.  If they do a 
    * default cb_ret_t, then the process remains stopped.
    **/

   if (!int_ebp->cb_bps.empty() && !ebp->suppressCB())
   {
      pthrd_printf("BP handler is setting user state to reflect breakpoint\n");
      switch (ebp->getSyncType()) {
         case Event::sync_process: {
            int_threadPool *pool = proc->threadPool();
            for (int_threadPool::iterator i = pool->begin(); i != pool->end(); i++)
               (*i)->getUserState().setState(int_thread::stopped);
            break;
         }
         case Event::sync_thread: 
            thrd->getUserState().setState(int_thread::stopped);
            break;
         case Event::async:
            assert(0); //Async BPs don't make sense to me
            break;
         case Event::unset:
            assert(0);
            break;
      }
   }

   /**
    * Mark the thread as being stopped on this breakpoint.  Control transfer
    * bps don't leave their thread stopped at the BP, so don't do them.
    **/
   if (!transferbp && breakpoint->needsClear()) {
      pthrd_printf("Breakpoint handler decided BP needs clearing\n");
      thrd->markStoppedOnBP(breakpoint);
   }
   else {
      pthrd_printf("Breakpoint handler decided BP does not need clearing\n");
   }

   return ret_success;
}

HandleBreakpointContinue::HandleBreakpointContinue() :
   Handler("BreakpointContinue")
{
}

HandleBreakpointContinue::~HandleBreakpointContinue()
{
}

void HandleBreakpointContinue::getEventTypesHandled(vector<EventType> &etypes)
{
   etypes.push_back(EventType(EventType::None, EventType::Breakpoint));
}

int HandleBreakpointContinue::getPriority() const
{
   return PostCallbackPriority;
}

Handler::handler_ret_t HandleBreakpointContinue::handleEvent(Event::ptr ev)
{
   /**
    * This is in a seperate handler from Breakpoint to make it run after
    * any subservient events associated with Breakpoint.  Library or SysV
    * events want to operate on a stopped process, so we've postponed this
    * continue until after those events are done.
    **/
   EventBreakpoint *ebp = static_cast<EventBreakpoint *>(ev.get());
   int_eventBreakpoint *int_bp = ebp->getInternal();
   
   if (int_bp->stopped_proc) {
      ebp->getThread()->llthrd()->getBreakpointHoldState().restoreStateProc();
   }
   return Handler::ret_success;
}

HandleBreakpointClear::HandleBreakpointClear() :
   Handler("BreakpointClear")
{
}

HandleBreakpointClear::~HandleBreakpointClear()
{
}

void HandleBreakpointClear::getEventTypesHandled(vector<EventType> &etypes)
{
   etypes.push_back(EventType(EventType::None, EventType::BreakpointClear));
}

/**
 * The handler triggers when a thread stopped on a breakpoint is continued.
 **/
Handler::handler_ret_t HandleBreakpointClear::handleEvent(Event::ptr ev)
{
   int_process *proc = ev->getProcess()->llproc();
   int_thread *thrd = ev->getThread()->llthrd();

   EventBreakpointClear *evbpc = static_cast<EventBreakpointClear *>(ev.get());
   int_eventBreakpointClear *int_bpc = evbpc->getInternal();
   bp_instance *ibp = thrd->isStoppedOnBP();
   hw_breakpoint *hwbp = dynamic_cast<hw_breakpoint *>(ibp);

   if (!ibp || !ibp->isInstalled()) {
      pthrd_printf("HandleBreakpointClear on thread without breakpoint.  BP must have been deleted\n");
      thrd->getBreakpointState().restoreStateProc();
      thrd->markStoppedOnBP(NULL);
      return Handler::ret_success;
   }
   assert(!int_bpc->stopped_proc ||
          proc->threadPool()->allStopped(int_thread::BreakpointStateID) ||
          proc->threadPool()->allStopped(int_thread::AsyncStateID));

   /**
    * Suspend breakpoint
    **/
   if (!int_bpc->started_bp_suspends) {
      pthrd_printf("Removing breakpoint from memory under HandleBreakpointClear\n");
      int_bpc->started_bp_suspends = true;
      bool result = (bool)ibp->suspend(proc, int_bpc->bp_suspend);
      if (!result) {
         pthrd_printf("Error suspending breakpoint in HandleBreakpointClear\n");
         return Handler::ret_error;
      }
   }

   /**
    * Handle cases where breakpoint suspends are async and not completed.
    **/
   for (set<response::ptr>::iterator i = int_bpc->bp_suspend.begin(); i != int_bpc->bp_suspend.end();) {
      response::ptr resp = *i;
      if (!resp || !resp->isPosted() || resp->isReady())
         int_bpc->bp_suspend.erase(i++);
      else
         i++;
   }
   if (!int_bpc->bp_suspend.empty()) {
      pthrd_printf("Breakpoint clear is pending in HandleBreakpointClear.  Returning async\n");
      proc->handlerPool()->notifyOfPendingAsyncs(int_bpc->bp_suspend, ev);
      return Handler::ret_async;
   }

   /**
    * Check if any of the int_breakpoints are going to be restored
    **/
   bool restore_bp = false;
   for (sw_breakpoint::iterator i = ibp->begin(); i != ibp->end(); i++) {
      if ((*i)->isOneTimeBreakpoint() && (*i)->isOneTimeBreakpointHit())
         continue;
      restore_bp = true;
      break;
   }

   thrd->markStoppedOnBP(NULL);
   if (restore_bp) {
      pthrd_printf("HandleBreakpointClear restoring BP.  Single stepping the process.\n");
      thrd->markClearingBreakpoint(ibp);
      thrd->setSingleStepMode(true);

      pthrd_printf("Making stop/continue decisions for handleBreakpointClear when stepping over BP\n");

      //Threads without breakpoint restores get set to stopped.  Threads resuming breakpoints
      // are set-to/left-at running.
      for (int_threadPool::iterator i = proc->threadPool()->begin(); i != proc->threadPool()->end(); i++) {
         if (hwbp && *i != hwbp->getThread()) {
            //Hardware breakpoints don't desync the whole process
            continue;
         }
         int_thread::StateTracker &st = (*i)->getBreakpointResumeState();
         if (st.getState() == int_thread::running || *i == thrd)
            st.desyncState(int_thread::running); //This thread is resuming, desync
         else if (int_bpc->stopped_proc)
            st.desyncState(int_thread::stopped); //Mark this as stopped as we restore the BP
      }
      if (!hwbp) {
         //If new threads come in during a BP clear, then they should be
         //created stopped.
         int bpr_stateid = int_thread::BreakpointResumeStateID;
         proc->getProcDesyncdStates()[bpr_stateid]++;
      }
   }
   else {
      pthrd_printf("HandleBreakpointClear will not restore BP.  Restoring process state.\n");
   }
   
   if (int_bpc->stopped_proc)
      thrd->getBreakpointState().restoreStateProc();
   else
      thrd->getBreakpointState().restoreState();

   return Handler::ret_success;
}


HandleBreakpointRestore::HandleBreakpointRestore() :
   Handler("Breakpoint Restore")
{
}

HandleBreakpointRestore::~HandleBreakpointRestore()
{
}

void HandleBreakpointRestore::getEventTypesHandled(vector<EventType> &etypes)
{
   etypes.push_back(EventType(EventType::None, EventType::BreakpointRestore));
}

Handler::handler_ret_t HandleBreakpointRestore::handleEvent(Event::ptr ev)
{
   int_process *proc = ev->getProcess()->llproc();
   int_thread *thrd = ev->getThread()->llthrd();
   EventBreakpointRestore *bpc = static_cast<EventBreakpointRestore *>(ev.get());
   int_eventBreakpointRestore *int_bpc = bpc->getInternal();
   bp_instance *bp = int_bpc->bp;
   sw_breakpoint *swbp = bp->swBP();
   bool result;   
   
   if (!int_bpc->set_states) {
      pthrd_printf("Restoring breakpoint at %lx for %d/%d\n", bp->getAddr(), proc->getPid(), thrd->getLWP());
      thrd->markClearingBreakpoint(NULL);
      thrd->setSingleStepMode(false);
      thrd->getBreakpointResumeState().setState(int_thread::stopped);
      int_bpc->set_states = true;
   }
   
   //Time to restore this bp (maybe, the BP will actually resume when it's last thread is done restoring)
   pthrd_printf("Restoring breakpoint to process\n");
   if (!int_bpc->bp_resume_started) {
      int_bpc->bp_resume_started = true;
      result = bp->resume(proc, int_bpc->bp_resume);
      if (!result) {
         pthrd_printf("Error resuming breakpoint in handler\n");
         return ret_error;
      }
   }
   for (set<response::ptr>::iterator i = int_bpc->bp_resume.begin(); i != int_bpc->bp_resume.end();) {
      response::ptr resp = *i;
      if (!resp || !resp->isPosted() || resp->isReady())
         int_bpc->bp_resume.erase(i++);
      else
         i++;
   }
   if (!int_bpc->bp_resume.empty()) {
      pthrd_printf("Postponing breakpoint clear while waiting for memwrite\n");
      proc->handlerPool()->notifyOfPendingAsyncs(int_bpc->bp_resume, ev);
      return ret_async;
   }

   pthrd_printf("Restoring thread state after breakpoint restore\n");
   if (swbp)
      thrd->getBreakpointResumeState().restoreStateProc();
   else
      thrd->getBreakpointResumeState().restoreState();

   return ret_success;
}

HandleEmulatedSingleStep::HandleEmulatedSingleStep() :
   Handler("Emulated Single Step")
{
}

HandleEmulatedSingleStep::~HandleEmulatedSingleStep()
{
}


void HandleEmulatedSingleStep::getEventTypesHandled(vector<EventType> &etypes)
{
   etypes.push_back(EventType(EventType::None, EventType::Breakpoint));
}

Handler::handler_ret_t HandleEmulatedSingleStep::handleEvent(Event::ptr ev)
{
   int_process *proc = ev->getProcess()->llproc();
   int_thread *thrd = ev->getThread()->llthrd();

   emulated_singlestep *em_singlestep = thrd->getEmulatedSingleStep();
   if (!em_singlestep)
      return ret_success;
   
   EventBreakpoint::ptr ev_bp = ev->getEventBreakpoint();
   Address addr = ev_bp->getInternal()->addr;
   assert(ev_bp);
   if (!em_singlestep->containsBreakpoint(addr))
      return ret_success;

   pthrd_printf("Breakpoint %lx corresponds to emulated single step.  Clearing BPs\n", addr);
   async_ret_t aresult = em_singlestep->clear();
   if (aresult == aret_async) {
      proc->handlerPool()->notifyOfPendingAsyncs(em_singlestep->clear_resps, ev);
      return ret_async;
   }

   EventSingleStep::ptr ev_ss = EventSingleStep::ptr(new EventSingleStep());
   ev_ss->setProcess(proc->proc());
   ev_ss->setThread(thrd->thread());
   ev_ss->setSyncType(ev->getSyncType());
   proc->handlerPool()->addLateEvent(ev_ss);

   em_singlestep->restoreSSMode();
   thrd->rmEmulatedSingleStep(em_singlestep);
   
   return ret_success;
}

int HandleEmulatedSingleStep::getPriority() const
{
   return PostPlatformPriority;
}

HandleLibrary::HandleLibrary() :
   Handler("Library Handler")
{
}

HandleLibrary::~HandleLibrary()
{
}

Handler::handler_ret_t HandleLibrary::handleEvent(Event::ptr ev)
{
   pthrd_printf("Handling library load/unload\n");
   EventLibrary *lev = static_cast<EventLibrary *>(ev.get());
   if(!lev->libsAdded().empty())
   {
	   MTLock lock_this_block;
	   // this happens on Windows, where we already did the decode when we got the event
	   for(std::set<Library::ptr>::const_iterator lib = lev->libsAdded().begin();
		   lib != lev->libsAdded().end();
		   ++lib)
	   {
              ev->getProcess()->llproc()->memory()->addLibrary((*lib)->debug());
	   }
   }
   if(!lev->libsRemoved().empty())
   {
	   MTLock lock_this_block;
	   for(std::set<Library::ptr>::const_iterator lib = lev->libsRemoved().begin();
		   lib != lev->libsRemoved().end();
		   ++lib)
	   {
              ev->getProcess()->llproc()->memory()->rmLibrary((*lib)->debug());
	   }
   }
   if(!lev->libsAdded().empty() || !lev->libsRemoved().empty())
	   return ret_success;

   int_process *proc = ev->getProcess()->llproc();
   set<int_library *> ll_added, ll_rmd;
   set<response::ptr> async_responses;
   bool async_pending = false;
   bool result = proc->refresh_libraries(ll_added, ll_rmd, async_pending, async_responses);
   if (!result && async_pending) {
      proc->handlerPool()->notifyOfPendingAsyncs(async_responses, ev);
      return ret_async;
   }
   if (!result) {
      pthrd_printf("Failed to refresh library list\n");
      return ret_error;
   }
   if (ll_added.empty() && ll_rmd.empty()) {
      pthrd_printf("Could not find actual changes in lib state\n");
      ev->setSuppressCB(true);
      return ret_success;
   }

   set<Library::ptr> added, rmd;
   for (set<int_library*>::iterator i = ll_added.begin(); i != ll_added.end(); i++) {
      added.insert((*i)->getUpPtr());
   }
   for (set<int_library*>::iterator i = ll_rmd.begin(); i != ll_rmd.end(); i++) {
      rmd.insert((*i)->getUpPtr());
   }
   for (set<int_library*>::iterator i = ll_rmd.begin(); i != ll_rmd.end(); i++) {
      (*i)->markAsCleanable();
   }
   lev->setLibs(added, rmd);
   return ret_success;
}

void HandleLibrary::getEventTypesHandled(std::vector<EventType> &etypes)
{
   etypes.push_back(EventType(EventType::None, EventType::Library));
}

HandleDetach::HandleDetach() :
   Handler("Detach")
{
}

HandleDetach::~HandleDetach()
{
}
   
Handler::handler_ret_t HandleDetach::handleEvent(Event::ptr ev)
{
   int_process *proc = ev->getProcess()->llproc();
   EventDetach::ptr detach_ev = ev->getEventDetach();
   int_eventDetach *int_detach_ev = detach_ev->getInternal();
   bool temporary = int_detach_ev->temporary_detach;
   bool leaveStopped = int_detach_ev->leave_stopped;
   bool &removed_bps = int_detach_ev->removed_bps;
   result_response::ptr &detach_response = int_detach_ev->detach_response;
   std::set<response::ptr> &async_responses = int_detach_ev->async_responses;
   mem_state::ptr mem = proc->memory();
   bool err = true;

   if (!removed_bps) 
   {
      if (!temporary) {
         while (!mem->breakpoints.empty())
         {
            std::map<Dyninst::Address, sw_breakpoint *>::iterator i = mem->breakpoints.begin();
            bool result = i->second->uninstall(proc, async_responses);
            if (!result) {
               perr_printf("Error removing breakpoint at %lx\n", i->first);
               ev->setLastError(err_internal, "Error removing breakpoint before detach\n");
               goto done;
            }
         }
      }
      else {
         for(std::map<Dyninst::Address, sw_breakpoint *>::iterator i = mem->breakpoints.begin();
             i != mem->breakpoints.end(); ++i)
         {
            bool result = i->second->suspend(proc, async_responses);
            if(!result) {
               perr_printf("Error suspending breakpoint at %lx\n", i->first);
               ev->setLastError(err_internal, "Error suspending breakpoint before detach\n");
               goto done;
            }
         }
      }
      removed_bps = true;
   }

   for (set<response::ptr>::iterator i = async_responses.begin(); i != async_responses.end(); i++) {
      if ((*i)->hasError()) {
         perr_printf("Failed to remove breakpoints\n");
         ev->setLastError(err_internal, "Error removing breakpoint before detach\n");
         goto done;
      }
      if (!(*i)->isReady()) {
         pthrd_printf("Suspending detach due to async in breakpoint removal.\n");
         proc->handlerPool()->notifyOfPendingAsyncs(async_responses, ev);
         return ret_async;
      }
   }

   if (!detach_response) {
      pthrd_printf("Detach handler is triggering platform detach\n");
      detach_response = result_response::createResultResponse();
      bool result = proc->plat_detach(detach_response, leaveStopped);
      if (!result) {
         pthrd_printf("Error performing platform detach on %d\n", proc->getPid());
         goto done;
      }
   }
   if (detach_response->isPosted() && !detach_response->isReady()) {
      pthrd_printf("Suspending detach operation while waiting for detach response\n");
      proc->handlerPool()->notifyOfPendingAsyncs(detach_response, ev);
      return ret_async;
   }
      
   if (!proc->plat_detachDone()) {
      //Currently used on BG/Q.  Its plat_detach is a multi-stage operation,
      // and will cause itself to be reinvoked until plat_detachDone is finished
      pthrd_printf("Not finishing detach because plat_detachDone reported false on %d\n",
                   proc->getPid());
      return ret_success;
   }

   if (temporary) {
      proc->setState(int_process::detached);
      proc->threadPool()->initialThread()->getDetachState().setStateProc(int_thread::detached);
   }
   else {
      ProcPool()->condvar()->lock();

      proc->setState(int_process::exited);
      ProcPool()->rmProcess(proc);

      ProcPool()->condvar()->broadcast();
      ProcPool()->condvar()->unlock();
   }

   err = false;
  done:
   int_detach_ev->done = true;
   proc->getStartupTeardownProcs().dec();
   return err ? ret_error : ret_success;
}
 
void HandleDetach::getEventTypesHandled(std::vector<EventType> &etypes)
{
   etypes.push_back(EventType(EventType::None, EventType::Detach));
}

HandleAsync::HandleAsync() :
   Handler("Async")
{
}

HandleAsync::~HandleAsync()
{
}
   
Handler::handler_ret_t HandleAsync::handleEvent(Event::ptr ev)
{
   EventAsync::ptr eAsync = ev->getEventAsync();
   set<response::ptr> &resps = eAsync->getInternal()->getResponses();

   pthrd_printf("Handling %lu async event(s) on %d/%d\n", 
                (unsigned long) resps.size(),
                eAsync->getProcess()->llproc()->getPid(),
                eAsync->getThread()->llthrd()->getLWP());

   assert(eAsync->getProcess()->llproc()->plat_needsAsyncIO());
   for (set<response::ptr>::iterator i = resps.begin(); i != resps.end(); i++) {
      response::ptr resp = *i;
      resp->markReady();
      resp->setEvent(Event::ptr());
   }

   return ret_success;
}

void HandleAsync::getEventTypesHandled(std::vector<EventType> &etypes)
{
   etypes.push_back(EventType(EventType::None, EventType::Async));
}

HandleAsyncIO::HandleAsyncIO() :
   Handler("AsyncIO Handler")
{
}

HandleAsyncIO::~HandleAsyncIO()
{
}
   
Handler::handler_ret_t HandleAsyncIO::handleEvent(Event::ptr ev)
{
   pthrd_printf("In AsyncIO Handler for %d\n", ev->getProcess()->getPid());
   EventAsyncIO::ptr evio = ev->getEventAsyncIO();
   assert(evio);
   int_eventAsyncIO *iev = evio->getInternalEvent();
   pthrd_printf("Dealing with int_eventAsyncIO %p\n", (void*)iev);
   assert(iev);
   assert(iev->resp);
   (void)iev->resp->isReady();
   return ret_success;
}

void HandleAsyncIO::getEventTypesHandled(std::vector<EventType> &etypes)
{
   etypes.push_back(EventType(EventType::None, EventType::AsyncRead));
   etypes.push_back(EventType(EventType::None, EventType::AsyncWrite));
   etypes.push_back(EventType(EventType::None, EventType::AsyncReadAllRegs));
   etypes.push_back(EventType(EventType::None, EventType::AsyncSetAllRegs));
}

HandleAsyncFileRead::HandleAsyncFileRead() :
   Handler("HandleAsyncFileRead")
{
}

HandleAsyncFileRead::~HandleAsyncFileRead()
{
}

Handler::handler_ret_t HandleAsyncFileRead::handleEvent(Event::ptr ev)
{
   EventAsyncFileRead::ptr fileev = ev->getEventAsyncFileRead();
   assert(fileev);
   int_eventAsyncFileRead *iev = fileev->getInternal();
   int_process *proc = ev->getProcess()->llproc();
   
   if (iev->resp)
      delete iev->resp;

   if (iev->isComplete())
      return ret_success;

   //Setup a read on the next part of the file, starting at the offset
   // after this read ends.
   int_eventAsyncFileRead *new_iev = new int_eventAsyncFileRead();
   new_iev->filename = iev->filename;
   new_iev->offset = iev->offset + iev->size;
   new_iev->whole_file = iev->whole_file;
   bool result = proc->getRemoteIO()->plat_getFileDataAsync(new_iev);
   if (!result) {
      pthrd_printf("Error requesting file data on %d from callback\n", proc->getPid());
      return ret_error;
   }
   return ret_success;
}

void HandleAsyncFileRead::getEventTypesHandled(std::vector<EventType> &etypes)
{
   etypes.push_back(EventType(EventType::None, EventType::AsyncFileRead));
}

HandleNop::HandleNop() :
   Handler("Nop Handler")
{
}

HandleNop::~HandleNop()
{
}

Handler::handler_ret_t HandleNop::handleEvent(Event::ptr)
{
   //Trivial handler, by definition
   return ret_success;
}

void HandleNop::getEventTypesHandled(std::vector<EventType> &etypes)
{
   etypes.push_back(EventType(EventType::None, EventType::Nop));
}

HandleCallbacks::HandleCallbacks() : 
   Handler("Callback")
{
}

HandleCallbacks::~HandleCallbacks()
{
}

HandleCallbacks *HandleCallbacks::getCB()
{
   static HandleCallbacks *cb = NULL;
   if (!cb) {
      cb = new HandleCallbacks();
      assert(cb);
      HandlerPool *hp = createDefaultHandlerPool(NULL);
      delete hp;
   }
   return cb;
}

int HandleCallbacks::getPriority() const
{
   return CallbackPriority;
}

void HandleCallbacks::getEventTypesHandled(std::vector<EventType> & /*etypes*/)
{
   //Callbacks are special cased, they respond to all event types.
}

bool HandleCallbacks::hasCBs(Event::const_ptr ev)
{
   return cbfuncs.find(ev->getEventType()) != cbfuncs.end();
}

bool HandleCallbacks::hasCBs(EventType et)
{
   return cbfuncs.find(et) != cbfuncs.end();
}

bool HandleCallbacks::requiresCB(Event::const_ptr ev)
{
   return hasCBs(ev) && !ev->suppressCB();
}

Handler::handler_ret_t HandleCallbacks::handleEvent(Event::ptr ev)
{
   int_thread *thr = NULL;
   if(ev->getThread()) thr = ev->getThread()->llthrd();
   int_process *proc = ev->getProcess()->llproc();
   
   if (ev->noted_event) {
      //Reset the event status here if the callback already had a delivery attempt
      notify()->clearEvent();
      if (thr)
         thr->getCallbackState().restoreStateProc();
      else
         proc->threadPool()->initialThread()->getCallbackState().restoreStateProc();
      ev->noted_event = false;
   }

   EventType evtype = ev->getEventType();
   std::map<EventType, std::set<Process::cb_func_t>, eventtype_cmp>::iterator i = cbfuncs.find(evtype);
   if (i == cbfuncs.end()) {
      pthrd_printf("No callback registered for event type '%s'\n", ev->name().c_str());

      if (proc->wasForcedTerminated() && thr) {
         thr->getUserState().setState(int_thread::running);
      }

      return ret_success;
   }

   if (proc &&
       (proc->getState() == int_process::neonatal ||
        proc->getState() == int_process::neonatal_intermediate))
   {
      pthrd_printf("No callback for neonatal process %d\n", proc->getPid());
      return ret_success;
   }
   const std::set<Process::cb_func_t> &cbs = i->second;

   if (ev->suppressCB()) {
      pthrd_printf("Suppressing callbacks for event %s\n", ev->name().c_str());

      if (proc->wasForcedTerminated() && thr) {
         thr->getUserState().setState(int_thread::running);
      }

      return ret_success;
   }

   if (proc->isRunningSilent()) {
      pthrd_printf("Suppressing callback for event %s due to process being in silent mode\n",
                   ev->name().c_str());

      if (proc->wasForcedTerminated() && thr) {
         thr->getUserState().setState(int_thread::running);
      }

      return ret_success;
   }
   
   return deliverCallback(ev, cbs);
}

static const char *action_str(Process::cb_action_t action)
{
   switch (action) {
      case Process::cbThreadContinue:
         return "cbThreadContinue";
      case Process::cbThreadStop:
         return "cbThreadStop";
      case Process::cbProcContinue:
         return "cbProcContinue";
      case Process::cbProcStop:
         return "cbProcStop";
      case Process::cbDefault:
         return "cbDefault";
      default:
         return "cbInvalid";
   }
   return NULL;
}

bool HandleCallbacks::handleCBReturn(Process::const_ptr proc, Thread::const_ptr thrd, 
                                     Process::cb_action_t ret)
{
   if (!thrd) {
      thrd = proc->llproc()->threadPool()->initialThread()->thread();
   }
   switch (ret) {
      case Process::cbThreadContinue:
		  if (!thrd->llthrd()->isUser()) break;
		  if (thrd == Thread::const_ptr()) {
            perr_printf("User returned invalid action %s for event\n", 
                        action_str(ret));
            return false;
         }
         pthrd_printf("Callbacks returned thread continue\n");
         thrd->llthrd()->getUserState().setState(int_thread::running);
         break;
      case Process::cbThreadStop:
		  if (!thrd->llthrd()->isUser()) break;
         if (thrd == Thread::const_ptr()) {
            perr_printf("User returned invalid action %s for event\n", 
                        action_str(ret));
            return false;
         }
         pthrd_printf("Callbacks returned thread stop\n");
         thrd->llthrd()->getUserState().setState(int_thread::stopped);
         break;
      case Process::cbProcContinue: {
         if (proc == Process::const_ptr()) {
            perr_printf("User returned invalid action %s for event\n", 
                        action_str(ret));
            return false;
         }
         pthrd_printf("Callbacks returned process continue\n");
         thrd->llthrd()->getUserState().setStateProc(int_thread::running);
         break;
      }
      case Process::cbProcStop: {
         if (proc == Process::const_ptr()) {
            perr_printf("User returned invalid action %s for event\n", 
                        action_str(ret));
            return false;
         }
         pthrd_printf("Callbacks returned process stop\n");
         thrd->llthrd()->getUserState().setStateProc(int_thread::stopped);
         break;
      }
      case Process::cbDefault:
         pthrd_printf("Callbacks returned default\n");
         break;
   }
   return true;
}

Handler::handler_ret_t HandleCallbacks::deliverCallback(Event::ptr ev, const set<Process::cb_func_t> &cbset)
{
   //We want the thread to remain in its appropriate state while the CB is in flight.
	int_thread *thr = ev->getThread() ? ev->getThread()->llthrd() : NULL;
   int_process *proc = ev->getProcess()->llproc();
   assert(proc);

   pthrd_printf("Changing callback state of %d before CB\n", proc->getPid());
   int_thread::StateTracker &cb_state = thr ? thr->getCallbackState() : proc->threadPool()->initialThread()->getCallbackState();
   cb_state.desyncStateProc(int_thread::ditto);

   if (isHandlerThread() && mt()->getThreadMode() != Process::CallbackThreading) {
      //We're not going to allow a handler thread to deliver a callback in this mode
      // trigger the notify, then return a request to delay from the delivery of this
      // callback to the user thread.
      ev->noted_event = true;
      notify()->noteEvent();
      return ret_cbdelay;
   }

   // Make sure the user can operate on the process in the callback
   // But if this is a PostCrash or PostExit, the underlying process and
   // threads are already gone and thus no operations on them really make sense
   if (!ev->getProcess()->isTerminated()) {
      ev->getProcess()->llproc()->threadPool()->saveUserState(ev);
   }

   //The following code loops over each callback registered for this event type
   // and triggers the callback for the user.  Return results are aggregated.
   unsigned k = 0;
   pthrd_printf("Triggering callback for event '%s'\n", ev->name().c_str());
   Process::cb_action_t parent_result = Process::cbDefault;
   Process::cb_action_t child_result = Process::cbDefault;
   std::set<Process::cb_func_t>::const_iterator j;
   for (j = cbset.begin(); j != cbset.end(); j++, k++) {
      pthrd_printf("Triggering callback #%u for event '%s'\n", k, ev->name().c_str());
      int_process::setInCB(true);
      Process::cb_ret_t ret = (*j)(ev);
      int_process::setInCB(false);

      if (ret.parent != Process::cbDefault)
         parent_result = ret.parent;
      if (ret.child != Process::cbDefault)
         child_result = ret.child;

      pthrd_printf("Callback #%u return %s/%s\n", k, action_str(ret.parent),
                   action_str(ret.child));
   }

   // Don't allow the user to change the state of forced terminated processes 
   if( ev->getProcess()->llproc() && ev->getProcess()->llproc()->wasForcedTerminated() ) {
      pthrd_printf("Process is in forced termination, overriding result to cbThreadContinue\n");
      parent_result = Process::cbThreadContinue;
      child_result = Process::cbDefault;
   }

   // Now that the callback is over, return the state to what it was before the
   // callback so the return value from the callback can be used to update the state
   if (!ev->getProcess()->isTerminated()) {
      ev->getProcess()->llproc()->threadPool()->restoreUserState();
      
      if (ev->getThread()->llthrd() != NULL) {
         //Given the callback return result, change the user state to the appropriate
         // setting.
         pthrd_printf("Handling return value for main process\n");
         handleCBReturn(ev->getProcess(), ev->getThread(), parent_result);
      }
   }

   pthrd_printf("Handling return value for child process/thread\n");
   Process::const_ptr child_proc = Process::const_ptr();
   Thread::const_ptr child_thread = Thread::const_ptr();
   bool event_has_child = false;
   switch (ev->getEventType().code()) {
      case EventType::Fork:
         event_has_child = true;
         child_proc = static_cast<EventFork *>(ev.get())->getChildProcess();
         break;
      case EventType::ThreadCreate:
      case EventType::UserThreadCreate:
      case EventType::LWPCreate:
         event_has_child = true;
         child_thread = static_cast<EventNewThread *>(ev.get())->getNewThread();
         break;
   }
   if (event_has_child)
      handleCBReturn(child_proc, child_thread, child_result);

   pthrd_printf("Restoring callback state of %d/%d after CB\n", ev->getProcess()->getPid(), 
                ev->getThread()->getLWP());
   cb_state.restoreStateProc();

   return ret_success;
}

void HandleCallbacks::getRealEvents(EventType ev, std::vector<EventType> &out_evs)
{
   switch (ev.code()) {
      case EventType::Terminate:
         out_evs.push_back(EventType(ev.time(), EventType::Exit));
         out_evs.push_back(EventType(ev.time(), EventType::Crash));
         out_evs.push_back(EventType(ev.time(), EventType::ForceTerminate));
         break;
      case EventType::ThreadCreate:
         out_evs.push_back(EventType(ev.time(), EventType::UserThreadCreate));
         out_evs.push_back(EventType(ev.time(), EventType::LWPCreate));
         break;
      case EventType::ThreadDestroy:
         out_evs.push_back(EventType(ev.time(), EventType::UserThreadDestroy));
         out_evs.push_back(EventType(ev.time(), EventType::LWPDestroy));
         break;
      default:
         out_evs.push_back(ev);
   }
}

bool HandleCallbacks::registerCallback_int(EventType ev, Process::cb_func_t func)
{
   pthrd_printf("Registering event %s with callback function %p\n", ev.name().c_str(), (void*)func);
   std::set<EventType, Dyninst::ProcControlAPI::eventtype_cmp>::iterator i = alleventtypes.find(ev);
   if (i == alleventtypes.end()) {
      pthrd_printf("Event %s does not have any handler\n", ev.name().c_str());
      return false;
   }
   cbfuncs[ev].insert(func);
   return true;
}

bool HandleCallbacks::registerCallback(EventType oev, Process::cb_func_t func)
{
   bool registered_cb = false;
   std::vector<EventType> real_evs;
   getRealEvents(oev, real_evs);
   
   for (std::vector<EventType>::iterator i = real_evs.begin(); i != real_evs.end(); i++)
   {
      EventType ev = *i;
      switch (ev.time()) {
         case EventType::Pre:
         case EventType::Post:
         case EventType::None: {
            bool result = registerCallback_int(ev, func);
            if (result)
               registered_cb = true;
            break;
         }
         case EventType::Any: {
            bool result1 = registerCallback_int(EventType(EventType::Pre, ev.code()), func);
            bool result2 = registerCallback_int(EventType(EventType::Post, ev.code()), func);
            bool result3 = registerCallback_int(EventType(EventType::None, ev.code()), func);
            if (result1 || result2 || result3)
               registered_cb = true;
            break;
         }
      }
   }
   if (!registered_cb) {
      pthrd_printf("Did not register any callbacks for %s\n", oev.name().c_str());
      ProcControlAPI::globalSetLastError(err_noevents, "EventType does not exist");
      return false;
   }
   return true;
}

bool HandleCallbacks::removeCallback_int(EventType et, Process::cb_func_t func)
{
   cbfuncs_t::iterator i = cbfuncs.find(et);
   if (i == cbfuncs.end()) {
      return false;
   }
   set<Process::cb_func_t> &func_set = i->second;
   set<Process::cb_func_t>::iterator j = func_set.find(func);
   if (j == func_set.end()) {
      return false;
   }
   func_set.erase(j);
   return true;
}

bool HandleCallbacks::removeCallback(EventType oet, Process::cb_func_t func)
{
   bool removed_cb = false;
   std::vector<EventType> real_ets;
   getRealEvents(oet, real_ets);
   pthrd_printf("Removing event %s callback with function %p\n", oet.name().c_str(), (void*)func);
   
   for (std::vector<EventType>::iterator i = real_ets.begin(); i != real_ets.end(); i++)
   {
      EventType et = *i;

      switch (et.time()) {
         case EventType::Pre:
         case EventType::Post:
         case EventType::None: {
            bool result = removeCallback_int(et, func);
            if (result)
               removed_cb = true;
            break;
         }
         case EventType::Any: {
            bool result1 = removeCallback_int(EventType(EventType::Pre, et.code()), func);
            bool result2 = removeCallback_int(EventType(EventType::Post,et.code()), func);
            bool result3 = removeCallback_int(EventType(EventType::None,et.code()), func);
            if (result1 || result2 || result3)
               removed_cb = true;
            break;
         }
      }
   }

   if (!removed_cb) {
      perr_printf("Attempted to remove non-existant callback %s\n", 
                  oet.name().c_str());
      ProcControlAPI::globalSetLastError(err_badparam, "Callback does not exist");
      return false;
   }
   return true;
}

bool HandleCallbacks::removeCallback_int(EventType et)
{
   cbfuncs_t::iterator i = cbfuncs.find(et);
   if (i == cbfuncs.end()) {
      return false;
   }
   cbfuncs.erase(i);
   return true;
}

bool HandleCallbacks::removeCallback(EventType et)
{
   bool result = false;
   switch (et.time()) {
      case EventType::Pre:
      case EventType::Post:
      case EventType::None: {
         result = removeCallback_int(et);
         break;
      }
      case EventType::Any: {
         bool result1 = removeCallback_int(EventType(EventType::Pre, et.code()));
         bool result2 = removeCallback_int(EventType(EventType::Post,et.code()));
         bool result3 = removeCallback_int(EventType(EventType::None,et.code()));
         result = (result1 || result2 || result3);
      }
   }
   if (!result) {
      perr_printf("Attempted to remove non-existant callback %s\n", 
                  et.name().c_str());
      ProcControlAPI::globalSetLastError(err_badparam, "Callback does not exist");
      return false;
   }
   return true;
}

bool HandleCallbacks::removeCallback(Process::cb_func_t func)
{
   bool rmd_something = false;
   for (cbfuncs_t::iterator i = cbfuncs.begin(); i != cbfuncs.end(); i++)
   {
      EventType et = i->first;
      bool result = removeCallback_int(et, func);
      if (result) 
         rmd_something = true;
   }
   if (!rmd_something) {
      perr_printf("Attempted to remove non-existant callback %p\n", (void*)func);
      ProcControlAPI::globalSetLastError(err_badparam, "Callback does not exist");
      return false;
   }
   return true;
}

HandlePostponedSyscall::HandlePostponedSyscall() :
   Handler("Postponed Syscall")
{
}

HandlePostponedSyscall::~HandlePostponedSyscall()
{
}

void HandlePostponedSyscall::getEventTypesHandled(vector<EventType> &etypes)
{
   etypes.push_back(EventType(EventType::None, EventType::PostponedSyscall));
}

Handler::handler_ret_t HandlePostponedSyscall::handleEvent(Event::ptr ev)
{
   int_thread *thrd = ev->getThread()->llthrd();
   thrd->getPostponedSyscallState().desyncState(int_thread::running);
   return ret_success;
}

HandlerPool *createDefaultHandlerPool(int_process *p)
{
   static bool initialized = false;
   static HandleBootstrap *hbootstrap = NULL;
   static HandleSignal *hsignal = NULL;
   static HandlePostExit *hpostexit = NULL;
   static HandlePreExit *hpreexit = NULL;
   static HandlePostExitCleanup *hpostexitcleanup = NULL;
   static HandleThreadCreate *hthreadcreate = NULL;
   static HandleThreadDestroy *hthreaddestroy = NULL;
   static HandleThreadCleanup *hthreadcleanup = NULL;
   static HandleThreadStop *hthreadstop = NULL;
   static HandleSingleStep *hsinglestep = NULL;
   static HandlePreSyscall *hpresyscall = NULL;
   static HandlePostSyscall *hpostsyscall = NULL;
   static HandleCrash *hcrash = NULL;
   static HandleBreakpoint *hbpoint = NULL;
   static HandleBreakpointContinue *hbpcontinue = NULL;
   static HandleBreakpointClear *hbpclear = NULL;
   static HandleBreakpointRestore *hbprestore = NULL;
   static HandleLibrary *hlibrary = NULL;
   static HandlePostFork *hpostfork = NULL;
   static HandlePostForkCont *hpostforkcont = NULL;
   static HandlePostExec *hpostexec = NULL;
   static HandleAsync *hasync = NULL;
   static HandleAsyncIO *hasyncio = NULL;
   static HandleForceTerminate *hforceterm = NULL;
   static HandleNop *hnop = NULL;
   static HandleDetach *hdetach = NULL;
   static HandleEmulatedSingleStep *hemulatedsinglestep = NULL;
   static iRPCHandler *hrpc = NULL;
   static iRPCPreCallbackHandler *hprerpc = NULL;
   static HandlePreBootstrap* hprebootstrap = NULL;
   static iRPCLaunchHandler *hrpclaunch = NULL;
   static HandleAsyncFileRead *hasyncfileread = NULL;
   static HandlePostponedSyscall *hppsyscall = NULL;
   if (!initialized) {
      hbootstrap = new HandleBootstrap();
      hsignal = new HandleSignal();
      hpostexit = new HandlePostExit();
      hpostexitcleanup = new HandlePostExitCleanup();
      hpreexit = new HandlePreExit();
      hthreadcreate = new HandleThreadCreate();
      hthreaddestroy = new HandleThreadDestroy();
      hthreadcleanup = new HandleThreadCleanup();
      hthreadstop = new HandleThreadStop();
      hsinglestep = new HandleSingleStep();
      hpresyscall = new HandlePreSyscall();
      hpostsyscall = new HandlePostSyscall();
      hcrash = new HandleCrash();
      hbpoint = new HandleBreakpoint();
      hbpcontinue = new HandleBreakpointContinue();
      hbpclear = new HandleBreakpointClear();
      hbprestore = new HandleBreakpointRestore();
      hrpc = new iRPCHandler();
      hprerpc = new iRPCPreCallbackHandler();
      hrpclaunch = new iRPCLaunchHandler();
      hlibrary = new HandleLibrary();
      hpostfork = new HandlePostFork();
      hpostforkcont = new HandlePostForkCont();
      hpostexec = new HandlePostExec();
      hasync = new HandleAsync();
      hasyncio = new HandleAsyncIO();
      hforceterm = new HandleForceTerminate();
      hprebootstrap = new HandlePreBootstrap();
      hnop = new HandleNop();
      hdetach = new HandleDetach();
      hemulatedsinglestep = new HandleEmulatedSingleStep();
      hasyncfileread = new HandleAsyncFileRead();
      hppsyscall = new HandlePostponedSyscall();
      initialized = true;
   }
   HandlerPool *hpool = new HandlerPool(p);
   hpool->addHandler(hbootstrap);
   hpool->addHandler(hsignal);
   hpool->addHandler(hpostexit);
   hpool->addHandler(hpostexitcleanup);
   hpool->addHandler(hpreexit);
   hpool->addHandler(hthreadcreate);
   hpool->addHandler(hthreaddestroy);
   hpool->addHandler(hthreadcleanup);
   hpool->addHandler(hthreadstop);
   hpool->addHandler(hsinglestep);
   hpool->addHandler(hpresyscall);
   hpool->addHandler(hpostsyscall);
   hpool->addHandler(hcrash);
   hpool->addHandler(hbpoint);
   hpool->addHandler(hbpcontinue);
   hpool->addHandler(hbpclear);
   hpool->addHandler(hbprestore);
   hpool->addHandler(hrpc);
   hpool->addHandler(hprerpc);
   hpool->addHandler(hlibrary);
   hpool->addHandler(hpostfork);
   hpool->addHandler(hpostforkcont);
   hpool->addHandler(hpostexec);
   hpool->addHandler(hrpclaunch);
   hpool->addHandler(hasync);
   hpool->addHandler(hasyncio);
   hpool->addHandler(hforceterm);
   hpool->addHandler(hprebootstrap);
   hpool->addHandler(hnop);
   hpool->addHandler(hdetach);
   hpool->addHandler(hemulatedsinglestep);
   hpool->addHandler(hasyncfileread);
   hpool->addHandler(hppsyscall);
   plat_createDefaultHandlerPool(hpool);

   print_add_handler = false;
   return hpool;
}
