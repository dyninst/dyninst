/************************************************************************
 * String.h: a simple character string class.
************************************************************************/

#if !defined(_String_h_)
#define _String_h_


/************************************************************************
 * header files.
************************************************************************/

#include <iostream.h>
#include "util/h/headers.h"


/************************************************************************
 * class string
************************************************************************/

class string {
public:
     string ();
     string (const char *);
     string (const string &);
     string (int);      // convert int to its string representation
     string (unsigned); // convert unsigned to its string representation
     string (float);    // convert float to its string representation
     string (double);   // convert double to its string representation
    ~string ();

    string& operator= (const char *);
    string& operator= (const string &);
    string& operator+= (const string &);
    string  operator+ (const string &) const;

    bool operator== (const string &) const;
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

    static unsigned       hash (const string &s) {return s.key_;}

    static string quote;

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
