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
 * Ident.C: build identification functions (for POSIX systems)
 * $Id: Ident.C,v 1.6 2008/06/11 22:48:17 legendre Exp $
************************************************************************/

#include "common/src/Ident.h"
#include <stdio.h>
#include <string>
#include <iostream>
#include <ostream>

using namespace std;

Ident::Ident (const char *Vstr, const char *expected_suite)
{
    const int es_len = strlen(expected_suite);  // expected suite name length
    char fullrelease[32];            // temporary holder for release_+buildnum_

#ifdef BROKEN_SSCANF
    char *p=strchr(Vstr,' ');
    if (!p || ((p-Vstr) > 16)) return;
#endif
    int n = sscanf (Vstr, IdentFormat, 
        suite_, fullrelease, component_, revision_, date_, time_, builder_);

    ok_ = (n==IdentFields);

    if (n <= 0) {
        // parsing failed completely!
        cerr << "Warning! Failed to parse identification string:\n<"
             << Vstr << ">" << endl;
    } else if (n < IdentFields) {
        // report how far parsing succeeded
        cerr << "Warning! Parsed only " << n << " of " << IdentFields
             << " fields from $" << suite_ << " identification string: <"
             << Vstr << ">:" << endl;
        cerr << "Release=("     << fullrelease << ") "
             << "Component=("   << component_ << ") "
             << "Revision=("    << revision_ << ") "  
             << "Date=("        << date_ << ") "
             << "Time=("        << time_ << ") "
             << "Builder=("     << builder_ << ") "
             << endl;
    } else { 
        // check Suite is that expected
        if (strncmp(expected_suite, suite_, es_len) == 0) {
            //cerr << "Successfully parsed $Suite identification string" << endl;
            suite_[es_len]='\0';         // remove unnecessary trailing ":"
        } else {
            if (char *p=strrchr(suite_, ':')) { *p='\0'; }
            cerr << "Warning! Identifier contained unexpected suite name: "
                 << suite_ << " <=> " << expected_suite << endl;
        }
        // separate buildnum from release identifier
        if (char *p=strrchr(fullrelease, '-')) {
            strncpy (buildnum_, p, sizeof(buildnum_));
            *p='\0';
            strncpy (release_, fullrelease, sizeof(release_));
        } else {
            //cerr << "No buildnum located in release id " << fullrelease << endl;
            strncpy (release_, fullrelease, sizeof(release_));
        }
    }
}

Ident::~Ident() { }

ostream& operator<< (ostream &os, const Ident &Id)
{
#if defined(notdef)
    // verbose tagged version intended for debugging
    pdstring buf = "{"
       + " suite="     + Id.suite()
       + " release="   + Id.release()
       + " buildnum="  + Id.buildnum()
       + " component=" + Id.component()
       + " revision="  + Id.revision()
       + " date="      + Id.date()
       + " time="      + Id.time()
       + " builder="   + Id.builder()
       + " }";
#else
    // concise inline version intended for normal use
    char buf[128];
    sprintf(buf, IdentOutFmt, Id.suite_,
        Id.release_, Id.buildnum_, Id.component_, Id.revision_,
        Id.date_, Id.time_, Id.builder_);
#endif

    return os << buf;
}

