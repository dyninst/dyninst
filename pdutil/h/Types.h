/*
 * Copyright (c) 1996 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
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

/************************************************************************
 * $Id: Types.h,v 1.8 1999/11/11 00:51:49 wylie Exp $
 * Types.h: commonly used types (used by runtime libs and other modules)
************************************************************************/

#if !defined(_Types_h_)
#define _Types_h_

/* This file is now included by the rtinst library also, so all
   C++ constructs need to be in #ifdef _cplusplus... 7/9/99 -bhs */

/* typedef appropriate definitions for ISO C9X standard integer types
   if these currently aren't provided in <sys/types.h> */
#if defined(i386_unknown_nt4_0)
typedef __int64 int64_t;
typedef __int32 int32_t;
typedef unsigned __int64 uint64_t;
typedef unsigned __int32 uint32_t;
#elif defined(alpha_dec_osf4_0) || \
      (defined(rs6000_ibm_aix4_1) && !defined(_H_INTTYPES))
typedef int int32_t;
typedef long long int64_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;
#else
#include <sys/types.h>
/* Includes
             int32_t   uint32_t    int64_t      uint64_t
Solaris      yes       yes         yes          yes
Linux        yes       no?         yes          no?
Irix         yes       yes         yes          yes
Osf          no        no          no           no
Aix4.1       no        no          no           no
Aix4.3       yes       yes         yes          yes
WindowsNT    no        no          no           no
*/
#endif

typedef int64_t time64;

typedef long unsigned int Address;
/* Note the inherent assumption that the size of a "long" integer matches
   that of an address (void*) on every supported Paradyn/Dyninst system!
   (This can be checked with Address_chk().)
*/

typedef unsigned int Word;

typedef long int RegValue;      /* register content */
/* This needs to be an int since it is sometimes used to pass offsets
   to the code generator (i.e. if-statement) - jkh 5/24/99 */
typedef unsigned int Register;  /* a register number, e.g., [0..31]  */
static const Register Null_Register = (Register)(-1);   /* '255' */

#ifdef __cplusplus

extern void Address_chk ();
extern char *Address_str (Address addr);

// NB: this is probably inappropriate for 64-bit addresses!
inline unsigned hash_address(const Address& addr) {
    return (addr >> 2);
}
#endif /* __cplusplus */

#endif /* !defined(_Types_h_) */



