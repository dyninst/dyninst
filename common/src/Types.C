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
 * $Id: Types.C,v 1.7 2007/05/30 19:20:17 legendre Exp $
 * Types.C: commonly used type-handling functions.
************************************************************************/

#include "common/src/Types.h"
#include <stdio.h>
#include <assert.h>

// verify the size of the defined Address type
void Address_chk ()
{
    assert (sizeof(Address) == sizeof(void*));
}

static const unsigned int _numaddrstrs=8;
                        // maximum number of addresses per outstanding printf!
static char _addrstr[_numaddrstrs][19]; // "0x"+16+'\0'

// Format an address string according to the size of the Address type.
// Note that "%x" outputs incorrect/incomplete addresses, and that "%lx"
// or system-dependent "%p" (generally also requiring a typecast to (void*))
// must be used instead!
char *Address_str (Address addr)
{
    static int i=0;
    i=(i+1)%_numaddrstrs;
    if (sizeof(Address) == sizeof(int))
        snprintf(_addrstr[i],19,"0x%08X",(unsigned int)addr);
    else
        snprintf(_addrstr[i],19,"0x%016lX",(unsigned long)addr);
    return (_addrstr[i]);
}


int ThrIDToTid(Dyninst::THR_ID id)
{
#if defined(os_windows)
	// This is vista-only; for the time being, we'll return the handle as a dword
	//   return GetThreadId(id);
	return (unsigned long) id;
#else
   return (int) id;
#endif
}
