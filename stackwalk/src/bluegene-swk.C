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
#include "stackwalk/src/bluegene-swk.h"
using namespace DebuggerInterface;

#include "stackwalk/src/bluegenel-swk.h"
#include "stackwalk/src/bluegenep-swk.h"
#include "stackwalk/src/sw.h"

#include "stackwalk/h/swk_errors.h"
#include "stackwalk/h/symlookup.h"
#include "stackwalk/h/walker.h"
#include "stackwalk/h/framestepper.h"
#include "stackwalk/h/procstate.h"
#include "stackwalk/src/symtab-swk.h"

using namespace Dyninst;
using namespace Dyninst::Stackwalker;

#include "common/src/SymLite-elf.h"


#include <string>
#include <memory>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/poll.h>
#include <assert.h>
using namespace std;


namespace Dyninst {
  namespace Stackwalker {

    bool bg_fdHasData(int fd) {
      int result;
      struct pollfd fds;
      fds.fd = fd;
      fds.events = POLLIN;
      fds.revents = 0;
      result = poll(&fds, 1, 0);
      if (result == -1) {
        int errnum = errno;
        sw_printf("[%s:%u] - Unable to poll fd %d: %s\n", FILE__, __LINE__,
                  fd, strerror(errnum));
        return false;
      }
      return (bool) (fds.revents & POLLIN);
    }


    // ============================================================ //
    // Walker
    // ============================================================ //
    bool Walker::createDefaultSteppers() {
      FrameStepper *stepper = new FrameFuncStepper(this);
      bool result = addStepper(stepper);
      if (!result) {
        sw_printf("[%s:%u] - Error adding stepper %p\n", FILE__, __LINE__, stepper);
      }
      return result;
    }


    // ============================================================ //
    // ProcSelf -- all stubs; this is 3rd-party only
    // ============================================================ //
    static bool proc_self_unsupported(const char *file, size_t line) {
      const char *msg = "ProcSelf not supported for 3rd party BG stackwalkers!";
      sw_printf("[%s:%u] - %s\n", file, line, msg);
      setLastError(err_procread, msg);
      return false;
    }

    bool ProcSelf::getThreadIds(std::vector<THR_ID> &) {
      return proc_self_unsupported(FILE__, __LINE__);
    }


    ProcSelf::ProcSelf(std::string exe_path) : ProcessState(getpid(), exe_path) {
      proc_self_unsupported(FILE__, __LINE__);
    }


    void ProcSelf::initialize() {
      proc_self_unsupported(FILE__, __LINE__);
    }


    bool ProcSelf::getDefaultThread(THR_ID &) {
      return proc_self_unsupported(FILE__, __LINE__);
    }


    bool ProcSelf::readMem(void *, Address, size_t) {
      return proc_self_unsupported(FILE__, __LINE__);
    }


    // ============================================================ //
    // ProcDebug
    // ============================================================ //
    ProcDebug *ProcDebug::newProcDebug(PID pid, string executable) {
      auto_ptr<ProcDebug> pd(ProcDebugBG::createProcDebugBG(pid, executable));
      if (!pd.get()) {
        const char *msg = "Error creating new ProcDebug object";
        sw_printf("[%s:%u] - %s\n", FILE__, __LINE__, msg);
        setLastError(err_internal, msg);
        return NULL;
      }

      bool result = pd->attach();
      if (!result || (pd->state() != ps_running && pd->state() != ps_attached))
      {
        pd->setState(ps_errorstate);
        proc_map.erase(pid);
        const char *msg = "Error attaching to process";
        sw_printf("[%s:%u] - %s %d\n", FILE__, __LINE__, msg, pid);
        setLastError(err_noproc, msg);
        return NULL;
      }

      return pd.release();
    }


    bool ProcDebug::newProcDebugSet(const vector<Dyninst::PID>&, vector<ProcDebug*>&) {
      setLastError(err_unsupported, "ERROR: newProcDebugSet not implemented for BlueGene Stackwalker!");
      return false;
    }


    ProcDebug *ProcDebug::newProcDebug(std::string, const std::vector<std::string> &) {
      setLastError(err_unsupported, "Executable launch not supported on BlueGene");
      return NULL;
    }


    bool ProcDebugBG::isLibraryTrap(Dyninst::THR_ID) {
      // Base ProcDebugBG doesn't support dynamic libs.
      return false;
    }


