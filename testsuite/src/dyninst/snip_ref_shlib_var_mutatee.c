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

#if 0
//  from libtestB -- the expected end values
int snip_ref_shlib_var1 = 5;
long snip_ref_shlib_var2 = 5L;
char snip_ref_shlib_var3 = 'e';
char * snip_ref_shlib_var4 = 0x5;
float snip_ref_shlib_var5 = 5.5e5;
double snip_ref_shlib_var6 = 5.5e50;
#endif

volatile int gv_srsv1 = 0;
volatile long gv_srsv2 = 0L;
volatile char gv_srsv3 = '\0';
volatile char *gv_srsv4 = NULL;
volatile float gv_srsv5 = 0.0;
volatile double gv_srsv6 = 0.0;

int srsv1(int x)
{
	return 8 * x;
}

int snip_ref_shlib_var_mutatee()
{
	int failed = 0;
	/*  mutator instruments srsv1 entry with assignments from variables in 
	    a shared library (libtestB) */

	int dont_care = 0;
	dont_care = srsv1(5);

	if (gv_srsv1 != 5) {failed = 1; goto finish_up;}
	if (gv_srsv2 != 5L) {failed = 2; goto finish_up;}
	if (gv_srsv3 != 'e') {failed = 3; goto finish_up;}
	if (gv_srsv4 != (char *)0x5) {failed = 4; goto finish_up;}
	if (gv_srsv5 != 5.5e5) {failed = 5; goto finish_up;}
#if 0
	if (gv_srsv6 != 5.5e50) {failed = 6; goto finish_up;}
#endif

finish_up:
	if (failed)
	{
      logerror("Failed snip_ref_shlib_var test with code %d\n", failed);
	  logerror("\tgv_srsv1 = %d\n", gv_srsv1);
	  logerror("\tgv_srsv2 = %lu\n", gv_srsv2);
	  logerror("\tgv_srsv3 = %c\n", gv_srsv3);
	  logerror("\tgv_srsv4 = %p\n", gv_srsv4);
	  logerror("\tgv_srsv5 = %f\n", gv_srsv5);
#if 0
	  logerror("\tgv_srsv6 = %e\n", gv_srsv6);
#endif
	  return -1;
	}

	logerror("Passed snip_ref_shlib_var test\n");
	test_passes(testname);

	return 0;
}

