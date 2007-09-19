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


// pathName.h

#ifndef _PATH_NAME_H_
#define _PATH_NAME_H_

#include "common/h/String.h"
#include <string>
using namespace std;

class pdstring;

pdstring expand_tilde_pathname(const pdstring &dir);
   // e.g. convert "~tamches/hello" to "/u/t/a/tamches/hello",
   // or convert "~/hello" to same.
   // In the spirit of Tcl_TildeSubst

pdstring concat_pathname_components(const pdstring &part1, const pdstring &part2);
   // concatenate path1 and part2, adding a "/" between them if neither
   // part1 ends in a "/" or part2 begins in one.

bool extractNextPathElem(const char * &ptr, pdstring &result);
   // assumes that "ptr" points to the value of the PATH environment
   // variable.  Extracts the next element (writing to result, updating
   // ptr, returning true) if available else returns false;

bool exists_executable(const pdstring &fullpathname);

bool executableFromArgv0AndPathAndCwd(pdstring &result,
				      const pdstring &i_argv0,
				      const pdstring &path,
				      const pdstring &cwd);
pdstring extract_pathname_tail(const pdstring &path);
std::string extract_pathname_tail(const std::string &path);


#endif
