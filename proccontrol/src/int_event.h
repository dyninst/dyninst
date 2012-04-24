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

#include "response.h"
#include <set>

namespace Dyninst {
namespace ProcControlAPI {

class int_eventBreakpoint
{
  public:
   int_eventBreakpoint(Address a, installed_breakpoint *i, int_thread *thr);
   ~int_eventBreakpoint();
   installed_breakpoint *lookupInstalledBreakpoint();
   
   //installed_breakpoint *ibp;
   Dyninst::Address addr;
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

   result_response::ptr memwrite_bp_suspend;
   bool started_bp_suspends;
   bool cached_bp_sets;
   bool set_singlestep;

   std::set<Thread::ptr> clearing_threads;
};
 
class int_eventBreakpointRestore
{
  public:
   int_eventBreakpointRestore(installed_breakpoint *breakpoint_);
   ~int_eventBreakpointRestore();

   bool set_states;
   result_response::ptr memwrite_bp_resume;
   result_response::ptr memwrite_bp_remove;
   installed_breakpoint *bp;
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

class int_eventThreadDB {
  public:
   int_eventThreadDB();
   ~int_eventThreadDB();
   
   std::set<Event::ptr> new_evs;
   bool completed_new_evs;
};

class int_eventDetach {
  public:
   int_eventDetach();
   ~int_eventDetach();

   std::set<response::ptr> async_responses;
   result_response::ptr detach_response;
   bool temporary_detach;
   bool removed_bps;
   bool done;
   bool had_error;
};

}
}
