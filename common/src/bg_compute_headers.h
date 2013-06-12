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
#if !defined(_bgl_compute_headers_h)
#define _bgl_compute_headers_h

#include <string.h>
#include <errno.h>
#include <sys/time.h>

inline const char * P_strrchr (const char *P_STRING, int C) {return (strrchr(P_STRING, C));}
inline char * P_strrchr (char *P_STRING, int C) {return (strrchr(P_STRING, C));}
inline void * P_memcpy (void *A1, const void *A2, size_t SIZE)
    { return memcpy( A1, A2, SIZE ); }

/* The following values are taken from demangle.h in binutils */
#define DMGL_PARAMS      (1 << 0)       /* Include function args */
#define DMGL_ANSI        (1 << 1)       /* Include const, volatile, etc */

#define DMGL_ARM         (1 << 11)      /* Use C++ ARM name mangling */ 

extern "C" char *cplus_demangle(char *, int);
extern void dedemangle( char * demangled, char * dedemangled );
extern char * COMMON_EXPORT P_cplus_demangle( const char * symbol, bool nativeCompiler,
				bool includeTypes = false );
#endif // _bgl_compute_headers_h
