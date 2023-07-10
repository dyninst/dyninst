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
#if !defined(GENERATOR_WINDOWS_H)
#define GENERATOR_WINDOWS_H

#include "Generator.h"
#include "Event.h"
#include "int_process.h"
#include <sys/types.h>
#include <vector>
#include <map>
#include <deque>


using namespace Dyninst;
using namespace ProcControlAPI;


class GeneratorWindows : public GeneratorMT
{
 public:
   GeneratorWindows();
   virtual ~GeneratorWindows();

   virtual bool initialize();
   virtual bool canFastHandle();
   virtual ArchEvent *getEvent(bool block);

   virtual bool plat_continue(ArchEvent* evt);
   virtual void plat_start();
   enum start_mode {
	   create,
	   attach
   };
   struct StartInfo
   {
	   start_mode mode;
	   int_process* proc;
   };
   struct processData
   {
	   typedef boost::shared_ptr<processData> ptr;
	   bool unhandled_exception;
	   int_process* proc;
	   state_t state;
	   processData(const processData& o) :
		unhandled_exception(o.unhandled_exception),
			proc(o.proc),
			state(o.state) {}
		processData() : unhandled_exception(false),
			proc(NULL),
			state(none) {}
   };

   void markUnhandledException(Dyninst::PID p);
   void enqueue_event(start_mode m, int_process* p);
   std::deque<StartInfo> procsToStart;
   std::map<int, processData::ptr> thread_to_proc;
   virtual bool isExitingState();
   virtual void setState(state_t newstate);
   virtual state_t getState();
   virtual bool hasLiveProc();
   void removeProcess(int_process* proc);

   virtual ArchEvent* getCachedEvent();
   virtual void setCachedEvent(ArchEvent* ae);
   std::map<int, ArchEvent*> m_Events;
   std::map<Dyninst::PID, long long> alreadyHandled;
};

#endif // !defined(GENERATOR_WINDOWS_H)
