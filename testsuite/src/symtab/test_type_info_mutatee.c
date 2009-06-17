/*
 * Copyright (c) 1996-2008 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

enum enum1 {
	ef1_1 = 20, 
	ef1_2 = 40, 
	ef1_3 = 60, 
	ef1_4 = 80
} e1;

struct mystruct {
	int elem1;
	double elem2;
	char elem3;
	float elem4;
} ms;

typedef enum {ef2_1, ef2_2, ef2_3, ef2_4} enum2;

typedef void (*func_ptr_typedef_t)(int);

const char * my_string = "my_string";
typedef int int_array_t [256];

typedef int * my_intptr_t;

typedef int int_alias_t;

union my_union{
	float my_float;
	int my_int;
} u1;

void my_test_type_info_func(int foo)
{
	int_array_t my_array;
	int rr =0;
	my_intptr_t ip = &rr;
	int_alias_t r = fprintf(stderr, "booga booga: %d: %s\n", foo, my_string);
	if (r)
	{
		(*ip) = (int) r;
		e1 =  ef1_1;
		u1.my_int = rr;
		ms.elem1 = rr;
		my_array[0] = rr;
	}
}

void my_test_type_info_func2()
{
   func_ptr_typedef_t myfuncptr2;
	void (*myfuncptr)(int) = my_test_type_info_func;
	(*myfuncptr)(5);
   myfuncptr2 = my_test_type_info_func;
	(*myfuncptr2)(6);
}

int test_type_info_mutatee() 
{
   /*If mutatee should run, things go here.*/
   return 0;
}

