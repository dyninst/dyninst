/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
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
//----------------------------------------------------------------------------
// $Id: shmSegment-nt.C,v 1.6 2004/03/23 01:12:36 eli Exp $
//----------------------------------------------------------------------------
//
// Definition of the ShmSegment class.
// A ShmSegment object represents a shared memory segment, providing an
// OS-independent interface to management of shared memory segments to 
// paradynd.
//
//----------------------------------------------------------------------------
#include "common/h/headers.h"
#include "common/h/std_namesp.h"
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



