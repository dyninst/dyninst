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
#include "Generator.h"
#include "Event.h"
#include "Mailbox.h"
#include "PCProcess.h"
#include "int_process.h"
#include "procpool.h"

#include "common/src/dthread.h"

#include <assert.h>
#include <iostream>

using namespace std;

std::set<Generator::gen_cb_func_t> Generator::CBs;
Mutex<> *Generator::cb_lock;

bool Generator::startedAnyGenerator = false;

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
   state(none),
   m_Event(NULL),
   name(name_),
   eventBlock_(false)
{
   if (!cb_lock) cb_lock = new Mutex<>();
   startedAnyGenerator = true;
}

Generator::~Generator()
{
   setState(exiting);
}

void Generator::forceEventBlock() {
   pthrd_printf("Forcing generator to block in waitpid\n");
   eventBlock_ = true;
   ProcPool()->condvar()->broadcast();
}

void Generator::stopDefaultGenerator() {
   if (!startedAnyGenerator)
      return;

   Generator *gen = Generator::getDefaultGenerator();
   if (gen) delete gen;
}

void Generator::registerNewEventCB(void (*func)())
{
   if (!cb_lock) cb_lock = new Mutex<>();
   Generator::cb_lock->lock();
   CBs.insert(func);
   Generator::cb_lock->unlock();
}

void Generator::removeNewEventCB(void (*func)())
{
   if (!cb_lock) cb_lock = new Mutex<>();
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

#if defined(STR_CASE)
#undef STR_CASE
#endif
#define STR_CASE(X) case Generator::X: return #X

const char * Generator::generatorStateStr(Generator::state_t s) {
   switch (s) {
      STR_CASE(none);
      STR_CASE(initializing);
      STR_CASE(process_blocked);
      STR_CASE(system_blocked);
      STR_CASE(decoding);
      STR_CASE(statesync);
      STR_CASE(handling);
      STR_CASE(queueing);
      STR_CASE(error);
      STR_CASE(exiting);
   }
   assert(0);
   return NULL;
}

void Generator::setState(Generator::state_t new_state)
{
   pthrd_printf("Setting generator state to %s\n", generatorStateStr(new_state));
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

bool Generator::getMultiEvent(bool block, vector<ArchEvent *> &events)
{
   //This function can be optionally overloaded by a platform
   // that may return multiple events.  Otherwise, it just 
   // uses the single-event interface and always returns a 
   // set of size 1 or 0.
   ArchEvent *ev = getEvent(block);
   if (!ev)
      return false;
   events.push_back(ev);
   eventBlock_ = false;
   return true;
}

bool Generator::getAndQueueEventInt(bool block)
{
   bool result = true;
   //static ArchEvent *arch_event = NULL;
   ArchEvent* arch_event = getCachedEvent();
   vector<Event::ptr> events;
   vector<ArchEvent *> archEvents;

   if (isExitingState()) {
      pthrd_printf("Generator exiting before processWait\n");
      result = false;
      goto done;
   }
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
   result = getMultiEvent(block, archEvents);
   pthrd_printf("Got event\n");
   if (isExitingState()) {
      pthrd_printf("Generator exiting after getEvent\n");
      result = false;
      goto done;
   }
   if (!result) {
      pthrd_printf("Error. Unable to recieve event\n");
      result = false;
      goto done;
   }

   setState(decoding);
   //mbox()->lock_queue();
   ProcPool()->condvar()->lock();

   for (vector<ArchEvent *>::iterator i = archEvents.begin(); i != archEvents.end(); i++) {
	   arch_event = *i;
      for (decoder_set_t::iterator j = decoders.begin(); j != decoders.end(); j++) {
         Decoder *decoder = *j;
         result = decoder->decode(arch_event, events);
         if (result)
            break;
      }
   }

   setState(statesync);
   for (vector<Event::ptr>::iterator i = events.begin(); i != events.end(); i++) {
      Event::ptr event = *i;
	  if(event) {
	      event->getProcess()->llproc()->updateSyncState(event, true);
	  }
   }

   ProcPool()->condvar()->unlock();

   setState(queueing);
   for (vector<Event::ptr>::iterator i = events.begin(); i != events.end(); ++i) {
      mbox()->enqueue(*i);
      Generator::cb_lock->lock();
      for (set<gen_cb_func_t>::iterator j = CBs.begin(); j != CBs.end(); ++j) {
         (*j)();
      }
      Generator::cb_lock->unlock(); 
   }
   //mbox()->unlock_queue();


   result = true;
 done:
   setState(none);
   setCachedEvent(arch_event);
   return result;
}

bool Generator::plat_skipGeneratorBlock()
{
   //Default, do nothing.  May be over-written
   return false;
}

Generator::state_t Generator::getState()
{
	return state;
}

// TODO: override this in Windows generator so that we use local counters
bool Generator::hasLiveProc()
{
   pthrd_printf("Entry to generator::hasLiveProc()\n");
   if (plat_skipGeneratorBlock()) {
      return true;
   }

   int num_running_threads = Counter::globalCount(Counter::GeneratorRunningThreads);
   int num_non_exited_threads = Counter::globalCount(Counter::GeneratorNonExitedThreads);
   int num_force_generator_blocking = Counter::globalCount(Counter::ForceGeneratorBlock);

   if (num_running_threads) {
      pthrd_printf("Generator has running threads, returning true from hasLiveProc\n");
      return true;
   }
   if (!num_non_exited_threads) {
      pthrd_printf("Generator has all exited threads, returning false from hasLiveProc\n");
      return false;
   }
   if (num_force_generator_blocking) {
      bool override = true;
#if defined(os_linux)
      // We only want to block if the mailbox is empty; otherwise we double up events
      if (mbox()->size() > 0) override = false;
#endif
      if (override) {
         pthrd_printf("Forcing generator blocking\n");
         return true;
      }
   }


   pthrd_printf("No live processes, ret false\n");
   return false;
}


struct GeneratorMTInternals
{
   GeneratorMTInternals() {}

   //Start-up synchronization
   CondVar<> init_cond;

   DThread thrd;
};
void GeneratorInternalJoin(GeneratorMTInternals *gen_int)
{
  gen_int->thrd.join();
}

#if defined(os_windows)
static unsigned long WINAPI start_generator(void *g)
#else
static void start_generator(void *g)
#endif
{
   GeneratorMT *gen = (GeneratorMT *) g;
   gen->start();
#if defined(os_windows)
   return 0;
#else
   return;
#endif
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
   setState(initializing);
   sync->thrd.spawn(start_generator, this);
   while (getState() == initializing)
      sync->init_cond.wait();
   sync->init_cond.unlock();

   if (getState() == error) {
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
   pthrd_printf("Generator started on thread %lx\n", (unsigned long)DThread::self());
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
   if(getState() == error) result = false;
   sync->init_cond.signal();
   sync->init_cond.unlock();

   if (result) {
      pthrd_printf("Starting main loop of generator thread\n");
      main();
   }
   pthrd_printf("Generator thread exiting\n");
}

void GeneratorMT::main()
{
   while (!isExitingState()) {
      bool result = getAndQueueEventInt(true);
      if (!result && !isExitingState()) {
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
         pp->condvar()->broadcast();
         pp->condvar()->unlock();
         return false;
      }
      pp->condvar()->wait();
   }
   pp->condvar()->broadcast();
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

GeneratorMTInternals *GeneratorMT::getInternals()
{
  return sync;
}
