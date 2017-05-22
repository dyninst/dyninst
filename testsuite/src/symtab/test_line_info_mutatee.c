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

/*  NOTE:  the #line preprocessor directive sets line the number for the NEXT 
    line of code, not the current textual (preprocessed) line */

#include "solo_mutatee_boilerplate.h"

#line 1000
int test_line_info_func(int arg1) {
#line 2000
	int local_var1;
	int local_var2;
	int i;
	local_var1 = 10;
	local_var2 = 2;
	for (i = 0; i < arg1; ++i)
	{
		local_var1 += local_var2;
	}
	return local_var1;
}

int test_line_info_mutatee() 
{
	test_line_info_func(1000);
   /*If mutatee should run, things go here.*/
   return 0;
}

