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
#include "stackwalk/src/bluegenep-swk.h"

#include "stackwalk/h/swk_errors.h"
#include "stackwalk/h/symlookup.h"
#include "stackwalk/h/walker.h"
#include "stackwalk/h/framestepper.h"
#include "stackwalk/h/procstate.h"
#include "stackwalk/src/symtab-swk.h"
#include "stackwalk/src/dbgstepper-impl.h"

#include "get_trap_instruction.h"

#include <string>
#include <algorithm>

#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/poll.h>
#include <assert.h>

using namespace Dyninst;
using namespace Dyninst::Stackwalker;
using namespace DebuggerInterface;
using namespace std;

namespace Dyninst {
  namespace Stackwalker {
    
    // ============================================================ //
    // ProcDebugBG
    // This is the factor function that initially creates a BGP stackwalker.
    // ============================================================ //
    ProcDebug *ProcDebugBG::createProcDebugBG(PID pid, string executable) {
      return new ProcDebugBGP(pid, executable);
    }

    
    // ============================================================ //
    // ProcDebugBGP
    // ============================================================ //
    ProcDebugBGP::ProcDebugBGP(PID pid, string exe) 
      : ProcDebugBG(pid, exe),
        lib_load_trap(0) { }
    
    ProcDebugBGP::~ProcDebugBGP() { }


    // this is based on the linux version.  BG/P uses Linux SysV dynamic libs
    bool ProcDebugBGP::debug_post_attach(ThreadState *) {
      sw_printf("[%s:%u] - Entering debug_post_attach on %d\n", FILE__, __LINE__, pid);

      pollForNewThreads();

      // make new thread states identical to the initial thread, since BG has no 
      // asynchronous thread control.  They should all be doing the same thing.
      for (thread_map_t::iterator i=threads.begin(); i != threads.end(); i++) {
        ThreadState *ts = i->second;
        if (ts->state() == ps_neonatal) {  // only do this to new threads.
          copy_thread_state(initial_thread, ts);
        }
      }

      if (!library_tracker)
         setDefaultLibraryTracker();
      // This should really be checking for exceptions, since the above are constructors.
      if (!library_tracker) {
        sw_printf("[%s:%u] - PID %d failed to create library tracker\n", FILE__, __LINE__, pid);
        setLastError(err_nolibtracker, "Failed to create library tracker!");
        return false;
      }

      sw_printf("[%s:%u] - PID %d registering lib spotter...\n", FILE__, __LINE__, pid);
      registerLibSpotter();
      sw_printf("[%s:%u] - PID %d registered lib spotter successfully\n", FILE__, __LINE__, pid);
      return true;
    }


    //
    // Dynamic library methods
    //
    bool ProcDebugBGP::isLibraryTrap(Dyninst::THR_ID thrd) {
      LibraryState *ls = getLibraryTracker();
      if (!ls)
        return false;
      Address lib_trap_addr = ls->getLibTrapAddress();
      if (!lib_trap_addr)
        return false;
      
      Dyninst::MachRegisterVal cur_pc;
      bool result = getRegValue(Dyninst::ReturnAddr, thrd, cur_pc);
      if (!result) {
        sw_printf("[%s:%u] - Error getting PC value for thrd %d\n",
                  FILE__, __LINE__, (int) thrd);
        return false;
      }
      if (cur_pc == lib_trap_addr || cur_pc-1 == lib_trap_addr)
        return true;
      return false;
    }

    bool ProcDebugBGP::cleanOnDetach() {
      bool result = pause();
      if (!result) {
	sw_printf("[%s:%u] - Failed to pause process for detach clean\n");
      }

      if (!wrote_trap)
      {
         sw_printf("[%s:%u] - Skipping trap clean\n", FILE__, __LINE__);
         return true;
      }

      lib_load_trap = library_tracker->getLibTrapAddress();
      if (!lib_load_trap) {
        sw_printf("[%s:%u] - Couldn't get trap addr, couldn't set up "
                  "library loading notification.\n", FILE__, __LINE__);
        return false;
      }

      // construct a bg debugger msg to clean out the trap 
      BG_Debugger_Msg write_trap_msg(SET_MEM, pid, 0, 0, 0);
      write_trap_msg.header.dataLength = sizeof(write_trap_msg.dataArea.SET_MEM);
      memcpy(write_trap_msg.dataArea.SET_MEM.data, lib_trap_orig_mem, trap_len);
      write_trap_msg.dataArea.SET_MEM.addr = lib_load_trap;
      write_trap_msg.dataArea.SET_MEM.len  = trap_len;

      sw_printf("[%s:%u] - Restoring memory over library trap at %lx\n", 
                  FILE__, __LINE__, lib_load_trap);
      write_ack = false;
      if (!BG_Debugger_Msg::writeOnFd(BG_DEBUGGER_WRITE_PIPE, write_trap_msg))
      {
        sw_printf("[%s:%u] - Couldn't write BG trap at load address: %lx\n", 
                  FILE__, __LINE__, lib_load_trap);
        return false;
      }

      bool handled;
      while (!write_ack) {
	result = debug_wait_and_handle(true, false, handled);
	if (result) {
	  sw_printf("[%s:%u] - Unable to get ack for setmem in " 
		    "registerLibSpotter()\n", FILE__, __LINE__);
	  break;
	}
      }
      
      return true;
    }

