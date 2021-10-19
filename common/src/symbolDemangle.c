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


#include <string.h>
#include <stdlib.h>
#include <demangle.h> // from libiberty
#include "symbolDemangle.h"


// Returns a malloc'd string that is the demangled symbol name.  THe caller is
// responsible for the freeing this memory.  Returns NULL on malloc failure.
// The symbol name symName is demangled using the cplus_demangle function after
// first removing any versioning suffixes (first '@')
// to create the mangled name.  If cplus_demangle fails, the mangled name
// is returned unmodified.  If cplus_demangle succeeds and includeParams is
// false, then any clone suffixes (the first '.' to the end of the mangled
// name) are appended to the value from cplus_demangle.
//
// Other than the removal of versioning suffixes, and appending any
// clone suffixes if includeParams is false to the result, the result should be
// equivalent to using c++filt.  Below are the c++filt and cplus_demangle
// options
//
// includeParams	c++ filt opts	cplus_demangle opts
// -------------	-------------	-------------------
//   false		  -i -p		DMGL_AUTO | DMGL_ANSI
//   true		  -i		DMGL_AUTO | DMGL_ANSI | DMGL_PARAMS
//
char *symbol_demangle(const char *symName, int includeParams)
{
    int cloneOffset = -1;		// offset to clone suffix
    int versionOffset = -1;	// offset to version suffix
    int lastOffset;			// offset to version suffix
					//   if present, else null of symName

    // find both clone, and then version suffixes if any
    for (lastOffset = 0; symName[lastOffset]; ++lastOffset)  {
	char c = symName[lastOffset];
	if (c == '.' && cloneOffset == -1)  {
	    // clone suffix start with first '.'
	    cloneOffset = lastOffset;
	}  else if (c == '@')  {
	    // version ('@') suffix found
	    versionOffset = lastOffset;
	    // stop searching
	    break;
	}
    }

    const char *mangledName = symName;	// symName without version suffix
    char *allocatedMangledName = 0;
    if (versionOffset != -1)  {
	// make a copy of the symName without version suffix
	allocatedMangledName = malloc(versionOffset + 1);
	if (allocatedMangledName == 0)  {
	    return NULL;
	}
	memcpy(allocatedMangledName, symName, versionOffset);
	allocatedMangledName[versionOffset] = '\0';
	mangledName = allocatedMangledName;
    }

    int opts = DMGL_AUTO | DMGL_ANSI;	// c++filt -i -p
    if (includeParams)  {
        opts |= DMGL_PARAMS;		// c++file -i
    }

    char *s = cplus_demangle(mangledName, opts);

    if (!s)  {
        // on failure, return copy of mangledName
	if (allocatedMangledName)  {
	    // use existing copy
	    s = allocatedMangledName;
	}  else  {
	    // make copy
	    // would like to use:  s = strdup(mangledName)
	    // but strdup not available on all std C libraries
	    s = malloc(lastOffset + 1);
	    if (s)  {
		memcpy(s, mangledName, lastOffset);
		s[lastOffset] = '\0';
	    }
	}
    }  else if (!includeParams) {
        if (cloneOffset != -1)  {
	    // append clone suffix if present to the pretty name
	    char *sOriginal = s;	// save incase realloc fails
	    size_t sLen = strlen(s);
	    //             length of s + length of clone suffix + 1 for null
            s = realloc(s, sLen + (lastOffset - cloneOffset) + 1);
	    if (s)  {
		memcpy(s + sLen, mangledName + cloneOffset, lastOffset - cloneOffset + 1);
	    }  else  {
		// realloc failed free sOriginal and return NULL
		free(sOriginal);
	    }
        }
	if (allocatedMangledName)  {
	    // allocated memory that is not returned, free
	    free(allocatedMangledName);
	}
    }

    return s; 
}
