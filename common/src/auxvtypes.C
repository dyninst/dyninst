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
#include "auxvtypes.h"

#include <cstdlib>
using namespace std;

const char *auxv_type_to_string(int type) {
  static const char *unknown = "UNKNOWN";
  static const char *names[] = {
    "AT_NULL",		       // 0
    "AT_IGNORE",	
    "AT_EXECFD",	
    "AT_PHDR",		
    "AT_PHENT",	
    "AT_PHNUM",	         // 5
    "AT_PAGESZ",	
    "AT_BASE",		
    "AT_FLAGS",	
    "AT_ENTRY",	
    "AT_NOTELF",	       // 10
    "AT_UID",		
    "AT_EUID",		
    "AT_GID",		
    "AT_EGID",		
    "AT_PLATFORM",       // 15
    "AT_HWCAP",		
    "AT_CLKTCK",	
    "AT_FPUCW",	
    "AT_DCACHEBSIZE",	
    "AT_ICACHEBSIZE",	   // 20
    "AT_UCACHEBSIZE",	
    "AT_IGNOREPPC",	
    "AT_SECURE",	
    unknown,
    unknown,           // 25
    unknown,
    unknown,
    unknown,
    unknown,
    unknown,           // 30,
    unknown,
    "AT,_SYSINFO",
    "AT_SYSINFO_EHDR",
    "AT_L1I_CACHESHAPE",
    "AT_L1D_CACHESHAPE", // 35
    "AT_L2_CACHESHAPE",	
    "AT_L3_CACHESHAPE"
  };
  
  const size_t size = (sizeof(names) / sizeof(const char*));
  return (type >= 0 && ((size_t) type) < size) ? names[type] : unknown;
}