    void ProcDebugBGP::registerLibSpotter() 
    {
      if (lib_load_trap)
        return;

      if (!library_tracker) {
        sw_printf("[%s:%u] - Not using lib tracker, don't know how "
                  "to get library load address\n", FILE__, __LINE__);
        return;
      }
   
      lib_load_trap = library_tracker->getLibTrapAddress();
      if (!lib_load_trap) {
        sw_printf("[%s:%u] - Couldn't get trap addr, couldn't set up "
                  "library loading notification.\n", FILE__, __LINE__);
        return;
      }

      // construct a bg debugger msg to install a trap instruction at the loader trap address.
      BG_Debugger_Msg write_trap_msg(SET_MEM, pid, 0, 0, 0);

      getTrapInstruction((char*)write_trap_msg.dataArea.SET_MEM.data,
                         BG_Debugger_Msg_MAX_MEM_SIZE, trap_len, true);
      write_trap_msg.header.dataLength = sizeof(write_trap_msg.dataArea.SET_MEM);
      write_trap_msg.dataArea.SET_MEM.addr = lib_load_trap;
      write_trap_msg.dataArea.SET_MEM.len  = trap_len;

      sw_printf("[%s:%u] - Reading original memory at library trap\n",
		FILE__, __LINE__);
      assert(trap_len <= sizeof(lib_trap_orig_mem));
      memset(lib_trap_orig_mem, 0, sizeof(lib_trap_orig_mem));
      bool result = readMem(lib_trap_orig_mem, lib_load_trap, trap_len);
      if (!result) {
	sw_printf("Failed to read memory for library trap\n");
	return;
      }

      sw_printf("[%s:%u] - Installing BG trap at load address: %lx\n", 
                  FILE__, __LINE__, lib_load_trap);
      write_ack = false;
      if (!BG_Debugger_Msg::writeOnFd(BG_DEBUGGER_WRITE_PIPE, write_trap_msg))
      {
        sw_printf("[%s:%u] - Couldn't write BG trap at load address: %lx\n", 
                  FILE__, __LINE__, lib_load_trap);
        return;
      }

      bool handled;
      while (!write_ack) {
	result = debug_wait_and_handle(true, false, handled);
	if (result) {
        sw_printf("[%s:%u] - Unable to get ack for setmem in registerLibSpotter()\n", FILE__, __LINE__);
	  break;
	}
      }
 
      wrote_trap = true;
      sw_printf("[%s:%u] - Successfully wrote BG library trap at %lx\n",
                FILE__, __LINE__, lib_load_trap);
    }



    //
    // Threading methods
    //
    bool ProcDebugBGP::getThreadIds(std::vector<THR_ID> &output) {
      output.clear();
      transform(threads.begin(), threads.end(), back_inserter(output), get_first());
      return true;
    }


    bool ProcDebugBGP::pollForNewThreads() {
      sw_printf("[%s:%u] - Polling for new threads on %d\n", FILE__, __LINE__, pid);

      // send a request for thread info to the BG debugger pipe
      BG_Debugger_Msg get_info_msg(GET_THREAD_INFO, pid, 0, 0, 0);
      get_info_msg.header.dataLength = sizeof(get_info_msg.dataArea.GET_THREAD_INFO);

      if (!BG_Debugger_Msg::writeOnFd(BG_DEBUGGER_WRITE_PIPE, get_info_msg)) {
        sw_printf("[%s:%u] - Error writing GET_THREAD_INFO to BG debug stream.\n", 
                  FILE__, __LINE__);
        setLastError(err_procread, "Failed to write GET_THREAD_INFO request.");
        return false;
      }
      
      // get a get_thread_info_ack from the read pipe, but do it through the 
      // stackwalker debug interface.
      if (!debug_waitfor(dbg_thread_info)) {
        sw_printf("[%s:%u] - Unable to get ack for GET_THREAD_INFO in pollForNewThreads()\n", 
                  FILE__, __LINE__);
      }

      return true;
    }


