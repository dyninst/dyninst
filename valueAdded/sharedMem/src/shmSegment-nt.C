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
// $Id: shmSegment-nt.C,v 1.1 2006/11/22 21:45:01 bernat Exp $
//----------------------------------------------------------------------------
//
// Definition of the ShmSegment class.
// A ShmSegment object represents a shared memory segment, providing an
// OS-independent interface to management of shared memory segments to 
// paradynd.
//
//----------------------------------------------------------------------------
#include "common/src/headers.h"
#include "common/src/std_namesp.h"
#include "shmSegment.h"


//----------------------------------------------------------------------------
// Prototypes of local functions
//----------------------------------------------------------------------------
static  BuildSegmentName( key_t key, unsigned int size, char* buffer );




//
// ShmSegment::~ShmSegment
//
// Release shared memory segment.
// 
ShmSegment::~ShmSegment( void )
{
    assert( seg_addr != NULL );
    assert( seg_id != NULL );

    // unmap the segment from our address space
    if( !UnmapViewOfFile( seg_addr ) )
    {
#if READY
        indicate error to user?  how?
#endif // READY
    }

    if(! leaveSegmentAroundOnExit) {  
       // release the segment
       if( !CloseHandle( seg_id ) )
       {
#if READY
       // indicate error to user? how?
#endif // READY
       }
    }
}



//
// ShmSegment::Create
//
// Access a shared memory segment whose name is based on the value
// of the given key, of at least the indicated size, and mapped into
// our address space at the indicated address (if specified).
// 
ShmSegment*
ShmSegment::Create( key_t& key, unsigned int size, void* addr )
{
    ShmSegment* seg = NULL;
    key_t curr_key = key;
    HANDLE hMapping = NULL;


    while( true )
    {
        // attempt to create a shared memory segment
        // with a name based on "key"
        char segName[32];

        BuildSegmentName( curr_key, size, segName );
        hMapping = CreateFileMapping( INVALID_HANDLE_VALUE,  // shared memory
            NULL,
            PAGE_READWRITE,
            0, size,
            segName );
        if( hMapping == NULL )
        {
            // unable to create segment
#if READY
            indicate error to user?
#else
        cerr << "Failed to create file mapping object : " << GetLastError() << endl;
#endif // READY
        }
        else if( GetLastError() == ERROR_ALREADY_EXISTS )
        {
            // we were trying to create a new segment,
            // but we found an existing one
            CloseHandle( hMapping );
            hMapping = NULL;
        }
        else
        {
            // we created the new mapping successfully
	    key = curr_key;
            break;
        }

        // try the next key
        curr_key++;
    }

    // we have a mapping object, now map it into our address space
    assert( hMapping != NULL );
    void* mapAddr = MapViewOfFile( hMapping,
                                    FILE_MAP_ALL_ACCESS,
                                    0, 0,
                                    0 );
    if( mapAddr != NULL )
    {
        // we mapped the object successfully
        // build an object to represent the segment
        seg = new ShmSegment( hMapping, curr_key, size, mapAddr );
    }
    else
    {
#if READY
        indicate failure to user
#else
        cerr << "Failed to map shared memory into address space : " << GetLastError() << endl;
#endif // READY
        CloseHandle( hMapping );
    }
    
    return seg;
}



//
// ShmSegment::Open
//
// Open and attach to an existing shared memory segment, whose name
// is based on the given key, of at least the indicated size, and 
// map it into our address space at the indcated address (if given).
//
ShmSegment*
ShmSegment::Open( key_t key, unsigned int size, void* addr )
{
    HANDLE hMapping = NULL;
    ShmSegment* shm = NULL;
    char segName[32];


    BuildSegmentName( key, size, segName );
    hMapping = OpenFileMapping( PAGE_READWRITE, FALSE, segName );
    if( hMapping != NULL )
    {
        // we opened the existing mapping object,
        // now we need to map it into our address space
        void* mapAddr = MapViewOfFile( hMapping,      // map object
                                        FILE_MAP_ALL_ACCESS,    // permissions
                                        0, 0,                   // high and low offsets of map start address
                                        0 );                    // map entire file
        if( mapAddr != NULL )
        {
            // build an object to represent the segment
            shm = new ShmSegment( hMapping, key, size, mapAddr );
        }
        else
        {
#if READY
            indicate failure to user?
#else
            cerr << "Failed to map shared memory into address space : " << GetLastError() << endl;
#endif // READY
            CloseHandle( hMapping );
            hMapping = NULL;
        }
    }
    else
    {
#if READY
        indicate failure to user?
#else
        cerr << "Failed to create file mapping object : " << GetLastError() << endl;
#endif // READY
    }

    return shm;
}



//
// BuildSegmentName
//
// Build a name for a named shared memory semgent based on the 
// given key value and size.
//
static
BuildSegmentName( key_t key, unsigned int size, char* buffer )
{
    sprintf( buffer, "ParadynD_%d_%d", key, size );
}



