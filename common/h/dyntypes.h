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

#if !defined(DYNTYPES_H)
#define DYNTYPES_H

#if defined(_MSC_VER)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <winsock2.h>
#endif

#ifndef FILE__
#include <string.h>
#if defined(_MSC_VER)
#define FILE__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#else
#define FILE__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif
#endif

#ifdef __cplusplus

#include <functional>
#include <memory>
#include <utility>
#include <unordered_set>
#include <unordered_map>

// NB: std::hash has overloads for [un]scoped enums
template <typename Key,
		  typename Value,
		  typename Hash = std::hash<Key>,
		  typename Comp = std::equal_to<Key>,
		  typename Alloc = std::allocator<std::pair<const Key, Value>>>
using dyn_hash_map = std::unordered_map<Key, Value, Hash, Comp, Alloc>;

template <typename Key,
		  typename Hash = std::hash<Key>,
		  typename Comp = std::equal_to<Key>,
		  typename Alloc = std::allocator<Key>>
using dyn_hash_set = std::unordered_set<Key, Hash, Comp, Alloc>;

// We require C++11 thread_local support
#define dyn_tls thread_local

namespace Dyninst
{
#if defined(_WIN64)
  typedef uintptr_t Address;
  typedef uintptr_t Offset;
#else
  typedef unsigned long Address;
  typedef unsigned long Offset;
#endif

  static constexpr Address ADDR_NULL{0};

#if defined(_MSC_VER)
   typedef int PID;
   typedef HANDLE PROC_HANDLE;
   typedef HANDLE LWP;
   typedef HANDLE THR_ID;
   typedef DWORD psaddr_t; // for breakpoints; match the debug struct

#define NULL_PID     -1
#define NULL_LWP     INVALID_HANDLE_VALUE
#define NULL_THR_ID     INVALID_HANDLE_VALUE
#define DYNINST_SINGLETHREADED INVALID_HANDLE_VALUE
#else
   typedef int PID;
   typedef int PROC_HANDLE;
   typedef int LWP;
   typedef long THR_ID;

#define NULL_PID     -1
#define NULL_LWP     -1
#define NULL_THR_ID     -1
#ifndef INVALID_HANDLE_VALUE
#define INVALID_HANDLE_VALUE -1
#endif
#endif

   inline int ThrIDToTid(Dyninst::THR_ID id) { return id; }
}

namespace Dyninst
{
   typedef enum {
      OSNone,
      Linux,
      FreeBSD,
      Windows
   } OSType;
}

#else
# define ADDR_NULL (0)
typedef unsigned long Address;
#endif

#endif