    void ProcDebugBGP::translate_event(const BG_Debugger_Msg& msg, DebugEvent& ev) {
      ThreadState *ts = ev.thr;
      switch (msg.header.messageType) {
      case SIGNAL_ENCOUNTERED:
        // this is special handling for library loads, which we'll only see on BGP
	ev.data.idata = msg.dataArea.SIGNAL_ENCOUNTERED.signal;
        if (ev.data.idata == SIGTRAP && isLibraryTrap(ts->getTid())) {
          ev.dbg = dbg_libraryload;
          sw_printf("[%s:%u] - Decoded library load event\n", FILE__, __LINE__);
        } else {
          ProcDebugBG::translate_event(msg, ev);
        }
        break;        

      case GET_THREAD_INFO_ACK:
        {
          ev.dbg = dbg_thread_info;
          ev.size = msg.dataArea.GET_THREAD_INFO_ACK.numThreads;
          ev.data.pdata = new uint32_t[ev.size];

          const uint32_t *msg_ids = msg.dataArea.GET_THREAD_INFO_ACK.threadIDS;
          uint32_t *dest = (uint32_t*)ev.data.pdata;
          copy(&msg_ids[0], &msg_ids[ev.size], dest);

          sw_printf("[%s:%u] - Received thread info: ", FILE__, __LINE__);
          for (size_t i=0; i < ev.size; i++) sw_printf("%d ", msg_ids[i]);
          sw_printf("\n");
        }
        break;

      default:
        // delegate other events to base class
        ProcDebugBG::translate_event(msg, ev);
      }
    }


    bool ProcDebugBGP::debug_handle_event(DebugEvent ev) {
      ThreadState *ts = ev.thr;
      THR_ID tid = ts->getTid();
      
      switch (ev.dbg) {
      case dbg_libraryload:
        {
          sw_printf("[%s:%u] - Handling library load event on %d/%d\n", FILE__, __LINE__, pid, tid);
          for_all_threads(set_stopped(true));
          LibraryState *ls = getLibraryTracker();
          if (!ls) {
            sw_printf("[%s:%u] - WARNING! No library tracker registered on %d/%d\n", 
                      FILE__, __LINE__, pid, tid);
            setLastError(err_nolibtracker, "No library tracker found!");
            return false;
          }
          ls->notifyOfUpdate();
          
	  /**
	   * Forward over the trap instructoin
	   **/
	  Dyninst::MachRegisterVal newpc = ls->getLibTrapAddress() + 4;
	  bool result = setRegValue(Dyninst::ReturnAddr, tid, newpc);
	  if (!result) {
	    sw_printf("[%s:%u] - Error! Could not set PC past trap!\n",
		      FILE__, __LINE__);
	    setLastError(err_internal, "Could not set PC after trap\n");
	    return false;
	  }
          
          if (!debug_continue(ts)) {
            sw_printf("[%s:%u] - Debug continue failed on %d/%d with %d\n", 
                      FILE__, __LINE__, pid, tid, ev.data.idata);
            setLastError(err_internal, "debug_continue() failed after library load.");
            return false;
          }
          return true;
        }    
        break;

      case dbg_thread_info:
        {
          // Create threads for new ids now that we're actually handling thread_info message.
          uint32_t *ids = (uint32_t*)ev.data.pdata;
          bool result = add_new_threads(&ids[0], &ids[ev.size]);
          delete [] ids;
          return result;
        }
        break;

      default:
        // delegate other events to base class
        return ProcDebugBG::debug_handle_event(ev);
      };
    }


    // ============================================================ //
    // DebugStepperImpl
    // ============================================================ //
    bool DebugStepperImpl::isFrameRegister(MachRegister) {
      return false;            // ppc has no frame pointer register.
    }
    
    bool DebugStepperImpl::isStackRegister(MachRegister reg) {
      return reg == BG_GPR1;   // by convention.
    }

  } // namespace Stackwalker
} // namespace Dyninst


