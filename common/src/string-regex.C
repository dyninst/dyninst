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


// $Id: String.C,v 1.39 2008/07/01 19:26:47 legendre Exp $

#include "common/src/headers.h"
#include <assert.h>
#if !defined (os_windows)
#include <sys/types.h>
#include <regex.h>
#endif

#include <boost/functional/hash.hpp>

/* This doesn't actually belong here. */
void dedemangle( char * demangled, char * result ) 
{
   /* Lifted from Jeffrey Odom.  Code reformatted so
      I could figure out how to eliminate compiler warnings.
      Adjusted to handle spaces inside templates intelligently.
      We cut off everything after the first l-paren, so we don't
      need to worry about the space after the parameters but
      before the 'const'. */
   char * resultBegins = NULL;
   char * resultEnds = NULL;
   
   if (	demangled[0] == '(' &&
	strstr( demangled, "::" ) != NULL) 
   {
     int depth = 0, i = 0;
     for (;;)
     { 
       if (demangled[i] == '\0')
	 break;
       if (demangled[i] == '(')
	 depth++;
       if (demangled[i] == ')') {
	 depth--;
	 if (!depth)
	   break;
       }
       i++;
     }
     if (demangled[i] != '\0') {
       resultBegins = demangled+i+1;
       if (resultBegins[0] == ':' && resultBegins[1] == ':')
	 resultBegins += 2;
       resultEnds = strrchr(resultBegins, ' ');
       if (resultEnds)
	 *resultEnds = '\0';
     }
     demangled = resultBegins;
   }
   if ( strrchr( demangled, '(' ) != NULL ) 
   {
      /* Strip off return types, if any.  Be careful not to
         pull off [template?/]class/namespace information.

         The only space that matters is the one that's _not_
         inside a template, so skip the templates and cut at the
         first space.  We can ignore 'operator[<[<]|>[>]]' because
         we'll stop before we reach it.

         Caveat: conversion operators (e.g., "operator bool") have
         spaces in the function name.  Right now we deal with this
         specifically (is the function "operator *"?).  Could be
         altered to after the last template but before the last
         left parenthesis.  (Instead of next, for "operator ()".)
      */

      resultBegins = demangled;
      int stack = 0; bool inTemplate = false;
      unsigned int offset;
      int lastColon = 0;
      for ( offset = 0; offset < strlen( resultBegins ); offset++ ) 
      {
         if ( resultBegins[offset] == '<' ) 
         {
            stack++;
            inTemplate = true;
         }
         else if ( resultBegins[offset] == '>' ) 
         {
            stack--;
            if ( stack == 0 ) 
            { 
               inTemplate = false;
            }
         }
         else if ( !inTemplate && resultBegins[offset] == '(' ) 
         {
            /* We've stumbled on something without a return value. */

            offset = 0;
            resultBegins = demangled;
            break;
         }
         else if ( !inTemplate && resultBegins[offset] == ' ' ) 
         {
            /* FIXME: verify that the space isn't in the function name,
               e.g., 'operator bool'.  If the first space we meet _is_
               a function name, it doesn't have a(n explicit) return type. */
            if ( strstr( &(resultBegins[ lastColon + 1 ]), "operator " ) == resultBegins + lastColon + 1 ) 
            {
               resultBegins = demangled;
               offset = 0;
            }
            else 
            {
               resultBegins = &(resultBegins[offset + 1]);
               offset++;
            }

            break;
         }
         else if ( !inTemplate && resultBegins[offset] == ':' ) 
         {
            lastColon = offset;
         }
      } /* end template elimination loop */

      /* Scan past the function name; the first left parenthesis
         not in in a template declaration starts the function arguments. */
      stack = 0; inTemplate = false;
      for ( ; offset < strlen( resultBegins ); offset++ ) 
      {
         if ( resultBegins[offset] == '<' ) 
         {
            stack++;
            inTemplate = true;
         }
         if ( resultBegins[offset] == '>' ) 
         {
            stack--;
            if ( stack == 0 ) { inTemplate = false; }
         }
         if ( !inTemplate && resultBegins[offset] == '(' ) 
         {
            resultEnds = const_cast<char *>(&(resultBegins[offset]));
            * resultEnds = '\0';
            break;
         } 
      } /* end template elimination loop */
   } /* end if a function prototype */
   else 
   {
      /* Assume demangle OK. */
      resultBegins = demangled;
   }

   strcpy( result, resultBegins );
} /* end dedemangle */


// Use POSIX regular expression pattern matching to check if string s matches
// the pattern in this string
bool regexEquiv(const char *str_,  const char *s, bool checkCase ) 
{
// Would this work under NT?  I don't know.
#if !defined(os_windows)
	regex_t r;
	int err;
	bool match = false;
	int cflags = REG_NOSUB;
	if ( !checkCase )
		cflags |= REG_ICASE;

	// Regular expressions must be compiled first, see 'man regexec'
	err = regcomp( &r, str_, cflags );

	if ( err == 0 ) 
   {
		// Now we can check for a match
		err = regexec( &r, s, 0, NULL, 0 );
		if( err == 0 )
			match = true;
	}

	// Deal with errors
	if ( err != 0 && err != REG_NOMATCH ) 
   {
		char errbuf[80] = "";
		regerror( err, &r, errbuf, 80 );
		//cerr << "string_ll::regexEquiv -- " << errbuf << endl;
	}

	// Free the pattern buffer
	regfree( &r );
	return match;
#else
	return false;
#endif
}

bool prefixed_by(std::string &haystack, std::string &prefix)
{
   if (!haystack.c_str()) 
      return false;

   const char *res = strstr(haystack.c_str(), prefix.c_str());

   if (res == haystack.c_str())
      return true;

   return false;
}

bool prefixed_by(std::string &haystack, const char *prefix)
{
   std::string pref_str(prefix);
   return prefixed_by(haystack, pref_str);
}


bool suffixed_by(std::string &haystack, std::string &suffix)
{
   if (!haystack.c_str()) 
      return false;

   int lastchar = haystack.length() - 1;

   int j = 0;
   for (int i = suffix.length() - 1; i >= 0; i--)
   {
      if (haystack[lastchar - i] != suffix[j])
         return false;
      j++;
   }

   return true;
}

bool suffixed_by(std::string &haystack, const char *suffix)
{
   std::string suff_str(suffix);
   return suffixed_by(haystack, suff_str);
}

