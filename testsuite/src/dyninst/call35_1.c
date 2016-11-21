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

/* Dummy Function */
static int call35_2() {
}  

/*
 call35_1 is the template for which the platform specific call35 
 functions used in test35 are based. call35_1_sparc_solaris.s, 
 call35_1_x86_solaris.s,  and call35_1_x86_linux.s were all compiled
 versions of this function, where they each have instructions added to 
 force relocation of the function.
*/
int test1_35_call1() {
  int localVariable35_1 = 1;
  int localVariable35_2 = 1; 
  int total35_1 = 0;
  int total35_2 = 2; 

  call35_2();

  total35_1 = localVariable35_1 * localVariable35_2;

  call35_2();

  call35_2();

  return total35_2;
}

