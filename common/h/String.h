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
 * $Id: String.h,v 1.24 2003/07/15 22:43:28 schendel Exp $
************************************************************************/

#if !defined(_String_h_)
#define _String_h_


/************************************************************************
 * header files.
************************************************************************/

#include "common/h/debugOstream.h"
#include "common/h/headers.h"

/************************************************************************
 * class string
************************************************************************/

const char WILDCARD_CHAR = '?';
const char MULTIPLE_WILDCARD_CHAR = '*';

class string_ll {
public:
    string_ll ();
    string_ll (const char *);
    string_ll (const char *, unsigned n); // just copy the first n chars
    string_ll (const string_ll &);
    ~string_ll ();

    // conversions to string_ll from standard types
    string_ll (char);
    string_ll (int);
    string_ll (unsigned);
    string_ll (long);
    string_ll (unsigned long);
    string_ll (float);
    string_ll (double);

    string_ll& operator= (const char *);
    string_ll& operator= (const string_ll &);
    string_ll& operator+= (const string_ll &);
    string_ll& operator+= (const char *);
    string_ll  operator+ (const string_ll &) const;
    string_ll  operator+ (const char *) const;

    char operator[] (unsigned pos) const {return str_[pos];}

    bool operator== (const string_ll &) const;
    bool operator== (const char *ptr) const {
      // This routine exists as an optimization; doesn't need to create 
      // a temporary instance of "string" for "ptr"; hence, doesn't call
      // string::pdstring(char *) which calls new[].
      return STREQ(ptr, str_);
    }
    bool operator!= (const string_ll &) const;
    bool operator<  (const string_ll &s) const {return STRLT(str_, s.str_);}
    bool operator<= (const string_ll &) const;
    bool operator>  (const string_ll &s) const {return STRGT(str_, s.str_);}
    bool operator>= (const string_ll &) const;

    bool prefix_of (const char *, unsigned) const;
    bool prefix_of (const char *s)          const {return prefix_of(s, STRLEN(s));};
    bool prefix_of (const string_ll &)      const;

    bool prefixed_by (const char *, unsigned) const;
    bool prefixed_by (const char *s)          const {return prefixed_by(s, STRLEN(s));};
    bool prefixed_by (const string_ll &)      const;

    bool suffix_of (const char *, unsigned) const;
    bool suffix_of (const char *s)          const {return suffix_of(s, STRLEN(s));};
    bool suffix_of (const string_ll &)      const;

    bool suffixed_by (const char *, unsigned) const;
    bool suffixed_by (const char *s)          const {return suffixed_by(s, STRLEN(s));};
    bool suffixed_by (const string_ll &)      const;

    unsigned find (const char *, unsigned) const;
    unsigned find (const char *s)          const {return find(s, STRLEN(s));};
    unsigned find (const string_ll &)      const;

    string_ll substr (unsigned, unsigned) const;

    bool wildcardEquiv( const string_ll &, bool ) const;
	bool wildcardEquiv( const char *ptr, bool checkCase ) const {
		return pattern_match( str_, ptr, checkCase );
	}

    bool regexEquiv( const string_ll &them, bool checkCase ) const {
		return regexEquiv( them.str_, checkCase );
	}
	bool regexEquiv( const char *, bool ) const;

    const char*       c_str() const {return str_;}
    unsigned         length() const {return len_;}

    friend ostream& operator<< (ostream &os, const string_ll &s);
    friend debug_ostream& operator<< (debug_ostream &os, const string_ll &s);
//    friend string_ll operator+(const char *, const string_ll &);
//       // a syntactical convenience

    static unsigned       hash (const string_ll &s) {
       s.updateKeyIfNeeded(); return s.key_;
    }

private:
    void updateKeyIfNeeded() const {if (0==key_) updateKey(); }
    void updateKey() const {
       key_ = hashs(str_); // properly checks for NULL; properly won't return 0
    }

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
    static const char*  STRCHR (const char *, char);

    static bool  pattern_match ( const char *, const char *, bool );

    char*    str_;
    unsigned len_;

    mutable unsigned key_;
};

#include "common/h/refCounter.h"

class pdstring {
   friend class string_counter;
 private:
   refCounter<string_ll> data;

   static pdstring *nilptr;

   static void initialize_static_stuff();
   static void free_static_stuff();

 public:
   // The second of the constructors below should be faster, but it means
   // we must rely on nil.data being initialized before any global string
   // objects (or static class members) created with this constructor.
//   pdstring() : data(string_ll()) {};
   pdstring() : data(nilptr->data) {} // should be more efficient than above
   pdstring(const char *str) : data(string_ll(str)) {}
   pdstring(const char *str, unsigned n) : data(string_ll(str,n)) {}
   pdstring(const pdstring& src) : data(src.data) {}
   ~pdstring() {}

   // don't allow implicit conversion to pdstring from standard types;
   // forces correct usage & removes ambiguities
   explicit pdstring(char c) : data(string_ll(c)) {}
   explicit pdstring(int i) : data(string_ll(i)) {}
   explicit pdstring(long l) : data(string_ll(l)) {}
   explicit pdstring(unsigned u) : data(string_ll(u)) {}
   explicit pdstring(unsigned long ul) : data(string_ll(ul)) {}
   explicit pdstring(float f) : data(string_ll(f)) {}
   explicit pdstring(double d) : data(string_ll(d)) {}

