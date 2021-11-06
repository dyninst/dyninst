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

COMMON_EXPORT unsigned addrHashCommon(const Address &addr)
{
   // inspired by hashs of string class

   unsigned result = 5381;

   Address accumulator = addr;
   while (accumulator > 0) {
      // We use 3 bits at a time from the address
      result = (result << 4) + result + (accumulator & 0x07);
      accumulator >>= 3;
   }

   return result;
}

COMMON_EXPORT unsigned addrHash(const Address & iaddr)
{
   return Dyninst::addrHashCommon(iaddr);
}

COMMON_EXPORT unsigned ptrHash(const void * iaddr)
{
   return Dyninst::addrHashCommon((Address)iaddr);
}

COMMON_EXPORT unsigned ptrHash(void * iaddr)
{
   return Dyninst::addrHashCommon((Address)iaddr);
}

COMMON_EXPORT unsigned addrHash4(const Address &iaddr)
{
   // call when you know that the low 2 bits are 0 (meaning they contribute
   // nothing to an even hash distribution)
   return Dyninst::addrHashCommon(iaddr >> 2);
}

COMMON_EXPORT unsigned addrHash16(const Address &iaddr)
{
   // call when you know that the low 4 bits are 0 (meaning they contribute
   // nothing to an even hash distribution)
   return Dyninst::addrHashCommon(iaddr >> 4);
}

// string hash grabbed from pdstring
unsigned stringhash(const std::string &s)
{
   const char *str = s.c_str();
   if (!str)
      return 1; // 0 is reserved for unhashed key

   unsigned h = 5381;
   while (*str) {
      h = (h << 5) + h + (unsigned) (*str);
      str++;
   }
   return h==0 ? 1 : h; // 0 is reserved for unhashed key
}

std::string itos(int in)
{
  char buf[16];
  snprintf(buf, 16, "%d", in);
  return std::string(buf);
}

std::string utos(unsigned in)
{
  char buf[16];
  snprintf(buf, 16, "%u", in);
  return std::string(buf);
}


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


const char *platform_string()
{
	const char *plat_str = getenv("PLATFORM");
	if (plat_str)
		return plat_str;

#if defined (arch_x86)
#if defined (os_linux)
	return "i386-unknown-linux2.4";
#elif defined (os_windows)
	return "i386-unknown-nt4.0";
#endif
#elif defined (arch_x86_64)
#if defined (os_linux)
	return "x86_64-unknown-linux2.4";
#elif defined (os_windows)
	return "x86_64-unknown-nt4.0";
#endif
#elif defined (arch_power)
#if defined (os_linux)
#if defined (arch_64bit)
	return "ppc64_linux";
#endif
#endif
#endif
	return "bad_platform";
}


//SymElf code is exclusively linked in each component, but we still want to share
//the cache information.  Thus the cache will live in libcommon.
class SymElf;

COMMON_EXPORT map<string, SymElf *> *getSymelfCache() {
   static map<string, SymElf *> elfmap;
   return &elfmap;
}

} // namespace Dyninst
