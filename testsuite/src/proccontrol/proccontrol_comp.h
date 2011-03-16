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

#if !defined(pcontrol_comp_h_)
#define pcontrol_comp_h

#include "test_lib.h"
#include "TestMutator.h"
#include "Process.h"
#include "Event.h"

#include <vector>

using namespace Dyninst;
using namespace ProcControlAPI;

#define NUM_PARALLEL_PROCS 8

class ProcControlComponent : public ComponentTester
{
private:
   bool setupServerSocket();
   bool acceptConnections(int num, int *attach_sock);
   bool cleanSocket();
   Process::ptr launchMutatee(RunGroup *group, ParameterDict &param);
   bool launchMutatees(RunGroup *group, ParameterDict &param);

public:
   int sockfd;
   char *sockname;
   int notification_fd;

   int num_processes;
   int num_threads;

   bool curgroup_self_cleaning;

   std::map<Process::ptr, int> process_socks;
   std::map<Dyninst::PID, Process::ptr> process_pids;
   std::vector<Process::ptr> procs;
   std::map<EventType, std::vector<Event::const_ptr>, eventtype_cmp > eventsRecieved;

   ParamPtr me;

   ProcControlComponent();
   virtual ~ProcControlComponent();

   bool recv_broadcast(unsigned char *msg, unsigned msg_size);
   bool send_broadcast(unsigned char *msg, unsigned msg_size);
   bool recv_message(unsigned char *msg, unsigned msg_size, int sfd);
   bool recv_message(unsigned char *msg, unsigned msg_size, Process::ptr p);
   bool send_message(unsigned char *msg, unsigned msg_size, int sfd);
   bool send_message(unsigned char *msg, unsigned msg_size, Process::ptr p);
   bool block_for_events();
   
   bool registerEventCounter(EventType et);

   bool checkThread(const Thread &thread);

   virtual test_results_t program_setup(ParameterDict &params);
   virtual test_results_t program_teardown(ParameterDict &params);
   virtual test_results_t group_setup(RunGroup *group, ParameterDict &params);
   virtual test_results_t group_teardown(RunGroup *group, ParameterDict &params);
   virtual test_results_t test_setup(TestInfo *test, ParameterDict &parms);
   virtual test_results_t test_teardown(TestInfo *test, ParameterDict &parms);
   virtual std::string getLastErrorMsg();
};

// Base class for the mutator part of a test
class COMPLIB_DLL_EXPORT ProcControlMutator : public TestMutator {
public:
  ProcControlMutator();
  virtual test_results_t setup(ParameterDict &param);
  virtual ~ProcControlMutator();
  ProcControlComponent *comp;
};

extern "C" {
	TEST_DLL_EXPORT TestMutator *TestMutator_factory();
}

extern "C"  {
   TEST_DLL_EXPORT ComponentTester *componentTesterFactory();
}

#endif