    DebugEvent ProcDebug::debug_get_event(bool block) {
      DebugEvent ev;
      BG_Debugger_Msg msg;

      if (!block && !bg_fdHasData(BG_DEBUGGER_READ_PIPE)) {
        ev.dbg = dbg_noevent;
        return ev;
      }

      // Read an event from the debug filehandle.
      sw_printf("[%s:%u] - Waiting for debug event\n", FILE__, __LINE__);
      bool result = BG_Debugger_Msg::readFromFd(BG_DEBUGGER_READ_PIPE, msg);
      if (!result) {
        sw_printf("[%s:%u] - Unable to wait for debug event on process\n",
                  FILE__, __LINE__);
        ev.dbg = dbg_err;
        return ev;
      }

      // extract process and thread id from the BG message.
      pid_t pid = msg.header.nodeNumber;
      THR_ID tid = msg.header.thread;
      int returnCode = msg.header.returnCode;
      sw_printf("[%s:%u] - Received debug event %s from pid %d, tid %d, rc %d\n", FILE__, __LINE__,
                BG_Debugger_Msg::getMessageName(msg.header.messageType), pid, tid, returnCode);

      if (returnCode > 0) {
        // Print out a message if we get a return code we don't expect.  Consult the debugger header
        // for the meanings of these.
        sw_printf("[%s:%u] - WARNING: return code for %s on pid %d, tid %d was non-zero: %d\n",
                  FILE__, __LINE__,
                  BG_Debugger_Msg::getMessageName(msg.header.messageType), pid, tid,
                  msg.header.returnCode);
      }

      // Look up the ProcDebugBG from which the event originated.
      std::map<PID, ProcessState *>::iterator i = proc_map.find(pid);
      if (i == proc_map.end()) {
        sw_printf("[%s:%u] - Error, received unknown pid %d\n", FILE__, __LINE__, pid);
        setLastError(err_internal, "Error waiting for debug event");
        ev.dbg = dbg_err;
        return ev;
      }
      ProcDebugBG *procbg = dynamic_cast<ProcDebugBG*>(i->second);
      assert(procbg);
      ev.proc = procbg;

      // below is some (somewhat nasty) magic to allow stackwalker to discover the
      // initial thread's id after it attaches.
      thread_map_t::iterator t = procbg->threads.find(tid);
      if (t == procbg->threads.end()) {
        BGThreadState *initial_thread = dynamic_cast<BGThreadState*>(procbg->initial_thread);

        if (tid != 0) {
          // Check to see if the initial thread is valid.  If its id is zero, it's a stub
          // from creation time and we got a signal from the real initial thread.  If it's
          // not zero, then we're seeing a strange thread id.
          if (initial_thread->getTid() != 0) {
            const char *msg = "Saw unknown thread id in ProcDebug::debug_get_evet()!";
            sw_printf("[%s:%u] - %s on pid %d\n", FILE__, __LINE__, msg, pid);
            setLastError(err_internal, msg);
            ev.dbg = dbg_err;
            return ev;
          }

          // if we see a threadid we don't know about, it's because we got a SIGNAL_ENCOUNTERED
          // from the main thread.  We default the initial thread id to zero, but this is only because
          // we need to start somewhere to attach.  This sets the main thread id to the real thread id
          // for the process.  This should happen relatively early when we stop the thread during attach.
          procbg->threads.erase(initial_thread->getTid());
          initial_thread->setTid(tid);
          procbg->threads[tid] = initial_thread;
          t = procbg->threads.find(tid);

        } else {
          // if the thread id we don't know about is zero, we got an ack for something we
          // did before we discovered what the initial thread id was.  So just point the event
          // at the initial thread
          t = procbg->threads.find(initial_thread->getTid());
        }
      }

      ThreadState *ts = t->second;
      assert(ts);
      ev.thr = ts;

      procbg->translate_event(msg, ev);
      return ev;
    }


   int ProcDebug::getNotificationFD() {
      return BG_DEBUGGER_READ_PIPE;
    }


