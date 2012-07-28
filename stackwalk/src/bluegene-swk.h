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

#ifndef BLUEGENE_SWK_H
#define BLUEGENE_SWK_H

#include <algorithm>
#include "procstate.h"
#include "stl_utils.h"

#ifndef os_bg_ion
#error "ERROR: bluegene-swk.h is only for BlueGene ION builds."
#endif // os_bg_ion

// TODO: these bg debugger headers could be merged w/ifdefs in the headers.
// TODO: this would make things a little less nasty here.
#if   defined(os_bgl)
#include "external/bluegene/bgl-debugger-interface.h"
#elif defined (os_bgp)
#include "external/bluegene/bgp-debugger-interface.h"
#else
#error "ERROR: No suitable debug interface for this BG ION."
#endif

#define SINGLE_STEP_SIG 32064

#if !defined(BG_READCACHE_SIZE)
#define BG_READCACHE_SIZE 2048
#endif

namespace DebuggerInterface {
  class DebugEvent;
}

namespace Dyninst {
  namespace Stackwalker {
    
    ///
    /// This is a base class for the BlueGene 3rd-party stackwalkers.  
    /// 
    /// Common functionality is implemented here, and subclasses should
    /// provide other features such as threads and dynamic library handling.
    ///
    class ProcDebugBG : public ProcDebug {
    protected:
      virtual bool debug_continue(ThreadState *);
      virtual bool version_msg();
      virtual bool debug_waitfor_version_msg();
      virtual bool debug_handle_event(DebugEvent ev);
      virtual bool debug_handle_signal(DebugEvent *ev);
      virtual bool debug_attach(ThreadState *);
      virtual bool debug_pause(ThreadState *);
      virtual bool debug_continue_with(ThreadState *, long sig);
      virtual bool debug_version_msg();

      // default implementation: this is a no-op.  Subclasses should override.
      virtual bool pollForNewThreads();

      // unsupported -- stub will fail!
      virtual bool debug_create(std::string, const std::vector<std::string> &);

      ///
      /// This implements basic BG debugger event -> stackwalker event translation.  It's
      /// called by the BG implementation of ProcDebug::debug_get_event().
      /// 
      /// Subclasses (future BG architecture implementations) should override to handle new 
      /// event types, then delegate here to handle old ones.
      ///
      friend DebugEvent ProcDebug::debug_get_event(bool);
      virtual void translate_event(const DebuggerInterface::BG_Debugger_Msg& msg, 
                                   DebugEvent& ev);

      /// BG currently has no asynchronous thread control, so the threads are stopped and
      /// continued all together.  This applies operations to all threads.
      template<class Functor> void for_all_threads(Functor f) {
        for_each(threads.begin(), threads.end(), do_to_second(f));
      }

    public:
      virtual bool detach(bool leave_stopped);
      virtual bool getThreadIds(std::vector<Dyninst::THR_ID> &threads);
      virtual bool getDefaultThread(Dyninst::THR_ID &default_tid);
      virtual unsigned getAddressWidth();
      virtual bool getRegValue(Dyninst::MachRegister reg, Dyninst::THR_ID thread, Dyninst::MachRegisterVal &val);
      virtual bool setRegValue(Dyninst::MachRegister reg, Dyninst::THR_ID thread, Dyninst::MachRegisterVal val);
      virtual bool readMem(void *dest, Dyninst::Address source, size_t size);
      virtual bool isLibraryTrap(Dyninst::THR_ID thrd);
      virtual bool cleanOnDetach();
    protected:
      ProcDebugBG(Dyninst::PID pid, std::string exe);
      virtual ~ProcDebugBG();

      /// Creation function for making platform-specific ProcDebugBG
      /// Need to include an implementation of this in any build that includes 
      /// ProcDebugBG.
      static ProcDebug *createProcDebugBG(PID pid, std::string executable);

      /// Clears this process's memory read cache along with register caches 
      /// from all of its threads.
      virtual void clear_cache();

      // Protected Data
      DebuggerInterface::BG_Debugger_Msg::DataArea *mem_data;    // data from memory reads.

      unsigned char *read_cache;
      Dyninst::Address read_cache_start;
      unsigned read_cache_size;
      bool detached;
      bool write_ack;
      bool wrote_trap;
    };

    // helper for dealing with filehandles
    bool bg_fdHasData(int fd);

    //
    // Like a regular ThreadState, but with a per-thread register cache.
    // Also porvides ability to set thread id to account for the way the 
    // BG debug interface handles threads.
    //
    class BGThreadState : public ThreadState {
    private:
      friend class ProcDebug;
      void setTid(Dyninst::THR_ID id) { tid = id; }

    public:
      BGThreadState(ProcDebug *parent, Dyninst::THR_ID id) 
        : ThreadState(parent, id), gprs_set(false) 
      { }

      virtual ~BGThreadState() { }

      DebuggerInterface::BG_GPRSet_t gprs;   // Per-thread register cache.
      bool gprs_set;                         // Valid flag for register cache.
      bool write_ack;
    };

    /// Copy one thread's state to another.  
    void copy_thread_state(ThreadState *source, ThreadState *dest);

  } // Stackwalker
} // Dyninst

#endif // BLUEGENE_SWK_H
