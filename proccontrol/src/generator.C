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
#include "proccontrol/h/Generator.h"
#include "proccontrol/h/Event.h"
#include "proccontrol/h/Mailbox.h"
#include "proccontrol/h/Process.h"
#include "proccontrol/src/int_process.h"
#include "proccontrol/src/procpool.h"
#include "proccontrol/src/windows_thread.h"
#include "proccontrol/src/windows_process.h"
#include "proccontrol/src/GeneratorWindows.h"

#include "common/h/dthread.h"

#include <assert.h>

using namespace std;

std::set<Generator::gen_cb_func_t> Generator::CBs;
Mutex *Generator::cb_lock;

/*
 * Library deinitialization
 *
 * Note: it is crucial that this variable is located here because it guarantees
 * that the threads will be stopped before destructing the CBs collection and
 * therefore avoiding a problem where the generator will continue to run but
 * the CBs collection has already been destructed
 */
static int_cleanup cleanup;

int_cleanup::~int_cleanup() {
    // First, stop the handler thread if necessary
    MTManager *tmpMt = mt();
    if( tmpMt ) tmpMt->stop();

    // Second, stop the generator
    Generator::stopDefaultGenerator();
}

Generator::Generator(std::string name_) :
   name(name_)
{
   if (!cb_lock) cb_lock = new Mutex();
}

Generator::~Generator()
{
   setState(exiting);
}

void Generator::registerNewEventCB(void (*func)())
{
   if (!cb_lock) cb_lock = new Mutex();
   Generator::cb_lock->lock();
   CBs.insert(func);
   Generator::cb_lock->unlock();
}

void Generator::removeNewEventCB(void (*func)())
{
   if (!cb_lock) cb_lock = new Mutex();
   Generator::cb_lock->lock();
   std::set<gen_cb_func_t>::iterator i = CBs.find(func);
   if (i != CBs.end())
      CBs.erase(i);
   Generator::cb_lock->unlock();
}

bool Generator::isExitingState()
{
   return (state == error || state == exiting);
}


void Generator::setState(Generator::state_t new_state)
{
	pthrd_printf("Setting generator state: %d\n", new_state);
   if (isExitingState())
      return;
   state = new_state;
}

ArchEvent* Generator::getCachedEvent() 
{
	return m_Event;
}

void Generator::setCachedEvent(ArchEvent* ae)
{
	m_Event = ae;
}

bool Generator::getAndQueueEventInt(bool block)
{
   bool result = true;
   //static ArchEvent *arch_event = NULL;
   ArchEvent* arch_event = getCachedEvent();
   vector<Event::ptr> events;

   setState(process_blocked);
   result = processWait(block);
   if (isExitingState()) {
      pthrd_printf("Generator exiting after processWait\n");
      result = false;
      goto done;
   }
   if (!result) {
	   pthrd_printf("Generator exiting after processWait returned false\n");
	   result = false;
      goto done;
   }
   result = plat_continue(arch_event);
   if(!result) {
	   pthrd_printf("Generator exiting after plat_continue returned false\n");
	   result = false;
	   goto done;
   }
   setState(system_blocked);
   pthrd_printf("About to getEvent()\n");
   arch_event = getEvent(block);
   if (isExitingState()) {
      pthrd_printf("Generator exiting after getEvent\n");
      result = false;
      goto done;
   }
   if (!arch_event) {
      pthrd_printf("Error. Unable to recieve event\n");
      result = false;
      goto done;
   }

   setState(decoding);
   ProcPool()->condvar()->lock();
   for (decoder_set_t::iterator i = decoders.begin(); i != decoders.end(); ++i) {
      Decoder *decoder = *i;
      bool result = decoder->decode(arch_event, events);
      if (!result)
         break;
   }
   for (vector<Event::ptr>::iterator i = events.begin(); i != events.end(); ++i) {
      Event::ptr event = *i;
	  if(event) {
	      event->getProcess()->llproc()->updateSyncState(event, true);
			// Added 9NOV11 - Bernat
		    pthrd_printf("Process %d: setting thread suspend/resume and generator states after decode\n", event->getProcess()->llproc()->getPid());
			int_threadPool *tp = event->getProcess()->llproc()->threadPool(); assert(tp);
			for (int_threadPool::iterator iter = tp->begin(); iter != tp->end(); ++iter) {
				windows_thread *winthr = dynamic_cast<windows_thread *>(*iter); assert(winthr);
				assert(winthr->getGeneratorState() != int_thread::running);
				winthr->plat_setSuspendCount(1);
			}
			windows_process *winproc = dynamic_cast<windows_process *>(event->getProcess()->llproc());
			winproc->lowlevel_processSuspended();

		   GeneratorWindows *genwin = dynamic_cast<GeneratorWindows *>(this);
		   if (genwin->waiters[winproc->getPid()]) {
			   ::ResetEvent(genwin->waiters[winproc->getPid()]->gen_wait);
		   }
			// End added 9NOV11
	  }
   }


   setState(queueing);
   for (vector<Event::ptr>::iterator i = events.begin(); i != events.end(); ++i) {
      mbox()->enqueue(*i);
      Generator::cb_lock->lock();
      for (set<gen_cb_func_t>::iterator j = CBs.begin(); j != CBs.end(); ++j) {
         (*j)();
      }
      Generator::cb_lock->unlock(); 
   }

   ProcPool()->condvar()->unlock();

   result = true;
 done:
   setState(none);
   setCachedEvent(arch_event);
   return result;
}