    // ============================================================ //
    // ProcDebugBG
    // ============================================================ /
    void ProcDebugBG::translate_event(const BG_Debugger_Msg& msg, DebugEvent& ev) {
      switch (msg.header.messageType) {
      case PROGRAM_EXITED:
        {
          ev.data.idata = msg.dataArea.PROGRAM_EXITED.rc;
          int exit_type = msg.dataArea.PROGRAM_EXITED.type;

          if (exit_type == 0) {
            ev.dbg = dbg_exited;
            sw_printf("[%s:%u] - Process %d exited with %d\n", FILE__, __LINE__, pid, ev.data.idata);
          } else if (exit_type == 1) {
            ev.dbg = dbg_crashed;
            sw_printf("[%s:%u] - Process %d crashed with %d\n", FILE__, __LINE__, pid, ev.data.idata);
          } else {
            ev.dbg = dbg_err;
            sw_printf("[%s:%u] - WARNING: Unknown exit type (%d) on process %d. "
                      "May be using outdated BG Debugger Interface!\n", FILE__, __LINE__,
                      msg.dataArea.PROGRAM_EXITED.type, pid);
          }
        }
        break;

      case SIGNAL_ENCOUNTERED:
        ev.dbg = dbg_stopped;
        ev.data.idata = msg.dataArea.SIGNAL_ENCOUNTERED.signal;
        sw_printf("[%s:%u] - Process %d stopped with %d\n",
                  FILE__, __LINE__, pid, msg.dataArea.SIGNAL_ENCOUNTERED.signal);
        break;

      case ATTACH_ACK:
        ev.dbg = dbg_attached;
        sw_printf("[%s:%u] - Process %d acknowledged attach\n", FILE__, __LINE__, pid);
        break;
      case DETACH_ACK:
         ev.dbg = dbg_detached;
         sw_printf("[%s:%u] - Process %d acknowledged detach\n", FILE__, __LINE__, pid);
         break;
      case CONTINUE_ACK:
        ev.dbg = dbg_continued;
        sw_printf("[%s:%u] - Process %d acknowledged continue\n", FILE__, __LINE__, pid);
        break;

      case KILL_ACK:
        ev.dbg = dbg_other;
        sw_printf("[%s:%u] - Process %d acknowledged kill\n", FILE__, __LINE__, pid);
        break;

      case GET_ALL_REGS_ACK:
        {
          ev.dbg = dbg_allregs_ack;
          ev.size = msg.header.dataLength;
          BG_GPRSet_t *data = new BG_GPRSet_t();
          *data = msg.dataArea.GET_ALL_REGS_ACK.gprs;
          ev.data.pdata = data;
          sw_printf("[%s:%u] - RegisterAll ACK on pid %d\n", FILE__, __LINE__, pid);
        }
        break;
      case SET_REG_ACK:
	{
	  ev.dbg = dbg_setreg_ack;
	  sw_printf("[%s:%u] - SET_REG ACK on pid %d\n", FILE__, __LINE__, pid);
	  break;
	}

      case GET_MEM_ACK:
        ev.dbg = dbg_mem_ack;
        ev.size = msg.header.dataLength;
        ev.data.pdata = new unsigned char[msg.header.dataLength];
        if (!ev.data.pdata) {
          ev.dbg = dbg_err;
          sw_printf("[%s:%u] - FATAL: Couldn't allocate enough space for memory read on pid %d\n",
                    FILE__, __LINE__, pid);
        } else {
          memcpy(ev.data.pdata, &msg.dataArea, msg.header.dataLength);
          sw_printf("[%s:%u] - Memory read ACK on pid %d\n", FILE__, __LINE__, pid);
        }
        break;

    case SET_MEM_ACK:
        ev.dbg = dbg_setmem_ack;
        ev.size = msg.dataArea.SET_MEM_ACK.len;
        sw_printf("[%s:%u] - Memory write ACK on pid %d ($d bytes at %x).  \n", FILE__, __LINE__,
                  msg.dataArea.SET_MEM_ACK.len, msg.dataArea.SET_MEM_ACK.addr);
        break;

      case SINGLE_STEP_ACK:
        // single step ack is just an ack (like KILL_ACK). We ignore this event and
        // handle SINGLE_STEP_SIG specially in debug_handle_signal().
        ev.dbg = dbg_other;
        sw_printf("[%s:%u] - Process %d received SINGLE_STEP\n", FILE__, __LINE__, pid, ev.data);
        break;

      default:
        sw_printf("[%s:%u] - Unknown debug message: %s (%d)\n",
                  FILE__, __LINE__,
                  BG_Debugger_Msg::getMessageName(msg.header.messageType),
                  msg.header.messageType);
        ev.dbg = dbg_noevent;
        break;
     }
  }


