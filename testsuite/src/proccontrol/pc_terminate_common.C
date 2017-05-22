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

#include "proccontrol_comp.h"
#include "communication.h"

#if defined(os_windows_test)
	void sleep(int msec)
	{
		::Sleep(msec);
	}
#endif

#if defined(STOP_TEST)
#define PC_TERMINATE(suffix) pc_terminate_stopped ## suffix
#else
#define PC_TERMINATE(suffix) pc_terminate ## suffix
#endif

class PC_TERMINATE(Mutator) : public ProcControlMutator {
public:
  virtual test_results_t executeTest();
};

extern "C" DLLEXPORT TestMutator* PC_TERMINATE(_factory) ()
{
   return new PC_TERMINATE(Mutator) ();
}

static int num_pre_exited;
static int num_post_exited;
static int num_post_crashed;

static bool error;

static bool should_stop();

static Process::cb_ret_t on_exit(Event::const_ptr ev) 
{
   if (ev->getEventType().code() != EventType::Exit) {
      logerror("Recieved non-exit in on_exit\n");
      error = true;
   }
   if (ev->getEventType().time() == EventType::Pre)
      num_pre_exited++;
   else if (ev->getEventType().time() == EventType::Post)
      num_post_exited++;
   return Process::cbDefault;
}

static Process::cb_ret_t on_crash(Event::const_ptr ev) 
{
   if (ev->getEventType().code() != EventType::Crash) {
      logerror("Recieved non-crash in on_crash\n");
      error = true;
   }
   num_post_crashed++;
   return Process::cbDefault;
}

test_results_t PC_TERMINATE(Mutator::executeTest) ()
{
   std::vector<Process::ptr>::iterator i;
   error = false;
   num_pre_exited = 0;
   num_post_exited = 0;
   num_post_crashed = 0;

   comp->curgroup_self_cleaning = true;

   Process::registerEventCallback(EventType::Exit, on_exit);
   Process::registerEventCallback(EventType::Crash, on_crash);

   for (i = comp->procs.begin(); i != comp->procs.end(); i++) {
      Process::ptr proc = *i;
      bool result = proc->continueProc();
      if (!result) {
         logerror("Failed to continue process\n");
         error = true;
      }
   }

   syncloc sync_messages[NUM_PARALLEL_PROCS];
   bool result = comp->recv_broadcast((unsigned char *) sync_messages, sizeof(syncloc));
   if (!result) {
      logerror("Failed to recieve broadcast\n");
      error = true;
   }

   if (should_stop()) {
      for (i = comp->procs.begin(); i != comp->procs.end(); i++) {
         Process::ptr proc = *i;
         result = proc->stopProc();
         if (!result) {
            logerror("Failed to stop process\n");
            error = true;
         }
      }
   }

#if defined(os_bg_test)
   //On BlueGene terminating one process causes a SIGTERM to be sent to
   // all others.  Thus not all processes exit on a force-terminate.
   bool count_crash = false;
#else
   bool count_crash = true;
#endif


   for (i = comp->procs.begin(); i != comp->procs.end(); i++) {
      Process::ptr proc = *i;
      result = proc->terminate();
      if (count_crash && !result) {
         logerror("Failed to terminate process\n");
         error = true;
      }
   }

   syncloc sync_point;
   sync_point.code = SYNCLOC_CODE;
   result = comp->send_broadcast((unsigned char *) &sync_point, sizeof(syncloc));
   if (result) {
      //We expected this call to fail.
      //
      //Linux can be annoying at times.  This send should fail because the
      //mutatee is gone, but it sometimes succeeds even if the mutatees is
      //properly gone.  
      //
      //We'll give the kernel a few tries to clean up the pipe and give us
      //an error.  If the mutatee didn't exit, then none of these sends
      //should error out.
      //
      //Update, BG just completely fails to give the error.  Oh, well...
      bool got_failure = false;
      for (unsigned i=0; i<5; i++) {
         result = comp->send_broadcast((unsigned char *) &sync_point, sizeof(syncloc));
         if (!result) {
            got_failure = true;
            break;
         }
         sleep(1);
      }
#if !defined(os_bg_test)
      if (!got_failure) {
         logerror("Error.  Succeeded at send sync broadcast\n");
         error = true;
      }
#endif
   }

   if (num_pre_exited || num_post_exited || (count_crash && num_post_crashed))
   {
      logerror("Error.  Recieved event callbacks for terminate\n");
      logerror("pre_exit = %d, post_exit = %d, post_crash = %d\n",
               num_pre_exited, num_post_exited, num_post_crashed);
      error = true;
   }

   for (i = comp->procs.begin(); i != comp->procs.end(); i++) {
      Process::ptr proc = *i;
      if (!proc->isTerminated()) {
         logerror("Error.  Process was not marked as terminated\n");
         error = true;
      }
      if (proc->isExited() || proc->getExitCode()) {
         logerror("Error.  Process was marked as having a normal exit\n");
         error = true;
      }
      if (count_crash && (proc->isCrashed() || proc->getCrashSignal())) {
         logerror("Error.  Process was marked as having crashed\n");
         error = true;
      }
   }   
   
   Process::removeEventCallback(EventType::Exit);
   Process::removeEventCallback(EventType::Crash);

   return error ? FAILED : PASSED;
}

