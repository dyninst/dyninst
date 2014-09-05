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
#include <stdlib.h>

#if defined(os_windows_test)
#define DLLEXPORT __declspec( dllexport )
#else
#define DLLEXPORT
#endif

#if defined(os_windows_test) && defined(__cplusplus)
extern "C" 
{
#endif
DLLEXPORT int snip_change_shlib_var = 20;

int snip_ref_shlib_var1 = 5;
long snip_ref_shlib_var2 = 5L;
char snip_ref_shlib_var3 = 'e';
char * snip_ref_shlib_var4 = (char*)0x5;
float snip_ref_shlib_var5 = 5.5e5;
double snip_ref_shlib_var6 = 5.5e50;

DLLEXPORT int check_snip_change_shlib_var()
{
	if (snip_change_shlib_var == 777)
		return 1;
	else
	{
		fprintf(stderr, "%s[%d]:  bad value for check_snip_change_shlib_var = %d, not 777\n",
				__FILE__, __LINE__, snip_change_shlib_var);
	}
	return 0;
}
	

/* These are copied in test1.mutatee.c and libtestA.c */
#define MAGIC22_1   2200100
#define MAGIC22_2   2200200
#define MAGIC22_3   2200300
#define MAGIC22_4   2200400
#define MAGIC22_5A  2200510
#define MAGIC22_5B  2200520
#define MAGIC22_6   2200600
#define MAGIC22_7   2200700

DLLEXPORT int call22_5b(int x)
{
    return x + MAGIC22_5B;
}
/* function to make regex (test 21)search non-trivial */
DLLEXPORT void cbll21_1()
{
     printf("This function was not meant to be called!\n");
}
/* function to make regex (test 21)search non-trivial */
DLLEXPORT void cbll22_1()
{
     printf("This function was not meant to be called!\n");
}

/* function to make regex (test21) search non-trivial */
DLLEXPORT void acbll22_1()
{
     printf("This function was not meant to be called!\n");
}

/* Keep this function at the end of this file to kludgily ensure that
   its base address differs from its counterpart in libtestA.c */
DLLEXPORT void call21_1()
{
     printf("This function was not meant to be called!\n");
}

#ifdef NOTDEF 
/* monitoring call (and associated variables) for test 40 */
extern unsigned gv40_call40_1_addr;
extern unsigned gv40_call40_2_addr;
extern unsigned gv40_call40_3_addr;
extern unsigned gv40_call40_5_addr1;
extern unsigned gv40_call40_5_addr2;
extern unsigned gv40_call40_5_addr3;
int call_counter = 0;
void func_40_monitorFunc(unsigned int callee_addr, unsigned int callsite_addr)
{
  if (call_counter == 0) {
    gv40_call40_5_addr1 = callsite_addr;
    gv40_call40_1_addr = callee_addr;
    call_counter++;
    return;
  }
  if (call_counter == 1) {
    gv40_call40_5_addr2 = callsite_addr;
    gv40_call40_2_addr = callee_addr;
    call_counter++;
    return;
  }
  if (call_counter == 2) {
    gv40_call40_5_addr3 = callsite_addr;
    gv40_call40_3_addr = callee_addr;
    call_counter++;
    return;
  }
   fprintf(stderr, "%s[%d]:  FIXME!\n", __FILE__, __LINE__);
  return;
}
#endif

#if defined(os_windows_test) && defined(__cplusplus)
}
#endif
