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

/* Externally accessed function prototypes.  These must have globally unique
 * names.  I suggest following the pattern <testname>_<function>
 */

/* Global variables accessed by the mutator.  These must have globally unique
 * names.
 */

/* Internally used function prototypes.  These should be declared with the
 * keyword static so they don't interfere with other mutatees in the group.
 */

/* Global variables used internally by the mutatee.  These should be declared
 * with the keyword static so they don't interfere with other mutatees in the
 * group.
 */

static int mutateeIdle = 0;

/* Function definitions follow */

/********************************************************************/
/********************************************************************/
/***********  Subtest 2:  dynamic callsites */
/********************************************************************/
/********************************************************************/

#define NUM_DYN_CALLS 8 
typedef int (*intFuncArg) (int);
int call2_zero() {return 0;}

int call2_1(int arg) {return arg+1;}
int call2_2(int arg) {return arg+2;}
int call2_3(int arg) {return arg+3;}
int call2_4(int arg) {return arg+4;}

int call2_dispatch(intFuncArg callme, int arg) 
{
  /*dprintf("%s[%d]:  inside call2_dispatch\n", __FILE__, __LINE__);*/
  static int callsite_selector = 0;
  int ret = -1;
  intFuncArg tocall = (intFuncArg) callme;

  ret = call2_zero(); /* lets have a non-dynamic call site here too */

  if (!tocall) {
    logerror("%s[%d]:  FIXME!\n", __FILE__, __LINE__);
    return -1;
  }

  /*  3 dynamic call sites */
  switch (callsite_selector) {
  case 0: ret = (tocall)(arg); callsite_selector++; break;
  case 1: ret = (tocall)(arg+1); callsite_selector++; break;
  case 2: ret = (tocall)(arg+2); callsite_selector = 0; break;
  }

  if (ret) 
    ret = call2_zero(); /* lets have a non-dynamic call site here too */

  return ret;

}

void func2_1()
{
  /*  want to trigger a lot of dynamic calls, and then stop the process. */
  /*  to make sure we test possible race in event handling. */

  int nextfunc = 1;
  unsigned int i;
  for (i = 0; i < NUM_DYN_CALLS; ++i) {
    switch (nextfunc) {
    case 1: call2_dispatch(call2_1, i); nextfunc++; break;
    case 2: call2_dispatch(call2_2, i); nextfunc++; break;
    case 3: call2_dispatch(call2_3, i); nextfunc++; break;
    case 4: call2_dispatch(call2_4, i); nextfunc = 1; break;
    }; 
  }
  
  mutateeIdle = 1;
  while (mutateeIdle);
}

int test_callback_1_mutatee() {
  func2_1();
  return 0; /* Is this return code checked? */
}
