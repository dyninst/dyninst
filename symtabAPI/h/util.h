/*
 * Copyright (c) 1996-2007 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

//Hashing function for dictionary_hashes

#if !defined(_symtab_util_h_)
#define _symtab_util_h_

using namespace std;

typedef long unsigned int OFFSET;

#if defined(_MSC_VER)	
#if !defined(DLLEXPORT)
#define DLLEXPORT __declspec(dllexport)
#endif
//On windows it is just hash_map otherwise its in ext/hash_map
#include <hash_map>
using stdext::hash_map;

#else
#if !defined(DLLEXPORT)
#define DLLEXPORT
#endif
#include <regex.h>
#include <ext/hash_map>
using namespace __gnu_cxx;
namespace __gnu_cxx {
   template<> struct hash<std::string> {
      hash<char*> h;  
      size_t operator()(const std::string &s) const {
         return h(s.c_str());
      };
   };              
}; //namespace

#endif

#endif