    void copy_thread_state(ThreadState *source, ThreadState *dest) {
      dest->setState(source->state());
      dest->setStopped(source->isStopped());
      dest->setShouldResume(source->shouldResume());
      dest->setUserStopped(source->userIsStopped());
    }

    bool ProcDebugBG::pollForNewThreads() {
      return true;  // by default, we don't find anything.
    }


    // This basic implementation assumes *only* the initial thread.
    bool ProcDebugBG::getThreadIds(std::vector<THR_ID> &threads) {
      threads.clear();
      threads.push_back(initial_thread->getTid());
      return true;
    }


    bool ProcDebugBG::getDefaultThread(THR_ID &default_tid) {
      default_tid = initial_thread->getTid();
      return true;
    }


    unsigned ProcDebugBG::getAddressWidth() {
      return sizeof(DebuggerInterface::BG_Addr_t);
    }


    bool ProcDebugBG::debug_continue(ThreadState *ts) {
      return debug_continue_with(ts, 0);
    }


    ProcDebugBG::~ProcDebugBG() {
      if (read_cache) delete [] read_cache;
    }


    ProcDebugBG::ProcDebugBG(PID pid, string exe)
      : ProcDebug(pid, exe),
        mem_data(NULL),
        read_cache(NULL),
        read_cache_start(0x0),
        read_cache_size(0x0),
        detached(false),
        write_ack(false),
        wrote_trap(false)
    {
    }


    bool ProcDebugBG::debug_create(std::string, const std::vector<std::string> &)
    {
      setLastError(err_unsupported, "Create mode not supported on BlueGene");
      return false;
    }


    struct set_gprs {
      bool val;
      set_gprs(bool v) : val(v) { }
      void operator()(ThreadState *ts) {
        dynamic_cast<BGThreadState*>(ts)->gprs_set = val;
      }
    };


    void ProcDebugBG::clear_cache() {
      for_all_threads(set_gprs(false));
      read_cache_start = 0x0;
      read_cache_size = 0x0;
      mem_data = NULL;
    }


    bool ProcDebugBG::version_msg()
    {
      bool result = debug_version_msg();
      if (!result) {
        sw_printf("[%s:%u] - Could not version msg debugee %d\n", FILE__, __LINE__, pid);
        return false;
      }

      result = debug_waitfor_version_msg();
      if (state() == ps_exited) {
        setLastError(err_procexit, "Process exited unexpectedly during version_msg");
        return false;
      }
      if (!result) {
        sw_printf("[%s:%u] - Error during process version_msg for %d\n",
                  FILE__, __LINE__, pid);
        return false;
      }
      return true;
    }


    bool ProcDebugBG::debug_waitfor_version_msg() {
      sw_printf("[%s:%u] - At debug_waitfor_Version_msg.\n", FILE__, __LINE__);
      bool handled, result;

      result = debug_wait_and_handle(true, false, handled);
      if (!result || state() == ps_errorstate) {
        sw_printf("[%s:%u] - Error,  Process %d errored during version_msg\n",
                  FILE__, __LINE__, pid);
        return false;
      }
      if (state() == ps_exited) {
        sw_printf("[%s:%u] - Error.  Process %d exited during version_msg\n",
                  FILE__, __LINE__, pid);
        return false;
      }
      sw_printf("[%s:%u] - Successfully version_msg %d\n",
                FILE__, __LINE__, pid);
      return true;
    }



