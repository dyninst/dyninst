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
#if defined(_MSC_VER)
#define FILE__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#elif defined(__GNUC__)
#define FILE__ ((strrchr(__FILE__, '/') ? : __FILE__ - 1) + 1)
#else
#define FILE__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif
#endif

#if defined(_POWER) && !defined(__GNUC__)
#define XLC
#endif

#if defined (_MSC_VER)
  //**************** Windows ********************
  #define DECLTHROW(x)
#elif defined(__GNUC__)
  #include <functional>
  #define DECLTHROW(x) throw(x)
#elif defined(XLC)
  #include <functional>
  #define DECLTHROW(x)
#else
   #error Unknown compiler
#endif


#include <unordered_set>
#include <unordered_map>
#define dyn_hash_set std::unordered_set
#define dyn_hash_map std::unordered_map

// NB: generic enum hashes aren't standard until C++14
#define _DYN_IMPL_ENUM_HASH(ENUM)       \
  namespace std {                       \
    template<> struct hash<ENUM> {      \
      inline size_t                     \
      operator()(ENUM e) const {        \
        return hash<size_t>()(e);       \
      }                                 \
    };                                  \
  }


namespace Dyninst
{
   typedef unsigned long Address;   
   typedef unsigned long Offset;

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

   int ThrIDToTid(Dyninst::THR_ID id);
}

namespace Dyninst
{
   typedef enum {
      OSNone,
      Linux,
      FreeBSD,
      Windows,
      VxWorks,
      BlueGeneL,
      BlueGeneP,
      BlueGeneQ
   } OSType;
}

#include "dyn_regs.h"
#endif
