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
// $Id: shmSegment.h,v 1.1 2006/11/22 21:45:05 bernat Exp $
//----------------------------------------------------------------------------
//
// Declaration of ShmSegment class.
// A ShmSegment object represents a shared memory segment, providing an
// OS-independent interface to management of shared memory segments to 
// paradynd.
//
//----------------------------------------------------------------------------
#ifndef SHMSEGMENT_H
#define SHMSEGMENT_H

#include "common/src/Types.h"
#include "common/src/Vector.h"

class BPatch_process;

#if defined(i386_unknown_nt4_0)
typedef HANDLE  shmid_t;
#else // defined(i386_unknown_nt4_0)
typedef int     shmid_t;   
#endif // defined(i386_unknown_nt4_0)


class ShmSegment
{
private:
    shmid_t seg_id;     // "id" for the segment
    key_t   seg_key;    // key that "names" segment
    unsigned int seg_size;  // size of segment
    // Attached at values
    Address baseAddrInDaemon;
    Address baseAddrInApplic;
    // 
    unsigned freespace;
    // Offset from start of segment
    Address highWaterMark;

    bool attached;
    
    bool leaveSegmentAroundOnExit;  // for example, if we attach to process

    // Constructor: does nothing but initialize
    ShmSegment(shmid_t id, key_t key, unsigned int size, Address addrInDaemon)
    : seg_id( id ), seg_key( key ), seg_size( size ),
    baseAddrInDaemon(addrInDaemon), baseAddrInApplic(0),
    freespace(size), highWaterMark(0), attached(false),
    leaveSegmentAroundOnExit(false)
    {
        // We fill the first unsigned with a cookie
        *((unsigned *)addrInDaemon) = ShmSegment::cookie;
        freespace -= sizeof(unsigned);
        highWaterMark += sizeof(unsigned);
    }

public:
    static const unsigned cookie;

    static  ShmSegment* Create( key_t& key, unsigned int size, bool freeWhenDeleted );
    static  ShmSegment* Open( key_t key, unsigned int size, void *addr);

    ShmSegment* Copy(BPatch_process *child_thr, bool sameAddress);

    bool attach(BPatch_process *appthread, Address baseAddr = 0);
    bool detach(BPatch_process *appthread);
    
    ~ShmSegment( void );

    Address malloc(unsigned size);
    void free(Address addr);
    
    shmid_t GetId( void ) const             { return seg_id; }
    key_t   GetKey( void ) const            { return seg_key; }
    unsigned int    GetSize( void ) const   { return seg_size; }

    // Is in the segment (from the perspective of the daemon)
    bool addrInSegmentDaemon(Address addr) const {
        return ((addr - baseAddrInDaemon) <= seg_size);
    }
    // Is in the segment (from the perspective of the application)
    bool addrInSegmentApplic(Address addr) const {
        return ((addr - baseAddrInApplic) <= seg_size);
    }

    Address applicToDaemon(Address addr) const {
        return addr - baseAddrInApplic + baseAddrInDaemon;
    }
    Address daemonToApplic(Address addr) const {
        return addr - baseAddrInDaemon + baseAddrInApplic;
    }

    Address offsetInDaemon(Address addr) const {
        return addr - baseAddrInDaemon;
    }
    Address offsetInApplic(Address addr) const {
        return addr - baseAddrInApplic;
    }
    Address   getAddrInDaemon( Address offset ) const  {
        return offset + baseAddrInDaemon;
    }
    Address   getAddrInApplic( Address offset ) const  {
        return offset + baseAddrInApplic;
    }
    
    
    void markAsLeaveSegmentAroundOnExit() { leaveSegmentAroundOnExit = true; }

    bool attachToApplication();
};

#endif // SHMSEGMENT_H