    bool ProcDebugBG::debug_handle_event(DebugEvent ev)
    {
      BGThreadState *thr = dynamic_cast<BGThreadState*>(ev.thr);

      switch (ev.dbg) {
      case dbg_stopped:
        // we got a signal from the process.  Stop all the threads and let
        // debug_handle_signal() take care of things.
        for_all_threads(set_stopped(true));
        return debug_handle_signal(&ev);

      case dbg_crashed:
        sw_printf("[%s:%u] - Process %d crashed!\n", FILE__, __LINE__, pid);
        // fallthru

      case dbg_exited:
        sw_printf("[%s:%u] - Handling process exit on %d\n", FILE__, __LINE__, pid);
        for_all_threads(set_stopped(true));
        for_all_threads(set_state(ps_exited));
        break;

      case dbg_continued:
        sw_printf("[%s:%u] - Process %d continued\n", FILE__, __LINE__, pid);
        clear_cache();
        assert(thr->isStopped());
        for_all_threads(set_stopped(false));
        break;

      case dbg_attached:
        sw_printf("[%s:%u] - Process %d attached\n", FILE__, __LINE__, pid);
        assert(state() == ps_neonatal);
        for_all_threads(set_state(ps_attached_intermediate));
        debug_pause(NULL); //immediately after debug_attach, pause the process.
        break;
      case dbg_detached:
	sw_printf("[%s:%u] - Process %d detached\n", FILE__, __LINE__, pid);
	detached = true;
	break;

      case dbg_mem_ack:
        sw_printf("[%s:%u] - Process %d returned a memory chunk of size %u\n",
                  FILE__, __LINE__, pid, ev.size);
        assert(!mem_data);
        mem_data = static_cast<BG_Debugger_Msg::DataArea*>(ev.data.pdata);
        break;

      case dbg_setmem_ack:
        sw_printf("[%s:%u] - Process %d set a chunk of memory of size %u\n",
                  FILE__, __LINE__, pid, ev.size);
	write_ack = true;
        break;

      case dbg_allregs_ack:
        {
          sw_printf("[%s:%u] - Process %d returned a register chunk of size %u\n",
                    FILE__, __LINE__, pid, ev.size);
          BG_GPRSet_t *data = static_cast<BG_GPRSet_t*>(ev.data.pdata);
          thr->gprs = *data;
          thr->gprs_set = true;
          ev.data.pdata = NULL;
          delete data;
        }
        break;
      case dbg_setreg_ack:
	{
	  sw_printf("[%s:%u] - Handling set reg ack on process %d\n",
		    FILE__, __LINE__, pid);
	  thr->write_ack = true;
	  break;
	}
      case dbg_other:
        sw_printf("[%s:%u] - Skipping unimportant event\n", FILE__, __LINE__);
        break;

      case dbg_err:
      case dbg_noevent:
      default:
        sw_printf("[%s:%u] - Unexpectedly handling an error event %d on %d\n",
                  FILE__, __LINE__, ev.dbg, pid);
        setLastError(err_internal, "Told to handle an unexpected event.");
        return false;
      }

      return true;
    }


    bool ProcDebugBG::debug_handle_signal(DebugEvent *ev) {
      assert(ev->dbg == dbg_stopped);
      // PRE: we got a signal event, and things are stopped.

      sw_printf("[%s:%u] - Handling signal for pid %d\n", FILE__, __LINE__, pid);
      BGThreadState *thr = dynamic_cast<BGThreadState*>(ev->thr);

      switch (ev->data.idata) {
      case SIGSTOP:
        // if we're attaching, SIGSTOP tells us we've completed.
        if (state() == ps_attached_intermediate) {
          setState(ps_attached);
          sw_printf("[%s:%u] - Moving %d to state ps_attached\n", FILE__, __LINE__, pid);
        }
        // if we're not attaching, do nothing.  leave the thread stopped.
        break;

      case SINGLE_STEP_SIG:
        // BG uses this special signal to indicate completion of a single step.
        // Ignore the event and continue if the user didn't stop. TODO: why?
        // Otherwise, if *we* stopped, just stay stopped.
        clear_cache();  // took a step, need to clear cache.
        return thr->userIsStopped() ? true : debug_continue_with(thr, 0);
        break;

      default:
        // by default, pass the signal back to the process being debugged.
        if (sigfunc) {
           bool user_stopped = ev->thr->userIsStopped();
           ev->thr->setUserStopped(true);
           sigfunc(ev->data.idata, ev->thr);
           ev->thr->setUserStopped(user_stopped);
        }
        return debug_continue_with(thr, ev->data.idata);
      }

      return true;
    }


    bool ProcDebugBG::debug_attach(ThreadState *ts)
    {
      THR_ID tid = ts->getTid();
      BG_Debugger_Msg msg(ATTACH, pid, tid, 0, 0);
      msg.header.dataLength = sizeof(msg.dataArea.ATTACH);

      // send attach message
      sw_printf("[%s:%u] - Attaching to pid %d, thread %d\n", FILE__, __LINE__, pid, tid);
      bool result = BG_Debugger_Msg::writeOnFd(BG_DEBUGGER_WRITE_PIPE, msg);
      if (!result) {
        sw_printf("[%s:%u] - Error sending attach to process %d, thread %d\n",
                  FILE__, __LINE__, pid, tid);
      }

      return result;
    }

    bool ProcDebugBG::cleanOnDetach() {
      //Overridden on BG/P
      return true;
    }

