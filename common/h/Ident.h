/*
 * Copyright (c) 1996-1999 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

/************************************************************************
 * Ident.h: build identification functions (for POSIX systems)
 * $Id: Ident.h,v 1.6 2003/07/18 15:43:22 schendel Exp $
************************************************************************/


#if !defined(_Ident_h_)
#define _Ident_h_

/************************************************************************
 * header files.
************************************************************************/

#include <iostream>
#include "common/h/String.h"

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
    pdstring Suite () const { return pdstring (suite_); }
    pdstring Release () const { return pdstring (release_); }
    pdstring BuildNum () const { return pdstring (buildnum_); }
    pdstring Component () const { return pdstring (component_); }
    pdstring Revision () const { return pdstring (revision_); }
    pdstring Date () const { return pdstring (date_); }
    pdstring Time () const { return pdstring (time_); }
    pdstring Builder () const { return pdstring (builder_); }

    friend ostream& operator<< (ostream &os, const Ident &Id);  // output

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
