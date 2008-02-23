#include <string>
#include "dynutil/h/util.h"
namespace Dyninst {
DLLEXPORT unsigned addrHashCommon(const Address &addr)
{
   // inspired by hashs of string class

   register unsigned result = 5381;

   register Address accumulator = addr;
   while (accumulator > 0) {
      // We use 3 bits at a time from the address
      result = (result << 4) + result + (accumulator & 0x07);
      accumulator >>= 3;
   }

   return result;
}

DLLEXPORT unsigned addrHash(const Address & iaddr)
{
   return Dyninst::addrHashCommon(iaddr);
}

DLLEXPORT unsigned ptrHash(const void * iaddr)
{
   return Dyninst::addrHashCommon((Address)iaddr);
}

DLLEXPORT unsigned addrHash4(const Address &iaddr)
{
   // call when you know that the low 2 bits are 0 (meaning they contribute
   // nothing to an even hash distribution)
   return Dyninst::addrHashCommon(iaddr >> 2);
}

DLLEXPORT unsigned addrHash16(const Address &iaddr)
{
   // call when you know that the low 4 bits are 0 (meaning they contribute
   // nothing to an even hash distribution)
   return Dyninst::addrHashCommon(iaddr >> 4);
}

// string hash grabbed from pdstring
unsigned hash(const std::string &s)
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
  sprintf(buf, "%d", in);
  return std::string(buf);
}

std::string utos(unsigned in)
{
  char buf[16];
  sprintf(buf, "%u", in);
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


} // namespace Dyninst
