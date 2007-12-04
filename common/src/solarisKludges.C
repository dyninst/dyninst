/*
 * Copyright (c) 1996-2007 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */


// $Id: solarisKludges.C,v 1.8 2007/12/04 18:05:22 legendre Exp $

#include "common/h/headers.h"
#include "common/h/parseauxv.h"

#include <sys/auxv.h>

void * P_memcpy (void *A1, const void *A2, size_t SIZE) {
  return (memcpy(A1, A2, SIZE));
}

unsigned long long PDYN_div1000(unsigned long long in) {
   /* Divides by 1000 without an integer division instruction or library call, both of
    * which are slow.
    * We do only shifts, adds, and subtracts.
    *
    * We divide by 1000 in this way:
    * multiply by 1/1000, or multiply by (1/1000)*2^30 and then right-shift by 30.
    * So what is 1/1000 * 2^30?
    * It is 1,073,742.   (actually this is rounded)
    * So we can multiply by 1,073,742 and then right-shift by 30 (neat, eh?)
    *
    * Now for multiplying by 1,073,742...
    * 1,073,742 = (1,048,576 + 16384 + 8192 + 512 + 64 + 8 + 4 + 2)
    * or, slightly optimized:
    * = (1,048,576 + 16384 + 8192 + 512 + 64 + 16 - 2)
    * for a total of 8 shifts and 6 add/subs, or 14 operations.
    *
    */

   unsigned long long temp = in << 20; // multiply by 1,048,576
      // beware of overflow; left shift by 20 is quite a lot.
      // If you know that the input fits in 32 bits (4 billion) then
      // no problem.  But if it's much bigger then start worrying...

   temp += in << 14; // 16384
   temp += in << 13; // 8192
   temp += in << 9;  // 512
   temp += in << 6;  // 64
   temp += in << 4;  // 16
   temp -= in >> 2;  // 2

   return (temp >> 30); // divide by 2^30
}

unsigned long long PDYN_divMillion(unsigned long long in) {
   /* Divides by 1,000,000 without an integer division instruction or library call,
    * both of which are slow.
    * We do only shifts, adds, and subtracts.
    *
    * We divide by 1,000,000 in this way:
    * multiply by 1/1,000,000, or multiply by (1/1,000,000)*2^30 and then right-shift
    * by 30.  So what is 1/1,000,000 * 2^30?
    * It is 1,074.   (actually this is rounded)
    * So we can multiply by 1,074 and then right-shift by 30 (neat, eh?)
    *
    * Now for multiplying by 1,074
    * 1,074 = (1024 + 32 + 16 + 2)
    * for a total of 4 shifts and 4 add/subs, or 8 operations.
    *
    * Note: compare with div1000 -- it's cheaper to divide by a million than
    *       by a thousand (!)
    *
    */

   unsigned long long temp = in << 10; // multiply by 1024
      // beware of overflow...if the input arg uses more than 52 bits
      // than start worrying about whether (in << 10) plus the smaller additions
      // we're gonna do next will fit in 64...

   temp += in << 5; // 32
   temp += in << 4; // 16
   temp += in << 1; // 2

   return (temp >> 30); // divide by 2^30
}

unsigned long long PDYN_mulMillion(unsigned long long in) {
   unsigned long long result = in;

   /* multiply by 125 by multiplying by 128 and subtracting 3x */
   result = (result << 7) - result - result - result;

   /* multiply by 125 again, for a total of 15625x */
   result = (result << 7) - result - result - result;

   /* multiply by 64, for a total of 1,000,000x */
   result <<= 6;

   /* cost was: 3 shifts and 6 subtracts
    * cost of calling mul1000(mul1000()) would be: 6 shifts and 4 subtracts
    *
    * Another algorithm is to multiply by 2^6 and then 5^6.
    * The former is super-cheap (one shift); the latter is more expensive.
    * 5^6 = 15625 = 16384 - 512 - 256 + 8 + 1
    * so multiplying by 5^6 means 4 shift operations and 4 add/sub ops
    * so multiplying by 1000000 means 5 shift operations and 4 add/sub ops.
    * That may or may not be cheaper than what we're doing (3 shifts; 6 subtracts);
    * I'm not sure.  --ari
    */

   return result;
}

bool AuxvParser::readAuxvInfo()
{
  auxv_t auxv_elm;

  char buffer[32];
  snprintf(buffer, 32, "/proc/%d/auxv", pid);
  int auxv_fd = P_open(buffer, O_RDONLY, pid);

  while(read(auxv_fd, &auxv_elm, sizeof(auxv_elm)) == sizeof(auxv_elm)) {
    if (auxv_elm.a_type == AT_BASE) {
      interpreter_base = (Address)auxv_elm.a_un.a_ptr;
      P_close(auxv_fd);
      return true;
    }
  }

  P_close(auxv_fd);  
  return false;
}
