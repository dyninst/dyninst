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

/* These are copied in test1.mutatee.c and libtestB.c */
#define MAGIC22_1   2200100
#define MAGIC22_2   2200200
#define MAGIC22_3   2200300
#define MAGIC22_4   2200400
#define MAGIC22_5A  2200510
#define MAGIC22_5B  2200520
#define MAGIC22_6   2200600
#define MAGIC22_7   2200700

#if defined(os_windows_test)
#define DLLEXPORT __declspec( dllexport )
#else
#define DLLEXPORT
#endif

#if defined(os_windows_test) && defined(__cplusplus)
extern "C" {
#endif

/* Keep this function at the start of this file to kludgily ensure
// that its base address differs from its counterpart in libtestB.c
*/
DLLEXPORT int call21_1()
{
     printf("This function was not meant to be called!\n");
     return -1;
}

DLLEXPORT int call22_4(int x)
{
    return x + MAGIC22_4;
}

DLLEXPORT int call22_5a(int x)
{
    return x + MAGIC22_5A;
}

DLLEXPORT int call22_6(int x)
{
    return x + MAGIC22_6;
}

DLLEXPORT int relocation_test_variable1 = 5;
DLLEXPORT int relocation_test_variable2 = 6;

DLLEXPORT int relocation_test_function1(int x)
{
    return x * relocation_test_variable1;
}

DLLEXPORT int relocation_test_function2(int x)
{
    return x * relocation_test_variable2;
}

DLLEXPORT void test1_14_call2_libA(int *var) {
    *var = 2;
}

#if defined(os_windows_test)
#define TLS_SPEC 
#else
#define TLS_SPEC __thread
#endif

TLS_SPEC int lib_tls_read_int = 0;
TLS_SPEC unsigned char lib_tls_write_char = 0x40;
TLS_SPEC signed long lib_tls_read_long = 0;

int lib_check_tls_write(unsigned char expected)
{
   return (lib_tls_write_char == expected);
}

void lib_update_tls_reads()
{
   lib_tls_read_int++;
   lib_tls_read_long--;
}

#if defined(os_windows_test) && defined(__cplusplus)
}
#endif

