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
#include "util.h"
#if defined(os_windows)
#include "common/src/ntHeaders.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <map>
#include "common/h/dyntypes.h"

using namespace std;

namespace Dyninst {

// This function will match string s against pattern p.
// Asterisks match 0 or more wild characters, and a question
// mark matches exactly one wild character.  In other words,
// the asterisk is the equivalent of the regex ".*" and the
// question mark is the equivalent of "."

bool pattern_match( const char *p, const char *s, bool checkCase ) 
{
   //const char *p = ptrn;
   //char *s = str;

   while ( true ) {

      // If at the end of the pattern, it matches if also at the end of the string

      if ( *p == '\0' )
         return ( *s == '\0' );

      // Process a '*'

      if ( *p == MULTIPLE_WILDCARD_CHAR ) {
         ++p;

         // If at the end of the pattern, it matches
         if ( *p == '\0' )
            return true;

         // Try to match the remaining pattern for each remaining substring of s
         for (; *s != '\0'; ++s )
            if ( pattern_match( p, s, checkCase ) )
               return true;
         // Failed
         return false;
      }

      // If at the end of the string (and at this point, not of the pattern), it fails
      if( *s == '\0' )
         return false;

      // Check if this character matches
      bool matchChar = false;
      if ( *p == WILDCARD_CHAR || *p == *s )
         matchChar = true;
      else if ( !checkCase ) {
         if ( *p >= 'A' && *p <= 'Z' && *s == ( *p + ( 'a' - 'A' ) ) )
            matchChar = true;
         else if ( *p >= 'a' && *p <= 'z' && *s == ( *p - ( 'a' - 'A' ) ) )
            matchChar = true;
      }

      if ( matchChar ) {
         ++p;
         ++s;
         continue;
      }

      // Did not match
      return false;
   }
}

bool wildcardEquiv(const std::string &us, const std::string &them, bool checkCase ) 
{
   if ( us == them )
      return true;
   else
      return pattern_match( us.c_str(), them.c_str(), checkCase );
}

} // namespace Dyninst
