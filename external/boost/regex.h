/*
 * Copyright (c) 1996-2011 Barton P. Miller
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */
/*
 *
 * Copyright (c) 1998-2000
 * Dr John Maddock
 *
 * Use, modification and distribution are subject to the 
 * Boost Software License, Version 1.0. (See accompanying file 
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */
 
 /*
  *   LOCATION:    see http://www.boost.org/libs/regex for documentation.
  *   FILE         regex.h
  *   VERSION      3.12
  *   DESCRIPTION: Declares POSIX API functions
  */

#ifndef BOOST_RE_REGEX_H
#define BOOST_RE_REGEX_H

#include <boost/cregex.hpp>

/*
*  add using declarations to bring POSIX API functions into
* global scope, only if this is C++ (and not C).
*/
#ifdef __cplusplus

using boost::regoff_t;
using boost::regex_tA;
using boost::regmatch_t;
using boost::REG_BASIC;
using boost::REG_EXTENDED;
using boost::REG_ICASE;
using boost::REG_NOSUB;
using boost::REG_NEWLINE;
using boost::REG_NOSPEC;
using boost::REG_PEND;
using boost::REG_DUMP;
using boost::REG_NOCOLLATE;
using boost::REG_ESCAPE_IN_LISTS;
using boost::REG_NEWLINE_ALT;
using boost::REG_PERL;
using boost::REG_AWK;
using boost::REG_GREP;
using boost::REG_EGREP;
using boost::REG_ASSERT;
using boost::REG_INVARG;
using boost::REG_ATOI;
using boost::REG_ITOA;

using boost::REG_NOTBOL;
using boost::REG_NOTEOL;
using boost::REG_STARTEND;

using boost::reg_comp_flags;
using boost::reg_exec_flags;
using boost::regcompA;
using boost::regerrorA;
using boost::regexecA;
using boost::regfreeA;

#ifndef BOOST_NO_WREGEX
using boost::regcompW;
using boost::regerrorW;
using boost::regexecW;
using boost::regfreeW;
using boost::regex_tW;
#endif

using boost::REG_NOERROR;
using boost::REG_NOMATCH;
using boost::REG_BADPAT;
using boost::REG_ECOLLATE;
using boost::REG_ECTYPE;
using boost::REG_EESCAPE;
using boost::REG_ESUBREG;
using boost::REG_EBRACK;
using boost::REG_EPAREN;
using boost::REG_EBRACE;
using boost::REG_BADBR;
using boost::REG_ERANGE;
using boost::REG_ESPACE;
using boost::REG_BADRPT;
using boost::REG_EEND;
using boost::REG_ESIZE;
using boost::REG_ERPAREN;
using boost::REG_EMPTY;
using boost::REG_E_MEMORY;
using boost::REG_E_UNKNOWN;
using boost::reg_errcode_t;

#endif /* __cplusplus */

#endif /* BOOST_RE_REGEX_H */




