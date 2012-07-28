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
#ifndef TEST_DATA_H
#define TEST_DATA_H

#include "test_lib_dll.h"
#include <vector>

enum start_state_t {
   // Start up the mutatee and pass it to the test in a stopped state
   STOPPED,
   // Start up the mutatee and start it before passing it to the test
   RUNNING,
   // Start mutatee stopped normally, but do not trigger attach ack
   SELFATTACH,
   // Start mutatee and create the attach pipe, but delay attach until 
   // test does so explicitly
   DELAYEDATTACH,
   // Allow the test to setup the mutatee itself
   SELFSTART
};

enum create_mode_t {
   CREATE = 0,
   USEATTACH,
   DISK,
   DESERIALIZE //Keep deserialize last to maintain proper sort
};

enum cleanup_mode_t {
   // If mutator passes collect exit code from the mutatee, kill mutatee 
   //    if mutator fails 
   COLLECT_EXITCODE,
   // Don't collect exit code from mutatee, just kill it
   KILL_MUTATEE,
   // The test contains it's own cleanup code
   NONE,
};

enum enabled_t {
   DISABLED = 0,
   ENABLED = 1,
};

enum grouped_test_t {
  GROUPED,
  SOLO
};

typedef std::vector<char*> mutatee_list_t;

typedef struct {
   bool i386_unknown_linux2_4;
   bool _i386_unknown_nt4_0_test; 
   bool _x86_64_unknown_linux2_4;
   bool _rs6000_ibm_aix5_1;
} platforms_t;

struct TESTLIB_DLL_EXPORT TestData {
   char *name;
   char *soname;
   mutatee_list_t &mutatee;
   platforms_t &platforms;
   start_state_t state;

   int oldtest;
   int subtest;
   cleanup_mode_t cleanup;
   create_mode_t useAttach;
   enabled_t enabled;
   grouped_test_t grouped;

   TestData(char *name, 
         char *soname, 
         mutatee_list_t &mutatee, 
         platforms_t &platforms,
         start_state_t state,
         int oldtest,
         int subtest,
         cleanup_mode_t cleanup,
         create_mode_t useAttach,
	 enabled_t enabled,
	 grouped_test_t grouped
         );
};

typedef TestData test_data_t;

#endif /* TEST_DATA_H */
