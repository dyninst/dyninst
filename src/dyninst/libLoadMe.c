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


extern int globalVariable5_1;

/*
   cribbed from RTcommon.c:

   _init table of methods:
   GCC: link with gcc -shared, and use __attribute__((constructor));
   AIX: ld with -binitfini:loadMe_init
   Solaris: ld with -z initarray=loadMe_init
   Linux: ld with -init loadMe_init
          gcc with -Wl,-init -Wl,...
          
*/

/* Convince GCC to run _init when the library is loaded */
#if defined(_GNUC) || defined(__GNUC__)
void loadMe_init(void) __attribute__ ((constructor));
#endif

/* UNIX-style initialization through _init */
int initCalledOnce = 0;
void loadMe_init() {

	globalVariable5_1 = 99;

}

