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

// Contains definitions for functions which NT does not give you
// The argument given is returned here

#include <stdio.h>
#include "common/src/headers.h"

/*
 * stripAtSuffix
 *
 * Strips off of a string any suffix that consists of an @ sign followed by
 * decimal digits.
 *
 * str  The string to strip the suffix from.  The string is altered in place.
 */
static void stripAtSuffix(char *str)
{
    // many symbols have a name like foo@4, we must remove the @4
    // just searching for an @ is not enough,
    // as it may occur on other positions. We search for the last one
    // and check that it is followed only by digits.
    char *p = strrchr(str, '@');
    if (p) {
      char *q = p+1;
      strtoul(p+1, &q, 10);
      if (q > p+1 && *q == '\0') {
        *p = '\0';
      }
    }
}

char *cplus_demangle(const char *c, int, bool includeTypes) {
    char buf[1000];
    if (c[0]=='_') {
       // VC++ 5.0 seems to decorate C symbols differently to C++ symbols
       // and the UnDecorateSymbolName() function provided by imagehlp.lib
       // doesn't manage (or want) to undecorate them, so it has to be done
       // manually, removing a leading underscore from functions & variables
       // and the trailing "$stuff" from variables (actually "$Sstuff")
       unsigned i;
       for (i=1; i<sizeof(buf) && c[i]!='$' && c[i]!='\0'; i++)
           buf[i-1]=c[i];
       buf[i-1]='\0';
       stripAtSuffix(buf);
       if (buf[0] == '\0')
          return 0; // avoid null names which seem to annoy Paradyn
       return P_strdup(buf);
    } else {
       if (includeTypes) {
          if (UnDecorateSymbolName(c, buf, 1000, UNDNAME_COMPLETE| UNDNAME_NO_ACCESS_SPECIFIERS|UNDNAME_NO_MEMBER_TYPE|UNDNAME_NO_MS_KEYWORDS)) {
            //   printf("Undecorate with types: %s = %s\n", c, buf);
            stripAtSuffix(buf);
            return P_strdup(buf);
          }
       }  else if (UnDecorateSymbolName(c, buf, 1000, UNDNAME_NAME_ONLY)) {
         //     else if (UnDecorateSymbolName(c, buf, 1000, UNDNAME_COMPLETE|UNDNAME_32_BIT_DECODE)) {
         //     printf("Undecorate: %s = %s\n", c, buf);
         stripAtSuffix(buf);
         return P_strdup(buf);
       }
    }
    return 0;
}
