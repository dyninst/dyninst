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

#include <stdio.h>
#include "solo_mutatee_boilerplate.h"

enum sf_enum1 {
	sf1_1 = 20,
	sf1_2 = 40,
	sf1_3 = 60,
	sf1_4 = 80
} sf_e1;

struct sf_mystruct {
	int sf_elem1;
	long sf_elem2;
	char sf_elem3;
	float sf_elem4;
} sf_ms;

typedef int sf_int_array_t [256];
sf_int_array_t sf_my_array;

typedef int * sf_my_intptr_t;

typedef int sf_int_alias_t;

union sf_my_union{
	char *sf_my_str;
	int sf_my_int;
} sf_u1;


void sf_my_test_type_info_func(int foo)
{
	int rr =0;
	sf_my_intptr_t ip = &rr;
	sf_int_alias_t r = fprintf(stderr, "booga booga: %d\n", foo);
	if (r)
	{
		(*ip) = (int) r;
		sf_e1 =  sf1_1;
		sf_u1.sf_my_int = rr;
		sf_ms.sf_elem1 = rr;
		sf_my_array[0] = rr;
	}
}

int test_symtab_ser_funcs_var1;

int test_symtab_ser_funcs_mutatee() {
	test_symtab_ser_funcs_var1 = 1;

	/*If mutatee should run, things go here.*/
	return 0;
}

