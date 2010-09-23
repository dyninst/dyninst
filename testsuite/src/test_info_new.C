/*
 * Copyright (c) 1996-2009 Barton P. Miller
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

#include "test_info_new.h"
#include <assert.h>

// The constructor for TestInfo
TestInfo::TestInfo(unsigned int i, const char *iname, const char *imrname,
                   const char *isoname, bool _serialize_enable, const char *ilabel) :
	index(i), name(iname), mutator_name(imrname), soname(isoname),
	serialize_enable(_serialize_enable),
	label(ilabel), mutator(NULL), disabled(false), limit_disabled(false),
	enabled(false), result_reported(false)
{
   for (unsigned i=0; i<NUM_RUNSTATES; i++)
   {
      results[i] = UNKNOWN;
   }
}

// Constructor for RunGroup, with an initial test specified
RunGroup::RunGroup(const char *mutatee_name, start_state_t state_init,
                   create_mode_t attach_init, 
                   test_threadstate_t threads_, test_procstate_t procs_, 
                   run_location_t mutator_location_, run_location_t mutatee_location_, 
                   mutatee_runtime_t mutatee_runtime_,
                   test_linktype_t linktype_,
                   bool ex,
                   test_pictype_t pic_,
                   TestInfo *test_init,
                   const char *modname_, const char *compiler_, const char *optlevel_, 
                   const char *abi_)
  : mutatee(mutatee_name), state(state_init), createmode(attach_init),
    customExecution(ex), disabled(false), mod(NULL), modname(modname_),
    threadmode(threads_), procmode(procs_),
    mutator_location(mutator_location_), mutatee_location(mutatee_location_),
    mutatee_runtime(mutatee_runtime_),
    linktype(linktype_),
    pic(pic_),
    compiler(compiler_), optlevel(optlevel_), abi(abi_)
{
  tests.push_back(test_init);
}

// Constructor for RunGroup with no initial test specified
RunGroup::RunGroup(const char *mutatee_name, start_state_t state_init,
                   create_mode_t attach_init, 
                   test_threadstate_t threads_, test_procstate_t procs_,
                   run_location_t mutator_location_, run_location_t mutatee_location_, 
                   mutatee_runtime_t mutatee_runtime_,
                   test_linktype_t linktype_,
                   bool ex,
                   test_pictype_t pic_,
                   const char *modname_,
                   const char *compiler_, const char *optlevel_, 
                   const char *abi_)
  : mutatee(mutatee_name), state(state_init), createmode(attach_init),
    customExecution(ex), disabled(false), mod(NULL), modname(modname_),
    threadmode(threads_), procmode(procs_),
    mutator_location(mutator_location_), mutatee_location(mutatee_location_),
    mutatee_runtime(mutatee_runtime_),
    linktype(linktype_),
    pic(pic_),
    compiler(compiler_), optlevel(optlevel_), abi(abi_)
{
}

// Constructor for RunGroup with no initial test specified
RunGroup::RunGroup(const char *mutatee_name, 
                   start_state_t state_init,
                   create_mode_t attach_init, 
                   bool ex,
                   const char *modname_, 
                   test_pictype_t pic_,
                   const char *compiler_, 
                   const char *optlevel_, 
                   const char *abi_)
  : mutatee(mutatee_name), state(state_init), createmode(attach_init),
    customExecution(ex), disabled(false), mod(NULL), modname(modname_),
    threadmode(TNone), procmode(PNone),
    linktype(DynamicLink),
    pic(pic_),
    compiler(compiler_), optlevel(optlevel_), abi(abi_)
{
}

// RunGroup's destructor clears its vector of tests
RunGroup::~RunGroup() {
   assert(0);
}

TestInfo::~TestInfo() {
   assert(0);
}



