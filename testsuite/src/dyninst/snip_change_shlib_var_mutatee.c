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

static volatile int _unused; /* move decl here to dump compiler warning - jkh */

volatile int gv_scsv1 = 0;

int scsv1(int x)
{
	return 8 * x;
}

int snip_change_shlib_var_mutatee()
{
	/*  mutator instruments scsv1 entry and exit to (1) modify a global variable
       in a shared library (libtestB), and (2) to call a function in that library
       that returns 1 if the change was successful and 0 otherwise.  The result
       of this function call is put in gv_scsv1; */

	int dont_care = 0;
	dont_care = scsv1(5);

	if (1 != gv_scsv1)
	{
      logerror("Failed snip_change_shlib_var test\n");
	  return -1;
	}

	logerror("Passed snip_change_shlib_var test\n");
	test_passes(testname);

	return 0;
}