   char operator[] (unsigned pos) const {
	   return data.getData()[ pos ];
   }

   pdstring& operator=(const char *str) {
      string_ll new_str_ll(str);
      refCounter<string_ll> newRefCtr(new_str_ll);
      
      data = newRefCtr;
      return *this;
   }
   pdstring& operator=(const pdstring &src) {
      data = src.data;
      return *this;
   }

   pdstring& operator+=(const pdstring &addme) {
      // see comment in next routine as to why we don't modify in-place.
      string_ll newstr = data.getData() + addme.data.getData();
      data = newstr;
      return *this;
   }
   pdstring& operator+=(const char *str) {
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

   pdstring operator+(const pdstring &src) const {
      pdstring result = *this;
      return (result += src);
   }
   pdstring operator+(const char *src) const {
      pdstring result = *this;
      return (result += src);
   }
   friend pdstring operator+(const char *src, const pdstring &str);
      // a syntactical convenience

   bool operator==(const pdstring &src) const {
      const string_ll &me = data.getData();
      const string_ll &them = src.data.getData();
      return (me == them);
   }
   bool operator==(const char *ptr) const {
      const string_ll &me = data.getData();
      return (me == ptr);
   }
   bool operator!=(const pdstring &src) const {
      const string_ll &me = data.getData();
      const string_ll &them = src.data.getData();
      return (me != them);
   }
   bool operator<(const pdstring &src) const {
      const string_ll &me = data.getData();
      const string_ll &them = src.data.getData();
      return (me < them);
   }
   bool operator<=(const pdstring &src) const {
      const string_ll &me = data.getData();
      const string_ll &them = src.data.getData();
      return (me <= them);
   }
   bool operator>(const pdstring &src) const {
      const string_ll &me = data.getData();
      const string_ll &them = src.data.getData();
      return (me > them);
   }
   bool operator>=(const pdstring &src) const {
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
   bool prefix_of(const pdstring &src) const {
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
   bool prefixed_by(const pdstring &src) const {
      const string_ll &me = data.getData();
      const string_ll &them = src.data.getData();
      return (me.prefixed_by(them));
   }

   bool suffixed_by(const char *str, unsigned n) const {
      const string_ll &me = data.getData();
      return (me.suffixed_by(str, n));
   }
   bool suffixed_by(const char *str) const {
      const string_ll &me = data.getData();
      return (me.suffixed_by(str));
   }
   bool suffixed_by(const pdstring &src) const {
      const string_ll &me = data.getData();
      const string_ll &them = src.data.getData();
      return (me.suffixed_by(them));
   }
   
   unsigned find(const char *str, unsigned n) const {
     const string_ll &me = data.getData();
     return (me.find(str, n));
   }
   
   unsigned find(const char *str) const {
     const string_ll &me = data.getData();
     return (me.find(str));
   }

   unsigned find(const pdstring &src) const {
     const string_ll &me = data.getData();
     const string_ll &them = src.data.getData();
     return (me.find(them));
   }


   const char *c_str() const {
      const string_ll &me = data.getData();
      return me.c_str();
   }

   unsigned length() const {
      const string_ll &me = data.getData();
      return me.length();
   }

   friend ostream& operator<<(ostream &os, const pdstring &s) {
      const string_ll &it = s.data.getData();
      return (os << it);
   }
   friend debug_ostream& operator<<(debug_ostream &os, const pdstring &s) {
      const string_ll &it = s.data.getData();
      return (os << it);
   }

   pdstring substr( unsigned pos, unsigned len ) const {
	   pdstring result = *this;
	   string_ll newstr = data.getData().substr( pos, len );
	   result.data = newstr;
	   return result;
   }

   bool wildcardEquiv( const pdstring &src, bool checkCase = true ) const {
      const string_ll &me = data.getData();
      const string_ll &them = src.data.getData();
      return me.wildcardEquiv( them, checkCase );
   }
   bool wildcardEquiv( const char *ptr, bool checkCase = true ) const {
      const string_ll &me = data.getData();
      return me.wildcardEquiv( ptr, checkCase );
   }

   bool regexEquiv( const pdstring &src, bool checkCase = true ) const {
      const string_ll &me = data.getData();
      const string_ll &them = src.data.getData();
      return me.regexEquiv( them, checkCase );
   }
   bool regexEquiv( const char *ptr, bool checkCase = true ) const {
      const string_ll &me = data.getData();
      return me.regexEquiv( ptr, checkCase );
   }

   static unsigned hash(const pdstring &s) {
      const string_ll &it = s.data.getData();
      return (string_ll::hash(it));
   }
};

// See Stroustrup, D & E, sec 3.11.4.2:
class string_counter {
 private:
   static int count;
  
 public:
   string_counter() {
      if (count++ == 0)
         pdstring::initialize_static_stuff();
   }
  ~string_counter() {
      if (--count == 0)
         pdstring::free_static_stuff();
   }
};
static string_counter sc;

#endif /* !defined(_String_h_) */
