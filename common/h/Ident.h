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

/************************************************************************
 * Ident.h: build identification functions (for POSIX systems)
 * $Id: Ident.h,v 1.10 2008/06/11 22:48:14 legendre Exp $
************************************************************************/


#if !defined(_Ident_h_)
#define _Ident_h_

/************************************************************************
 * header files.
************************************************************************/

#include <iostream>
#include <ostream>
//#include "debugOstream.h"
#include "headers.h"
#include "string-regex.h"

/************************************************************************
 * class Ident
 ************************************************************************/

class Ident {
  private:
    // making these private ensures they're not used:
    Ident &operator=(const Ident &);
    Ident(const Ident &);

  public:
     Ident (const char *Vstr, const char *expected_suite); // constructor
    ~Ident ();                                             // destructor

    bool OK () const { return ok_; }

    // uncapitalized versions are plain C strings
    const char* suite () const { return suite_; }
    const char* release () const { return release_; }
    const char* buildnum () const { return buildnum_; }
    const char* component () const { return component_; }
    const char* revision () const { return revision_; }
    const char* date () const { return date_; }
    const char* time () const { return time_; }
    const char* builder () const { return builder_; }

    // capitalized versions are C++ "String" strings
    std::string Suite () const { return std::string (suite_); }
    std::string Release () const { return std::string (release_); }
    std::string BuildNum () const { return std::string (buildnum_); }
    std::string Component () const { return std::string (component_); }
    std::string Revision () const { return std::string (revision_); }
    std::string Date () const { return std::string (date_); }
    std::string Time () const { return std::string (time_); }
    std::string Builder () const { return std::string (builder_); }

    friend std::ostream& operator<< (std::ostream &os, const Ident &Id);  // output

  private:
    bool ok_;                   
    char suite_[16];            // e.g. "Paradyn"
    char release_[16];          // e.g. "v2.1beta"
    char buildnum_[5];          // e.g. "-000"
    char component_[32];        // e.g. "paradynd"
    char revision_[5];          // e.g. "#314"
    char date_[11];             // e.g. "1999/12/31"
    char time_[6];              // e.g. "23:58"
    char builder_[32];          // e.g. "paradyn@cs.wisc.edu"
};

// NB: IdentFields/Formats below depend on Ident private members above
static const int  IdentFields=7;
#ifdef BROKEN_SSCANF
static const char IdentFormat[]="$%s %20s %32s %5s %11s %6s %32s $";
#else
static const char IdentFormat[]="$%16s %20s %32s %5s %11s %6s %32s $";
#endif
static const char IdentOutFmt[]="$%s: %-5s%4s %-16s %-5s %s %s %s $";

#endif /* !defined(_Ident_h_) */