    bool ProcDebugBG::detach(bool /*leave_stopped*/) {
      // TODO: send detach message
      sw_printf("[%s:%u] - Detaching from process %d\n", FILE__, __LINE__,
		pid);
      bool result = cleanOnDetach();
      if (!result) {
	sw_printf("[%s:%u] - Failed to clean process %d\n",
		  FILE__, __LINE__, pid);
	return false;
      }

      result = resume();
      if (!result) {
	sw_printf("[%s:%u] - Error resuming process before detach\n");
      }

      BG_Debugger_Msg msg(DETACH, pid, 0, 0, 0);
      msg.header.dataLength = sizeof(msg.dataArea.DETACH);
      result = BG_Debugger_Msg::writeOnFd(BG_DEBUGGER_WRITE_PIPE, msg);
      if (!result) {
	sw_printf("[%s:%u] - Failed to write detach to process %d\n",
		  FILE__, __LINE__, pid);
	setLastError(err_internal, "Could not write to debug pipe\n");
	return false;
      }
      while (!detached) {
	bool handled;
	bool result = debug_wait_and_handle(true, false, handled);
	if (!result) {
	  sw_printf("[%s:%u] - Error while waiting for detach\n",
		    FILE__, __LINE__);
	    return false;
	}
      }
      return true;
    }


    bool ProcDebugBG::debug_pause(ThreadState *ts)
    {
      THR_ID tid = initial_thread->getTid();  // default to initial thread.
      if (ts) tid = ts->getTid();

      BG_Debugger_Msg msg(KILL, pid, tid, 0, 0);
      msg.dataArea.KILL.signal = SIGSTOP;
      msg.header.dataLength = sizeof(msg.dataArea.KILL);

      sw_printf("[%s:%u] - Sending SIGSTOP to pid %d, thread %d\n", FILE__, __LINE__, pid, tid);
      bool result = BG_Debugger_Msg::writeOnFd(BG_DEBUGGER_WRITE_PIPE, msg);
      if (!result) {
        sw_printf("[%s:%u] - Unable to send SIGSTOP to process %d, thread\n", FILE__, __LINE__, pid, tid);
      }
      return result;
    }


    bool ProcDebugBG::debug_continue_with(ThreadState *ts, long sig)
    {
      THR_ID tid = ts->getTid();
      if (!ts->isStopped()) {
        sw_printf("[%s:%u] - Error in debug_continue_with(): pid %d, thread %d not stopped.\n",
                  FILE__, __LINE__, pid, tid);
        return false;
      }

      BG_Debugger_Msg msg(CONTINUE, pid, tid, 0, 0);
      msg.dataArea.CONTINUE.signal = sig;
      msg.header.dataLength = sizeof(msg.dataArea.CONTINUE);

      sw_printf("[%s:%u] - Sending signal %d to pid %d, thread %d\n", FILE__, __LINE__, sig, pid, tid);
      bool result = BG_Debugger_Msg::writeOnFd(BG_DEBUGGER_WRITE_PIPE, msg);
      if (!result) {
        sw_printf("[%s:%u] - Unable to send %d to process %d, thread %d\n", FILE__, __LINE__, sig, pid, tid);
      }
      return result;
    }


    bool ProcDebugBG::getRegValue(MachRegister reg, THR_ID tid, MachRegisterVal &val) {
      assert(threads.count(tid));
      BGThreadState *thr = dynamic_cast<BGThreadState*>(threads[tid]);

      if (!thr->gprs_set) {
        BG_Debugger_Msg msg(GET_ALL_REGS, pid, tid, 0, 0);
        msg.header.dataLength = sizeof(msg.dataArea.GET_ALL_REGS);

        bool result = BG_Debugger_Msg::writeOnFd(BG_DEBUGGER_WRITE_PIPE, msg);
        if (!result) {
          sw_printf("[%s:%u] - Unable to write to process %d, thread %d\n",
                    FILE__, __LINE__, pid, tid);
          return true;
        }

        //Wait for the read to return
        sw_printf("[%s:%u] - Waiting for read to return on pid %d, tid %d\n",
                  FILE__, __LINE__, pid, tid);

        do {
          bool handled, result;
          result = debug_wait_and_handle(true, false, handled);
          if (!result)
          {
            sw_printf("[%s:%u] - Error while waiting for read to return\n",
                      FILE__, __LINE__);
            return false;
          }
        } while (!thr->gprs_set);
      }

      switch(reg.val())
      {
         case Dyninst::iReturnAddr:
         case Dyninst::ppc32::ipc:
            val = thr->gprs.iar;
            break;
         case Dyninst::iFrameBase:
         case Dyninst::ppc32::ir1:
            val = thr->gprs.gpr[BG_GPR1];
            break;
         case Dyninst::iStackTop:
            val = 0x0;
            break;
         default:
            sw_printf("[%s:%u] - Request for unsupported register %d\n", FILE__, __LINE__, reg.name().c_str());
            setLastError(err_badparam, "Unknown register passed in reg field");
            return false;
      }
      return true;
    }



