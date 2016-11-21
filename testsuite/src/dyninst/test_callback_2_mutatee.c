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
#include "mutatee_util.h"
#include "solo_mutatee_boilerplate.h"
#include "test12.h"


/* Externally accessed function prototypes.  These must have globally unique
 * names.  I suggest following the pattern <testname>_<function>
 */

int test_callback_2_call1();

/* Global variables accessed by the mutator.  These must have globally unique
 * names.
 */

volatile int test_callback_2_idle = 0;

/* Internally used function prototypes.  These should be declared with the
 * keyword static so they don't interfere with other mutatees in the group.
 */

/* Global variables used internally by the mutatee.  These should be declared
 * with the keyword static so they don't interfere with other mutatees in the
 * group.
 */

/* Function definitions follow */

int call7_2(int x)
{
  int i;
  int y = x;
  for (i = 0; i < 0xfff; ++i) 
    y += i;
  
  return y;
}

int test_callback_2_call1() 
{
  int x = 0;
  int z = 0;
  int i;

  for (i = 0; i < TEST7_NUMCALLS; ++i) {
    z += call7_2(x); 
  }
  return z;
}

void func7_1()
{
  /* This is a simple single threaded scenario for user defined callback
   * testing.  The entry, exit, and call points of test_callback_2_call1 are
   * instrumented with messaging functions.
   */
  int x = 0;
  x = test_callback_2_call1();
  /*  sleep to make sure that all the async calls have had a chance to be received
      before the process terminates.  (under ordinary circumstances process exits
      take precedence in handling to any asynchronous events) */
  sleep(5);
#if 0
  while (test_callback_2_idle == 0); /* Loop while *not* idle? */
#endif
}

int test_callback_2_mutatee() {
  func7_1();
  return 0; /* Return code is not checked */
}
