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
 *
 * $Id: RTshared-win.c,v 1.2 2006/04/14 02:08:09 legendre Exp $
 * RTshared.c: shared memory implementation
 *
 ************************************************************************/

#include <stdio.h>
#include <windows.h>
#include <mmsystem.h>
#include <errno.h>
#include <limits.h>

/* Unfortunately, we still have platform problems... here it's unix vs NT */

void *RTsharedAttach(unsigned key, unsigned size, void *addr) {
    HANDLE hMap;
    TCHAR strName[128];
    void *result;
    
    /* Attach to pre-created shared memory segment */
    sprintf(strName, "ParadynD_%d_%d", key, size);
    hMap = OpenFileMapping(FILE_MAP_ALL_ACCESS,
                           TRUE,
                           strName);
    if( hMap == NULL )
    {
        fprintf( stderr, "OpenFileMapping failed: %d\n", GetLastError() );
    }

    result = MapViewOfFile( hMap,
			    FILE_MAP_ALL_ACCESS,
			    0, 0,
			    0 );
    if( result == NULL )
    {
        fprintf( stderr, "MapViewOfFile failed: %d\n", GetLastError() );
    }
    return result;
}


void *RTsharedDetach(unsigned key, unsigned size, void *addr) {
    UnmapViewOfFile( addr );
    /*CloseHandle( hMapping );*/
    return NULL;
}

