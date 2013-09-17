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
//----------------------------------------------------------------------------
// $Id: int64iostream.C,v 1.8 2007/05/30 19:20:21 legendre Exp $
//----------------------------------------------------------------------------
//
// Utility functions adding support for Microsoft's 64-bit integer
// data type (__int64) into the iostream I/O mechanism.
//
//----------------------------------------------------------------------------
#include <stdlib.h>
#include "common/src/int64iostream.h"

ostream&
operator<<( ostream& s, int64_t val )
{
    char buffer[32] = "";

    _i64toa( val, buffer, 10 );
    s << buffer << std::ends;

    return s;
}

ostream&
operator<<( ostream& s, uint64_t val )
{
    char buffer[32] = "";

    _i64toa( val, buffer, 10 );
    s << buffer << std::ends;

    return s;
}

