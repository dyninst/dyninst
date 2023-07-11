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

#if !defined(INT_EVENT_H_)
#define INT_EVENT_H_
#if defined(cap_thread_db)
#include "int_thread_db.h"
#endif
#include "response.h"
#include "resp.h"
#include <stddef.h>
#include <string>
#include <vector>
#include <set>

namespace Dyninst {
namespace ProcControlAPI {

class int_eventBreakpoint
{
  public:
   int_eventBreakpoint(Address a, sw_breakpoint *i, int_thread *thr);
   int_eventBreakpoint(hw_breakpoint *i, int_thread *thr);
   ~int_eventBreakpoint();
   bp_instance *lookupInstalledBreakpoint();

   //Only one of addr or hwbp will be set
   Dyninst::Address addr;
   hw_breakpoint *hwbp;

   result_response::ptr pc_regset;
   int_thread *thrd;
   bool stopped_proc;

   std::set<Breakpoint::ptr> cb_bps;
};

class int_eventBreakpointClear
{
  public:
   int_eventBreakpointClear();
   ~int_eventBreakpointClear();

   std::set<response::ptr> bp_suspend;
   bool started_bp_suspends;
   bool cached_bp_sets;
   bool set_singlestep;
   bool stopped_proc;

   std::set<Thread::ptr> clearing_threads;
};
 
class int_eventBreakpointRestore
{
  public:
   int_eventBreakpointRestore(bp_instance *breakpoint_);
   ~int_eventBreakpointRestore();

   bool set_states;
   bool bp_resume_started;
   std::set<response::ptr> bp_resume;
   bp_instance *bp;
};

class int_eventRPC {
  public:
   int_eventRPC();
   ~int_eventRPC();
   reg_response::ptr alloc_regresult;
   result_response::ptr memrestore_response;
   result_response::ptr regrestore_response;

   void getPendingAsyncs(std::set<response::ptr> &pending);
};

class int_eventAsync {
  private:
   std::set<response::ptr> resp;
  public:
   int_eventAsync(response::ptr r);
   ~int_eventAsync();

   std::set<response::ptr> &getResponses();
   void addResp(response::ptr r);
};

class int_eventNewUserThread {
  public:
   int_eventNewUserThread();
   ~int_eventNewUserThread();

   int_thread *thr;
   Dyninst::LWP lwp;
   void *raw_data;
   bool needs_update;
};

class int_eventNewLWP {
  public:
   int_eventNewLWP();
   ~int_eventNewLWP();

   Dyninst::LWP lwp;
   int_thread::attach_status_t attach_status;
};

class int_eventThreadDB {
  public:
   int_eventThreadDB();
   ~int_eventThreadDB();
   
   std::set<Event::ptr> new_evs;
   bool completed_new_evs;

   bool completed_getmsgs;
#if defined(cap_thread_db)
   std::vector<td_event_msg_t> msgs;
   std::vector<td_thrhandle_t> handles;
#endif
};

class int_eventDetach {
  public:
   int_eventDetach();
   ~int_eventDetach();

   std::set<response::ptr> async_responses;
   result_response::ptr detach_response;
   bool temporary_detach;
   bool leave_stopped;
   bool removed_bps;
   bool done;
   bool had_error;
};

class int_eventControlAuthority {
  public:
   int_eventControlAuthority(std::string toolname_, unsigned int toolid_, int priority_, EventControlAuthority::Trigger trigger_);
   int_eventControlAuthority();
   ~int_eventControlAuthority();
   std::string toolname;
   unsigned int toolid;
   int priority;
   EventControlAuthority::Trigger trigger;
   bool control_lost;
   bool handled_bps;
   bool took_ca;
   bool did_desync;
   bool unset_desync;
   bool dont_delete;
   bool waiting_on_stop;
   std::set<response::ptr> async_responses;
   data_response::ptr dresp;
};

class int_eventAsyncFileRead {
  public:
   int_eventAsyncFileRead();
   ~int_eventAsyncFileRead();
   bool isComplete();

   void *data;
   size_t size;
   size_t orig_size;
   void *to_free;
   std::string filename;
   size_t offset;
   int errorcode;
   bool whole_file;
   Resp::ptr resp;
};

class int_eventAsyncIO {
  public:
   enum asyncio_type {
      memread,
      memwrite,
      regallread,
      regallwrite
   };

   int_eventAsyncIO(response::ptr resp_, asyncio_type);
   ~int_eventAsyncIO();

   response::ptr resp;
   void *local_memory;
   Address remote_addr;
   size_t size;
   void *opaque_value;
   asyncio_type iot;
   RegisterPool *rpool;
};

}
}

#endif
