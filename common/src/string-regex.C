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
   if (haystack.empty())
      return false;

   if(haystack.find(prefix) == 0) return true;

   return false;
}

bool prefixed_by(std::string &haystack, const char *prefix)
{
   std::string pref_str(prefix);
   return prefixed_by(haystack, pref_str);
}


bool suffixed_by(std::string &haystack, std::string &suffix)
{
   if (haystack.empty())
      return false;
   if(haystack.rfind(suffix) == haystack.length() - suffix.length()) return true;
   return false;
}

bool suffixed_by(std::string &haystack, const char *suffix)
{
   std::string suff_str(suffix);
   return suffixed_by(haystack, suff_str);
}

