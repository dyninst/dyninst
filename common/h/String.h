/*
 * Copyright (c) 1996 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
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
 * String.h: a simple character string class.
************************************************************************/

#if !defined(_String_h_)
#define _String_h_


/************************************************************************
 * header files.
************************************************************************/

#include <iostream.h>
#include "util/h/debugOstream.h"
#include "util/h/headers.h"


/************************************************************************
 * class string
************************************************************************/

class string {
public:
     string ();
     string (const char *);
     string (const char *, unsigned n); // just copy the first n chars
     string (const string &);
     string (int);      // convert int to its string representation
     string (long);      // convert int to its string representation
     string (unsigned); // convert unsigned to its string representation
     string (unsigned long); // convert unsigned to its string representation
     string (float);    // convert float to its string representation
     string (double);   // convert double to its string representation
    ~string ();

    string& operator= (const char *);
    string& operator= (const string &);
    string& operator+= (const string &);
    string  operator+ (const string &) const;

    bool operator== (const string &) const;
    bool operator== (const char *ptr) const {
       // This routine exists as an optimization; doesn't need to create a temporary
       // instance of "string" for "ptr"; hence, doesn't call string::string(char *)
       // which calls new[].
       return STREQ(ptr, str_);
    }
    bool operator!= (const string &) const;
    bool operator<  (const string &s) const {return STRLT(str_, s.str_);}
    bool operator<= (const string &) const;
    bool operator>  (const string &s) const {return STRGT(str_, s.str_);}
    bool operator>= (const string &) const;

    bool prefix_of (const char *, unsigned) const;
    bool prefix_of (const char *s)          const {return prefix_of(s, STRLEN(s));};
    bool prefix_of (const string &)         const;

    bool prefixed_by (const char *, unsigned) const;
    bool prefixed_by (const char *s)          const {return prefixed_by(s, STRLEN(s));};
    bool prefixed_by (const string &)         const;

    const char*   string_of () const {return str_;}
    unsigned         length () const {return len_;}

    friend ostream& operator<< (ostream &os, const string &s);
    friend debug_ostream& operator<< (debug_ostream &os, const string &s);

    static unsigned       hash (const string &s) {return s.key_;}

private:
    static unsigned      hashs (const char *);

    static unsigned     STRLEN (const char *);
    static char*        STRDUP (const char *);

    static bool          STREQ (const char *, const char *);
    static bool         STREQN (const char *, const char *, unsigned);
    static bool          STRNE (const char *, const char *);
    static bool          STRLT (const char *, const char *);
    static bool          STRLE (const char *, const char *);
    static bool          STRGT (const char *, const char *);
    static bool          STRGE (const char *, const char *);

    char*    str_;
    unsigned len_;
    unsigned key_;
};

#endif /* !defined(_String_h_) */
