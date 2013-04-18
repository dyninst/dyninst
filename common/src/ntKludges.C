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

char *optarg;
int P_getopt(int argc, char *argv[], const char *optstring)
{
  int i;
  char arg_found = (char) 0;
  static int lastArgSeen = 0;
  if (lastArgSeen >= argc) return EOF;
  for (i = lastArgSeen; i < argc; i++) {
    unsigned int opt_index = 0;
    char *nextArg = argv[i];
    if (nextArg[0] != '-') continue;
    for (opt_index = 0; opt_index < P_strlen(optstring); opt_index++)
      if (optstring[opt_index] == nextArg[1]) {
        arg_found = optstring[opt_index];
        // We may have an argument value
        if ((opt_index < strlen(optstring)-1) // overflow bad
            && (optstring[opt_index+1] == ':')) {
          // Argument. Either the last part of the string, or the 
          // next item in the argv set
          if (strlen(nextArg) > 2) { // Last part of the argument
            optarg = (char *)(nextArg + 2);
          }
          else
            if (i < argc - 1) {              
              optarg = argv[i + 1];
            }
            else {
              optarg = (char *)0;
            }
        } // argument found
        break;
      }
    if (opt_index == strlen(optstring)) // Nothing found
      break;
    if (arg_found)
      break;
  }
  lastArgSeen = i+1;
  if (!arg_found)
      return EOF;
  return arg_found;
}
