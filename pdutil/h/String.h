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

class string_ll {
public:
     string_ll ();
     string_ll (const char *);
     string_ll (const char *, unsigned n); // just copy the first n chars
     string_ll (const string_ll &);
     string_ll (int);      // convert int to its string_ll representation
     string_ll (long);      // convert int to its string_ll representation
     string_ll (unsigned); // convert unsigned to its string_ll representation
     string_ll (unsigned long); // convert unsigned to its string_ll representation
     string_ll (float);    // convert float to its string_ll representation
     string_ll (double);   // convert double to its string_ll representation
    ~string_ll ();

    string_ll& operator= (const char *);
    string_ll& operator= (const string_ll &);
    string_ll& operator+= (const string_ll &);
    string_ll& operator+= (const char *);
    string_ll  operator+ (const string_ll &) const;
    string_ll  operator+ (const char *) const;

    bool operator== (const string_ll &) const;
    bool operator== (const char *ptr) const {
       // This routine exists as an optimization; doesn't need to create a temporary
       // instance of "string" for "ptr"; hence, doesn't call string::string(char *)
       // which calls new[].
       return STREQ(ptr, str_);
    }
    bool operator!= (const string_ll &) const;
    bool operator<  (const string_ll &s) const {return STRLT(str_, s.str_);}
    bool operator<= (const string_ll &) const;
    bool operator>  (const string_ll &s) const {return STRGT(str_, s.str_);}
    bool operator>= (const string_ll &) const;

    bool prefix_of (const char *, unsigned) const;
    bool prefix_of (const char *s)          const {return prefix_of(s, STRLEN(s));};
    bool prefix_of (const string_ll &)         const;

    bool prefixed_by (const char *, unsigned) const;
    bool prefixed_by (const char *s)          const {return prefixed_by(s, STRLEN(s));};
    bool prefixed_by (const string_ll &)         const;

    const char*   string_of () const {return str_;}
    unsigned         length () const {return len_;}

    friend ostream& operator<< (ostream &os, const string_ll &s);
    friend debug_ostream& operator<< (debug_ostream &os, const string_ll &s);
//    friend string_ll operator+(const char *, const string_ll &);
//       // a syntactical convenience

    static unsigned       hash (const string_ll &s) {
       s.updateKeyIfNeeded(); return s.key_;
    }

private:
    void updateKeyIfNeeded() const {if (0==key_) updateKey(); }
    void updateKey() const {if (str_) key_ = hashs(str_);}

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

    mutable unsigned key_;
};

#include "util/h/refCounter.h"

class string {
 private:
   refCounter<string_ll> data;

 public:
   static string nil;

   // The second of the constructors below should be faster, but it means
   // we must rely on nil.data being initialized before any global string
   // objects (or static class members) created with this constructor.
   string() : data(string_ll()) {};
   // string() : data(nil.data) {} // should be more efficient than above

   string(const char *str) : data(string_ll(str)) {}

   string(const char *str, unsigned n) : data(string_ll(str,n)) {}

   string(const string& src) : data(src.data) {}
   string(int i) : data(string_ll(i)) {}
   string(long l) : data(string_ll(l)) {}
   string(unsigned u) : data(string_ll(u)) {}
   string(unsigned long ul) : data(string_ll(ul)) {}
   string(float f) : data(string_ll(f)) {}
   string(double d) : data(string_ll(d)) {}
  ~string() {}

   string& operator=(const char *str) {
      string_ll new_str_ll(str);
      refCounter<string_ll> newRefCtr(new_str_ll);
      
      data = newRefCtr;
      return *this;
   }
   string& operator=(const string &src) {
      data = src.data;
      return *this;
   }

   string& operator+=(const string &addme) {
      // see comment in next routine as to why we don't modify in-place.
      string_ll newstr = data.getData() + addme.data.getData();
      data = newstr;
      return *this;
   }

   string& operator+=(const char *str) {
      // You might wonder why we can't just do:
      // data.getData() += str; return *this;
      // The answer is that that would make an in-place modification,
      // thus messing up any others who might be attached to the same object as us.
      // There are two alternative solutions: use prepareToModifyInPlace() followed
      // by the above sequence, or what we use below.  In general, if the in-place
      // modification doesn't require making an entirely new low-level object
      // (the below example does require that), then it's better to use prepareToModifyInPlace()
      // instead of operator=().
      
      string_ll newstr = data.getData() + str;
      data = newstr;
      return *this;
   }

   string operator+(const string &src) const {
      string result = *this;
      return (result += src);
   }
   string operator+(const char *src) const {
      string result = *this;
      return (result += src);
   }
   friend string operator+(const char *src, const string &str);
      // a syntactical convenience

   bool operator==(const string &src) const {
      const string_ll &me = data.getData();
      const string_ll &them = src.data.getData();
      return (me == them);
   }
   bool operator==(const char *ptr) const {
      const string_ll &me = data.getData();
      return (me == ptr);
   }
   bool operator!=(const string &src) const {
      const string_ll &me = data.getData();
      const string_ll &them = src.data.getData();
      return (me != them);
   }
   bool operator<(const string &src) const {
      const string_ll &me = data.getData();
      const string_ll &them = src.data.getData();
      return (me < them);
   }
   bool operator<=(const string &src) const {
      const string_ll &me = data.getData();
      const string_ll &them = src.data.getData();
      return (me <= them);
   }
   bool operator>(const string &src) const {
      const string_ll &me = data.getData();
      const string_ll &them = src.data.getData();
      return (me > them);
   }
   bool operator>=(const string &src) const {
      const string_ll &me = data.getData();
      const string_ll &them = src.data.getData();
      return (me >= them);
   }
   bool prefix_of(const char *str, unsigned n) const {
      const string_ll &me = data.getData();
      return (me.prefix_of(str, n));
   }
   bool prefix_of(const char *str) const {
      const string_ll &me = data.getData();
      return (me.prefix_of(str));
   }
   bool prefix_of(const string &src) const {
      const string_ll &me = data.getData();
      const string_ll &them = src.data.getData();
      return (me.prefix_of(them));
   }

   bool prefixed_by(const char *str, unsigned n) const {
      const string_ll &me = data.getData();
      return (me.prefixed_by(str, n));
   }
   bool prefixed_by(const char *str) const {
      const string_ll &me = data.getData();
      return (me.prefixed_by(str));
   }
   bool prefixed_by(const string &src) const {
      const string_ll &me = data.getData();
      const string_ll &them = src.data.getData();
      return (me.prefixed_by(them));
   }

   const char *string_of() const {
      const string_ll &me = data.getData();
      return me.string_of();
   }

   unsigned length() const {
      const string_ll &me = data.getData();
      return me.length();
   }

   friend ostream& operator<<(ostream &os, const string &s) {
      const string_ll &it = s.data.getData();
      return (os << it);
   }
   friend debug_ostream& operator<<(debug_ostream &os, const string &s) {
      const string_ll &it = s.data.getData();
      return (os << it);
   }

   static unsigned hash(const string &s) {
      const string_ll &it = s.data.getData();
      return (string_ll::hash(it));
   }
};

#endif /* !defined(_String_h_) */