    bool ProcDebugBG::setRegValue(MachRegister reg, THR_ID tid, MachRegisterVal val) {
      assert(threads.count(tid));
      BGThreadState *thr = dynamic_cast<BGThreadState*>(threads[tid]);

      BG_Debugger_Msg msg(SET_REG, pid, tid, 0, 0);
      msg.header.dataLength = sizeof(msg.dataArea.SET_REG);
      msg.dataArea.SET_REG.value = val;
      switch(reg.val())
      {
         case Dyninst::iFrameBase:
         case Dyninst::ppc32::ir1:
            msg.dataArea.SET_REG.registerNumber = BG_GPR1;
            break;
         case Dyninst::iReturnAddr:
         case Dyninst::ppc32::ipc:
            msg.dataArea.SET_REG.registerNumber = BG_IAR;
            break;
         default:
            sw_printf("[%s:%u] - Request for unsupported register %s\n", FILE__, __LINE__, reg.name().c_str());
            setLastError(err_badparam, "Unknown register passed in reg field");
            return false;
      }
      thr->write_ack = false;
      bool result = BG_Debugger_Msg::writeOnFd(BG_DEBUGGER_WRITE_PIPE, msg);
      if (!result) {
	sw_printf("[%s:%u] - Unable to set reg to process %d, thread %d\n",
		  FILE__, __LINE__, pid, tid);
	return true;
      }

      //Wait for the read to return
      sw_printf("[%s:%u] - Waiting for set reg to return on pid %d, tid %d\n",
		FILE__, __LINE__, pid, tid);

      do {
	bool handled, result;
	result = debug_wait_and_handle(true, false, handled);
	if (!result)
          {
            sw_printf("[%s:%u] - Error while waiting for read to return\n",
                      FILE__, __LINE__);
            return false;
          }
      } while (!thr->write_ack);

      return true;
    }