bool Generator::allStopped(int_process *proc, void *)
{
	if(proc == NULL) {
		pthrd_printf("Process was NULL, treating as stopped\n");
		return true;
	}
   bool all_exited = true;
   int_threadPool *tp = proc->threadPool();
   if(!tp) {
	   pthrd_printf("Process had no threadpool, treating as stopped\n");
	   return true;
   }
   assert(tp);
   for (int_threadPool::iterator i = tp->begin(); i != tp->end(); ++i) {
	   // 9NOV11 - Bernat
	   // With our changes to the generatorState, this needs to check
	   // handlerState instead. 
      if ((*i)->getHandlerState() == int_thread::running ||
          (*i)->getHandlerState() == int_thread::neonatal_intermediate) 
      {
         pthrd_printf("Found running thread: %d/%d is %s\n", 
                      proc->getPid(), (*i)->getLWP(), 
                      int_thread::stateStr((*i)->getGeneratorState()));
         return false;
      }
      if ((*i)->getHandlerState() != int_thread::exited) {
         all_exited = false;
      }
   }
   // When we're attaching, the threadpool could be empty on Windows.
   // Don't assume that an empty threadpool means everything is exited...
   if(tp->empty()) {
	   pthrd_printf("empty threadpool for %d, treating as running\n",
		   proc->getPid());
		return false;
   }
   if (all_exited) {
      pthrd_printf("All threads are exited for %d, treating as stopped\n",
                   proc->getPid());
      return true;
   }
   if (proc->forceGeneratorBlock()) {
      pthrd_printf("Process %d is forcing generator input\n", proc->getPid());
      return false;
   }
   pthrd_printf("Checking for running process: %d is stopped\n", proc->getPid());
   return true;
}

bool Generator::hasLiveProc()
{
	pthrd_printf("begin hasLiveProc()\n");
   ProcessPool *procpool = ProcPool();
   bool result = !procpool->for_each(Generator::allStopped, NULL);
   pthrd_printf("end hasLiveProc(), return %s, procpool %s\n",
	   result ? "TRUE" : "FALSE",
	   procpool->numProcs() == 0 ? "EMPTY" : "NON-EMPTY");
   return result;
}

struct GeneratorMTInternals
{
   GeneratorMTInternals() {}

   //Start-up synchronization
   CondVar init_cond;

   DThread thrd;
};

static unsigned long WINAPI start_generator(void *g)
{
   GeneratorMT *gen = (GeneratorMT *) g;
   gen->start();
   return 0;
}

GeneratorMT::GeneratorMT(std::string name_) :
   Generator(name_)
{
   //Make sure these structures exist before any generators run.
   mbox();
   ProcPool();

   sync = new GeneratorMTInternals();
}

void GeneratorMT::lock()
{
   sync->init_cond.lock();
}
void GeneratorMT::unlock()
{
   sync->init_cond.unlock();
}
void GeneratorMT::launch()
{
   sync->init_cond.lock();
   state = initializing;
   sync->thrd.spawn(start_generator, this);
   while (state == initializing)
      sync->init_cond.wait();
   sync->init_cond.unlock();

   if (state == error) {
      pthrd_printf("Error creating generator\n");
   }
}

GeneratorMT::~GeneratorMT()
{
   setState(exiting);

   // Wake up the generator thread if it is waiting for processes
   ProcPool()->condvar()->lock();
   ProcPool()->condvar()->signal();
   ProcPool()->condvar()->unlock();

   sync->thrd.join();
   delete sync;
   sync = NULL;
}

void GeneratorMT::start() 
{
   setGeneratorThread(DThread::self());
   pthrd_printf("Generator started on thread %lx\n", DThread::self());
   bool result;

   sync->init_cond.lock();
   result = initialize();
   if (!result) {
      pthrd_printf("Error initializing Generator\n");
      setState(error);
   }
   else {
      setState(none);
   }
   plat_start();
   sync->init_cond.signal();
   sync->init_cond.unlock();

   if (result)
      main();
   // Let the process pool know that we're not going to mess with processes anymore on this thread,
   // so any pending deletes (e.g. on Windows) can happen
   ProcPool()->condvar()->signal();
   pthrd_printf("Generator thread exiting\n");
}

void GeneratorMT::main()
{
   while (!isExitingState()) {
      bool result = getAndQueueEventInt(true);
      if (!result) {
         pthrd_printf("Error return in getAndQueueEventInt\n");
      }
   }
}

bool GeneratorMT::processWait(bool block)
{
   ProcessPool *pp = ProcPool();
   pp->condvar()->lock();
   pthrd_printf("Checking for live processes\n");
   while (!hasLiveProc() && !isExitingState()) {
      pthrd_printf("Checked and found no live processes\n");
      if (!block) {
         pthrd_printf("Returning from non-blocking processWait\n");
		 pp->condvar()->signal();
		 pp->condvar()->unlock();
         return false;
      }
      pp->condvar()->wait();
   }
   pp->condvar()->signal();
   pp->condvar()->unlock();
   pthrd_printf("processWait returning true\n");
   return true;
}

bool GeneratorMT::getAndQueueEvent(bool)
{
   //Doesn't really make sense to be calling this for a MT
   // generator--part of the point is that you don't have
   // to call it.
   return true;
}
