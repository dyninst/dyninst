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

/************************************************************************
 * $Id: Types.h,v 1.38 2008/08/29 21:45:10 legendre Exp $
 * Types.h: commonly used types (used by runtime libs and other modules)
************************************************************************/

#if !defined(_Types_h_)
#define _Types_h_

#if defined __cplusplus
#  include <cstdint>
#else
#  include <stdint.h>
#endif

   /*
typedef int64_t time64;
*/

#if defined(__cplusplus)
#include "common/h/dyntypes.h"
using namespace Dyninst;
static const Address ADDR_NULL = (Address)(0);
#else
#define ADDR_NULL (0)
typedef unsigned long Address;
#endif
/* Note the inherent assumption that the size of a "long" integer matches
   that of an address (void*) on every supported Paradyn/Dyninst system!
   (This can be checked with Address_chk().)
*/

typedef unsigned int Word;

typedef long long int RegValue;      /* register content 64-bit */
/* This needs to be an int since it is sometimes used to pass offsets
   to the code generator (i.e. if-statement) - jkh 5/24/99 */
typedef unsigned int Register;  /* a register number, e.g., [0..31]  */
static const Register Null_Register = (Register)(-1);   /* '255' */
/* Easily noticeable name... */
static const Register REG_NULL = (Register)(-1);

// Virtual Memory Map -- shared between platforms
#define PREMS_PRIVATE (1 << 4)
#define PREMS_SHARED  (1 << 3)
#define PREMS_READ    (1 << 2)
#define PREMS_WRITE   (1 << 1)
#define PREMS_EXEC    (1 << 0)

#define MAPENTRIES_PATH_SIZE 512
#define MAPENTRIES_PATH_SIZE_STR "512"
typedef struct maps_entries {
   Address start;
   Address end;
   unsigned prems;
   Address offset;
   int dev_major;
   int dev_minor;
   unsigned long inode;
   char path[MAPENTRIES_PATH_SIZE];
} map_entries;

#ifdef __cplusplus

#include "common/h/util.h"

COMMON_EXPORT void Address_chk ();

// NB: this is probably inappropriate for 64-bit addresses!
inline unsigned hash_address(const Address& addr) {
   return (unsigned) ((addr >> 2) & 0xffffffff);
}
#endif /* __cplusplus */

#endif /* !defined(_Types_h_) */


