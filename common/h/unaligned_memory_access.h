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

#ifndef unaligned_memory_access_h_
#define unaligned_memory_access_h_


/*
 * Routines to access data at an arbitrary memory location (that is possibly
 * unaligned for the data type) via char*-like pointer with no alignment
 * restrictions.  The set of functions range from safe read and write of memory
 * cast to a type; to functions that cast to a pointer with more restrictive
 * alignment (caller is responsible for addr's alignment correctness).
 */


#ifdef __cplusplus

#include <cstring>
#include <cstdint>

namespace Dyninst {

/*
 * read_memory_as<TYPE>(addr)
 *
 * Return an object of TYPE created by reading sizeof(TYPE) bytes at addr.
 * The address addr does not have to be properly aligned for the type.
 */
template <typename ResultType>
inline ResultType read_memory_as(const void *addr)
{
    ResultType r;
    std::memcpy(static_cast<void *>(&r), addr, sizeof r);
    return r;
}


/*
 * write_memory_as<TYPE>(adrr, &data)
 *
 * Write sizeof(TYPE) bytes to the memory pointed to by addr by copying the
 * bytes of the supplied parameter.  The address addr does not have to be
 * properly aligned for the type.
 */
template <typename DataType>
inline void write_memory_as(void *addr, const DataType &data)
{
    std::memcpy(addr, static_cast<const void*>(&data), sizeof data);
}


/*
 * append_memory_as<TYPE>(adrr, &data)
 *
 * Write sizeof(TYPE) bytes to the memory pointed to by addr by copying the
 * bytes of the supplied parameter, and adjust addr by sizeof(TYPE).  The
 * address addr does not have to be properly aligned for the type.
 */
template <typename PointerType, typename DataType>
inline void append_memory_as(PointerType *&addr, const DataType &data)
{
    write_memory_as(addr, data);
    *reinterpret_cast<std::uint8_t **>(&addr) += sizeof(data);
}


/*
 * append_memory_as_byte(adrr, data)
 *
 * Convenience function to avoid uint8_t cast when calling append_memory_as
 * with a uint8_t type.
 */
template <typename PointerType>
inline void append_memory_as_byte(PointerType *&addr, std::uint8_t data)
{
    append_memory_as(addr, data);
}


/*
 * alignas_cast<TYPE>(addr)
 *
 * Convert the addr pointer to TYPE*, that avoids triggering a warning that the
 * alignment of the new pointer type is larger than the current pointer type.
 *
 * WARNING:  The caller is responsible to ensure that the alignment is correct
 * if the pointer is dereferenced.  Uncomment the assert to audit proper
 * alignment of addr.
 *
 * Use read_memory_as or write_memory_as if possible and definitely if the
 * alignment of addr can not be guaranteed.
 */
template <typename DataType>
inline DataType* alignas_cast(const void *addr)
{
    // assert((reinterpret_cast<uintptr_t>(addr) % alignof(DataType)) == 0);
    return static_cast<DataType*>(addr);
}


template <typename DataType>
inline DataType* alignas_cast(void *addr)
{
    // assert((reinterpret_cast<uintptr_t>(addr) % alignof(DataType)) == 0);
    return static_cast<DataType*>(addr);
}


}

#else

/*
 * C language - functions
 */

/* CAST_WITHOUT_ALIGNMENT_WARNING(toType, addr)
 *
 * C language macro that casts the expression addr to type toType
 * without producing a warning that alignment of the new pointer type is
 * larger than the current pointer type.
 *
 * WARNING:  The caller is responsible to ensure that the alignment is
 * correct or use memcpy if the pointer is dereferenced.
 */
#define CAST_WITHOUT_ALIGNMENT_WARNING(toType, addr) (toType)(void*)(addr)


#endif

#endif