    bool ProcDebugBG::readMem(void *dest, Address source, size_t size)
    {
      unsigned char *ucdest = (unsigned char *) dest;
      bool result;

      for (;;)
      {
        if (size == 0)
          return true;

        sw_printf("[%s:%u] - Reading memory from 0x%lx to 0x%lx (into %p)\n",
                  FILE__, __LINE__, source, source+size, ucdest);
        if (source >= read_cache_start &&
            source+size < read_cache_start + read_cache_size)
        {
          //Lucky us, we have everything cached.
          memcpy(ucdest, read_cache + (source - read_cache_start), size);
          return true;
        }
        if (source >= read_cache_start &&
            source < read_cache_start + read_cache_size)
        {
          //The beginning is in the cache, read those bytes
          long cached_bytes = read_cache_start + read_cache_size - source;
          memcpy(ucdest, read_cache + (source - read_cache_start), cached_bytes);
          //Change the parameters and continue the read (this is a kind of
          // optimized tail call).
          ucdest += cached_bytes;
          source = read_cache_start + read_cache_size;
          size -= cached_bytes;
          continue;
        }
        if (source+size > read_cache_start &&
            source+size <= read_cache_start + read_cache_size)
        {
          //The end is in the cache, read those bytes
          long cached_bytes = (source+size) - read_cache_start;
          memcpy(ucdest + read_cache_start - source, read_cache, cached_bytes);
          //Change the parameters and continue the read (this is a kind of
          // optimized tail call).
          size -= cached_bytes;
          continue;
        }
        if (source < read_cache_start &&
            source+size >= read_cache_start+read_cache_size)
        {
          //The middle is in the cache
          unsigned char *dest_start = ucdest + read_cache_start - source;
          Address old_read_cache_start = read_cache_start;
          memcpy(dest_start, read_cache, read_cache_size);
          //Use actual recursion here, as we need to queue up two reads
          // First read out of the high memory with recursion
          result = readMem(dest_start + read_cache_size,
                           read_cache_start+read_cache_size,
                           source+size - read_cache_start+read_cache_size);
          if (!result) {
            return false;
          }
          // Now read out of the low memory
          size = old_read_cache_start - source;
          continue;
        }
        //The memory isn't cached, read a new cache'd page.
        assert(source < read_cache_start ||
               source >= read_cache_start+read_cache_size);

        if (!read_cache) {
          read_cache = new unsigned char[BG_READCACHE_SIZE];
          assert(read_cache);
        }

        read_cache_size = BG_READCACHE_SIZE;
        read_cache_start = source - (source % read_cache_size);
        sw_printf("[%s:%u] - Caching memory from 0x%lx to 0x%lx\n",
                  FILE__, __LINE__, read_cache_start,
                  read_cache_start+read_cache_size);

        //Read read_cache_start to read_cache_start+read_cache_size into our
        // cache.
        assert(!mem_data);
        assert(read_cache_size < BG_Debugger_Msg_MAX_MEM_SIZE);

        BG_Debugger_Msg msg(GET_MEM, pid, 0, 0, 0);
        msg.header.dataLength = sizeof(msg.dataArea.GET_MEM);
        msg.dataArea.GET_MEM.addr = read_cache_start;
        msg.dataArea.GET_MEM.len = read_cache_size;
        bool result = BG_Debugger_Msg::writeOnFd(BG_DEBUGGER_WRITE_PIPE, msg);
        if (!result) {
          sw_printf("[%s:%u] - Unable to write to process %d\n", FILE__, __LINE__, pid);
          return true;
        }

        //Wait for the read to return
        sw_printf("[%s:%u] - Waiting for read to return on pid %d\n", FILE__, __LINE__);

        if (!debug_waitfor(dbg_mem_ack)) {
          sw_printf("[%s:%u] - Error while waiting for read to return\n", FILE__, __LINE__);
          return false;
        }

        sw_printf("[%s:%u] - Asserting mem_data post-read.\n", FILE__, __LINE__);
        assert(mem_data->GET_MEM_ACK.addr == read_cache_start);
        assert(mem_data->GET_MEM_ACK.len == read_cache_size);
        memcpy(read_cache, mem_data->GET_MEM_ACK.data, read_cache_size);

        //Free up the memory data
        delete mem_data;
        mem_data = NULL;
      }
    }


    bool ProcDebugBG::debug_version_msg()
    {
      BG_Debugger_Msg msg(VERSION_MSG, pid, 0, 0, 0);
      msg.header.dataLength = sizeof(msg.dataArea.VERSION_MSG);

      sw_printf("[%s:%u] - Sending VERSION_MSG to pid %d\n", FILE__, __LINE__, pid);
      bool result = BG_Debugger_Msg::writeOnFd(BG_DEBUGGER_WRITE_PIPE, msg);

      if (!result) {
        sw_printf("[%s:%u] - Unable to send VERSION_MSG to process %d\n",
                  FILE__, __LINE__, pid);
        return false;
      }
      return true;
    }

    // ============================================================ //
    // SymtabLibState -- need to differentiate for P
    // ============================================================ //
     bool LibraryState::updateLibsArch(vector<pair<LibAddrPair, unsigned int> > &)
     {
        return true;
     }


    // ============================================================ //
    // ThreadState
    // ============================================================ //
    ThreadState* ThreadState::createThreadState(ProcDebug *parent, Dyninst::THR_ID tid, bool) {
      THR_ID thread_id = tid;
      if (thread_id == NULL_THR_ID) {
        thread_id = 0;
      }
      BGThreadState *ts = new BGThreadState(parent, thread_id);
      return ts;
    }

    void BottomOfStackStepperImpl::initialize()
    {
    }

     Dyninst::Architecture ProcDebug::getArchitecture()
     {
        return Dyninst::Arch_ppc32;
     }

     void BottomOfStackStepperImpl::newLibraryNotification(LibAddrPair *, lib_change_t change)
     {
        if (change == library_unload)
           return;
        if (!libthread_init || !aout_init) {
           initialize();
        }
     }

     SymbolReaderFactory *getDefaultSymbolReader()
     {
        static SymElfFactory symelffact;
        if (!Walker::symrfact)
           Walker::symrfact = (SymbolReaderFactory *) &symelffact;
        return Walker::symrfact;
     }
  } // namespace Stackwalker
} // namespace Dyninst

